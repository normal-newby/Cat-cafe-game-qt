#pragma once

// Player: the cat the player steers. Knows where it is, what it is carrying and how much
// money it has.
// Replaces: Player.java, without the painting (the scene draws it) and without
//           itemChange() (each Interactable now decides what E does to it).
// Guide: chapter 7 (unique_ptr), chapter 13 (signals) and chapter 16 (scene items).
//
// It inherits from two classes. QGraphicsPixmapItem makes it a picture the scene can
// draw and move. QObject lets it have signals. When a class inherits from QObject and
// something else, QObject must be listed first.

#include <QGraphicsPixmapItem>
#include <QList>
#include <QObject>
#include <QRect>
#include <memory>

#include "items/Cup.h"
#include "items/Item.h"
#include "world/SpriteAnimation.h"

class Player : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    Player();   // no parent: the scene it is added to becomes its owner

    // --- movement ---
    void setKeyHeld(int key, bool held);        // Qt::Key_W, A, S or D; other keys are ignored
    void stop();                                // as if every key had been released
    void step(const QList<QRect> &solids);      // one 16 ms tick: move, collide, animate
    void moveTo(double x, double y);            // jump somewhere (used by the tests)
    QRect bodyRect() const;                     // the part of the cat that bumps into things
    QRect reachRect() const;                    // how far it can reach to use something

    // --- what it carries ---
    Item *heldItem() const;                     // nullptr when its paws are empty
    void give(std::unique_ptr<Item> item);      // only when its paws are empty
    std::unique_ptr<Item> takeItem();           // hands over whatever it holds (maybe nothing)
    std::unique_ptr<Cup> takeCup();             // hands over the cup; does nothing if it holds no cup

    // --- money and upgrades ---
    int money() const;
    void addMoney(int delta);
    int tips() const;
    void addTip();
    double speed() const;
    void addSpeed(double amount);

signals:
    void moneyChanged(int money);
    void heldItemChanged();
    void tipsChanged(int tips);

private:
    SpriteAnimation &currentAnimation();
    void heldItemWasChanged();

    double m_x = 100;
    double m_y = 100;
    double m_speed = 2.2;
    bool m_up = false;
    bool m_down = false;
    bool m_left = false;
    bool m_right = false;

    std::unique_ptr<Item> m_held;   // the player owns what it carries
    int m_money = 1000;
    int m_tips = 0;

    SpriteAnimation m_stationary;
    SpriteAnimation m_rollRight;
    SpriteAnimation m_rollLeft;
    SpriteAnimation m_rollUp;
    SpriteAnimation m_rollDown;

    QGraphicsPixmapItem *m_heldIcon;   // the small picture beside the cat; a child item
};
