#include "world/Counter.h"

#include "items/ItemCatalog.h"
#include "world/Player.h"

Counter::Counter(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side,
                 const ItemCatalog &catalog, const QString &itemName, int stock)
    : Interactable(image, rect, reachX, reachY, side)
    , m_catalog(catalog)
    , m_itemName(itemName)
    , m_stock(stock)
{
}

QString Counter::itemName() const
{
    return m_itemName;
}

int Counter::stock() const
{
    return m_stock;
}

void Counter::addStock(int amount)
{
    m_stock += amount;
}

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
