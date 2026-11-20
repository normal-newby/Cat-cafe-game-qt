#include "world/NonCollidable.h"

#include "Assets.h"

NonCollidable::NonCollidable(const QString &image, const QRect &rect)
    : m_rect(rect)
    , m_solidRect(rect)
{
    setPixmap(Assets::pixmap(image, rect.size()));
    setPos(rect.topLeft());
    setZValue(2);   // above the player (1), below customers (3)
}

QRect NonCollidable::rect() const
{
    return m_rect;
}

QRect NonCollidable::solidRect() const
{
    return m_solidRect;
}

void NonCollidable::setSolidRect(const QRect &rect)
{
    m_solidRect = rect;
}
