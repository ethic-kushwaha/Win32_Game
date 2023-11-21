#include <stdio.h>

#pragma warning(push, 3)
#include <windows.h>
#pragma warning(pop)
#include "main.h"

BOOL gGameIsRunning;
HWND gGameWindow;
GAMEBITMAP gDrawingSurface;

//Main Windows

int __stdcall WinMain(_In_ HINSTANCE Instance, _In_ HINSTANCE hInstPrev, _In_ PSTR CommandLine, _In_ int cmdshow)
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


    gDrawingSurface.BitmapInfo.bmiHeader.biSize = sizeof(gDrawingSurface.BitmapInfo.bmiHeader);
    gDrawingSurface.BitmapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
    gDrawingSurface.BitmapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;
    gDrawingSurface.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;
    gDrawingSurface.BitmapInfo.bmiHeader.biCompression = BI_RGB;
    gDrawingSurface.BitmapInfo.bmiHeader.biPlanes = 1;
    gDrawingSurface.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (gDrawingSurface.Memory == NULL)
    {
        MessageBoxA(NULL, "Failed to Allocated Memory for Drawing Surface", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

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
    WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WindowClass.lpszMenuName = NULL;
    WindowClass.lpszClassName = GAME_NAME "_WINDOWSCLASS";


    if (RegisterClassExA(&WindowClass) == 0)
    {
        Result = GetLastError();
        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gGameWindow = CreateWindowExA(WS_EX_CLIENTEDGE, WindowClass.lpszClassName, "Window Title", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 640, 440, NULL, NULL, GetModuleHandleA(NULL), NULL);
    if (gGameWindow == NULL)
    {
        Result = GetLastError();
        MessageBoxA(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }
    //ShowWindow(WindowHandle, TRUE);
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
    SHORT EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);
    if (EscapeKeyIsDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }
}





//RenderFrameGraphics
VOID RenderFrameGraphics(VOID)
{

}