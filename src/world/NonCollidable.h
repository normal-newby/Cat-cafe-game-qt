#pragma once

// NonCollidable: something in the cafe the player cannot walk through. A picture at a
// place, plus the rectangle that blocks movement.
// Replaces: NonCollidables.java (without its static list of every object and its static
//           paint(): the scene keeps the list and does the drawing).
// Guide: chapter 16 (Graphics View: scene, view, items).
//
// The Java class was `abstract`. C++ has no such keyword; a class is abstract when it
// has a pure virtual function, and this one has none. Making the constructor protected
// gives the same result: only subclasses can be created.

#include <QGraphicsPixmapItem>
#include <QRect>
#include <QString>

class NonCollidable : public QGraphicsPixmapItem
{
public:
    QRect rect() const;         // where the picture is
    QRect solidRect() const;    // what blocks the player (the same, unless a subclass changes it)

protected:
    // `image` is an Assets name; it is scaled to fill `rect`.
    NonCollidable(const QString &image, const QRect &rect);

    void setSolidRect(const QRect &rect);

private:
    QRect m_rect;
    QRect m_solidRect;
};
