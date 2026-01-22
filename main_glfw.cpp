
#include <stdlib.h>
#include <random>
#include <math.h>
#include <fstream>
#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "framebuffer.h"
#include "iso_math.h"
#include "fixed_point.h"

#include "assets/skeleton_alpha.h"
#include "assets/wizard.h"
#include "assets/wizard2.h"

using namespace Framebuffer;

GLFWwindow* g_window = nullptr;
GLuint g_texture = 0;

// Forward declaration
void present_frame();

static std::mt19937 rng(std::random_device{}());

int random_int_distr(int min, int max) {
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}
int random_int_modulo(int min, int max) {
	return min + (rng() % (max - min + 1));
}

void fps_counter() {
	static double last_time = -1.0;  // -1 means "not initialized"
	static uint32_t frame_count = 0;

	// Initialize on first call (after glfwInit has been called)
	if (last_time < 0.0) {
		last_time = glfwGetTime();
	}

	frame_count++;

	double current_time = glfwGetTime();
	double elapsed = current_time - last_time;

	if (elapsed >= 1.0) {  // 1 second passed
		printf("FPS: %u\n", frame_count);
		frame_count = 0;
		last_time = current_time;
	}
}
double delta_time() {

	static double last_time = -1;
	if (last_time < 0.0) last_time = glfwGetTime();

	double current_time = glfwGetTime();
	double delta_time =  current_time - last_time;
	last_time = current_time;
	return delta_time;
}

struct NamedColor {
	const char* name;
	uint16_t value;
};

static const NamedColor COLORS[] = {
	{"RED",     0xF800}, // 0
	{"GREEN",   0x07E0}, // 1
	{"BLUE",    0x001F}, // 2
	{"WHITE",   0xFFFF}, // 3
	{"BLACK",   0x0000}, // 4
	{"GREAY",   0x8410}, // 5
	{"YELLOW",  0xFFE0}, // 6
	{"CYAN",    0x07FF}, // 7
	{"MAGENTA", 0xF81F}  // 8
};

void color_test() {

	for(const auto& color: COLORS) {
		fill_with_color(color.value);
		swap_buffers();
		present_frame();
	}
}

void random_pixels_test() {

	fill_with_color(0x0000);  // Black background
	// Draw complex scene to back buffer
	for(int i = 0; i < 5000; i++) {
		set_pixel(random_int_modulo(0, 160), random_int_modulo(0, 128), COLORS[random_int_modulo(5, 7)].value);
	}

	fps_counter();

	swap_buffers();
	while (!glfwWindowShouldClose(g_window)) {
		present_frame();
	}
}

void line_test(){

	fill_with_color(0x0000);
	draw_line(50, 50, 50, 0xFFE0);
	swap_buffers();
	while (!glfwWindowShouldClose(g_window)) {
		present_frame();
	}
}

void rectangle_test() {

	fill_with_color(0x0000);
	draw_rectangle_memset(128/2 - 25/2, 25, 160/2 - 25/2, 25, 0xFFE0);
	swap_buffers();
	while (!glfwWindowShouldClose(g_window)) {
		present_frame();
	}
}

struct Entity {

	Fixed_q16	y;
	uint16_t	height;

	Fixed_q16	x;
	uint16_t	width;

	uint16_t	color;
};

bool handle_movement(Entity& rect) {

	Fixed_q16 speed = 100;
	Fixed_q16 dt(static_cast<float>(delta_time()));
	Fixed_q16 movement = speed * dt;

	bool moved = false;
	if (glfwGetKey(g_window, GLFW_KEY_W) == GLFW_PRESS && rect.y - movement >= 0) {
		rect.y -= movement;
		moved = true;
	}
	if (glfwGetKey(g_window, GLFW_KEY_A) == GLFW_PRESS && rect.x - movement >= 0) {
		rect.x -= movement;
		moved = true;
	}
	if (glfwGetKey(g_window, GLFW_KEY_S) == GLFW_PRESS && rect.y + rect.height + movement <= SCREEN_HEIGHT) {
		rect.y += movement;
		moved = true;
	}
	if (glfwGetKey(g_window, GLFW_KEY_D) == GLFW_PRESS && rect.x + rect.width + movement <= SCREEN_WIDTH) {
		rect.x += movement;
		moved = true;
	}
	return moved;
}

