/**
 * @file main.cpp
 * @brief PocketGateEngine STM32H743 Entry Point
 *
 * Initial hardware test for WeAct STM32H743VIT6 + 4" ST7796S Display
 */

#include "stm32h7xx_hal.h"
#include "board_config.h"
#include <cstdio>

// =============================================================================
// Peripheral Handles
// =============================================================================

SPI_HandleTypeDef hspi3;
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_spi3_tx;

// =============================================================================
// Forward Declarations
// =============================================================================

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void SPI3_Init(void);
static void I2C1_Init(void);
static void Error_Handler(void);

// Test functions
static void LED_Test(void);
static void LCD_Reset(void);
static bool LCD_Test_Communication(void);

// =============================================================================
// Main Entry Point
// =============================================================================

int main(void)
{
    // HAL initialization
    HAL_Init();

    // Configure system clock to 480 MHz
    SystemClock_Config();

    // Initialize peripherals
    GPIO_Init();
    SPI3_Init();
    I2C1_Init();

    // Turn on backlight
    LCD_LED_ON();

    // Reset LCD
    LCD_Reset();

    // Test communication
    printf("PocketGateEngine STM32H743 Booting...\n");
    printf("System Clock: %lu MHz\n", SystemCoreClock / 1000000);

    if (LCD_Test_Communication()) {
        printf("LCD Communication: OK\n");
    } else {
        printf("LCD Communication: FAILED\n");
    }

    // Main loop - simple LED blink to confirm we're running
    while (1)
    {
        LED_Test();
        HAL_Delay(500);
    }

    return 0;
}

// =============================================================================
// System Clock Configuration (480 MHz from 25 MHz HSE)
// =============================================================================

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Supply configuration update enable
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    // Configure voltage scaling for max performance
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    // Configure HSE and PLL
    // PLL: 25 MHz / 5 * 192 / 2 = 480 MHz
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 5;
    RCC_OscInitStruct.PLL.PLLN = 192;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    // Configure system clocks
    // SYSCLK = 480 MHz, HCLK = 240 MHz, APB1/APB2 = 120 MHz
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }
}

// =============================================================================
// GPIO Initialization
// =============================================================================

static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable GPIO clocks
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    // Configure LCD control pins as outputs (PE5-PE11)
    GPIO_InitStruct.Pin = LCD_CS_PIN | LCD_RST_PIN | LCD_DC_PIN | LCD_LED_PIN
                        | CTP_RST_PIN | SD_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    // Configure touch interrupt pin (PE6) as input with falling edge interrupt
    GPIO_InitStruct.Pin = CTP_INT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(CTP_INT_PORT, &GPIO_InitStruct);

    // Set default states
    LCD_CS_HIGH();      // Deselect LCD
    SD_CS_HIGH();       // Deselect SD card
    LCD_RST_HIGH();     // Not in reset
    CTP_RST_HIGH();     // Not in reset
    LCD_LED_OFF();      // Backlight off initially
}

// =============================================================================
// SPI3 Initialization (Display + SD Card)
// =============================================================================

static void SPI3_Init(void)
{
    // Enable SPI3 clock
    __HAL_RCC_SPI3_CLK_ENABLE();

    // Configure SPI3 pins
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = SPI3_SCK_PIN | SPI3_MISO_PIN | SPI3_MOSI_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = SPI3_SCK_AF;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Configure SPI3
    hspi3.Instance = SPI3;
    hspi3.Init.Mode = SPI_MODE_MASTER;
    hspi3.Init.Direction = SPI_DIRECTION_2LINES;
    hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi3.Init.NSS = SPI_NSS_SOFT;
    hspi3.Init.BaudRatePrescaler = SPI_PRESCALER_SAFE;  // Start slow (7.5 MHz)
    hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi3.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    hspi3.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;

    if (HAL_SPI_Init(&hspi3) != HAL_OK) {
        Error_Handler();
    }
}

// =============================================================================
// I2C1 Initialization (Touch Controller)
// =============================================================================

static void I2C1_Init(void)
{
    // Enable I2C1 clock
    __HAL_RCC_I2C1_CLK_ENABLE();

    // Configure I2C1 pins
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = I2C1_SCL_PIN | I2C1_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // External pull-ups on board
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = I2C1_SCL_AF;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Configure I2C1 - 400 kHz Fast Mode
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10C0ECFF;  // 400 kHz @ 120 MHz APB1
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}

// =============================================================================
// Test Functions
// =============================================================================

/**
 * @brief Simple LED test - toggles backlight
 */
static void LED_Test(void)
{
    static bool led_state = false;
    led_state = !led_state;

    if (led_state) {
        LCD_LED_ON();
    } else {
        LCD_LED_OFF();
    }
}

/**
 * @brief Hardware reset the LCD
 */
static void LCD_Reset(void)
{
    LCD_RST_HIGH();
    HAL_Delay(10);
    LCD_RST_LOW();
    HAL_Delay(10);
    LCD_RST_HIGH();
    HAL_Delay(120);  // Wait for LCD to initialize
}

/**
 * @brief Test SPI communication with LCD
 * @return true if communication successful
 */
static bool LCD_Test_Communication(void)
{
    uint8_t cmd = 0x04;  // RDDID - Read Display ID
    uint8_t data[4] = {0};

    LCD_CS_LOW();
    LCD_DC_CMD();

    // Send command
    if (HAL_SPI_Transmit(&hspi3, &cmd, 1, 100) != HAL_OK) {
        LCD_CS_HIGH();
        return false;
    }

    LCD_DC_DATA();

    // Read response (dummy + 3 ID bytes)
    if (HAL_SPI_Receive(&hspi3, data, 4, 100) != HAL_OK) {
        LCD_CS_HIGH();
        return false;
    }

    LCD_CS_HIGH();

    // ST7796S should return ID bytes
    printf("LCD ID: 0x%02X 0x%02X 0x%02X\n", data[1], data[2], data[3]);

    // Check if we got non-zero response
    return (data[1] != 0x00 || data[2] != 0x00 || data[3] != 0x00);
}

// =============================================================================
// Error Handler
// =============================================================================

static void Error_Handler(void)
{
    // Disable interrupts
    __disable_irq();

    // Blink LED rapidly to indicate error
    while (1) {
        HAL_GPIO_TogglePin(LCD_LED_PORT, LCD_LED_PIN);
        for (volatile int i = 0; i < 1000000; i++) {}
    }
}

// =============================================================================
// HAL Callbacks
// =============================================================================

extern "C" void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
}

// SysTick handler for HAL timing
extern "C" void SysTick_Handler(void)
{
    HAL_IncTick();
}

// For printf support via SWO/ITM (optional, requires debugger)
extern "C" int _write(int file, char *ptr, int len)
{
    (void)file;
    for (int i = 0; i < len; i++) {
        ITM_SendChar(*ptr++);
    }
    return len;
}
