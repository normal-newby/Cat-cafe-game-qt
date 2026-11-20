# The Cat Café guide

This guide uses your own game, rewritten in C++ and Qt, to teach two things:

1. **C++ classes**, for someone who knows classes from Java and C++ syntax from
   competitive programming.
2. **How Qt works**, for someone who knows Swing.

It assumes you know what a class, a subclass and an abstract method are. It spends its
time on what C++ and Qt do *differently*, and why.

## How to use it

- Every chapter starts by naming a file. Open that file beside the guide.
- Code in a `cpp` block is copied from this project exactly. Code in a `java` block is
  from your original. Code in a `text` block is a sketch, not real project code.
- Part 1 is about the language, Part 2 about the framework, Part 3 is tables to look
  things up in. Read Parts 1 and 2 in order once; after that, use Part 3.
- Build and run first, so you know it works: double-click `run.bat`. Details are in
  chapter 25.

## Contents

**Part 1 — C++ classes for a Java programmer**
1. Two files per class
2. Constructors, initializer lists, `const`, `explicit`
3. Values, references and pointers
4. No garbage collector: who deletes what
5. Inheritance: `virtual`, `override`, `= 0`
6. `enum class` and `dynamic_cast`
7. `unique_ptr` and `std::move`: the cup changes hands
8. Forward declarations and include cycles
9. Habits from competitive programming

**Part 2 — How Qt works**
10. The build: CMake, `moc`, resources
11. `QApplication` and the event loop
12. The object tree
13. Signals and slots
14. Widgets, events and painting
15. Timers
16. Graphics View: scene, view, items
17. Layouts and scroll areas
18. Animation
19. Sound
20. Testing

**Part 3 — Reference**
21. Swing → Qt
22. Java → C++
23. Where each Java file went
24. What was fixed, and what was kept
25. Build, run, test
26. From café to graph visualizer

---

# Part 1 — C++ classes for a Java programmer

## 1. Two files per class

**Open:** `src/items/Cup.h` and `src/items/Cup.cpp`

In Java, `Cup.java` holds everything about `Cup`. In C++ a class is normally split in two.

The **header** (`.h`) says what the class looks like: its functions and its fields, with
no function bodies.

```cpp
class Cup : public Item
{
public:
    Cup() = default;                                 // an empty cup
    explicit Cup(const QStringList &ingredients);    // a cup that already has things in it

    void add(const QString &ingredient);
    const QStringList &ingredients() const;

    // True when both cups hold the same ingredients, in any order.
    bool matches(const Cup &other) const;

    QString name() const override;     // always "cup"
    QString image() const override;    // "emptyCup" or "filledCup"

private:
    QStringList m_ingredients;
};
```

The **source** (`.cpp`) holds the bodies. Each one is prefixed with `Cup::` to say which
class it belongs to.

```cpp
void Cup::add(const QString &ingredient)
{
    m_ingredients.append(ingredient);
}
```

**Why two files.** The C++ compiler handles one `.cpp` file at a time and knows nothing
about the others. When `PrepTable.cpp` uses a `Cup`, the compiler needs to know what a
`Cup` is: how big it is and what functions it has. It does not need the bodies. So
`PrepTable.cpp` says `#include "items/Cup.h"`, which literally pastes the text of the
header in at that line. Afterwards a separate step, the linker, joins all the compiled
pieces together.

Java's compiler finds other classes for you. C++'s does not. `#include` is how you tell it.

Things to notice in the header:

| You see | It means |
|---|---|
| `#pragma once` at the top | "If this header gets pasted in twice, ignore the second time." Every header has it. |
| `class Cup : public Item` | `class Cup extends Item`. |
| `public:` and `private:` | They are section labels, not per-member modifiers. Everything after `public:` is public until the next label. |
| `};` | A class definition ends with a semicolon. Forgetting it produces confusing errors in the *next* file. |
| `#include <QString>` vs `#include "items/Item.h"` | Angle brackets for libraries, quotes for this project's own files. |
| `m_ingredients` | Qt's naming habit: members start with `m_`. It does the job of writing `this.` in Java. |

One class in this project has no `.cpp` at all. `src/world/Decoration.h` is so short that
its one function is written inside the header. That is allowed, and is normal for tiny
functions.

## 2. Constructors, initializer lists, `const`, `explicit`

**Open:** `src/items/Ingredient.cpp` and `src/world/Interactable.cpp`

A Java constructor assigns to fields in its body. A C++ constructor has a special place
for that, between the parameter list and the body:

```cpp
Ingredient::Ingredient(const ItemInfo &info)
    : m_name(info.name)
    , m_image(info.image)
{
}
```

That is the **member initializer list**. It matters because, in C++, every member is
already constructed by the time the body starts. Writing `m_name = info.name;` in the body
would construct an empty string first and then overwrite it. The initializer list builds
it correctly the first time. Some members can *only* be set there: references and `const`
members, which cannot be assigned to afterwards.

The base class is initialised in the same list. This is `super(...)`:

```cpp
Interactable::Interactable(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side)
    : NonCollidable(image, rect)   // the base class is constructed first, like Java's super(...)
    , m_reachX(reachX)
    , m_reachY(reachY)
    , m_side(side)
{
}
```

Simple defaults can be written beside the member instead, as in `Customer.h`
(`int m_target = 0;`), exactly as Java allows.

**`const` after a function.** Look at `Cup::matches(const Cup &other) const`. The second
`const` is a promise that calling this function does not change the cup. The compiler
enforces it. Java has nothing like it. The pay-off: if a function is handed a
`const Cup &`, it can call `matches()` and `ingredients()` but not `add()`, so you can see
from a signature whether something might be modified.

**`explicit`.** A constructor with one parameter also works as an automatic conversion:
without `explicit`, C++ would quietly turn a `QStringList` into a `Cup` wherever a `Cup`
was expected. `explicit` switches that off, so a cup is only made when you write `Cup(...)`.
Put it on every one-parameter constructor.

**`= default`.** `Cup() = default;` asks the compiler to write the obvious constructor
(one that does nothing extra).

**A protected constructor.** `NonCollidable` was `abstract` in your Java. C++ has no
`abstract` keyword: a class is abstract only if it has an abstract function (chapter 5),
and `NonCollidable` has none. Its constructor is `protected` instead, which has the same
effect: only subclasses can create one.

## 3. Values, references and pointers

**Open:** `src/world/Counter.cpp`

This is the biggest difference from Java, and everything else in Part 1 depends on it.

In Java, a variable of class type is always a reference to an object that lives somewhere
else. In C++ you choose, each time, between three things:

| Written | What it is | Can be "nothing"? | Used with |
|---|---|---|---|
| `Cup cup` | **A value.** The object itself, right here. Assigning it copies the whole object. | No | `cup.add(...)` |
| `Cup &cup` | **A reference.** Another name for an object that exists elsewhere. Cannot be changed to refer to something else. | No | `cup.add(...)` |
| `Cup *cup` | **A pointer.** The address of an object. Can be changed, and can be `nullptr`. | Yes | `cup->add(...)` |

A Java reference behaves most like a C++ pointer.

```cpp
Outcome Counter::interact(Player &player)
{
    if (player.heldItem() != nullptr)
        return Outcome::Nothing;       // paws are full

    if (m_stock == 0)
        return Outcome::CounterEmpty;

    --m_stock;
    player.give(m_catalog.make(m_itemName));   // a new Cup or Ingredient, now the player's
    return Outcome::TookItem;
}
```

- `Player &player` is a reference. Inside this function `player` *is* the caller's player,
  not a copy, and it cannot be null. Use `.` on it.
- `player.heldItem()` returns an `Item *`, a pointer, because the answer may be "nothing".
  `nullptr` is C++'s `null`.

**`const T &` parameters.** Almost every function in this project takes its arguments as
`const QString &name` or `const QRect &rect`. Read that as "let me look at the caller's
object without copying it, and I promise not to change it". For small things like `int`
and `bool`, just pass the value.

**Pointers into a container.** `ItemCatalog::find` returns a pointer so that it can say
"not found":

```cpp
    for (const ItemInfo &info : m_items) {
        if (info.name == name)
            return &info;   // the address of the element inside the list
    }
    return nullptr;
```

`&info` here means "the address of `info`". The same symbol `&` means "reference" in a
declaration and "address of" in an expression; context tells you which.

**Values really are copies.** `SpriteAnimation` is used as a value on purpose. In
`tests/tst_widgets.cpp`:

```cpp
        SpriteAnimation a("stationary");
        SpriteAnimation b = a;
        a.next();
        QCOMPARE(b.next().cacheKey(), b.frame(1).cacheKey());
```

`b = a` makes a second animation with its own position. In Java, `b = a` would make two
names for one object. This is how every customer gets its own animation in this version
(each `Customer` holds a *copy* of the sprites), where the Java shared one counter between
all of them.

**A reference as a member.** `Counter` has `const ItemCatalog &m_catalog;`. The counter
does not contain a catalog and does not own one; it knows where the scene's catalog is.
That only works if the catalog outlives the counter. It does: `CafeScene` owns both.

**What this project does, as rules of thumb:**

- Small plain data (`QRect`, `QPoint`, `QString`, `Cup` orders): values.
- "Use this, don't copy it": `const T &`.
- "Use this and change it": `T &`.
- "This might be absent", or "this is a Qt object that lives in a tree": `T *`.

## 4. No garbage collector: who deletes what

**Open:** `src/world/CafeScene.cpp`, function `customerTick`

