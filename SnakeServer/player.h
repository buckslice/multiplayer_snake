// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis		13792598	gmachlis@uci.edu

#pragma once
#include "SFML/Graphics.hpp"
#include "point.h"
#include <vector>
#include <unordered_map>

struct PlayerInput {
    point inone;
    point intwo;
};

class Player {
public:
    Player(int id);

    point getPos();
    std::vector<point>& getPoints();
    point getMove();

    point move();

    void spawn(int x, int y, int dx, int dy);

    void grow(int amount);

    friend bool operator==(const Player& p1, const Player& p2);

    point dir, inone, intwo;
    bool dead;
    int id;
    int score;
    std::string playerName;

private:
    std::vector<point> points;
    int growth;
};