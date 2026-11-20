# Cat Café Qt Rewrite Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rebuild the Java Swing game in `D:\Projects\Cat-cafe-game` as an idiomatic C++17 / Qt 6 application in `D:\Projects\Cat-cafe-game-qt`, together with a guide that teaches C++ classes and Qt from that code.

**Architecture:** A `QMainWindow` holds a `QStackedWidget` of three screens and is the only place that wires parts together with `connect`. The café is a `QGraphicsScene` of items (`NonCollidable` → `Interactable` → four concrete kinds, plus `Customer` and `Player`); controls are `QWidget`s layered over the `QGraphicsView`. Game rules live in classes that are tested without a visible window.

**Tech Stack:** C++17, Qt 6.11.2 (Widgets, Multimedia, Test), CMake 3.30 + Ninja 1.12, MinGW g++ 13.1.

**Spec:** `docs/superpowers/specs/2026-11-09-cat-cafe-qt-rewrite-design.md`. Read it first. The Java source in `D:\Projects\Cat-cafe-game` is the reference for any coordinate, size, price or rule not restated here.

## Global Constraints

- Project root is `D:\Projects\Cat-cafe-game-qt`. Never modify anything under `D:\Projects\Cat-cafe-game`; assets are copied from it.
- **No git repository, no commits.** Where a plan would say "commit", tick the task's boxes in this file and update the **Progress** section of `CLAUDE.md`.
- The toolchain is not on `PATH`. Kit `C:\Qt\6.11.2\mingw_64`; compiler `C:\Qt\Tools\mingw1310_64\bin\g++.exe`; CMake `C:\Qt\Tools\CMake_64\bin`; Ninja `C:\Qt\Tools\Ninja`. Never use anything under `C:\msys64`.
- C++17. `-Wall -Wextra`; the build must produce zero warnings.
- All UI is built in C++. No `.ui` files, no QML.
- Window client area is exactly 1000 × 665 and not resizable.
- Java constructors take `(image, x, y, h, w, …)`, **height before width**. C++ takes `QRect(x, y, w, h)`. Every rectangle in this plan is already converted.
- Fonts use pixel sizes (`QFont::setPixelSize`): 14 by default; 12, 18, 24, 36, 48 where stated.
- Java `drawString(text, x, y)` and `QPainter::drawText(x, y, text)` both take the text **baseline**; painted-text coordinates in this plan are baselines.
- The only intended behaviour differences from the Java are fixes F1–F13 in spec §7.
- Plain constructs only (spec §4): no templates beyond standard containers, no macros beyond Qt's, no PIMPL. A lambda is allowed where it is the standard Qt idiom (a `connect` that must pass an argument).
- Style: `m_` member prefix; `override` on every override; `connect` with function pointers; no `using namespace std;` in headers; `#pragma once` in headers; includes are relative to `src/` (`#include "world/Player.h"`).
- Ownership: widgets and scene items are raw pointers owned by their parent widget or their scene. An item that is also a `QObject` (`Player`) is owned by the scene only; never also give it a `QObject` parent. Carried items are `std::unique_ptr`.
- Tests: Qt Test, one executable per `tests/tst_*.cpp`, each ending in `QTEST_MAIN(ClassName)` and `#include "tst_<name>.moc"`. Tests never `show()` a window. Key events are sent with `QApplication::sendEvent` and hand-built `QKeyEvent`s.
- Every class header starts with a short comment: what the class is, which Java class it replaces, and which guide chapter (Task 15 numbering) explains the technique it shows.

## Review Focus

Conditions the spec implies but does not spell out, most likely to bite a player. Each has a test in the task named.

1. The music file is missing beside the executable → the game starts and plays without music, no crash. (Task 13)
2. The window loses focus while a movement key is held (alt-tab) → the player stops instead of running on forever. (Task 8)
3. A customer is served and deleted while the Orders panel is open → no stale card, no crash. (Task 10)
4. Back to the menu, then Play again → the same game resumes; timers are not doubled; no key is stuck. (Task 8)
5. E pressed with nothing in reach, and keys used while the Restock or Orders panel is open → nothing breaks; the keyboard still drives the player. (Tasks 8 and 12)

## Open Questions

For the owner (spec §12). The tasks implement the defaults; nothing is blocked.

- **Q1 — keyboard dead while the Upgrades panel is open?** Default: **yes, as in the Java.** Implemented in Task 12 (`upgradesPanelLocksTheControls`). If the owner says no: delete that test and the check, make `keysWorkWithAPanelOpen` cover all three panels, and add F14 to spec §7.
- **Q2 — lopsided reach rectangle?** Default: **copy the Java, `(x+2, y+2, 44, 44)`.** Implemented in Task 5. If the owner says centre it: `reachRect()` becomes `(x-2, y-2, 44, 44)`; the expected rectangles in Task 5 become `QRect(98, 98, 44, 44)` and `QRect(498, 498, 44, 44)`; re-check every `moveTo` position in Tasks 8, 10 and 12 against the world table; add F15 to spec §7.

## File Structure

```
CMakeLists.txt            targets: catcafe_core (static lib), CatCafe (exe), tst_* (tests)
env.bat build.bat run.bat test.bat
README.md  CLAUDE.md
assets/images/*.png (86)  assets/sounds/*.wav (4)  assets/fonts/WaterlilyScript.ttf
docs/GUIDE.md
src/main.cpp              QApplication, font, MainWindow, exec()
src/MainWindow.{h,cpp}    window, stacked screens, all cross-part connect() calls
src/Assets.{h,cpp}        pixmap loading/scaling/cache, game font, music path
src/ui/ImageButton.{h,cpp}
src/screens/MenuScreen.{h,cpp}  InstructionsScreen.{h,cpp}  GameScreen.{h,cpp}
src/items/ItemCatalog.{h,cpp}  Item.h  Ingredient.{h,cpp}  Cup.{h,cpp}
src/world/SpriteAnimation.{h,cpp}  Player.{h,cpp}
src/world/NonCollidable.{h,cpp}  Decoration.h  Interactable.{h,cpp}
src/world/Counter.{h,cpp}  PrepTable.{h,cpp}  Trashbin.{h,cpp}
src/world/Chair.{h,cpp}  Customer.{h,cpp}  SpawnOdds.{h,cpp}  CafeScene.{h,cpp}
src/panels/HeldItemPanel.{h,cpp}  RestockingPanel.{h,cpp}
src/panels/OrderPanel.{h,cpp}  OrderCard.{h,cpp}  UpgradesPanel.{h,cpp}
src/audio/SoundManager.{h,cpp}
tests/tst_assets.cpp  tst_items.cpp  tst_widgets.cpp  tst_screens.cpp  tst_player.cpp
tests/tst_interactables.cpp  tst_customers.cpp  tst_cafescene.cpp  tst_gamescreen.cpp
tests/tst_panels.cpp  tst_sound.cpp  tst_mainwindow.cpp
```

Stages (update `CLAUDE.md` at the end of each): **A** skeleton (Task 1) · **B** rules without a window (2) · **C** shell and menu (3–4) · **D** the world (5–8) · **E** controls (9–12) · **F** sound (13) · **G** finish (14–15).

---

### Task 1: Project skeleton, scripts, assets, `Assets`

**Files:**
- Create: `CMakeLists.txt`, `env.bat`, `build.bat`, `run.bat`, `test.bat`, `src/main.cpp`, `src/MainWindow.{h,cpp}`, `src/Assets.{h,cpp}`, `tests/tst_assets.cpp`
- Create by copying: `assets/images/`, `assets/sounds/`, `assets/fonts/`

**Interfaces:**
- Produces:

```cpp
// src/Assets.h
namespace Assets {
QPixmap pixmap(const QString &name);                     // ":/images/<name>.png", natural size; null + qWarning if missing
QPixmap pixmap(const QString &name, const QSize &size);  // smooth-scaled to size, cached per (name, size)
QFont   font(int pixelSize);                             // the game font; registers the .ttf on first use
QString musicFilePath();                                 // <dir of the .exe>/assets/sounds/backgroundMusic.wav
}
// src/MainWindow.h
class MainWindow : public QMainWindow { Q_OBJECT
public: explicit MainWindow(QWidget *parent = nullptr); };   // title "Cat Cafe", setFixedSize(1000, 665), central QStackedWidget
```

- Resource paths: `:/images/<name>.png`, `:/sounds/<name>.wav`, `:/fonts/WaterlilyScript.ttf`.
- CMake targets: `catcafe_core` (static library: everything in `src/` except `main.cpp`, plus the resources, `PUBLIC` include dir `src`, `PUBLIC` links `Qt6::Widgets Qt6::Multimedia`), `CatCafe` (`WIN32` executable), one `tst_<name>` per file in `tests/`, each registered with `add_test`. All executables go to `${CMAKE_BINARY_DIR}` (`CMAKE_RUNTIME_OUTPUT_DIRECTORY`).

- [ ] **Step 1: Copy the assets.** From `D:\Projects\Cat-cafe-game` into `assets/images/` exactly these 86 PNGs (names without extension):
  - screens: `background menuBackground instructionsBackground instructionsImage`
  - buttons: `backBtn backBtnHover playImg playHoverImg instructionsImg instructionsHoverImg restockBtn ordersBtn upgradesBtn soundOn soundOff musicOn musicOff`
  - banners: `itemBanner ordersBanner restockBanner upgradesBanner`
  - items: `boba milk sugar ice grapefruit greentea jelly lychee mango emptyCup filledCup`
  - counters: `cupCounter bobaCounter milkCounter sugarCounter iceCounter grapefruitCounter greenteaCounter jellyCounter lycheeCounter mangoCounter`
  - furniture: `prepTable trashbin longTable table chairLeft chairRight chairSeatedLeft chairSeatedRight wall`
  - pop-ups: `counterPopUp counterPopDown counterPopLeft counterPopRight customer1PopUp` … `customer8PopUp insufficientMessage`
  - sprites: `stationary rollLeft rollRight rollUp rollDown customerUp customerDown customerLeft customerRight`
  - panels: `currentItemBackground orderDisplayBox restockBackground restockActionBtn restockActionBtnGrey plus1 minus1 plus10 minus10 upgradesBackground upgrade max insufficient`

  Into `assets/sounds/`: `click.wav meowHappy.wav meowAngry.wav backgroundMusic.wav`. Into `assets/fonts/`: `Waterlily Script.ttf` renamed `WaterlilyScript.ttf`.

- [ ] **Step 2: Write `env.bat`, `build.bat`, `run.bat`, `test.bat`.**

```bat
@echo off
rem env.bat - puts the Qt toolchain on PATH for this console only
set "QT_KIT=C:\Qt\6.11.2\mingw_64"
set "PATH=C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\mingw1310_64\bin;%QT_KIT%\bin;%PATH%"
```

`build.bat`: `call "%~dp0env.bat"`, then `cmake -S "%~dp0." -B "%~dp0build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=%QT_KIT% -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe`, then `cmake --build "%~dp0build"`; exit non-zero if either fails. `run.bat`: call `build.bat`, then `start "" "%~dp0build\CatCafe.exe"`. `test.bat [name]`: call `build.bat`, then `ctest --test-dir "%~dp0build" --output-on-failure` (with `-R %1` when an argument is given).

- [ ] **Step 3: Write the failing test** `tests/tst_assets.cpp`:

```cpp
void imagesAreEmbedded()      { QCOMPARE(QDir(":/images").entryList(QDir::Files).size(), 86); }
void pixmapLoadsNaturalSize() { QCOMPARE(Assets::pixmap("background").size(), QSize(1000, 665)); }
void pixmapScales()           { QCOMPARE(Assets::pixmap("boba", QSize(25, 25)).size(), QSize(25, 25)); }
void missingPixmapIsNull()    { QTest::ignoreMessage(QtWarningMsg, QRegularExpression("nope"));
                                QVERIFY(Assets::pixmap("nope").isNull()); }
void fontIsTheGameFont()      { const QFont f = Assets::font(48);
                                QCOMPARE(f.pixelSize(), 48); QVERIFY(f.family().contains("Waterlily")); }
void shortSoundsAreEmbedded() { QVERIFY(QFile::exists(":/sounds/click.wav"));
                                QVERIFY(!QFile::exists(":/sounds/backgroundMusic.wav")); }
void musicPathIsBesideExe()   { QVERIFY(Assets::musicFilePath().endsWith("/assets/sounds/backgroundMusic.wav"));
                                QVERIFY(QFile::exists(Assets::musicFilePath())); }
```

- [ ] **Step 4: Write `CMakeLists.txt`.** `cmake_minimum_required(VERSION 3.21)`; `find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Multimedia Test)`; `qt_standard_project_setup()`; embed with `qt_add_resources(catcafe_core "assets" PREFIX "/" BASE assets FILES …)` listing every PNG, the font and the three short WAVs (not the music); a `POST_BUILD` step on `CatCafe` that creates `<build>/assets/sounds` and copies `backgroundMusic.wav` there. Use the font family name returned by `QFontDatabase::applicationFontFamilies`, never a hard-coded string. If resources added to the static library are not visible at run time, add them to each executable instead.

