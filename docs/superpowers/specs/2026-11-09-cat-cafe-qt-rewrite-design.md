# Cat Café: C++ / Qt rewrite — design

Date: 2026-11-09
Status: design agreed in conversation; this document is the written form. Read by a fresh
reviewer on 2026-11-09 and corrected. Two questions are open for the owner (section 12);
each has a default, so work is not blocked.

Throughout this document, **"the owner"** is the person who wrote the original Java game
and asked for this rewrite. Statements marked **(owner)** are things they said or decided.
Statements marked **(my call)** were chosen by the designer and accepted by the owner as
part of the overall description; they can be revisited without going back on anything the
owner specifically asked for.

---

## 1. Intent

The owner wrote a Java Swing game, "Cat Café" (`D:\Projects\Cat-cafe-game`, 27 `.java`
files, no build system, assets beside the source). They wrote it without AI and understand
its architecture completely.

They want it rewritten in C++ with the Qt framework, **as a way to learn Qt**. Because they
already know what every part of the game does, reading the Qt version of something familiar
lets them concentrate on the framework instead of the problem.

**Why now (owner):** the next thing they want to build in Qt is a *competitive-programming
algorithm visualizer*: users draw a graph in a graph editor, then follow an algorithm
running on it visually. The café rewrite is the practice run for that.

**The owner's C++ level (owner):** comfortable with C++ *syntax* from competitive
programming, and has used `struct`s there, but has never written a real C++ class. They
know object-oriented programming well from Java (inheritance, abstract classes, interfaces,
statics).

Two deliverables, equally important:

1. The game, rewritten in idiomatic C++/Qt, in a new folder.
2. A guide that uses that code to teach (a) C++ classes to someone who knows Java classes
   and (b) how Qt works and how C++ code plugs into it.

## 2. Goals

- The rewritten game plays the same as the original: same art, same coordinates, same
  prices, same rules, same pacing — except for the listed fixes (section 7).
