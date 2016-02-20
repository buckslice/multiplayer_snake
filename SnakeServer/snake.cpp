// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include "snake.h"

Snake::Snake() {
    init();
    start();
}

Snake::~Snake() {
    listener.close();

    // delete client sockets pointers
    for (size_t i = 0, len = clients.size(); i < len; ++i) {
        clients[i]->disconnect();
        delete clients[i];
    }
    clients.erase(clients.begin(), clients.end());

    delete window;
}


void Snake::init() {
    // get port number
    std::string port;
    std::cout << "Enter port number: ";
    std::getline(std::cin, port);
    if (port == "") {
        port = "8000";
        std::cout << "defaulting to port: " << port << std::endl;
    } else {
        std::cout << port << std::endl;
    }

    // bind the listener to a port
    listener.setBlocking(false);
    if (listener.listen(std::atoi(port.c_str())) != sf::Socket::Done) {
        std::cout << "CANT LISTEN AT THIS PORT" << std::endl;
    }

    std::cout << "SERVER STARTED..." << std::endl;

    // build window to use for game loop and for rendering game state
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;
    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Snake Server", sf::Style::Default, settings);
    window->setFramerateLimit(60);

    // load font
    if (!font.loadFromFile("OCRAEXT.TTF")) {
        std::cout << "FAILED LOADING FONT" << std::endl;
    }
    text.setFont(font);
    text.setCharacterSize(30);

    // set up map
    map = Map((int)(WIDTH / TILE), (int)(HEIGHT / TILE) - 1);
    map.pos = { 0,(int)TILE };
    map.generate();

}

void Snake::checkNewConnections() {
    // always keep one open connection at end of client list that listens for new connections
    if (clients.size() == players.size()) {
        sf::TcpSocket* client = new sf::TcpSocket();
        client->setBlocking(false);
        clients.push_back(client);
    }

    // if successfully get new connection then add player to list
    if (listener.accept(*clients[players.size()]) == sf::Socket::Done) {

        // find a free id for new player
        int freeID = 0;
        bool foundFree = false;
        while (!foundFree) {
            foundFree = true;
            for (size_t i = 0; i < players.size(); ++i) {
                if (players[i].id == freeID) {
                    freeID++;
                    foundFree = false;
                }
            }
        }

        players.push_back(Player(freeID));    // player ids are 0 indexed
        std::cout << "New client connected, set as player " << players.size() << std::endl;

        // start game if enough players
        if (players.size() >= 2) {
            startGame(2.0f);
        }
    }
}

void Snake::checkClientMessages() {
    // iterate over each connected client and check for received messages
    // skip last element of clients since that is an open socket waiting for a new connection
    for (size_t i = 0, len = clients.size() - 1; i < len; ++i) {
        sf::Packet packet;

        // loop until all packets are processed
        while (true) {
            // get packet from client socket
            sf::Socket::Status status = clients[i]->receive(packet);

            // if no data from the client this frame or some sort of error then continue (by breaking infinite while)
            if (status == sf::Socket::NotReady || status == sf::Socket::Error) {
                break;
            } else if (status == sf::Socket::Disconnected) {    // if client disconnected
                std::cout << "Client " << (i + 1) << " disconnected" << std::endl;

                // erase client info from vectors
                delete(clients[i]);
                clients.erase(clients.begin() + i);
                players.erase(players.begin() + i);

                if (players.size() < 2) {
                    // resets game while waiting
                    gameTime = -2.0f;
                    gameRunning = false;
                    winner = 0;
                    map.generate();
                }
                break;
            }
            // reaching this point means data was successfully received

            processPacket(packet, i);
        }
    }
}

void Snake::processPacket(sf::Packet& packet, int index) {
    sf::Uint8 b;
    packet >> b;
    if (b == 0) {
        packet >> players[index].inone;
        packet >> players[index].intwo;
    } else if (b == 1) {    // if type == 1
        std::string s;
        packet >> s;

        std::cout << s << std::endl;
    } else {
        std::cout << "Unknown packet type received from client" << std::endl;
    }
}

void Snake::broadcastPacket(sf::Packet& packet) {
    for (size_t i = 0; i < clients.size() - 1; ++i) {
        clients[i]->send(packet);
    }
}

void Snake::broadcastGameState() {
    // PACKET LAYOUT
    // type
    // number of players
      // for each player
        // player id
        // player score
        // player dir, and inputs
        // length of snake
        // points in snake
    // food pos
    // id of client in player list

    sf::Packet packet;

    packet << (sf::Uint8) 0;
    packet << (sf::Uint8) players.size();
    for (size_t i = 0; i < players.size(); ++i) {
        Player& player = players[i];
        packet << (sf::Uint8)player.id;
        packet << (sf::Uint8)player.score;
        packet << player.dir;
        packet << player.inone;
        packet << player.intwo;

        auto& points = player.getPoints();
        packet << (sf::Uint8) points.size();

        for (size_t j = 0; j < points.size(); ++j) {
            packet << points[j];
        }

    }
    packet << foodPos;

    // send game state to each client
    for (size_t i = 0; i < clients.size() - 1; ++i) {
        sf::Packet clientPacket = packet;
        clientPacket << (sf::Uint8) i;
        clients[i]->send(clientPacket);
    }
}

