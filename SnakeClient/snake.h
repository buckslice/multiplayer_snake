// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis		13792598	gmachlis@uci.edu

#pragma once
#include "map.h"
#include "player.h"
#include <SFML/Network.hpp>

struct Gamestate {
	unsigned int gamestateID;
	std::vector<Player> playerVector;

	Gamestate(unsigned int id, std::vector<Player>& players) {
		gamestateID = id;
		playerVector = players;
	}

	friend bool operator==(const Gamestate& g1, const Gamestate& g2)
	{
		if (g1.gamestateID != g2.gamestateID)
		{
			return false;
		}
		for (size_t i = 0; i < g1.playerVector.size(); i++)
		{
			if (!(g1.playerVector[i] == g2.playerVector[i]))
			{
				return false;
			}
		}
		return true;
	}
};

class Snake {
public:
    Snake();

    const int WIDTH = 1000;
    const int HEIGHT = 600;
    const float TILE = 25.0f;

	// tile IDs
	static const int GROUND = 0;
	static const int WALL = 1;
	static const int FOOD = 2;
	static const int PLAYER = 3;
private:
    sf::RenderWindow* window;

    sf::TcpSocket socket;

    // which player you are in list of players
    int playerIndex = -1;

    // will be used as a holder for scores
    // and when getting input say players[playerIndex].getInput()
    std::vector<Player> players;
	unsigned int extraStateID;			// Counter for current gamestate
	unsigned int trueStateID;

	bool gameRunning = false;
	float gameTime = -1000.0f;		// current game time in seconds
	const float tickTime = 0.1f;	// defines how quickly game moves // should slow down when testing

	std::vector<Gamestate> gamestateVector;

    std::string titleText;
	std::string pingText;

	bool pingWait = false;
	unsigned pingTime;
	std::vector<unsigned> pingVector;

    Map map;
    sf::Font font;
    sf::Text text;

    void init();
	void gameTick();    // progresses state of game by one game tick
    void start();

    // returns false when server is disconnected
    bool checkServerMessages();
    void processPacket(sf::Packet& packet);

    void render();
    void generateVertices(sf::VertexArray& verts);

    unsigned timeSinceEpochMillis();

    bool hasConnectedBefore = false;
    
};
