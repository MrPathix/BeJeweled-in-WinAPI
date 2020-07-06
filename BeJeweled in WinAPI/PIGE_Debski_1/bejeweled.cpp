// PIGE_Debski_1.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "PIGE_Debski_1.h"
#include <time.h>
#include <string>
#include <windowsx.h>
#include <list>


#define MAX_LOADSTRING 100

#define PARTICLE_SIZE 8

#define SMALL_SIZE 8
#define MEDIUM_SIZE 10
#define LARGE_SIZE 12
#define SMALL_FIELD 80
#define MEDIUM_FIELD 70
#define LARGE_FIELD 60
#define GAP 10

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3



HBRUSH windowColors[6] = {
    CreateSolidBrush(RGB(255,0,0)),
    CreateSolidBrush(RGB(0,255,0)),
    CreateSolidBrush(RGB(0,0,255)),
    CreateSolidBrush(RGB(255,255,0)),
    CreateSolidBrush(RGB(0,255,255)),
    CreateSolidBrush(RGB(255,0,255))
};

HBRUSH crossColors[6] = {
    CreateHatchBrush(HS_CROSS, RGB(255,0,0)),
    CreateHatchBrush(HS_CROSS, RGB(0,255,0)),
    CreateHatchBrush(HS_CROSS, RGB(0,0,255)),
    CreateHatchBrush(HS_CROSS, RGB(255,255,0)),
    CreateHatchBrush(HS_CROSS, RGB(0,255,255)),
    CreateHatchBrush(HS_CROSS, RGB(255,0,255))
};

COLORREF colorrefs[6] = {
    RGB(255,0,0),
    RGB(0,255,0),
    RGB(0,0,255),
    RGB(255,255,0),
    RGB(0,255,255),
    RGB(255,0,255)
};

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HBRUSH defaultColor = CreateSolidBrush(RGB(50, 50, 50));
HBRUSH backgroundColor = CreateSolidBrush(RGB(255, 255, 255));
HBRUSH bigblackbrush = CreateSolidBrush(RGB(0, 0, 0));
HBRUSH crissCross = CreateHatchBrush(HS_CROSS, RGB(0, 0, 0));

HDC doubleBuffer;
HBITMAP btmp;

int fieldSize = SMALL_FIELD;
int boardSize = SMALL_SIZE;

struct ChildWindow
{
    HWND hWnd = NULL;
    HBRUSH color = CreateSolidBrush(RGB(50, 50, 50));
    bool clicked = false;
    bool destroy = false;
    int sequences[4] = { 0, 0, 0, 0 };
};

struct ClickedWindow
{
    int i = -1;
    int j = -1;
    bool init = false;
};

class Particle
{
public:
    int x;
    int y;
    int vx;
    int vy;
    HBRUSH color;

    Particle(int left, int top, HBRUSH color) : color(color)
    {
        this->x = left + fieldSize / 2;
        this->y = top + fieldSize / 2;

        vx = rand() % 10 + 1;
        vy = rand() % 10 + 1;

        vx = rand() % 2 ? vx : -1 * vx;
        vy = rand() % 2 ? vy : -1 * vy;

        double trouble = sqrt(vx * vx + vy * vy);
        vx = vx * 10 / trouble;
        vy = vy * 10 / trouble;
    }
};

TCHAR debug[256];

bool initializing = false;
bool playing = false;
bool isAnySequence = false;
bool destroying = false;
bool debugOn = false;
int drawn = 0;

std::list<Particle> particles;

ClickedWindow clickedWindow1;
ClickedWindow clickedWindow2;
ChildWindow childWin[LARGE_SIZE][LARGE_SIZE];
HWND mainWin;
HWND transpWin;
RECT rc;
LPTRACKMOUSEEVENT event;

int start = 0;
int random;


int sizeX()
{
    return boardSize * (fieldSize + GAP) + 3 * GAP / 2 + 1;
}

int sizeY()
{
    return boardSize * (fieldSize + GAP) + 59;
}

