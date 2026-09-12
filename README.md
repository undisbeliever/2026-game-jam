undisbeliever's 2026 SNESDEV Game Jam Entry
===========================================

I am making a small prototype SNES game in C to validate a moving
platform design for Space Rescue Squad.

I intend to blog about the game as I develop it.  Posts about the game
will be published on
[my website](https://undisbeliever.net/tags/2026-game-jam.html).


Licensing
=========

This repository is a non-commercial homebrew SNES game.

It contains code written for the vbcc65816 compiler, which has a
non-commercial use clause.

All code in the `src` directory is licensed under the
[zlib license](LICENSE).


Build Requirements
==================

 * Python 3.10+
 * [Python Pillow Imaging Library](https://python-pillow.github.io)
 * [JCC816](https://github.com/Iacon1/JCC816)
 * [llvm-mos](https://github.com/llvm-mos/llvm-mos-sdk/releases/)
 * [vbcc for 65816](http://www.compilers.de/vbcc.html)
 * GNU make


Build Instructions
==================

 1. Install python-pillow, in either a venv, user profile or with a
    package manager.
 2. Setup the following environment variables:
    * `JCC816_DIR` pointing to the directory of the JCC816 repository.
    * `VBCC` pointing to the vbcc package.
      (`[...]/vbcc65816/vbcc65816_linux/vbcc` for linux or
      `[...]/vbcc65816/vbcc65816_win/vbcc` for windows.)
    * `PATH` containing the llvm-mos bin directory.
 3. Run `make` in the repository root.  
    Alternatively, you can run `make jcc816`, `make llvm-mos`, or
    `make vbcc` to compile the game with a specific C compiler.

The makefile will create 3 different copies of the game a `build`
directory.

