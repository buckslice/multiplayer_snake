// ICS 167 Multiplayer Snake Project by:
// Matt Ruiz        28465978    mpruiz@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// John Collins     75665849    jfcollin@uci.edu

#pragma once
#include "SFML/Graphics.hpp"
#include "point.h"
#include <vector>

class Player {
public:
    Player(int id);

    point getPos();
    std::vector<point>& getPoints();
    point getMove();

    point move();

    void spawn(int x, int y, int dx, int dy);

    void grow(int amount);

    bool dead;
    int id;
    int score;

    sf::Color color;
private:
    std::vector<point> points;
    point dir, inone, intwo;
    int growth;
};