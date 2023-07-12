#include <fstream>
#include <string>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

struct AppInfo                                   // хранение данных об окне и его внутренности (значения по умолчанию)
{
    VOID update_info(UINT width, UINT height)    // обновление данных в случае изменения размеров окна
    {
        window_width = width;
        window_height = height;
        grid_width = (double)window_width / (double)(N + 1);
        grid_height = (double)window_height / (double)(N + 1);
        figure_size = (grid_width > grid_height) ? grid_height * 0.15 : grid_width * 0.15;
    }

    UINT window_width = 320;                    // ширина окна
    UINT window_height = 240;                   // высота окна
    UINT N = 2;                                 // размерность сетки
    DOUBLE grid_width;                          // ширина ячейки сетки 
    DOUBLE grid_height;                         // высота ячейки сетки
    DOUBLE figure_size;                         // переменная для уменьшения размера фигур внутри клетки
};

struct Color                                    // структура для удобного хранения цветов в rgb-формате
{
    Color() : first_value(0u), second_value(0u), third_value(0u) {}
    explicit Color(UINT first, UINT second, UINT third) :
        first_value(first), second_value(second), third_value(third) {}

    VOID operator() (UINT first, UINT second, UINT third)
    {
        first_value = first;
        second_value = second;
        third_value = third;
    }

    UINT first_value;  // красный
    UINT second_value; // зеленый
    UINT third_value;  // синий
};

AppInfo app_info;            // объект, хранящий данные об окне
Color background(0u, 0u, 255u); // цвет фона
Color grid(255u, 0u, 0u);       // цвет сетки

//матрица для определения заполненности фигурами ячеек
INT* matrix;

CONST UINT wm_synch = RegisterWindowMessage((LPCWSTR)"wm_synchSYNCH");

CONST wchar_t class_name[] = L"Sample Window Class";    // название класса
CONST TCHAR szTitle[] = L"Tic-Tac-Toe";                 // заголовок окна

HWND hwnd;
POINT mouse;             // переменная для вычисления нажатия
HPEN grid_color;         // кисть для сетки
HBRUSH background_color; // кисть для фона
RECT size;               // структура размеров

// переменная для плавного изменения цвета колесом мыши
BOOLEAN black_or_white = TRUE;
BOOLEAN stop_thread = FALSE;

HANDLE hRenderThread;
HANDLE hRenderMutex;

struct Game
{
    UINT filled_cells = 0;
    BOOLEAN whose_turn = true; // true - крестики, false - нолики

    UINT8 check_if_game_ended(UINT x, UINT y)
    {
        UINT8 value = matrix[x * (app_info.N + 1) + y];
        bool flag = true;
        for (UINT i = 0; i < (app_info.N + 1); i++) {
            if (matrix[x * (app_info.N + 1) + i] != value) {
                flag = false;
                break;
            }
        }
        if (flag) {
            return value;
        }
        flag = true;
        for (UINT i = 0; i < (app_info.N + 1); i++) {
            if (matrix[i * (app_info.N + 1) + y] != value) {
                flag = false;
                break;
            }
        }
        if (flag) {
            return value;
        }
        if (x == y || x == ((app_info.N + 1) - 1 - y)) {
            flag = true;
            for (UINT i = 0; i < (app_info.N + 1); i++) {
                if (matrix[i * (app_info.N + 1) + i] != value) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                return value;
            }
            flag = true;
            for (UINT i = 0; i < (app_info.N + 1); i++) {
                if (matrix[((app_info.N + 1) - 1 - i) * (app_info.N + 1) + i] != value) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                return value;
            }
        }
        if (filled_cells == (app_info.N + 1) * (app_info.N + 1)) {
            return 3;
        }
        return 0;

    }

