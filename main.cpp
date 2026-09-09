/*
    main.cpp - the entry point.

    All the real work lives in Game, so main only has to create one board-sized
    match and hand control over to it.
*/
#include "Game.h"

int main() {
    Game game(8, 8);
    game.run();
    return 0;
}
