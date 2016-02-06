#include "input.h"

static bool thisframe[GLFW_KEY_LAST];
static bool lastframe[GLFW_KEY_LAST];

void Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (action == GLFW_PRESS) {
        thisframe[key] = true;
    } else if (action == GLFW_RELEASE) {
        thisframe[key] = false;
    }
}

void Input::update() {
    for (int i = 0; i < GLFW_KEY_LAST; ++i) {
        lastframe[i] = thisframe[i];
    }
}

bool Input::pressed(int key) {
    return thisframe[key];
}

bool Input::justPressed(int key) {
    return (thisframe[key] && !lastframe[key]);
}

bool Input::justReleased(int key) {
    return !thisframe[key] && lastframe[key];
}