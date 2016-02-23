// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis		13792598	gmachlis@uci.edu

#include "input.h"

const int KEYS = sf::Keyboard::KeyCount;

static bool thisframe[KEYS];
static bool lastframe[KEYS];

void Input::update() {
    for (int i = 0; i < KEYS; i++) {
        lastframe[i] = thisframe[i];
        thisframe[i] = sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(i));
    }
}

bool Input::pressed(int key) {
    return thisframe[key];
}

bool Input::justPressed(int key) {
    return thisframe[key] && !lastframe[key];
}

bool Input::justReleased(int key) {
    return !thisframe[key] && lastframe[key];
}