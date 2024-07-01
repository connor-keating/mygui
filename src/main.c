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
            RECT windowDimensions = {0};
            GetClientRect(windowHandle, &windowDimensions); 
            // int left, int top, int right, int bottom
            glViewport(0, 0, windowDimensions.right, windowDimensions.bottom);
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

static void application_end_error(LPCWSTR message)
{
    MessageBoxExW(NULL, message, L"Error", MB_ICONEXCLAMATION | MB_OK, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    exit(1);
}

static char* read_shader(char *fullFilePath)
{
    FILE *stream;
    char *shader = 0;
    fopen_s(&stream, fullFilePath, "rb");
    if (stream)
    {
        fseek(stream, 0, SEEK_END);
        size_t memorySize = ftell(stream);
        shader = (char*) malloc(memorySize+1);
        memset(shader, 0, memorySize);
        fseek(stream, 0, SEEK_SET);
        size_t bytesRead = fread(shader, 1, memorySize, stream);
        if (bytesRead != memorySize)
        {
            application_end_error(L"ERROR: Read incorrect number of bytes from file.");
        }
        shader[memorySize] = '\0';
    }
    else
    {
        application_end_error(L"ERROR: Failed to read file.");
    }
    fclose(stream);
    return shader;
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
    
    glfunc_wglChoosePixelFormatARB *wglChoosePixelFormatARB = 0;
    glfunc_wglCreateContextAttribsARB *wglCreateContextAttribsARB = 0;

    // Initialize fake window for OpenGL.
    {
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
            application_end_error(L"ERROR: Failed to create fake window.");
        }
        
        HDC fakeDC = GetDC(myWindow);
        if (fakeDC == 0)
        {
            application_end_error(L"ERROR: Failed to get fake device context.");
        }

        PIXELFORMATDESCRIPTOR pixelFormat = {0};
        pixelFormat.nSize = sizeof(pixelFormat);
        pixelFormat.nVersion = 1;
        pixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pixelFormat.iPixelType = PFD_TYPE_RGBA;
        pixelFormat.cColorBits = 32;
        pixelFormat.cAlphaBits = 8;
        pixelFormat.cDepthBits = 24;
        int chosenPixelFormat = ChoosePixelFormat(fakeDC, &pixelFormat);
        if (chosenPixelFormat == 0)
        {
            application_end_error(L"ERROR: Failed to choose pixel format descriptor.");
        }
        int isPixelFormatSet = SetPixelFormat(fakeDC, chosenPixelFormat, &pixelFormat);
        if (isPixelFormatSet == 0)
        {
            application_end_error(L"ERROR: Failed to set the pixel format descriptor.");
        }
        HGLRC fakeRC = wglCreateContext(fakeDC);
        if (fakeRC == 0)
        {
            application_end_error(L"ERROR: Failed to create fake rendering context.");
        }
        int isFakeCurrent = wglMakeCurrent(fakeDC, fakeRC);
        if (isFakeCurrent == 0)
        {
            application_end_error(L"ERROR: Failed to make the OpenGL rendering context the current rendering context.");
        }
        wglChoosePixelFormatARB    = (glfunc_wglChoosePixelFormatARB*) wglGetProcAddress("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB = (glfunc_wglCreateContextAttribsARB*) wglGetProcAddress("wglCreateContextAttribsARB");
        if (wglChoosePixelFormatARB == 0 || wglCreateContextAttribsARB == 0)
        {
            application_end_error(L"ERROR: Failed to load OpenGL functions.");
        }
        wglMakeCurrent(fakeDC, 0);
        wglDeleteContext(fakeRC);
        ReleaseDC(myWindow, fakeDC);
        DestroyWindow(myWindow);
    }

    // Initialize real window for app.
    HDC appDeviceContext;
    {
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
            application_end_error(L"ERROR: Failed to create window.");
        }
        appDeviceContext = GetDC(myWindow);
        if (appDeviceContext == 0)
        {
            application_end_error(L"ERROR: Failed to get device context.");
        }
        const int pixelAttribs[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_SWAP_METHOD_ARB,    WGL_SWAP_COPY_ARB,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,     32,
            WGL_ALPHA_BITS_ARB,     8,
            WGL_DEPTH_BITS_ARB,     24,
            0 // Terminate with 0, otherwise OpenGL will throw an Error!
        };

        UINT numPixelFormats;
        int pixelFormat = 0;
        BOOL chosenPixelFormatARB = wglChoosePixelFormatARB(
            appDeviceContext,
            pixelAttribs,
            0, // Float List
            1, // Max Formats
            &pixelFormat,
            &numPixelFormats
        );
        if(chosenPixelFormatARB == 0)
        {
            application_end_error(L"ERROR: Failed to wglChoosePixelFormatARB");
        }
        PIXELFORMATDESCRIPTOR pixelFormatDescriptor = {0};
        DescribePixelFormat(appDeviceContext, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDescriptor);
        BOOL isPixelFormatSet = SetPixelFormat(appDeviceContext, pixelFormat, &pixelFormatDescriptor);
        if (isPixelFormatSet == 0)
        {
            application_end_error(L"ERROR: Failed to set the pixel format.");
        }

        const int contextAttribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB, 
            WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            WGL_CONTEXT_FLAGS_ARB, 
            WGL_CONTEXT_DEBUG_BIT_ARB,
            0 // Terminate the Array
        };

        HGLRC renderingContext = wglCreateContextAttribsARB(appDeviceContext, 0, contextAttribs);
        if (renderingContext == 0)
        {
            application_end_error(L"ERROR: Failed to create rendering context.");
        }
        BOOL isContextSet = wglMakeCurrent(appDeviceContext, renderingContext);
        if (isContextSet == 0)
        {
            application_end_error(L"ERROR: Failed to set the device and rendering context.");
        }
    }
    ShowWindow(myWindow, displayFlag);
    
    // Initialize OpenGL
    opengl_load_functions();


    // Creating OpenGL program.
    char* vertexShader   = read_shader("shaders\\vertex.shader");
    char* fragmentShader = read_shader("shaders\\fragment.shader");


    RUNNING_GAME = true;
    while (RUNNING_GAME)
    {
        win32_message_procedure(myWindow);

        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT); // also clear the depth buffer now!
        SwapBuffers(appDeviceContext);
    }
    



    return 0;
}
