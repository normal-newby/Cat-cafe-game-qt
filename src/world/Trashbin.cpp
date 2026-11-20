#include "world/Trashbin.h"

#include "world/Player.h"

Trashbin::Trashbin(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side)
    : Interactable(image, rect, reachX, reachY, side)
{
}

Outcome Trashbin::interact(Player &player)
{
    if (player.heldItem() == nullptr)
        return Outcome::Nothing;

    // takeItem() returns the item in a unique_ptr. Nothing stores it, so it is deleted
    // at the end of this statement. In C++, "throwing it away" is simply not keeping it.
    player.takeItem();
    return Outcome::Discarded;
}
