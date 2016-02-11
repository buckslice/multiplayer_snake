// ICS 167 Multiplayer Snake Project by:
// Matt Ruiz        28465978    mpruiz@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// John Collins     75665849    jfcollin@uci.edu

#include <SFML/Network.hpp>
#include <iostream>
#include <sstream>
#include <vector>

int main() {
    sf::TcpListener listener;

    int port;
    std::cout << "GIME PORT: ";
    std::cin >> port;

    // bind the listener to a port
    if (listener.listen(port) != sf::Socket::Done) {
        std::cout << "CANT LISTEN M8" << std::endl;
    }

    // wait for new connection
    sf::TcpSocket client;
    if (listener.accept(client) != sf::Socket::Done) {
        std::cout << "NO CLIENT" << std::endl;
    } else {
        std::cout << "CLIENT CONNECTED!" << std::endl;
    }

    char in[128];
    std::size_t received;

    // later find number of players
    std::vector<int> scores(2);

    while (true) {
        if (client.receive(in, sizeof(in), received) != sf::Socket::Done) {
            return 0;
        }

        std::string msg = std::string(in);
        if (msg == "0") {
            for (size_t i = 0; i < scores.size(); ++i) {
                scores[i] = 0;
            }
        } else {
            // add one to players score
            int player = std::atoi(msg.c_str()) - 1;
            scores[player]++;

            // send back score vector
            std::ostringstream oss;
            for (size_t i = 0; i < scores.size(); ++i) {
                oss << scores[i];
            }

            std::string s = oss.str();
            client.send(s.c_str(), s.length());
        }

        std::cout << "Answer received from the client: \"" << in << "\"" << std::endl;

        // empty our message
        int len = sizeof(in) / sizeof(char);
        for (int i = 0; i < len; ++i) {
            in[i] = '\0';
        }

    }

    system("pause");

    return 0;
}