#include "world/PrepTable.h"

#include "Assets.h"
#include "world/Player.h"

PrepTable::PrepTable(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side)
    : Interactable(image, rect, reachX, reachY, side)
    , m_cupPicture(new QGraphicsPixmapItem(this))
{
    m_cupPicture->setPos(15, -10);   // relative to the table's top-left corner
    m_cupPicture->hide();
}

const Cup *PrepTable::cup() const
{
    return m_cup.get();
}

Outcome PrepTable::interact(Player &player)
{
    Item *held = player.heldItem();

    // Empty paws: pick the cup up, if there is one.
    if (held == nullptr) {
        if (m_cup == nullptr)
            return Outcome::Nothing;

        // std::move hands the cup over. Afterwards m_cup is empty and the player owns it.
        player.give(std::move(m_cup));
        showCup();
        return Outcome::PickedCupUp;
    }

    // Holding a cup: put it down, if the table is free.
    if (dynamic_cast<Cup *>(held) != nullptr) {
        if (m_cup != nullptr)
            return Outcome::Nothing;

        m_cup = player.takeCup();
        showCup();
        return Outcome::PutCupDown;
    }

    // Holding an ingredient: it goes into the cup on the table, if there is one.
    if (m_cup == nullptr)
        return Outcome::Nothing;

    m_cup->add(held->name());
    player.takeItem();   // nobody keeps what this returns, so the ingredient is destroyed here
    showCup();
    return Outcome::AddedIngredient;
}

void PrepTable::showCup()
{
    if (m_cup != nullptr) {
        m_cupPicture->setPixmap(Assets::pixmap(m_cup->image(), QSize(50, 50)));
        m_cupPicture->show();
    } else {
        m_cupPicture->hide();
    }
}
