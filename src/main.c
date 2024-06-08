#pragma once

#include "main.h"

#define GAME_NAME L"MyGUI"
bool32 RUNNING_GAME; 

LRESULT CALLBACK UnicodeWindowsProcedure(HWND windowHandle, UINT messageID, WPARAM wParam, LPARAM lParam)
{ 
    // Characters are coming in Unicode 
    // lstrcmpW(L"Q", (LPCWSTR) wParam)
    LRESULT result = 0;
    switch (messageID) 
    { 
        case WM_CLOSE:
        {
            OutputDebugStringW(L"Closing time!\n");
            RUNNING_GAME = false;
            PostQuitMessage(0);
            break;
        }
        case WM_SIZE:
            // Set the size and position of the window. 
            // RECT windowDimensions = {0};
            // GetClientRect(windowHandle, &windowDimensions); 
            // opengl_resize(windowDimensions);
        default: 
            result = DefWindowProcW(windowHandle, messageID, wParam, lParam); 
    }
    return result;
} 


static void win32_message_procedure(HWND windowHandle)
{
    MSG osMessage = {0};
    bool32 foundMessage = true;
    while (foundMessage)
    {
        foundMessage = PeekMessageW(&osMessage, windowHandle, 0, 0, PM_REMOVE);
        switch (osMessage.message)
        {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 VKCode = (u32) osMessage.wParam;
                bool32 altKeyValue =  (1 << 29);
                bool32 wasDownValue = (1 << 30);
                bool32 isDownValue =  (1 << 31);

                // if both of these int32s have a 1 bit in the same position the result will also have that bit set
                bool32 wasDownBitCheck = (osMessage.lParam & wasDownValue);
                bool32 isDownBitCheck  = (osMessage.lParam & isDownValue);
                bool32 altKeyBitCheck  = (osMessage.lParam & altKeyValue);

                bool32 WasDown = (wasDownBitCheck != 0);
                bool32 isDown  = (isDownBitCheck  == 0);
                if(WasDown != isDown)
                {
                    if ( (VKCode == VK_ESCAPE) || ((VKCode == VK_F4) && altKeyBitCheck) )
                    {
                        SendMessageW(windowHandle, WM_CLOSE, 0, 0);
                    }
                }
                break;
            }
        default:
        {
            TranslateMessage(&osMessage); // turn keystrokes into characters
            DispatchMessageW(&osMessage); // tell OS to call window procedure
            break;
        }
        }
    }
}

int WINAPI wWinMain(HINSTANCE currentInstanceHandle, HINSTANCE prevInstanceHandle, PWSTR argsCommandLine, int displayFlag)
{
    UNREFERENCED_PARAMETER(prevInstanceHandle);
    UNREFERENCED_PARAMETER(argsCommandLine);
    UNREFERENCED_PARAMETER(DISPLAYCONFIG_PIXELFORMAT_NONGDI);

    RECT windowSize = {0, 0, 800, 600};
    AdjustWindowRect(&windowSize, WS_OVERLAPPEDWINDOW, FALSE);
    int WindowWidth = windowSize.right - windowSize.left;
    int WindowHeight = windowSize.bottom - windowSize.top;

    HWND myWindow = 0;
    WNDCLASSEXW windowClass = {0};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW|CS_VREDRAW, // |CS_OWNDC;
    windowClass.lpfnWndProc = UnicodeWindowsProcedure;
    windowClass.hInstance = currentInstanceHandle;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    // windowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255)); // Magenta background
    windowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
    windowClass.lpszClassName = GAME_NAME "WINDOW_CLASS";

    if (RegisterClassExW(&windowClass) == 0)
    {
        MessageBoxExW(NULL, L"ERROR: Failed to register window.", L"Error", MB_ICONEXCLAMATION | MB_OK, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
        exit(1);
    }
    
    myWindow = CreateWindowExW(
        0,                      // WS_EX_TOPMOST|WS_EX_LAYERED,
        windowClass.lpszClassName,
        GAME_NAME,
        WS_OVERLAPPEDWINDOW,    // | WS_VISIBLE,
        300,                    // CW_USEDEFAULT // x 
        300,                    // CW_USEDEFAULT // y
        WindowWidth,            //CW_USEDEFAULT, // width
        WindowHeight,           //CW_USEDEFAULT, // height
        0,
        0,
        currentInstanceHandle,
        0
    );
    if (myWindow == 0)
    {
        MessageBoxExW(NULL, L"ERROR: Failed to create window.", L"Error", MB_ICONEXCLAMATION | MB_OK, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
        exit(1);
    }
    ShowWindow(myWindow, displayFlag);
    
    RUNNING_GAME = true;
    while (RUNNING_GAME)
    {
        win32_message_procedure(myWindow);
    }
    



    return 0;
}
