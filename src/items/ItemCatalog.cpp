#include "items/ItemCatalog.h"

#include <QRandomGenerator>

#include "items/Ingredient.h"

ItemCatalog::ItemCatalog()
    : m_items{
          {"boba", ItemKind::Topping, 2, "boba"},
          {"milk", ItemKind::Liquid, 2, "milk"},
          {"sugar", ItemKind::Extra, 1, "sugar"},
          {"ice", ItemKind::Extra, 1, "ice"},
          {"grapefruit", ItemKind::Liquid, 2, "grapefruit"},
          {"greentea", ItemKind::Liquid, 2, "greentea"},
          {"jelly", ItemKind::Topping, 2, "jelly"},
          {"lychee", ItemKind::Liquid, 2, "lychee"},
          {"mango", ItemKind::Liquid, 2, "mango"},
          {"cup", ItemKind::Cup, 1, "emptyCup"},
      }
{
}

const ItemInfo *ItemCatalog::find(const QString &name) const
{
    // `const ItemInfo &info` looks at each element in place instead of copying it.
    for (const ItemInfo &info : m_items) {
        if (info.name == name)
            return &info;   // the address of the element inside the list
    }
    return nullptr;
}

QStringList ItemCatalog::namesOf(ItemKind kind) const
{
    QStringList names;
    for (const ItemInfo &info : m_items) {
        if (info.kind == kind)
            names.append(info.name);
    }
    return names;
}

std::unique_ptr<Item> ItemCatalog::make(const QString &name) const
{
    const ItemInfo *info = find(name);
    if (info == nullptr)
        return nullptr;

    // make_unique is `new` that hands back an owning pointer: whoever holds the
    // unique_ptr owns the object, and the object is deleted when that holder lets go.
    if (info->kind == ItemKind::Cup)
        return std::make_unique<Cup>();
    return std::make_unique<Ingredient>(*info);
}

int ItemCatalog::orderCost(const Cup &cup) const
{
    int cost = 1;   // the cup itself
    for (const QString &ingredient : cup.ingredients()) {
        const ItemInfo *info = find(ingredient);
        if (info != nullptr)
            cost += info->cost;
    }
    return cost;
}

Cup ItemCatalog::randomOrder(QRandomGenerator &rng) const
{
    const QStringList liquids = namesOf(ItemKind::Liquid);
    const QStringList toppings = namesOf(ItemKind::Topping);

    Cup order;
    order.add(liquids.at(rng.bounded(liquids.size())));     // bounded(n) is 0 .. n-1
    order.add(toppings.at(rng.bounded(toppings.size())));
    if (rng.bounded(2) == 0)
        order.add("sugar");
    if (rng.bounded(2) == 0)
        order.add("ice");
    return order;
}
