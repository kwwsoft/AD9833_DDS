// afc.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include <commctrl.h>
#include <windowsx.h>
#include <Windows.h>
#include <string>
#include "global.h"
#include "afc.h"
#include "LogFileBinary.h"
#include "EditFreqSubClass.h"
#include "GraphControl.h"
#include "RingBuffer.h"
#include "SerialPort.h"
#include "MainWorker.h"
#include "IniStoreSetting.h"
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Minimum window size (pixels)
const int MAIN_WINDOW_WIDTH = 1024;
const int MAIN_WINDOW_HEIGHT = 700;
const COLORREF BACKGROUND_COLOR = RGB(235, 235, 235);
//-----------------------------------------------------------------------------
UINT32 userSettings[4] = { 0 }; // 0 - frequency from, 1 - frequency to, 2 - крок по частоті
//3 - 0 - обмін даними вимкнений або зупинений
//    1 - запит до генератора відправлено але підтвердження немає
//    2 - запит до генератора відправлено і підтвердження є
//буфер для відправки даних в порт
uint8_t serialTxBuffer[9]; // 9 байт для одного пакета: 1 стартовий байт, 1 байт команди, 6 байт даних, 1 байт CRC
double Vdd = 0.0; // змінна для зберігання останнього виміряного значення Vdd
//для списку портів в системі і індекс в меню вибраного порту
std::vector<std::pair<std::wstring, std::wstring>> g_ComPorts;
int g_SelectedPort = -1;
//кроки частоти для комбобокса
UINT32 uFreqSteps[FREQ_STEP_COUNT] = { 1, 10, 50, 100, 200, 500, 1000 };
//-----------------------------------------------------------------------------
// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
WCHAR szCheckSnapToPointsText[MAX_LOADSTRING]; // текст для чекбокса прив'язки до точок
WCHAR szOnlyVoutText[MAX_LOADSTRING];           // текст для чекбокса
HMENU g_hComMenu = nullptr;                      //MainMenu з компортами
HWND hWndMain = nullptr;                       // main window handle
HWND hGraph = nullptr;                         // графічний контроль для відображення графіка
HWND hStatus = nullptr;                         // status bar
HWND hStaticFreqFrom = nullptr;                   // static частота з
HWND hStaticFreqTo = nullptr;                     // static частота до
HWND hstaticFreqStep = nullptr;                   // static крок по частоті
HWND hstaticStepCount = nullptr;                  // static кількість кроків
HWND hEditFreqFrom = nullptr;                     // edit частота з
HWND hEditFreqTo = nullptr;                       // edit частота до
HWND hComboStepCount = nullptr;                    // combo кількість кроків
HWND hCheckSnapToPoints = nullptr;                 // checkbox прив'язка до точок
HWND hOnlyVout = nullptr;                          // checkbox яку напругу виводити
HWND hButtonStart = nullptr;                      // button старт
HWND hButtonStop = nullptr;                       // button стоп
//-----------------------------------------------------------------------------
SerialPort* serialPort = nullptr; // вказівник на об'єкт SerialPort для роботи з послідовним портом
MainWorker* mainWorkerPtr = nullptr; // вказівник на об'єкт MainWorker для обробки повідомлень з послідовного порта
//-----------------------------------------------------------------------------
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void CreateMainMenu(HWND hWnd);
void ChangeControlsState();
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    // TODO: Place code here.
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_AFC, szWindowClass, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_SNAP_TO_POINT_BOX, szCheckSnapToPointsText, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_ONLY_VOUT, szOnlyVoutText, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AFC));
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
//-----------------------------------------------------------------------------
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AFC));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    //wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_AFC);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}
//-----------------------------------------------------------------------------
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   // Get work area (excludes taskbar) and calculate start_size% centered
   RECT workArea;
   if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0))
   {
       // fallback to full screen if SPI fails
       workArea.left = 0;
       workArea.top = 0;
       workArea.right = GetSystemMetrics(SM_CXSCREEN);
       workArea.bottom = GetSystemMetrics(SM_CYSCREEN);
   }
   // Calculate window position to be centered in the work area
   int workWidth = workArea.right - workArea.left;
   int workHeight = workArea.bottom - workArea.top;
   // Start at 25% of the work area to leave space for taskbar and other elements
   int x = workArea.left + (workWidth - MAIN_WINDOW_WIDTH) / 4;
   int y = workArea.top + (workHeight - MAIN_WINDOW_HEIGHT) / 4;
   // Create the main application window
   hWndMain = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       x, y, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT, nullptr, nullptr, hInstance, nullptr);
      if (!hWndMain)
   {
      return FALSE;
   }
   //set main window color to gray
   HBRUSH hBrush = CreateSolidBrush(BACKGROUND_COLOR); 
   SetClassLongPtr(hWndMain, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
   //
   // Initialize common controls for StatusBar
   INITCOMMONCONTROLSEX icex;
   icex.dwSize = sizeof(icex);
   icex.dwICC = ICC_BAR_CLASSES;
   InitCommonControlsEx(&icex);
   // Create status bar with
   hStatus = CreateWindowExW(0, STATUSCLASSNAMEW, nullptr,
       WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWndMain, nullptr, hInstance, nullptr);
   if (hStatus)
   {
       // Define three parts: last stretch (-1)
       int parts[3];
       parts[0] = MAIN_WINDOW_WIDTH / 4;
       parts[1] = (MAIN_WINDOW_WIDTH * 3) / 4;
       parts[2] = -1;
       SendMessageW(hStatus, SB_SETPARTS, 3, (LPARAM)parts);
   }

   //get status bar height
   RECT rcStatus;
   if (hStatus != nullptr) {
       GetWindowRect(hStatus, &rcStatus);
   }
   int statusBarHeight = rcStatus.bottom - rcStatus.top;
   //get client area height and width
   RECT rcClient;
   GetClientRect(hWndMain, &rcClient);
   int clientAreaHeight = rcClient.bottom - rcClient.top;
   int clientAreaWidth = rcClient.right - rcClient.left;

   //створити графічний контроль для відображення графіка
   GraphControl::Register(hInstance);
   hGraph = CreateWindowExW(0, L"GraphControl", nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER,
       10, 10, clientAreaWidth - 20, clientAreaHeight - 90, hWndMain, nullptr, hInstance, nullptr);

   //create static control at bootom of the main window
   hStaticFreqFrom = CreateWindowExW(0, L"STATIC", L"Frequency From: ", WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER,
       10, clientAreaHeight - statusBarHeight - 42, 140, 20, hWndMain, nullptr, hInstance, nullptr);
   hStaticFreqTo = CreateWindowExW(0, L"STATIC", L"Frequency To: ", WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER,
       10, clientAreaHeight - statusBarHeight - 20, 140, 20, hWndMain, nullptr, hInstance, nullptr);
    hstaticFreqStep = CreateWindowExW(0, L"STATIC", L"Frequency Step: ", WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER, 
 	   270, clientAreaHeight - statusBarHeight - 42, 130, 20, hWndMain, nullptr, hInstance, nullptr);
    hstaticStepCount = CreateWindowExW(0, L"STATIC", L"Step count: N/A", WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER,
 	   270, clientAreaHeight - statusBarHeight - 20, 205, 20, hWndMain, nullptr, hInstance, nullptr);

    //create edit control right from static control hStaticFreqFrom
    hEditFreqFrom = CreateWindowExW(0, L"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | WS_TABSTOP,
 	   152, clientAreaHeight - statusBarHeight - 42, 90, 20, hWndMain, nullptr, hInstance, nullptr);
    SendMessageW(hEditFreqFrom, EM_SETLIMITTEXT, 8, 0);
    SetWindowSubclass(hEditFreqFrom, EditFreqFromSubClass, 1, 0);
    //create edit control right from static control hStaticFreqTo
    hEditFreqTo = CreateWindowExW(0, L"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | WS_TABSTOP,
       152, clientAreaHeight - statusBarHeight - 20, 90, 20, hWndMain, nullptr, hInstance, nullptr);
    SendMessageW(hEditFreqTo, EM_SETLIMITTEXT, 8, 0);
    SetWindowSubclass(hEditFreqTo, EditFreqToSubClass, 2, 0);
	//create combo control right from static control hstaticStepCount and tabstop to it and fill it with values
    hComboStepCount = CreateWindowW(L"COMBOBOX", nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST | WS_TABSTOP,
        402, clientAreaHeight - statusBarHeight - 42, 75, 200, hWndMain, nullptr, hInstance, nullptr);
    SetWindowSubclass(hComboStepCount, ComboStepCountSubClass, 3, 0);
    //заносимо в бокс кроки по частоті від 1 до 1000Гц
    for (int i = 0; i < FREQ_STEP_COUNT; i++)
     SendMessageW(hComboStepCount, CB_ADDSTRING, 0, (LPARAM)std::to_wstring(uFreqSteps[i]).c_str());
   // SendMessageW(hComboStepCount, CB_SETCURSEL, 0, 0);
	userSettings[2] = 1; // Инициализируем значение шага по умолчанию
    //кнопки стар стоп
    hButtonStart = CreateWindowExW(0, L"BUTTON", L"Start", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        500, clientAreaHeight - statusBarHeight - 42, 75, 21, hWndMain, (HMENU)IDC_START_BUTTON, hInstance, nullptr);
	hButtonStop = CreateWindowExW(0, L"BUTTON", L"Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		500, clientAreaHeight - statusBarHeight - 20, 75, 21, hWndMain, (HMENU)IDC_STOP_BUTTON, hInstance, nullptr);
	//створити checkbox для прив'язки до точок
    hCheckSnapToPoints = CreateWindowExW(0, L"BUTTON", szCheckSnapToPointsText, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		600, clientAreaHeight - statusBarHeight - 42, 150, 21, hWndMain, (HMENU)IDC_SNAP_TO_POINT_BOX, hInstance, nullptr);
    // за замовчуванням включаємо режим прив'язки до точок
    GraphControl::SetSnapToPoints(hGraph, true); 
	//чекбокс вибрається, бо snapToPoints true
    SendMessageW(hCheckSnapToPoints, BM_SETCHECK, BST_CHECKED, 0);
    //створити checkbox для для вибору тільки вихідна напруга чи відношення вихідної до вхідної
    hOnlyVout = CreateWindowExW(0, L"BUTTON", szOnlyVoutText, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        600, clientAreaHeight - statusBarHeight - 20, 190, 21, hWndMain, (HMENU)IDC_ONLY_VOUT, hInstance, nullptr);


    // Створюємо екземпляр MainWorker для обробки повідомлень з послідовного порта
	mainWorkerPtr = new MainWorker(hInstance); 
    //перевірка та відкриття порта і дозвіл на контроли
    ChangeControlsState();
    serialPort = new SerialPort(hWndMain, 262144);
    if (IniStoreSetting::GetPortAvaliable()) {
        std::wstring port = L"\\\\.\\";
        bool opened;
        port += IniStoreSetting::GetPortName();
        opened = serialPort->Open(port, 460800);
        SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(1, 0), (LPARAM)IniStoreSetting::GetPortNameLong().c_str());
        EnableWindow(hButtonStop, FALSE); // спочатку кнопка "Stop" неактивна
        EnableWindow(hButtonStart, FALSE); // спочатку кнопка "Start" неактивна, оскільки частоти не встановлені
    }
    //читати з налаштувань початкову частоту
    userSettings[0] = std::stoul(IniStoreSetting::LoadFrequencyFrom());
    SetWindowTextW(hEditFreqFrom, IniStoreSetting::LoadFrequencyFrom().c_str());
    SetFocus(hEditFreqFrom);
    //читати з налаштувань кінцеву частоту
    userSettings[1] = std::stoul(IniStoreSetting::LoadFrequencyTo());
    SetWindowTextW(hEditFreqTo, IniStoreSetting::LoadFrequencyTo().c_str());
    SetFocus(hEditFreqTo);
    //читати крок по частоті
    userSettings[2] = std::stoul(IniStoreSetting::LoadFrequencyStep());
    //встановивти в боксі зчитане значення кроку
    for (int i = 0; i < FREQ_STEP_COUNT; i++) {
        if (uFreqSteps[i] == userSettings[2]) {
            SendMessageW(hComboStepCount, CB_SETCURSEL, i, 0);
            //оновлення кількості кроків при завантаженні, якщо обидві частоти вже встановлені
            SendMessageW(hComboStepCount, WM_COMMAND, MAKEWPARAM(0, CBN_SELCHANGE), 0);
        }
    }
    //Vout or Vout/Vin
    std::wstring wsOnlyVout = IniStoreSetting::LoadOnlyVout();
    if (wsOnlyVout == L"1") {
        GraphControl::SetOnlyVout(hGraph, true);
        SendMessageW(hOnlyVout, BM_SETCHECK, BST_CHECKED, (LPARAM)0);
    }
    else{
        GraphControl::SetOnlyVout(hGraph, false);
        SendMessageW(hOnlyVout, BM_SETCHECK, BST_UNCHECKED, (LPARAM)0);
    }



        

	// Set initial focus to the first edit control
	SetFocus(hEditFreqFrom);

   ShowWindow(hWndMain, nCmdShow);
   UpdateWindow(hWndMain);

   return TRUE;
}
//-----------------------------------------------------------------------------

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
//OutputDebugString(L"case WM_SERIAL_START\n");
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        CreateMainMenu(hWnd);
        //відновити попередній вибраний порт
        IniStoreSetting::RestoreSelectedPort();
    }
    break;
    //
    case WM_COMMAND:
        {
            UINT itemId = LOWORD(wParam);
            //UINT notif = HIWORD(wParam);
            //HWND hwndCtl = (HWND)lParam;
            //wchar_t dbg[128];
            //swprintf(dbg, _countof(dbg), L"WM_COMMAND: id=%u notif=%u hwnd=0x%p\n", itemId, notif, hwndCtl);
            //OutputDebugStringW(dbg);

            // Handle dynamic COM menu items first (safe cast)
            if (itemId >= IDM_COM_BASE && itemId < IDM_COM_BASE + (UINT)g_ComPorts.size())
            {
                UINT index = itemId - IDM_COM_BASE;
                g_SelectedPort = (int)index;

                // uncheck all
                for (size_t i = 0; i < g_ComPorts.size(); ++i)
                {
                    CheckMenuItem(g_hComMenu, IDM_COM_BASE + (UINT)i, MF_BYCOMMAND | MF_UNCHECKED);
                }
                // check selected
                CheckMenuItem(g_hComMenu, itemId, MF_BYCOMMAND | MF_CHECKED);
                //зберегти вибраний порт в файл налаштувань
                IniStoreSetting::SaveSelectedPort(g_ComPorts[index].first);
                IniStoreSetting::SetPortNameLong(g_ComPorts[index].second);
                //response
                SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(1, 0), (LPARAM)g_ComPorts[index].second.c_str());
                break; // handled
            }
            // Parse command:
            switch (itemId)
            {
			//--- Handle checkbox toggle for snap to points ---    
            case IDC_SNAP_TO_POINT_BOX:
            {
                // Toggle snap to points mode
                bool snap = (SendMessageW(hCheckSnapToPoints, BM_GETCHECK, 0, 0) == BST_CHECKED);
                GraphControl::SetSnapToPoints(hGraph, snap);
                break;
            }
            case IDC_ONLY_VOUT:
            {
                // Toggle OnlyVout mode
                bool value = (SendMessageW(hOnlyVout, BM_GETCHECK, 0, 0) == BST_CHECKED);
                GraphControl::SetOnlyVout(hGraph, value);
                if (value) {
                    IniStoreSetting::SaveOnlyVout(L"1");
                }
                else {
                    IniStoreSetting::SaveOnlyVout(L"0");
                }
                break;
            }
            //--- Handle Start button click ---
            case IDC_START_BUTTON:
            {
				EnableWindow(hButtonStart, FALSE); // Деактивуємо кнопку "Start" після натискання
				EnableWindow(hButtonStop, TRUE); // Активуємо кнопку "Stop" після натискання "Start"
				userSettings[3] = 1; // Встановлюємо статус обміну даними на "запит до генератора відправлено, і він не активний"
                // Відправляємо повідомлення про старт в MainWorker
				SendMessage(mainWorkerPtr->GetHwnd(), WM_SERIAL_START, 0, 0); 
                 break;
            }
            case IDC_STOP_BUTTON:
            {
                EnableWindow(hButtonStart, TRUE); // Активуємо кнопку "Start" після натискання "Stop"
                EnableWindow(hButtonStop, FALSE); // Деактивуємо кнопку "Stop" після натискання
                userSettings[3] = 0; // Встановлюємо статус обміну даними на "обмін даними вимкнений або зупинений"
                // Відправляємо повідомлення про стоп в MainWorker
                SendMessage(mainWorkerPtr->GetHwnd(), WM_SERIAL_TX_07, 0, 0);
                break;
            }




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
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_GETMINMAXINFO:
    {
        // Enforce the minimum and maximum window tracking size
        LPMINMAXINFO pInfo = reinterpret_cast<LPMINMAXINFO>(lParam);
        if (pInfo)
        {
            pInfo->ptMinTrackSize.x = MAIN_WINDOW_WIDTH;
            pInfo->ptMinTrackSize.y = MAIN_WINDOW_HEIGHT;
			pInfo->ptMaxTrackSize.x = MAIN_WINDOW_WIDTH;
			pInfo->ptMaxTrackSize.y = MAIN_WINDOW_HEIGHT;
        }
    }
    break;
    case WM_SIZE:
    {
       /* int cx = LOWORD(lParam);
        int cy = HIWORD(lParam);
        //обмежити розмір вікна по x y
        if (cx > MIN_WINDOW_WIDTH || cy > MIN_WINDOW_HEIGHT )
        {
            cx = MIN_WINDOW_WIDTH;
			cy = MIN_WINDOW_HEIGHT;
            SetWindowPos(hWnd, nullptr, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER);
		}
  	    return true;*/
	}
    case WM_KEYDOWN:
        {
            if (wParam == VK_ESCAPE)
            {
                DestroyWindow(hWnd);
            }
            break;
		}
    case WM_DESTROY:
		// Закриваємо порт і видаляємо об'єкт SerialPort перед завершенням програми
        if (serialPort) {
			serialPort->Close(); // Закриваємо порт перед видаленням об'єкта SerialPort
			delete serialPort; // Освобождаем ресурсы SerialPort
        }
		// Видаляємо об'єкт MainWorker перед завершенням програми
        if (mainWorkerPtr) {
            delete mainWorkerPtr; // Освобождаем ресурсы MainWorker
		}

        //remove subclassing for edit control
		if (hEditFreqFrom)
		    RemoveWindowSubclass(hEditFreqFrom, EditFreqFromSubClass, 1);
		if (hEditFreqTo)
			RemoveWindowSubclass(hEditFreqTo, EditFreqToSubClass, 2);
		if (hComboStepCount)
			RemoveWindowSubclass(hComboStepCount, ComboStepCountSubClass, 3);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CreateMainMenu(HWND hWnd)
{
    // створюємо головне меню
    HMENU hMainMenu = CreateMenu();

    // створюємо підменю COM
    g_hComMenu = CreatePopupMenu();

    g_ComPorts = EnumerateComPorts();

    for (size_t i = 0; i < g_ComPorts.size(); ++i)
    {
        UINT id = IDM_COM_BASE + (UINT)i;

        AppendMenuW(
            g_hComMenu,
            MF_STRING,
            id,
            g_ComPorts[i].second.c_str());

        // робимо radio-check стиль
        MENUITEMINFO mii{};
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_FTYPE;
        mii.fType = MFT_RADIOCHECK;
        SetMenuItemInfo(g_hComMenu, id, FALSE, &mii);
    }

    // додаємо підменю в головне
    AppendMenuW(
        hMainMenu,
        MF_POPUP,
        (UINT_PTR)g_hComMenu,
        L"COM ports");

    // прикріплюємо меню до вікна
    SetMenu(hWnd, hMainMenu);
    DrawMenuBar(hWnd);
}
//-----------------------------------------------------------------------------
void ChangeControlsState()
{
    EnableWindow(hEditFreqFrom, IniStoreSetting::GetPortAvaliable());
    EnableWindow(hEditFreqTo, IniStoreSetting::GetPortAvaliable());
    EnableWindow(hButtonStart, IniStoreSetting::GetPortAvaliable());
    EnableWindow(hButtonStop, IniStoreSetting::GetPortAvaliable());
    EnableWindow(hComboStepCount, IniStoreSetting::GetPortAvaliable());
    EnableWindow(hCheckSnapToPoints, IniStoreSetting::GetPortAvaliable());
    if (!IniStoreSetting::GetPortAvaliable()) {
        SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(1, 0), (LPARAM)L"COM port not open yet");
    }
}
//-----------------------------------------------------------------------------
