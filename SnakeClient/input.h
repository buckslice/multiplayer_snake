// ICS 167 Multiplayer Snake Project by:
// Matt Ruiz        28465978    mpruiz@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// John Collins     75665849    jfcollin@uci.edu

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