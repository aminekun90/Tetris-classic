#include "filehandler.h"
#ifdef _WIN32
#include "ufmod.h"
#else
#include "platform/ufmod_posix.h"
#endif
#define VER "1.4"
#define ROUGE FOREGROUND_RED + FOREGROUND_INTENSITY
#define VERT FOREGROUND_GREEN + FOREGROUND_INTENSITY
#define BLANC FOREGROUND_GREEN + FOREGROUND_RED + FOREGROUND_BLUE + FOREGROUND_INTENSITY
#define BLEU FOREGROUND_GREEN + FOREGROUND_BLUE + FOREGROUND_INTENSITY
#define JAUNE FOREGROUND_RED + FOREGROUND_GREEN + FOREGROUND_INTENSITY
#define GRIS FOREGROUND_RED + FOREGROUND_GREEN + FOREGROUND_BLUE
#define BBLANC BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY

void DrawHLine(HANDLE console, COORD coord, WORD len, WORD attrs = BBLANC);
void DrawVLine(HANDLE console, COORD coord, WORD len, WORD attrs = BBLANC);
void DrawText(HANDLE console, COORD coord, const wchar_t* text, WORD attrs = JAUNE);
void DrawText(HANDLE console, COORD coord, const string&, WORD attrs = JAUNE);
void PrintHightScore(HANDLE buf);

void menu();
void start();
void score();
