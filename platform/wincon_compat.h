// Émulation POSIX de la partie de l'API console Win32 utilisée par Tetris-classic.
//
// Le jeu date de 2008 et parle directement à la console Windows. Plutôt que de
// réécrire fonctions.cpp, on réimplémente les vingt fonctions dont il se sert,
// au-dessus de ncursesw. Le code de 2008 compile alors sans modification.
//
// Ce fichier n'est jamais inclus sous Windows : filehandler.h prend <windows.h>.
#ifndef WINCON_COMPAT_H
#define WINCON_COMPAT_H
#ifndef _WIN32

// <windows.h> tirait <string.h> par transitivité : le code de 2008 appelle
// memcpy et memset sans jamais les inclure. MSVC et libc++ le laissent passer,
// libstdc++ non. Le shim remplace windows.h, il doit donc en fournir autant.
#include <cstddef>
#include <cstring>
#include <cwchar>

typedef short           SHORT;
typedef long            LONG;
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;
typedef int             BOOL;
typedef void*           HANDLE;
typedef void*           HWND;
typedef wchar_t*        LPWSTR;
typedef const wchar_t*  LPCWSTR;
typedef char*           LPSTR;
typedef const char*     LPCSTR;

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

// Les constructeurs évitent les erreurs de rétrécissement sur « COORD c = {a, b} »
// quand a et b sont des WORD, ce que MSVC tolère et clang refuse.
struct COORD {
    SHORT X;
    SHORT Y;
    COORD() : X(0), Y(0) {}
    COORD(int x, int y) : X(static_cast<SHORT>(x)), Y(static_cast<SHORT>(y)) {}
};

struct RECT { LONG left; LONG top; LONG right; LONG bottom; };

struct CONSOLE_CURSOR_INFO {
    DWORD dwSize;
    BOOL  bVisible;
};

#define KEY_EVENT 0x0001

// ⚠️ ncurses définit lui aussi KEY_EVENT, à 0633 octal (411). Dans le .cpp,
// <curses.h> est inclus après cet en-tête et écrase la macro : écrire
// « EventType = KEY_EVENT » y produisait 411, que le jeu — qui n'inclut jamais
// curses — comparait à 1. Le shim doit utiliser cette constante, pas la macro.
const WORD kWinConKeyEvent = 0x0001;

struct KEY_EVENT_RECORD {
    BOOL  bKeyDown;
    WORD  wRepeatCount;
    WORD  wVirtualKeyCode;
    WORD  wVirtualScanCode;
    WORD  UnicodeChar;
    DWORD dwControlKeyState;
};

struct INPUT_RECORD {
    WORD EventType;
    union { KEY_EVENT_RECORD KeyEvent; } Event;
};

// Attributs de couleur : mêmes bits que Windows.
#define FOREGROUND_BLUE      0x0001
#define FOREGROUND_GREEN     0x0002
#define FOREGROUND_RED       0x0004
#define FOREGROUND_INTENSITY 0x0008
#define BACKGROUND_BLUE      0x0010
#define BACKGROUND_GREEN     0x0020
#define BACKGROUND_RED       0x0040
#define BACKGROUND_INTENSITY 0x0080

// Codes de touches virtuelles, restreints à ceux que le jeu lit.
#define VK_RETURN 0x0D
#define VK_ESCAPE 0x1B
#define VK_LEFT   0x25
#define VK_UP     0x26
#define VK_RIGHT  0x27
#define VK_DOWN   0x28

#define STD_INPUT_HANDLE   ((DWORD)-10)
#define STD_OUTPUT_HANDLE  ((DWORD)-11)
#define GENERIC_READ       0x80000000u
#define GENERIC_WRITE      0x40000000u
#define FILE_SHARE_READ    0x00000001u
#define FILE_SHARE_WRITE   0x00000002u
#define CONSOLE_TEXTMODE_BUFFER 1

HANDLE GetStdHandle(DWORD nStdHandle);
HANDLE CreateConsoleScreenBuffer(DWORD dwDesiredAccess, DWORD dwShareMode,
                                 const void* lpSecurityAttributes,
                                 DWORD dwFlags, void* lpScreenBufferData);