- [ ] **Step 5: Run** `test.bat assets`. Expected first: build fails (`Assets.h` not found). Then implement `Assets`, `MainWindow`, `main.cpp` (`QApplication::setFont(Assets::font(14))`).

- [ ] **Step 6: Run** `test.bat`. Expected: `100% tests passed`, no compiler warnings. **Run** `run.bat`. Expected: an empty 1000 × 665 window titled "Cat Cafe".

- [ ] **Step 7:** Tick this task; update `CLAUDE.md` Progress (stage A done).

---

### Task 2: Items — `ItemCatalog`, `Item`, `Ingredient`, `Cup`

**Files:**
- Create: `src/items/ItemCatalog.{h,cpp}`, `src/items/Item.h`, `src/items/Ingredient.{h,cpp}`, `src/items/Cup.{h,cpp}`
- Test: `tests/tst_items.cpp`

**Interfaces:**
- Consumes: nothing (no pixmaps here; items name their image, `Assets` loads it when something draws).
- Produces:

```cpp
enum class ItemKind { Cup, Liquid, Topping, Extra };
struct ItemInfo { QString name; ItemKind kind; int cost; QString image; };

class Item {                                   // abstract
public:
    virtual ~Item() = default;
    virtual QString name() const = 0;          // shown in the Current Item box
    virtual QString image() const = 0;         // asset name
};
class Ingredient : public Item {
public:
    explicit Ingredient(const ItemInfo &info);
    QString name() const override;  QString image() const override;
};
class Cup : public Item {
public:
    Cup() = default;
    explicit Cup(const QStringList &ingredients);
    void add(const QString &ingredient);
    const QStringList &ingredients() const;
    bool matches(const Cup &other) const;      // same ingredients, any order
    QString name() const override;             // always "cup"            (F12)
    QString image() const override;            // "emptyCup" or "filledCup"
};
class ItemCatalog {
public:
    ItemCatalog();
    const ItemInfo *find(const QString &name) const;          // nullptr if unknown
    QStringList namesOf(ItemKind kind) const;                 // definition order
    std::unique_ptr<Item> make(const QString &name) const;    // Cup for "cup", else Ingredient; nullptr if unknown
    int orderCost(const Cup &cup) const;                      // 1 + cost of each ingredient (unknown = 0)
    Cup randomOrder(QRandomGenerator &rng) const;             // liquid, topping, then sugar (50 %), then ice (50 %)
};
```

- Definitions, in this order (name, kind, cost, image): `boba` Topping 2 `boba` · `milk` Liquid 2 `milk` · `sugar` Extra 1 `sugar` · `ice` Extra 1 `ice` · `grapefruit` Liquid 2 · `greentea` Liquid 2 · `jelly` Topping 2 · `lychee` Liquid 2 · `mango` Liquid 2 · `cup` Cup 1 `emptyCup`.

- [ ] **Step 1: Write the failing tests** (`ItemCatalog catalog;` is a fixture member):

```cpp
void catalogKnowsTheItems() {
    QCOMPARE(catalog.find("boba")->cost, 2);   QCOMPARE(catalog.find("boba")->kind, ItemKind::Topping);
    QCOMPARE(catalog.find("sugar")->cost, 1);  QCOMPARE(catalog.find("sugar")->kind, ItemKind::Extra);
    QCOMPARE(catalog.find("cup")->cost, 1);    QCOMPARE(catalog.find("cup")->image, QString("emptyCup"));
    QVERIFY(catalog.find("tea") == nullptr);
    QCOMPARE(catalog.namesOf(ItemKind::Liquid), QStringList({"milk", "grapefruit", "greentea", "lychee", "mango"}));
    QCOMPARE(catalog.namesOf(ItemKind::Topping), QStringList({"boba", "jelly"}));
}
void makeBuildsTheRightClass() {
    QVERIFY(dynamic_cast<Cup *>(catalog.make("cup").get()) != nullptr);
    QVERIFY(dynamic_cast<Ingredient *>(catalog.make("milk").get()) != nullptr);
    QCOMPARE(catalog.make("milk")->name(), QString("milk"));
    QVERIFY(catalog.make("tea") == nullptr);
}
void cupImageFollowsContents() {
    Cup c; QCOMPARE(c.image(), QString("emptyCup")); QCOMPARE(c.name(), QString("cup"));
    c.add("milk"); QCOMPARE(c.image(), QString("filledCup"));
}
void cupsMatchInAnyOrder() {
    QVERIFY(Cup({"milk", "boba", "ice"}).matches(Cup({"ice", "milk", "boba"})));
    QVERIFY(!Cup({"milk", "boba"}).matches(Cup({"milk", "boba", "ice"})));
    QVERIFY(!Cup({"milk", "milk", "boba"}).matches(Cup({"milk", "boba", "boba"})));
    QVERIFY(Cup().matches(Cup()));
}
void orderCost() {
    QCOMPARE(catalog.orderCost(Cup({"milk", "boba"})), 5);
    QCOMPARE(catalog.orderCost(Cup({"mango", "jelly", "sugar", "ice"})), 7);
    QCOMPARE(catalog.orderCost(Cup()), 1);
}
void randomOrdersAreWellFormed() {            // QRandomGenerator rng(1234); 300 orders
    // every order: size 2..4; [0] is a liquid; [1] is a topping; any further entries are
    // "sugar" then "ice" in that order, each at most once.
    // across the 300: every liquid and both toppings occur; orders with and without sugar both occur.
}
```

- [ ] **Step 2: Run** `test.bat items` — expected: build fails, `items/ItemCatalog.h` not found.
- [ ] **Step 3: Implement** the four units. `Cup::matches` compares sorted copies of the two lists.
- [ ] **Step 4: Run** `test.bat items` — expected: PASS, zero warnings.
- [ ] **Step 5:** Tick; update `CLAUDE.md` (stage B done).

---

### Task 3: `SpriteAnimation` and `ImageButton`

**Files:**
- Create: `src/world/SpriteAnimation.{h,cpp}`, `src/ui/ImageButton.{h,cpp}`
- Test: `tests/tst_widgets.cpp`

**Interfaces:**
- Consumes: `Assets::pixmap` (Task 1).
- Produces:

```cpp
class SpriteAnimation {                         // copyable: frames are shared, the position is per copy (F7)
public:
    SpriteAnimation() = default;
    explicit SpriteAnimation(const QString &sheet);   // 200x200 cells read row by row, each scaled to 40x40
    int frameCount() const;
    const QPixmap &frame(int index) const;
    const QPixmap &next();                      // advances, then returns: the first call returns frame 1; wraps
};

class ImageButton : public QAbstractButton { Q_OBJECT
public:
    ImageButton(const QString &image, const QRect &geometry, QWidget *parent);
    void setImage(const QString &image);                           // the normal image
    void setHoverImage(const QString &image);                      // shown while the mouse is over it
    void setHoverImage(const QString &image, const QSize &size);   // ...and the button grows to size from its top-left
    QString currentImage() const;                                  // asset name being drawn now
signals:
    void hovered();                                                // mouse entered
protected:
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;
};
```

`ImageButton` draws only its image scaled to its size, has `Qt::NoFocus`, and a pointing-hand cursor. A disabled `ImageButton` does not emit `hovered()` and does not swap to its hover image (clicks are already ignored by `QAbstractButton`).

- [ ] **Step 1: Write the failing tests:**

```cpp
void sheetsCutIntoFrames() {
    QCOMPARE(SpriteAnimation("stationary").frameCount(), 3);
    QCOMPARE(SpriteAnimation("rollLeft").frameCount(), 14);
    QCOMPARE(SpriteAnimation("rollUp").frameCount(), 28);
    QCOMPARE(SpriteAnimation("customerLeft").frameCount(), 21);
    QCOMPARE(SpriteAnimation("customerDown").frameCount(), 28);
    QCOMPARE(SpriteAnimation("stationary").frame(0).size(), QSize(40, 40));
}
void nextStartsAtFrameOneAndWraps() {
    SpriteAnimation a("stationary");
    QCOMPARE(a.next().cacheKey(), a.frame(1).cacheKey());
    QCOMPARE(a.next().cacheKey(), a.frame(2).cacheKey());
    QCOMPARE(a.next().cacheKey(), a.frame(0).cacheKey());
}
void copiesAnimateIndependently() {
    SpriteAnimation a("stationary"); SpriteAnimation b = a; a.next();
    QCOMPARE(b.next().cacheKey(), b.frame(1).cacheKey());
}
void buttonBasics() {
    QWidget parent; ImageButton b("playImg", QRect(430, 535, 150, 50), &parent);
    QCOMPARE(b.geometry(), QRect(430, 535, 150, 50));
    QCOMPARE(b.focusPolicy(), Qt::NoFocus);
    QCOMPARE(b.currentImage(), QString("playImg"));
    QSignalSpy clicked(&b, &ImageButton::clicked);
    QTest::mouseClick(&b, Qt::LeftButton); QCOMPARE(clicked.count(), 1);
}
void hoverSwapsImageAndEmits() {               // send QEnterEvent / QEvent(QEvent::Leave) with QApplication::sendEvent
    // after setHoverImage("playHoverImg"): enter -> hovered emitted once, currentImage "playHoverImg";
    // leave -> currentImage "playImg". With no hover image set: enter still emits hovered, image unchanged.
}
void hoverCanGrow() {
    // ImageButton b("backBtn", QRect(870, 10, 100, 50), &parent); b.setHoverImage("backBtnHover", QSize(106, 53));
    // enter -> geometry QRect(870, 10, 106, 53); leave -> QRect(870, 10, 100, 50).
}
void disabledButtonIsQuiet() {
    // b.setHoverImage("playHoverImg"); b.setEnabled(false):
    // enter -> hovered not emitted, currentImage still "playImg"; mouseClick -> clicked not emitted.
}
```

- [ ] **Step 2: Run** `test.bat widgets` — expected: build fails (headers missing).
- [ ] **Step 3: Implement** both classes.
- [ ] **Step 4: Run** `test.bat widgets` — expected: PASS, zero warnings.
- [ ] **Step 5:** Tick.

---

### Task 4: `MenuScreen`, `InstructionsScreen`, screen switching

**Files:**
- Create: `src/screens/MenuScreen.{h,cpp}`, `src/screens/InstructionsScreen.{h,cpp}`
- Modify: `src/MainWindow.{h,cpp}`
- Test: `tests/tst_screens.cpp`, `tests/tst_mainwindow.cpp`

**Interfaces:**
- Consumes: `ImageButton`, `SpriteAnimation`, `Assets`.
- Produces:

```cpp
class MenuScreen : public QWidget { Q_OBJECT
public: explicit MenuScreen(QWidget *parent = nullptr);
signals: void playRequested(); void instructionsRequested(); };

class InstructionsScreen : public QWidget { Q_OBJECT
public: explicit InstructionsScreen(QWidget *parent = nullptr);
    void start(); void stop(); bool isRunning() const;        // the 30 ms timer
    int customerY() const; bool customerVisible() const; bool chairOccupied() const;
public slots: void advance();                                  // one tick
signals: void backRequested(); };

// MainWindow gains:
    QWidget *currentScreen() const;
public slots: void showMenu(); void showInstructions();
```

