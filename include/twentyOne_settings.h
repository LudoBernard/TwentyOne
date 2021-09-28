#pragma once
#include <SFML/System/Vector2.hpp>

namespace twentyOne
{
constexpr int maxClientNmb = 2;
constexpr unsigned short serverPortNumber = 44444;
enum class TwentyOnePhase
{
    CONNECTION,
    GAME,
    END
};

using PlayerNumber = unsigned char;
}
