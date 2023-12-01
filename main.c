#include <stdio.h>
#pragma warning(push, 3)
#include <windows.h>
#pragma warning(pop)

#include <stdint.h>
#include "main.h"

BOOL gGameIsRunning;
HWND gGameWindow;
GAMEBITMAP gBackBuffer;
MONITORINFO gMonitorInfo = { sizeof(MONITORINFO)};

uint32_t gMonitorWidth;
uint32_t gMonitorHeight;

//Main Windows

int __stdcall WinMain(HINSTANCE Instance, HINSTANCE hInstPrev,PSTR CommandLine,int cmdshow)
{

    UNREFERENCED_PARAMETER(hInstPrev);

    UNREFERENCED_PARAMETER(CommandLine);

    UNREFERENCED_PARAMETER(cmdshow);

    if (GameIsAlreadyRunning() == TRUE)
    {
        MessageBoxA(NULL, "Anothger Instance of this program is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (CreateMainGameWindow() != ERROR_SUCCESS)
    {
        goto Exit;
    }


    gBackBuffer.BitmapInfo.bmiHeader.biSize = sizeof(gBackBuffer.BitmapInfo.bmiHeader);
    gBackBuffer.BitmapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
    gBackBuffer.BitmapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;
    gBackBuffer.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;
    gBackBuffer.BitmapInfo.bmiHeader.biCompression = BI_RGB;
    gBackBuffer.BitmapInfo.bmiHeader.biPlanes = 1;
    gBackBuffer.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (gBackBuffer.Memory == NULL)
    {
        MessageBoxA(NULL, "Failed to Allocated Memory for Drawing Surface", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }
    memset(gBackBuffer.Memory,0x7F,GAME_DRAWING_AREA_MEMORY_SIZE);
    MSG Messsage = { 0 };
    gGameIsRunning = TRUE;
    
    while(gGameIsRunning == TRUE)
    {
        while (PeekMessageA(&Messsage, gGameWindow, 0, 0,PM_REMOVE))
        {
            DispatchMessageA(&Messsage); //Redirect MainWndProc 
        }

        ProcessPlayerInput();

        RenderFrameGraphics();

        Sleep(1);
    }
 
Exit:
    return(0);
}



//Windows Procedure
LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam)
{
    LRESULT Result = 0;
    switch (Message)
    {

    case WM_CLOSE:
    {   
        gGameIsRunning = FALSE;
        PostQuitMessage(0);
        break;
    }
    default:
        {  
            Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
        }
    }
    return(Result);
}




//CreateMainGameWindow
DWORD CreateMainGameWindow(Void)
{
    DWORD Result = ERROR_SUCCESS;

    WNDCLASSEXA WindowClass = { 0 };


    WindowClass.cbSize = sizeof(WNDCLASSEXA);
    WindowClass.style = 0;
    WindowClass.lpfnWndProc = MainWindowProc;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = GetModuleHandleA(NULL);
    WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);
    WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
    WindowClass.lpszMenuName = NULL;
    WindowClass.lpszClassName = GAME_NAME "_WINDOWSCLASS";


    if (RegisterClassExA(&WindowClass) == 0)
    {
        Result = GetLastError();
        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gGameWindow = CreateWindowExA(0, WindowClass.lpszClassName, "Window Title", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 640, 440, NULL, NULL, GetModuleHandleA(NULL), NULL);
    if (gGameWindow == NULL)
    {
        Result = GetLastError();
        MessageBoxA(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gMonitorInfo) == 0)
    {
        Result = ERROR_MONITOR_NO_DESCRIPTOR;
        goto Exit;
    }
    gMonitorWidth = gMonitorInfo.rcMonitor.right - gMonitorInfo.rcMonitor.left;
    gMonitorHeight = gMonitorInfo.rcMonitor.bottom - gMonitorInfo.rcMonitor.top;

    if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW)==0)
    {
        Result = GetLastError();
        goto Exit;
    }
    if (SetWindowPos(gGameWindow, HWND_TOP, gMonitorInfo.rcMonitor.left, gMonitorInfo.rcMonitor.top, gMonitorWidth, gMonitorHeight, SWP_FRAMECHANGED)==0)
    {
        Result = GetLastError();
        goto Exit;
    }

Exit:
    return(Result);
}


//GameAlreadyRunning
BOOL GameIsAlreadyRunning(Void)
{
    HANDLE Mutex = NULL;
    Mutex = CreateMutexA(NULL, FALSE,GAME_NAME "_GAMEMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }

}


//PalyerInput
VOID ProcessPlayerInput(VOID)
{
    int16_t EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);
    if (EscapeKeyIsDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }
}





//RenderFrameGraphics
VOID RenderFrameGraphics(VOID)
{
    PIXEL32 Pixel = { 0 };
    Pixel.Blue = 0x7f;
    Pixel.Red = 0;
    Pixel.Green = 0;
    Pixel.Alpha = 0xff;

    for (int x = 0;x < GAME_RES_WIDTH * GAME_RES_HEIGHT;x++)
    {
        memcpy_s((PIXEL32*)gBackBuffer.Memory + x,sizeof(PIXEL32), & Pixel, sizeof(PIXEL32));
    }

    HDC DeviceContext = GetDC(gGameWindow);

    StretchDIBits(DeviceContext,
        0,
        0,
        gMonitorWidth,
        gMonitorHeight,
        0,
        0,
        GAME_RES_WIDTH,
        GAME_RES_HEIGHT,
        gBackBuffer.Memory,
        &gBackBuffer.BitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY);
    ReleaseDC(gGameWindow, DeviceContext);
}