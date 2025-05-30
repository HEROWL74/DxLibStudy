#include "Game.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    Game game;
    if (!game.Initialize()) return -1;
    game.Run();
    game.Finalize();
    return 0;
}