In Java you write `new` and forget about it. In C++ an object made with `new` exists
until something calls `delete` on its pointer. If nothing does, it leaks. If something
does and another pointer still points at it, that pointer is now dangling, and using it
is a crash or worse.

The way to stay sane is to make sure every object has exactly one **owner**: one thing
responsible for its death. This project uses four kinds of owner, and calls `delete` by
hand in only two places.

| What | Owner | When it dies |
|---|---|---|
| Local variables and members that are values (`ItemCatalog m_catalog;`, `QPainter painter(this);`) | The enclosing scope or object | Automatically, when that scope ends or that object dies |
| Widgets, timers, anything given a parent (`new ImageButton(..., this)`) | The parent `QObject` | When the parent dies (chapter 12) |
| Things in the scene (`addItem(new Counter(...))`) | The scene | When the scene dies |
| What the player carries, and the cup on a prep table | A `std::unique_ptr` | When that `unique_ptr` dies or is given something else (chapter 7) |

So when you see `new` in this project with no matching `delete`, look for where the
pointer is handed to an owner on the same line or the next.

Both hand-written deletes are for things that must disappear in the middle of a game.
The first is a customer who has left. The scene owns customers, so it is asked to let go
first:

```cpp
            m_customers.removeAt(i);
            removeItem(customer);   // the scene gives ownership back to us...
            delete customer;        // ...so now it is ours to delete
```

The second is `qDeleteAll(m_cards)` in `OrderPanel::rebuild`, which calls `delete` on
every pointer in a list: the old order cards, thrown away before new ones are made.
Deleting a parented object early is always allowed. It removes itself from its parent
(and a widget from its layout) on the way out, so the parent will not delete it again.

**Dangling pointers are why `OrderCard` copies.** Your Java `OrderDisplayPanel` kept a
reference to its `Customer`. Here, that customer may be deleted while the card is still
on screen. So `OrderCard` is given the number, the ingredient names, the cost and the
payout as values, and holds no pointer at all. There is a test for exactly this,
`cardsDoNotOutliveTheirCustomer` in `tests/tst_panels.cpp`.

**Order of destruction.** When an object dies, its members die in reverse order of
declaration, and *then* its base class is destroyed. For `CafeScene` that means
`m_catalog` is gone before `QGraphicsScene` deletes the counters that refer to it. That
is safe only because `Counter` does not touch the catalog while being destroyed. Keep
destructors from reaching for other objects and this never bites.

## 5. Inheritance: `virtual`, `override`, `= 0`

**Open:** `src/world/Interactable.h`, with your `Interactables.java` beside it

Your Java class ended like this:

```java
    public abstract String getTypeOfItem();
    public abstract void setHasCup(boolean b);
    public abstract void setCup(Cup c);
    public abstract Cup getCup();
    public abstract boolean getHasCup();
    public abstract void createCup(ArrayList<String> ingredientsInCup);
    public abstract void addIngredient(String s);
    public abstract boolean customerSeated();
    public abstract void removeCustomer();
    public abstract Customer getCustomer();

    public abstract int getItemsLeft();
    public abstract void setItemsLeft(int i);
```

Twelve questions that any interactable could be asked, so that `Player.itemChange()` could
ask them. Every subclass had to answer all twelve, mostly with empty bodies; `Trashbin.java`
is seventy lines of them. The C++ class asks one question:

```cpp
    // Can it be used right now? Almost always yes; a chair says no while it is empty.
    virtual bool canInteract() const;

    // "= 0" makes this pure virtual: there is no version here, every subclass must
    // write one. It is also what makes Interactable an abstract class.
    virtual Outcome interact(Player &player) = 0;
```

The keywords:

| C++ | Java | Note |
|---|---|---|
| `virtual` | (every method) | In Java every method can be overridden. In C++ only those marked `virtual` can. Without it, the *variable's* type decides which function runs, not the object's. |
| `override` | `@Override` | Optional, but always write it: the compiler then checks that you really are overriding something. |
| `= 0` | `abstract` | "Pure virtual". A class with at least one cannot be instantiated. |
| (nothing) | `interface` | C++ has no interfaces. A class with only pure virtual functions does the job. |

With that, a whole subclass is short. This is all of `Trashbin`'s behaviour:

```cpp
Outcome Trashbin::interact(Player &player)
{
    if (player.heldItem() == nullptr)
        return Outcome::Nothing;

    // takeItem() returns the item in a unique_ptr. Nothing stores it, so it is deleted
    // at the end of this statement. In C++, "throwing it away" is simply not keeping it.
    player.takeItem();
    return Outcome::Discarded;
}
```

And the `instanceof` chain in `Player.itemChange()` becomes one line in
`CafeScene::interact`:

```cpp
    const Outcome outcome = m_target->interact(*m_player);
```

`m_target` is an `Interactable *`. Which `interact` runs depends on what it really points
at. That is the whole point of `virtual`. (`*m_player` turns the pointer into the object
it points to, because `interact` wants a reference.)

**Virtual destructors.** `Item.h` has `virtual ~Item() = default;`. `~Item` is the
destructor, the function that runs when an item dies. If you ever delete an object through
a pointer to its base class, as `unique_ptr<Item>` does with a `Cup`, the base class's
destructor must be virtual or only the `Item` part gets cleaned up. Rule: a class with any
virtual function gets a virtual destructor.

**Two base classes.** `Player` inherits from both `QObject` and `QGraphicsPixmapItem`.
Java forbids this; C++ allows it. It is used here for one reason: `QGraphicsPixmapItem`
makes the player drawable, `QObject` lets it have signals. When `QObject` is one of
several bases it must come first.

## 6. `enum class` and `dynamic_cast`

**Open:** `src/world/Interactable.h` and `src/world/Player.cpp`

Your Java passed directions as strings (`"up"`, `"down"`) and compared them with
`.equals`. A typo there compiles and silently does nothing. C++ has a better tool:

```cpp
// Which side of the object its "press E" pop-up appears on.
enum class PopUpSide { Up, Down, Left, Right };
```

`PopUpSide` is now a type. `PopUpSide::Upp` does not compile. A `switch` over it is
checked too; from `CafeScene.cpp`:

```cpp
QString popUpImage(PopUpSide side)
{
    switch (side) {
    case PopUpSide::Up: return "counterPopUp";
    case PopUpSide::Down: return "counterPopDown";
    case PopUpSide::Left: return "counterPopLeft";
    case PopUpSide::Right: return "counterPopRight";
    }
    return QString();
}
```

If you add a fifth side and forget it here, the compiler warns. The same idea gives
`Outcome` (what `interact` did), `ItemKind` (which replaced the `Liquid` and `Topping`
classes), `Customer::State` and `GameScreen::Panel`. The last two are declared inside
their class, so from outside they are written with the class name in front.

**`dynamic_cast` is `instanceof`.** Sometimes you do need to ask what something really is.
From `Player::takeCup`:

```cpp
    Cup *cup = dynamic_cast<Cup *>(m_held.get());
    if (cup == nullptr)
        return nullptr;
```

`dynamic_cast<Cup *>(pointer)` gives you a `Cup *` if the object really is a `Cup`, and
`nullptr` if it is not. It is Java's `if (x instanceof Cup) { Cup c = (Cup) x; ... }` in
one step. Use it sparingly: four places in this project, all asking "is the thing in the
player's paws a cup?". Most "what kind are you?" questions are better answered by a
virtual function, as chapter 5 showed.

**`static_cast`** is the other cast you will see, in `UpgradesPanel.cpp`:

```cpp
    return m_entries.at(static_cast<int>(upgrade));
```

It converts between types that are plainly related (an enum and its number) with no
run-time check. Avoid the old C style `(int)upgrade`; it will do any conversion at all,
including ones you did not mean.

## 7. `unique_ptr` and `std::move`: the cup changes hands

**Open:** `src/world/Player.cpp` and `src/world/PrepTable.cpp`

Chapter 4 said every object needs one owner. For the things the player carries, the owner
is a `std::unique_ptr`: a small object that holds a pointer and deletes what it points to
when it dies. `Player.h` declares

```cpp
    std::unique_ptr<Item> m_held;   // the player owns what it carries
```

and `PrepTable.h` declares

```cpp
    std::unique_ptr<Cup> m_cup;            // the table owns the cup while it stands here
```

A `unique_ptr` cannot be copied, because then two owners would both delete the object.
It can only be **moved**: handed over, leaving the original empty. That is what happens
when the player gets something:

```cpp
void Player::give(std::unique_ptr<Item> item)
{
    Q_ASSERT(m_held == nullptr);
    // A unique_ptr cannot be copied, only moved: after this line `item` is empty and
    // m_held owns the object. Ownership has changed hands.
    m_held = std::move(item);
    heldItemWasChanged();
}
```

Follow one cup through the game:

```text
  Counter::interact          PrepTable::interact           PrepTable::interact
  m_catalog.make("cup")      m_cup = player.takeCup()      player.give(std::move(m_cup))

  [new Cup] ──▶ Player.m_held ──────▶ PrepTable.m_cup ──────▶ Player.m_held ──▶ ...
                (owner)               (owner)                 (owner)

  At every moment exactly one unique_ptr owns the cup. When the cup is served, the
  last owner lets it go without passing it on, and the cup is deleted.
```

The table picking the cup up from the player and giving it back, from
`PrepTable::interact`:

```cpp
        // std::move hands the cup over. Afterwards m_cup is empty and the player owns it.
        player.give(std::move(m_cup));
```

