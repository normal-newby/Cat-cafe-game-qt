#pragma once

// Cup: a cup and the ingredients that have been put into it. A customer's order is
// also a Cup: "the cup I would like to receive".
// Replaces: Cup.java.
// Guide: chapter 1 (headers and sources) and chapter 7 (unique_ptr: the cup changes hands).

#include <QString>
#include <QStringList>

#include "items/Item.h"

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
