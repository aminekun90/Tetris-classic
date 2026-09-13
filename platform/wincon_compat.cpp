// Implémentation ncursesw de l'API console Win32 utilisée par Tetris-classic.
//
// Modèle : chaque « screen buffer » Windows est un tableau de cellules
// (caractère large + attribut). Les écritures vont dans le tableau ; si le
// buffer est actif, on le recopie dans ncurses et on rafraîchit. C'est
// exactement la sémantique de CreateConsoleScreenBuffer /
// SetConsoleActiveScreenBuffer, dont le jeu se sert pour son double buffering.
#ifndef _WIN32

#include "wincon_compat.h"

// L'API large de ncurses (cchar_t, setcchar, mvadd_wch) n'est exposée que si
// _XOPEN_SOURCE_EXTENDED est défini avant l'inclusion. Sur macOS c'est
// indispensable : sans cela <curses.h> ne déclare que l'API 8 bits.
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED 1
#endif

// Le sous-répertoire ncursesw/ existe sur Debian et dérivés ; macOS livre
// un ncurses compatible large sous <curses.h>.
#if defined(__has_include)
#  if __has_include(<ncursesw/curses.h>)
#    include <ncursesw/curses.h>
#  elif __has_include(<ncurses.h>)
#    include <ncurses.h>
#  else
#    include <curses.h>
#  endif
#else
#  include <curses.h>
#endif

#include <clocale>
#include <cstdarg>
#include <cstdio>
#include <cstdio>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>

namespace {

struct ScreenBuffer {
    int cols = 120;
    int rows = 40;
    std::vector<wchar_t> chars;
    std::vector<WORD>    attrs;
    WORD currentAttr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    bool cursorVisible = true;

    ScreenBuffer() { resize(cols, rows); }

