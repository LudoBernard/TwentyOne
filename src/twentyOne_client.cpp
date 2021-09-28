#include "twentyOne_client.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/Socket.hpp>

#include "twentyOne_packet.h"

namespace twentyOne
{
    sf::Socket::Status TwentyOneClient::Connect(sf::IpAddress address, unsigned short portNumber)
    {
        const auto status = socket_.connect(address, portNumber);
        socket_.setBlocking(false);
        return status;
    }

    TwentyOnePhase TwentyOneClient::GetPhase() const
    {
        return phase_;
    }

    bool TwentyOneClient::IsConnected() const
    {
        return socket_.getLocalPort() != 0;
    }

    void TwentyOneClient::Init()
    {
    }

    void TwentyOneClient::ReceivePacket(sf::Packet& packet)
    {
        Packet twentyOnePacket{};
        packet >> twentyOnePacket;

        switch (static_cast<PacketType>(twentyOnePacket.packetType))
        {
        case PacketType::GAME_INIT:
        {
            GameInitPacket gameInitPacket{};
            packet >> gameInitPacket;
            playerNumber_ = gameInitPacket.playerNumber;
            phase_ = TwentyOnePhase::GAME;
            std::cout << "You are player " << gameInitPacket.playerNumber + 1 << '\n';
            break;
        }
        case PacketType::ROLL_DIE:
        {
            if (phase_ != TwentyOnePhase::GAME)
                break;
            RollPacket rollPacket{};
            packet >> rollPacket;
            std::cout << "Receive roll packet from player " <<
                rollPacket.playerNumber + 1 << ". They rolled a " << rollPacket.roll << " !" << '\n';
            break;
        }
        case PacketType::END:
        {
            if (phase_ != TwentyOnePhase::GAME)
            {
                break;
            }
            EndPacket endPacket;
            packet >> endPacket;
            switch (endPacket.endType)
            {
            case EndType::STALEMATE:
                endMessage_ = "Stalemate";
                break;
            case EndType::WIN_P1:
                endMessage_ = playerNumber_ == 0 ? "You won" : "You lost";
                break;
            case EndType::WIN_P2:
                endMessage_ = playerNumber_ == 1 ? "You won" : "You lost";
                break;
            case EndType::ERROR:
                endMessage_ = "Error";
                break;
            default:;
            }
            phase_ = TwentyOnePhase::END;
            break;
        }
        	default:
                break;
        }
    }

    void TwentyOneClient::Update()
    {
        //Receive packetS
        if (socket_.getLocalPort() != 0)
        {
            sf::Packet receivedPacket;
            sf::Socket::Status status;
            do
            {
                status = socket_.receive(receivedPacket);
            } while (status == sf::Socket::Partial);

            if (status == sf::Socket::Done)
            {
                ReceivePacket(receivedPacket);
            }

            if (status == sf::Socket::Disconnected)
            {
                socket_.disconnect();
                std::cerr << "Server disconnected\n";
            }
        }
    }

    void TwentyOneClient::Destroy()
    {
    }

    int TwentyOneClient::GetPlayerNumber() const
    {
        return playerNumber_;
    }

    void TwentyOneClient::SendNewRoll()
    {
        RollPacket rollPacket{};
        rollPacket.packetType = PacketType::ROLL_DIE;
        rollPacket.playerNumber = playerNumber_;
        sf::Packet packet;
        packet << rollPacket;
        sf::Socket::Status sentStatus;
        do
        {
            sentStatus = socket_.send(packet);
        } while (sentStatus == sf::Socket::Partial);
    }


    std::string_view TwentyOneClient::GetEndMessage() const
    {
        return endMessage_;
    }

    TwentyOneView::TwentyOneView(TwentyOneClient& client) : client_(client)
    {
    }

    void TwentyOneView::DrawImGui()
    {
        switch (client_.GetPhase())
        {
        case TwentyOnePhase::CONNECTION:
        {
            if (client_.IsConnected())
                return;
            ImGui::Begin("Client");

            ImGui::InputText("Ip Address", &ipAddressBuffer_);
            ImGui::InputInt("Port Number", &portNumber_);
            if (ImGui::Button("Connect"))
            {
                const auto status = client_.Connect(sf::IpAddress(ipAddressBuffer_), portNumber_);
                if (status != sf::Socket::Done)
                {
                    switch (status)
                    {
                    case sf::Socket::NotReady:
                        std::cerr << "Not ready to connect to " << ipAddressBuffer_ << ':' << portNumber_ << '\n';
                        break;
                    case sf::Socket::Partial:
                        std::cerr << "Connecting to " << ipAddressBuffer_ << ':' << portNumber_ << '\n';
                        break;
                    case sf::Socket::Disconnected:
                        std::cerr << "Disconnecting to " << ipAddressBuffer_ << ':' << portNumber_ << '\n';
                        break;
                    case sf::Socket::Error:
                        std::cerr << "Error connecting to " << ipAddressBuffer_ << ':' << portNumber_ << '\n';
                        break;
                    default:;
                    }
                }
                else
                {
                    std::cout << "Successfully connected to server\n";
                }

            }
            ImGui::End();
            break;
        }


        	
        case TwentyOnePhase::END:
        {
            ImGui::Begin("Client");
            ImGui::Text("%s", client_.GetEndMessage().data());
            ImGui::End();
            break;
        }
        case TwentyOnePhase::GAME:
        {
            const auto playerNumber = client_.GetPlayerNumber();
            ImGui::Begin("Client");
            ImGui::Text("You are player %d", playerNumber);
        	if(ImGui::Button("Roll die"))
        	{
                client_.SendNewRoll();
        	}
            ImGui::End();
			break;
        }
        }



    }

}
