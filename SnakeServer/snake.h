// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis		13792598	gmachlis@uci.edu

#pragma once
#include <SFML/Network.hpp>
#include <queue>
#include <random>
#include "map.h"
#include "player.h"
#include <unordered_map>

struct GameState {
    unsigned int gameFrame;
    std::vector<Player> playerVector;
    // should add food into this too

    GameState(unsigned int id, std::vector<Player>& players) {
        gameFrame = id;
        playerVector = players;
    }

    friend bool operator==(const GameState& g1, const GameState& g2) {
        if (g1.gameFrame != g2.gameFrame) {
            return false;
        }
        for (size_t i = 0; i < g1.playerVector.size(); i++) {
            if (!(g1.playerVector[i] == g2.playerVector[i])) {
                return false;
            }
        }
        return true;
    }
};

struct DelayedPacket {
    sf::Packet packet;
    int clientIndex;
    unsigned readyTime;

    DelayedPacket(sf::Packet p, int i, unsigned t) {
        packet = p;
        clientIndex = i;
        readyTime = t;
    }
};

struct PlayerInput {
    point inone;
    point intwo;
};

class Snake {
public:
    Snake();
    ~Snake();

    // size of window in pixels
    const int WIDTH = 1000;
    const int HEIGHT = 600;

    const float TILE = 25.0f;   // size of each tile in pixels

    // tile IDs
    static const int GROUND = 0;
    static const int WALL = 1;
    static const int FOOD = 2;
    static const int PLAYER = 3;
private:
    sf::RenderWindow* window;

    sf::TcpListener listener;  // network listener

    // client position in list are ordered by longest time connected
    std::vector<sf::TcpSocket*> clients;

    // client[0] is always player[0] and so on, but ids may mismatch depending on disconnects
    // client 1 might be player 2 for example
    std::vector<Player> players;

    Map map;        // holds tile data, used for rendering and collision testing
    point foodPos;  // where the current food on the map is 

    sf::Font font;
    sf::Text text;

    void init();    // server set up
    void start();   // main loop 

    void checkNewConnections(); // listens for new client connections
    void checkClientMessages(); // checks messages from client sockets
    void processPacket(sf::Packet& packet, int index);  // processes a packet from client at index

    int winner; // current state of game (0 means game still going, -1 is draw, > 0 means player n has won)
    bool gameRunning = false;  
    float gameTime = -1000.0f;     // current game time in seconds
    unsigned serverFrame = -1;
    const float tickTime = 0.1f;  // defines how quickly game moves // should slow down when testing

    void gameTick();    // progresses state of game by one game tick

    void broadcastGameState();
    void broadcastPacket(sf::Packet& packet);
    void sendPacket(sf::Packet& packet, int clientIndex);

    void checkAndSendDelayed(); // checks delayedQueue and sends packets that have waited long enough
    void checkAndReceiveDelayed();

    // latency simulation
    bool addSendLatency = true;
    bool addReceiveLatency = false;

    std::vector<DelayedPacket> delaySendList;
    std::vector<DelayedPacket> delayReceivedList;

    std::vector<GameState> previousStates;
    std::vector<std::unordered_map<unsigned, PlayerInput>> inputs;
    unsigned receivedInputs = 0;
    unsigned oldestFrame = -1;
    unsigned lastGameStartTime;
    void resimulateGameToPresentState();

    std::vector<point> clientDelays;
    unsigned getDelay(int index); 

    std::mt19937 rng;    

    int getWinner();    // sets winner
    void startGame(float delay);    // resets game to start state with a given delay

    std::string getTitle(); // figures out the title from gametime and winner
    void render();  // draws game to window
    void generateVertices(sf::VertexArray& verts); // builds vertex array from map data

    unsigned timeSinceEpochMillis();

};
