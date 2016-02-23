// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis		13792598	gmachlis@uci.edu

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