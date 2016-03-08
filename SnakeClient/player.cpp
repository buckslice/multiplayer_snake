// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis		13792598	gmachlis@uci.edu

#include "player.h"
#include "input.h"

Player::Player(int id) : id{ id } {
    points.clear();
    growth = 0;
    score = 0;
    dead = false;
}

void Player::spawn(int x, int y, int dx, int dy) {
    points.clear();
    points.push_back({ x,y });
    dir = { dx,dy };
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

bool operator==(const Player& p1, const Player& p2) {
    if ((p1.points.size() != p2.points.size()) || !(p1.dir == p2.dir)) {
        return false;
    }
    for (size_t i = 0; i < p1.points.size(); i++) {
        if (!(p1.points[i] == p2.points[i]))
            return false;
    }
    return true;
}