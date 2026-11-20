#include "items/Cup.h"

Cup::Cup(const QStringList &ingredients)
    : m_ingredients(ingredients)
{
}

void Cup::add(const QString &ingredient)
{
    m_ingredients.append(ingredient);
}

const QStringList &Cup::ingredients() const
{
    return m_ingredients;
}

bool Cup::matches(const Cup &other) const
{
    // Sort copies of both lists, then compare them. The copies are ours to reorder;
    // the cups themselves are not changed (this function is const).
    QStringList mine = m_ingredients;
    QStringList theirs = other.m_ingredients;
    mine.sort();
    theirs.sort();
    return mine == theirs;
}

QString Cup::name() const
{
    return "cup";
}

QString Cup::image() const
{
    return m_ingredients.isEmpty() ? "emptyCup" : "filledCup";
}