    void resize(int c, int r) {
        cols = c > 0 ? c : 1;
        rows = r > 0 ? r : 1;
        chars.assign(static_cast<size_t>(cols) * rows, L' ');
        attrs.assign(static_cast<size_t>(cols) * rows, currentAttr);
    }
    size_t index(int x, int y) const { return static_cast<size_t>(y) * cols + x; }
    bool inside(int x, int y) const { return x >= 0 && y >= 0 && x < cols && y < rows; }
};

ScreenBuffer* g_active = nullptr;
bool          g_dirty = false;
ScreenBuffer* g_stdout = nullptr;
bool          g_cursesReady = false;
bool          g_hasColor = false;

// Une paire ncurses par combinaison premier plan / arrière-plan.
short pairFor(WORD attr) {
    short fg = static_cast<short>(((attr & FOREGROUND_RED)   ? COLOR_RED   : 0) |
                                  ((attr & FOREGROUND_GREEN) ? COLOR_GREEN : 0) |
                                  ((attr & FOREGROUND_BLUE)  ? COLOR_BLUE  : 0));
    short bg = static_cast<short>(((attr & BACKGROUND_RED)   ? COLOR_RED   : 0) |
                                  ((attr & BACKGROUND_GREEN) ? COLOR_GREEN : 0) |
                                  ((attr & BACKGROUND_BLUE)  ? COLOR_BLUE  : 0));
    return static_cast<short>(fg * 8 + bg + 1);
}

void shutdownCurses() {
    if (g_cursesReady) {
        endwin();
        g_cursesReady = false;
    }
}

void ensureCurses() {
    if (g_cursesReady) return;

    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    // Lecture non bloquante : le jeu interroge le clavier dans sa boucle
    // principale et ne doit jamais attendre dessus.
    nodelay(stdscr, TRUE);

    g_hasColor = has_colors();
    if (g_hasColor) {
        start_color();
        for (short fg = 0; fg < 8; ++fg)
            for (short bg = 0; bg < 8; ++bg)
                init_pair(static_cast<short>(fg * 8 + bg + 1), fg, bg);
    }

    g_cursesReady = true;
    atexit(shutdownCurses);   // exit(0) est appelé depuis le menu du jeu
}

// Peint une seule cellule. Repeindre tout l'écran à chaque écriture coûtait
// des centaines de milliers d'appels par image : le jeu écrit cellule par
// cellule, on suit le même grain.
void paintCell(const ScreenBuffer& sb, int x, int y) {
    if (x < 0 || y < 0 || x >= getmaxx(stdscr) || y >= getmaxy(stdscr)) return;
    size_t i = sb.index(x, y);
    WORD a = sb.attrs[i];
    short pair = g_hasColor ? pairFor(a) : 0;
    attr_t at = A_NORMAL;
    if (g_hasColor) at |= COLOR_PAIR(pair);
    if (a & FOREGROUND_INTENSITY) at |= A_BOLD;
    // La console Windows affiche un blanc pour NUL et les caractères de
    // contrôle. ncurses, lui, les dessine en notation caret : un NUL sort
    // en « ^@ », sur DEUX cellules. Le jeu écrit justement L"" — donc un
    // NUL — pour chaque case de pièce quand le mode « points » est actif :
    // les formes se retrouvaient éclatées et débordaient hors du plateau.
    wchar_t ch = sb.chars[i];
    if (ch < 32) ch = L' ';

    wchar_t wc[2] = { ch, L'\0' };
    cchar_t cc;
    if (setcchar(&cc, wc, at, pair, nullptr) == OK)
        mvadd_wch(y, x, &cc);
}

void blit(const ScreenBuffer& sb) {
    ensureCurses();
    for (int y = 0; y < sb.rows; ++y)
        for (int x = 0; x < sb.cols; ++x)
            paintCell(sb, x, y);
    curs_set(sb.cursorVisible ? 1 : 0);
    g_dirty = true;
}

// Repeint une plage linéaire de cellules à partir de (x, y), en suivant le
// retour à la ligne comme le fait la console Windows.
void touchedRange(ScreenBuffer* sb, int x, int y, DWORD count) {
    if (!sb || sb != g_active || count == 0) return;
    ensureCurses();
    for (DWORD i = 0; i < count; ++i) {
        if (y >= sb->rows) break;
        paintCell(*sb, x, y);
        if (++x >= sb->cols) { x = 0; ++y; }
    }
    g_dirty = true;
}

// Windows rend l'écran en continu ; ncurses veut un refresh explicite. Plutôt
// que d'en faire un par écriture — le jeu en émet des dizaines par image — on
// marque l'écran sale et on rafraîchit aux deux endroits où le jeu rend la
// main : la lecture du clavier et Sleep().
// Outil de mise au point : TETRIS_DUMP_SCREEN=/chemin écrit la grille active
// à chaque rafraîchissement. Les cellules ayant un fond coloré sortent en '#',
// ce qui rend les bordures et les blocs visibles en texte. Indispensable pour
// diagnostiquer le rendu sans écran.
void dumpScreen() {
    const char* path = getenv("TETRIS_DUMP_SCREEN");
    if (!path || !g_active) return;
    FILE* f = fopen(path, "w");
    if (!f) return;
    const ScreenBuffer& sb = *g_active;
    fprintf(f, "buffer %dx%d\n", sb.cols, sb.rows);
    for (int y = 0; y < sb.rows; ++y) {
        for (int x = 0; x < sb.cols; ++x) {
            WORD a = sb.attrs[sb.index(x, y)];
            wchar_t c = sb.chars[sb.index(x, y)];
            bool bg = (a & (BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE)) != 0;
            fputc(bg ? '#' : ((c >= 32 && c < 127) ? (int)c : '.'), f);
        }
        fputc('\n', f);
    }
    fclose(f);
}

void flushIfDirty() {
    if (!g_dirty) return;
    refresh();
    dumpScreen();
    g_dirty = false;
}

ScreenBuffer* asScreen(HANDLE h) { return static_cast<ScreenBuffer*>(h); }

// Entrée : ncurses consomme la touche, on la garde en attente pour que
// PeekConsoleInput puis FlushConsoleInputBuffer se comportent comme sous Windows.
bool g_hasPending = false;
WORD g_pendingKey = 0;

WORD translate(int ch) {
    switch (ch) {
        case KEY_UP:    return VK_UP;
        case KEY_DOWN:  return VK_DOWN;
        case KEY_LEFT:  return VK_LEFT;
        case KEY_RIGHT: return VK_RIGHT;
        case '\n': case '\r': case KEY_ENTER: return VK_RETURN;
        case 'q': case 'Q': return VK_ESCAPE;
        default: return 0;
    }
}

// En lecture non bloquante, ncurses rend l'ESC d'une séquence de flèche
// (ESC [ A) sans attendre la suite : keypad() ne suffit pas, et le jeu prend
// cet ESC pour Échap — il quitte au lieu de déplacer la pièce. On assemble
// donc la séquence à la main. Les trois octets d'une flèche arrivent dans le
// même paquet, ils sont déjà dans le tampon quand on relit.
WORD readKey() {
    int ch = getch();
    if (ch == ERR) return 0;

    if (ch != 27) return translate(ch);

    int second = getch();
    if (second == ERR) return VK_ESCAPE;          // Échap seul
    if (second != '[' && second != 'O') return VK_ESCAPE;

    switch (getch()) {
        case 'A': return VK_UP;
        case 'B': return VK_DOWN;
        case 'C': return VK_RIGHT;
        case 'D': return VK_LEFT;
        default:  return 0;                        // séquence ignorée
    }
}

} // namespace

