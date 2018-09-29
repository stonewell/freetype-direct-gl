#include "glfw_hack.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#include "GLFW/glfw3native.h"

void updateGlfwContext(GLFWwindow *window) {
	[glfwGetNSGLContext(window) update];
}
