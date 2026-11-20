# Cat Café — C++ / Qt rewrite

This folder is a rewrite of the owner's Java Swing game (`D:\Projects\Cat-cafe-game`,
read-only reference, never modify it) in C++17 with Qt 6. The owner wrote the Java
themselves and is using this rewrite **to learn Qt**; their next project is a
competitive-programming algorithm visualizer with a graph editor. They know Java OOP and
C++ syntax (structs, STL) but have never written a C++ class and have never used Qt.

Two deliverables, equally important: the game, and `docs/GUIDE.md`, which teaches C++
classes and Qt through this code.

## Read these before doing anything

1. `docs/superpowers/specs/2026-11-09-cat-cafe-qt-rewrite-design.md` — the agreed design:
   intent, goals, anti-goals, structure, the list of fixes F1–F13, facts about the Java
   that are easy to get wrong.
2. `docs/superpowers/plans/2026-11-09-cat-cafe-qt-rewrite.md` — the task-by-task plan
   (15 tasks in 7 stages) with exact names, signatures, coordinates and tests.
3. The **Progress** section at the bottom of this file.

## Rules the owner set

- Write it the way a Qt developer would, not line-for-line from the Java.
- Fix things in the Java that look unintended, and list every fix in the guide.
- Keep this file current: update **Progress** at the end of every task, so a new session
  can pick up from here. The owner checks token usage between phases — finish the task in
  hand, update this file, and stop when they say they want to check.
- No git repository and no commits unless the owner asks.
- Do not modify anything under `D:\Projects\Cat-cafe-game`. Assets are *copied* from it.

## Rules that follow from who the reader is

- Prefer the plain construct. No templates beyond standard containers, no macros beyond
  Qt's own, no PIMPL, no clever lambda chains. If a feature is used, the guide must
  explain it.
- No `using namespace std;` in headers. `m_` prefix on members. `override` on every
  override. `connect` with function pointers, never `SIGNAL()`/`SLOT()` strings.
- Qt-owned objects (widgets, scene items, anything with a parent) are raw pointers owned
  by their parent or scene. Game items the player carries are `std::unique_ptr`. This
  contrast is deliberate and is taught in the guide.
- All UI is built in C++. No `.ui` files, no QML.
- Comments explain why Qt wants something done a certain way, where a Swing programmer
  would be surprised. They do not narrate the code.
- The guide is written last, from the finished code. Its snippets must match the files.

## Toolchain (nothing is on PATH)

