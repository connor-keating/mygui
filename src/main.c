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

    // TODO complete the monitor resolution check.
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    window_shape active_window = { .width = 1000, .height = 600};
    RECT windowSize = {0, 0, active_window.width, active_window.height};
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
        

        // TODO: Monitor size may be wrong because of zoom level?
        HMONITOR monitor_h = MonitorFromWindow(myWindow, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO monitor_info = {0};
        monitor_info.cbSize = sizeof ( MONITORINFO );
        BOOL monitor_info_success = GetMonitorInfoW(monitor_h, &monitor_info);
        if (monitor_info_success == 0)
        {
            application_end_error(L"ERROR: Failed to get monitor info.");
        }
        active_window.width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
        active_window.height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;

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
    GLuint vao, vertShaderID, fragShaderID, programID;

    const char* vertShader = read_shader("shaders\\vertex.shader");
    vertShaderID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShaderID, 1, &vertShader, 0);
    glCompileShader(vertShaderID);
    // Test if vertex shader compiled successfully.
    {
        int success = 0;
        char shaderLog[512] = {0};
        glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(vertShaderID, 512, 0, shaderLog);
            OutputDebugStringA(shaderLog);
            application_end_error(L"Failed to compile vertex shaders"); //, shaderLog);
        }
    }

    const char* fragShader = read_shader("shaders\\fragment.shader");
    fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShaderID, 1, &fragShader, 0);
    glCompileShader(fragShaderID);
    // Test if fragment shader compiled successfully.
    {
        int success = 0;
        char shaderLog[512] = {0};
        glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(fragShaderID, 512, 0, shaderLog);
            OutputDebugStringA(shaderLog);
            application_end_error(L"Failed to compile fragment shaders"); //, shaderLog);
        }
    }

    programID = glCreateProgram();
    glAttachShader(programID, vertShaderID);
    glAttachShader(programID, fragShaderID);
    glLinkProgram(programID);
    // Test if program compiled successfully.
    {
        int success = 0;
        char programLog[512] = {0};
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(programID, 512, 0, programLog);
            OutputDebugStringA(programLog);
            application_end_error(L"Failed to compile OpenGL program."); //, shaderLog);
        }
    }

    glDetachShader(programID, vertShaderID);
    glDetachShader(programID, fragShaderID);
    glDeleteShader(vertShaderID);
    glDeleteShader(fragShaderID);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Depth Testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

    glUseProgram(programID);


    RUNNING_GAME = true;
    while (RUNNING_GAME)
    {
        win32_message_procedure(myWindow);

        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        SwapBuffers(appDeviceContext);
    }

    return 0;
}
