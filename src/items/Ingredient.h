#pragma once

// Ingredient: one thing that can go into a cup (milk, boba, sugar, ...).
// Replaces: Ingredient.java, and Liquid.java and Topping.java, which only differed in
//           which static list they added themselves to.
// Guide: chapter 2 (constructors and member initializer lists).

#include <QString>

#include "items/Item.h"

struct ItemInfo;   // defined in ItemCatalog.h; a reference to it is all this header needs

class Ingredient : public Item
{
public:
    explicit Ingredient(const ItemInfo &info);

    QString name() const override;
    QString image() const override;

private:
    QString m_name;
    QString m_image;
};
