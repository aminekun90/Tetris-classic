# Tetris-classic — build moderne d'un jeu console Win32 de 2008.
#
# La cible reste x86 32 bits : ufmod.lib, la bibliothèque qui joue le module
# .xm embarqué dans les ressources, n'existe qu'en i386. Configurer avec -A Win32.
cmake_minimum_required(VERSION 3.20)
project(Tetris VERSION 1.3.0 LANGUAGES CXX)

if(NOT WIN32)
  message(FATAL_ERROR
    "Tetris-classic utilise l'API console Win32 (CreateConsoleScreenBuffer, "
    "WriteConsoleOutput, uFMOD). Il ne compile que sous Windows.")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR
    "Build 64 bits demandé, mais ufmod.lib est en i386. "
    "Reconfigurer avec :  cmake -B build -A Win32")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Tetris
  tetris.cpp
  fonctions.cpp
  filehandler.cpp
  music.rc            # embarque a.xm (RCDATA id 1) et l'icône
)

target_include_directories(Tetris PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# winmm vient du SDK Windows — ne jamais vendoriser une copie.
# ufmod.lib est la bibliothèque tierce livrée avec le dépôt.
target_link_libraries(Tetris PRIVATE
  winmm
  ${CMAKE_CURRENT_SOURCE_DIR}/ufmod.lib
)

# Le jeu est écrit pour un build UNICODE : buffers wchar_t, littéraux L"...",
# et wsprintf qui doit se résoudre en wsprintfW. Le .vcxproj d'origine porte
# CharacterSet=Unicode ; retirer ces deux définitions casse la compilation.
target_compile_definitions(Tetris PRIVATE UNICODE _UNICODE)

if(MSVC)
  # Le code de 2008 est antérieur aux fonctions « sécurisées » du CRT.
  target_compile_definitions(Tetris PRIVATE _CRT_SECURE_NO_WARNINGS)
  # Les sources sont en ISO-8859-1, pas en UTF-8.
  target_compile_options(Tetris PRIVATE /source-charset:.1252)
endif()

# Les scores sont lus et écrits à côté de l'exécutable.
foreach(data score.data splus.data)
  add_custom_command(TARGET Tetris POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${data} $<TARGET_FILE_DIR:Tetris>/${data})
endforeach()