WNDCLASSEXW childClass;
WNDCLASSEXW transpWindow;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                MyRegisterChildClass(HINSTANCE hInstance);
ATOM                MyRegisterTransparentClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    ChildWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    TranspWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void InitializeGame();
void InitializeWindow(int, int);
void ClearWindows();
void CheckSequencesWindow(int, int);
void SwapColors(int, int, int, int);
bool IsThereASequence();
void DestroySequences();
bool MoveHasPotential(int, int, int, int);
bool SimulateGravity();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    srand((unsigned int)time(NULL));

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PIGEDEBSKI1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    MyRegisterChildClass(hInstance);
    MyRegisterTransparentClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PIGEDEBSKI1));
    TrackMouseEvent(event);
    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIGEDEBSKI1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PIGEDEBSKI1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

ATOM MyRegisterChildClass(HINSTANCE hInstance)
{
    childClass.cbSize = sizeof(WNDCLASSEX);

    childClass.style = CS_HREDRAW | CS_VREDRAW;
    childClass.lpfnWndProc = ChildWndProc;
    childClass.cbClsExtra = 0;
    childClass.cbWndExtra = 0;
    childClass.hInstance = hInstance;
    childClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIGEDEBSKI1));
    childClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    childClass.hbrBackground = CreateSolidBrush(RGB(50,50,50));
    childClass.lpszMenuName = NULL;
    childClass.lpszClassName = L"childClass";
    childClass.hIconSm = LoadIcon(childClass.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&childClass);
}

