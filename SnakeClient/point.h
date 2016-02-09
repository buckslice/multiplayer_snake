#pragma once

struct point {
    int x, y;
    struct point& operator+=(const point& p) { x += p.x; y += p.y; return *this; }
    friend point operator+(const point& p1, const point& p2) {
        return{ p1.x + p2.x, p1.y + p2.y };
    }
};