```cpp
        m_cup = player.takeCup();
```

The two directions are not symmetrical. Table to player is a plain move, because a
`unique_ptr<Cup>` converts to a `unique_ptr<Item>` by itself (a cup is an item). Player
to table is not, because the player holds an `Item` that might not be a cup. So
`Player::takeCup` checks, and re-wraps:

```cpp
    // m_held is a unique_ptr<Item>; the caller wants a unique_ptr<Cup>. release() makes
    // m_held let go without deleting, and the new unique_ptr takes over the same object.
    m_held.release();
    heldItemWasChanged();
    return std::unique_ptr<Cup>(cup);
```

The vocabulary:

| Call | Meaning |
|---|---|
| `std::make_unique<Cup>()` | `new Cup()`, already wrapped in an owner |
| `ptr.get()` | Lend out the plain pointer for looking. Ownership stays. |
| `std::move(ptr)` | Hand ownership to someone else. `ptr` is `nullptr` afterwards. |
| `ptr.release()` | Give up ownership *without* deleting. You must hand the pointer to another owner at once. |
| `ptr = nullptr` or letting it go out of scope | Delete the object |

Why Qt objects do not use this is chapter 12.

## 8. Forward declarations and include cycles

**Open:** `src/world/Chair.h` and `src/world/Customer.h`

A chair knows its customer, and a customer is built from its chair. In Java two classes
that mention each other are no problem. In C++, `Chair.h` including `Customer.h` while
`Customer.h` includes `Chair.h` cannot work: `#include` pastes text, and the paste would
never end.

The way out is that a header often does not need the other class's full definition. To
declare a *pointer or reference* to a class, the compiler only needs to know that the
name is a class. That is a **forward declaration**, in `Chair.h`:

```cpp
class Customer;
```

and in `Customer.h`:

```cpp
class Chair;
```

`Chair.h` can then say `Customer *m_customer = nullptr;`. The full `#include` goes in the
`.cpp` file, where functions of the other class are actually called. `Chair.cpp`:

```cpp
#include "world/Chair.h"

#include "items/Cup.h"
#include "world/Customer.h"
#include "world/Player.h"
```

The rule:

- Only a pointer or reference to it appears → forward-declare it in the header.
- You inherit from it, hold it by value, or call its functions → include its header.

Most headers in this project forward-declare what they can. Besides breaking cycles, it
makes builds faster: a change to `Player.h` then recompiles only the files that really use
`Player`.

## 9. Habits from competitive programming

You already write C++ every day, so here is what changes when the program is a project
instead of one file.

**Drop**

- **`using namespace std;` in headers.** A header gets pasted into every file that
  includes it, so this would force that choice on all of them and invite name clashes.
  That is why the project writes `std::unique_ptr`, `std::move`, `std::sqrt` in full.
  Inside a single `.cpp` it is tolerable; the project does not use it at all.
- **`#include <bits/stdc++.h>`.** It is a GCC-only convenience and it drags in everything.
  Include what you use.
- **Globals.** In contest code, global arrays are fine. Here, state lives in objects, and
  an object is handed the things it needs through its constructor. This is the same
  change the rewrite made to your Java, where `static` lists and `Game.getPlayerMoney()`
  were the globals.
- **`#define` for constants and shortcuts.** Use `const` variables, `enum class`, and
  functions. The compiler understands those; it cannot see into a macro.
- **`int` used as a flag.** Use `bool`, or an `enum class` when there are more than two
  states.

**Keep**

- **`auto`**, where the type is obvious from the right-hand side. The project uses it for
  one pattern only:

  ```cpp
    auto *play = new ImageButton("playImg", QRect(430, 535, 150, 50), this);
  ```

- **Range-`for`**, but add `const &` so elements are not copied:
  `for (const QString &ingredient : cup.ingredients())`.
- **What you know about containers.** Qt has its own, used wherever Qt functions are
  involved:

  | STL | Qt | |
  |---|---|---|
  | `std::string` | `QString` | Unicode; has `.arg()` for formatting |
  | `std::vector<T>` | `QList<T>` | Same idea; `.append()`, `.at(i)`, `.size()` |
  | `std::vector<std::string>` | `QStringList` | A `QList<QString>` with extras such as `.sort()` |
  | `std::unordered_map<K, V>` | `QHash<K, V>` | Used for the picture cache in `Assets.cpp` |

**Three meanings of `static`** that you will meet in this code:

- Inside a function (`Assets.cpp`): `static QHash<QString, QPixmap> cache;` is created the
  first time the function runs and then persists. It is a global that only that function
  can see.
- Inside a class (`Chair.h`): `static constexpr int WrongOrderPenalty = 3;` is Java's
  `static final`. `constexpr` means "known at compile time".
- An **unnamed namespace** (`namespace { ... }` near the top of `Customer.cpp`) makes
  helper functions private to that one file.

**`Q_ASSERT(condition)`** (in `Player::give`) stops a debug build if the condition is
false. It documents and checks an assumption, such as "the player's paws are empty".

---

# Part 2 — How Qt works

## 10. The build: CMake, `moc`, resources

**Open:** `CMakeLists.txt`

Your Java project built with `javac Main.java`. A C++ project needs a description of what
to compile and link. `CMakeLists.txt` is that description, and CMake turns it into
instructions for the tool that does the work (Ninja, which runs g++).

When you run `build.bat`:

```text
  CMakeLists.txt ──cmake──▶ build/ (instructions for Ninja)
                                │
                              ninja
                                │
        ┌───────────────────────┼─────────────────────────┐
        ▼                       ▼                         ▼
   moc reads headers       rcc packs assets          g++ compiles each
   with Q_OBJECT and       into C++ source           .cpp to an object
   writes extra .cpp                                 file
        └───────────────────────┴─────────────────────────┘
                                │
                          g++ links them
                                │
                  CatCafe.exe  and  tst_*.exe
```

The Qt-specific lines:

```cmake
find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Multimedia Test)

# Turns on Qt's code generators. The important one is "moc": it reads every header that
# contains Q_OBJECT and writes the extra C++ that makes signals and slots work.
qt_standard_project_setup()
```

**`moc` and `Q_OBJECT`.** C++ has no reflection: a running program cannot ask an object
"what functions do you have?". Signals and slots need exactly that. Qt's answer is a
small program, the Meta-Object Compiler, that reads your headers *before* the real
compiler runs. For each class containing the `Q_OBJECT` macro it generates a `.cpp` file
with the bodies of that class's signals and a table describing it. That is why:

- a signal is declared in the header but has no body anywhere in `src/`: `moc` wrote it;
- a class that declares signals or slots must contain `Q_OBJECT`;
- forgetting `Q_OBJECT`, or adding it and not re-running CMake, gives a linker error
  mentioning `vtable` or `staticMetaObject`. When you see that, check `Q_OBJECT` first.

**Resources.** The pictures are not loaded from files beside the program. They are
compiled into it:

```cmake
file(GLOB ASSET_IMAGES CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR} assets/images/*.png)
qt_add_resources(catcafe_core "assets"
    PREFIX "/"
    BASE "assets"
    FILES
        ${ASSET_IMAGES}
        assets/sounds/click.wav
        assets/sounds/meowHappy.wav
        assets/sounds/meowAngry.wav
        assets/fonts/WaterlilyScript.ttf
)
```

Anything listed there is then readable through a path starting with `:/`, such as
`":/images/boba.png"`, from anywhere in the program (see `src/Assets.cpp`). The game needs
no particular working directory, which is why your Java only ran from its own folder and
this does not care. The 13 MB background music is left out on purpose and copied beside
the `.exe` instead; compiling 13 MB into a program makes every build slow.

**Targets.** The file defines three kinds of thing to build:

- `catcafe_core`: every file in `src/` except `main.cpp`, as a library.
- `CatCafe`: `main.cpp` plus that library. The game.
- `tst_*`: one test program per file in `tests/`, each also linked to the library. The
  library exists so that the tests can use the game's classes.

The game itself is three lines:

```cmake
qt_add_executable(CatCafe src/main.cpp)
set_target_properties(CatCafe PROPERTIES WIN32_EXECUTABLE ON)   # no console window
target_link_libraries(CatCafe PRIVATE catcafe_core)
```

**When you add a class**, add its `.h` and `.cpp` to the list under `qt_add_library`.
Qt Creator does this for you if you use File → New File → C++ Class.

**The kit.** A Qt "kit" is a Qt build plus the compiler it was built with. They must
match. This project uses Qt's MinGW kit with the g++ in `C:\Qt\Tools`. You also have a
g++ under `C:\msys64`; it is a different build of the compiler and must not be mixed in.

## 11. `QApplication` and the event loop

**Open:** `src/main.cpp`

```cpp
int main(int argc, char *argv[])
{
    // Exactly one QApplication must exist before any widget, pixmap or font is created.
    QApplication app(argc, argv);

    // Every widget that does not choose its own font uses this one.
    QApplication::setFont(Assets::font(14));

    MainWindow window;
    window.show();

    // exec() runs the event loop: it waits for clicks, key presses and timer ticks and
    // hands each one to the object it belongs to. It only returns when the window closes.
    return app.exec();
}
```

In Swing, `main` builds the frame and returns, and a hidden thread (the event dispatch
thread) keeps the program alive. In Qt the loop is not hidden: `app.exec()` *is* the
loop, and it runs on the main thread.

