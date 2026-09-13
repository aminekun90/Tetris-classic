# Tetris-classic — Claude Context

Jeu console Win32 en C++ écrit vers 2008 (HelloW-Dev), remis en état de compiler
sans être réécrit. **La valeur de ce dépôt est historique** : on le fait vivre, on ne
le modernise pas pour le plaisir.

## Contraintes non négociables

**Cible x86 32 bits.** `ufmod.lib` — le lecteur de modules `.xm` — n'existe qu'en i386
(vérifié : un seul objet COFF, machine `0x014c`). Tout build 64 bits est impossible tant
qu'uFMOD n'est pas remplacé. `CMakeLists.txt` s'arrête avec un message clair plutôt que
de laisser échouer l'éditeur de liens. Configurer avec **`cmake -B build -A Win32`**.

**Windows uniquement.** Le rendu est de l'API console Win32 (`CreateConsoleScreenBuffer`,
`WriteConsoleOutput`, `HANDLE`, `COORD`, attributs `FOREGROUND_*`). Il n'y a pas de couche
d'abstraction : porter ailleurs serait une réécriture, pas un portage. Ne pas l'entreprendre
sans que ce soit demandé explicitement.

**Ne pas « nettoyer » le code de 2008.** `fonctions.cpp` fait 1 300 lignes en
ISO-8859-1, avec des noms français et des globales. C'est le sujet du dépôt. Les seules
modifications légitimes sont celles qui débloquent la compilation ou corrigent un bug de
jeu réel.

## Pièges

**Les sources sont en ISO-8859-1, pas en UTF-8.** `fonctions.cpp` en particulier. `grep`
peut traiter le fichier comme binaire ; passer par `iconv -f ISO-8859-1 -t UTF-8` pour le
lire. MSVC reçoit `/source-charset:.1252` — le retirer casse l'affichage des accents.

**Le build est en UNICODE, et ce n'est pas négociable.** Le code utilise des buffers
`wchar_t`, des littéraux `L"…"` et `wsprintf`, qui doit se résoudre en `wsprintfW`. Le
`.vcxproj` d'origine porte `CharacterSet=Unicode`. Compiler en MBCS donne une cascade de
`C2664: cannot convert 'wchar_t [N]' to 'LPSTR'` — c'est exactement l'erreur que la
première tentative de CI a produite. `CMakeLists.txt` définit `UNICODE` et `_UNICODE`.

**`DrawText` est un macro Windows.** `windows.h` le résout en `DrawTextW` en build
UNICODE, ce qui correspond bien à la surcharge `DrawText(HANDLE, COORD, wchar_t*, WORD)`
de `functions.h`. Cohérent, mais fragile : c'est un macro, pas une fonction.

**La musique vient des ressources, pas d'un fichier.** `uFMOD_PlaySong((void*)1, NULL,
XM_RESOURCE)` lit la ressource `RCDATA` d'identifiant 1, définie par `MUSIC` dans
`resource.h` et remplie par `music.rc` avec `a.xm`. Supprimer `a.xm` du dépôt ou retirer
`music.rc` de la cible enlève la musique sans aucune erreur de compilation.

**Il y avait deux fichiers `resource`.** `resource` (sans extension) et `resource.h`
étaient des doublons exacts ; `music.rc` incluait `"resource."`, ce qui ne fonctionne que
parce que Windows ignore le point final. Le doublon a été supprimé — ne pas le recréer.

**`score.data` et `splus.data` sont lus dans le répertoire courant.** CMake les recopie à
côté de l'exécutable après le build. Lancer le jeu depuis un autre dossier perd les
scores sans message.

**Ne jamais revendoriser `winmm.dll` / `winmm.lib`.** Ils étaient commités ; ce sont des
composants du SDK Windows. On lie `winmm` par son nom.

## Commandes
```bat
cmake -B build -A Win32
cmake --build build --config Release
build\Release\Tetris.exe
```
Sur macOS ou Linux, **rien de tout cela ne tourne** : la seule vérification possible
depuis un poste non-Windows est la CI GitHub (`.github/workflows/build.yml`,
`windows-latest`), qui compile et publie `Tetris.exe` en artefact.

## État
Le projet Visual Studio d'origine (`Tetris.sln`, `.vcxproj`, toolsets `v140_xp` / `v141`)
est conservé pour l'histoire mais n'est plus le build supporté. Si les deux divergent,
c'est `CMakeLists.txt` qui fait foi.
