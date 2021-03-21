#  RPGME

RPGME is a player plugin for RetroPlayer based on Game Music Emu (https://bitbucket.org/mpyne/game-music-emu/wiki/Home).

The GME source tree is located in the Source/gme folder and is a Git submodule and can be checked out with the following commands:

    git submodule init
    git submodule update

To configure the build the following defines have to the Preprocessor Macros has been set in Build Settings

1. BLARGG_BUILD_DLL for building GME as a library.
2. VGM_YM2612_NUKED to use the YM2612 emulation with the best sound quality.
