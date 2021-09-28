#include "engine.h"
#include "twentyOne_client.h"

int main()
{
    twentyOne::TwentyOneClient client;
    twentyOne::TwentyOneView view(client);
    Engine engine(sf::Vector2i(480,480));
    engine.AddDrawImGuiSystem(&view);
    engine.AddSystem(&client);
    engine.Run();
    return 0;
}
