#pragma once

#include <string>
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Socket.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include "twentyOne_settings.h"
#include "system.h"

namespace twentyOne
{

    class TwentyOneClient : public System
    {
    public:
        sf::Socket::Status Connect(sf::IpAddress address, unsigned short portNumber);
        TwentyOnePhase GetPhase() const;
        bool IsConnected() const;
        void Init() override;
        void ReceivePacket(sf::Packet& packet);
        void Update() override;
        void Destroy() override;
        int GetPlayerNumber() const;
        void SendNewRoll();
        std::string_view GetEndMessage() const;
    private:
        sf::TcpSocket socket_;
        TwentyOnePhase phase_ = TwentyOnePhase::CONNECTION;
        std::string endMessage_;
        PlayerNumber playerNumber_ = 255u;
    };


    class TwentyOneView : public DrawImGuiInterface
    {
    public:
        TwentyOneView(TwentyOneClient& client);
        void DrawImGui() override;


        TwentyOneClient& client_;
        std::string ipAddressBuffer_ = "localhost";
        int portNumber_ = serverPortNumber;

        sf::Vector2i windowSize_;
    };
}