```text
        ┌──────────────────────────────────────────────┐
        │                 app.exec()                   │
        │                                              │
        │   wait for the next event                    │
        │        │                                     │
        │        ▼                                     │
        │   mouse click? key? timer due? repaint?      │
        │        │                                     │
        │        ▼                                     │
        │   call the right function on the right       │
        │   object (keyPressEvent, paintEvent, a       │
        │   timer's slot ...)                          │
        │        │                                     │
        │        └──────── back to the top ────────────┘
```

Consequences worth knowing:

- Nothing appears and nothing reacts until `exec()` is running. `window.show()` only
  marks the window as wanting to be shown.
- Everything in this game runs on that one thread, one event at a time. Two timer slots
  never run at once, so there are no locks anywhere.
- If one of your functions takes a long time, the whole window freezes, because the loop
  cannot get back to the top. In the visualizer, an algorithm must not run to completion
  inside one event; chapter 26 comes back to this.

`MainWindow window;` is a value on the stack (chapter 3). When `main` returns it is
destroyed, and it takes every screen, button, scene and timer with it. The next chapter
explains how.

## 12. The object tree

**Open:** `src/MainWindow.cpp` and `src/screens/MenuScreen.cpp`

This is Qt's answer to "who deletes this?". Any `QObject` can be given a **parent**. A
parent deletes its children when it dies. Since widgets are QObjects, building a window
builds a tree, and deleting the root deletes everything.

```cpp
    auto *play = new ImageButton("playImg", QRect(430, 535, 150, 50), this);
```

That `this` at the end is the parent. From this line on, the menu screen owns the button.
There is no `delete` for it anywhere, and none is needed.

The tree of this program, roughly:

```text
  MainWindow
  ├── SoundManager
  │   └── 3 x QSoundEffect, QMediaPlayer, QAudioOutput
  └── QStackedWidget
      ├── MenuScreen
      │   └── 2 x ImageButton
      ├── InstructionsScreen
      │   ├── ImageButton (back)
      │   └── QTimer
      └── GameScreen
          ├── CafeScene ── owns every item in the cafe (a second, separate tree)
          │   └── 4 x QTimer
          ├── QGraphicsView
          ├── HeldItemPanel, QLabel x 3, ImageButton x 6, QPropertyAnimation
          ├── RestockingPanel ── QComboBox, 5 x ImageButton
          ├── OrderPanel ── QScrollArea ── content widget ── OrderCard, OrderCard ...
          └── UpgradesPanel ── 4 x ImageButton
```

For widgets, the parent also decides where a child is drawn: inside the parent, moving
with it, hidden when it is hidden. Some functions set the parent for you:
`m_screens->addWidget(m_menu)` makes the stack the menu's parent, and
`scroll->setWidget(m_content)` does the same for the scroll area.

**This is why Qt objects are raw pointers and not `unique_ptr`.** Something already owns
them. Wrapping a parented object in a `unique_ptr` would give it two owners and delete it
twice.

**Searching the tree.** Because the tree exists, you can ask it questions. The hover click
on every button in the game is wired by three lines in `MainWindow.cpp`:

```cpp
    const QList<ImageButton *> buttons = findChildren<ImageButton *>();
    for (ImageButton *button : buttons)
        connect(button, &ImageButton::hovered, m_sounds, &SoundManager::playClick);
```

`findChildren<ImageButton *>()` walks everything below the window and returns every
`ImageButton`, however deeply nested. In the Java, each panel had to remember to call
`addMouseListener` on each of its buttons, and the music toggle was missed. The tests use
the single-result version with a name, `findChild<ImageButton *>("playButton")`; the
names come from `setObjectName`.

**The scene is a second tree.** Items in a `QGraphicsScene` have their own parent–child
relationship, with the same ownership rule: the scene owns top-level items, an item owns
its child items. Scene items are not QObjects (they are lighter), with one exception here.
`Player` is both, so it could have two owners. It is given exactly one: the scene. That is
why its constructor takes no parent.

## 13. Signals and slots

**Open:** `src/screens/MenuScreen.cpp`, `src/MainWindow.cpp`, `src/world/Player.cpp`

In Swing, an object that wants to hear about a click implements `ActionListener`, and one
`actionPerformed` receives every click and works out which button it was:

```java
    public void actionPerformed(ActionEvent e) {
        if (e.getSource() == play){
            if (Sounds.getSoundsOn() && Sounds.getMusicOn()) {
                Sounds.startBackgroundMusic();
            }
            Main.cardL.show(Main.c, "Game");
            Main.c.getComponent(1).requestFocusInWindow();
            Game.startTimers();
        }
```

Notice how much the menu has to know: about `Sounds`, about `Main`'s layout, about `Game`'s
timers. Qt replaces listeners with **signals and slots**.

- A **signal** is something an object announces: "I was clicked", "my money changed".
- A **slot** is an ordinary member function that should run in response.
- **`connect`** joins one signal of one object to one slot of another.

The sender does not know who is listening, or whether anyone is.

**Declaring and emitting.** `MenuScreen.h` declares two signals. They have no bodies;
`moc` generates them (chapter 10).

```cpp
signals:
    void playRequested();
    void instructionsRequested();
```

`MenuScreen.cpp` makes the buttons' clicks into those announcements. A signal can be
connected straight to another signal:

```cpp
    connect(play, &ImageButton::clicked, this, &MenuScreen::playRequested);
    connect(instructions, &ImageButton::clicked, this, &MenuScreen::instructionsRequested);
```

Read `connect(a, &A::signal, b, &B::slot)` as "when `a` emits `signal`, call `b->slot()`".

That is everything the menu does. It has never heard of the game screen. `MainWindow`
decides what "play requested" means:

```cpp
    connect(m_menu, &MenuScreen::playRequested, this, &MainWindow::showGame);
    connect(m_menu, &MenuScreen::instructionsRequested, this, &MainWindow::showInstructions);
    connect(m_game, &GameScreen::backRequested, this, &MainWindow::showMenu);
    connect(m_instructions, &InstructionsScreen::backRequested, this, &MainWindow::showMenu);
```

**Signals with data.** A signal can carry arguments. When the player's money changes:

```cpp
void Player::addMoney(int delta)
{
    m_money += delta;
    emit moneyChanged(m_money);   // anyone connected to this signal is told right now
}
```

`emit` is only a marker for the reader; the line is a function call. Every slot connected
to `moneyChanged` runs immediately, in the order it was connected, before `addMoney`
returns. Three things listen: the money label in `GameScreen`, the Restock panel and the
Upgrades panel. In the Java each of those called `Game.getPlayerMoney()` every time it
repainted. Here nobody polls; they are told.

A slot may take fewer arguments than the signal provides. `RestockingPanel::refresh()`
takes none, and is connected to three different signals:

```cpp
    connect(m_combo, &QComboBox::currentIndexChanged, this, &RestockingPanel::refresh);
    connect(m_scene, &CafeScene::stockChanged, this, &RestockingPanel::refresh);
    connect(m_scene->player(), &Player::moneyChanged, this, &RestockingPanel::refresh);
```

**Lambdas: when the slot needs an argument the signal does not have.** `clicked()` carries
nothing, but `changeQuantity` needs to know *how much*. The gap is filled by a lambda, a
small unnamed function written in place:

```cpp
    connect(plus1, &ImageButton::clicked, this, [this] { changeQuantity(+1); });
```

`[this]` is the capture list: what the lambda is allowed to use from its surroundings.
`{ ... }` is its body. The `this` just before the lambda is the "context": if this panel
is destroyed, the connection is removed with it. In `UpgradesPanel.cpp` the lambda also
captures a copy of a variable, so each button remembers which upgrade it is for:

```cpp
    connect(button, &ImageButton::clicked, this, [this, upgrade] { buy(upgrade); });
```

Use a lambda for this one job (adapting a signal to a slot that needs more). When the
signal and slot already match, connect them directly.

**Following one event through the program.** The player presses E beside a seated customer
while holding the wrong cup:

```text
  keyboard
     │
     ▼
  GameScreen::keyPressEvent            (chapter 14)
     │  calls
     ▼
  CafeScene::interact
     │  calls (virtual)
     ▼
  Chair::interact ── Player::addMoney(-3) ── emit moneyChanged(997)
     │                                          ├──▶ GameScreen::showMoney
     │  returns Outcome::ServedWrong            ├──▶ RestockingPanel::refresh
     ▼                                          └──▶ UpgradesPanel::refreshButtons
  CafeScene::interact
     ├── emit orderServed(false, -3) ──▶ GameScreen::showMoneyChange
     │                                      ├── shows "-$3" and starts it drifting
     │                                      └── emit orderServed(false) ──▶ SoundManager::playMeow
     └── emit ordersChanged() ─────────▶ OrderPanel::rebuild
```

`Chair` knows about `Player` and nothing else. The scene knows nothing about panels or
sound. `SoundManager` knows nothing about the game. The only place that knows how they fit
together is the wiring in `MainWindow.cpp` and the constructors of `GameScreen` and the
panels.

**In the visualizer:** your algorithm object emits `stepDone(...)`; the canvas, a log
panel and a step counter each connect to it. Add a fourth listener later and the algorithm
does not change.

## 14. Widgets, events and painting

**Open:** `src/ui/ImageButton.cpp`, `src/screens/InstructionsScreen.cpp`,
`src/screens/GameScreen.cpp`

Swing has you attach listener objects to a component from outside. Qt has a widget handle
its own events by overriding virtual functions. Signals are for telling *other* objects;
event handlers are for the widget itself.

