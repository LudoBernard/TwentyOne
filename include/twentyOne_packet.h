#pragma once

#include <twentyOne_settings.h>
#include <SFML/Network/Packet.hpp>
#include <SFML/System/Vector2.hpp>

namespace twentyOne
{
    enum class PacketType : unsigned char
    {
        GAME_INIT,
    	ROLL_DIE,
    	FOLD,
        END
    };

    struct Packet
    {
        PacketType packetType;
    };

    inline sf::Packet& operator <<(sf::Packet& packet, const Packet& twentyOnePacket)
    {
        const auto packetType = static_cast<unsigned char>(twentyOnePacket.packetType);
        return packet << packetType;
    }

    inline sf::Packet& operator >>(sf::Packet& packet, Packet& twentyOnePacket)
    {
        unsigned char packetType;
        packet >> packetType;
        twentyOnePacket.packetType = static_cast<PacketType>(packetType);
        return packet;
    }

    struct GameInitPacket : Packet
    {
        PlayerNumber playerNumber;
    };

    inline sf::Packet& operator <<(sf::Packet& packet, const GameInitPacket& gameInitPacket)
    {
        return packet << static_cast<unsigned char>(gameInitPacket.packetType)
        << gameInitPacket.playerNumber;
    }

    inline sf::Packet& operator >>(sf::Packet& packet, GameInitPacket& gameInitPacket)
    {
        return packet >> gameInitPacket.playerNumber;
    }

    struct RollPacket : Packet
    {
        PlayerNumber playerNumber;
        int roll;
    };

    inline sf::Packet& operator <<(sf::Packet& packet, const RollPacket& rollPacket)
    {
        return packet << static_cast<unsigned char>(rollPacket.packetType) << rollPacket.playerNumber << rollPacket.roll;
    }

    inline sf::Packet& operator >>(sf::Packet& packet, RollPacket& rollPacket)
    {
        return packet >> rollPacket.playerNumber >> rollPacket.roll;
    }

    struct FoldPacket : Packet
    {
        PlayerNumber playerNumber;
    };

    inline sf::Packet& operator <<(sf::Packet& packet, const FoldPacket& foldPacket)
    {
        return packet << static_cast<unsigned char>(foldPacket.packetType) << foldPacket.playerNumber;
    }

    inline sf::Packet& operator >>(sf::Packet& packet, FoldPacket& foldPacket)
    {
        return packet >> foldPacket.playerNumber;
    }

    enum class EndType : unsigned char
    {
        NONE,
        STALEMATE,
        WIN_P1,
        WIN_P2,
        ERROR
    };

    struct EndPacket : Packet
    {
        EndType endType;
    };

    inline sf::Packet& operator <<(sf::Packet& packet, const EndPacket& endPacket)
    {
        const auto endType = static_cast<unsigned char>(endPacket.endType);
        return packet << static_cast<unsigned char>(endPacket.packetType)
            << endType;
    }

    inline sf::Packet& operator >>(sf::Packet& packet, EndPacket& endPacket)
    {
        unsigned char endType = 0;
        packet >> endType;
        endPacket.endType = static_cast<EndType>(endType);
        return packet;
    }
}
