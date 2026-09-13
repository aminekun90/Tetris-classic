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

## Pièges du portage POSIX (`platform/`)

**`curses.h` redéfinit `KEY_EVENT`.** ncurses le met à `0633` octal, soit **411**.
Comme `<curses.h>` est inclus après `wincon_compat.h`, écrire
`record.EventType = KEY_EVENT` dans le shim y met 411, alors que le jeu — qui
n'inclut jamais curses — compare à 1. Résultat : le clavier ne répond à rien,
sans le moindre message. Le shim utilise la constante `kWinConKeyEvent`, jamais
la macro. **Vérifier ce genre de collision pour tout nom ajouté à
`wincon_compat.h`.**

**En lecture non bloquante, ncurses ne rassemble pas les séquences d'échappement.**
Une flèche envoie `ESC [ A` ; `getch()` rend l'`ESC` seul, même avec `keypad()`.
Le jeu le prend pour Échap et appelle `exit(0)`. `readKey()` assemble la séquence
à la main — les trois octets arrivent dans le même paquet, ils sont déjà dans le
tampon quand on relit.

**Ne pas rafraîchir à chaque écriture.** Le menu du jeu boucle à vide et émet
plus de 800 000 écritures en une seconde. Un `refresh()` par écriture produisait
2,4 Mo de séquences ANSI en trois secondes. Le rendu se fait cellule par cellule
et le rafraîchissement est différé aux deux endroits où le jeu rend la main :
`PeekConsoleInput` et `Sleep`. Même écran, 584 octets.

**`<windows.h>` fournissait `<string.h>`.** Le code appelle `memcpy` et `memset`
sans les inclure. MSVC et libc++ laissent passer, libstdc++ non — d'où un échec
CI Linux seul. Le shim inclut `<cstring>` puisqu'il remplace `windows.h`.

**`WriteConsoleOutputCharacter` n'écrit QUE le caractère.** Sous Windows il ne
touche pas aux attributs de la cellule. Les écraser avec l'attribut courant —
c'était mon premier réflexe — efface la bordure blanche du plateau partout où le
jeu pose un bloc, et casse `MoveMatrixDown`, qui **relit l'écran** (`ReadConsoleOutput*`)
pour faire descendre les lignes. Le jeu se sert de la console comme structure de
données : toute approximation sur la sémantique d'une de ces fonctions devient un
bug de gameplay.

**NUL et les caractères de contrôle doivent sortir en blanc.** `DrawFigure`
écrit `L""` — une chaîne vide, donc un `\0` — pour chaque case d'une pièce quand
le mode « points » est actif. La console Windows affiche NUL comme un blanc et
seule la couleur de fond dessine le bloc. ncurses, lui, rend les caractères de
contrôle en **notation caret** : un NUL sort en `^@`, **sur deux cellules**. Les
pièces apparaissaient éclatées en `^@ @` et débordaient hors du plateau.
`paintCell` remplace donc tout caractère `< 32` par une espace.

**Pour diagnostiquer le rendu sans écran** : `TETRIS_DUMP_SCREEN=/tmp/s.txt` écrit
la grille active à chaque rafraîchissement, les cellules à fond coloré marquées
`#`. C'est ce qui a montré la bordure trouée.

**`cout` / `cin` pendant que ncurses tient l'écran.** La saisie du nom dans
`RegisterScore` écrivait n'importe où sans écho. `WinConSuspend()` /
`WinConResume()` rendent le terminal à stdio le temps de la saisie. Même endroit,
`system("cls")` n'existe pas hors Windows.

**`DebugMatrix` est une vue de mise au point restée active** dans le code de
2008 : elle imprime la matrice de collision en 0 et 1 par-dessus l'aire de jeu.
Elle est maintenant conditionnée à `TETRIS_DEBUG_MATRIX=1`. C'est la seule
entorse assumée à la règle « on ne nettoie pas le code de 2008 ».

## L'enveloppe macOS (`macos/`)

`Tetris.app` est une fenêtre SwiftUI qui contient un émulateur de terminal
(**SwiftTerm**) dans lequel tourne le binaire du jeu. Le C++ n'est pas modifié :
l'app lance l'exécutable comme un processus enfant.

```bash
./macos/make-app.sh      # CMake + SwiftPM + assemblage du bundle
open macos/Tetris.app
```

**`DEVELOPER_DIR` est obligatoire.** SwiftTerm embarque un shader Metal ; le
compilateur `metal` n'existe pas dans les Command Line Tools. Sans
`DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer`, `swift build` échoue
sur `unable to spawn process 'metal'`. `make-app.sh` le pose par défaut.

**Ne pas hériter de `LocalProcessTerminalView` pour être son propre délégué** :
la classe implémente déjà `LocalProcessTerminalViewDelegate` pour son compte, et
la sous-classer donne `overriding declaration requires an override keyword`. Le
délégué est un objet séparé (`TerminalCoordinator`).

Le binaire est cherché d'abord dans `Contents/Resources` (app distribuée), puis en
remontant vers `build/Tetris` (développement) — un bundle sans binaire affiche les
commandes de build au lieu d'une fenêtre vide.

## État
Le projet Visual Studio d'origine (`Tetris.sln`, `.vcxproj`, toolsets `v140_xp` / `v141`)
est conservé pour l'histoire mais n'est plus le build supporté. Si les deux divergent,
c'est `CMakeLists.txt` qui fait foi.
