# Tetris-classic

Un Tetris en C++ qui tourne dans la console Windows, écrit vers 2008 sous le nom
**HelloW-Dev**. Pas de moteur, pas de bibliothèque graphique : l'API console Win32
(`CreateConsoleScreenBuffer`, `WriteConsoleOutput`, attributs de couleur) et
**uFMOD** pour jouer un module `.xm` embarqué dans les ressources de l'exécutable.

Il n'a pas été touché entre 2008 et aujourd'hui. Ce dépôt le remet en état de
compiler, sans le réécrire.

## Trois façons de le lancer

| Plateforme | Ce que tu obtiens |
|-|-|
| **macOS** | `Tetris.app`, une fenêtre native — `./macos/make-app.sh && open macos/Tetris.app` |
| **Linux / macOS en terminal** | `cmake -B build && cmake --build build && ./build/Tetris` |
| **Windows** | le programme d'origine, code de 2008 inchangé |

## Compiler

### Windows — le code d'origine

CMake ≥ 3.20 et MSVC (les Build Tools suffisent).

```bat
cmake -B build -A Win32
cmake --build build --config Release
build\Release\Tetris.exe
```

**`-A Win32` n'est pas optionnel** sous Windows : `ufmod.lib` n'existe qu'en i386.
Une configuration 64 bits s'arrête avec un message explicite plutôt que sur une
erreur d'édition de liens incompréhensible.

### Linux et macOS — le portage

```bash
# Debian et dérivés
sudo apt install cmake pkg-config libncursesw5-dev libxmp-dev libsdl2-dev
# macOS
brew install cmake pkg-config libxmp sdl2

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/Tetris
```

Ici le build est **64 bits** : uFMOD était la seule chose qui clouait le projet
en i386.

### macOS — la fenêtre native

```bash
./macos/make-app.sh
open macos/Tetris.app
```

`Tetris.app` est une fenêtre SwiftUI contenant un émulateur de terminal
(SwiftTerm) dans lequel tourne le jeu. Il faut Xcode : SwiftTerm embarque un
shader Metal, et le compilateur `metal` n'est pas dans les Command Line Tools.

La CI compile les trois plateformes à chaque push et publie un artefact par OS.

## Ce qu'il y a dans le dépôt

| Fichier | Rôle |
|-|-|
| `tetris.cpp` | point d'entrée, appelle `menu()` |
| `fonctions.cpp` | 1 300 lignes : rendu, pièces, rotations, collisions, menus |
| `filehandler.cpp` | lecture/écriture des scores (`score.data`, `splus.data`) |
| `music.rc` | embarque `a.xm` en `RCDATA` et l'icône |
| `ufmod.lib`, `ufmod.h` | lecteur de modules .xm (tierce partie, i386) |
| `Tetris.sln`, `Tetris.vcxproj` | l'ancien projet Visual Studio, conservé tel quel |

Les fichiers Visual Studio d'origine restent là par honnêteté historique. Ils
ciblent les toolsets `v140_xp` / `v141` ; le build supporté est CMake.

## Le portage, sans réécrire le jeu

`fonctions.cpp` garde ses `CreateConsoleScreenBuffer` et ses
`WriteConsoleOutputCharacter`. Ce ne sont pas les appels qui changent, ce sont les
fonctions : `platform/` réimplémente les vingt fonctions de l'API console Windows
dont le jeu se sert, au-dessus de ncursesw.

| Élément Windows | Remplacement POSIX |
|-|-|
| buffers d'écran, attributs, curseur | tableaux de cellules blittés dans ncurses |
| `INPUT_RECORD` / `VK_*` | `wgetch` en `nodelay` + table de correspondance |
| uFMOD (asm i386) | **libxmp + SDL2** — `a.xm` est un module XM standard |
| `SYSTEMTIME`, `Sleep` | `clock_gettime`, `nanosleep` |

Windows rend l'écran en continu, ncurses veut un `refresh()` explicite. En
rafraîchissant à chaque écriture, le jeu crachait **2,4 Mo** de séquences ANSI en
trois secondes. Le rendu se fait maintenant cellule par cellule et le
rafraîchissement est différé aux deux endroits où le jeu rend la main — la lecture
du clavier et `Sleep()`. Même écran, **584 octets**.

## Ce qui a changé pour le faire revivre

Rien de la logique de jeu. Uniquement ce qui empêchait la compilation :

- `void main()` → `int main()`, refusé par les compilateurs actuels
- `music.rc` incluait `afxres.h` (MFC, absent des Build Tools) → `winres.h`
- `music.rc` incluait `"resource."` — un fichier sans extension, doublon exact de
  `resource.h`. Le doublon est supprimé, l'include corrigé
- les `#pragma comment(lib, …)` sont remplacés par l'édition de liens CMake
- `winmm.dll` et `winmm.lib` étaient **vendorisés dans le dépôt**. Ce sont des
  composants du SDK Windows : une copie de 2008 ne sert à rien et peut nuire. Supprimés,
  on lie `winmm` normalement
- `Tetris.v12.suo` (état d'IDE binaire de Visual Studio 2013) et `Tetris.vcxproj.user`
  sortent du dépôt
- `fonctions.cpp` rappelait `main()` récursivement pour revenir au menu — illégal
  en C++. Remplacé par `menu()`, ce que `main()` ne faisait qu'appeler
- `DrawText` prend des paramètres `const` : MSVC liait un littéral et un
  temporaire à des références non const, clang refuse
- sources converties d'ISO-8859-1 en UTF-8, clang n'acceptant pas
  `-finput-charset`

## Licence

Voir `LICENSE`.
