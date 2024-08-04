#pragma once

#include "types.h"

#define GAME_NAME L"MyGUI"
#define GAME_WIDTH 640
#define GAME_HEIGHT 360

typedef struct game_screen game_screen;
struct game_screen
{
    fvec2 resolution;
    fvec2 position;
};
