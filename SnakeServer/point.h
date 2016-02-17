// ICS 167 Multiplayer Snake Project by:
// Matt Ruiz        28465978    mpruiz@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// John Collins     75665849    jfcollin@uci.edu

#pragma once

struct point {  // should prob just switch to sf::Vector2i now that we are using SFML
    int x, y;
    struct point& operator+=(const point& p) { x += p.x; y += p.y; return *this; }
    friend point operator+(const point& p1, const point& p2) {
        return{ p1.x + p2.x, p1.y + p2.y };
    }
};