- Every screen is `setFixedSize(1000, 665)` and paints its background at (0, 0) in `paintEvent`.
- Menu: `ImageButton`s with `objectName` `"playButton"` at (430, 535, 150, 50) `playImg` / hover `playHoverImg`, and `"instructionsButton"` at (430, 600, 150, 50) `instructionsImg` / hover `instructionsHoverImg`. Background `menuBackground`.
- Instructions paints, in this order: `instructionsBackground` (0, 0); `instructionsImage` (90, 100) natural size; the stationary-cat frame (650, 190); `restockBtn` 120 × 30 at (480, 375); `ordersBtn` 120 × 30 at (750, 375); the customer "down" frame at (440, `customerY()`) while visible; the chair 75 × 75 at (420, 300), `chairRight`, or `chairSeatedRight` while occupied. Button `"backButton"` at (870, 10, 100, 50), `backBtn`, hover `backBtnHover` growing to 106 × 53.
- `advance()` (the Java's 1-second wall-clock pause becomes 33 ticks):

```cpp
if (m_pauseTicks > 0) { --m_pauseTicks; return; }      // customer hidden, chair shows the seated image
m_chairOccupied = false;  m_customerY += 2;            // and take the next "down" and stationary frames
if (m_customerY > 300) { m_customerY = 200; m_chairOccupied = true; m_pauseTicks = 33; }
update();
```

  `customerVisible()` is `m_pauseTicks == 0`. Initial state: y 200, visible, chair empty.
- `MainWindow`: menu `playRequested` is connected in Task 8. `showInstructions()` switches and calls `start()`; `showMenu()` switches and calls `stop()`.

- [ ] **Step 1: Write the failing tests:**

```cpp
// tst_screens.cpp
void menuButtonsEmit() {
    MenuScreen menu; QSignalSpy play(&menu, &MenuScreen::playRequested);
    QTest::mouseClick(menu.findChild<ImageButton *>("playButton"), Qt::LeftButton);
    QCOMPARE(play.count(), 1);                 // same for instructionsButton / instructionsRequested
    QCOMPARE(menu.size(), QSize(1000, 665));
}
void instructionsLoop() {
    InstructionsScreen s;
    QCOMPARE(s.customerY(), 200); QVERIFY(s.customerVisible()); QVERIFY(!s.chairOccupied());
    for (int i = 0; i < 50; ++i) s.advance();
    QCOMPARE(s.customerY(), 300); QVERIFY(s.customerVisible()); QVERIFY(!s.chairOccupied());
    s.advance();                               // 302 > 300: sits
    QCOMPARE(s.customerY(), 200); QVERIFY(!s.customerVisible()); QVERIFY(s.chairOccupied());
    for (int i = 0; i < 33; ++i) s.advance();  // the pause
    QCOMPARE(s.customerY(), 200); QVERIFY(s.chairOccupied());
    s.advance();
    QCOMPARE(s.customerY(), 202); QVERIFY(s.customerVisible()); QVERIFY(!s.chairOccupied());
}
void instructionsStartStop() { InstructionsScreen s; QVERIFY(!s.isRunning()); s.start(); QVERIFY(s.isRunning()); s.stop(); QVERIFY(!s.isRunning()); }
void screenshots() {                           // for the builder to look at; writes <build>/screenshots/menu.png, instructions.png
    QDir().mkpath("screenshots"); MenuScreen m; QVERIFY(m.grab().save("screenshots/menu.png")); /* same for instructions */
}
// tst_mainwindow.cpp
void startsOnMenu()       { MainWindow w; QCOMPARE(w.size(), QSize(1000, 665)); QVERIFY(qobject_cast<MenuScreen *>(w.currentScreen())); }
void instructionsAndBack() {
    MainWindow w;
    QTest::mouseClick(w.findChild<ImageButton *>("instructionsButton"), Qt::LeftButton);
    auto *ins = qobject_cast<InstructionsScreen *>(w.currentScreen());
    QVERIFY(ins); QVERIFY(ins->isRunning());
    QTest::mouseClick(ins->findChild<ImageButton *>("backButton"), Qt::LeftButton);
    QVERIFY(qobject_cast<MenuScreen *>(w.currentScreen())); QVERIFY(!ins->isRunning());
}
```

- [ ] **Step 2: Run** `test.bat` — expected: build fails (screens missing).
- [ ] **Step 3: Implement.**
- [ ] **Step 4: Run** `test.bat` — expected: PASS, zero warnings.
- [ ] **Step 5: Look.** Open `build/screenshots/menu.png` and `instructions.png` and compare with the Java screens (`open.bat` in the Java folder). Then `run.bat`: hover and click through Menu → Instructions → Back; the customer walks down, sits for a second, repeats.
- [ ] **Step 6:** Tick; update `CLAUDE.md` (stage C done).

---

### Task 5: `Player`

**Files:**
- Create: `src/world/Player.{h,cpp}`
- Test: `tests/tst_player.cpp`

**Interfaces:**
- Consumes: `Item`, `Cup` (Task 2), `SpriteAnimation` (Task 3), `Assets`.
- Produces:

```cpp
class Player : public QObject, public QGraphicsPixmapItem { Q_OBJECT
public:
    Player();                                   // no parent: the scene will own it
    void setKeyHeld(int key, bool held);        // Qt::Key_W / A / S / D; other keys ignored
    void stop();                                // as if every key were released
    void step(const QList<QRect> &solids);      // one 16 ms tick: move, collide, next sprite frame
    void moveTo(double x, double y);            // teleport (used by tests)
    QRect bodyRect() const;                     // collision box
    QRect reachRect() const;                    // interaction box
    Item *heldItem() const;                     // nullptr when empty-handed
    void give(std::unique_ptr<Item> item);      // precondition: empty-handed
    std::unique_ptr<Item> takeItem();           // hands over whatever is held (may be nullptr)
    std::unique_ptr<Cup>  takeCup();            // hands over the held cup; nullptr and no change if not holding a cup
    int money() const;   void addMoney(int delta);
    int tips() const;    void addTip();
    double speed() const; void addSpeed(double amount);
signals:
    void moneyChanged(int money);
    void heldItemChanged();
    void tipsChanged(int tips);
};
```

- Values: 40 × 40; starts at (100, 100); speed 2.2 per step; money 1000; tips 0; z-value 1. Position is kept as two `double`s; the item is placed at the rounded position (`qRound`).
- `bodyRect()` = `QRect(round(x)+5, round(y)+5, 30, 30)`. `reachRect()` = `QRect(round(x)+2, round(y)+2, 44, 44)` — copied from the Java as it is, lopsided (Open Question Q2).
- `step`: `dx = (A ? -speed : 0) + (D ? speed : 0)`, `dy` likewise for W/S; if both are non-zero each becomes `±speed/√2`; add both; if `bodyRect()` intersects any solid, **or** `x <= 0 || x >= 960 || y <= 0 || y >= 620`, subtract both again.
- Sprite, checked in this order: D without A → `rollRight`; A without D → `rollLeft`; W without S → `rollUp`; S without W → `rollDown`; else `stationary`. `step` shows `next()` of the chosen animation.
- Held-item icon: a child `QGraphicsPixmapItem` at (40, 20), 25 × 25, hidden when empty-handed.

- [ ] **Step 1: Write the failing tests:**

```cpp
void startsWhereTheJavaDoes() {
    Player p; QCOMPARE(p.pos(), QPointF(100, 100)); QCOMPARE(p.money(), 1000); QCOMPARE(p.tips(), 0);
    QCOMPARE(p.speed(), 2.2); QVERIFY(p.heldItem() == nullptr); QCOMPARE(p.zValue(), 1.0);
    QCOMPARE(p.bodyRect(), QRect(105, 105, 30, 30)); QCOMPARE(p.reachRect(), QRect(102, 102, 44, 44));
}
void movesRight()          { Player p; p.setKeyHeld(Qt::Key_D, true); for (int i = 0; i < 10; ++i) p.step({});
                             QCOMPARE(p.pos(), QPointF(122, 100)); }
void diagonalIsNormalised(){ Player p; p.setKeyHeld(Qt::Key_D, true); p.setKeyHeld(Qt::Key_S, true);
                             for (int i = 0; i < 10; ++i) p.step({}); QCOMPARE(p.pos(), QPointF(116, 116)); }
void opposingKeysCancel()  { Player p; p.setKeyHeld(Qt::Key_A, true); p.setKeyHeld(Qt::Key_D, true); p.step({});
                             QCOMPARE(p.pos(), QPointF(100, 100)); }
void stopsAtSolids()       { Player p; p.setKeyHeld(Qt::Key_D, true); for (int i = 0; i < 20; ++i) p.step({QRect(140, 90, 50, 60)});
                             QCOMPARE(p.pos(), QPointF(104, 100)); }
void staysInBounds() {
    Player a; a.setKeyHeld(Qt::Key_A, true); for (int i = 0; i < 100; ++i) a.step({}); QCOMPARE(a.pos(), QPointF(1, 100));
    Player s; s.setKeyHeld(Qt::Key_S, true); for (int i = 0; i < 400; ++i) s.step({}); QCOMPARE(s.pos(), QPointF(100, 619));
    Player d; d.setKeyHeld(Qt::Key_D, true); for (int i = 0; i < 600; ++i) d.step({}); QCOMPARE(d.pos(), QPointF(958, 100));
}
void stopReleasesEverything(){ Player p; p.setKeyHeld(Qt::Key_D, true); p.stop(); p.step({}); QCOMPARE(p.pos(), QPointF(100, 100)); }
void moveTo()              { Player p; p.moveTo(500, 500); QCOMPARE(p.pos(), QPointF(500, 500)); QCOMPARE(p.reachRect(), QRect(502, 502, 44, 44)); }
void carriesOneItem() {
    Player p; QSignalSpy changed(&p, &Player::heldItemChanged); ItemCatalog catalog;
    p.give(catalog.make("milk")); QCOMPARE(changed.count(), 1);
    QVERIFY(p.takeCup() == nullptr); QVERIFY(p.heldItem() != nullptr); QCOMPARE(changed.count(), 1);
    std::unique_ptr<Item> milk = p.takeItem(); QCOMPARE(milk->name(), QString("milk"));
    QVERIFY(p.heldItem() == nullptr); QCOMPARE(changed.count(), 2);
    p.give(std::make_unique<Cup>(QStringList{"milk"}));
    std::unique_ptr<Cup> cup = p.takeCup(); QCOMPARE(cup->ingredients(), QStringList{"milk"}); QVERIFY(p.heldItem() == nullptr);
}
void moneyTipsSpeedAnnounce() {
    Player p; QSignalSpy money(&p, &Player::moneyChanged); QSignalSpy tips(&p, &Player::tipsChanged);
    p.addMoney(-3); QCOMPARE(p.money(), 997); QCOMPARE(money.last().at(0).toInt(), 997);
    p.addTip();     QCOMPARE(p.tips(), 1);    QCOMPARE(tips.last().at(0).toInt(), 1);
    p.addSpeed(0.3); QCOMPARE(p.speed(), 2.5);
}
```

- [ ] **Step 2: Run** `test.bat player` — expected: build fails (`world/Player.h`).
- [ ] **Step 3: Implement.** `takeCup` is where the guide's `dynamic_cast` + `unique_ptr` hand-over lives: check with `dynamic_cast<Cup *>(m_held.get())`, then `release()` into a `std::unique_ptr<Cup>`.
- [ ] **Step 4: Run** `test.bat player` — expected: PASS, zero warnings.
- [ ] **Step 5:** Tick.

---

### Task 6: `NonCollidable`, `Decoration`, `Interactable`, `Counter`, `PrepTable`, `Trashbin`

**Files:**
- Create: `src/world/NonCollidable.{h,cpp}`, `Decoration.h`, `Interactable.{h,cpp}`, `Counter.{h,cpp}`, `PrepTable.{h,cpp}`, `Trashbin.{h,cpp}`
- Test: `tests/tst_interactables.cpp`

**Interfaces:**
- Consumes: `Player` (Task 5), `ItemCatalog`, `Cup`, `Ingredient` (Task 2).
- Produces:

```cpp
class NonCollidable : public QGraphicsPixmapItem {
public:
    NonCollidable(const QString &image, const QRect &rect);   // image scaled to rect.size(); placed at rect.topLeft(); z-value 2
    QRect rect() const;
    QRect solidRect() const;                                   // what blocks the player; rect() unless changed
protected:
    void setSolidRect(const QRect &rect);
};
class Decoration : public NonCollidable { public: Decoration(const QString &image, const QRect &rect); };

enum class PopUpSide { Up, Down, Left, Right };
enum class Outcome { Nothing, TookItem, CounterEmpty, PutCupDown, PickedCupUp, AddedIngredient,
                     Discarded, ServedRight, ServedWrong };

class Interactable : public NonCollidable {
public:
    Interactable(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side);
    QRect reachRect() const;            // rect() grown by reachX on the left and right, reachY above and below
    QRect popUpRect() const;            // 50 x 50, see below
    PopUpSide popUpSide() const;
    virtual bool canInteract() const;   // true
    virtual Outcome interact(Player &player) = 0;
};
class Counter : public Interactable {
public:
    Counter(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side,
            const ItemCatalog &catalog, const QString &itemName, int stock);
    QString itemName() const;  int stock() const;  void addStock(int amount);
    Outcome interact(Player &player) override;
};
class PrepTable : public Interactable {
public:
    PrepTable(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side);
    const Cup *cup() const;             // nullptr when the table is empty
    Outcome interact(Player &player) override;
};
class Trashbin : public Interactable {
public:
    Trashbin(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side);
    Outcome interact(Player &player) override;
};
```

- Pop-up position for an item at (x, y) of size w × h (integer division): Up `(x + w/2 − 25, y − 50)`; Down `(x + w/2 − 25, y + h)`; Left `(x − 50, y + h/2 − 25)`; Right `(x + w, y + h/2 − 25)`.
- `Counter::interact`: holding anything → `Nothing`. Empty-handed with stock → give `catalog.make(itemName)`, stock − 1, `TookItem`. Empty-handed with no stock → `CounterEmpty`.
- `PrepTable::interact`: empty-handed and table has a cup → player gets it, `PickedCupUp`. Holding a cup and table empty → table gets it, `PutCupDown`. Holding a non-cup and table has a cup → its name is added to the cup, the held item is destroyed, `AddedIngredient`. Anything else → `Nothing`.
- The table's cup is drawn by a child `QGraphicsPixmapItem` at (15, −10), 50 × 50, showing the cup's `image()`; hidden when empty.
- `Trashbin::interact`: holding anything → destroyed, `Discarded`; else `Nothing`.
- The catalog must outlive every `Counter`. `CafeScene` (Task 8) owns both; note that C++ destroys a class's members *before* its base class, so counters still exist briefly after the catalog is gone — `Counter` must not touch the catalog in a destructor.

- [ ] **Step 1: Write the failing tests** (`ItemCatalog catalog; Player player;` created per test):

```cpp
void geometry() {
    Counter cup("cupCounter", QRect(235, 150, 75, 75), 10, 10, PopUpSide::Up, catalog, "cup", 3);
    QCOMPARE(cup.pos(), QPointF(235, 150)); QCOMPARE(cup.pixmap().size(), QSize(75, 75)); QCOMPARE(cup.zValue(), 2.0);
    QCOMPARE(cup.solidRect(), QRect(235, 150, 75, 75)); QCOMPARE(cup.reachRect(), QRect(225, 140, 95, 95));
    QCOMPARE(cup.popUpRect(), QRect(247, 100, 50, 50));
    Counter boba("bobaCounter", QRect(150, 0, 75, 75), 0, 10, PopUpSide::Down, catalog, "boba", 1);
    QCOMPARE(boba.reachRect(), QRect(150, -10, 75, 95)); QCOMPARE(boba.popUpRect(), QRect(162, 75, 50, 50));
    Counter ice("iceCounter", QRect(0, 225, 75, 75), 10, 0, PopUpSide::Right, catalog, "ice", 2);
    QCOMPARE(ice.popUpRect(), QRect(75, 237, 50, 50));
    Trashbin left("trashbin", QRect(200, 200, 50, 50), 10, 10, PopUpSide::Left);
    QCOMPARE(left.popUpRect(), QRect(150, 200, 50, 50));
    Decoration wall("wall", QRect(448, 75, 4, 275)); QCOMPARE(wall.solidRect(), QRect(448, 75, 4, 275));
}
void counterHandsOutStock() {
    Counter c("cupCounter", QRect(235, 150, 75, 75), 10, 10, PopUpSide::Up, catalog, "cup", 3);
    QCOMPARE(c.interact(player), Outcome::TookItem); QCOMPARE(c.stock(), 2);
    QVERIFY(dynamic_cast<Cup *>(player.heldItem()) != nullptr);
    QCOMPARE(c.interact(player), Outcome::Nothing);  QCOMPARE(c.stock(), 2);       // hands full
    c.addStock(9); QCOMPARE(c.stock(), 11);
}
void counterMakesIngredients() {
    Counter c("milkCounter", QRect(225, 0, 75, 75), 0, 10, PopUpSide::Down, catalog, "milk", 1);
    QCOMPARE(c.interact(player), Outcome::TookItem); QCOMPARE(player.heldItem()->name(), QString("milk"));
    QVERIFY(dynamic_cast<Cup *>(player.heldItem()) == nullptr);
}
void emptyCounterSaysSo() {
    Counter c("milkCounter", QRect(225, 0, 75, 75), 0, 10, PopUpSide::Down, catalog, "milk", 0);
    QCOMPARE(c.interact(player), Outcome::CounterEmpty); QVERIFY(player.heldItem() == nullptr);
}
void prepTableHandOver() {
    PrepTable t("prepTable", QRect(150, 150, 75, 75), 10, 10, PopUpSide::Up);
    player.give(catalog.make("cup"));
    QCOMPARE(t.interact(player), Outcome::PutCupDown); QVERIFY(player.heldItem() == nullptr);
    QVERIFY(t.cup() != nullptr); QVERIFY(t.cup()->ingredients().isEmpty());
    player.give(catalog.make("milk"));
    QCOMPARE(t.interact(player), Outcome::AddedIngredient); QVERIFY(player.heldItem() == nullptr);
    QCOMPARE(t.cup()->ingredients(), QStringList{"milk"});
    QCOMPARE(t.interact(player), Outcome::PickedCupUp); QVERIFY(t.cup() == nullptr);
    QCOMPARE(player.heldItem()->image(), QString("filledCup"));
}
void prepTableRefusals() {
    PrepTable t("prepTable", QRect(150, 150, 75, 75), 10, 10, PopUpSide::Up);
    QCOMPARE(t.interact(player), Outcome::Nothing);                               // nothing anywhere
    player.give(catalog.make("milk"));
    QCOMPARE(t.interact(player), Outcome::Nothing); QVERIFY(player.heldItem() != nullptr);   // ingredient, no cup
    player.takeItem(); player.give(catalog.make("cup")); t.interact(player); player.give(catalog.make("cup"));
    QCOMPARE(t.interact(player), Outcome::Nothing); QVERIFY(player.heldItem() != nullptr);   // cup onto cup
}
void trashbin() {
    Trashbin b("trashbin", QRect(0, 325, 50, 50), 10, 10, PopUpSide::Right);
    QCOMPARE(b.interact(player), Outcome::Nothing);
    player.give(catalog.make("milk"));
    QCOMPARE(b.interact(player), Outcome::Discarded); QVERIFY(player.heldItem() == nullptr);
}
```

- [ ] **Step 2: Run** `test.bat interactables` — expected: build fails.
- [ ] **Step 3: Implement.**
- [ ] **Step 4: Run** `test.bat interactables` — expected: PASS, zero warnings.
- [ ] **Step 5:** Tick.

---

### Task 7: `Customer`, `Chair`, `SpawnOdds`

**Files:**
- Create: `src/world/Customer.{h,cpp}`, `Chair.{h,cpp}`, `SpawnOdds.{h,cpp}`
- Test: `tests/tst_customers.cpp`

**Interfaces:**
- Consumes: `Interactable`, `Outcome` (Task 6), `Player`, `Cup`, `SpriteAnimation`.
- Produces:

```cpp
struct CustomerSprites {                 // loaded once by the scene; each customer keeps its own copy
    CustomerSprites();                   // customerUp/Down/Left/Right sheets, chairSeatedLeft/Right at 75x75,
    SpriteAnimation up, down, left, right;   // customer1PopUp..customer8PopUp at 50x50
    QPixmap seatedLeft, seatedRight;
    QList<QPixmap> numberBubbles;        // [0] is customer1PopUp
};
class Customer : public QGraphicsPixmapItem {
public:
    enum class State { Arriving, Seated, Leaving, Gone };
    Customer(const Cup &order, int orderCost, const Chair &chair, const CustomerSprites &sprites);
    State state() const;
    const Cup &order() const;  int orderCost() const;
    int payout(int tips) const;          // orderCost * 3 / 2 + tips  (integer division: the F4 rule)
    void step();                         // one 10 ms tick
    void leave();                        // Seated -> Leaving
    void setNumber(int number);          // 1..8; the bubble is visible only while Seated
    int number() const;
};
class Chair : public Interactable {
public:
    Chair(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side,
          bool facesRight, const QList<QPoint> &route);
    bool facesRight() const;  const QList<QPoint> &route() const;
    bool isFree() const;                 // no customer assigned
    Customer *customer() const;
    void assign(Customer *customer);
    bool canInteract() const override;   // a customer is assigned and Seated
    Outcome interact(Player &player) override;
    static constexpr int WrongOrderPenalty = 3;
};
class SpawnOdds {
public:
    bool roll(double random01);              // n = int(random01 * chance / divisor); chance = int(chance / 1.08); return n == 0
    void customerSpawned(int customersBefore);   // chance = 100 * (customersBefore + 1)
    void setAdvertisementLevel(int level);   // divisor = 1 + pow(level, 1.1) / 10
    int chance() const;  double divisor() const;     // start: 10 and 1.0
};
```

- `Chair`'s solid rectangle is `QRect(x + 10, y + 13, w − 20, h − 15)`.
- `Customer`: z-value 3. Created at `route.first()` showing the first `left` frame (40 × 40), `Arriving`, target index 0. The number bubble is a child item at (0, −50).
- `step()` while `Arriving`: if the target index equals the route length → become `Seated` (position = chair `rect().topLeft()`, pixmap = `seatedRight` or `seatedLeft` by `facesRight`); otherwise move one pixel towards the target on each axis (`dx`, `dy` ∈ {−1, 0, 1}), pick the animation (`dx > 0` right, else `dx < 0` left, else `dy < 0` up, else `dy > 0` down, else keep the current pixmap) and show its `next()`; if now on the target, target index + 1.
- `leave()`: `Leaving`, position = `route.last()`, target index = last index. `step()` while `Leaving`: move as above; if now on the target: when the index is 0 → `Gone`, else index − 1. `Seated` and `Gone` customers ignore `step()`.
- `Chair::interact`: unless `canInteract()` and the player holds a `Cup` → `Nothing`. If the cup `matches` the order → `player.addMoney(customer->payout(player.tips()))`, `ServedRight`; else `player.addMoney(-WrongOrderPenalty)`, `ServedWrong`. Either way the cup is destroyed, the customer `leave()`s and the chair is free at once (F1).

- [ ] **Step 1: Write the failing tests** (fixture: `ItemCatalog catalog; CustomerSprites sprites; Player player;` and
  `Chair chair("chairRight", QRect(800, 62, 75, 75), 10, 10, PopUpSide::Up, true, {{980, 180}, {850, 180}, {850, 100}});`
  `Cup order({"milk", "boba"});`, cost 5):

```cpp
void chairGeometry() { QCOMPARE(chair.solidRect(), QRect(810, 75, 55, 60)); QCOMPARE(chair.reachRect(), QRect(790, 52, 95, 95));
                       QVERIFY(chair.isFree()); QVERIFY(!chair.canInteract()); }
void walksInAndSits() {
    Customer c(order, 5, chair, sprites); chair.assign(&c);
    QCOMPARE(c.pos(), QPointF(980, 180)); QCOMPARE(c.zValue(), 3.0); QVERIFY(!chair.isFree());
    for (int i = 0; i < 211; ++i) c.step();
    QCOMPARE(c.state(), Customer::State::Arriving); QCOMPARE(c.pos(), QPointF(850, 100)); QVERIFY(!chair.canInteract());
    c.step();
    QCOMPARE(c.state(), Customer::State::Seated); QCOMPARE(c.pos(), QPointF(800, 62));
    QCOMPARE(c.pixmap().size(), QSize(75, 75)); QVERIFY(chair.canInteract());
}
void walksOutAndGoes() {                    // seat as above, then:
    c.leave(); QCOMPARE(c.pos(), QPointF(850, 100));
    for (int i = 0; i < 210; ++i) c.step(); QCOMPARE(c.state(), Customer::State::Leaving);
    c.step(); QCOMPARE(c.state(), Customer::State::Gone); QCOMPARE(c.pos(), QPointF(980, 180));
}
void payout() { Customer a(order, 5, chair, sprites); QCOMPARE(a.payout(0), 7); QCOMPARE(a.payout(2), 9);
                Customer b(order, 7, chair, sprites); QCOMPARE(b.payout(0), 10); }
void rightCupPays() {                       // seated customer; player.give(std::make_unique<Cup>(QStringList{"boba", "milk"}))
    QCOMPARE(chair.interact(player), Outcome::ServedRight); QCOMPARE(player.money(), 1007);
    QVERIFY(player.heldItem() == nullptr); QVERIFY(chair.isFree()); QCOMPARE(c.state(), Customer::State::Leaving);
}
void wrongCupCosts() {                      // player holds Cup({"milk"})
    QCOMPARE(chair.interact(player), Outcome::ServedWrong); QCOMPARE(player.money(), 997);
    QVERIFY(chair.isFree()); QCOMPARE(c.state(), Customer::State::Leaving);
}
void tipsAreAdded()   { /* player.addTip() twice, right cup -> money 1009 */ }
void chairRefuses() {
    // seated customer, empty hands -> Nothing; holding catalog.make("milk") -> Nothing and still held;
    // customer still Arriving, holding the right cup -> Nothing and still held.
}
void freedChairIsReusedWhileTheOldCustomerLeaves() {     // F1
    // seat c1, serve it; Customer c2(order, 5, chair, sprites); chair.assign(&c2);
    // step both 211 times -> c1 is Gone at (980, 180); c2 is Arriving at (850, 100); one more step -> c2 Seated.
}
void numberBubble()   { /* setNumber(3): number() == 3; bubble child visible only once Seated, hidden again after leave() */ }
void spawnOdds() {
    SpawnOdds o; QCOMPARE(o.chance(), 10); QCOMPARE(o.divisor(), 1.0);
    QVERIFY(!o.roll(0.5));  QCOMPARE(o.chance(), 9);       // int(0.5 * 10) = 5
    QVERIFY(o.roll(0.05));  QCOMPARE(o.chance(), 8);       // int(0.05 * 9) = 0
    o.customerSpawned(0); QCOMPARE(o.chance(), 100);
    o.customerSpawned(2); QCOMPARE(o.chance(), 300);
    o.setAdvertisementLevel(1); QCOMPARE(o.divisor(), 1.1);                      // F2: level 1 already helps
    o.setAdvertisementLevel(5); QCOMPARE(o.divisor(), 1 + std::pow(5, 1.1) / 10);
    SpawnOdds z; for (int i = 0; i < 11; ++i) z.roll(0.99);                      // 10, 9, ... decays to 0
    QCOMPARE(z.chance(), 0); QVERIFY(z.roll(0.99));                              // at 0 it always spawns
}
```

- [ ] **Step 2: Run** `test.bat customers` — expected: build fails.
- [ ] **Step 3: Implement.** `Chair.h` and `Customer.h` each forward-declare the other class and include the other's header only in the `.cpp` (the guide's forward-declaration example).
- [ ] **Step 4: Run** `test.bat customers` — expected: PASS, zero warnings.
- [ ] **Step 5:** Tick.

