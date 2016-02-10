#pragma once
#include <SFML/Window/Keyboard.hpp>

class Input {
public:

    static void update();

    static bool pressed(int key);

    static bool justPressed(int key);

    static bool justReleased(int key);

private:

};