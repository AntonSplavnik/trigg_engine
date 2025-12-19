#ifndef ST7735_COMMANDS_H
#define ST7735_COMMANDS_H

// System Function Commands
#define ST7735_NOP     0x00  // No Operation
#define ST7735_SWRESET 0x01  // Software Reset
#define ST7735_SLPIN   0x10  // Sleep In
#define ST7735_SLPOUT  0x11  // Sleep Out
#define ST7735_PTLON   0x12  // Partial Mode On
#define ST7735_NORON   0x13  // Normal Display Mode On

// Display Function Commands
#define ST7735_INVOFF  0x20  // Display Inversion Off
#define ST7735_INVON   0x21  // Display Inversion On
#define ST7735_DISPOFF 0x28  // Display Off
#define ST7735_DISPON  0x29  // Display On
#define ST7735_CASET   0x2A  // Column Address Set
#define ST7735_RASET   0x2B  // Row Address Set
#define ST7735_RAMWR   0x2C  // Memory Write

// Panel Function Commands
#define ST7735_MADCTL  0x36  // Memory Data Access Control
#define ST7735_COLMOD  0x3A  // Interface Pixel Format

// More commands...
#define ST7735_FRMCTR1 0xB1  // Frame Rate Control (Normal mode)
#define ST7735_PWCTR1  0xC0  // Power Control 1
#define ST7735_GMCTRP1 0xE0  // Gamma Correction (positive)

#endif
