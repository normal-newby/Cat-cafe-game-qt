#pragma once

// Decoration: a NonCollidable that does nothing but be in the way (tables, walls).
// Replaces: Decorations.java.
// Guide: chapter 5 (inheritance).
//
// The whole class fits in the header: its one function is so short that it is written
// right here instead of in a .cpp file.

#include "world/NonCollidable.h"

class Decoration : public NonCollidable
{
public:
    Decoration(const QString &image, const QRect &rect)
        : NonCollidable(image, rect)
    {
    }
};
