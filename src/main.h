#pragma once

#pragma warning(push, 0)
#include <windows.h>
#include <stdio.h>
#pragma warning(pop)

#include "types.h"
#include "win32_opengl.h"

#if _DEBUG_BUILD
    #define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
    #else
    // If not debugging, this expands to nothing, so there is no cost of leaving this in your production build
    #define Assert(Expression)
#endif

typedef struct window_shape window_shape;
struct window_shape
{
    i32 width;
    i32 height;
};
