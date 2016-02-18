// ICS 167 Multiplayer Snake Project by:
// Matt Ruiz        28465978    mpruiz@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// John Collins     75665849    jfcollin@uci.edu

#include "player.h"

Player::Player(int id) : id{ id } {

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

std::vector<point>& Player::getPoints() {
    return points;
}

point Player::getMove() {
    if (inone.x != 0 || inone.y != 0) {
        dir = inone;
        inone = intwo;
        intwo = { 0,0 };
    }
    return points[0] + dir;
}