BOOL SetConsoleActiveScreenBuffer(HANDLE hConsoleOutput);
BOOL CloseHandle(HANDLE hObject);
BOOL SetConsoleScreenBufferSize(HANDLE hConsoleOutput, COORD dwSize);
BOOL SetConsoleTextAttribute(HANDLE hConsoleOutput, WORD wAttributes);
BOOL SetConsoleCursorInfo(HANDLE hConsoleOutput, const CONSOLE_CURSOR_INFO* lpci);
BOOL SetConsoleCursorPosition(HANDLE hConsoleOutput, COORD dwCursorPosition);

BOOL WriteConsoleOutputCharacterW(HANDLE h, LPCWSTR lpCharacter, DWORD nLength,
                                  COORD dwWriteCoord, DWORD* lpNumberOfCharsWritten);
BOOL WriteConsoleOutputCharacterA(HANDLE h, LPCSTR lpCharacter, DWORD nLength,
                                  COORD dwWriteCoord, DWORD* lpNumberOfCharsWritten);
BOOL WriteConsoleOutputAttribute(HANDLE h, const WORD* lpAttribute, DWORD nLength,
                                 COORD dwWriteCoord, DWORD* lpNumberOfAttrsWritten);
BOOL FillConsoleOutputCharacterW(HANDLE h, wchar_t cCharacter, DWORD nLength,
                                 COORD dwWriteCoord, DWORD* lpNumberOfCharsWritten);
BOOL FillConsoleOutputCharacterA(HANDLE h, char cCharacter, DWORD nLength,
                                 COORD dwWriteCoord, DWORD* lpNumberOfCharsWritten);
BOOL FillConsoleOutputAttribute(HANDLE h, WORD wAttribute, DWORD nLength,
                                COORD dwWriteCoord, DWORD* lpNumberOfAttrsWritten);

BOOL ReadConsoleOutputCharacterW(HANDLE h, LPWSTR lpCharacter, DWORD nLength,
                                 COORD dwReadCoord, DWORD* lpNumberOfCharsRead);
BOOL ReadConsoleOutputAttribute(HANDLE h, WORD* lpAttribute, DWORD nLength,
                                COORD dwReadCoord, DWORD* lpNumberOfAttrsRead);

struct SYSTEMTIME {
    WORD wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
};
void GetSystemTime(SYSTEMTIME* lpSystemTime);
void GetLocalTime(SYSTEMTIME* lpSystemTime);

BOOL PeekConsoleInputW(HANDLE h, INPUT_RECORD* lpBuffer, DWORD nLength, DWORD* lpNumberOfEventsRead);
BOOL FlushConsoleInputBuffer(HANDLE h);

BOOL SetConsoleTitleW(LPCWSTR lpConsoleTitle);
HWND GetConsoleWindow(void);
BOOL MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
BOOL GetWindowRect(HWND hWnd, RECT* lpRect);
int  lstrlenW(LPCWSTR s);

void Sleep(DWORD dwMilliseconds);

// Le jeu écrit et lit au clavier avec cout / cin pour saisir le nom du
// gagnant. ncurses possède l'écran pendant ce temps : il faut lui rendre le
// terminal, sinon la saisie s'affiche n'importe où et l'écho est coupé.
void WinConSuspend();   // rend le terminal à stdio, écran effacé
void WinConResume();    // reprend la main, écran restauré
int  wsprintfW(wchar_t* buffer, const wchar_t* format, ...);

// Le jeu est compilé en UNICODE : les macros Windows pointent sur les variantes W.
#define WriteConsoleOutputCharacter WriteConsoleOutputCharacterW
#define FillConsoleOutputCharacter  FillConsoleOutputCharacterW
#define PeekConsoleInput            PeekConsoleInputW
#define ReadConsoleOutputCharacter  ReadConsoleOutputCharacterW
#define SetConsoleTitle             SetConsoleTitleW
#define wsprintf                    wsprintfW

#endif // !_WIN32
#endif // WINCON_COMPAT_H