// progress state of game by one move
void Snake::gameTick() {
    for (size_t i = 0; i < players.size(); ++i) {
        // figure out dir
        Player& p = players[i];
        point mv = p.getMove();
        if (map.isWalkable(mv.x, mv.y)) {   // valid move
            point oldend = p.move();
            map.setTile(oldend, GROUND);
            map.setTile(p.getPos(), p.id + PLAYER);
        } else if (map.getTile(mv.x, mv.y) == FOOD) {   // ran into food
            point oldend = p.move();
            map.setTile(oldend, GROUND);
            map.setTile(p.getPos(), p.id + PLAYER);
            p.grow(3);
            p.score++;

            foodPos = map.spawnRandom(FOOD);
        } else {    // hit wall or part of a snake so this player dies!
            p.dead = true;
        }
    }
}

void Snake::start() {
    sf::Clock frameTime;
    bool running = true;
    bool paused = false;
    while (running) {
        // increment gameTime
        float delta = frameTime.restart().asSeconds();

        // process window events
        sf::Event e;
        while (window->pollEvent(e)) {
            switch (e.type) {
            case sf::Event::Closed:
                running = false;
                break;
            case sf::Event::KeyPressed:
                if (e.key.code == sf::Keyboard::Escape) {
                    running = false;
                } else if (e.key.code == sf::Keyboard::Space) {
                    paused = !paused;
                }
            default:
                break;
            }
        }
        if (paused) {
            render();
            continue;
        }

        checkNewConnections();

        checkClientMessages();

        // send packet to clients to update title message
        sf::Packet titlePacket;
        titlePacket << (sf::Uint8) 1;   // message type
        titlePacket << getTitle();
        broadcastPacket(titlePacket);

        if (gameRunning) {
            gameTime += delta;
            // if enough time has passed then do a game tick
            if (gameTime >= tickTime) {
                gameTick();
                gameTime = 0.0f;

                winner = getWinner();
                if (winner != 0) {
                    // set to -2 so can have 1 second to see result
                    // before game is reset down below
                    gameTime = -2.0f;
                }

                // send game state back to clients
                broadcastGameState();
            }

            // if game ended then restart after delay
            if (winner != 0 && gameTime > -1.0f) {
                startGame(1.0f);
                broadcastGameState();
            }
        }

        // render board and limits framerate
        render();
    }

}

void Snake::startGame(float delay) {
    std::cout << "Starting new game!" << std::endl;
    gameRunning = true;
    gameTime = -fabs(delay);
    winner = 0;

    map.generate();

    // spawn players
    for (size_t i = 0; i < players.size(); ++i) {
        Player& p = players[i];
        switch (p.id) {    // spawn points hardcoded for 2 players for now
        case 0:
            p.spawn(4, 4, 0, 1);
            break;
        case 1:
            p.spawn(map.getW() - 5, map.getH() - 5, 0, -1);
            break;
        default:
            std::cout << "spawning player " << (i + 1) << " at default spawn" << std::endl;
            p.spawn(map.getW() / 2, map.getH() / 2, 1, 0);
            break;
        }
        map.setTile(p.getPos(), p.id + PLAYER);
    }

    foodPos = map.spawnRandom(FOOD);

}


// return 0 means game still going
// return -1 means draw
// return > 0 means player n won
int Snake::getWinner() {
    bool oneAlive = false;
    int winner = 0;
    for (size_t i = 0; i < players.size(); ++i) {
        if (!players[i].dead) {
            if (oneAlive) { // if another player has been found alive then return -1 cuz game isnt over
                return 0;
            }
            winner = i + 1;
            oneAlive = true;
        }
    }
    if (!oneAlive) {  // this means it was a draw
        return -1;
    }

    return winner;
}

// returns what the title should be based off gametime and if a winner has been found
std::string Snake::getTitle() {
    std::ostringstream oss;
    if (gameTime < -1.0f) {
        if (winner < 0) {
            oss << "DRAW!";
        } else if (winner == 0) {
            oss << "WAITING...";
        } else {
            oss << "Player " << winner << " wins!";
        }
    } else if (gameTime < 0.0f) {
        oss << "GET READY";
    } else {
        oss << "SNAKE!";
    }
    return oss.str();
}

void Snake::render() {
    // clear the window to same color as wall tile
    window->clear(sf::Color(12, 26, 51));

    sf::VertexArray verts;
    verts.setPrimitiveType(sf::PrimitiveType::Quads);

    generateVertices(verts);

    window->draw(verts);

    // draw player text (currently hardcoded for two players)
    for (size_t i = 0; i < players.size(); ++i) {
        std::ostringstream oss;

        Player& p = players[i];
        oss << "P " << (p.id + 1) << " : " << p.score << " ";
        text.setString(oss.str());
        text.setColor(Player::getColorFromID(p.id));
        // should change this if here to just something like this below
        // text.setPosition(sf::Vector2f(i * WIDTH/numPlayers, 0.0f));
        if (i == 0) {
            text.setPosition(sf::Vector2f(0.0f, 0.0f));
        } else {
            sf::FloatRect fr = text.getLocalBounds();
            text.setPosition(sf::Vector2f(WIDTH - fr.width, 0.0f));
        }
        window->draw(text);
    }

    // draw title text
    text.setColor(sf::Color(200, 255, 255));
    text.setString(getTitle());
    sf::FloatRect fr = text.getLocalBounds();
    text.setPosition(sf::Vector2f(WIDTH / 2 - fr.width / 2.0f, 0.0f));
    window->draw(text);

    // swap buffers
    window->display();
}

// builds vertex array from map
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

            default:
                color = Player::getColorFromID(id - PLAYER);
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
    Snake server;
    //std::cout << std::endl << "SERVER STOPPED..." << std::endl;
    //system("pause");
    return 0;
}