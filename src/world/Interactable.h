#pragma once

// Interactable: a NonCollidable the player can use by standing next to it and pressing E.
// Replaces: Interactables.java.
// Guide: chapter 5 (virtual, override, = 0) and chapter 6 (enum class).
//
// The Java class declared twelve abstract methods (getCup, setHasCup, customerSeated,
// getItemsLeft, ...) so that Player.itemChange() could ask any object any question, and
// every subclass had to write empty versions of the ones that made no sense for it.
// Here there is one question, interact(): "the player pressed E next to you; do your
// thing and say what happened". Each subclass contains only its own behaviour.

#include <QRect>
#include <QString>

#include "world/NonCollidable.h"

class Player;   // a reference to a Player is all this header needs

// Which side of the object its "press E" pop-up appears on.
enum class PopUpSide { Up, Down, Left, Right };

// What interact() did. The scene reads this and decides what the rest of the game
// should do about it (play a sound, refresh a panel, show a message).
enum class Outcome {
    Nothing,
    TookItem,          // took one from a counter
    CounterEmpty,      // tried to, but the counter has run out
    PutCupDown,        // onto a prep table
    PickedCupUp,       // off a prep table
    AddedIngredient,   // into the cup on a prep table
    Discarded,         // into a bin
    ServedRight,       // gave a customer what they ordered
    ServedWrong        // gave a customer something else
};

class Interactable : public NonCollidable
{
public:
    // reachX / reachY: how far beyond the picture, sideways and up-and-down, the player
    // can still reach it.
    Interactable(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side);

    QRect reachRect() const;
    QRect popUpRect() const;      // where the 50 x 50 pop-up goes
    PopUpSide popUpSide() const;

    // Can it be used right now? Almost always yes; a chair says no while it is empty.
    virtual bool canInteract() const;

    // "= 0" makes this pure virtual: there is no version here, every subclass must
    // write one. It is also what makes Interactable an abstract class.
    virtual Outcome interact(Player &player) = 0;

private:
    int m_reachX;
    int m_reachY;
    PopUpSide m_side;
};
