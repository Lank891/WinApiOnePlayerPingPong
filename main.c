#include <Windows.h>
#include <stdio.h>
#include "Logs/log.h"
#include "RNG/rng.h"

//Global variables
HINSTANCE hInst; //Instance global handler
POINT padCenter = {190, 530};
#define padLenH 50 //Half of length of pad
#define padHeightH 10 //Half of height of pad
struct {
    POINT ball;
    POINT fade1;
    POINT fade2;
    POINT fade3;
    POINT fade4;
} balls = { {190, 200}, {200, 200}, {200, 200}, {200, 200}, {200, 200} }; //Ball and fades
#define radius 10 //radius of ball
struct {
    double xDir;
    double yDir;
    double velocity;
} movement; //informations about movement of the ball
unsigned long losses = 0; //How many times player lost
BOOL playing = FALSE; //Is game running
#define LOGIC_TIMER 1 //ID of timer updating ball
unsigned long long frames = 0; //How many frames has passed
//Global bitmaps for off-screen drawing
HDC offDC = NULL; //offscreen context
HBITMAP offOldBmp = NULL; //place forold bitmap
HBITMAP offBmp = NULL; //bitmap to draw


//Functions declarations

//Register window Class
ATOM windowClassRegister(HINSTANCE);
//Initialize instance
BOOL initInstance(HINSTANCE, int);
//Process messages
LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    initLogger("log.txt");

    windowClassRegister(hInstance);
    if(!initInstance(hInstance, nCmdShow)) return FALSE;

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    closeLogger();
    return (int)msg.wParam;
}

ATOM windowClassRegister(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = wndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL; //defautlt
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = NULL;
    wcex.lpszMenuName   = NULL; //No menu
    wcex.lpszClassName  = L"pongClass";
    wcex.hIconSm        = NULL; //default

    return RegisterClassExW(&wcex);
}