ATOM MyRegisterTransparentClass(HINSTANCE hInstance)
{
    transpWindow.cbSize = sizeof(WNDCLASSEX);

    transpWindow.style = CS_HREDRAW | CS_VREDRAW;
    transpWindow.lpfnWndProc = TranspWndProc;
    transpWindow.cbClsExtra = 0;
    transpWindow.cbWndExtra = 0;
    transpWindow.hInstance = hInstance;
    transpWindow.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIGEDEBSKI1));
    transpWindow.hCursor = LoadCursor(nullptr, IDC_ARROW);
    transpWindow.hbrBackground = NULL;
    transpWindow.lpszMenuName = NULL;
    transpWindow.lpszClassName = L"TransparentWindow";
    transpWindow.hIconSm = LoadIcon(childClass.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&transpWindow);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindow(szWindowClass, L"BeWindowed 2020 a.k.a Bejeweled in WinAPI", WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
      (GetSystemMetrics(SM_CXSCREEN) - sizeX())/2,
      (GetSystemMetrics(SM_CYSCREEN) - sizeY())/2,
       sizeX(), sizeY(),
       nullptr, nullptr, hInstance, nullptr);

   transpWin = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT,
       transpWindow.lpszClassName, L"Transparent Window", 
       WS_POPUP, 
       0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 
       hWnd, NULL, hInstance, NULL);
   
   doubleBuffer = CreateCompatibleDC(GetDC(transpWin));
   btmp = CreateCompatibleBitmap(GetDC(transpWin), GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

   SelectObject(doubleBuffer, btmp);

   SetLayeredWindowAttributes(transpWin, RGB(0,0,0), 0, LWA_COLORKEY);

   if (!hWnd)
   {
      return FALSE;
   }

   mainWin = hWnd;

   for (int i = 0; i < LARGE_SIZE; i++)
   {
       for (int j = 0; j < LARGE_SIZE; j++)
       {
           childWin[i][j].hWnd = CreateWindowW(childClass.lpszClassName, L"Child",
               WS_POPUP,
               GAP / 2 + i * (fieldSize + GAP), GAP / 2 + j * (fieldSize + GAP),
               fieldSize, fieldSize, NULL, NULL, hInstance, NULL);

           SetParent(childWin[i][j].hWnd, hWnd);
       }
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   for (int i = 0; i < LARGE_SIZE; i++)
   {
       for (int j = 0; j < LARGE_SIZE; j++)
       {
           ShowWindow(childWin[i][j].hWnd, nCmdShow);
           UpdateWindow(childWin[i][j].hWnd);
       }
   }


   //SetWindowPos(transpWin, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
   ShowWindow(transpWin, nCmdShow);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HMENU menu;
    RECT area;
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {

            case ID_SIZE_SMALL:

                if (initializing == false) 
                {
                    playing = false;
                    menu = GetMenu(hWnd);
                    CheckMenuItem(menu, ID_SIZE_SMALL, MF_CHECKED);
                    CheckMenuItem(menu, ID_SIZE_MEDIUM, MF_UNCHECKED);
                    CheckMenuItem(menu, ID_SIZE_LARGE, MF_UNCHECKED);

                    if (fieldSize == SMALL_FIELD)
                    {
                        ClearWindows();
                        break;
                    }

                    boardSize = SMALL_SIZE;
                    fieldSize = SMALL_FIELD;

                    MoveWindow(hWnd,
                        (GetSystemMetrics(SM_CXSCREEN) - sizeX()) / 2,
                        (GetSystemMetrics(SM_CYSCREEN) - sizeY()) / 2,
                        sizeX(), sizeY(), TRUE);

                    for (int i = 0; i < LARGE_SIZE; i++)
                    {
                        for (int j = 0; j < LARGE_SIZE; j++)
                        {
                            childWin[i][j].color = backgroundColor;
                        }
                    }

                    for (int i = 0; i < LARGE_SIZE; i++)
                    {
                        for (int j = 0; j < LARGE_SIZE; j++)
                        {
                            MoveWindow(childWin[i][j].hWnd,
                                GAP / 2 + i * (fieldSize + GAP),
                                GAP / 2 + j * (fieldSize + GAP),
                                fieldSize, fieldSize, TRUE);
                        }
                    }
                }

                break;

            case ID_SIZE_MEDIUM:

                if (initializing == false) 
                {
                    playing = false;
                    menu = GetMenu(hWnd);
                    CheckMenuItem(menu, ID_SIZE_SMALL, MF_UNCHECKED);
                    CheckMenuItem(menu, ID_SIZE_MEDIUM, MF_CHECKED);
                    CheckMenuItem(menu, ID_SIZE_LARGE, MF_UNCHECKED);

                    if (fieldSize == MEDIUM_FIELD)
                    {
                        ClearWindows();
                        break;
                    }

                    boardSize = MEDIUM_SIZE;
                    fieldSize = MEDIUM_FIELD;

                    for (int i = 0; i < LARGE_SIZE; i++)
                    {
                        for (int j = 0; j < LARGE_SIZE; j++)
                        {
                            childWin[i][j].color = backgroundColor;
                        }
                    }

                    MoveWindow(hWnd,
                        (GetSystemMetrics(SM_CXSCREEN) - sizeX()) / 2,
                        (GetSystemMetrics(SM_CYSCREEN) - sizeY()) / 2,
                        sizeX(), sizeY(), TRUE);

                    for (int i = 0; i < LARGE_SIZE; i++)
                    {
                        for (int j = 0; j < LARGE_SIZE; j++)
                        {
                            MoveWindow(childWin[i][j].hWnd,
                                GAP / 2 + i * (fieldSize + GAP),
                                GAP / 2 + j * (fieldSize + GAP),
                                fieldSize, fieldSize, TRUE);
                        }
                    }
                }

                break;

            case ID_SIZE_LARGE:

                if (initializing == false)
                {
                    playing = false;
                    menu = GetMenu(hWnd);
                    CheckMenuItem(menu, ID_SIZE_SMALL, MF_UNCHECKED);
                    CheckMenuItem(menu, ID_SIZE_MEDIUM, MF_UNCHECKED);
                    CheckMenuItem(menu, ID_SIZE_LARGE, MF_CHECKED);

                    if (fieldSize == LARGE_FIELD)
                    {
                        ClearWindows();
                        break;
                    }

                    boardSize = LARGE_SIZE;
                    fieldSize = LARGE_FIELD;

                    for (int i = 0; i < LARGE_SIZE; i++)
                    {
                        for (int j = 0; j < LARGE_SIZE; j++)
                        {
                            childWin[i][j].color = defaultColor;
                        }
                    }

                    MoveWindow(hWnd,
                        (GetSystemMetrics(SM_CXSCREEN) - sizeX()) / 2,
                        (GetSystemMetrics(SM_CYSCREEN) - sizeY()) / 2,
                        sizeX(), sizeY(), TRUE);


                    for (int i = 0; i < LARGE_SIZE; i++)
                    {
                        for (int j = 0; j < LARGE_SIZE; j++)
                        {
                            MoveWindow(childWin[i][j].hWnd,
                                GAP / 2 + i * (fieldSize + GAP),
                                GAP / 2 + j * (fieldSize + GAP),
                                fieldSize, fieldSize, TRUE);
                        }
                    }
                }
                break;

            case ID_GAME_NEWGAME:
                InitializeGame();
                break;

            case ID_ACCELERATOR32781:
                InitializeGame();
                break;

            case ID_HELP_DEBUGF12:
                

            case ID_DEBUGKEY:

                menu = GetMenu(mainWin);

                if (debugOn)
                {
                    _stprintf_s(debug, 256, _T(""));
                    area = { 2 * GetSystemMetrics(SM_CXSCREEN) / 5, 0, 3 * GetSystemMetrics(SM_CXSCREEN) / 5, GetSystemMetrics(SM_CYSCREEN) / 6 };
                    DrawText(GetDC(transpWin), debug, -1, &area, DT_VCENTER | DT_CENTER);
                    CheckMenuItem(menu, ID_HELP_DEBUGF12, MF_UNCHECKED);
                    InvalidateRect(transpWin, NULL, TRUE);
                }
                else
                {
                    CheckMenuItem(menu, ID_HELP_DEBUGF12, MF_CHECKED);
                    _stprintf_s(debug, 256, _T("[DEBUG] Particles active: %d"), particles.size());
                    area = { 2 * GetSystemMetrics(SM_CXSCREEN) / 5, 0, 3 * GetSystemMetrics(SM_CXSCREEN) / 5, GetSystemMetrics(SM_CYSCREEN) / 6 };
                    DrawText(GetDC(transpWin), debug, -1, &area, DT_VCENTER | DT_CENTER);
                }

                debugOn = !debugOn;

                break;

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
    {

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...

        if (destroying)
        {
            FillRect(hdc, &ps.rcPaint, bigblackbrush);
        }
        else
        {
            FillRect(hdc, &ps.rcPaint, backgroundColor);
        }

        EndPaint(hWnd, &ps);
    }
        break;

    case WM_TIMER:
    {
        switch (wParam) {

            case 2: // GAME INITIALIZATION
            {
                int i = drawn / boardSize;
                int j = drawn % boardSize;

                InitializeWindow(i, j);
                drawn++;

                if (i * boardSize + j + 1 == boardSize * boardSize)
                {
                    initializing = false;
                    playing = true;

                    KillTimer(hWnd, wParam);

                    if(IsThereASequence())
                        DestroySequences();

                }
                break;
            }
            case 7: // SIMULATING GRAVITY FOR SEQUENCES DESTRUCTION
            {
                if (!SimulateGravity())
                {
                    KillTimer(hWnd, wParam);
                    if (IsThereASequence())
                        DestroySequences();
                    else
                    {
                        destroying = false;
                        InvalidateRect(mainWin, NULL, TRUE);
                    }
                }
                break;
            }
        }
    }
        break;

    case WM_DESTROY:
        if (hWnd == mainWin)
            PostQuitMessage(0);
        else return 0;
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


//
//     CHILD MESSAGES HANDLER
//
LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT rect;

    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
        break;

    case WM_MOUSEMOVE: 
    {
        
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_HOVER | TME_LEAVE;
        tme.dwHoverTime = 1;
        tme.hwndTrack = hWnd;

        TrackMouseEvent(&tme);
        break;
    }

    case WM_MOUSEHOVER: 
    {

        GetWindowRect(hWnd, &rect);
        POINT pt{ rect.left, rect.top };
        POINT pt2{ rect.right, rect.bottom };
        int width = pt2.x - pt.x;
        ScreenToClient(mainWin, &pt);

        if(!(width > fieldSize))
            MoveWindow(hWnd, pt.x - 4, pt.y - 4, fieldSize + 8, fieldSize + 8, TRUE);
    }
        break;
    
    case WM_MOUSELEAVE:
    {
        SetTimer(hWnd, 1, 50, NULL);
    }
        break;

    case WM_LBUTTONDOWN:
    {
        if (initializing || !playing || destroying) break;

        RECT area;
        GetWindowRect(hWnd, &area);
        POINT a2{ area.left, area.top };
        ScreenToClient(mainWin, &a2);

        // extracting the array indeces from coordinates of the window
        int i = a2.x / (fieldSize + GAP);
        int j = a2.y / (fieldSize + GAP);


        if (!clickedWindow1.init)
        {
            clickedWindow1.init = true;
            clickedWindow1.i = i;
            clickedWindow1.j = j;
            childWin[i][j].clicked = true;

            InvalidateRect(childWin[i][j].hWnd, NULL, TRUE);
        }
        else
        {
            if (MoveHasPotential(clickedWindow1.i, clickedWindow1.j, i, j))
            {
                SwapColors(clickedWindow1.i, clickedWindow1.j, i, j);
                if(IsThereASequence())
                    DestroySequences();
            }

            clickedWindow1.init = false;
            childWin[clickedWindow1.i][clickedWindow1.j].clicked = false;
            InvalidateRect(childWin[clickedWindow1.i][clickedWindow1.j].hWnd, NULL, TRUE);
        }

    }
        break;

    case WM_TIMER:
    {    
        if (wParam == 1)
        {
            GetWindowRect(hWnd, &rect);
            POINT p1{ rect.left, rect.top };
            POINT p2{ rect.right, rect.bottom };
            int width = p2.x - p1.x;
            ScreenToClient(mainWin, &p1);

            if (width > fieldSize)
            {
                MoveWindow(hWnd, p1.x + 1, p1.y + 1, width - 2, width - 2, TRUE);
            }
            else KillTimer(hWnd, 1);
        }
    }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        RECT area;
        HDC hdc = BeginPaint(hWnd, &ps);
        //TODO: Add any drawing code that uses hdc here...
        
        GetWindowRect(hWnd, &area);
        POINT a2{ area.left, area.top };
        ScreenToClient(mainWin, &a2);

        int i = a2.x / (fieldSize+GAP);
        int j = a2.y / (fieldSize+GAP);
        
        if (playing || initializing)
        {
            if (childWin[i][j].destroy)
            {
                int k;
                for (k = 0; k < 6; k++)
                {
                    if (windowColors[k] == childWin[i][j].color)
                        break;
                }
                FillRect(hdc, &ps.rcPaint,crossColors[k]);
            }
            else if (!childWin[i][j].clicked || initializing)
            {
                FillRect(hdc, &ps.rcPaint, childWin[i][j].color);
            }
            else
            {
                RECT temp = ps.rcPaint;
                temp.left += 5;
                temp.top += 5;
                temp.right -= 5;
                temp.bottom -= 5;

                FillRect(hdc, &temp, childWin[i][j].color);
            }
        }
        else
        {
            FillRect(hdc, &ps.rcPaint, defaultColor);
        }
        
        EndPaint(hWnd, &ps);
    }
    break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


//
//     TRANSPARENT WINDOW MESSAGE HANDLER
//
LRESULT CALLBACK TranspWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            RECT area;
            HDC hdc = BeginPaint(hWnd, &ps);
            //TODO: Add any drawing code that uses hdc here...

            GetClientRect(hWnd, &area);
            FillRect(doubleBuffer, &area, bigblackbrush);

            for (auto it = particles.begin(); it != particles.end();)
            {
                it->x += it->vx;
                it->y += it->vy;

                if (it->x < 0 || it->x > GetSystemMetrics(SM_CXSCREEN) || it->y < 0 || it->y > GetSystemMetrics(SM_CYSCREEN))
                {
                    it = particles.erase(it);
                }
                else
                {
                    area = { it->x, it->y, it->x + PARTICLE_SIZE, it->y + PARTICLE_SIZE };
                    FillRect(doubleBuffer, &area, it->color);
                    it++;
                }

                if (debugOn)
                {
                    _stprintf_s(debug, 256, _T("[DEBUG] Particles active: %d"), particles.size());
                    area = { 2* GetSystemMetrics(SM_CXSCREEN) / 5, 0, 3 * GetSystemMetrics(SM_CXSCREEN) / 5, GetSystemMetrics(SM_CYSCREEN) / 6 };
                    DrawText(doubleBuffer, debug, -1, &area, DT_VCENTER | DT_CENTER);
                }
            }

            BitBlt(hdc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), doubleBuffer, 0, 0, SRCCOPY);
            EndPaint(hWnd, &ps);
            break;
        }

        case WM_TIMER:
        {
            switch (wParam)
            {
                case 4:
                {
                    if (particles.size() == 0)
                    {
                        KillTimer(hWnd, wParam);
                    }
                    else
                    {
                        InvalidateRect(hWnd, NULL, TRUE);
                    }
                }
            }
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void InitializeWindow(int i, int j)
{
    int random;
    RECT area;

    random = rand() % 6;
    childWin[j][i].clicked = false;
    childWin[j][i].color = windowColors[random];
    GetClientRect(childWin[j][i].hWnd, &area);
    FillRect(GetDC(childWin[j][i].hWnd), &area, windowColors[random]);
}

//
//  SETS BACKGROUND COLORS OF ALL THE GEMS TO DEFAULT COLOR
//
//  used in changing board size
void ClearWindows()
{
    RECT area;

    for (int i = 0; i < LARGE_SIZE; i++)
    {
        for (int j = 0; j < LARGE_SIZE; j++)
        {
            childWin[j][i].clicked = false;
            childWin[j][i].color = defaultColor;
            GetClientRect(childWin[j][i].hWnd, &area);
            InvalidateRect(childWin[j][i].hWnd, &area, FALSE);
        }
    }
}

void InitializeGame()
{
    if (!initializing)
    {
        ClearWindows();
        drawn = 0;
        initializing = true;
        SetTimer(mainWin, 2, 40, NULL);
    }
}

//
//  UPDATES THE SEQUENCES ARRAY FOR childWin[i][j]
//
void CheckSequencesWindow(int i, int j)
{
    int u, v;

    for (int dir = 0; dir < 4; dir++)
    {
        u = i;
        v = j;

        do {
            switch (dir)
            {
            case 0:
                v--;
                break;
            case 1:
                u++;
                break;
            case 2:
                v++;
                break;
            case 3:
                u--;
                break;
            }
        } while (u>=0 && v>=0 && v<fieldSize && u<fieldSize && childWin[u][v].color == childWin[i][j].color);

        childWin[i][j].sequences[dir] = abs(u - i) + abs(v - j) - 1;
    }
}

//
//  SWITCHES COLORS BETWEEN TWO GEMS AND INVALIDATES THEM
//
//  note: function doesn't check if the move is correct, don't use on its own
void SwapColors(int i, int j, int u, int v)
{
    if (i < 0 || j < 0 || u < 0 || v < 0)
        return;

    HBRUSH temp = childWin[i][j].color;
    childWin[i][j].color = childWin[u][v].color;
    childWin[u][v].color = temp;

    CheckSequencesWindow(i, j);
    CheckSequencesWindow(u, v);

    InvalidateRect(childWin[i][j].hWnd, NULL, TRUE);
    InvalidateRect(childWin[u][v].hWnd, NULL, TRUE);
}


//
//  UPDATES THE ChildWindow.sequences ARRAY FOR ALL GEMS ON BOARD
//
//  RETURNS TRUE IF THERE'S ALREADY A SEQUENCE OF 3 OR MORE ON BOARD
//  ELSE, IT RETURNS FALSE
//
bool IsThereASequence()
{
    bool check = false; 
    RECT rect;

    for (int i = 0; i < boardSize; i++)
    {
        for (int j = 0; j < boardSize; j++)
        {
            CheckSequencesWindow(i, j);

            if (childWin[i][j].sequences[UP] + childWin[i][j].sequences[DOWN] + 1 >= 3 ||
                childWin[i][j].sequences[LEFT] + childWin[i][j].sequences[RIGHT] + 1 >= 3)
            {
                check = true;
                GetClientRect(childWin[i][j].hWnd, &rect);
                POINT pt = { rect.left, rect.top };
                ClientToScreen(childWin[i][j].hWnd, &pt);

                for (int k = 0; k < 100; k++)
                {
                    particles.push_back(Particle(pt.x, pt.y, childWin[i][j].color));
                }
            }
        }
    }

    return check;
}

//
//  RETURNS TRUE IF SWITCHING childWin[i][j] and childWin[u][v]
//  IS A CORRECT MOVE AND CREATES A NEW SEQUENCE
//  ELSE, IT RETURNS FALSE
//
//  note: function doesn't switch two gems, only calculates the possible outcome
//
bool MoveHasPotential(int i, int j, int u, int v)
{
    if (abs(i - u) + abs(j - v) > 1)
    {
        return false;
    }

    bool check = false;

    HBRUSH temp = childWin[i][j].color;
    childWin[i][j].color = childWin[u][v].color;
    childWin[u][v].color = temp;

    CheckSequencesWindow(i, j);
    CheckSequencesWindow(u, v);

    if (childWin[i][j].sequences[UP] + childWin[i][j].sequences[DOWN] + 1 >= 3 || 
        childWin[i][j].sequences[LEFT] + childWin[i][j].sequences[RIGHT] + 1 >= 3)
        check = true;
    else if (childWin[u][v].sequences[UP] + childWin[u][v].sequences[DOWN] + 1 >= 3 || 
        childWin[u][v].sequences[LEFT] + childWin[u][v].sequences[RIGHT] + 1 >= 3)
        check = true;

    temp = childWin[i][j].color;
    childWin[i][j].color = childWin[u][v].color;
    childWin[u][v].color = temp;

    CheckSequencesWindow(i, j);
    CheckSequencesWindow(u, v);


    return check;
}

//
//  PERFORM ONLY WHEN (int[]) ChildWindow.sequences IS UP TO DATE
//  (use IsThereASequence() for updating) 
//
//  switches ChildWindow.destroy to true for every gem that is part of a sequence
//
void DestroySequences()
{
    destroying = true;
    InvalidateRect(mainWin, NULL, TRUE);

    for (int i = 0; i < boardSize; i++)
    {
        for (int j = 0; j < boardSize; j++)
        {
            if (childWin[i][j].sequences[UP] + childWin[i][j].sequences[DOWN] + 1 >= 3 ||
                childWin[i][j].sequences[LEFT] + childWin[i][j].sequences[RIGHT] + 1 >= 3)
            {
                childWin[i][j].destroy = true;

                InvalidateRect(childWin[i][j].hWnd, NULL, TRUE);
            }

        }
    }

    SetTimer(transpWin, 4, 100/6, NULL);

    // Timer for main window, for simulating gravity
    SetTimer(mainWin, 7, 500, NULL);
}

//
//  FUNCTION GOES OVER THE childWin DOUBLE ARRAY (from bottom right - IMPORTANT!) AND SEARCHES FOR GEMS WITH destroy = true
//  WHEN IT FINDS ONE, IT SWAPS THE GEM WITH THE NEXT GEM ABOVE IT
//
//  Returns true, when there is any more blocks to destroy after the iteration is finished.
//  If there's no blocks to destroy, returns false.
//
bool SimulateGravity()
{
    bool check = false;
    bool temp;

    for (int j = boardSize - 1; j > 0; j--)
    {
        for (int i = boardSize - 1; i >= 0; i--)
        {
            if (childWin[i][j].destroy)
            {
                check = true;
                temp = childWin[i][j - 1].destroy;
                childWin[i][j - 1].destroy = childWin[i][j].destroy;
                childWin[i][j].destroy = temp;
                SwapColors(i, j, i, j - 1);
            }
        }
    }

    for (int j = boardSize - 1; j >= 0; j--)
    {
        if (childWin[j][0].destroy)
        {
            childWin[j][0].color = windowColors[rand() % 6];
            childWin[j][0].destroy = false;
            InvalidateRect(childWin[j][0].hWnd, NULL, TRUE);
        }
    }

    return check;
}