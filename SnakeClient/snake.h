#pragma once
#include "map.h"
#include "player.h"

struct CVertex {
    float x, y;
    float r, g, b;
};

class Snake {
public:
    Snake();

    const GLuint WIDTH = 800;
    const GLuint HEIGHT = 600;
    const GLfloat TILE = 25.0f;

    static const GLuint GROUND = 0;
    static const GLuint WALL = 1;
    static const GLuint FOOD = 2;
    static const GLuint PLAYER = 3;
private:
    GLFWwindow* window;

    Map map;
    std::vector<Player> players;

    void start();
    void render();

    double gameTime;

    // graphics stuff
    GLuint VBO, VAO, shaderProgram;
    std::vector<CVertex> verts;
    std::vector<GLuint> tris;
    void generateVertices(std::vector<CVertex>& verts);
    void generateTriangles(std::vector<GLuint>& tris);

};

// inline shaders
#define GLSL(version, shader) "#version " #version "\n" #shader

const GLchar* vert =
GLSL(330 core, layout(location = 0) in vec2 p; layout(location = 1) in vec3 col;
out vec3 Color;
void main() {
    gl_Position = vec4(p, 0.0, 1.0);
    Color = col;
}
);
const GLchar* frag =
GLSL(330 core, in vec3 Color;
out vec4 color;
void main() {
    color = vec4(Color, 1.0f);
}
);