void movement_tracking_test_regular() {

	Entity rect = {128/2 - 25/2, 25, 160/2 - 25/2, 25, 0xFFE0};
	fill_with_color(0x0000);
	draw_rectangle_memset(rect.y.to_int(), rect.height, rect.x.to_int(), rect.width, rect.color);
	swap_buffers();
	present_frame();

	while (!glfwWindowShouldClose(g_window)) {
		glfwPollEvents();

		if (handle_movement(rect)) {
			fill_with_color(0x0000);
			draw_rectangle_memset(rect.y.to_int(), rect.height, rect.x.to_int(), rect.width, rect.color);
			fps_counter();
			swap_buffers();
			present_frame();
		}
	}
}
void movement_tracking_test_polac() {
	// this function performance could be greatly optimised by using frame_buffer insead of set_pixel. drawing rectangle and select random color.
	Entity rect = {128/2 - 25/2, 25, 160/2 - 25/2, 25, 0xFFE0};
	fill_with_color(0x0000);
	for(int i = 0; i < 3536; i++) {
		set_pixel(random_int_modulo(rect.x.to_int(), rect.x.to_int() + rect.width), random_int_modulo(rect.y.to_int(), rect.y.to_int() + rect.height), COLORS[random_int_modulo(5, 7)].value);
	}
	swap_buffers();
	present_frame();

	while (!glfwWindowShouldClose(g_window)) {
		glfwPollEvents();

		if (handle_movement(rect)) {
			fill_with_color(0x0000);
			for(int i = 0; i < 3536; i++) {
				set_pixel(random_int_modulo(rect.x.to_int(), rect.x.to_int() + rect.width), random_int_modulo(rect.y.to_int(), rect.y.to_int() + rect.height), COLORS[random_int_modulo(5, 7)].value);
			}
			fps_counter();
			swap_buffers();
			present_frame();
		}
		else {
			fill_with_color(0x0000);
			for(int i = 0; i < 3536; i++) {
				set_pixel(random_int_modulo(rect.x.to_int(), rect.x.to_int() + rect.width), random_int_modulo(rect.y.to_int(), rect.y.to_int() + rect.height), COLORS[random_int_modulo(5, 7)].value);
			}
			fps_counter();
			swap_buffers();
			present_frame();
		}
	}
}

void sprite_test() {

	fill_with_color(COLORS[3].value);
	// draw_sprite(128/2 - 25/2, 43, 160/2 - 25/2, 59, skeleton_data);

	draw_sprite_alpha(2, skeleton_alpha_height, 2, skeleton_alpha_width, skeleton_alpha_data);
	draw_sprite_alpha(2, skeleton_alpha_height, 160 - skeleton_alpha_width, skeleton_alpha_width, skeleton_alpha_data);

	draw_sprite_alpha(128/2 - skeleton_alpha_height/2, skeleton_alpha_height, 160/2 - skeleton_alpha_width/2, skeleton_alpha_width, skeleton_alpha_data);

	draw_sprite_alpha(128 - skeleton_alpha_height, skeleton_alpha_height, 2, skeleton_alpha_width, skeleton_alpha_data);
	draw_sprite_alpha(128 - skeleton_alpha_height, skeleton_alpha_height, 160 - skeleton_alpha_width, skeleton_alpha_width, skeleton_alpha_data);

	swap_buffers();
	while (!glfwWindowShouldClose(g_window)) {
		present_frame();
	}
}