HANDLE GetStdHandle(DWORD nStdHandle) {
    ensureCurses();
    if (nStdHandle == STD_INPUT_HANDLE) return reinterpret_cast<HANDLE>(-1);
    if (!g_stdout) {
        g_stdout = new ScreenBuffer();
        if (!g_active) g_active = g_stdout;
    }
    return g_stdout;
}

HANDLE CreateConsoleScreenBuffer(DWORD, DWORD, const void*, DWORD, void*) {
    ensureCurses();
    return new ScreenBuffer();
}

BOOL SetConsoleActiveScreenBuffer(HANDLE h) {
    ScreenBuffer* sb = asScreen(h);
    if (!sb) return FALSE;
    g_active = sb;
    blit(*sb);
    return TRUE;
}

BOOL CloseHandle(HANDLE h) {
    ScreenBuffer* sb = asScreen(h);
    if (!sb || sb == g_stdout || reinterpret_cast<long>(h) == -1) return TRUE;
    if (g_active == sb) g_active = g_stdout;
    delete sb;
    return TRUE;
}

BOOL SetConsoleScreenBufferSize(HANDLE h, COORD size) {
    ScreenBuffer* sb = asScreen(h);
    if (!sb) return FALSE;
    sb->resize(size.X, size.Y);
    if (sb == g_active) blit(*sb);
    return TRUE;
}

BOOL SetConsoleTextAttribute(HANDLE h, WORD attr) {
    ScreenBuffer* sb = asScreen(h);
    if (!sb) return FALSE;
    sb->currentAttr = attr;
    return TRUE;
}

BOOL SetConsoleCursorInfo(HANDLE h, const CONSOLE_CURSOR_INFO* ci) {
    ScreenBuffer* sb = asScreen(h);
    if (!sb || !ci) return FALSE;
    sb->cursorVisible = ci->bVisible != FALSE;
    if (sb == g_active && g_cursesReady) curs_set(sb->cursorVisible ? 1 : 0);
    return TRUE;
}

BOOL SetConsoleCursorPosition(HANDLE h, COORD pos) {
    ScreenBuffer* sb = asScreen(h);
    if (!sb) return FALSE;
    if (sb == g_active && g_cursesReady) move(pos.Y, pos.X);
    return TRUE;
}

BOOL WriteConsoleOutputCharacterW(HANDLE h, LPCWSTR text, DWORD len,
                                  COORD at, DWORD* written) {
    ScreenBuffer* sb = asScreen(h);
    if (!sb || !text) return FALSE;
    DWORD n = 0;
    for (DWORD i = 0; i < len; ++i) {
        int x = at.X + static_cast<int>(i);
        if (!sb->inside(x, at.Y)) break;
        // Windows n'écrit ici QUE le caractère : les attributs de la cellule
        // sont laissés intacts. Les écraser effaçait la bordure blanche du
        // plateau partout où le jeu écrit un bloc, et cassait MoveMatrixDown,
        // qui relit l'écran pour faire descendre les lignes.
        sb->chars[sb->index(x, at.Y)] = text[i];
        ++n;
    }
    if (written) *written = n;
    touchedRange(sb, at.X, at.Y, n);
    return TRUE;
}

