#pragma once
#include <GLFW/glfw3.h>

class Input {
public:

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

    static void update();

    static bool pressed(int key);

    static bool justPressed(int key);

    static bool justReleased(int key);

private:

};