BOOL initInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;

    HWND hWnd = CreateWindowExW(WS_EX_LAYERED, L"pongClass", L"Win Pong", WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX, 
    (GetSystemMetrics(SM_CXSCREEN)-400)/2, (GetSystemMetrics(SM_CYSCREEN) - 600) / 2,
    400, 600, NULL, NULL, hInstance, NULL);

    if(!hWnd) return FALSE;

    SetLayeredWindowAttributes(hWnd, 0, (255*90)/100, LWA_ALPHA);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch(message) {
        case WM_ERASEBKGND: {
            return 1;
            break;
        }
        case WM_MOUSEMOVE: {
            short xPos = (short)LOWORD(lParam);

            if(xPos < 25) xPos = 25;
            if(xPos > 375) xPos = 375;
            padCenter.x = xPos;
            //We don't redraw everytime it chages, lags too much
            //We redraw only when ball is flying and is updated, this way we don't have lags and it's good enough
            break;
        }
        case WM_KEYUP: {
            if(!playing && wParam == VK_SPACE) {
                //Start game and timer
                playing = TRUE;
                movement.xDir = (randomUniform()/2 + 0.25) * (randomUniform()-0.5 > 0 ? 1 : -1);
                movement.yDir = (randomUniform()/2 + 0.75) * (randomUniform()-0.5 > 0 ? 1 : -1);
                movement.velocity = 10;
                frames = 0;
                SetTimer(hWnd, LOGIC_TIMER, 30, NULL);
            }
            break;
        }
        case WM_TIMER: {
            //Frames counter and fadin effect
            frames += 1;
            if(frames >= 4) balls.fade4 = balls.fade3;
            if(frames >= 3) balls.fade3 = balls.fade2;
            if(frames >= 2) balls.fade2 = balls.fade1;
            balls.fade1 = balls.ball;

            //Half movement now
            balls.ball.x += (int)(movement.xDir * movement.velocity / 2);
            balls.ball.y += (int)(movement.yDir * movement.velocity / 2);

            //Check if we hit left or rght
            if(balls.ball.x < radius || balls.ball.x > 390-radius) movement.xDir *= -1;
            //Check if we hit top
            if(balls.ball.y < radius) movement.yDir *= -1;
            //Check if we are below the pad
            if(balls.ball.y > padCenter.y - padHeightH - radius) {
                //If yes but we hit the pad then bounce
                if(balls.ball.x < padCenter.x + padLenH && balls.ball.x > padCenter.x - padLenH) 
                    movement.yDir *= -1;
                else {
                    //Otherwise reset ball, stop playing, add 1 lost and kill timer
                    balls.ball = balls.fade1 = balls.fade2 = balls.fade3 = balls.fade4 = (POINT){190, 200};
                    padCenter.x = 190;
                    playing = FALSE;
                    losses += 1;
                    KillTimer(hWnd, LOGIC_TIMER);
                }
            }
            //If we are still playing then do 2nd half of move and increase velocity until limit
            if(playing) {
                balls.ball.x += (int)(movement.xDir * movement.velocity / 2);
                balls.ball.y += (int)(movement.yDir * movement.velocity / 2);
                logWrite("Play: %u; Pos: %d, %d; Dir: %.4lf, %.4lf; Vel: %.4lf\n", losses, balls.ball.x, balls.ball.y, movement.xDir, movement.yDir, movement.velocity);
                movement.velocity += 0.005;
                if(movement.velocity > 20) movement.velocity -= 0.0025;
                if(movement.velocity > 40) movement.velocity = 40;
            }
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            HBRUSH backBrush = CreateSolidBrush(RGB(20, 20, 20));
            HBRUSH padBrush = CreateSolidBrush(RGB(0, 180, 230));
            HBRUSH fade100 = CreateSolidBrush(RGB(200, 0, 200));
            HBRUSH fade80 = CreateSolidBrush(RGB(164, 0, 164));
            HBRUSH fade60 = CreateSolidBrush(RGB(128, 0, 128));
            HBRUSH fade40 = CreateSolidBrush(RGB(92, 0, 92));
            HBRUSH fade20 = CreateSolidBrush(RGB(56, 0, 56));

            HBRUSH emptyBrush = (HBRUSH)SelectObject(offDC, backBrush);

            HFONT font = CreateFontW(48, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE,
                                     FALSE, ANSI_CHARSET, OUT_OUTLINE_PRECIS,
                                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                     DEFAULT_PITCH | FF_DECORATIVE, NULL);
            HFONT frameFont = CreateFontW(20, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE,
                                     FALSE, ANSI_CHARSET, OUT_OUTLINE_PRECIS,
                                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                     DEFAULT_PITCH | FF_DECORATIVE, NULL);
            HFONT emptyFont = (HFONT)SelectObject(offDC, font);

            RECT c;
			GetClientRect(hWnd, &c);

            //Background
            Rectangle(offDC, -10, -10, c.right+10, c.bottom+10);

            //Texts
            SetBkMode(offDC, TRANSPARENT);
            SetTextColor(offDC, RGB(0, 180, 230));
            wchar_t str[256];
            swprintf_s(str, 256, L"Losses: %lu", losses);
            DrawTextW(offDC, str, (int)wcslen(str), &c, DT_SINGLELINE | DT_TOP | DT_CENTER);

            if(!playing) {
                DrawTextW(offDC, L"Space to start", 15, &c, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
            }

            //Balls
            if(frames >= 4) {
                SelectObject(offDC, fade20);
                Ellipse(offDC, balls.fade4.x-radius, balls.fade4.y-radius, balls.fade4.x+radius, balls.fade4.y+radius);
            }
            if(frames >= 3) {
                SelectObject(offDC, fade40);
                Ellipse(offDC, balls.fade3.x-radius, balls.fade3.y-radius, balls.fade3.x+radius, balls.fade3.y+radius);
            }
            if(frames >= 2) {
                SelectObject(offDC, fade60);
                Ellipse(offDC, balls.fade2.x-radius, balls.fade2.y-radius, balls.fade2.x+radius, balls.fade2.y+radius);
            }
            if(frames >= 1) {
                SelectObject(offDC, fade80);
                Ellipse(offDC, balls.fade1.x-radius, balls.fade1.y-radius, balls.fade1.x+radius, balls.fade1.y+radius);
            }
            SelectObject(offDC, fade100);
            Ellipse(offDC, balls.ball.x-radius, balls.ball.y-radius, balls.ball.x+radius, balls.ball.y+radius);

            //Pad
            SelectObject(offDC, padBrush);
            Rectangle(offDC, padCenter.x-padLenH, padCenter.y-padHeightH, padCenter.x+padLenH, padCenter.y+padHeightH);

            //Counter on pad
            SelectObject(offDC, frameFont);
            swprintf_s(str, 256, L"%llu", frames);
            RECT pc;
            pc.left = padCenter.x-padLenH;
            pc.right = padCenter.x+padLenH;
            pc.top = padCenter.y-padHeightH;
            pc.bottom = padCenter.y+padHeightH;
            SetTextColor(offDC, RGB(230, 180, 0));
            DrawTextW(offDC, str, (int)wcslen(str), &pc, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

            BitBlt(hdc, 0, 0, c.right, c.bottom, offDC, 0, 0, SRCCOPY);

            SelectObject(offDC, emptyBrush);
            DeleteObject(backBrush);
            DeleteObject(padBrush);
            DeleteObject(fade100);
            DeleteObject(fade80);
            DeleteObject(fade60);
            DeleteObject(fade40);
            DeleteObject(fade20);
            SelectObject(offDC, emptyFont);
            DeleteObject(font);
            DeleteObject(frameFont);
            break;
        }
        case WM_CREATE: {
            RECT c;
			GetClientRect(hWnd, &c);

            HDC hdc = GetDC(hWnd);
            offDC = CreateCompatibleDC(hdc);

            offBmp = CreateCompatibleBitmap(hdc, c.right-c.left, c.bottom-c.top);
            offOldBmp = (HBITMAP)SelectObject(offDC, offBmp);

            ReleaseDC(hWnd, hdc);
            break;
        }
        case WM_GETMINMAXINFO: {
            MINMAXINFO *mmi = (MINMAXINFO*)lParam;
            mmi->ptMaxSize.x = mmi->ptMaxTrackSize.x = 400;
            mmi->ptMinTrackSize.x = 400;
            mmi->ptMaxSize.y = mmi->ptMaxTrackSize.y = 600;
            mmi->ptMinTrackSize.y = 600;
            break;
        }
        case WM_DESTROY: {
            if(offOldBmp) SelectObject(offDC, offOldBmp);
            if(offDC) DeleteDC(offDC);
            if(offBmp) DeleteObject(offBmp);
            PostQuitMessage(0);
            break;
        }
        default: {
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }
    }
    return 0;
}