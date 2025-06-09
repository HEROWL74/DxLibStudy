#include "Game.h"

int WINAPI WinMain(_In_ HINSTANCE,_In_opt_ HINSTANCE,_In_ LPSTR,_In_ int)
{
    Game game;
    if (!game.Initialize()) return -1;
    game.Run();
    game.Finalize();
    return 0;
}