BOOL WriteConsoleOutputCharacterA(HANDLE h, LPCSTR text, DWORD len,
                                  COORD at, DWORD* written) {
    if (!text) return FALSE;
    std::vector<wchar_t> wide(len);
    for (DWORD i = 0; i < len; ++i) wide[i] = static_cast<unsigned char>(text[i]);
    return WriteConsoleOutputCharacterW(h, wide.data(), len, at, written);
}

BOOL WriteConsoleOutputAttribute(HANDLE h, const WORD* attrs, DWORD len,
                                 COORD at, DWORD* written) {
    ScreenBuffer* sb = asScreen(h);
    if (!sb || !attrs) return FALSE;
    DWORD n = 0;
    for (DWORD i = 0; i < len; ++i) {
        int x = at.X + static_cast<int>(i);
        if (!sb->inside(x, at.Y)) break;
        sb->attrs[sb->index(x, at.Y)] = attrs[i];
        ++n;
    }
    if (written) *written = n;
    touchedRange(sb, at.X, at.Y, n);
    return TRUE;
}

BOOL FillConsoleOutputCharacterW(HANDLE h, wchar_t c, DWORD len,
                                 COORD at, DWORD* written) {
    ScreenBuffer* sb = asScreen(h);
    if (!sb) return FALSE;
    DWORD n = 0;
    int x = at.X, y = at.Y;
    for (DWORD i = 0; i < len; ++i) {
        if (y >= sb->rows) break;
        sb->chars[sb->index(x, y)] = c;
        ++n;
        if (++x >= sb->cols) { x = 0; ++y; }
    }
    if (written) *written = n;
    touchedRange(sb, at.X, at.Y, n);
    return TRUE;
}

BOOL FillConsoleOutputCharacterA(HANDLE h, char c, DWORD len, COORD at, DWORD* written) {
    return FillConsoleOutputCharacterW(h, static_cast<wchar_t>(static_cast<unsigned char>(c)),
                                       len, at, written);
}

BOOL FillConsoleOutputAttribute(HANDLE h, WORD attr, DWORD len, COORD at, DWORD* written) {
    ScreenBuffer* sb = asScreen(h);
    if (!sb) return FALSE;
    DWORD n = 0;
    int x = at.X, y = at.Y;
    for (DWORD i = 0; i < len; ++i) {
        if (y >= sb->rows) break;
        sb->attrs[sb->index(x, y)] = attr;
        ++n;
        if (++x >= sb->cols) { x = 0; ++y; }
    }
    if (written) *written = n;
    touchedRange(sb, at.X, at.Y, n);
    return TRUE;
}

BOOL ReadConsoleOutputCharacterW(HANDLE h, LPWSTR out, DWORD len,
                                 COORD at, DWORD* read) {
    ScreenBuffer* sb = asScreen(h);
    if (!sb || !out) return FALSE;
    DWORD n = 0;
    for (DWORD i = 0; i < len; ++i) {
        int x = at.X + static_cast<int>(i);
        if (!sb->inside(x, at.Y)) break;
        out[i] = sb->chars[sb->index(x, at.Y)];
        ++n;
    }
    if (read) *read = n;
    return TRUE;
}

BOOL ReadConsoleOutputAttribute(HANDLE h, WORD* out, DWORD len,
                                COORD at, DWORD* read) {
    ScreenBuffer* sb = asScreen(h);
    if (!sb || !out) return FALSE;
    DWORD n = 0;
    for (DWORD i = 0; i < len; ++i) {
        int x = at.X + static_cast<int>(i);
        if (!sb->inside(x, at.Y)) break;
        out[i] = sb->attrs[sb->index(x, at.Y)];
        ++n;
    }
    if (read) *read = n;
    return TRUE;
}

