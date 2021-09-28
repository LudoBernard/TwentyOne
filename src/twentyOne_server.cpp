#include "twentyOne_server.h"
#include <SFML/Network/TcpSocket.hpp>

#include <iostream>
#include <random>

#include "twentyOne_packet.h"

namespace twentyOne
{
    void TwentyOneServer::ReceivePacket()
    {
        if (selector_.wait(sf::milliseconds(20)))
        {
            for (auto& socket : sockets_)
            {
                if (selector_.isReady(socket))
                {
                    sf::Packet receivedPacket;
                    sf::Socket::Status receivingStatus;
                    do
                    {
                        receivingStatus = socket.receive(receivedPacket);
                    } while (receivingStatus == sf::Socket::Partial);

                    Packet statusPacket;
                    receivedPacket >> statusPacket;
                    switch (static_cast<PacketType>(statusPacket.packetType))
                    {
                    case PacketType::ROLL_DIE:
                    {
                        RollPacket rollPacket;
                        receivedPacket >> rollPacket;
                        ManageRollPacket(rollPacket);
                        break;
                    }
                    case PacketType::FOLD:
                    {
                        FoldPacket foldPacket;
                        receivedPacket >> foldPacket;
                        ManageFoldPacket(foldPacket);
						break;
                    }
                    }

                }
            }
        }
    }

    void TwentyOneServer::ManageRollPacket(const RollPacket& rollPacket)
    {   
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distribution(1, 6);
        int roll = distribution(gen);
    	
        std::cout << "Received packet from player " << rollPacket.playerNumber + 1 <<
            ". They rolled a " << roll << " !" << '\n';

        if (phase_ != TwentyOnePhase::GAME)
            return;

        if (currentDiceIndex_ % 2 != rollPacket.playerNumber)
        {
            //TODO return to player an error msg
            return;
        }

        currentDiceIndex_++;
        sum += roll;

    	
        EndType endType = EndType::NONE;
        
        if (triggerWinThisRound_ && rollPacket.playerNumber == 0)
        {
            endType = EndType::WIN_P1;
        }
        else if (triggerWinThisRound_ && rollPacket.playerNumber == 1)
        {
            endType = EndType::WIN_P2;
        }
    	
        //TODO check victory condition
    	if(sum>21 && rollPacket.playerNumber == 0)
    	{
            endType = EndType::WIN_P2;
    	}
        else if(sum > 21 && rollPacket.playerNumber == 1)
        {
            endType = EndType::WIN_P1;
        }
        else if(sum == 21 && rollPacket.playerNumber == 0)
        {
            endType = EndType::WIN_P1;
        }
        else if (sum == 21 && rollPacket.playerNumber == 1)
        {
            endType = EndType::WIN_P2;
        }

    	
        
        RollPacket newRollPacket = rollPacket;
        newRollPacket.packetType = PacketType::ROLL_DIE;

        //sent new move to all players
        for(auto& socket: sockets_)
        {
            sf::Packet sentPacket;
            newRollPacket.roll = roll;
            sentPacket << newRollPacket;
            sf::Socket::Status sentStatus;
            do
            {
                sentStatus = socket.send(sentPacket);
            } while (sentStatus == sf::Socket::Partial);
            
        }
        //send end of game packet
        if (endType != EndType::NONE)
        {
            EndPacket endPacket{};
            endPacket.packetType = PacketType::END;
            endPacket.endType = endType;

            //sent new move to all players
            for (auto& socket : sockets_)
            {
                sf::Packet sentPacket;
                sentPacket << endPacket;
                sf::Socket::Status sentStatus;
                do
                {
                    sentStatus = socket.send(sentPacket);
                } while (sentStatus == sf::Socket::Partial);

            }

            phase_ = TwentyOnePhase::END;
        }
    }

    int TwentyOneServer::Run()
    {
        if (listener_.listen(serverPortNumber) != sf::Socket::Done)
        {
            std::cerr << "[Error] Server cannot bind port: " << serverPortNumber << '\n';
            return EXIT_FAILURE;
        }
        std::cout << "Server bound to port " << serverPortNumber << '\n';

        while (true)
        {
            switch (phase_)
            {
            case TwentyOnePhase::CONNECTION:
                ReceivePacket();
                UpdateConnectionPhase();
                break;
            case TwentyOnePhase::GAME:
                ReceivePacket();
                break;
            case TwentyOnePhase::END:
                return EXIT_SUCCESS;
            default:;
            }
        }
    }

    void TwentyOneServer::StartNewGame()
    {
        //Switch to Game state
        phase_ = TwentyOnePhase::GAME;
        //Send game init packet
        std::cout << "Two players connected!\n";

        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(0, 1);
        int dice_roll = distribution(generator);

        for (unsigned char i = 0; i < sockets_.size(); i++)
        {
            GameInitPacket gameInitPacket{};
            gameInitPacket.packetType = PacketType::GAME_INIT;
            gameInitPacket.playerNumber = i != dice_roll;
            sf::Packet sentPacket;
            sentPacket << gameInitPacket;
            sf::Socket::Status sentStatus;
            do
            {
                sentStatus = sockets_[i].send(sentPacket);
            } while (sentStatus == sf::Socket::Partial);
        }
    }

    void TwentyOneServer::UpdateConnectionPhase()
    {
        // accept a new connection
        const auto nextIndex = GetNextSocket();

        if (nextIndex != -1)
        {
            auto& newSocket = sockets_[nextIndex];
            if (listener_.accept(newSocket) == sf::Socket::Done)
            {
                std::cout << "New connection from " <<
                    newSocket.getRemoteAddress().toString() << ':' << newSocket.
                    getRemotePort() << '\n';
                newSocket.setBlocking(false);
                selector_.add(newSocket);
                if (nextIndex == 1)
                {
                    StartNewGame();

                }
            }
        }
    }

    void TwentyOneServer::ManageFoldPacket(const FoldPacket& foldPacket)
    {
        std::cout << "Received packet from player " << foldPacket.playerNumber + 1 <<
            ". They folded!" << '\n';

        if (phase_ != TwentyOnePhase::GAME)
            return;

        if (currentDiceIndex_ % 2 != foldPacket.playerNumber)
        {
            //TODO return to player an error msg
            return;
        }

        currentDiceIndex_++;

        EndType endType = EndType::NONE;

        FoldPacket newFoldPacket = foldPacket;
        newFoldPacket.packetType = PacketType::FOLD;

        if (triggerWinThisRound_)
        {
            endType = EndType::STALEMATE;
        }

        triggerWinThisRound_ = true;

       
        //sent new move to all players
        for (auto& socket : sockets_)
        {
            sf::Packet sentPacket;
            sentPacket << newFoldPacket;
            sf::Socket::Status sentStatus;
            do
            {
                sentStatus = socket.send(sentPacket);
            } while (sentStatus == sf::Socket::Partial);

        }
        //send end of game packet
        if (endType != EndType::NONE)
        {
            EndPacket endPacket{};
            endPacket.packetType = PacketType::END;
            endPacket.endType = endType;

            //sent new move to all players
            for (auto& socket : sockets_)
            {
                sf::Packet sentPacket;
                sentPacket << endPacket;
                sf::Socket::Status sentStatus;
                do
                {
                    sentStatus = socket.send(sentPacket);
                } while (sentStatus == sf::Socket::Partial);

            }

            phase_ = TwentyOnePhase::END;
        }
    }

    int TwentyOneServer::GetNextSocket()
    {
        for (int i = 0; i < maxClientNmb; i++)
        {
            if (sockets_[i].getLocalPort() == 0)
            {
                return i;
            }
        }
        return -1;
    }
}
