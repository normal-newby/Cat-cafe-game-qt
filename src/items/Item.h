#pragma once

// Item: anything the player can carry.
// Replaces: Item.java (the abstract class; its static list of every item is now ItemCatalog).
// Guide: chapter 5 (virtual, override, = 0).
//
// This is an abstract class: it only says what every item can do. In C++ a function is
// abstract when it is declared `virtual ... = 0`. There is no `abstract` keyword.

#include <QString>

class Item
{
public:
    // A class that will be deleted through a pointer to its base needs a virtual
    // destructor, so that deleting an Item* that is really a Cup runs Cup's clean-up.
    virtual ~Item() = default;

    // The word shown in the "Current Item" box.
    virtual QString name() const = 0;

    // The name of its picture, to be loaded with Assets::pixmap().
    virtual QString image() const = 0;
};
