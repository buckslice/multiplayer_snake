// ICS 167 Multiplayer Snake Project by:
// Matt Ruiz        28465978    mpruiz@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// John Collins     75665849    jfcollin@uci.edu

#include "player.h"
#include "input.h"

Player::Player(int id) : id{ id } {

}

void Player::checkInput() {
    if (Input::justPressed(sf::Keyboard::A) || Input::justPressed(sf::Keyboard::Left)) {
        if (dir.x == 0) {
            inone = { -1,0 };
        } else if (inone.y != 0) {
            intwo = { -1,0 };
        }
    }
    if (Input::justPressed(sf::Keyboard::D) || Input::justPressed(sf::Keyboard::Right)) {
        if (dir.x == 0) {
            inone = { 1,0 };
        } else if (inone.y != 0) {
            intwo = { 1,0 };
        }
    }
    if (Input::justPressed(sf::Keyboard::S) || Input::justPressed(sf::Keyboard::Down)) {
        if (dir.y == 0) {
            inone = { 0,1 };
        } else if (inone.x != 0) {
            intwo = { 0,1 };
        }
    }
    if (Input::justPressed(sf::Keyboard::W) || Input::justPressed(sf::Keyboard::Up)) {
        if (dir.y == 0) {
            inone = { 0,-1 };
        } else if (inone.x != 0) {
            intwo = { 0,-1 };
        }
    }
}

void Player::spawn(int x, int y, int dx, int dy) {
    points.clear();
    points.push_back({ x,y });
    dir = { dx,dy };
    inone = { 0,0 };
    intwo = { 0,0 };
    growth = 0;
    score = 0;
    dead = false;
}

void Player::grow(int amount) {
    growth += amount;
}

point Player::move() {  // return -1,-1 if growing
    size_t i = points.size() - 1;
    point end = points[i];
    while (i >= 1) {
        points[i] = points[i - 1];
        --i;
    }
    points[0] += dir;
    if (--growth < 0) {
        growth = 0;
        return end;
    } else {
        points.push_back({ end.x,end.y });
        return{ -1,-1 };
    }
}

point Player::getPos() {
    return points[0];
}

point Player::getMove() {
    if (inone.x != 0 || inone.y != 0) {
        dir = inone;
        inone = intwo;
        intwo = { 0,0 };
    }
    return points[0] + dir;
}