---

### Task 8: `CafeScene`, the playable core of `GameScreen`, Play and Back

**Files:**
- Create: `src/world/CafeScene.{h,cpp}`, `src/screens/GameScreen.{h,cpp}`
- Modify: `src/MainWindow.{h,cpp}`
- Test: `tests/tst_cafescene.cpp`, `tests/tst_gamescreen.cpp`, `tests/tst_mainwindow.cpp`

**Interfaces:**
- Consumes: everything from Tasks 2–7.
- Produces:

```cpp
class CafeScene : public QGraphicsScene { Q_OBJECT
public:
    explicit CafeScene(QObject *parent = nullptr);
    Player *player() const;
    const ItemCatalog &catalog() const;
    const QList<Counter *> &counters() const;
    Counter *findCounter(const QString &itemName) const;   // nullptr if none
    const QList<Chair *> &chairs() const;
    const QList<Interactable *> &interactables() const;    // in the order listed below
    QList<QRect> solidRects() const;
    QList<Customer *> seatedCustomers() const;             // arrival order; number = index + 1
    int customerCount() const;                             // arriving + seated + leaving
    Interactable *target() const;                          // first usable interactable in reach, else nullptr
    SpawnOdds &spawnOdds();
    Customer *seatNewCustomer(Chair *chair);               // new customer with a random order, walking to chair
    void restock(Counter *counter, int amount);            // addStock, then emits stockChanged
    void start();  void stop();  bool isRunning() const;   // the three timers
public slots:
    void frameTick();      // 16 ms: player step, then choose target() and place the pop-up
    void customerTick();   // 10 ms: step all; newly seated -> renumber + ordersChanged; Gone -> removed and deleted
    void spawnTick();      // 1 s: if a chair is free, roll; on success seatNewCustomer on a random free chair
    void interact();       // the E key
signals:
    void ordersChanged();
    void stockChanged();
    void orderServed(bool correct, int moneyDelta);
    void counterEmpty();
};
class GameScreen : public QWidget { Q_OBJECT
public:
    explicit GameScreen(QWidget *parent = nullptr);
    CafeScene *scene() const;
    void start();          // scene()->start() and take keyboard focus
    void stop();           // scene()->stop() and player stop
signals:
    void backRequested();
protected:
    void keyPressEvent(QKeyEvent *) override;  void keyReleaseEvent(QKeyEvent *) override;
    void focusOutEvent(QFocusEvent *) override;
};
// MainWindow gains:  public slots: void showGame();
```

