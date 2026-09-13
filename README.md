# Tetris-classic

Un Tetris en C++ qui tourne dans la console Windows, écrit vers 2008 sous le nom
**HelloW-Dev**. Pas de moteur, pas de bibliothèque graphique : l'API console Win32
(`CreateConsoleScreenBuffer`, `WriteConsoleOutput`, attributs de couleur) et
**uFMOD** pour jouer un module `.xm` embarqué dans les ressources de l'exécutable.

Il n'a pas été touché entre 2008 et aujourd'hui. Ce dépôt le remet en état de
compiler, sans le réécrire.

## Compiler

Il faut Windows, CMake ≥ 3.20 et MSVC (Visual Studio Build Tools suffisent).

```bat
cmake -B build -A Win32
cmake --build build --config Release
build\Release\Tetris.exe
```

**`-A Win32` n'est pas optionnel.** `ufmod.lib` n'existe qu'en i386 ; une
configuration 64 bits s'arrête avec un message explicite plutôt que sur une erreur
d'édition de liens incompréhensible.

La CI compile la même chose à chaque push et publie `Tetris.exe` en artefact.

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

## Licence

Voir `LICENSE`.
