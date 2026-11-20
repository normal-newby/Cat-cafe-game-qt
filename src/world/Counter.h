#pragma once

// Counter: a box of one kind of item. An empty-pawed player takes one from it.
// Replaces: Counter.java.
// Guide: chapter 3 (values, references and pointers).

#include <QString>

#include "world/Interactable.h"

class ItemCatalog;

class Counter : public Interactable
{
public:
    Counter(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side,
            const ItemCatalog &catalog, const QString &itemName, int stock);

    QString itemName() const;
    int stock() const;
    void addStock(int amount);

    Outcome interact(Player &player) override;

private:
    // A reference member: this counter does not own the catalog, it just knows where it
    // is. The catalog must exist for as long as the counter does (the scene owns both).
    const ItemCatalog &m_catalog;
    QString m_itemName;
    int m_stock;
};