- Scene rect (0, 0, 1000, 665). Z-values: background image 0, player 1, solids 2, customers 3, pop-up 4, "out of stock" message 6.
- The world, in creation order (this is also the order of `interactables()`; all rectangles are x, y, w, h):

| # | Class | Image | Rect | reach x, y | Pop-up | Extra |
|---|---|---|---|---|---|---|
| 0 | Counter | cupCounter | 235, 150, 75, 75 | 10, 10 | Up | `cup`, stock 3 |
| 1 | Counter | bobaCounter | 150, 0, 75, 75 | 0, 10 | Down | `boba`, 1 |
| 2 | Counter | milkCounter | 225, 0, 75, 75 | 0, 10 | Down | `milk`, 1 |
| 3 | Counter | sugarCounter | 300, 0, 75, 75 | 0, 10 | Down | `sugar`, 2 |
| 4 | Counter | iceCounter | 0, 225, 75, 75 | 10, 0 | Right | `ice`, 2 |
| 5 | Counter | grapefruitCounter | 0, 75, 75, 75 | 10, 0 | Right | `grapefruit`, 1 |
| 6 | Counter | greenteaCounter | 0, 150, 75, 75 | 10, 0 | Right | `greentea`, 1 |
| 7 | Counter | jellyCounter | 150, 300, 75, 75 | 0, 10 | Up | `jelly`, 1 |
| 8 | Counter | lycheeCounter | 225, 300, 75, 75 | 0, 10 | Up | `lychee`, 1 |
| 9 | Counter | mangoCounter | 300, 300, 75, 75 | 0, 10 | Up | `mango`, 1 |
| 10 | PrepTable | prepTable | 150, 150, 75, 75 | 10, 10 | Up | |
| 11 | PrepTable | prepTable | 320, 150, 75, 75 | 10, 10 | Up | |
| 12 | Trashbin | trashbin | 0, 325, 50, 50 | 10, 10 | Right | |
| 13 | Trashbin | trashbin | 75, 0, 50, 50 | 10, 10 | Down | |
| – | Decoration | longTable | 575, 475, 400, 100 | | | |
| 14 | Chair | chairLeft | 575, 385, 75, 75 | 10, 10 | Up | left; (980,180) (900,180) (900,350) (600,350) (600,380) |
| 15 | Chair | chairLeft | 675, 385, 75, 75 | 10, 10 | Up | left; (980,180) (900,180) (900,350) (700,350) (700,380) |
| 16 | Chair | chairRight | 775, 385, 75, 75 | 10, 10 | Up | right; (980,180) (900,180) (900,350) (800,350) (800,380) |
| 17 | Chair | chairRight | 875, 385, 75, 75 | 10, 10 | Up | right; (980,180) (900,180) (900,350) (900,380) |
| – | Decoration | table | 700, 50, 100, 100 | | | |
| 18 | Chair | chairLeft | 625, 62, 75, 75 | 10, 10 | Up | left; (980,180) (620,180) (620,100) |
| 19 | Chair | chairRight | 800, 62, 75, 75 | 10, 10 | Up | right; (980,180) (850,180) (850,100) |
| – | Decoration | table | 700, 230, 100, 100 | | | |
| 20 | Chair | chairLeft | 625, 242, 75, 75 | 10, 10 | Up | left; (980,180) (620,180) (620,260) |
| 21 | Chair | chairRight | 800, 242, 75, 75 | 10, 10 | Up | right; (980,180) (850,180) (850,260) |
| – | Decoration | wall | 0, 423, 450, 4 | | | |
| – | Decoration | wall | 448, 75, 4, 275 | | | |
| – | Decoration | wall | 448, 425, 4, 165 | | | |
| – | Decoration | wall | 450, 588, 550, 4 | | | |

- `target()` after `frameTick`: the first entry of `interactables()` whose `reachRect()` intersects the player's `reachRect()` **and** whose `canInteract()` is true (F5). One pop-up item is reused: placed at the target's `popUpRect()` with `counterPopUp` / `counterPopDown` / `counterPopLeft` / `counterPopRight` by `popUpSide()`, hidden when there is no target.
- `interact()`: no target → return. Otherwise call it and react: `TookItem` → `stockChanged`; `CounterEmpty` → show `insufficientMessage` (500 × 100 at (250, 300)) for 1000 ms (a single-shot timer, restarted if already showing) and emit `counterEmpty`; `ServedRight` / `ServedWrong` → renumber the seated customers, emit `orderServed(correct, money after − money before)` then `ordersChanged`.
- `spawnTick()`: `customerSpawned(customerCount())` is called with the count *before* the new customer is added; the order is `catalog().randomOrder(...)`, its cost `catalog().orderCost(order)`. Uses `QRandomGenerator::global()`.
- Timers: 16, 10 and 1000 ms, `Qt::PreciseTimer`, each connected to its slot; `start()` on running timers must not create extra ticks.
- `GameScreen` (this task): a `QGraphicsView` at (0, 0, 1000, 665) — no frame, both scroll bars always off, `Qt::NoFocus`, not interactive — showing a `CafeScene` whose `QObject` parent is the screen; `"backButton"` at (870, 10, 100, 50) `backBtn`, hover `backBtnHover` at 106 × 53, emitting `backRequested`. The screen has `Qt::StrongFocus`. Keys: W/A/S/D → `player->setKeyHeld`; E press → `scene->interact()`; events with `isAutoRepeat()` are ignored (F8); other keys go to the base class. `focusOutEvent` → `player->stop()`.
- `MainWindow`: menu `playRequested` → `showGame()` (switch, `start()`); game `backRequested` → `showMenu()` (which now also calls game `stop()`).

- [ ] **Step 1: Write the failing tests.** Helper used below: `seat(scene, chair)` = `seatNewCustomer(chair)` then `customerTick()` until that customer is `Seated` (at most 600 ticks), returning it.

```cpp
// tst_cafescene.cpp
void worldIsBuilt() {
    CafeScene s; QCOMPARE(s.sceneRect(), QRectF(0, 0, 1000, 665));
    QCOMPARE(s.counters().size(), 10); QCOMPARE(s.chairs().size(), 8);
    QCOMPARE(s.interactables().size(), 22); QCOMPARE(s.solidRects().size(), 29);
    QCOMPARE(s.findCounter("cup")->stock(), 3); QCOMPARE(s.findCounter("sugar")->stock(), 2);
    QCOMPARE(s.findCounter("boba")->stock(), 1); QVERIFY(s.findCounter("tea") == nullptr);
    QVERIFY(s.solidRects().contains(QRect(448, 75, 4, 275)));   QVERIFY(s.solidRects().contains(QRect(0, 423, 450, 4)));
    QVERIFY(s.solidRects().contains(QRect(575, 475, 400, 100))); QVERIFY(s.solidRects().contains(QRect(585, 398, 55, 60)));
    QCOMPARE(s.interactables().at(10)->rect(), QRect(150, 150, 75, 75));
    QCOMPARE(s.interactables().at(14)->rect(), QRect(575, 385, 75, 75));
    QCOMPARE(s.chairs().at(5)->rect(), QRect(800, 62, 75, 75));
}
void startingSpotIsInReachOfThePrepTable() { CafeScene s; s.frameTick(); QCOMPARE(s.target(), s.interactables().at(10)); }
void nothingInReach()   { CafeScene s; s.player()->moveTo(500, 500); s.frameTick(); QVERIFY(s.target() == nullptr); }
void interactWithNothingIsHarmless() {                      // Review Focus 5
    CafeScene s; s.player()->moveTo(500, 500); s.frameTick();
    QSignalSpy stock(&s, &CafeScene::stockChanged); s.interact(); QCOMPARE(stock.count(), 0);
}
void emptyChairDoesNotBlockItsNeighbour() {                 // F5
    CafeScene s; s.player()->moveTo(630, 340); s.frameTick(); QVERIFY(s.target() == nullptr);
    seat(s, s.chairs().at(1)); s.frameTick(); QVERIFY(s.target() == s.chairs().at(1));
}
void takingFromACounter() {
    CafeScene s; s.player()->moveTo(250, 100); s.frameTick(); QCOMPARE(s.target(), s.interactables().at(0));
    QSignalSpy stock(&s, &CafeScene::stockChanged); s.interact();
    QCOMPARE(stock.count(), 1); QCOMPARE(s.findCounter("cup")->stock(), 2); QVERIFY(s.player()->heldItem() != nullptr);
}
void emptyCounterIsReported() {
    CafeScene s; s.player()->moveTo(160, 45); s.frameTick(); QSignalSpy empty(&s, &CafeScene::counterEmpty);
    s.interact(); s.player()->takeItem(); s.interact();
    QCOMPARE(empty.count(), 1); QVERIFY(s.player()->heldItem() == nullptr);
}
void restockAnnounces() { CafeScene s; QSignalSpy stock(&s, &CafeScene::stockChanged);
                          s.restock(s.findCounter("boba"), 9); QCOMPARE(s.findCounter("boba")->stock(), 10); QCOMPARE(stock.count(), 1); }
void seatingAnnouncesAndNumbers() {
    CafeScene s; QSignalSpy orders(&s, &CafeScene::ordersChanged);
    Customer *a = seat(s, s.chairs().at(5)); Customer *b = seat(s, s.chairs().at(4));
    QCOMPARE(orders.count(), 2); QCOMPARE(s.seatedCustomers(), QList<Customer *>({a, b}));
    QCOMPARE(a->number(), 1); QCOMPARE(b->number(), 2); QCOMPARE(a->orderCost(), s.catalog().orderCost(a->order()));
}
void servingAnnouncesRenumbersAndCleansUp() {
    // seat a on chairs().at(4) (625,62) and b on chairs().at(5); const int pay = a->payout(0);
    // player()->give(std::make_unique<Cup>(a->order().ingredients())); moveTo(640, 20); frameTick();
    // spies on orderServed and ordersChanged; interact():
    //   orderServed emitted once with (true, pay); ordersChanged emitted once; money 1000 + pay;
    //   seatedCustomers() == {b}; b->number() == 1; customerCount() == 2.
    // then customerTick() x 500 -> customerCount() == 1 (a was deleted by the scene).
}
void wrongOrderAnnouncesALoss()  { /* as above with Cup({"sugar"}): orderServed(false, -3); money 997 */ }
void spawnTickSeatsSomeone() {
    CafeScene s; for (int i = 0; i < 12 && s.customerCount() == 0; ++i) s.spawnTick();   // chance decays to 0 by the 11th roll
    QCOMPARE(s.customerCount(), 1); QCOMPARE(s.spawnOdds().chance(), 100);
}
void noSpawnWhenFull() { CafeScene s; for (Chair *c : s.chairs()) s.seatNewCustomer(c);
                         for (int i = 0; i < 50; ++i) s.spawnTick(); QCOMPARE(s.customerCount(), 8); }
void startStop()       { CafeScene s; QVERIFY(!s.isRunning()); s.start(); s.start(); QVERIFY(s.isRunning()); s.stop(); QVERIFY(!s.isRunning()); }

// tst_gamescreen.cpp   (sendKey(widget, QEvent::KeyPress | KeyRelease, key, autoRepeat = false) builds a QKeyEvent and sends it)
void viewIsInert() { GameScreen g; auto *v = g.findChild<QGraphicsView *>(); QCOMPARE(v->geometry(), QRect(0, 0, 1000, 665));
                     QCOMPARE(v->focusPolicy(), Qt::NoFocus); QCOMPARE(g.focusPolicy(), Qt::StrongFocus); }
void keysDriveThePlayer() {
    GameScreen g; sendKey(&g, QEvent::KeyPress, Qt::Key_D); g.scene()->frameTick();
    QVERIFY(g.scene()->player()->pos().x() > 100);
    const QPointF where = g.scene()->player()->pos();
    sendKey(&g, QEvent::KeyRelease, Qt::Key_D); g.scene()->frameTick(); QCOMPARE(g.scene()->player()->pos(), where);
}
void eInteractsOncePerPress() {                             // F8
    GameScreen g; g.scene()->player()->moveTo(250, 100); g.scene()->frameTick();
    sendKey(&g, QEvent::KeyPress, Qt::Key_E, true);  QVERIFY(g.scene()->player()->heldItem() == nullptr);   // auto-repeat ignored
    sendKey(&g, QEvent::KeyPress, Qt::Key_E);        QVERIFY(g.scene()->player()->heldItem() != nullptr);
}
void losingFocusStopsThePlayer() {                          // Review Focus 2
    GameScreen g; sendKey(&g, QEvent::KeyPress, Qt::Key_D);
    QFocusEvent out(QEvent::FocusOut); QApplication::sendEvent(&g, &out);
    g.scene()->frameTick(); QCOMPARE(g.scene()->player()->pos(), QPointF(100, 100));
}
void backButtonEmits()   { /* click "backButton" -> backRequested once */ }
void stopClearsHeldKeys(){ /* press D, g.stop(), g.start(), frameTick -> still at (100, 100) */ }

// tst_mainwindow.cpp
void playBackPlay() {                                       // Review Focus 4
    MainWindow w; QTest::mouseClick(w.findChild<ImageButton *>("playButton"), Qt::LeftButton);
    auto *game = qobject_cast<GameScreen *>(w.currentScreen()); QVERIFY(game); QVERIFY(game->scene()->isRunning());
    game->scene()->player()->addMoney(-1);
    QTest::mouseClick(game->findChild<ImageButton *>("backButton"), Qt::LeftButton);
    QVERIFY(qobject_cast<MenuScreen *>(w.currentScreen())); QVERIFY(!game->scene()->isRunning());
    QTest::mouseClick(w.findChild<ImageButton *>("playButton"), Qt::LeftButton);
    QVERIFY(game->scene()->isRunning()); QCOMPARE(game->scene()->player()->money(), 999);   // same game resumed
}
```

