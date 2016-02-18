// ICS 167 Multiplayer Snake Project by:
// Matt Ruiz        28465978    mpruiz@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// John Collins     75665849    jfcollin@uci.edu

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>

#include "snake.h"
#include "input.h"

Snake::Snake() {
    init();
    start();
}

void Snake::init() {
    std::string port;
    std::cout << "Enter port number: ";
    std::getline(std::cin, port);
    if (port == "") {
        port = "8000";
        std::cout << "defaulting to port: " << port << std::endl;
    }

    std::string ip;
    std::cout << "Enter IP address: ";
    std::getline(std::cin, ip);
    if (ip == "") {
        ip = "127.0.0.1";
        std::cout << "defaulting to ip: " << ip << std::endl;
    }

    if (socket.connect(ip, std::atoi(port.c_str())) != sf::Socket::Done) {
        std::cout << "SOCKET FAILED TO CONNECT!" << std::endl;
    } else {
        std::cout << "SOCKET CONNECTED!" << std::endl;
    }
    socket.setBlocking(false);

    sendData("Hello from the client!");
    sendData("Hello from the client!");

    // build window
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;
    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Snake Client", sf::Style::Default, settings);
    window->setFramerateLimit(60);

    if (!font.loadFromFile("OCRAEXT.TTF")) {
        std::cout << "FAILED LOADING FONT" << std::endl;
    }

    map = Map((int)(WIDTH / TILE), (int)(HEIGHT / TILE) - 1);
    map.pos = { 0,(int)TILE };
    map.generate();

    text.setFont(font);
    text.setCharacterSize(30);
}

void Snake::start() {

    sf::Clock frameTime;
    bool running = true;
    while (running) {
        float delta = frameTime.restart().asSeconds();

        // process window events
        sf::Event e;
        while (window->pollEvent(e)) {
            switch (e.type) {
            case sf::Event::Closed:
                running = false;
                break;
            default:
                break;
            }
        }

        // update input arrays
        Input::update();

        if (Input::justPressed(sf::Keyboard::Escape)) {
            running = false;
        }

        checkServerMessage();

        if (gameRunning) {
            players[playerIndex].checkInput();

            // send message to server containing your direction

        }
        
        // renders the board and limits framerate
        render();
    }

    socket.disconnect();

    // cleanup
    delete window;
}

template <typename T>
void Snake::sendData(T data) {
    std::ostringstream oss;
    oss << data;
    std::string s = oss.str();
    size_t len;
    socket.send(s.c_str(), s.length(), len);
}

void Snake::checkServerMessage() {
    // check for scores from server
    sf::Socket::Status status = socket.receive(in, sizeof(in), received_len);
    if (status != sf::Socket::Done) {
        clearMessageBuffer(); // not sure
        return;
    }

    // set null character to ignore anything after received length (non blocking sockets get some trash data at end)
    in[received_len] = '\0';
    std::string msg = std::string(in);
    if (msg == "") {
        return;
    }

    std::istringstream mss(msg);
    // for each message in stream
    while (!mss.eof()) {
        std::string update;
        getline(mss, update, '.'); // full game updates are seperated by '.'
        std::cout << update << std::endl;

        // now gotta split update up and process
        // use another istringstream within the while probably

        //int type = msg.at(0) - '0';
        //if (type == 0) {    // game state update from server
        //} else if (type == 1){
        //    playerIndex = msg.at(1) - '0';
        //} else {
        //    std::cout << "unknown message type received from server" << std::endl;
        //}
    }

    clearMessageBuffer();

}

void Snake::clearMessageBuffer() {
    int len = sizeof(in) / sizeof(char);
    for (int i = 0; i < len; ++i) {
        in[i] = '\0';
    }
}

void Snake::render() {
    window->clear(sf::Color(12, 26, 51));

    sf::VertexArray verts;
    verts.setPrimitiveType(sf::PrimitiveType::Quads);

    generateVertices(verts);

    window->draw(verts);

    // add text back in later

    window->display();
}

void Snake::generateVertices(sf::VertexArray& verts) {
    for (int y = 0; y < map.getH(); ++y) {
        for (int x = 0; x < map.getW(); ++x) {
            int id = map.getTile(x, y);

            float x0 = x * TILE + map.pos.x;
            float y0 = y * TILE + map.pos.y;
            float x1 = (x + 1) * TILE + map.pos.x;
            float y1 = (y + 1) * TILE + map.pos.y;

            sf::Color color;
            switch (id) {
            case GROUND:
                color = sf::Color(51, 77, 77);
                break;
            case WALL:
                color = sf::Color(12, 26, 51);
                break;
            case FOOD:
                color = sf::Color(0, 255, 0);
                break;

                // temp colors like this for now
            case PLAYER:
                color = sf::Color(255, 127, 51);
                break;
            case PLAYER+1:
                color = sf::Color(255, 255, 51);
                break;
            default:
                color = sf::Color(255, 0, 255);
                break;
            }

            verts.append(sf::Vertex(sf::Vector2f(x0, y0), color));
            verts.append(sf::Vertex(sf::Vector2f(x1, y0), color));
            verts.append(sf::Vertex(sf::Vector2f(x1, y1), color));
            verts.append(sf::Vertex(sf::Vector2f(x0, y1), color));
        }
    }
}

int main() {

    Snake game;
    return 0;
}