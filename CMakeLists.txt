# Tetris-classic — build moderne d'un jeu console de 2008.
#
# Windows : code d'origine, API console native + uFMOD. La cible y reste
#           x86 32 bits, ufmod.lib n'existant qu'en i386 — cmake -B build -A Win32.
# POSIX   : platform/ réimplémente l'API console sur ncursesw et uFMOD sur
#           libxmp + SDL2. 64 bits, et fonctions.cpp n'est pas modifié.
cmake_minimum_required(VERSION 3.20)
project(Tetris VERSION 1.3.0 LANGUAGES CXX)

# Sous Windows on garde le code d'origine : API console native et uFMOD.
# Ailleurs, platform/ réimplémente l'une et l'autre — ncursesw pour la console,
# libxmp + SDL2 pour le module .xm — et fonctions.cpp n'est pas modifié.
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR
    "Build Windows 64 bits demandé, mais ufmod.lib est en i386. "
    "Reconfigurer avec :  cmake -B build -A Win32  (le portage POSIX, lui, est 64 bits)")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TETRIS_SOURCES tetris.cpp fonctions.cpp filehandler.cpp)

if(WIN32)
  list(APPEND TETRIS_SOURCES music.rc)   # embarque a.xm (RCDATA id 1) et l'icône
else()
  list(APPEND TETRIS_SOURCES
    platform/wincon_compat.cpp
    platform/ufmod_posix.cpp)
endif()

add_executable(Tetris ${TETRIS_SOURCES})

target_include_directories(Tetris PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
  # winmm vient du SDK Windows — ne jamais vendoriser une copie.
  # ufmod.lib est la bibliothèque tierce livrée avec le dépôt.
  target_link_libraries(Tetris PRIVATE
    winmm
    ${CMAKE_CURRENT_SOURCE_DIR}/ufmod.lib
  )
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(XMP REQUIRED IMPORTED_TARGET libxmp)
  pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

  set(CURSES_NEED_NCURSES TRUE)
  if(NOT APPLE)
    # Sur Debian et dérivés, le wide ncurses est un paquet distinct (libncursesw).
    # macOS livre un ncurses déjà compatible large sous <curses.h>, que
    # CURSES_NEED_WIDE ne sait pas reconnaître.
    set(CURSES_NEED_WIDE TRUE)
  endif()
  find_package(Curses REQUIRED)
  target_include_directories(Tetris PRIVATE ${CURSES_INCLUDE_DIRS})
  target_link_libraries(Tetris PRIVATE
    ${CURSES_LIBRARIES}
    PkgConfig::XMP
    PkgConfig::SDL2
  )
endif()

# Le jeu est écrit pour un build UNICODE : buffers wchar_t, littéraux L"...",
# et wsprintf qui doit se résoudre en wsprintfW. Le .vcxproj d'origine porte
# CharacterSet=Unicode ; retirer ces deux définitions casse la compilation.
target_compile_definitions(Tetris PRIVATE UNICODE _UNICODE)

if(NOT WIN32)
  # Sans ressources embarquées, uFMOD_PlaySong lit a.xm posé à côté du binaire.
  add_custom_command(TARGET Tetris POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/a.xm $<TARGET_FILE_DIR:Tetris>/a.xm)
endif()

if(MSVC)
  # Le code de 2008 est antérieur aux fonctions « sécurisées » du CRT.
  target_compile_definitions(Tetris PRIVATE _CRT_SECURE_NO_WARNINGS)
  # Les sources ont été converties d'ISO-8859-1 en UTF-8 lors du portage.
  target_compile_options(Tetris PRIVATE /utf-8)
endif()

# Les scores sont lus et écrits à côté de l'exécutable.
foreach(data score.data splus.data)
  add_custom_command(TARGET Tetris POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${data} $<TARGET_FILE_DIR:Tetris>/${data})
endforeach()