- [ ] **Step 2: Run** `test.bat` — expected: build fails (`world/CafeScene.h`).
- [ ] **Step 3: Implement** `CafeScene`, then `GameScreen`, then the `MainWindow` wiring.
- [ ] **Step 4: Run** `test.bat` — expected: all PASS, zero warnings.
- [ ] **Step 5: Play it.** `run.bat` → Play. Check against the Java: walking speed and diagonal; the player is stopped by counters, tables, chairs and walls and cannot leave the play area; pop-ups appear in the right places; take a cup, put it on a prep table, add a liquid and a topping, pick it up; customers arrive, sit, show a number; serve one; it walks out and disappears; an empty counter shows the message for a second.
- [ ] **Step 6:** Tick; update `CLAUDE.md` (stage D done).

---

### Task 9: `RestockingPanel`

**Files:**
- Create: `src/panels/RestockingPanel.{h,cpp}`
- Test: `tests/tst_panels.cpp`

**Interfaces:**
- Consumes: `CafeScene` (`counters`, `findCounter`, `catalog`, `player`, `restock`, `stockChanged`), `Player::moneyChanged`, `ImageButton`.
- Produces:

```cpp
class RestockingPanel : public QWidget { Q_OBJECT
public:
    explicit RestockingPanel(CafeScene *scene, QWidget *parent = nullptr);
    QString selectedItem() const;       // "boba" when the combo shows "boba counter"
    int quantity() const;  int unitCost() const;  int totalCost() const;  bool canAfford() const;
    int remaining() const;              // the selected counter's stock, read live (F9)
public slots:
    void changeQuantity(int delta);     // +1, -1, +10, -10; a decrease larger than the quantity is ignored
    void restock();                     // only if quantity > 0 and money >= totalCost
};
```

- Size 375 × 150. Children (panel coordinates): `QComboBox` `"counterCombo"` at (10, 10, 130, 20), at most 5 visible rows, `Qt::NoFocus`, entries `"<item> counter"` for every counter, sorted; `ImageButton`s `"restockAction"` (15, 105, 120, 40), `"plus1"` (170, 10, 50, 50), `"minus1"` (170, 70, 50, 50), `"plus10"` (230, 10, 50, 50), `"minus10"` (230, 70, 50, 50).
- `paintEvent`: `restockBackground` over the whole panel; the selected item's image 70 × 70 at (40, 32); 14 px text: `"Buying: N"` (290, 20), `"Cost: C"` (290, 50) where C is the total, `"Remaining: R"` (290, 80), and at (170, 140) `"Insufficient money"` or `"Money after purchase: M"`.
- `"restockAction"` shows `restockActionBtn`, or `restockActionBtnGrey` when `!canAfford()`.
- `restock()`: `scene->restock(counter, quantity)`, `player->addMoney(-totalCost)`, quantity back to 0. The quantity survives changing the combo selection (as in the Java).
- Refreshes on `moneyChanged`, `stockChanged`, selection change and quantity change (F9).

- [ ] **Step 1: Write the failing tests:**

```cpp
void comboListsCountersSorted() {
    CafeScene s; RestockingPanel p(&s); auto *combo = p.findChild<QComboBox *>("counterCombo");
    QStringList shown; for (int i = 0; i < combo->count(); ++i) shown << combo->itemText(i);
    QCOMPARE(shown, QStringList({"boba counter", "cup counter", "grapefruit counter", "greentea counter", "ice counter",
                                 "jelly counter", "lychee counter", "mango counter", "milk counter", "sugar counter"}));
    QCOMPARE(p.selectedItem(), QString("boba")); QCOMPARE(p.unitCost(), 2); QCOMPARE(p.quantity(), 0);
}
void quantityButtons() {
    CafeScene s; RestockingPanel p(&s);
    p.changeQuantity(-1);  QCOMPARE(p.quantity(), 0);
    p.changeQuantity(+10); p.changeQuantity(-1); QCOMPARE(p.quantity(), 9); QCOMPARE(p.totalCost(), 18);
    p.changeQuantity(-10); QCOMPARE(p.quantity(), 9);
    QTest::mouseClick(p.findChild<ImageButton *>("plus1"), Qt::LeftButton); QCOMPARE(p.quantity(), 10);
}
void restockBuys() {
    CafeScene s; RestockingPanel p(&s); p.changeQuantity(+10); p.changeQuantity(-1);
    QTest::mouseClick(p.findChild<ImageButton *>("restockAction"), Qt::LeftButton);
    QCOMPARE(s.findCounter("boba")->stock(), 10); QCOMPARE(s.player()->money(), 982); QCOMPARE(p.quantity(), 0);
    p.restock(); QCOMPARE(s.player()->money(), 982);                      // quantity 0: nothing happens
}
void tooExpensiveIsGreyAndRefused() {
    CafeScene s; RestockingPanel p(&s); for (int i = 0; i < 60; ++i) p.changeQuantity(+10);   // 600 x $2 = 1200
    QVERIFY(!p.canAfford()); QCOMPARE(p.findChild<ImageButton *>("restockAction")->currentImage(), QString("restockActionBtnGrey"));
    p.restock(); QCOMPARE(s.findCounter("boba")->stock(), 1); QCOMPARE(s.player()->money(), 1000);
}
void selectionChangesCost() { /* setCurrentText("sugar counter") -> selectedItem "sugar", unitCost 1; "cup counter" -> 1 */ }
void remainingIsLive() {                   // F9
    CafeScene s; RestockingPanel p(&s); QCOMPARE(p.remaining(), 1);
    s.player()->moveTo(160, 45); s.frameTick(); s.interact();          // takes the only boba
    QCOMPARE(p.remaining(), 0);
}
```

- [ ] **Step 2: Run** `test.bat panels` — expected: build fails.
- [ ] **Step 3: Implement.**
- [ ] **Step 4: Run** `test.bat panels` — expected: PASS, zero warnings.
- [ ] **Step 5:** Tick.

---

### Task 10: `OrderPanel` and `OrderCard`

**Files:**
- Create: `src/panels/OrderPanel.{h,cpp}`, `src/panels/OrderCard.{h,cpp}`
- Test: `tests/tst_panels.cpp`

**Interfaces:**
- Consumes: `CafeScene::seatedCustomers`, `ordersChanged`; `Customer::order`, `orderCost`, `payout`; `Player::tips`, `tipsChanged`.
- Produces:

```cpp
class OrderCard : public QWidget {      // copies what it shows; keeps no pointer to the customer
public:
    OrderCard(int number, const QStringList &ingredients, int cost, int payout, QWidget *parent = nullptr);
    int number() const;  int payout() const;
};
class OrderPanel : public QWidget { Q_OBJECT
public:
    explicit OrderPanel(CafeScene *scene, QWidget *parent = nullptr);
    int cardCount() const;
    const OrderCard *card(int index) const;
public slots:
    void rebuild();                     // one card per seated customer, in number order
};
```

- `OrderCard`: fixed 350 × 100. Paints `orderDisplayBox` scaled to 350 × 100; 48 px `"#N"` at (30, 65); 18 px ingredient *i* at (110, 45 + 15·i); `"$cost"` at (250, 45); `"$payout"` at (250, 75).
- `OrderPanel`: 375 × 150. A frameless `QScrollArea` filling it (`widgetResizable`, vertical single step 8, `Qt::NoFocus`) whose content widget has background `#FFE2DF` and a `QVBoxLayout` of cards followed by a stretch. This is the project's one real layout; keep it simple and readable.
- `rebuild()` runs on `ordersChanged` and `tipsChanged`; the payout passed to each card is `customer->payout(player->tips())` (F4).

- [ ] **Step 1: Write the failing tests** (reuse the `seat` helper):

```cpp
void oneCardPerSeatedCustomer() {
    CafeScene s; OrderPanel p(&s); QCOMPARE(p.cardCount(), 0);
    Customer *a = seat(s, s.chairs().at(5)); QCOMPARE(p.cardCount(), 1);
    QCOMPARE(p.card(0)->number(), 1); QCOMPARE(p.card(0)->payout(), a->payout(0));
    seat(s, s.chairs().at(4)); QCOMPARE(p.cardCount(), 2); QCOMPARE(p.card(1)->number(), 2);
}
void tipsRaiseTheShownPayout() {       // F4
    CafeScene s; OrderPanel p(&s); Customer *a = seat(s, s.chairs().at(5));
    s.player()->addTip(); QCOMPARE(p.card(0)->payout(), a->payout(1));
}
void cardsDoNotOutliveTheirCustomer() {     // Review Focus 3
    // seat a on chairs().at(4); give the player a matching cup; moveTo(640, 20); frameTick(); interact();
    // QCOMPARE(p.cardCount(), 0); then customerTick() x 500 (a is deleted); p.grab() must not crash.
}
void cardSize() { OrderCard c(1, {"milk", "boba"}, 5, 7); QCOMPARE(c.size(), QSize(350, 100)); }
```

- [ ] **Step 2: Run** `test.bat panels` — expected: build fails.
- [ ] **Step 3: Implement.** Delete old cards with `deleteLater()` or by deleting the layout's widgets directly; either is fine as long as the layout is left empty before the new cards are added.
- [ ] **Step 4: Run** `test.bat panels` — expected: PASS, zero warnings.
- [ ] **Step 5:** Tick.

---

### Task 11: `UpgradesPanel`

**Files:**
- Create: `src/panels/UpgradesPanel.{h,cpp}`
- Test: `tests/tst_panels.cpp`

