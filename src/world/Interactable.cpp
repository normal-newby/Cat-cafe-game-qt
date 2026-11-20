#include "world/Interactable.h"

Interactable::Interactable(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side)
    : NonCollidable(image, rect)   // the base class is constructed first, like Java's super(...)
    , m_reachX(reachX)
    , m_reachY(reachY)
    , m_side(side)
{
}

QRect Interactable::reachRect() const
{
    // adjusted() moves the left, top, right and bottom edges by the given amounts.
    return rect().adjusted(-m_reachX, -m_reachY, m_reachX, m_reachY);
}

QRect Interactable::popUpRect() const
{
    const QRect r = rect();
    const int size = 50;

    switch (m_side) {
    case PopUpSide::Up:
        return QRect(r.x() + r.width() / 2 - 25, r.y() - size, size, size);
    case PopUpSide::Down:
        return QRect(r.x() + r.width() / 2 - 25, r.y() + r.height(), size, size);
    case PopUpSide::Left:
        return QRect(r.x() - size, r.y() + r.height() / 2 - 25, size, size);
    case PopUpSide::Right:
        return QRect(r.x() + r.width(), r.y() + r.height() / 2 - 25, size, size);
    }
    return QRect();   // not reached: every value of the enum is handled above
}

PopUpSide Interactable::popUpSide() const
{
    return m_side;
}

bool Interactable::canInteract() const
{
    return true;
}
