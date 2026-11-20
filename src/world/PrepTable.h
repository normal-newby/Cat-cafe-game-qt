#pragma once

// PrepTable: a table that can hold one cup while ingredients are added to it.
// Replaces: PrepTable.java.
// Guide: chapter 7 (unique_ptr and std::move: the cup changes hands).

#include <memory>

#include "items/Cup.h"
#include "world/Interactable.h"

class PrepTable : public Interactable
{
public:
    PrepTable(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side);

    const Cup *cup() const;   // the cup on the table, or nullptr; for looking only

    Outcome interact(Player &player) override;

private:
    void showCup();

    std::unique_ptr<Cup> m_cup;            // the table owns the cup while it stands here
    QGraphicsPixmapItem *m_cupPicture;     // drawn on top of the table; a child item
};