    BOOLEAN try_make_turn(UINT i, UINT j, BOOLEAN turn)
    {
        if (turn != whose_turn)
        {
            if (whose_turn)
            {
                MessageBox(hwnd, L"Сейчас ходят крестики!", L"Ошибка!", MB_ICONWARNING);
                return FALSE;
            }
            else
            {
                MessageBox(hwnd, L"Сейчас ходят нолики!", L"Ошибка!", MB_ICONWARNING);
                return FALSE;
            }
        }
        whose_turn = !turn;
        filled_cells++;
        return TRUE;
    }
};

Game game;

VOID change_grid_color(BOOLEAN positive) // изменение цвета сетки
{
    switch (positive)
    {
    case TRUE:
    {
        if (black_or_white) { // true - если дошли до черного, false - до белого
            if (grid.first_value == 255u) {
                if (grid.second_value == 255u) {
                    if (grid.third_value == 255u) {
                        black_or_white = FALSE;
                    }
                    else {
                        grid.third_value += 5u;
                    }
                }
                else {
                    grid.second_value += 5u;
                }
            }
            else {
                grid.first_value += 5u;
            }
        }
        else {
            if (grid.first_value == 0u) {
                if (grid.second_value == 0u) {
                    if (grid.third_value == 0u) {
                        black_or_white = TRUE;
                    }
                    else {
                        grid.third_value -= 5u;
                    }
                }
                else {
                    grid.second_value -= 5u;
                }
            }
            else {
                grid.first_value -= 5u;
            }
        }
    }
    break;

    case FALSE:
    {
        if (black_or_white) { //true - если дошли до белого, false - до черного
            if (grid.third_value == 0u) {
                if (grid.second_value == 0u) {
                    if (grid.first_value == 0u) {
                        black_or_white = FALSE;
                    }
                    else {
                        grid.first_value -= 5u;
                    }
                }
                else {
                    grid.second_value -= 5u;
                }
            }
            else {
                grid.third_value -= 5u;
            }
        }
        else {
            if (grid.third_value == 255u) {
                if (grid.second_value == 255u) {
                    if (grid.first_value == 255u) {
                        black_or_white = TRUE;
                    }
                    else {
                        grid.first_value += 5u;
                    }
                }
                else {
                    grid.second_value += 5u;
                }
            }
            else {
                grid.third_value += 5u;
            }
        }
    }
    break;
    }
}

UINT GetThreadPriority(UINT d) {
    switch (d) {
    case 1:
        return THREAD_PRIORITY_IDLE;
    case 2:
        return THREAD_PRIORITY_LOWEST;
    case 3:
        return THREAD_PRIORITY_BELOW_NORMAL;
    case 5:
        return THREAD_PRIORITY_ABOVE_NORMAL;
    case 6:
        return THREAD_PRIORITY_HIGHEST;
    case 7:
        return THREAD_PRIORITY_TIME_CRITICAL;
    default:
        return THREAD_PRIORITY_NORMAL;
    }
}

VOID write_file(HWND hwnd) // записывает параметры окна в конфигурационный файл
{
    std::ofstream out("config.txt");
    GetWindowRect(hwnd, &size); // обновляем значение размеров окна, так как сейчас там только клиентская область
    app_info.update_info(size.right - size.left, size.bottom - size.top);
    if (out.is_open())
    {
        out.clear();
        out << app_info.N << "\n"
            << app_info.window_width << "\n"
            << app_info.window_height << "\n"
            << background.first_value << "\n"
            << background.second_value << "\n"
            << background.third_value << "\n"
            << grid.first_value << "\n"
            << grid.second_value << "\n"
            << grid.third_value;
        out.close();
    }
}

