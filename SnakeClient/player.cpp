#include "player.h"
#include "input.h"

void Player::checkInput() {
    if (Input::justPressed(GLFW_KEY_LEFT) || Input::justPressed(GLFW_KEY_A)) {
        if (dir.x == 0) {
            inone = { -1,0 };
        } else if (inone.y != 0) {
            intwo = { -1,0 };
        }
    }
    if (Input::justPressed(GLFW_KEY_RIGHT) || Input::justPressed(GLFW_KEY_D)) {
        if (dir.x == 0) {
            inone = { 1,0 };
        } else if (inone.y != 0) {
            intwo = { 1,0 };
        }
    }
    if (Input::justPressed(GLFW_KEY_DOWN) || Input::justPressed(GLFW_KEY_S)) {
        if (dir.y == 0) {
            inone = { 0,-1 };
        } else if (inone.x != 0) {
            intwo = { 0,-1 };
        }
    }
    if (Input::justPressed(GLFW_KEY_UP) || Input::justPressed(GLFW_KEY_W)) {
        if (dir.y == 0) {
            inone = { 0,1 };
        } else if (inone.x != 0) {
            intwo = { 0,1 };
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