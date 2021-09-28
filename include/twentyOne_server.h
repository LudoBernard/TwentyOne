#pragma once

#include <array>
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <twentyOne_settings.h>

#include "twentyOne_packet.h"

namespace twentyOne
{

class TwentyOneServer
{
public:
    int Run();
private:
    std::array<sf::TcpSocket, maxClientNmb> sockets_;
    sf::SocketSelector selector_;
    sf::TcpListener listener_;
    TwentyOnePhase phase_ = TwentyOnePhase::CONNECTION;
    int currentDiceIndex_ = 0;
    int sum = 0;
    bool triggerWinThisRound_ = false;

    void StartNewGame();
    void UpdateConnectionPhase();
    void ManageFoldPacket(const FoldPacket& foldPacket);
    void ReceivePacket();
    PlayerNumber CheckWinner() const;
    void ManageRollPacket(const RollPacket& rollPacket);

    int GetNextSocket();
};
}