VOID read_file() // считываем данные из конфигурационного файла и обновляем параметры окна
{
    std::ifstream in("config.txt");
    if (in.is_open())
    {
        std::string str;
        if (!getline(in, str)) return;
        app_info.N = std::stoi(str);
        if (!getline(in, str)) return;
        app_info.window_width = std::stoi(str);
        if (!getline(in, str)) return;
        app_info.window_height = std::stoi(str);
        if (!getline(in, str)) return;
        background.first_value = std::stoi(str);
        if (!getline(in, str)) return;
        background.second_value = std::stoi(str);
        if (!getline(in, str)) return;
        background.third_value = std::stoi(str);
        if (!getline(in, str)) return;
        grid.first_value = std::stoi(str);
        if (!getline(in, str)) return;
        grid.second_value = std::stoi(str);
        if (!getline(in, str)) return;
        grid.third_value = std::stoi(str);
        in.close();
    }
}

VOID run_notepad() // запускаем блокнот
{
    STARTUPINFO sInfo;
    PROCESS_INFORMATION pInfo;
    ZeroMemory(&sInfo, sizeof(STARTUPINFO));
    CreateProcess(_T("C:\\Windows\\Notepad.exe"),
        NULL, NULL, NULL, FALSE, 0, NULL, NULL, &sInfo, &pInfo);
}

VOID paint_cross(HDC hdc, INT i, INT j) // рисуем крестики
{
    MoveToEx(hdc, i * app_info.grid_width + app_info.figure_size,
        j * app_info.grid_height + app_info.figure_size, NULL);
    LineTo(hdc, (i + 1) * app_info.grid_width - app_info.figure_size,
        (j + 1) * app_info.grid_height - app_info.figure_size);
    MoveToEx(hdc, (i + 1) * app_info.grid_width - app_info.figure_size,
        j * app_info.grid_height + app_info.figure_size, NULL);
    LineTo(hdc, i * app_info.grid_width + app_info.figure_size,
        (j + 1) * app_info.grid_height - app_info.figure_size);
}

VOID paint_circle(HDC hdc, INT i, INT j) // рисуем нолики
{
    Arc(hdc, i * app_info.grid_width + app_info.figure_size,
        j * app_info.grid_height + app_info.figure_size,
        (i + 1) * app_info.grid_width - app_info.figure_size,
        (j + 1) * app_info.grid_height - app_info.figure_size,
        i * app_info.grid_width + app_info.figure_size,
        j * app_info.grid_height + app_info.grid_height / 2 + app_info.figure_size,
        i * app_info.grid_width + app_info.figure_size,
        j * app_info.grid_height + app_info.grid_height / 2 + app_info.figure_size);
}