**Interfaces:**
- Consumes: `CafeScene::spawnOdds`, `player`; `Player::addMoney`, `addSpeed`, `addTip`, `moneyChanged`; `ImageButton`.
- Produces:

```cpp
enum class Upgrade { Advertisement, Energy, Cuteness, Meowmax };
class UpgradesPanel : public QWidget { Q_OBJECT
public:
    explicit UpgradesPanel(CafeScene *scene, QWidget *parent = nullptr);
    int level(Upgrade upgrade) const;
    int cost(Upgrade upgrade) const;
    int maxLevel(Upgrade upgrade) const;   // 5 for the first three (8, then 11, after Meowmax); always 2 for Meowmax
public slots:
    void buy(Upgrade upgrade);             // only if money >= cost and level < maxLevel
};
```

| Upgrade | First cost | Cost step | Effect of buying | Button (`objectName`, rect) | Text at |
|---|---|---|---|---|---|
| Advertisement | 20 | +5 | `spawnOdds().setAdvertisementLevel(new level)` (F2) | `"advertisementButton"` 63, 183, 100, 25 | (47, 159) |
| Energy | 10 | +5 | `player->addSpeed(0.3)` | `"energyButton"` 237, 183, 100, 25 | (220, 159) |
| Cuteness | 50 | +15 | `player->addTip()` | `"cutenessButton"` 63, 331, 100, 25 | (47, 306) |
| Meowmax | 100 | +100 | the other three's max level + 3 | `"meowmaxButton"` 237, 331, 100, 25 | (220, 306) |

- Buying subtracts the cost, then raises the level and the cost. Size 400 × 400.
- `paintEvent`: `upgradesBackground` 400 × 400; white 12 px text per upgrade: `"Upgrade Level: L/M"` at the position above and, 15 px lower, `"Cost to Upgrade: C"` or `"MAX LEVEL"`.
- Each button shows `max` at max level, else `insufficient` when money < cost, else `upgrade`; refreshed on `moneyChanged` and after `buy` (F10: never inside `paintEvent`). A button showing `max` or `insufficient` is disabled, so it neither buys nor clicks on hover (the Java silences these on purpose).
- Each button's `clicked` is connected with a lambda that calls `buy(...)` with its upgrade.

- [ ] **Step 1: Write the failing tests:**

```cpp
void startingState() {
    CafeScene s; UpgradesPanel p(&s);
    QCOMPARE(p.cost(Upgrade::Advertisement), 20); QCOMPARE(p.cost(Upgrade::Energy), 10);
    QCOMPARE(p.cost(Upgrade::Cuteness), 50);      QCOMPARE(p.cost(Upgrade::Meowmax), 100);
    QCOMPARE(p.maxLevel(Upgrade::Energy), 5);     QCOMPARE(p.maxLevel(Upgrade::Meowmax), 2);
    QCOMPARE(p.findChild<ImageButton *>("energyButton")->currentImage(), QString("upgrade"));
    QVERIFY(p.findChild<ImageButton *>("energyButton")->isEnabled());
}
void energy() {
    CafeScene s; UpgradesPanel p(&s);
    QTest::mouseClick(p.findChild<ImageButton *>("energyButton"), Qt::LeftButton);
    QCOMPARE(p.level(Upgrade::Energy), 1); QCOMPARE(p.cost(Upgrade::Energy), 15);
    QCOMPARE(s.player()->money(), 990);    QCOMPARE(s.player()->speed(), 2.5);
    for (int i = 0; i < 10; ++i) p.buy(Upgrade::Energy);                    // 15 + 20 + 25 + 30 more, then capped
    QCOMPARE(p.level(Upgrade::Energy), 5); QCOMPARE(s.player()->money(), 900);
    QCOMPARE(p.findChild<ImageButton *>("energyButton")->currentImage(), QString("max"));
}
void advertisementAppliesTheNewLevel() {   // F2
    CafeScene s; UpgradesPanel p(&s); p.buy(Upgrade::Advertisement);
    QCOMPARE(s.spawnOdds().divisor(), 1.1); QCOMPARE(s.player()->money(), 980); QCOMPARE(p.cost(Upgrade::Advertisement), 25);
}
void cuteness()  { /* buy -> player tips 1, money 950, cost 65 */ }
void meowmaxRaisesTheOthers() {
    CafeScene s; UpgradesPanel p(&s); p.buy(Upgrade::Meowmax);
    QCOMPARE(p.maxLevel(Upgrade::Energy), 8); QCOMPARE(p.cost(Upgrade::Meowmax), 200); QCOMPARE(s.player()->money(), 900);
    p.buy(Upgrade::Meowmax); QCOMPARE(p.maxLevel(Upgrade::Cuteness), 11); QCOMPARE(s.player()->money(), 700);
    p.buy(Upgrade::Meowmax); QCOMPARE(p.level(Upgrade::Meowmax), 2); QCOMPARE(s.player()->money(), 700);
}
void tooPoor() {
    CafeScene s; UpgradesPanel p(&s); s.player()->addMoney(-995);           // $5 left
    QCOMPARE(p.findChild<ImageButton *>("energyButton")->currentImage(), QString("insufficient"));
    QVERIFY(!p.findChild<ImageButton *>("energyButton")->isEnabled());
    p.buy(Upgrade::Energy); QCOMPARE(p.level(Upgrade::Energy), 0); QCOMPARE(s.player()->money(), 5);
}
```

- [ ] **Step 2: Run** `test.bat panels` — expected: build fails.
- [ ] **Step 3: Implement.** Keep the four upgrades as data (one small struct each: level, cost, cost step) rather than four copies of the same code.
- [ ] **Step 4: Run** `test.bat panels` — expected: PASS, zero warnings.
- [ ] **Step 5:** Tick.

---

### Task 12: `HeldItemPanel`, the HUD, and panel switching in `GameScreen`

**Files:**
- Create: `src/panels/HeldItemPanel.{h,cpp}`
- Modify: `src/screens/GameScreen.{h,cpp}`
- Test: `tests/tst_gamescreen.cpp`

**Interfaces:**
- Consumes: the three panels (Tasks 9–11), `Player` signals, `CafeScene::orderServed`.
- Produces:

```cpp
class HeldItemPanel : public QWidget { Q_OBJECT
public:
    explicit HeldItemPanel(Player *player, QWidget *parent = nullptr);
    QString shownName() const;          // the held item's name, or "Nothing"
};
// GameScreen gains:
    enum class Panel { None, Restock, Orders, Upgrades };
    Panel openPanel() const;
    QString bannerImage() const;        // "itemBanner", "restockBanner", "ordersBanner" or "upgradesBanner"
public slots:
    void togglePanel(Panel panel);      // toggles it; opening one closes the others
    void showAudioState(bool soundOn, bool musicOn);
signals:
    void soundToggleRequested();  void musicToggleRequested();
    void orderServed(bool correct);     // forwarded from the scene, for the meows
```

- `HeldItemPanel`: 375 × 150, repaints on `heldItemChanged`. Paints (panel coordinates): `currentItemBackground`; colour `#5097B9`; 48 px `"Current Item: "` at (20, 45) and the name or `"Nothing"` at (20, 105); when holding something, its image 90 × 90 at (280, 25); when it is a cup, 36 px `"Ingredients: "` at (120, 85) and 24 px ingredient names: the first two at x = 100 with baselines 115 and 140, every later one at x = 200 alternating baselines 115 and 140.
- `GameScreen` children, created in this order so later ones are on top (screen coordinates):
  - `HeldItemPanel` at (0, 435).
  - Banner: a `QLabel` at (375, 435, 75, 150) showing `bannerImage()`.
  - `ImageButton`s: `"restockButton"` `restockBtn` (20, 600, 200, 50); `"ordersButton"` `ordersBtn` (240, 600, 200, 50); `"upgradesButton"` `upgradesBtn` (460, 600, 200, 50); `"musicButton"` `musicOn` (850, 600, 50, 50); `"soundButton"` `soundOn` (920, 600, 50, 50).
  - `QLabel` `"moneyLabel"`: `"Money: $1000"`, 24 px, `rgb(21, 120, 17)`, left edge x = 700, text baseline y = 650; follows `moneyChanged`.
  - `QLabel` `"moneyDeltaLabel"`: hidden; on `orderServed(correct, delta)` shows `"+$7"` in the same green or `"-$3"` in red, 24 px, with its baseline starting at (780, 620) and drifting 13 px down over 2000 ms (a `QPropertyAnimation` on `pos`), then hides.
  - `RestockingPanel` and `OrderPanel` at (0, 435); `UpgradesPanel` at (300, 150); all hidden.
- The three panel buttons each call `player->stop()` then `togglePanel(...)`. The banner is the open panel's banner, or `itemBanner` when none is open.
- While the Upgrades panel is open, key presses for W/A/S/D/E are ignored (Open Question Q1 default; the Java behaves this way). Restock and Orders do not lock anything.
- `showAudioState` sets `"soundButton"` to `soundOn` / `soundOff` and `"musicButton"` to `musicOn` / `musicOff`. The two buttons only emit the `…ToggleRequested` signals.

- [ ] **Step 1: Write the failing tests:**

```cpp
void heldItemPanelShowsTheName() {
    GameScreen g; auto *panel = g.findChild<HeldItemPanel *>(); QCOMPARE(panel->shownName(), QString("Nothing"));
    g.scene()->player()->give(g.scene()->catalog().make("milk")); QCOMPARE(panel->shownName(), QString("milk"));
    QCOMPARE(panel->geometry(), QRect(0, 435, 375, 150));
}
void panelsToggle() {
    GameScreen g; QCOMPARE(g.openPanel(), GameScreen::Panel::None); QCOMPARE(g.bannerImage(), QString("itemBanner"));
    QTest::mouseClick(g.findChild<ImageButton *>("restockButton"), Qt::LeftButton);
    QCOMPARE(g.openPanel(), GameScreen::Panel::Restock); QCOMPARE(g.bannerImage(), QString("restockBanner"));
    QVERIFY(!g.findChild<RestockingPanel *>()->isHidden()); QVERIFY(g.findChild<OrderPanel *>()->isHidden());
    g.togglePanel(GameScreen::Panel::Orders);
    QVERIFY(g.findChild<RestockingPanel *>()->isHidden()); QVERIFY(!g.findChild<OrderPanel *>()->isHidden());
    QCOMPARE(g.bannerImage(), QString("ordersBanner"));
    g.togglePanel(GameScreen::Panel::Orders);
    QCOMPARE(g.openPanel(), GameScreen::Panel::None); QCOMPARE(g.bannerImage(), QString("itemBanner"));
    QCOMPARE(g.findChild<UpgradesPanel *>()->geometry(), QRect(300, 150, 400, 400));
}
void openingAPanelStopsThePlayer() {
    GameScreen g; sendKey(&g, QEvent::KeyPress, Qt::Key_D);
    QTest::mouseClick(g.findChild<ImageButton *>("ordersButton"), Qt::LeftButton);
    g.scene()->frameTick(); QCOMPARE(g.scene()->player()->pos(), QPointF(100, 100));
}
void keysWorkWithAPanelOpen() {            // Review Focus 5
    GameScreen g; g.togglePanel(GameScreen::Panel::Restock);
    sendKey(&g, QEvent::KeyPress, Qt::Key_D); g.scene()->frameTick(); QVERIFY(g.scene()->player()->pos().x() > 100);
    QCOMPARE(g.findChild<QComboBox *>("counterCombo")->focusPolicy(), Qt::NoFocus);
    // the same holds with Panel::Orders open
}
void upgradesPanelLocksTheControls() {     // Open Question Q1 default
    GameScreen g; g.scene()->player()->moveTo(250, 100); g.scene()->frameTick();
    g.togglePanel(GameScreen::Panel::Upgrades);
    sendKey(&g, QEvent::KeyPress, Qt::Key_D); sendKey(&g, QEvent::KeyPress, Qt::Key_E); g.scene()->frameTick();
    QCOMPARE(g.scene()->player()->pos(), QPointF(250, 100)); QVERIFY(g.scene()->player()->heldItem() == nullptr);
    g.togglePanel(GameScreen::Panel::Upgrades);                  // closed again: keys work
    sendKey(&g, QEvent::KeyPress, Qt::Key_E); QVERIFY(g.scene()->player()->heldItem() != nullptr);
}
void moneyLabelFollows()  { GameScreen g; g.scene()->player()->addMoney(-3);
                            QCOMPARE(g.findChild<QLabel *>("moneyLabel")->text(), QString("Money: $997")); }
void servingShowsTheDelta() {
    // seat a customer on chairs().at(4); give Cup({"sugar"}); moveTo(640, 20); frameTick(); QSignalSpy served(&g, &GameScreen::orderServed);
    // sendKey E -> moneyDeltaLabel text "-$3", !isHidden(); served.count() == 1 and its argument is false.
}
void audioIcons() { GameScreen g; g.showAudioState(false, false);
                    QCOMPARE(g.findChild<ImageButton *>("soundButton")->currentImage(), QString("soundOff"));
                    QCOMPARE(g.findChild<ImageButton *>("musicButton")->currentImage(), QString("musicOff")); }
void toggleButtonsOnlyAsk() { /* clicking soundButton emits soundToggleRequested once and does not change its image */ }
void screenshot() { /* press nothing; g.grab().save("screenshots/game.png"); also with each panel open: game_restock/orders/upgrades.png */ }
```