static void fillSystemTime(SYSTEMTIME* st, const struct tm& t, long ms) {
    if (!st) return;
    st->wYear         = static_cast<WORD>(t.tm_year + 1900);
    st->wMonth        = static_cast<WORD>(t.tm_mon + 1);
    st->wDayOfWeek    = static_cast<WORD>(t.tm_wday);
    st->wDay          = static_cast<WORD>(t.tm_mday);
    st->wHour         = static_cast<WORD>(t.tm_hour);
    st->wMinute       = static_cast<WORD>(t.tm_min);
    st->wSecond       = static_cast<WORD>(t.tm_sec);
    st->wMilliseconds = static_cast<WORD>(ms);
}

void GetSystemTime(SYSTEMTIME* st) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm t;
    gmtime_r(&ts.tv_sec, &t);
    fillSystemTime(st, t, ts.tv_nsec / 1000000L);
}

void GetLocalTime(SYSTEMTIME* st) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm t;
    localtime_r(&ts.tv_sec, &t);
    fillSystemTime(st, t, ts.tv_nsec / 1000000L);
}

BOOL PeekConsoleInputW(HANDLE, INPUT_RECORD* buf, DWORD nLength, DWORD* read) {
    ensureCurses();
    flushIfDirty();
    if (!g_hasPending) {
        WORD vk = readKey();
        if (vk) {
            g_pendingKey = vk; g_hasPending = true;
        }
    }
    DWORD n = 0;
    if (g_hasPending && buf && nLength > 0) {
        buf[0].EventType = kWinConKeyEvent;   // surtout pas la macro : curses la redéfinit
        buf[0].Event.KeyEvent.bKeyDown = TRUE;
        buf[0].Event.KeyEvent.wRepeatCount = 1;
        buf[0].Event.KeyEvent.wVirtualKeyCode = g_pendingKey;
        buf[0].Event.KeyEvent.wVirtualScanCode = 0;
        buf[0].Event.KeyEvent.UnicodeChar = 0;
        buf[0].Event.KeyEvent.dwControlKeyState = 0;
        n = 1;
    }
    if (read) *read = n;
    return TRUE;
}

BOOL FlushConsoleInputBuffer(HANDLE) {
    g_hasPending = false;
    g_pendingKey = 0;
    if (g_cursesReady) flushinp();
    return TRUE;
}

BOOL SetConsoleTitleW(LPCWSTR) { return TRUE; }   // sans objet dans un terminal
HWND GetConsoleWindow(void) { return nullptr; }
BOOL MoveWindow(HWND, int, int, int, int, BOOL) { return TRUE; }

// Le jeu s'en sert pour recentrer sa fenêtre ; dans un terminal on rend la
// taille courante, ce qui laisse le calcul se faire sans effet visible.
BOOL GetWindowRect(HWND, RECT* r) {
    if (!r) return FALSE;
    ensureCurses();
    r->left = 0;
    r->top = 0;
    r->right = getmaxx(stdscr);
    r->bottom = getmaxy(stdscr);
    return TRUE;
}

int lstrlenW(LPCWSTR s) {
    if (!s) return 0;
    int n = 0;
    while (s[n]) ++n;
    return n;
}

void WinConSuspend() {
    if (!g_cursesReady) return;
    def_prog_mode();
    endwin();
    std::fflush(stdout);
    std::printf("\033[2J\033[H");   // écran propre pour la saisie stdio
    std::fflush(stdout);
}

void WinConResume() {
    if (!g_cursesReady) return;
    reset_prog_mode();
    if (g_active) blit(*g_active);
    refresh();
}

void Sleep(DWORD ms) {
    if (g_cursesReady) flushIfDirty();
    struct timespec ts;
    ts.tv_sec  = static_cast<time_t>(ms / 1000);
    ts.tv_nsec = static_cast<long>((ms % 1000) * 1000000L);
    nanosleep(&ts, nullptr);
}

int wsprintfW(wchar_t* buffer, const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    // wsprintf de Windows plafonne à 1024 caractères : on reproduit la limite.
    int n = vswprintf(buffer, 1024, format, args);
    va_end(args);
    return n;
}

#endif // !_WIN32
