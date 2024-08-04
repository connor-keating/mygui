#pragma once

#pragma warning(push, 0)
#include <windows.h>
#include <stdio.h>
#pragma warning(pop)

#include "application.h"
#include "types.h"
#include "win32_opengl.h"

#if _DEBUG_BUILD
    #define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
    #else
    // If not debugging, this expands to nothing, so there is no cost of leaving this in your production build
    #define Assert(Expression)
#endif

typedef struct window_info window_info;
struct window_info
{
    DWORD style_extended; // has list of possible values.
    LPCWSTR class_name; // null-terminated string.
    LPCWSTR name; // string window name to display in title bar.
    DWORD style_basic; // has list of possible values.
    i32 position_x; // starting X position on monitor.
    i32 position_y; // starting Y position on monitor.
    i32 width;
    i32 height;
    HWND window_parent; // Handle to the parent or owner window being created.
    HMENU window_menu; // optional a child-wndow ID.
    HINSTANCE window_handle; // handle to this window's module.
    LPVOID window_data_pointer; // optional CREATESTRUCT data for window.
};

typedef struct window_shape window_shape;
struct window_shape
{
    i32 width;
    i32 height;
};