| Thing | Location |
|---|---|
| Qt 6.11.2 MinGW kit | `C:\Qt\6.11.2\mingw_64` |
| g++ 13.1 (the kit's compiler) | `C:\Qt\Tools\mingw1310_64\bin` |
| CMake | `C:\Qt\Tools\CMake_64\bin` |
| Ninja | `C:\Qt\Tools\Ninja` |
| Qt Creator | `C:\Qt\Tools\QtCreator\bin\qtcreator.exe` |

Do **not** use `C:\msys64\ucrt64\bin\g++.exe` (different runtime from the Qt kit).

PowerShell, from this folder:

```powershell
$env:PATH = "C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.2\mingw_64\bin;" + $env:PATH
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:/Qt/6.11.2/mingw_64
cmake --build build
ctest --test-dir build --output-on-failure
.\build\CatCafe.exe
```

Once Task 1 of the plan is done, `build.bat`, `run.bat` and `test.bat` do the same.

The original game runs with `open.bat` in `D:\Projects\Cat-cafe-game` (needs `javac` and
`java`, both on PATH) and is the reference for look and feel.

## Shape of the code (summary; the spec has the detail)

- `MainWindow` holds a `QStackedWidget` of three screens and is the only place that wires
  parts together with `connect`.
- The café is a `QGraphicsScene` (`CafeScene`) of items: `NonCollidable` → `Interactable`
  → `Counter` / `PrepTable` / `Trashbin` / `Chair`; `Decoration`; `Customer`; `Player`.
- Controls (panels, buttons, readouts) are `QWidget`s layered over the view.
- Each interactable implements one virtual `interact(...)`; there are no static object
  lists and no `instanceof` chains.
- State changes travel by signals (`moneyChanged`, `ordersChanged`, ...), never by
  polling global statics.

## Progress

Update this list as work proceeds. Tick a line only when it is really finished and
verified.

- [x] Requirements gathered with the owner (2026-11-09)
- [x] Design written: `docs/superpowers/specs/2026-11-09-cat-cafe-qt-rewrite-design.md`
- [x] Design reviewed by a fresh reader and corrected (2026-11-09)
- [x] Implementation plan written: `docs/superpowers/plans/2026-11-09-cat-cafe-qt-rewrite.md`
- [x] Owner said "continue" (2026-11-09). Executing inline in one session, no subagents,
      with the defaults for Q1 and Q2 (owner has been told both and can still change them)
- [x] Stage A — skeleton (Task 1): builds, 86 images embedded, tst_assets 9/9
- [x] Stage B — rules without a window (Task 2): tst_items 6/6
- [x] Stage C — shell and menu (Tasks 3–4): tst_widgets 7/7, tst_screens 5/5,
      tst_mainwindow 2/2; menu and instructions screenshots looked at and correct
- [x] Stage D — the world (Tasks 5–8): tst_player 11/11, tst_interactables 7/7,
      tst_customers 11/11, tst_cafescene 15/15, tst_gamescreen 7/7, tst_mainwindow 3/3;
      game screenshot looked at: kitchen, dining room, walls, cat and pop-up all in place.
      `CatCafe.exe` launches. The game is playable (walk, take, mix, serve) but has no
      HUD, panels or sound yet.
- [x] Stage E — controls (Tasks 9–12): tst_panels 16/16, tst_gamescreen 16/16, all 11
      test programs pass, zero warnings. Screenshots of the HUD and of each panel looked
      at (2026-11-10): text and pictures line up with the art.
- [x] Stage F — sound (Task 13): tst_sound 4/4, tst_mainwindow 6/6; all 12 test programs
      pass, zero warnings. A throwaway probe confirmed Qt plays `backgroundMusic.wav` here
      (QMediaPlayer playing, no error) and loads the click. **Not checked: nobody has
      listened to the game.** That is on the owner's list below.
- [x] Stage G — README and guide (Tasks 14–15): `README.md` and `docs/GUIDE.md` (26
      chapters, about 1800 lines) are written. Checked by script on 2026-11-10: all 327
      snippet lines in the guide appear verbatim in the source or in the Java; all 27 Java
      files and fixes F1–F13 are covered; every cited Java line number is right; every
      path exists. Clean rebuild: zero warnings, 12 test programs, 112 tests, all pass.
      A real-time test (`clocksDriveTheGame`) confirms the clocks run at the intended
      pace.
- [ ] **Left for the owner:** the by-eye and by-ear checks listed below; answers to Q1
      and Q2; and, if wanted, an independent code review (none has been run).

**All 15 plan tasks are done as far as an agent can take them.** If you are a new session:
there is nothing to build. Wait for the owner's findings from playing it. If they change
anything, edit the code test-first and keep `docs/GUIDE.md` in step (its `cpp` snippets
must match the source exactly; chapter 24 lists the fixes and the two open questions).
Run tests with `cmd /c "test.bat > build-log.txt 2>&1"` and read the lines
of `build-log.txt` that match `Test +#|tests passed|FAIL!|warning:`. A test showing
`***Not Run` straight after a build is a virus scanner holding the new exe; `test.bat`
retries it automatically, so read to the end of the log.

**State of the code (2026-11-10):** `run.bat` starts a playable game: menu, instructions,
walking, counters, prep tables, bins, customers, serving, all three panels, money. There
is no sound yet, no `README.md` and no `docs/GUIDE.md`.

### For the owner to check by playing (an agent cannot see or hear the running game)

Everything below is covered by automated tests where a test can cover it; these are the
parts that need a person. Run `run.bat`, and the Java with `open.bat` beside it.

- Sound: every button clicks on hover (not the greyed-out upgrade buttons); music starts
  on Play, loops, stops on Back, resumes where it left off; both toggles; happy and
  angry meows; with sound off nothing is audible.
- Feel: walking speed, diagonal speed and customer walking speed compared with the Java.
  Qt's timers are exact and the Java's run about 10 % late, so this version may feel a
  little faster. If it does, say so; the fix is two numbers in `CafeScene.cpp`.
- Look: anything that sits a few pixels off compared with the Java.
- The two open questions further down (Q1 Upgrades panel freezing the cat, Q2 reach box).

### Smart App Control (resolved 2026-11-09: the owner switched it off)

Kept for the record, in case a blocked `.exe` ever shows up again. While it was on:
**Smart App Control blocked `build\CatCafe.exe`** ("An Application
Control policy has blocked this file"; CodeIntegrity event 3077, policy
{0283ac0f-fff1-49ae-ada1-8a933130cad6}). It also blocks **some** freshly built test
programs: the verdict is per executable and unpredictable (five of six `tst_*.exe` were
allowed; `tst_player.exe` was blocked four times in a row; `tst_screens.exe` was blocked
once and allowed seconds later). In ctest a block shows as `***Not Run` with
`[unknown error]`. So the game cannot be launched here, and tests cannot be relied on to
run. Work stopped at this point on 2026-11-09 to ask the owner. Do not try to get around
this (no renaming, re-linking tricks, or running the code through another program).
Options are the owner's: switch Smart App Control off in Windows Security (on some
Windows versions it cannot be switched back on without resetting Windows), or sign the
executable with a trusted certificate. Until then, verify by tests and by screenshots that
tests write to `build\screenshots\`; the play-through steps (Task 14) need the owner.

### Ledger: rulings made while executing

- Setup: Ruling: no git, so no worktree and no superpowers ledger scripts; this section
  is the ledger — owner said no git — cost if wrong: no per-task history to roll back to.
- Task 1: Ruling: tests get `QT_FORCE_STDERR_LOGGING=1` (CMake) — on Windows Qt Test
  prints nothing when no console is attached, so failures were invisible — cost: none.
- Task 1: Ruling: `test.bat` retries failed tests once after ~4 s — a virus scanner
  briefly locks freshly linked exes ("resource busy or locked") — cost: a real failure
  takes 4 s longer to report.
- Task 1: Ruling: Step 6 "run.bat shows an empty window" not verified by launching,
  because of the blocker above; window size is covered by a test in Task 4 — cost if
  wrong: a launch-only problem surfaces when the owner first runs it.
- Task 3: Ruling: `ImageButton` does not set a pointing-hand cursor although the plan
  says so — the Java shows the normal arrow and the spec says "plays the same" — cost:
  one line to add if the owner wants it.
- Task 6: Ruling: `NonCollidable`'s constructor is protected, not public as the plan's
  interface shows — it stands in for the Java class being `abstract` — cost: none, only
  subclasses construct it.
- Task 7: Ruling: a customer standing up shows a walking frame at once instead of the
  seated picture for one tick — the seated picture is 75 px and would flash at the wrong
  size — cost: none visible.
- Tasks 9–11: Ruling: done as one test-first cycle (one test file, three panels) instead
  of three — a failing test in C++ is a failed build either way — cost if wrong: a
  failure would have been narrowed to three files rather than one.
- Task 13: Ruling: `SoundManager` gained a signal `effectPlayed(QString)` that the plan
  does not list — without it nothing observable proves a click or meow was played, or
  that muting stops them (F3) — cost: one extra signal nobody else connects to.
- Task 13: Ruling: the music file is opened on first play, not at start-up, and stopping
  pauses rather than rewinds — the Java's `Clip.stop()` keeps its position, and opening
  13 MB for a menu that plays nothing is waste — cost: none known.
- Task 13: Ruling: the two tests that start music silence one log category
  (`qt.multimedia.ffmpeg.mediadataholder`) — destroying a player mid-load logs a true but
  harmless "immediate exit requested" — cost: a real warning in that one category would
  be hidden in those two test files only.
- Final: Ruling: no independent whole-project code review was run, although the
  executing-plans process calls for one — the owner was short on tokens and had not asked
  for one — cost if wrong: defects that tests and screenshots do not catch stay unfound
  until the owner plays or asks for the review.
- Task 15: Ruling: added a real-time test, `clocksDriveTheGame`, after the code rather
  than before it — nothing else exercised the real timers — it must use a real
  `QEventLoop`, because `QTest::qWait` naps 10 ms between rounds and starves a 10 ms
  timer — cost: the suite takes 2.4 s longer.
- Task 14: Ruling: Steps 1, 1b and the listening part of Task 13 Step 5 are handed to the
  owner — they need eyes, ears and hands on the running game — cost: differences in feel
  are found by the owner, not before.
- Tasks 9, 11, 12: Ruling: lambdas are used in `connect` wherever a click must pass an
  argument (`changeQuantity(+1)`, `buy(upgrade)`, `togglePanel(...)`), about ten in all,
  not the "one lambda" the plan's guide outline mentions — it is the standard Qt idiom
  and the alternative is a slot per button — cost: guide chapter 13 must explain lambdas
  properly rather than in passing.

### Open questions for the owner (asked 2026-11-09, not yet answered)

Both have defaults already written into the plan, so an answer of "keep the default"
changes nothing.

- **Q1.** In the Java, the keyboard is dead while the Upgrades panel is open (the Upgrades
  button never hands focus back to the game; Restock and Orders do). Keep that, or let the
  player move as with the other panels? Default: keep it. If changed: plan section "Open
  Questions" says what to edit; it becomes fix F14.
- **Q2.** The Java's reach rectangle is `(x+2, y+2, 44, 44)`, which reaches further right
  and down than left and up. Copy it exactly, or centre it? Default: copy it. If changed:
  it becomes fix F15.

### Things the owner has been told and may want to revisit

- Pacing: Qt's timers are exact; the Java's fire about 10 % late on this machine. The plan
  keeps the source's periods and times both games at the end (Task 14, Step 1b). Do not
  retune without asking.
- The Java has 27 source files, not 28 (an earlier count was wrong).
