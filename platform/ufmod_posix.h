// Remplacement POSIX d'uFMOD.
//
// uFMOD est de l'assembleur i386 pour Windows : c'est lui, et lui seul, qui
// clouait ce projet en 32 bits. Ici, libxmp décode le module .xm et SDL2 le
// joue. Même API, mêmes constantes — fonctions.cpp n'est pas modifié.
//
// Le jeu n'appelle que PlaySong, Pause et Resume ; le reste de l'API d'uFMOD
// n'est pas réimplémenté puisqu'il n'est pas utilisé.
#ifndef UFMOD_POSIX_H
#define UFMOD_POSIX_H
#ifndef _WIN32

typedef void HWAVEOUT;

#define XM_RESOURCE   0
#define XM_MEMORY     1
#define XM_FILE       2
#define XM_NOLOOP     8
#define XM_SUSPENDED  16

#define uFMOD_MIN_VOL     0
#define uFMOD_MAX_VOL     25
#define uFMOD_DEFAULT_VOL uFMOD_MAX_VOL

// Sous Windows, XM_RESOURCE lit le module embarqué dans l'exécutable.
// Ici il n'y a pas de ressources : on charge le fichier a.xm posé à côté du
// binaire par CMake. Si le fichier manque, le jeu tourne en silence.
HWAVEOUT* uFMOD_PlaySong(void* lpXM, void* param, int fdwSong);
void      uFMOD_Pause();
void      uFMOD_Resume();
void      uFMOD_SetVolume(unsigned int vol);

#define uFMOD_StopSong() uFMOD_PlaySong(0, 0, 0)

#endif // !_WIN32
#endif // UFMOD_POSIX_H