| Swing | Qt |
|---|---|
| `paintComponent(Graphics g)` | `paintEvent(QPaintEvent *)`, drawing with a `QPainter` |
| `repaint()` | `update()` |
| `MouseListener.mouseEntered / mouseExited` | `enterEvent` / `leaveEvent` |
| `KeyListener.keyPressed / keyReleased` | `keyPressEvent` / `keyReleaseEvent` |
| `setBounds(x, y, w, h)` | `setGeometry(x, y, w, h)` |
| `requestFocusInWindow()` | `setFocus()`, plus a focus policy |

`ImageButton` is the smallest complete example. It draws itself and reacts to the mouse:

```cpp
void ImageButton::paintEvent(QPaintEvent *)
{
    // A QPainter drawing on `this` is Qt's version of the Graphics object Swing passes
    // to paintComponent. Anything not drawn stays see-through.
    QPainter painter(this);
    painter.drawPixmap(0, 0, Assets::pixmap(currentImage(), size()));
}

void ImageButton::enterEvent(QEnterEvent *)
{
    if (!isEnabled())
        return;   // a button that cannot be used stays silent and does not react

    m_mouseOver = true;
    resize(m_hoverSize);
    update();
    emit hovered();
}
```

That one class replaces every `mouseEntered` / `mouseExited` pair in your Java, the
`setContentAreaFilled(false)` calls and `MyPanel.makeButton`. It inherits clicking from
`QAbstractButton`, so `clicked()` comes for free.

**Painting a whole screen by hand.** `InstructionsScreen` is your `Instructions.java`
almost unchanged in shape: a timer changes some numbers and asks for a repaint.

```cpp
    m_chairOccupied = false;
    m_customerY += 2;
    m_customerFrame = m_customerWalk.next();
    m_catFrame = m_cat.next();

    if (m_customerY > 300) {   // reached the chair
        m_customerY = 200;
        m_chairOccupied = true;
        m_pauseTicks = 33;
    }

    update();   // Swing's repaint()
```

`update()` does not paint. It tells Qt the widget is out of date, and Qt calls
`paintEvent` soon after, once, however many times `update()` was called.

Two details that matter when moving drawing code from Swing:

- `painter.drawText(x, y, text)` and Swing's `drawString(text, x, y)` both take the line
  the letters stand on, so text coordinates carry over unchanged.
- Font sizes are set in pixels here (`setPixelSize` in `Assets::font`), because that is
  what Java's sizes were in practice.

**Colours of ordinary widgets** come from a *palette*, a small table of colours by role.
A label's text colour is the role `QPalette::WindowText`; a widget's background is
`QPalette::Window`. `GameScreen.cpp` makes the money label green by changing one entry
and calling `setPalette`, and `OrderPanel.cpp` makes its background pink the same way.
Inside a `paintEvent` you do not need any of that: you set the painter's pen, as
`HeldItemPanel` does with `painter.setPen(QColor("#5097B9"))`.

**The keyboard and focus.** Key presses go to the one widget that has *focus*. In the
Java, clicking any button moved focus to that button and the cat stopped responding, which
is why `addNotify()` was called after every click. Qt lets each widget say whether it
wants focus at all. `ImageButton`, the combo box and the view are all set to
`Qt::NoFocus`; `GameScreen` is `Qt::StrongFocus`. So focus never leaves the game screen
and nothing has to win it back.

```cpp
void GameScreen::keyPressEvent(QKeyEvent *event)
{
    // Holding a key down makes the keyboard send the press again and again. Those
    // repeats are not new presses.
    if (event->isAutoRepeat())
        return;

    const int key = event->key();
    const bool isGameKey = key == Qt::Key_W || key == Qt::Key_A || key == Qt::Key_S || key == Qt::Key_D
                           || key == Qt::Key_E;
    if (!isGameKey) {
        QWidget::keyPressEvent(event);   // not ours: let the normal handling have it
        return;
    }

    // As in the original, the cat stands still while the Upgrades shop is open.
    if (m_openPanel == Panel::Upgrades)
        return;

    if (key == Qt::Key_E)
        m_scene->interact();
    else
        m_scene->player()->setKeyHeld(key, true);
}
```

When you override an event handler and do not want an event, pass it to the base class
(`QWidget::keyPressEvent(event)`), as you would call `super` in Java.

**What is drawn on top of what.** A child widget is drawn over its parent. Among siblings,
one created later is drawn over one created earlier. `GameScreen`'s constructor relies on
this: the view first, then the controls, then the three panels, so the panels cover
whatever is beneath them.

## 15. Timers

**Open:** `src/world/CafeScene.cpp`, the constructor

Your `Game` had four Swing timers all pointing at one `actionPerformed`, which then asked
`if (e.getSource() == timer)`, `== customerTimer`, and so on. In Qt a timer is an object
with a `timeout` signal, and each timer is connected to the one function it exists for:

```cpp
    m_frameTimer->setInterval(16);
    m_customerTimer->setInterval(10);
    m_spawnTimer->setInterval(1000);
```

```cpp
    connect(m_frameTimer, &QTimer::timeout, this, &CafeScene::frameTick);
    connect(m_customerTimer, &QTimer::timeout, this, &CafeScene::customerTick);
    connect(m_spawnTimer, &QTimer::timeout, this, &CafeScene::spawnTick);
```

The game loop is then just this, sixty-odd times a second:

```cpp
void CafeScene::frameTick()
{
    m_player->step(m_solidRects);
    updateTarget();
}
```

There is no `repaint()` in it. Moving an item is enough; the scene redraws what changed.

**Single-shot timers** fire once. The "out of stock" message hides itself a second after
it appears:

```cpp
    m_messageTimer->setInterval(1000);
    m_messageTimer->setSingleShot(true);
    connect(m_messageTimer, &QTimer::timeout, this, &CafeScene::hideOutOfStockMessage);
```

Things to know:

- Timers are delivered by the event loop (chapter 11). They are not threads. A slot that
  runs long delays every other timer.
- `start()` on a running timer restarts it; it never creates a second one.
- These timers are set to `Qt::PreciseTimer`. The default type may drift a few percent to
  save power, which is fine for "check for mail every minute" and wrong for a game.
- The timers only *call* the tick functions. The tests call `frameTick()` and
  `customerTick()` directly, as many times as they like, so no test waits for real time.

**In the visualizer:** one timer whose slot performs one step of the algorithm. Play is
`start()`, pause is `stop()`, speed is `setInterval()`, and a Step button calls the same
slot once.

## 16. Graphics View: scene, view, items

**Open:** `src/world/CafeScene.cpp`, `src/world/NonCollidable.cpp`,
`src/screens/GameScreen.cpp`

This is the part of Qt your visualizer will be built on.

Your `Game.paintComponent` redrew the entire café every frame: background, then
`NonCollidables.paint`, `Customer.paint`, `Interactables.paint`, each looping over a
static list. That is *immediate* drawing: nothing exists between frames except your own
lists.

Graphics View is the opposite. You create objects once and Qt remembers them.

| Piece | What it is | Here |
|---|---|---|
| `QGraphicsScene` | A container of items, in its own coordinate space. Not visible by itself. | `CafeScene` |
| `QGraphicsItem` | One thing in the scene: it has a position, a z-value, maybe a picture, maybe children. | `NonCollidable` and its subclasses, `Customer`, `Player`, the pop-up |
| `QGraphicsView` | A widget that shows a scene. | Created in `GameScreen` |

**Building the world** is a list of "make this, put it in the scene":

```cpp
    addSolid(new Decoration("table", QRect(700, 50, 100, 100)));
    addChair("chairLeft", 625, 62, false, {door, {620, 180}, {620, 100}});
    addChair("chairRight", 800, 62, true, {door, {850, 180}, {850, 100}});
```

```cpp
void CafeScene::addSolid(NonCollidable *item)
{
    addItem(item);   // the scene takes ownership
    m_solidRects.append(item->solidRect());
}
```

After `addItem`, the scene draws the item, and deletes it at the end. The scene's own
lists (`m_interactables`, `m_chairs`, ...) are only ways of finding items again; they
replace the static lists in your Java classes.

**Z-values** decide what is in front, regardless of the order things were added:

| z | What |
|---|---|
| 0 | Floor |
| 1 | Player |
| 2 | Counters, tables, chairs, walls |
| 3 | Customers |
| 4 | The "E" pop-up |
| 6 | The "out of stock" message |

This reproduces the order of the calls in your `paintComponent`. An item sets its own in
its constructor, for example `setZValue(2)` in `NonCollidable`.

**Child items** are positioned relative to their parent and move with it. The small picture
of what the cat is holding is a child of the player:

```cpp
    m_heldIcon->setPos(40, 20);
    m_heldIcon->hide();
```

(40, 20) is "just right of the cat, halfway down", wherever the cat is. Your Java computed
`x + WIDTH, y + HEIGHT/2` on every frame. The cup standing on a prep table and the number
bubble over a seated customer are child items too.

**Deciding what is in reach** uses plain rectangles, as your Java did:

```cpp
    for (Interactable *candidate : m_interactables) {
        if (candidate->canInteract() && candidate->reachRect().intersects(reach)) {
            m_target = candidate;
            break;
        }
    }
```

`reachRect()` is the object's rectangle made a little bigger, which `QRect` does in one
call: `rect().adjusted(-m_reachX, -m_reachY, m_reachX, m_reachY)` moves the left and top
edges out and the right and bottom edges out by the same amounts.

