#pragma once

#include <GLFW/glfw3.h>

//temp fix for glfw on macos 10.14 or upper
//need update the context before swap buffer

#ifdef __cplusplus
extern "C"
{
#endif
void updateGlfwContext(GLFWwindow *window);
#ifdef __cplusplus
}
#endif