- [ ] **Step 2: Run** `test.bat gamescreen` — expected: build fails.
- [ ] **Step 3: Implement.**
- [ ] **Step 4: Run** `test.bat` — expected: all PASS, zero warnings.
- [ ] **Step 5: Look and play.** Compare the four `screenshots/game*.png` with the Java game screen, adjusting label positions until the text sits where the Java draws it. Then `run.bat`: buy stock, read orders, buy each upgrade; movement still works with every panel open.
- [ ] **Step 6:** Tick; update `CLAUDE.md` (stage E done).

---

### Task 13: `SoundManager` and the last wiring

**Files:**
- Create: `src/audio/SoundManager.{h,cpp}`
- Modify: `src/MainWindow.{h,cpp}`
- Test: `tests/tst_sound.cpp`, `tests/tst_mainwindow.cpp`

**Interfaces:**
- Consumes: `Assets::musicFilePath`, `ImageButton::hovered`, `GameScreen` signals and `showAudioState`.
- Produces:

```cpp
class SoundManager : public QObject { Q_OBJECT
public:
    explicit SoundManager(const QString &musicFile, QObject *parent = nullptr);
    bool soundOn() const;  bool musicOn() const;   // both start true
    bool isMusicPlaying() const;                   // what the manager asked for, not what the speakers do
public slots:
    void playClick();
    void playMeow(bool happy);
    void startMusic();     // plays only if soundOn && musicOn and the file exists
    void stopMusic();
    void toggleSound();
    void toggleMusic();
signals:
    void stateChanged(bool soundOn, bool musicOn);
};
```

- Effects: `QSoundEffect` from `qrc:/sounds/click.wav`, `meowHappy.wav`, `meowAngry.wav`; silent while sound is off (F3). Music: `QMediaPlayer` + `QAudioOutput`, looping forever, volume 0.32. If `QMediaPlayer` proves unreliable here, a looping `QSoundEffect` is the accepted fallback (spec §6.5).
- `toggleSound()`: flip `soundOn`; `musicOn` becomes equal to it; music plays exactly when it is on; emit `stateChanged`.
- `toggleMusic()`: ignored while sound is off; otherwise flip `musicOn`, start or stop the music, emit `stateChanged`.
- A missing music file: one `qWarning`, `isMusicPlaying()` stays false, everything else works.
- `MainWindow` wiring (all in its constructor, after the screens exist):
  - `for (ImageButton *b : findChildren<ImageButton *>()) connect(b, &ImageButton::hovered, m_sounds, &SoundManager::playClick);` (F10)
  - game `orderServed` → `playMeow`; game `soundToggleRequested` → `toggleSound`; `musicToggleRequested` → `toggleMusic`; `stateChanged` → game `showAudioState`.
  - `showGame()` also calls `startMusic()`; `showMenu()` also calls `stopMusic()`.
  - `MainWindow` gains `SoundManager *sounds() const;` for tests.

- [ ] **Step 1: Write the failing tests:**

```cpp
// tst_sound.cpp
void flagsAndToggles() {
    SoundManager m(Assets::musicFilePath()); QSignalSpy changed(&m, &SoundManager::stateChanged);
    QVERIFY(m.soundOn()); QVERIFY(m.musicOn()); QVERIFY(!m.isMusicPlaying());
    m.startMusic();  QVERIFY(m.isMusicPlaying());
    m.toggleMusic(); QVERIFY(!m.musicOn()); QVERIFY(m.soundOn()); QVERIFY(!m.isMusicPlaying());
    m.toggleSound(); QVERIFY(!m.soundOn()); QVERIFY(!m.musicOn());
    m.toggleMusic(); QVERIFY(!m.musicOn());                    // ignored while sound is off
    m.toggleSound(); QVERIFY(m.soundOn()); QVERIFY(m.musicOn()); QVERIFY(m.isMusicPlaying());
    QCOMPARE(changed.count(), 3);
    m.stopMusic();   QVERIFY(!m.isMusicPlaying()); QVERIFY(m.musicOn());
}
void startMusicRespectsFlags() { SoundManager m(Assets::musicFilePath()); m.toggleMusic(); m.startMusic(); QVERIFY(!m.isMusicPlaying()); }
void missingMusicFileIsSurvivable() {      // Review Focus 1
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("nope"));
    SoundManager m("Z:/nope.wav"); m.startMusic(); QVERIFY(!m.isMusicPlaying());
    m.playClick(); m.playMeow(true); m.toggleSound(); m.toggleSound();         // none of these may crash
}
void effectsAreSafeWhenMuted() { SoundManager m(Assets::musicFilePath()); m.toggleSound(); m.playClick(); m.playMeow(false); }
// tst_mainwindow.cpp
void musicFollowsTheScreens() {
    MainWindow w; QVERIFY(!w.sounds()->isMusicPlaying());
    QTest::mouseClick(w.findChild<ImageButton *>("playButton"), Qt::LeftButton); QVERIFY(w.sounds()->isMusicPlaying());
    QTest::mouseClick(w.findChild<ImageButton *>("soundButton"), Qt::LeftButton);
    QVERIFY(!w.sounds()->soundOn());
    QCOMPARE(w.findChild<ImageButton *>("soundButton")->currentImage(), QString("soundOff"));
    QCOMPARE(w.findChild<ImageButton *>("musicButton")->currentImage(), QString("musicOff"));
    auto *game = qobject_cast<GameScreen *>(w.currentScreen());      // two screens have a "backButton": ask the game for its own
    QTest::mouseClick(game->findChild<ImageButton *>("backButton"), Qt::LeftButton); QVERIFY(!w.sounds()->isMusicPlaying());
}
```

- [ ] **Step 2: Run** `test.bat` — expected: build fails.
- [ ] **Step 3: Implement.**
- [ ] **Step 4: Run** `test.bat` — expected: all PASS, zero warnings.
- [ ] **Step 5: Listen.** `run.bat`: every button clicks on hover; music starts on Play, loops, stops on Back; both toggles behave as specified; right order → happy meow, wrong → angry; with sound off nothing is audible.
- [ ] **Step 6:** Tick; update `CLAUDE.md` (stage F done).

---

### Task 14: Play-through against the Java, polish, README

**Files:**
- Modify: whatever the play-through finds; every header (comment pass)
- Create: `README.md`

- [ ] **Step 1: Side by side.** Run the Java (`open.bat` in its folder) and `run.bat`. Tick each: menu and instructions look the same · player collision feels the same · every pop-up position · held-item icon beside the player · Current Item box text positions, including a cup with four ingredients · each panel's layout and numbers · floating money text · money colours · "out of stock" message position and duration · seated number bubbles · order cards scroll with three or more customers.
- [ ] **Step 1b: Pacing (spec §6.5).** In both games, time (stopwatch or screen recording, three runs each): a customer from appearing at the door to sitting on the top-right chair (212 ticks, 2.12 s at the nominal 10 ms), and the player walking from the starting spot straight down until stopped (nominally 2.2 px per 16 ms tick). The Java is expected to be slower: its timers measured about 11.1 ms and 17.2 ms on this machine before any painting cost. Write both sets of numbers into `CLAUDE.md` and **report them to the owner** with the timer periods that would match; do not change the periods unasked.
- [ ] **Step 2: Fixes behave.** F1: serve a customer and wait for the chair to be retaken while they walk out — nobody freezes. F3: sound off silences meows and clicks. F4: card payout equals what is paid. F5: stand between two long-table chairs with only the right one occupied — it can be served. F8: holding E does one thing.
- [ ] **Step 3: Display scaling.** In a console after `call env.bat`: `set QT_SCALE_FACTOR=1.5` then `build\CatCafe.exe`. Expected: everything is larger and still lined up; nothing clipped.
- [ ] **Step 4: Clean build.** Delete `build/`, run `build.bat` — expected: zero warnings. `test.bat` — expected: `100% tests passed`.
- [ ] **Step 5: Comment pass.** Every class header has the three-line comment required by Global Constraints; comments explain Qt's reasons, not what the next line does; no leftover debug output.
- [ ] **Step 6: Write `README.md`:** what this is; how to build, run and test with the `.bat` files; how to open it in Qt Creator (open `CMakeLists.txt`, choose the "Desktop Qt 6.11.2 MinGW 64-bit" kit); a pointer to `docs/GUIDE.md`; controls (W A S D, E).
- [ ] **Step 7:** Tick; update `CLAUDE.md`.

---

### Task 15: The guide

**Files:**
- Create: `docs/GUIDE.md`

Written from the finished code. Reader: knows Java OOP and Swing, knows C++ syntax, STL and structs, has never written a C++ class or used Qt (spec §9). Each chapter opens by naming the file it is anchored on, shows the owner's Java beside the C++ where that helps, and ends with a short "in the visualizer, this becomes…" note where one applies. ASCII diagrams only.

- [ ] **Step 1: Write Part 1 — C++ classes for a Java programmer.**
  1. Two files per class: headers, sources, `#include`, `#pragma once` — `items/Cup.h`, `Cup.cpp`
  2. Constructors, member initializer lists, `const` member functions, `explicit` — `items/Ingredient`, `world/NonCollidable`
  3. Values, references and pointers; `const &` parameters; `nullptr` — `Counter::interact(Player &)`
  4. No garbage collector: who deletes what — the two ownership systems in this project, side by side
  5. Inheritance: `virtual`, `override`, `= 0`, virtual destructors — `world/Interactable.h` against `Interactables.java`
  6. `enum class` and `dynamic_cast` in place of strings and `instanceof` — `Outcome`, `PopUpSide`, `Player::takeCup`
  7. `std::unique_ptr` and `std::move`: the cup changes hands — `Player::give`, `PrepTable::interact`
  8. Forward declarations and include cycles — `Chair.h` and `Customer.h`
  9. Habits from competitive programming to drop — `using namespace std;`, `<bits/stdc++.h>`, globals, plain `int` flags; and what to keep (`auto`, range-`for`)
- [ ] **Step 2: Write Part 2 — How Qt works.**
  10. The build: CMake, the kit, what `moc` generates and why `Q_OBJECT` exists, the resource system — `CMakeLists.txt`
  11. `QApplication` and the event loop, against Swing's event thread — `main.cpp`
  12. The object tree: parents own children; `findChildren` — `MainWindow`
  13. Signals and slots, against `ActionListener`; the one lambda — `MenuScreen` → `MainWindow`, `Player::moneyChanged`, `UpgradesPanel`
  14. Widgets, events and painting — `ImageButton`, `InstructionsScreen`, `GameScreen::keyPressEvent`
  15. Timers — `CafeScene`'s three ticks, the single-shot message
  16. Graphics View: scene, view, items, coordinates, z-order, child items — `CafeScene`, `NonCollidable`, `Player`
  17. Layouts and scroll areas, and why most of this game positions things by hand — `OrderPanel`
  18. The animation framework — the floating money text
  19. Sound — `SoundManager`
  20. Testing with Qt Test and `QSignalSpy` — `tests/tst_items.cpp`, `tests/tst_player.cpp`
- [ ] **Step 3: Write Part 3 — Reference.**
  21. Swing → Qt lookup table
  22. Java → C++ lookup table
  23. Where each of the 27 Java files went
  24. What was fixed: F1–F13 (and F14 / F15 if the owner's answers to Q1 / Q2 create them), each with the Java lines involved and what changed; then "noticed but kept on purpose" (spec §7, plus Q1 and Q2 if kept); then "differences that come from the toolkit" (button borders, combo box and scroll bar looks, text rasterisation, pacing if Task 14 found a difference)
  25. Build, run, test, open in Qt Creator
  26. From café to graph visualizer: nodes and edges as scene items; `ItemIsMovable` / `ItemIsSelectable` and `itemChange`; a step timer with play, pause and speed; control panels driving the scene through signals; and four exercises to try here first (make a table draggable; add a pause button; show the reach rectangles as an overlay; add an eleventh counter)
- [ ] **Step 4: Check it against the code.**
  - Every fenced C++ snippet that claims to come from a file appears verbatim in that file (search for a distinctive line of each).
  - Every one of the 27 Java file names appears in chapter 23; every `F1`…`F13` appears in chapter 24.
  - Every C++/Qt construct that appears in `src/` is explained somewhere: search `src/` for `dynamic_cast`, `static_cast`, `std::move`, `unique_ptr`, `make_unique`, `enum class`, `= 0`, `override`, `explicit`, `constexpr`, `auto`, `[this`, `Q_OBJECT`, `signals:`, `slots:`, `emit`, `connect(`, `findChild`, `QTimer`, `QPropertyAnimation`, `QSignalSpy`, forward declarations (`class X;`) and confirm each has a home in the guide.
  - Every file path the guide mentions exists.
- [ ] **Step 5:** Tick; set `CLAUDE.md` Progress to "complete", listing anything left for the owner to decide.