Qt can also answer such questions itself (`collidingItems()`, `items(QRectF)`), using each
item's real shape. This game keeps your rectangles because they are tuned by hand: the
chair's solid area is smaller than its picture, and the reach areas are larger. A graph
editor would use Qt's own hit-testing.

**The view** is configured to be a plain window onto the scene:

```cpp
    m_view->setGeometry(0, 0, 1000, 665);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setFocusPolicy(Qt::NoFocus);
    m_view->setInteractive(false);
    m_view->setRenderHint(QPainter::SmoothPixmapTransform);
```

`setInteractive(false)` means mouse clicks on the scene are ignored, because this game is
keyboard-only. In a graph editor that line goes, and the items start receiving mouse
events of their own (chapter 26).

**World versus controls.** Everything *in* the café is a scene item. Everything *about*
the café (panels, buttons, the money readout) is a widget placed over the view. A
visualizer has the same two halves: the graph on a canvas, the controls around it.

## 17. Layouts and scroll areas

**Open:** `src/panels/OrderPanel.cpp`

Almost everything in this game sits at a fixed pixel position, because the art was drawn
for exact places. That is unusual. A normal Qt program never calls `setGeometry` on its
widgets; it puts them in **layouts**, which position and resize them, and which keep
working when the window is resized or the font is larger.

The Orders panel is the one place this project does it the normal way. Cards are handed to
a vertical layout:

```cpp
    for (int i = 0; i < seated.size(); ++i) {
        const Customer *customer = seated.at(i);
        auto *card = new OrderCard(i + 1, customer->order().ingredients(), customer->orderCost(),
                                   customer->payout(tips), m_content);
        m_cardLayout->addWidget(card, 0, Qt::AlignHCenter);
        m_cards.append(card);
    }
```

No coordinates. The layout stacks them; add a third and it goes under the second. The
layout was installed on the content widget in the constructor's initializer list, with
`new QVBoxLayout(m_content)`.

When there are more cards than fit, the scroll area takes over:

```cpp
    auto *scroll = new QScrollArea(this);
    scroll->setGeometry(0, 0, 375, 150);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setFocusPolicy(Qt::NoFocus);
    scroll->setWidgetResizable(true);           // let the content be as wide as the area
    scroll->verticalScrollBar()->setSingleStep(8);
    scroll->setWidget(m_content);               // the scroll area now owns m_content
```

This is Swing's `new JScrollPane(ordersDisplay)` with a `GridLayout` inside.

The layouts you will use most:

| Layout | Arranges widgets |
|---|---|
| `QVBoxLayout` / `QHBoxLayout` | In a column / in a row |
| `QGridLayout` | In rows and columns |
| `QFormLayout` | As label–field pairs |
| `QSplitter` (a widget, not a layout) | Side by side with a draggable divider |

**In the visualizer**, use layouts for everything except the canvas: a `QHBoxLayout` with
the view on the left and a column of controls on the right, or `QMainWindow`'s dock
widgets and toolbars.

## 18. Animation

**Open:** `src/screens/GameScreen.cpp`, the constructor

When an order is served, "+$7" appears above the money and drifts down for two seconds.
Your Java did the arithmetic inside `paint`:

```java
            g.drawString((plus ? "+" : "-") + ("$" + Math.abs(moneyGained)), 780, 620 + (int)(System.currentTimeMillis() - currentTime)/150);
```

Qt has an animation framework for "change this value smoothly over this long":

```cpp
    m_moneyDeltaDrift = new QPropertyAnimation(m_moneyDeltaLabel, "pos", this);
    m_moneyDeltaDrift->setDuration(2000);
    m_moneyDeltaDrift->setStartValue(m_moneyDeltaLabel->pos());
    m_moneyDeltaDrift->setEndValue(m_moneyDeltaLabel->pos() + QPoint(0, 13));
    connect(m_moneyDeltaDrift, &QPropertyAnimation::finished, m_moneyDeltaLabel, &QLabel::hide);
```

