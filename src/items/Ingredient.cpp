#include "items/Ingredient.h"

#include "items/ItemCatalog.h"

// The part after the colon is the member initializer list: it constructs m_name and
// m_image directly from the given values, before the (empty) constructor body runs.
Ingredient::Ingredient(const ItemInfo &info)
    : m_name(info.name)
    , m_image(info.image)
{
}

QString Ingredient::name() const
{
    return m_name;
}

QString Ingredient::image() const
{
    return m_image;
}