VOID paint_grid(HDC hdc) // рисуем сетку
{
    for (DOUBLE x = app_info.grid_width, i = 0; i < app_info.N; x += app_info.grid_width, ++i)   // вертикальные полосы
    {
        MoveToEx(hdc, x, 0, NULL);
        LineTo(hdc, x, app_info.window_height);
    }
    for (DOUBLE y = app_info.grid_height, i = 0; i < app_info.N; y += app_info.grid_height, ++i) // горизонтальные полосы
    {
        MoveToEx(hdc, 0, y, NULL);
        LineTo(hdc, app_info.window_width, y);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg)
    {
    case WM_DESTROY:
    {
        write_file(hwnd);
        TerminateThread(hRenderThread, 0);
        PostQuitMessage(0);
    }
    return 0;

    case WM_SIZE:
    {
        app_info.update_info(LOWORD(lParam), HIWORD(lParam));
    }
    return 0;

    case WM_KEYDOWN:
    {
        switch (wParam)
        {
            case VK_ESCAPE: // обработка нажатия esc
            {
                write_file(hwnd);
                TerminateThread(hRenderThread, 0);
                PostQuitMessage(0);
                return 0;
            }

            case VK_RETURN: // обработка нажатия enter
            {
                background(rand() % 256, rand() % 256, rand() % 256);
                background_color = CreateSolidBrush(
                    RGB(background.first_value, background.second_value, background.third_value));
                HBRUSH background_color_old = (HBRUSH)(SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG)background_color));
                DeleteObject(background_color_old);
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            }

            case VK_SPACE:
            {
                if (!stop_thread)
                {
                    SuspendThread(hRenderThread);
                    stop_thread = ~stop_thread;
                }
                else
                {
                    ResumeThread(hRenderThread);
                    stop_thread = ~stop_thread;
                }
            }

            case 67:
            {
                if (GetKeyState(VK_SHIFT) & 0x400) // обработка нажатия shift+c
                {
                    run_notepad();
                }
                return 0;
            }

            case 81:
            {
                if (GetKeyState(VK_CONTROL) & 0x400) // обработка нажатия ctrl+q
                {
                    write_file(hwnd);
                    TerminateThread(hRenderThread, 0);
                    PostQuitMessage(0);
                    break;
                }
                return 0;
            }

            default: 
                if (wParam > 48 && wParam < 56) {
                    if (!SetThreadPriority(hRenderThread, GetThreadPriority(wParam - 48))) {
                        _tprintf(_T("Thread priority changing error!\n"));
                    }
                }
        }
    }
    return 0;

    case WM_MOUSEWHEEL:
    {
        if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)      // если крутим колесико вверх
        {
            for (INT i = 0; i < GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA; ++i)
            {
                change_grid_color(TRUE);
            }
            InvalidateRect(hwnd, NULL, FALSE);
        }
        else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)  // если крутим колесико вниз
        {
            for (INT i = 0; i > GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA; --i)
            {
                change_grid_color(FALSE);
            }
            InvalidateRect(hwnd, NULL, FALSE);
        }
    }
    return 0;

    case WM_RBUTTONDOWN: // записываем нажатие правой клавиши мыши в matrix (крестик)
    {
        mouse.x = GET_X_LPARAM(lParam);
        mouse.y = GET_Y_LPARAM(lParam);
        UINT i = mouse.x / app_info.grid_width;
        UINT j = mouse.y / app_info.grid_height;
        if (i == app_info.N + 1) --i;
        if (j == app_info.N + 1) --j;
        if (matrix[i * (app_info.N + 1) + j] == 0)
        {
            if (game.try_make_turn(i, j, true)) {
                matrix[i * (app_info.N + 1) + j] = 2;
                SendMessage(HWND_BROADCAST, wm_synch, i, j);
            }
        }
    }
    return 0;

    case WM_LBUTTONDOWN: // записываем нажатие левой клавиши мыши в matrix (нолик)
    {
        mouse.x = GET_X_LPARAM(lParam);
        mouse.y = GET_Y_LPARAM(lParam);
        UINT i = mouse.x / app_info.grid_width;
        UINT j = mouse.y / app_info.grid_height;
        if (i == app_info.N + 1) --i;
        if (j == app_info.N + 1) --j;
        if (matrix[i * (app_info.N + 1) + j] == 0)
        {
            if (game.try_make_turn(i, j, false)) {
                matrix[i * (app_info.N + 1) + j] = 1;
                SendMessage(HWND_BROADCAST, wm_synch, i, j);
            }
        }
    }
    return 0;

    default:
    {
        if (msg == wm_synch) {
            UINT result = game.check_if_game_ended(wParam, lParam);
            if (result)
            {
                switch (result)
                {
                case 1:
                    MessageBox(hwnd, L"Победили нолики!", L"Игра завершена.", MB_ICONINFORMATION);
                    break;
                case 2:
                    MessageBox(hwnd, L"Победили крестики!", L"Игра завершена.", MB_ICONINFORMATION);
                    break;
                case 3:
                    MessageBox(hwnd, L"Ничья!", L"Игра завершена.", MB_ICONINFORMATION);
                    break; 
                }
                PostMessage(hwnd, WM_DESTROY, 0, 0);
            }
        }
        break;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

DWORD WINAPI RenderThread(LPVOID)
{
    while (TRUE) {
        background.third_value = 125;
        if (background.second_value < 254)
            background.second_value += 5;
        else background.second_value = 45;
        if (background.first_value < 254)
            background.first_value += 5;
        else background.first_value = 0;
        background_color = CreateSolidBrush(RGB(background.first_value, background.second_value, background.third_value));
        HBRUSH background_color_old = (HBRUSH)SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)background_color);
        Sleep(50);
        InvalidateRect(hwnd, NULL, 1);
        DeleteObject(background_color_old);

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        grid_color = CreatePen(PS_SOLID, 2, RGB(grid.first_value, grid.second_value, grid.third_value));
        SelectObject(hdc, grid_color);
        paint_grid(hdc);

        grid_color = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        SelectObject(hdc, grid_color);

        for (UINT i = 0; i < app_info.N + 1; ++i) {
            for (UINT j = 0; j < app_info.N + 1; ++j) {
                if (matrix[i * (app_info.N + 1) + j] == 2) {
                    paint_cross(hdc, i, j);
                }
                else if (matrix[i * (app_info.N + 1) + j] == 1) {
                    paint_circle(hdc, i, j);
                }
            }
        }
        EndPaint(hwnd, &ps);
        DeleteObject(grid_color);
        DeleteObject(hdc);
    }
    return TRUE;
}

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, INT nCmdShow)
{
    srand(time(NULL));

    INT argc;
    LPWSTR* lpArgv = CommandLineToArgvW(GetCommandLineW(), &argc); // для получения аргумента командной строки

    read_file();

    if (argc > 1) {
        app_info.N = std::stoi(lpArgv[1]);
    }

    HANDLE hMap = CreateFileMapping(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (app_info.N + 1) * (app_info.N + 1) * sizeof(int), _T("FUNNY_JOKE.CPP"));

    if (hMap == NULL)
    {
        return EXIT_FAILURE;
    }

    matrix = (int*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, (app_info.N + 1) * (app_info.N + 1) * sizeof(int));

    if (matrix == NULL)
    {
        return EXIT_FAILURE;
    }

    LocalFree(lpArgv);

    background_color = CreateSolidBrush(
        RGB(background.first_value, background.second_value, background.third_value));

    WNDCLASS window{};
    window.style = CS_HREDRAW | CS_VREDRAW; // перерисовка при изменении размеров окна 
    window.lpfnWndProc = WindowProc;
    window.hInstance = hInstance;
    window.lpszClassName = class_name;
    window.hbrBackground = background_color;

    if (!RegisterClass(&window))
    {
        return EXIT_FAILURE;
    }

    hwnd = CreateWindowEx
    (
        0,                              // расширенный стиль класса
        class_name,                     // название класса
        szTitle,                        // заголовок
        WS_OVERLAPPEDWINDOW,            // стиль окна
        CW_USEDEFAULT, CW_USEDEFAULT,   // позиция окна
        app_info.window_width,
        app_info.window_height,         // размер окна
        NULL,                           // родительское окно
        NULL,                           // дочернее окно
        hInstance,                      // передача histance 
        NULL                            // дополнительные данные приложения
    );

    GetClientRect(hwnd, &size);  // обновляем значения размеров окна на размеры клиентской области для более точного рисования
    app_info.update_info(size.right - size.left, size.bottom - size.top);

    if (hwnd == INVALID_HANDLE_VALUE)
    {
        return EXIT_FAILURE;
    }

    ShowWindow(hwnd, nCmdShow);
    hRenderThread = CreateThread(NULL, 0, RenderThread, NULL, 0, 0);

    BOOL bMessageOk{};
    MSG message{};

    while ((bMessageOk = GetMessage(&message, NULL, 0, 0)) != 0)
    {
        if (bMessageOk == -1)
        {
            MessageBox(hwnd, L"GetMessage failed", L"ERROR!", MB_ICONINFORMATION);
            break;
        }
        else
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    DeleteObject(background_color);
    DeleteObject(grid_color);
    DestroyWindow(hwnd);
    UnregisterClass(class_name, window.hInstance);
    UnmapViewOfFile(matrix);
    CloseHandle(hMap);

    return EXIT_SUCCESS;
}