- The code is written the way an experienced Qt developer would write it, **not** a
  line-for-line translation **(owner: "go with the one that makes more sense for native
  Qt")**.
- The class names and hierarchy the owner already knows are kept wherever a Qt developer
  would plausibly keep them, so they can find their bearings.
- The parts of Qt that the visualizer will need get the most care and the clearest code:
  a scene of objects, timers driving steps, control panels talking to the scene through
  signals and slots.
- Everything the code does is explained somewhere in the guide.
- Things in the Java that look unintended are fixed, and every fix is listed in the guide
  **(owner)**.
- A `CLAUDE.md` in the new folder always describes the agreed decisions and current
  progress, so that a later session can continue the work without the conversation
  **(owner)**.

## 3. Non-goals

- New gameplay, new art, new sounds, rebalancing.
- QML / Qt Quick, Qt Designer `.ui` files **(my call:** all UI is built in C++ so the
  owner reads one language and sees every object being created).
- An installer or a redistributable package. It needs to build and run on the owner's
  machine.
- Cross-platform polish. Windows 11 is the target; nothing should be gratuitously
  Windows-only in the C++ itself, but only Windows is verified.
- Save games, settings persistence, resizable or scalable window.
- Touching the original Java project in any way.

## 4. Anti-goals (would make this a failure even if it runs)

- **Java with C++ syntax.** Static lists of every object, `instanceof` chains, a dozen
  empty overrides per subclass, panels pulling state through global static functions,
  manual "repaint everything" calls. These are exactly what the owner wants to see the Qt
  alternative to.
- **Cleverness the guide doesn't explain.** Templates beyond using standard containers,
  macros beyond Qt's own, PIMPL, CRTP, operator overloading for its own sake, dense lambda
  chains. Prefer the plain construct. If a C++ or Qt feature appears in the code, the guide
  explains it; if it is not worth explaining, it should not be in the code.
- **A generic Qt tutorial.** The guide must be anchored on this project's real files, with
  snippets that match the code as it finally exists.
- **A game that feels different.** Different speeds, different layout, different timing.

## 5. Constraints

Environment (verified on the owner's machine on 2026-11-09):

| Thing | Location |
|---|---|
| Qt 6.11.2, MinGW 64-bit kit | `C:\Qt\6.11.2\mingw_64` (has Core, Gui, Widgets, Multimedia, Svg, Test, Qml, Quick) |
| Compiler (g++ 13.1, matches the kit) | `C:\Qt\Tools\mingw1310_64\bin` |
| CMake | `C:\Qt\Tools\CMake_64\bin\cmake.exe` |
| Ninja | `C:\Qt\Tools\Ninja\ninja.exe` |
| Qt Creator | `C:\Qt\Tools\QtCreator\bin\qtcreator.exe` |

- **None of these are on `PATH`.** Any script or command must set up `PATH` itself.
- There is also an unrelated MSYS2 g++ at `C:\msys64\ucrt64\bin`. It must not be used with
  the Qt MinGW kit (different runtime); use the compiler under `C:\Qt\Tools`.
- Visual Studio Build Tools 2022 are installed, but there is no MSVC Qt kit. Use MinGW.
- Windows 11; default shell is Windows PowerShell 5.1.
- New project location **(owner: "a new folder under Projects")**:
  `D:\Projects\Cat-cafe-game-qt` **(name: my call)**.
- **No git repository and no commits unless the owner asks** (my call, accepted). Progress
  is tracked in `CLAUDE.md` instead.
- The owner checks token usage between phases. Work should be done in stages that each
  leave the project building and `CLAUDE.md` current.

## 6. How it gets made

### 6.1 Technology (my calls)

- C++17.
- Qt 6 Widgets for windows, panels, buttons; the Graphics View framework
  (`QGraphicsScene` / `QGraphicsView` / `QGraphicsItem`) for the café itself;
  Qt Multimedia for sound.
- CMake, with a `CMakeLists.txt` that Qt Creator opens directly.
- `build.bat` and `run.bat` at the project root, in the spirit of the original `open.bat`,
  which set `PATH` and call CMake/Ninja so the owner can build and play without an IDE.
- Qt Test for automated tests of the game rules.

### 6.2 The central idea: world vs. controls (owner approved)

Everything on the game screen is one of two kinds of thing:

- **World** — background, counters, prep tables, bins, tables, chairs, walls, customers,
  the player, the "press E" pop-ups, the "out of stock" message. These are *items in a
  `QGraphicsScene`*. Qt draws them, orders them by z-value and keeps track of where they
  are.
- **Controls** — the current-item box, the Restock / Orders / Upgrades panels, the three
  panel buttons, sound and music toggles, the back button, the banner, the money readout.
  These are ordinary `QWidget`s positioned on top of the view that shows the scene.

This is deliberately the same split the visualizer will have (graph canvas vs. control
panels).

Painting by hand (`paintEvent` + `QPainter`, the direct equivalent of Swing's
`paintComponent`) is still used where it is the natural tool: the menu and instructions
screens, the image buttons, and the panels that draw text over a background image. So the
owner sees both approaches and when each fits.

### 6.3 What is kept from the Java, and what changes (owner approved)

Kept:

- The hierarchy `NonCollidable` → `Interactable` → `Counter` / `PrepTable` / `Trashbin` /
  `Chair`, and `NonCollidable` → `Decoration`. (Singular names; the Java used plurals.)
- `Item` → `Ingredient`, `Item` → `Cup`. `Customer`. `Player`.
- Three timers with the original periods: 16 ms (player movement and pop-up check),
  10 ms (customer movement), 1000 ms (customer spawning and renumbering).
- All coordinates, sizes, prices, speeds, probabilities and rules (the Java source is the
  reference for these; see section 8 for the traps).

Changed, and why:

1. **No static lists of objects.** `Interactables.interactableArr`, `Customer.customers`,
   `NonCollidables.objects`, `Item.items`, `Chair.chairs` and the rest go away. The scene
   owns its items and keeps the typed lists it needs as ordinary members.
2. **No `instanceof` chain.** `Player.itemChange()` and the abstract-methods-with-empty-
   overrides in `Interactables` become one pure virtual function,
   `Outcome interact(Player &player)`, implemented by each of the four concrete classes
   with only the logic that belongs to it. It returns what happened (took an item, counter
   empty, served the right order, and so on) and the scene decides what the rest of the
   game does about that. The items therefore know nothing about the scene and can be
   tested on their own.
3. **Static calls become signals and slots.** Instead of panels calling
   `Game.getPlayerMoney()` every repaint, the player emits `moneyChanged` and whoever cares
   connects to it. Screen switching, hover-click sounds, meows, the orders list refreshing
   and the toggle icons all work this way.
4. **`Liquid` and `Topping` are not classes.** In the Java they existed only to hold the
   static lists used for picking a random order. They become a `kind` field in a small
   catalog of item definitions.
5. **Customers move themselves.** In the Java a chair moved "its" customer, which is the
   root of fix F1 below. Each customer now carries its own route and state.
6. **`MyPanel` is gone.** It existed only for a static helper and the hover-click
   listener; neither is needed.

### 6.4 Structure

```
Cat-cafe-game-qt/
  CMakeLists.txt  CLAUDE.md  README.md  build.bat  run.bat
  assets/
    images/   *.png           only the images the game loads
    sounds/   *.wav
    fonts/    WaterlilyScript.ttf
  docs/
    GUIDE.md
    superpowers/specs/   this document
    superpowers/plans/   the implementation plan
  src/
    main.cpp
    MainWindow.{h,cpp}
    Assets.{h,cpp}                 loading / scaling / caching pixmaps, the game font
    screens/  MenuScreen  InstructionsScreen  GameScreen
    world/    CafeScene  NonCollidable  Decoration  Interactable
              Counter  PrepTable  Trashbin  Chair  Customer  Player
              SpriteAnimation
    items/    Item  Ingredient  Cup  ItemCatalog
    panels/   HeldItemPanel  RestockingPanel  OrderPanel  OrderCard  UpgradesPanel
    ui/       ImageButton
    audio/    SoundManager
  tests/      Qt Test sources
```

`tests/` and `Assets` were not in the layout shown to the owner; they are additions
(my call).

Responsibilities (what each unit is for; exact member names are the builder's):

**App shell**

- `main.cpp` — creates `QApplication`, registers the font, sets the application font,
  creates and shows `MainWindow`, runs the event loop.
- `MainWindow` — the window (fixed client size 1000 × 665, see section 8). Holds a
  `QStackedWidget` with the three screens (replaces `JFrame` + `CardLayout`). Owns the
  `SoundManager`. **It is the one place where the parts are wired together with
  `connect`**: screen switching, starting/stopping the game and the instructions
  animation, music start/stop, click-on-hover, meows, toggle buttons. No screen knows
  about another screen or about the sound manager.
- `Assets` — small free functions: load a pixmap from the resource system by name, load it
  scaled to a size (smooth, cached), and build the game font at a pixel size. Replaces
  `Main.getResizedImage` and `Main.getCustomFont`.

**Screens** (each a `QWidget`, each emits signals rather than switching screens itself)

- `MenuScreen` — paints the menu background; Play and Instructions buttons.
- `InstructionsScreen` — paints the instructions page and runs the small looping animation
  (customer walks down to a chair, sits, repeats) from its own 30 ms `QTimer`. Start/stop
  from outside.
- `GameScreen` — holds the `QGraphicsView` showing the `CafeScene`, and all the controls
  layered over it. Handles the keyboard (W/A/S/D held state, E to interact). Opens and
  closes the three panels (only one open at a time; opening one stops the player; the
  banner shows which is open). Keeps keyboard focus on itself: buttons, the combo box and
  the view do not take focus (this replaces the Java `addNotify()` workaround).

**World**

- `CafeScene` (`QGraphicsScene`) — builds the café (the layout in the `Game` constructor),
  owns every world item, owns the three timers and the spawn odds. Each timer is connected
  to one slot. Finds which interactable the player can reach and shows the pop-up beside
  it. On E it asks that interactable to `interact` and turns the outcome it returns into
  the scene's signals (stock changed, counter empty, order served, orders changed).
  Exposes what the panels need (player, counters, seated customers, catalog). Start/stop
  the simulation from outside.
- `NonCollidable` (`QGraphicsPixmapItem`) — something the player cannot walk through: an
  image at a position, plus a solid rectangle (usually the image bounds; `Chair` shrinks
  it).
- `Decoration` — a `NonCollidable` with nothing added (tables, walls).
- `Interactable` — a `NonCollidable` the player can use: has a reach rectangle (the Java
  `hitBox`), knows where its pop-up goes, a virtual "can this be used right
  now" (default yes), and the pure virtual `interact`.
- `Counter` — item name and stock. Empty-handed player takes one (a new `Cup` from the cup
  counter, otherwise a new `Ingredient`); if stock is zero, reports "counter empty".
- `PrepTable` — may hold one cup. Player with a cup puts it down; empty-handed player
  picks it up; player with an ingredient adds it to the cup on the table. The cup on the
  table is drawn as a child item.
- `Trashbin` — whatever the player holds is discarded.
- `Chair` — which way it faces, its route from the door, and the customer assigned to it
  (if any). Usable only while that customer is seated. A player holding a cup serves it:
  right order pays, wrong order costs, the customer leaves either way.
- `Customer` (`QGraphicsPixmapItem`) — its order (a `Cup`), its route, and a state:
  arriving → seated → leaving → gone. Steps one pixel per 10 ms tick, choosing its sprite
  by direction. Shows its number bubble as a child item while seated.
- `Player` (`QObject` + `QGraphicsPixmapItem`) — position, speed, held keys, the held item,
  money, tips. Moves and collides each 16 ms tick, picks its animation by direction. Emits
  `moneyChanged`, `heldItemChanged`, `tipsChanged`. The small icon of the held item beside
  the player is a child item.
- `SpriteAnimation` — cuts a sprite sheet into frames and hands out the next frame
  (the Java `Animations`).

**Items** (plain C++, no Qt object machinery; this is where ownership is taught)

- `ItemCatalog` — the ten item definitions (name, kind: cup / liquid / topping / extra,
  cost, image); lookup by name; random liquid; random topping; makes a new item by name.
- `Item` — abstract: a name and an image.
- `Ingredient` — one ingredient.
- `Cup` — a list of ingredient names; knows whether it matches another cup (same
  ingredients in any order); its image is the empty or filled cup; makes random orders
  (one liquid, one topping, sugar 50 %, ice 50 %).

The player holds its item through a `std::unique_ptr<Item>`, and a prep table holds its cup
through a `std::unique_ptr<Cup>`. Putting a cup down and picking it up *moves* ownership
between them. This is intentional: it is the guide's worked example of C++ ownership
(there is no garbage collector; exactly one owner; handing something over is a move).
Table to player is a plain `std::move`. Player to table is not, because the player holds
an `Item` and the table wants a `Cup`: that direction needs a `dynamic_cast` and a
`release()`, kept inside one `Player` function. The guide uses both directions.
Qt-owned objects (widgets, scene items, anything with a `QObject` parent) use raw pointers
and Qt's parent–child ownership, which is the Qt convention; the guide contrasts the two.

**Panels** (widgets; each is given what it needs through its constructor)

- `HeldItemPanel` — the "Current Item" box: paints the held item's name and image, and a
  cup's ingredients. Repaints when the player's held item changes.
- `RestockingPanel` — combo box of counters (sorted by name), +1 / −1 / +10 / −10, the
  Restock button (grey when the player cannot afford it), and painted readouts. Updates
  live when money or stock changes.
- `OrderPanel` + `OrderCard` — a scroll area with one card per seated customer (number,
  ingredients, cost, payout), laid out with a real Qt layout. Rebuilt when the scene says
  orders changed or the tip level changes.
- `UpgradesPanel` — the four upgrades with their levels and costs; button image reflects
  upgrade / insufficient / max.

**UI and audio**

- `ImageButton` — a button that is just an image, with an optional hover image (and
  optional hover size, for the back button that grows on hover). Emits a signal when the
  mouse enters it. Never takes keyboard focus.
- `SoundManager` (`QObject`) — the four sounds, the master sound flag and the music flag,
  slots to play/toggle, signals when a flag changes.

### 6.5 Decisions the builder should not have to guess

- **Z-order in the scene** reproduces the Java paint order: background, then the player,
  then solid things (so a chair's image can overlap the player), then customers, then
  pop-ups, then the "out of stock" message on top. Because pop-ups must be above customers
  (a chair's "E" pop-up covers the seated customer's number bubble in the Java), the
  pop-up is a scene-level item with a high z-value, not a child of its interactable. Only
  one interactable is in reach at a time, so the scene keeps a single pop-up item and
  moves it.
- **The three timers live in `CafeScene`**; the instructions animation timer lives in
  `InstructionsScreen`. All are stopped when their screen is not showing.
- **Pacing (my call).** The timers are precise and use the periods written in the Java
  source (16, 10, 1000 ms). The reviewer measured `javax.swing.Timer` on the owner's
  machine: the 16 ms timer really fires about every 17.2 ms and the 10 ms one about every
  11.1 ms, and the real game is probably slower still, because it rescales about forty
  images per frame on the same thread. So the Qt version may run around 10 % faster than
  what the owner has played. Keep the source's numbers; at the end, time both games side
  by side and bring any noticeable difference to the owner with the timings. Do not retune
  silently.
- **Key auto-repeat is ignored** for all keys.
- **Hover click sound** is wired in one place in `MainWindow` by finding every
  `ImageButton` in the window's object tree (`findChildren`) and connecting its hover
  signal. This is deliberate: it is the guide's demonstration of the QObject tree.
  Upgrade buttons that cannot be bought (showing "insufficient" or "max") are disabled and
  stay silent on hover; the Java does this on purpose (`UpgradesPanel.java` 105–113).
- **Sounds:** short effects with `QSoundEffect`; background music with `QMediaPlayer` +
  `QAudioOutput`, looping, at roughly −10 dB (≈ 0.32 linear), as in the Java. If
  `QMediaPlayer` proves unreliable on this machine, falling back to a looping
  `QSoundEffect` is acceptable.
- **Assets:** images, the font and the three short sounds are embedded with the Qt
  resource system. The 13 MB `backgroundMusic.wav` is **not** embedded; CMake copies it
  beside the executable and it is loaded from disk, so compiles stay fast. If it is
  missing the game runs silently rather than failing. The guide explains both mechanisms.
- **Asset files are copied** from the original folder, never moved. Only files the game
  loads are copied (not the `- Copy` duplicates, `bird.jpg`, `layout.png`, and other
  unused images). `Waterlily Script.ttf` is renamed without the space.
- **Fonts:** Java font sizes are effectively pixels; use pixel sizes in Qt (14 default,
  and 12 / 18 / 24 / 36 / 48 where the Java asks for them).
- **Randomness:** `QRandomGenerator`. Game rules that involve chance are written so the
  tests can exercise them deterministically.
- **Style:** `m_` prefix on members; `connect` with function pointers (not the
  string-based `SIGNAL`/`SLOT` macros); `override` on every override; no
  `using namespace std;` in headers (the owner's competitive-programming habit; the guide
  says why); Qt types (`QString`, `QList`, `QRect`, `QPixmap`) at Qt boundaries; comments
  that explain *why Qt wants it this way* where a Swing programmer would be surprised.
- **Build layout:** the game code is built as one library target that both the game
  executable and the test executable link, so tests can exercise it.
- **Compiler warnings** are on (`-Wall -Wextra`) and the code builds without warnings.

### 6.6 Left to the builder

Exact class member names and signatures; how `Assets` caches; whether the background is an
item or drawn by the scene; file-level organisation of tests; whether pixmaps are prepared
at device pixel ratio for sharper high-DPI output (nice to have, not required); the exact
mechanism of the floating "+$7" text (a short Qt animation is preferred, since the
animation framework is one of the guide's topics); how deterministic randomness is
arranged for tests; anything not stated here.

## 7. Fixes relative to the Java (all to be listed in the guide)

The owner asked for things that look unintended to be fixed and mentioned in the guide.
Everything else must behave as the Java does.

Fixes the owner has seen and approved:

- **F1 — Frozen customer.** A chair is marked available the moment its customer stands up,
  but still drives that customer's walk to the door. If a new customer is assigned to the
  chair meanwhile, the old one is orphaned and stays on screen forever. Fix: customers
  move themselves; the chair is free immediately (as in the Java) and both customers
  behave.
- **F2 — First Advertisement upgrade does nothing.**
  `Chair.setChanceDivisor(advertisementLevel++)` passes the level *before* the increment,
  so every purchase applies the previous level's bonus. Fix: apply the new level.
- **F3 — Sound-off does not silence effects.** Meows and clicks ignore the master flag.
  Fix: master off silences everything. Turning master back on turns music on as well
  (this matches what the Java shows on screen) and leaves the two flags consistent.
- **F4 — Order card over-promises.** The card shows `round(cost × 1.5) + tips`; the player
  is paid `floor(cost × 1.5) + tips`. Fix: one payout function (the `floor` one, so actual
  earnings are unchanged) used by both.

Smaller ones found while reading (my call to include; same rule — fix and list):

- **F5** — An *empty* chair within reach blocks the pop-up of the next interactable
  (first-in-list wins even when it cannot be used), so standing between an empty chair and
  an occupied one can make the occupied one unservable. Fix: the first *usable*
  interactable in reach wins.
- **F6** — Counters are registered in the solid-object list twice (drawn and
  collision-checked twice). Once now.
- **F7** — All customers share one animation frame counter, so they animate faster the
  more of them are walking. Each customer keeps its own.
- **F8** — Holding E repeats the interaction at the keyboard repeat rate. One press, one
  interaction.
- **F9** — The Restock panel's "Remaining" goes stale if the player takes items while the
  panel is open. It updates live.
- **F10** — The music toggle has no hover listener (the sound toggle has two), and the
  Upgrades panel re-adds its hover listener on every repaint. Every usable button clicks
  once on hover. Upgrade buttons showing "insufficient" or "max" stay silent, as the Java
  intends.
- **F11** — Debug `System.out.println` calls are not carried over.
- **F12** — A cup picked up from a prep table is labelled "Cup", one from the counter
  "cup". Always "cup".
- **F13** — The Java window is 1000 × 700 *including* title bar and borders, which clips
  the 1000 × 665 art by a few pixels. The Qt window's client area is exactly 1000 × 665.

Kept on purpose (looks odd, but is game feel, not a defect): the player stops dead on
collision rather than sliding along walls; money can go negative; the −$3 penalty for a
wrong order; diagonal customer steps.

Differences that come from the toolkit rather than from a decision (the guide lists them
as such): Swing's default button border is not reproduced on the image buttons; the combo
box and scroll bar look like Qt's; text is rasterised slightly differently.

## 8. Facts about the original that are easy to get wrong

- **Argument order.** `NonCollidables`, `Interactables` and every subclass constructor
  take `(image, x, y, h, w, …)` — **height before width**. E.g. the wall
  `new Decorations(wall, 448, 75, 275, 4)` is 4 wide and 275 tall. The rewrite should take
  rectangles in the normal order and convert once, in the scene-building code.
- **Art size.** `background.png`, `menuBackground.png` and `instructionsBackground.png`
  are 1000 × 665. Player movement limits stay at the Java's values (x in (0, 960),
  y in (0, 620)); do not derive them from the new window height.
- **Sprite sheets** are grids of 200 × 200 frames, scaled to 40 × 40: `stationary`
  600 × 200 (3 frames), `rollLeft`/`rollRight` 1400 × 400 (14), `rollUp`/`rollDown`
  1400 × 800 (28), `customerUp`/`customerDown` 1400 × 800 (28),
  `customerLeft`/`customerRight` 1400 × 600 (21). `Animations.getNextImage()`
  pre-increments, so the first frame returned is index 1.
- **Player.** 40 × 40, starts at (100, 100), speed 2.2 px per tick, diagonal speed
  normalised. Body rectangle for collisions is `(x+5, y+5, 30, 30)`. The reach rectangle
  is `(x+2, y+2, 44, 44)` (`Player.java` 165): it is *shifted*, not centred, so it reaches
  6 px past the sprite on the right and bottom and stops 2 px inside it on the left and
  top (section 12, Q2).
  On any collision or out-of-bounds, *both* axes of that tick's movement are undone.
  Sprite priority: right, left, up, down, else stationary.
- **Paint order** of the game screen in Java: background, banner, player (and the
  current-item box, money text and floating money text), solids (with prep-table cups),
  customers (with number bubbles), pop-ups, message.
- **Customers** spawn at (980, 180) at 40 × 40, walk the chair's route one pixel per tick,
  and when seated are drawn as the 75 × 75 "seated" chair image exactly over the chair.
  Leaving, they walk the route backwards and vanish at its first point.
- **Spawn odds** (once a second, only if some chair is free): roll
  `int(random × chance / divisor)`; then `chance = int(chance / 1.08)`; on a roll of 0, a
  customer spawns on a random free chair and `chance = 100 × (customers present + 1)`.
  `chance` starts at 10, `divisor` at 1, and `divisor = 1 + level^1.1 / 10` after
  Advertisement upgrades.
- **Money.** Starts at 1000. Order cost = 1 (the cup) + the cost of each ingredient
  (2 each, sugar and ice 1). Correct order pays `floor(cost × 1.5) + tips`; wrong order
  costs 3. The floating text shows the change for 2 s, drifting down.
- **Text colours.** Current-item box text `#5097B9`; money green `rgb(21,120,17)`; loss
  red; Upgrades panel text white.
- **Panels.** Restock and Orders cover the current-item box at (0, 435, 375 × 150);
  Upgrades is 400 × 400, centred on the Java's 1000 × 700 (i.e. at (300, 150)). The
  "out of stock" message is 500 × 100 at (250, 300), shown for 1 s.
- **Sound.** Play (from the menu) starts the music only if both flags are on; Back stops
  it.

## 9. The guide

`docs/GUIDE.md`, one Markdown file **(my call:** it lives beside the code it explains and
reads in any editor or on GitHub). Plain ASCII diagrams only.

Written for this reader specifically: knows Java OOP and Swing well; knows C++ syntax, the
STL and structs; has never written a C++ class; has never used Qt. It should not explain
what a class or inheritance *is*; it should explain what C++ and Qt do differently from
Java and Swing, and why.

Every chapter is anchored on a real file in the project and shows the owner's Java beside
the C++ where that helps. Snippets must match the final code. The guide is written after
the code is finished, not before.

1. **C++ classes for a Java programmer** — header and source files; constructors and
   member initializer lists; `const`; values, references and pointers; `new` without a
   garbage collector and `nullptr`; `virtual`, `override`, `= 0`, virtual destructors;
   `dynamic_cast` versus `instanceof`; `std::unique_ptr` and `std::move` (the cup changing
   hands); why headers avoid `using namespace std;`.
2. **How Qt works** — the build (CMake, what `moc` generates and why `Q_OBJECT` exists,
   the resource system); `QApplication` and the event loop; `QObject` parent–child trees
   as Qt's answer to "who deletes this?"; signals and slots (versus `ActionListener`);
   widgets, events and painting; timers; the Graphics View framework (scene, view, items,
   coordinates, z-order, child items); layouts and scroll areas; the animation framework;
   sound.
3. **Reference** — Swing → Qt and Java → C++ lookup tables; a map from each Java file to
   where its job went; the list of fixes (section 7); how to build, run and test; and
   **"From café to graph visualizer"**: which pieces carry over and how (nodes and edges as
   scene items, dragging and selection, a step timer with play/pause, control panels
   driving the scene through signals), with a few small exercises to try in this codebase
   first.

The code's own comments complement the guide: brief, placed where a Swing programmer would
be surprised, and not a substitute for it.

## 10. Verification

- Automated (Qt Test): the rules that can be checked without eyes — cup matching, order
  cost and payout, catalog lookups, counter stock, the prep-table hand-over, serving right
  and wrong orders, chair/customer state changes including the F1 scenario, spawn odds
  arithmetic, player movement and collision, upgrade and restock arithmetic.
- By eye: the builder runs the game at each stage and compares it with the Java version
  (which runs on this machine via `open.bat`). Rendering a screen to an image file from a
  test and looking at it is an acceptable way to check layout without a human at the
  keyboard.
- Done means: builds with no warnings; tests pass; every screen, panel, sound and rule
  works when played; the guide is complete and agrees with the code; `CLAUDE.md` is
  current.

## 11. Order of work

`CLAUDE.md` first. Then stages that each end with something that builds and runs: project
skeleton with menu and screen switching; the world and the player; items and
interactions; customers; panels; sound; the guide last. `CLAUDE.md` is updated at the end
of every stage.

## 12. Open questions for the owner

Raised by the review. Each has a default that the plan implements, so nothing is blocked;
the owner's answer may change it.

- **Q1 — Should the keyboard be dead while the Upgrades panel is open?** In the Java, the
  Restock and Orders buttons hand keyboard focus back to the game (`Game.java` 229–242),
  but the Upgrades button does not (243–247). So W/A/S/D/E do nothing until Upgrades is
  closed, while customers keep arriving. The panel covers the middle of the café.
  *Default: copy the Java* (those keys are ignored while Upgrades is open). If the owner
  says it was an oversight, the player moves freely as with the other two panels, and
  that becomes fix F14.
- **Q2 — Is the lopsided reach rectangle intended?** `Player.java` 165 builds
  `(x+2, y+2, 44, 44)`, which reaches further right and down than left and up. A centred
  one would be `(x-2, y-2, 44, 44)`. *Default: copy the Java exactly*, because the game
  was tuned by playing with it. If the owner says it was a slip, centre it; that becomes
  fix F15 and changes a few numbers in the tests.
