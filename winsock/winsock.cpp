
#include "framework.h"
#include "winsock.h"

#include "./servStart.h"
#include "./cltStart.h"

#include <thread>


#define MAX_LOADSTRING 100

std::wstring g_serverStatus = L"Сервер не запущен";

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HWND g_hButtonServer = NULL;
HWND g_hButtonClient = NULL;
HWND g_hEditServer = NULL;
HWND g_hEditClient = NULL;
HWND g_hServerWindow = NULL;
HWND g_hClientWindow = NULL;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void UpdateServerStatus(const wchar_t* status) {
    if (g_hEditServer) {
        g_serverStatus = status;
        PostMessage(g_hEditServer, WM_SETTEXT, 0, (LPARAM)status);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINSOCK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINSOCK));

    MSG msg;

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


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINSOCK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINSOCK);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_SERVER:
            if (!g_hServerWindow) {
                g_hServerWindow = CreateWindowEx(0, L"STATIC", L"Сервер",
                    WS_CHILD | WS_VISIBLE | WS_BORDER,
                    10, 10, 400, 300, hWnd, NULL, hInst, NULL);

                g_hButtonServer = CreateWindow(L"BUTTON", L"Запустить сервер",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    20, 40, 150, 30, g_hServerWindow, (HMENU)ID_BUTTON_START_SERVER, hInst, NULL);

                g_hEditServer = CreateWindow(L"EDIT", L"Статус сервера...",
                    WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY,
                    20, 80, 350, 200, g_hServerWindow, (HMENU)ID_EDIT_SERVER_STATUS, hInst, NULL);

                SetTimer(hWnd, TIMER_UPDATE_STATUS, TIMER_INTERVAL, NULL);
            }
            ShowWindow(g_hServerWindow, SW_SHOW);
            if (g_hClientWindow) ShowWindow(g_hClientWindow, SW_HIDE);
            break;

        case IDM_CLIENT:
            if (!g_hClientWindow) {
                g_hClientWindow = CreateWindowEx(0, L"STATIC", L"Клиент",
                    WS_CHILD | WS_VISIBLE | WS_BORDER,
                    10, 10, 400, 300, hWnd, NULL, hInst, NULL);

                g_hButtonClient = CreateWindow(L"BUTTON", L"Подключиться к серверу",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    20, 40, 150, 30, g_hClientWindow, (HMENU)ID_BUTTON_CONNECT_CLIENT, hInst, NULL);

                g_hEditClient = CreateWindow(L"EDIT", L"Статус клиента...",
                    WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY,
                    20, 80, 350, 200, g_hClientWindow, (HMENU)ID_EDIT_CLIENT_STATUS, hInst, NULL);
            }
            ShowWindow(g_hClientWindow, SW_SHOW);
            if (g_hServerWindow) ShowWindow(g_hServerWindow, SW_HIDE);
            break;

        case ID_BUTTON_START_SERVER:
        {
            MessageBox(hWnd, L"Testing direct server_init call", L"Debug", MB_OK);
            int result = server_init();
            wchar_t debugMsg[256];
            swprintf(debugMsg, 256, L"server_init() returned: %d", result);
            MessageBox(hWnd, debugMsg, L"Debug", MB_OK);
            UpdateServerStatus(L"Нажата кнопка запуска сервера...");
            EnableWindow(g_hButtonServer, FALSE);

            std::thread serverThread([]() {
                UpdateServerStatus(L"Поток сервера запущен...");
                Sleep(1000);

                UpdateServerStatus(L"Вызов server_init()...");
                int result = server_init();

                if (result == 0) {
                    UpdateServerStatus(L"server_init() успешно выполнен...");
                }
                else {
                    UpdateServerStatus(L"server_init() вернул ошибку...");
                    EnableWindow(g_hButtonServer, TRUE);
                }
                });
            serverThread.detach();
        }
        break;

        case ID_BUTTON_CONNECT_CLIENT:
        {
            std::thread clientThread([]() {
                client_init();
                });
            clientThread.detach();
            SetWindowText(g_hEditClient, L"Подключение к серверу...");
        }
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
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        KillTimer(hWnd, TIMER_UPDATE_STATUS);
        PostQuitMessage(0);
        break;
    case WM_TIMER:
        if (wParam == TIMER_UPDATE_STATUS) {
            if (g_hEditServer && IsWindowVisible(g_hServerWindow)) {
                SetWindowText(g_hEditServer, g_serverStatus.c_str());
            }
        }
        break;
    case WM_SETTEXT:
        if (g_hEditServer && hWnd == g_hEditServer) {
            SetWindowText(g_hEditServer, (LPCWSTR)lParam);
            return TRUE;
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

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