A `QPropertyAnimation` takes an object, the name of one of its *properties* (`"pos"`, the
widget's position), a start value, an end value and a duration. While it runs, Qt sets
the property a little further along each frame. The last line uses its `finished` signal
to hide the label. Starting it is one call, in `GameScreen::showMoneyChange`.

Properties are another thing `moc` provides: named values on a QObject that can be read
and written by name. Widgets have many (`pos`, `size`, `windowOpacity`, ...).

The customers and the cat are *not* animated this way. They move a pixel per tick under
the game's own rules, because their movement is the game logic, not decoration.

**In the visualizer**, this is how a node changes colour when visited or an edge lights
up when relaxed. For that the item must be a QObject with a property; `QGraphicsObject`
is the ready-made base class. Animations can be chained with
`QSequentialAnimationGroup` and given acceleration with easing curves.

## 19. Sound

**Open:** `src/audio/SoundManager.cpp`

Qt Multimedia has two tools, and the game uses both:

| Class | For | Here |
|---|---|---|
| `QSoundEffect` | Short sounds that must start instantly | The click and the two meows |
| `QMediaPlayer` + `QAudioOutput` | Long audio: it decodes as it plays | The background music |

The player decodes; the audio output is the speaker it plays through and where the volume
is set. The music loops forever at about a third of full volume, as in your Java.

Your `Sounds.java` was a class of statics that anything could call. `SoundManager` is an
ordinary object, and **nothing in the game calls it**. `MainWindow` connects signals to
its slots (chapter 13): a hovered button to `playClick`, a served order to `playMeow`,
the two toggle buttons to `toggleSound` and `toggleMusic`. The café does not know sound
exists.

The two switches:

```cpp
void SoundManager::toggleSound()
{
    m_soundOn = !m_soundOn;
    m_musicOn = m_soundOn;   // the music follows the master switch, in both directions
    setMusicPlaying(m_musicOn);
    emit stateChanged(m_soundOn, m_musicOn);
}
```

and the rule that the master switch really is a master (it was not, in the Java: fix F3):

```cpp
void SoundManager::playEffect(QSoundEffect *effect, const QString &name)
{
    if (!m_soundOn)
        return;   // the master switch silences effects too

    effect->play();
    emit effectPlayed(name);
}
```

`stateChanged` goes back to `GameScreen::showAudioState`, which changes the two button
pictures. The buttons never decide what they show; they ask, and are told.

Details: the short sounds use `qrc:/sounds/click.wav`, the URL form of a `:/` resource
path. The music file is opened the first time it is needed, not at start-up. Leaving the
game pauses the music instead of stopping it, so it resumes where it was, as Java's
`Clip.stop()` did. If the music file is missing, the game prints one warning and runs
without music.

## 20. Testing

**Open:** `tests/tst_items.cpp` and `tests/tst_player.cpp`

Run `test.bat`. It builds twelve small programs, one per file in `tests/`, and runs them.
Between them, a little over a hundred tests cover the rules of the game.

A test file is a QObject whose private slots are the tests:

```cpp
    void cupsMatchInAnyOrder()
    {
        QVERIFY(Cup({"milk", "boba", "ice"}).matches(Cup({"ice", "milk", "boba"})));
        QVERIFY(!Cup({"milk", "boba"}).matches(Cup({"milk", "boba", "ice"})));
        QVERIFY(!Cup({"milk", "milk", "boba"}).matches(Cup({"milk", "boba", "boba"})));
        QVERIFY(Cup().matches(Cup()));
    }
```

`QVERIFY(x)` fails if `x` is false. `QCOMPARE(actual, expected)` fails if they differ and
prints both. The file ends with two lines that turn it into a program:

```cpp
QTEST_MAIN(TestItems)
#include "tst_items.moc"
```

**Testing signals.** A `QSignalSpy` records every time a signal is emitted, with its
arguments:

```cpp
        Player p;
        QSignalSpy money(&p, &Player::moneyChanged);
        QSignalSpy tips(&p, &Player::tipsChanged);

        p.addMoney(-3);
        QCOMPARE(p.money(), 997);
        QCOMPARE(money.last().at(0).toInt(), 997);
```

**Testing input without a person.** An event is an object, so a test can build one and
deliver it. From `tests/tst_gamescreen.cpp`:

```cpp
    static void sendKey(QWidget *widget, QEvent::Type type, Qt::Key key, bool autoRepeat = false)
    {
        QKeyEvent event(type, key, Qt::NoModifier, QString(), autoRepeat);
        QApplication::sendEvent(widget, &event);
    }
```

No window is ever shown. The same file has a test called `screenshots` that draws the
game screen into picture files under `build\screenshots\` (`widget.grab().save(...)`),
which is how the layout was checked while this was being written.

**Code that is easy to test is shaped differently.** Two examples from this project:

The spawn rule does not roll its own dice. It is given the random number:

```cpp
bool SpawnOdds::roll(double random01)
{
    // A whole number from 0 up to chance/divisor; only 0 wins.
    const int rolled = int(random01 * (m_chance / m_divisor));
    m_chance = int(m_chance / 1.08);
    return rolled == 0;
}
```

So a test can pass `0.5` and `0.05` and know exactly what should happen.

And nothing waits for a clock. The timers only call `frameTick()` and `customerTick()`,
and a customer only moves when its `step()` is called. A test calls `step()` 212 times in
a loop and checks that the customer has sat down, in well under a millisecond.

Each of the fixes in chapter 24 that can be tested has a test with the fix's number in a
comment. Search `tests/` for `fix F`.

**In the visualizer**, the algorithms are the part most worth testing, and the easiest:
keep them in plain classes that do not know about the screen (as `items/` does here), and
test them exactly as you would check a contest solution, but automatically.

---

# Part 3 — Reference

## 21. Swing → Qt

| Swing | Qt | In this project |
|---|---|---|
| `JFrame` | `QMainWindow` | `MainWindow` |
| `CardLayout`, `cardL.show(c, "Game")` | `QStackedWidget`, `setCurrentWidget(...)` | `MainWindow` |
| `JPanel` | `QWidget` | every screen and panel |
| `JLayeredPane` | child widgets (later ones on top); z-values in a scene | `GameScreen`, `CafeScene` |
| `JButton` with an `ImageIcon` | a `QAbstractButton` subclass | `ImageButton` |
| `JLabel` | `QLabel` | banner, money readout |
| `JComboBox` + `ItemListener` | `QComboBox` + `currentIndexChanged` | `RestockingPanel` |
| `JScrollPane` | `QScrollArea` | `OrderPanel` |
| `GridLayout`, `BorderLayout` | `QVBoxLayout`, `QGridLayout`, ... | `OrderPanel` |
| `setLayout(null)` + `setBounds` | no layout + `setGeometry` / `move` | most widgets here |
| `ActionListener.actionPerformed` | a signal, `connect`, a slot | everywhere |
| `e.getSource() == button` | not needed: one connection per button | |
| `MouseListener.mouseEntered` | override `enterEvent` | `ImageButton` |
| `KeyListener.keyPressed` | override `keyPressEvent` | `GameScreen` |
| `requestFocusInWindow()`, `addNotify()` | `setFocus()`, `setFocusPolicy(...)` | `GameScreen`, `ImageButton` |
| `paintComponent(Graphics g)` | `paintEvent` + `QPainter` | `InstructionsScreen`, panels |
| `repaint()` | `update()` | |
| `g.drawImage(img, x, y, w, h, null)` | `painter.drawPixmap(x, y, pixmap)` | |
| `g.drawString(text, x, y)` | `painter.drawText(x, y, text)` | |
| `javax.swing.Timer` | `QTimer` | `CafeScene`, `InstructionsScreen` |
| `ImageIcon`, `BufferedImage` | `QPixmap` | `Assets` |
| `getScaledInstance(w, h, ...)` | `pixmap.scaled(...)` | `Assets::pixmap(name, size)` |
| `getSubimage(x, y, w, h)` | `pixmap.copy(x, y, w, h)` | `SpriteAnimation` |
| `Rectangle`, `.intersects()` | `QRect`, `.intersects()` | `Player`, `Interactable` |
| `Color.decode("#5097B9")` | `QColor("#5097B9")` | `HeldItemPanel` |
| `Font.createFont` + `registerFont` | `QFontDatabase::addApplicationFont` | `Assets::font` |
| `UIManager.put(key, font)` for every key | `QApplication::setFont(font)` | `main.cpp` |
| `new File("boba.png")` | `":/images/boba.png"` (a resource) | `Assets` |
| `Clip` | `QSoundEffect`, `QMediaPlayer` | `SoundManager` |
| `Math.random()` | `QRandomGenerator` | `CafeScene`, `ItemCatalog` |
| `System.out.println` | `qDebug() << ...`, `qWarning() << ...` | `Assets`, `SoundManager` |
| the event dispatch thread | the event loop, `app.exec()` | `main.cpp` |

## 22. Java → C++

| Java | C++ | Chapter |
|---|---|---|
| one `.java` file | a `.h` and a `.cpp` | 1 |
| `import` | `#include`, or a forward declaration | 1, 8 |
| `class A extends B` | `class A : public B` | 1 |
| `super(args)` | `B(args)` in the initializer list | 2 |
| `super.method()` | `B::method()` | 14 |
| `this.x = x` in a constructor | `: m_x(x)` | 2 |
| every method overridable | only `virtual` ones | 5 |
| `@Override` | `override` | 5 |
| `abstract` method | `virtual ... = 0` | 5 |
| `abstract class` with no abstract method | protected constructor | 2 |
| `interface` | a class of pure virtual functions | 5 |
| `x instanceof Cup` then `(Cup) x` | `dynamic_cast<Cup *>(x)` | 6 |
| `(int) x` | `static_cast<int>(x)` | 6 |
| `null` | `nullptr` | 3 |
| `Foo f = new Foo()` | `Foo f;` or `new Foo(parent)` or `std::make_unique<Foo>()` | 3, 4, 7 |
| garbage collection | scope, a Qt parent, or a `unique_ptr` | 4 |
| `a = b` shares one object | `a = b` copies (for values) | 3 |
| `final` variable | `const` | 2 |
| (nothing) | `const` after a member function | 2 |
| `static final int N = 3` | `static constexpr int N = 3` | 9 |
| a class of static helpers | a `namespace` of free functions | 9 |
| static fields holding shared state | members of one owning object | 4 |
| `String` | `QString` | 9 |
| `ArrayList<T>` | `QList<T>` | 9 |
| `ArrayList<String>` | `QStringList` | 9 |
| `HashMap<K, V>` | `QHash<K, V>` | 9 |
| `for (T x : list)` | `for (const T &x : list)` | 9 |
| `boolean` | `bool` | |
| `enum` | `enum class` | 6 |
| strings used as modes (`"up"`) | `enum class` | 6 |
| `equals()` | a named function (`Cup::matches`) or `operator==` | 1 |
| `getMoney()` | `money()` (Qt drops the "get") | |
| `"Money: $" + money` | `QString("Money: $%1").arg(money)` | |

## 23. Where each Java file went

| Java file | Now | Notes |
|---|---|---|
| `Animations.java` | `world/SpriteAnimation` | Copyable; each copy keeps its own frame |
| `Chair.java` | `world/Chair` | Spawn odds moved to `world/SpawnOdds`; walking moved to `Customer`; the static lists moved to `CafeScene` |
| `Counter.java` | `world/Counter` | |
| `Cup.java` | `items/Cup` | `randomOrder` moved to `ItemCatalog` |
| `Customer.java` | `world/Customer` | Now carries its own route and moves itself |
| `CustomerImages.java` | `CustomerSprites` in `world/Customer.h` | An ordinary struct instead of statics |
| `Decorations.java` | `world/Decoration.h` | |
| `Game.java` | `screens/GameScreen` + `world/CafeScene` | Controls and keyboard in one, the world and its clocks in the other |
| `Ingredient.java` | `items/Ingredient` | |
| `Instructions.java` | `screens/InstructionsScreen` | |
| `Interactables.java` | `world/Interactable` | Twelve abstract methods became one |
| `Item.java` | `items/Item` + `items/ItemCatalog` | The class, and the list of all items |
| `Liquid.java` | `ItemKind::Liquid` in `items/ItemCatalog.h` | A value, not a class |
| `Main.java` | `main.cpp` + `MainWindow` + `Assets` | |
| `Menu.java` | `screens/MenuScreen` | |
| `MyPanel.java` | gone | `ImageButton` does what `makeButton` and the hover listener did |
| `NonCollidables.java` | `world/NonCollidable` | |
| `OrderDisplayPanel.java` | `panels/OrderCard` | Copies its values instead of pointing at a customer |
| `OrderPanel.java` | `panels/OrderPanel` | |
| `Player.java` | `world/Player` + `panels/HeldItemPanel` | `itemChange()` was split among the four `interact()` functions; the money text is a label in `GameScreen` |
| `PopUp.java` | gone | The pop-up and the message are plain picture items in `CafeScene` |
| `PrepTable.java` | `world/PrepTable` | |
| `RestockingPanel.java` | `panels/RestockingPanel` | |
| `Sounds.java` | `audio/SoundManager` | |
| `Topping.java` | `ItemKind::Topping` in `items/ItemCatalog.h` | |
| `Trashbin.java` | `world/Trashbin` | Seventy lines became twenty |
| `UpgradesPanel.java` | `panels/UpgradesPanel` | Four copies of the logic became one, over four rows of data |

New files with no Java counterpart: `ui/ImageButton`, `world/SpawnOdds`, `Assets`, and
everything in `tests/`.

## 24. What was fixed, and what was kept

The game is meant to play exactly as yours does. These are the places where it does not,
on purpose, because the Java looked unintended. Each one that can be tested has a test
marked `fix F…`.

**F1 — The frozen customer.** `Chair.java` line 49 marks the chair available the moment
its customer stands up, but the chair still moves that customer (lines 65–93). If a new
customer is assigned meanwhile, line 32 overwrites `customer`, and the old one is never
moved or removed again. Now each customer carries its own route and walks itself out; the
chair is free at once, as before, and nobody freezes.

**F2 — The first Advertisement upgrade did nothing.** `UpgradesPanel.java` line 63 is
`Chair.setChanceDivisor(advertisementLevel++)`. The `++` after the name passes the *old*
level, so each purchase applied the previous level's bonus and the first applied none.
The new level is applied.

**F3 — Sound-off did not silence the meows and clicks.** `Sounds.java` lines 77–88 play
them without checking `soundsOn`. And line 255 of `Game.java` set the music button's
picture from the sound switch, leaving the `musicOn` flag disagreeing with what was on
screen. Now the sound switch silences everything, and the music switch follows it both
ways.

**F4 — Order cards promised more than was paid.** `OrderDisplayPanel.java` line 34 shows
`Math.round(cost * 1.5) + tips`; `Player.java` line 126 pays `(int)(cost * 1.5) + tips`.
For an order costing 5 the card said $8 and you got $7. Both now use
`Customer::payout`, which rounds down, so what you earn is unchanged and the card is
right.

**F5 — An empty chair could hide the one beside it.** `Game.checkInteractables` (lines
169–185) stops at the first object in reach, even an empty chair that cannot be used.
Standing between an empty long-table chair and an occupied one, the occupied one could
not be served. The first *usable* object in reach now wins.

**F6 — Counters were in the solid list twice.** `Counter.java` line 13 calls
`addObject(this)`, which adds the counter to `NonCollidables.objects` a second time; the
constructor in `NonCollidables.java` line 19 had already added it. They were drawn and
collision-checked twice. Once now.

**F7 — Customers animated faster in a crowd.** `CustomerImages.java` shares one set of
animations between all customers, so each walking customer advanced everyone's frame.
Each customer now has its own copy.

**F8 — Holding E repeated the action.** `Game.keyPressed` runs `itemChange()` for every
key event, including the repeats the keyboard sends while a key is held, so a cup on a
prep table flickered up and down. One press now does one thing.

**F9 — "Remaining" in the Restock panel went stale.** `RestockingPanel.java` only read
the counter's stock when the selection changed (line 92) or after a purchase (line 76).
Take an item with the panel open and the number was wrong. It is read live.

**F10 — Hover clicks were uneven.** `Game.java` adds the mouse listener to the sound
toggle twice (lines 140 and 149) and never to the music toggle. `UpgradesPanel.drawSection`
re-adds its listener on every repaint. Now every image button clicks once on hover. Upgrade
buttons showing "insufficient" or "max" stay silent, which the Java did deliberately. The
drop-down list in the Restock panel no longer clicks; it is not an image button.

**F11 — Debug output.** The `System.out.println` calls (`Game.java` 199 and 212,
`Player.java` 78, `Animations.java` 15) were not carried over.

**F12 — "Cup" or "cup".** A cup from the counter was labelled `cup`; one picked up from a
prep table was `Cup` (`Cup.java` line 16). Always `cup`.

**F13 — The window clipped the art.** `Main.java` line 48 sets the frame to 1000 × 700
*including* its title bar and borders, which hid a few pixels of the 1000 × 665 art. In Qt
a window's size is its inside, and it is exactly 1000 × 665.

### Noticed and kept, pending your call

Two things look as if they might be accidents but could equally be intended, so they
behave as the Java does. Each is a one-line change if you want it otherwise.

- **Q1 — The cat cannot move while Upgrades is open.** In `Game.java` the Restock and
  Orders buttons call `addNotify()` to take the keyboard back (lines 229–242); the
  Upgrades button does not (243–247). So W A S D and E are dead until Upgrades is closed,
  while customers keep arriving. The check is in `GameScreen::keyPressEvent`.
- **Q2 — The reach box leans right and down.** `Player.java` line 165 builds
  `(x + 2, y + 2, 44, 44)`: 6 pixels past the cat on the right and bottom, 2 pixels short
  on the left and top. A centred box would be `(x - 2, y - 2, 44, 44)`. It is in
  `Player::reachRect`.

### Kept on purpose

The cat stops dead on a collision instead of sliding along the wall. Money can go
negative. A wrong order costs $3. Customers cut corners diagonally.

### Differences that come from the toolkit

- **Speed.** Swing's timers fire late: on your machine the "10 ms" timer measured about
  11.1 ms and the "16 ms" one about 17.2 ms, before the cost of repainting. Qt's fire on
  time. So this version may run around a tenth faster than what you are used to. The
  periods are the ones in your source. If it feels too fast, they are the three
  `setInterval` lines in `CafeScene.cpp`.
- **Buttons** have no Swing border; they are only their picture.
- **The drop-down list and scroll bar** look like Qt's, not Swing's.
- **Text** is drawn by a different engine, so letters sit a pixel differently here and
  there.

## 25. Build, run, test

`README.md` has the short version. In full:

| To | Do |
|---|---|
| Play | Double-click `run.bat` |
| Build only | `build.bat` |
| Run every test | `test.bat` |
| Run some tests | `test.bat player` (any part of a test's name) |
| Start from scratch | Delete the `build` folder, then `build.bat` |

A test that shows `***Not Run` immediately after a build has been held for a moment by
your virus scanner; `test.bat` waits a few seconds and runs it again by itself.

**Qt Creator** is the better way to work on the code. Open `CMakeLists.txt` with it and
choose the kit *Desktop Qt 6.11.2 MinGW 64-bit*. Then:

- Ctrl+click a name to jump to its definition; F4 switches between a `.h` and its `.cpp`.
- F1 on any Qt class or function opens its documentation. Qt's documentation is very
  good, and reading the page for a class you are about to use is the normal way to work.
- The Welcome screen's Examples tab has complete programs. Search it for **Elastic Nodes**
  and **Diagram Scene** before starting the visualizer: one is a graph whose edges follow
  dragged nodes, the other is a small diagram editor.
- F5 runs under the debugger; click in the left margin to set a breakpoint.

## 26. From café to graph visualizer

What carries over, piece by piece.

| In the café | In the visualizer |
|---|---|
| `CafeScene` | A `GraphScene` holding node and edge items |
| `NonCollidable` and friends (picture items) | `NodeItem`, `EdgeItem`: your own `QGraphicsItem` subclasses that draw a circle and a line in `paint()` |
| Child items (held icon, number bubble) | A node's label; an edge's weight |
| Z-values | Edges below nodes |
| `GameScreen` holding a view plus controls | A window with the view in the middle and panels around it |
| `m_view->setInteractive(false)` | Leave it on: items then get mouse events |
| The three `QTimer`s | One timer that advances the algorithm one step |
| `CafeScene::start()` / `stop()` | Play and pause |
| `Player::moneyChanged` → panels | `Algorithm::stepDone` → canvas, log, counters |
| `items/` (plain C++, no Qt drawing) | The graph and the algorithms, as plain classes |
| `QPropertyAnimation` on the money text | A node changing colour when visited |
| `tests/tst_items.cpp` | Tests for each algorithm on small graphs |

**Dragging and selecting** are built in. An item opts in with flags:

```text
NodeItem::NodeItem()
{
    setFlag(QGraphicsItem::ItemIsMovable);            // can be dragged with the mouse
    setFlag(QGraphicsItem::ItemIsSelectable);         // can be clicked to select
    setFlag(QGraphicsItem::ItemSendsGeometryChanges); // tell me when I move
}
```

**Edges that follow nodes.** An edge keeps pointers to its two nodes. A node overrides
`itemChange`, which Qt calls when the item moves, and tells its edges to redraw:

```text
QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged) {
        for (EdgeItem *edge : m_edges)
            edge->adjust();          // recompute the line between its two nodes
    }
    return QGraphicsItem::itemChange(change, value);
}
```

That is the whole trick of the Elastic Nodes example.

**Keep the graph separate from its picture.** The algorithm should work on an ordinary
adjacency list, the kind you write in contests, and know nothing about the screen. The
scene's items are a *view* of that data. `items/` here is the same idea: `Cup` and
`ItemCatalog` never draw anything.

**Running an algorithm without freezing the window.** A `while` loop that runs BFS to
completion inside one click blocks the event loop (chapter 11): nothing is drawn until it
ends. Two ways round it, both simple:

1. Run the algorithm to the end immediately, but have it *record* what it did as a list of
   steps ("visit 3", "relax edge 3→5"). Then a `QTimer` plays the list back, one step per
   tick. Stepping backwards and changing speed come almost free.
2. Write the algorithm as an object with a `step()` function that does one unit of work
   and remembers where it was, like `Customer::step()` here.

The first is easier and is what most visualizers do.

### Four exercises in this codebase

Each is small, and each practises something the visualizer needs. The tests will tell you
if you broke anything.

**1. Make a table draggable.** In `CafeScene::buildWorld`, keep the pointer to one
`Decoration`, call `setFlag(QGraphicsItem::ItemIsMovable)` on it before `addSolid`, and
remove `m_view->setInteractive(false)` in `GameScreen.cpp`. Drag the table. Then walk the
cat to where the table *used* to be: it still bumps into nothing. Work out why (look at
what `addSolid` stores), then fix it so the solid area follows the table. This is the
"edges must follow nodes" problem in disguise.

**2. Add a pause button.** Put one more `ImageButton` in `GameScreen`'s constructor
(reuse any picture), and connect its `clicked` to a lambda that stops the scene if it is
running and starts it if it is not. `CafeScene` already has `start()`, `stop()` and
`isRunning()`. This is your Play/Pause control.

**3. Show the reach boxes.** In `CafeScene::addInteractable`, add a red outline for each
object's reach area: `addRect(item->reachRect(), QPen(Qt::red))` returns a rectangle item;
give it a z-value of 5. Then add one for the cat and move it in `frameTick` with
`setRect(m_player->reachRect())`. You will see Q2 from chapter 24 with your own eyes, and
you will have written your first debug overlay.

**4. Add an eleventh counter.** Add a row to the list in `ItemCatalog.cpp` (reuse a
picture), and one `addCounter(...)` line in `buildWorld` at a free spot, for example
x 75, y 300. Run the game: the Restock list already shows it, and customers start
ordering it, with no other change. Then run `test.bat`. Several tests fail, and each
failure names exactly what you changed. Update them. That is what the tests are for.

