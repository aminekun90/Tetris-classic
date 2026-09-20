#ifndef _WIN32

#include "ufmod_posix.h"

#include <SDL2/SDL.h>
#include <xmp.h>

#include <cstring>
#include <string>

namespace {

xmp_context      g_ctx = nullptr;
SDL_AudioDeviceID g_dev = 0;
bool             g_playing = false;
int              g_volume = uFMOD_MAX_VOL;

constexpr int kRate = 44100;

void audioCallback(void*, Uint8* stream, int len) {
    std::memset(stream, 0, static_cast<size_t>(len));
    if (!g_ctx || !g_playing) return;
    if (xmp_play_buffer(g_ctx, stream, len, 0) < 0) g_playing = false;
}

// a.xm est déposé à côté de l'exécutable par CMake ; on tente aussi le
// répertoire courant, pour un lancement depuis l'arborescence source.
std::string locateModule() {
    const char* base = SDL_GetBasePath();
    if (base) {
        std::string p = std::string(base) + "a.xm";
        SDL_free(const_cast<char*>(base));
        SDL_RWops* f = SDL_RWFromFile(p.c_str(), "rb");
        if (f) { SDL_RWclose(f); return p; }
    }
    return "a.xm";
}

void teardown() {
    if (g_dev) { SDL_CloseAudioDevice(g_dev); g_dev = 0; }
    if (g_ctx) {
        xmp_end_player(g_ctx);
        xmp_release_module(g_ctx);
        xmp_free_context(g_ctx);
        g_ctx = nullptr;
    }
    g_playing = false;
}

} // namespace

HWAVEOUT* uFMOD_PlaySong(void* lpXM, void*, int fdwSong) {
    // uFMOD_StopSong() se traduit par PlaySong(0, 0, 0).
    if (lpXM == nullptr && fdwSong == 0) { teardown(); return nullptr; }

    teardown();

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return nullptr;

    g_ctx = xmp_create_context();
    if (!g_ctx) return nullptr;

    std::string path = locateModule();
    if (xmp_load_module(g_ctx, const_cast<char*>(path.c_str())) != 0) {
        xmp_free_context(g_ctx);
        g_ctx = nullptr;
        return nullptr;   // pas de musique, mais le jeu continue
    }

    SDL_AudioSpec want{};
    want.freq     = kRate;
    want.format   = AUDIO_S16SYS;
    want.channels = 2;
    want.samples  = 2048;
    want.callback = audioCallback;

    g_dev = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);
    if (!g_dev) { teardown(); return nullptr; }

    xmp_start_player(g_ctx, kRate, 0);
    uFMOD_SetVolume(static_cast<unsigned int>(g_volume));
    g_playing = true;
    SDL_PauseAudioDevice(g_dev, 0);
    return reinterpret_cast<HWAVEOUT*>(1);
}

void uFMOD_Pause() {
    g_playing = false;
    if (g_dev) SDL_PauseAudioDevice(g_dev, 1);
}

void uFMOD_Resume() {
    if (!g_ctx) return;
    g_playing = true;
    if (g_dev) SDL_PauseAudioDevice(g_dev, 0);
}

void uFMOD_SetVolume(unsigned int vol) {
    if (vol > uFMOD_MAX_VOL) vol = uFMOD_MAX_VOL;
    g_volume = static_cast<int>(vol);
    // uFMOD va de 0 à 25 ; libxmp attend 0 à 100.
    if (g_ctx) xmp_set_player(g_ctx, XMP_PLAYER_VOLUME, g_volume * 4);
}

#endif // !_WIN32