void movement_tracking_test_sprite_skeleton() {

	Entity sprite_coord = {128/2 - skeleton_alpha_height/2, skeleton_alpha_height, 160/2 - skeleton_alpha_width/2, skeleton_alpha_width};
	fill_with_color(COLORS[3].value);
	draw_sprite_alpha(sprite_coord.y.to_int(), sprite_coord.height, sprite_coord.x.to_int(), sprite_coord.width, skeleton_alpha_data);
	swap_buffers();
	present_frame();

	while (!glfwWindowShouldClose(g_window)) {

		glfwPollEvents();
		if (handle_movement(sprite_coord)) {
			fill_with_color(COLORS[3].value);
			draw_sprite_alpha(sprite_coord.y.to_int(), sprite_coord.height, sprite_coord.x.to_int(), sprite_coord.width, skeleton_alpha_data);
			fps_counter();
			swap_buffers();
			present_frame();
		}
	}
}
void movement_tracking_test_sprite_wizard() {

	Entity wizard = {2, wizard_height, 2, wizard_width};
	Entity wizard2 = {128/2 - wizard2_height/2, wizard2_height, 160/2 - wizard2_width/2, wizard2_width};

	fill_with_color(COLORS[4].value);
	draw_sprite_alpha(wizard.y.to_int(), wizard.height, wizard.x.to_int(), wizard.width, wizard_data);
	draw_sprite_alpha(wizard2.y.to_int(), wizard2.height, wizard2.x.to_int(), wizard2.width, wizard2_data);

	swap_buffers();
	present_frame();

	while (!glfwWindowShouldClose(g_window)) {

		glfwPollEvents();
		fill_with_color(COLORS[4].value);
		draw_sprite_alpha(wizard.y.to_int(), wizard.height, wizard.x.to_int(), wizard.width, wizard_data);
		handle_movement(wizard2);
		draw_sprite_alpha(wizard2.y.to_int(), wizard2.height, wizard2.x.to_int(), wizard2.width, wizard2_data);
		fps_counter();
		swap_buffers();
		present_frame();
	}
}

void bresenham_line_drawing_test() {

	fill_with_color(COLORS[4].value);
	draw_line_bresenham(0, 0, 159, 127, COLORS[6].value);
	draw_line_bresenham(159/2, 0, 159/2, 127, COLORS[6].value);
	draw_line_bresenham(0, 127/2, 159, 127/2, COLORS[6].value);
	draw_line_bresenham(159, 0, 0, 127, COLORS[6].value);

	swap_buffers();
	while (!glfwWindowShouldClose(g_window)) {
		present_frame();
	}
}

void diamond_outline_test() {

	fill_with_color(COLORS[4].value);
	draw_diamond_outline(159/2, 127/2, 32, 16, COLORS[6].value);

	swap_buffers();
	while (!glfwWindowShouldClose(g_window)) {
		present_frame();
	}

}

void world_to_screen_test() {

}


void error_callback(int error, const char* description) {
	fprintf(stderr, "Error %s\n", description);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void present_frame() {
	glBindTexture(GL_TEXTURE_2D, g_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 160, 128, 0,
				GL_RGB, GL_UNSIGNED_SHORT_5_6_5, get_front_buffer());

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f,  1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f,  1.0f);
	glEnd();

	glfwSwapBuffers(g_window);
	glfwPollEvents();
}

int main(){

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) return -1;  // MUST call first

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	g_window = glfwCreateWindow(160*4, 128*4, "PocketGateEngine", NULL, NULL);
	if (!g_window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(g_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);  // load OpenGL
	glfwSwapInterval(0);

	glGenTextures(1, &g_texture);
	glBindTexture(GL_TEXTURE_2D, g_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glfwSetKeyCallback(g_window, key_callback);

	// color_test();
	// random_pixels_test();
	// line_test();
	// rectangle_test();
	// movement_tracking_test_polac();
	// sprite_test();
	movement_tracking_test_sprite_wizard();
	// bresenham_line_drawing_test();
	// diamond_outline_test();

	glfwDestroyWindow(g_window);
	glfwTerminate();
	exit(EXIT_SUCCESS);

	return 0;
}
