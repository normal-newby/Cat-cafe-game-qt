#pragma once

// Trashbin: whatever the player is carrying is thrown away.
// Replaces: Trashbin.java (which was sixty lines of empty overrides).
// Guide: chapter 5 (virtual, override, = 0).

#include "world/Interactable.h"

class Trashbin : public Interactable
{
public:
    Trashbin(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side);

    Outcome interact(Player &player) override;
};
