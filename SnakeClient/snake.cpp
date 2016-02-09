
#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <iostream>

#include "snake.h"
#include "input.h"

Snake::Snake() {
    start();
}

void Snake::start() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Snake!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, Input::key_callback);

    glewExperimental = GL_TRUE;
    glewInit();

    glViewport(0, 0, WIDTH, HEIGHT);

    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vert, NULL);
    glCompileShader(vertexShader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &frag, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    map.generate((int)(WIDTH / TILE), (int)(HEIGHT / TILE));
    generateTriangles(tris);

    GLuint EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tris.size()*sizeof(GLuint), &tris[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(CVertex), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CVertex), (GLvoid*)(2 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    int ids = PLAYER;
    players.push_back(Player(true, ids++));
    players.push_back(Player(false, ids++));

    std::vector<point> spawns;
    spawns.push_back({ 4, 4 });
    spawns.push_back({ map.getW() - 4, 4 });

    for (size_t i = 0; i < players.size(); ++i) {
        point p = spawns[i];
        players[i].spawn(p.x,p.y, 0, 1);
        map.setTile(players[i].getPos(), players[i].id);
    }

    map.spawnRandom(FOOD);
    gameTime = -1.0;
    render();

    double lastTime = glfwGetTime();
    bool running = true;
    while (running) {
        Input::update();
        glfwPollEvents();

        if (glfwWindowShouldClose(window) || Input::justPressed(GLFW_KEY_ESCAPE)) {
            running = false;
        }

        for (size_t i = 0; i < players.size(); ++i) {
            players[i].checkInput();
        }

        double now = glfwGetTime();
        double delta = now - lastTime;
        lastTime = now;
        gameTime += delta;

        double gameTick = 0.1;  // snake moves 10 times a second
        if (gameTime >= gameTick) {
            // do game tick
            glClearColor(0.05f, 0.1f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            for (size_t i = 0; i < players.size(); ++i) {
                // figure out dir
                Player& p = players[i];
                point mv = p.getMove();
                if (map.isWalkable(mv.x, mv.y)) {   // valid move
                    point oldend = p.move();
                    map.setTile(oldend, 0);
                    map.setTile(p.getPos(), p.id);
                } else if (map.getTile(mv.x, mv.y) == FOOD) {   // ran into food
                    point oldend = p.move();
                    map.setTile(oldend, 0);
                    map.setTile(p.getPos(), p.id);
                    p.grow(3);
                    map.spawnRandom(FOOD);
                } else {    // hit wall or part of snake
                    p.dead = true;
                }
                if (p.dead) {
                    gameTime = -2.0;
                    break;
                } else {
                    gameTime = 0.0;
                }
            }

            render();
        }

        bool playerIsDead = false;
        for (size_t i = 0; i < players.size(); ++i) {
            playerIsDead = playerIsDead || players[i].dead;
        }

        if (playerIsDead && gameTime > -1.0) {
            map.generate((int)(WIDTH / TILE), (int)(HEIGHT / TILE));
            for (size_t i = 0; i < players.size(); ++i) {
                point p = spawns[i];
                players[i].spawn(p.x, p.y, 0, 1);
                map.setTile(players[i].getPos(), players[i].id);
            }
            map.spawnRandom(FOOD);
            render();
        }

    }

    // shut down
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
}

void Snake::render() {
    generateVertices(verts);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(CVertex), &verts[0], GL_STREAM_DRAW);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, tris.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
}

void Snake::generateVertices(std::vector<CVertex>& verts) {
    verts.clear();
    for (int y = 0; y < map.getH(); ++y) {
        for (int x = 0; x < map.getW(); ++x) {
            int id = map.getTile(x, y);

            float x0 = x * TILE;
            float y0 = y * TILE;
            float x1 = (x + 1) * TILE;
            float y1 = (y + 1) * TILE;

            float r, g, b;
            switch (id) {
            case GROUND:
                r = 0.2f; g = 0.3f, b = 0.3f;
                break;
            case WALL:
                r = 0.05f; g = 0.1f, b = 0.2f;
                break;
            case FOOD:
                r = 0.0f; g = 1.0f; b = 0.0f;
                break;
            default:
                int index = id - PLAYER;
                if (gameTime < 0.0 && players[index].dead) {
                    r = 1.0f; g = 0.0f; b = 0.0f;
                } else {
                    r = 1.0f; g = 0.5f + 0.5f * index; b = 0.2f;
                }
                break;
            }

            verts.push_back(CVertex{ x0,y0,r,g,b });
            verts.push_back(CVertex{ x0,y1,r,g,b });
            verts.push_back(CVertex{ x1,y1,r,g,b });
            verts.push_back(CVertex{ x1,y0,r,g,b });
        }
    }
    // weird hack to bypass need for orthographic projection
    for (size_t i = 0; i < verts.size(); ++i) {
        verts[i].x = (verts[i].x - WIDTH / 2.0f) / (WIDTH / 2.0f);
        verts[i].y = (verts[i].y - HEIGHT / 2.0f) / (HEIGHT / 2.0f);
    }
}

void Snake::generateTriangles(std::vector<GLuint>& tris) {
    tris.clear();
    GLuint t = 0;
    for (int i = 0; i < map.getW() * map.getH(); ++i) {
        tris.push_back(t);
        tris.push_back(t + 1);
        tris.push_back(t + 2);
        tris.push_back(t + 2);
        tris.push_back(t + 3);
        tris.push_back(t);
        t += 4;
    }
}

int main() {
    Snake game;
    return 0;
}