# Cat Café (C++ / Qt)

A rewrite of the Java Swing game in `D:\Projects\Cat-cafe-game` using C++17 and Qt 6.
Same game, same art, same rules; written the way a Qt program is normally written.

**To learn from it, read [`docs/GUIDE.md`](docs/GUIDE.md).** It uses this code to explain
C++ classes to a Java programmer, and how Qt works.

## Run it

Double-click `run.bat`, or from a terminal in this folder:

| Command | What it does |
|---|---|
| `run.bat` | Builds if needed, then starts the game |
| `build.bat` | Builds the game and the tests into `build\` |
| `test.bat` | Builds, then runs every test |
| `test.bat items` | Builds, then runs only the tests whose name contains `items` |

Nothing needs to be on your `PATH`: the scripts point at the Qt installation in `C:\Qt`
(Qt 6.11.2, the MinGW 64-bit kit, and the CMake, Ninja and g++ that came with it). If Qt
is somewhere else on another machine, change the paths at the top of `env.bat` and in
`build.bat`.

If Windows refuses to start `build\CatCafe.exe` with "An Application Control policy has
blocked this file", that is Smart App Control, which blocks programs you build yourself.

## Controls

| Key | Action |
|---|---|
| W A S D | Move |
| E | Use the thing the "E" bubble is pointing at |
| Mouse | Buttons and panels |

## Open it in Qt Creator

1. Start Qt Creator (`C:\Qt\Tools\QtCreator\bin\qtcreator.exe`).
2. File → Open File or Project → choose `CMakeLists.txt` in this folder.
3. When asked for a kit, tick **Desktop Qt 6.11.2 MinGW 64-bit** and press Configure.
4. The green arrow builds and runs. Ctrl+click on any name jumps to where it is defined,
   and F1 on a Qt class opens its documentation.

Qt Creator builds into its own folder, so it does not disturb `build\`.

## What is where

```
CMakeLists.txt   how the project is built
env.bat build.bat run.bat test.bat
assets/          images, sounds and the font
docs/GUIDE.md    the guide
src/
  main.cpp       where the program starts
  MainWindow     the window; connects all the parts to each other
  Assets         loads pictures and the font
  screens/       MenuScreen, InstructionsScreen, GameScreen
  world/         the cafe: CafeScene and everything in it
  items/         what the player carries: Item, Ingredient, Cup, ItemCatalog
  panels/        Current Item box, Restock, Orders, Upgrades
  ui/            ImageButton
  audio/         SoundManager
tests/           one test program per tst_*.cpp
```
