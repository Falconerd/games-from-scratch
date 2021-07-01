#include "shared.h"
#include "input.h"

static Input_Context context = {0};

void input_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

Input_Context *input_setup() {
	return &context;
}
