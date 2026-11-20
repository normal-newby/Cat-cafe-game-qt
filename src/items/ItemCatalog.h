#pragma once

// ItemCatalog: the list of every kind of item in the game and what each one costs.
// Replaces: the static lists in Item.java, Liquid.java and Topping.java, plus
//           Item.findItem(), Liquid.randomItem(), Topping.randomItem(), Cup.randomOrder()
//           and Customer.getOrderCost().
// Guide: chapter 4 (who owns what) and chapter 6 (enum class).
//
// In the Java every item object added itself to a static list. Here there is one
// ordinary object that holds the definitions; CafeScene owns it.

#include <QList>
#include <QString>
#include <QStringList>
#include <memory>

#include "items/Cup.h"
#include "items/Item.h"

class QRandomGenerator;

// `enum class` is a set of named values with its own type. Unlike the strings the Java
// used ("up", "down", ...), a misspelt value does not compile.
enum class ItemKind { Cup, Liquid, Topping, Extra };

// A plain bundle of data, so it is a struct: everything public, no behaviour.
struct ItemInfo
{
    QString name;
    ItemKind kind;
    int cost;
    QString image;
};

class ItemCatalog
{
public:
    ItemCatalog();

    // The definition with this name, or nullptr if there is none.
    const ItemInfo *find(const QString &name) const;

    // The names of every item of one kind, in the order they were defined.
    QStringList namesOf(ItemKind kind) const;

    // A brand-new item for the player to carry: a Cup for "cup", otherwise an Ingredient.
    // The caller owns the result. Returns nullptr for an unknown name.
    std::unique_ptr<Item> make(const QString &name) const;

    // What an order costs to make: 1 for the cup, plus the cost of each ingredient.
    int orderCost(const Cup &cup) const;

    // A customer's order: one liquid, one topping, and sugar and ice half the time each.
    Cup randomOrder(QRandomGenerator &rng) const;

private:
    QList<ItemInfo> m_items;
};
