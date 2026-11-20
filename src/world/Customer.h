#pragma once

// Customer: walks from the door to a chair, sits and waits for a drink, then walks out.
// Replaces: Customer.java and CustomerImages.java, plus the walking code that lived in
//           Chair.updateCustomerPosition().
// Guide: chapter 6 (enum class) and chapter 16 (scene items).
//
// In the Java a chair moved "its" customer, so a customer whose chair was given to
// someone else was never moved again and froze on screen. Here each customer carries its
// own route and moves itself.

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPixmap>
#include <QPoint>

#include "items/Cup.h"
#include "world/SpriteAnimation.h"

class Chair;

// The customer pictures, loaded once and then copied into each customer. A copy shares
// the pixels but has its own animation position, so customers do not speed each other up.
struct CustomerSprites
{
    CustomerSprites();

    SpriteAnimation up;
    SpriteAnimation down;
    SpriteAnimation left;
    SpriteAnimation right;
    QPixmap seatedLeft;
    QPixmap seatedRight;
    QList<QPixmap> numberBubbles;   // [0] is the bubble for customer number 1
};

class Customer : public QGraphicsPixmapItem
{
public:
    enum class State { Arriving, Seated, Leaving, Gone };

    Customer(const Cup &order, int orderCost, const Chair &chair, const CustomerSprites &sprites);

    State state() const;
    const Cup &order() const;
    int orderCost() const;
    int payout(int tips) const;   // what a correct drink earns

    void step();                  // one 10 ms tick: walk one pixel
    void leave();                 // get up and head for the door

    void setNumber(int number);   // the number in the bubble over its head (1 to 8)
    int number() const;

private:
    bool walkTowards(const QPoint &target);   // true once it is standing on `target`
    void face(int dx, int dy);
    void sitDown();
    void showNumber();

    Cup m_order;
    int m_orderCost;

    QList<QPoint> m_route;
    QPoint m_seat;
    bool m_seatFacesRight;

    CustomerSprites m_sprites;
    State m_state = State::Arriving;
    int m_target = 0;             // index into m_route of the corner it is heading for
    int m_number = 0;

    QGraphicsPixmapItem *m_bubble;   // a child item: it sits above the customer's head
};
