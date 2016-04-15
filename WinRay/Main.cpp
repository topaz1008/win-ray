#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
#endif

#include <stdio.h>
#include "resource.h"
#include "RayTracer.h"

// Global variables
RayTracer* g_pRayTracer = NULL;
HWND g_hWnd = NULL;
HMENU g_hMenu = NULL;

u_long RenderThread(void* pParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int iCmdShow)
{
    LPTSTR WindowTitle = _T("WinRay");
    USHORT Width = 300;
    USHORT Height = 300;
    MSG Msg;

    // Register the window.
    WNDCLASS wc;
    wc.style = CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = (HINSTANCE)GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WindowTitle;
    RegisterClass(&wc);

    g_hMenu = LoadMenu(wc.hInstance, MAKEINTRESOURCE(IDR_MENU1));

    // Create the rendering window
    g_hWnd = CreateWindow(WindowTitle, WindowTitle, WS_SYSMENU, CW_USEDEFAULT,
        CW_USEDEFAULT, Width, Height, NULL, g_hMenu, wc.hInstance, NULL);
    // Bail on error
    if (!g_hWnd) return 1;

    // Show the window
    SetWindowPos(g_hWnd, NULL, 50, 50, Width, Height, SWP_NOMOVE);
    ShowWindow(g_hWnd, SW_SHOW);

    // Loop until idle
    while (TRUE)
    {
        // Did we receive a message?
        if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
        {
            if (Msg.message == WM_QUIT)
            {
                break;
            }

            // Issue messages
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }

    // Clean up
    SAFE_DELETE(g_pRayTracer);
    DestroyWindow(g_hWnd);
    g_hWnd = NULL;

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_CHECK_CRT_DF);
#endif

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    OPENFILENAME File;
    static TCHAR FileName[MAX_PATH];
    static TCHAR OutFileName[MAX_PATH];
    static TCHAR Buffer[1024];

    // Determine message type
    switch (Message)
    {
    case WM_CREATE:
        break;

    case WM_SETFOCUS:
    case WM_MOVE:
        // Keeps image on client area from getting deleted by other windows
        if (g_pRayTracer)
            g_pRayTracer->PresentFrameBuffer();
        break;

    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case ID_FILE_OPENSCENE:
                {
                    // Fill out the open file struct
                    ZeroMemory(&File, sizeof(OPENFILENAME));
                    File.lStructSize = sizeof(OPENFILENAME);
                    File.hwndOwner = NULL;
                    File.nFilterIndex = 1;
                    File.lpstrTitle = _T("Select Scene Input File");
                    File.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST |
                        OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
                    File.lpstrFile = FileName;
                    File.nMaxFile = MAX_PATH - 1;
                    File.lpstrFilter = _T("INI Files (*.ini)\0")_T("*.INI\0");

                    if (!GetOpenFileName(&File)) break;
                    GetFileTitle(FileName, Buffer, 1024);

                    SAFE_DELETE(g_pRayTracer);
                    g_pRayTracer = new RayTracer();

                    g_pRayTracer->LoadScene(FileName);
                    g_pRayTracer->InitRayTracer(hWnd);
                    SetWindowPos(hWnd, NULL, 50, 50, g_pRayTracer->GetWidth() + 5, g_pRayTracer->GetHeight() + 45, SWP_NOMOVE);

                    // Disable the UI while worker thread is running
                    EnableMenuItem(g_hMenu, ID_FILE_OPENSCENE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
                    EnableMenuItem(g_hMenu, ID_FILE_RELOAD, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
                    EnableMenuItem(g_hMenu, ID_FILE_CLEAR, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
                    EnableMenuItem(g_hMenu, ID_FILE_SAVEAS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

                    // Create the worker thread for rendering
                    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RenderThread, g_pRayTracer, 0, NULL);
                }
                break;

            case ID_FILE_RELOAD:
                // Validate required
                if (g_pRayTracer && g_pRayTracer->IsLoaded())
                {
                    SAFE_DELETE(g_pRayTracer);
                    g_pRayTracer = new RayTracer();

                    g_pRayTracer->LoadScene(FileName);
                    g_pRayTracer->InitRayTracer(hWnd);
                    SetWindowPos(hWnd, NULL, 50, 50, g_pRayTracer->GetWidth() + 5, g_pRayTracer->GetHeight() + 45, SWP_NOMOVE);

                    // Disable the UI while worker thread is running
                    EnableMenuItem(g_hMenu, ID_FILE_OPENSCENE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
                    EnableMenuItem(g_hMenu, ID_FILE_RELOAD, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
                    EnableMenuItem(g_hMenu, ID_FILE_CLEAR, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
                    EnableMenuItem(g_hMenu, ID_FILE_SAVEAS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

                    // Create the worker thread for rendering
                    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RenderThread, g_pRayTracer, 0, NULL);
                }
                break;

            case ID_FILE_CLEAR:
                if (g_pRayTracer)
                {
                    g_pRayTracer->Clear();
                    SetWindowPos(hWnd, NULL, 50, 50, 300, 300, SWP_NOMOVE);
                    SetWindowText(hWnd, "WinRay");
                }
                break;

            case ID_FILE_SAVEAS:
                {
                    // Fill out the open file struct
                    ZeroMemory(&File, sizeof(OPENFILENAME));
                    ZeroMemory(OutFileName, MAX_PATH * sizeof(TCHAR));
                    File.lStructSize = sizeof(OPENFILENAME);
                    File.hwndOwner = NULL;
                    File.nFilterIndex = 1;
                    File.lpstrTitle = _T("Select Output File");
                    File.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST |
                        OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
                    File.lpstrFile = OutFileName;
                    File.nMaxFile = MAX_PATH - 1;
                    File.lpstrFilter = _T("Targa Image File (*.tga)\0")_T("*.TGA\0");

                    if (!GetSaveFileName(&File)) break;
                    strcpy(OutFileName, File.lpstrFile);
                    strcat(OutFileName, _T(".tga"));

                    if (g_pRayTracer)
                        g_pRayTracer->Save(OutFileName);
                }
                break;

            case ID_FILE_EXIT:
                SAFE_DELETE(g_pRayTracer);
                PostQuitMessage(0);
                break;
            }
        }
        break;

    case WM_DESTROY:
    case WM_CLOSE:
        SAFE_DELETE(g_pRayTracer);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, Message, wParam, lParam);
    } // End Switch

    return 0;
}

u_long RenderThread(void* pParam)
{
    // Retrieve the parameter 
    RayTracer* pRayTracer = (RayTracer*)pParam;

    // Trigger render
    pRayTracer->Render();

    // Re-enable the UI elements
    EnableMenuItem(g_hMenu, ID_FILE_OPENSCENE, MF_BYCOMMAND | MF_ENABLED);
    EnableMenuItem(g_hMenu, ID_FILE_RELOAD, MF_BYCOMMAND | MF_ENABLED);
    EnableMenuItem(g_hMenu, ID_FILE_CLEAR, MF_BYCOMMAND | MF_ENABLED);
    EnableMenuItem(g_hMenu, ID_FILE_SAVEAS, MF_BYCOMMAND | MF_ENABLED);

    return 0;
}
