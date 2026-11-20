#pragma once

// CafeScene: the cafe itself. It owns every object in it, runs the three game clocks,
// and announces what happens with signals.
// Replaces: the world-building half of Game.java, Game.checkInteractables(), the timer
//           branches of Game.actionPerformed(), and the static lists that lived in
//           NonCollidables, Interactables, Counter, Chair and Customer.
// Guide: chapter 15 (timers) and chapter 16 (Graphics View: scene, view, items).
//
// A QGraphicsScene is a container of items. You add items to it; Qt draws them in
// z-order, and deletes them all when the scene is deleted. Nothing here paints anything.

#include <QGraphicsScene>
#include <QList>
#include <QRect>

#include "items/ItemCatalog.h"
#include "world/Customer.h"
#include "world/Interactable.h"
#include "world/SpawnOdds.h"

class Chair;
class Counter;
class NonCollidable;
class Player;
class QTimer;

class CafeScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit CafeScene(QObject *parent = nullptr);

    // --- what is in the cafe ---
    Player *player() const;
    const ItemCatalog &catalog() const;
    const QList<Counter *> &counters() const;
    Counter *findCounter(const QString &itemName) const;   // nullptr if there is none
    const QList<Chair *> &chairs() const;
    const QList<Interactable *> &interactables() const;
    QList<QRect> solidRects() const;                       // everything that blocks the player

    // --- customers ---
    QList<Customer *> seatedCustomers() const;             // in arrival order; number = index + 1
    int customerCount() const;                             // arriving + seated + leaving
    Customer *seatNewCustomer(Chair *chair);               // a new customer sets off for `chair`
    SpawnOdds &spawnOdds();

    // --- the thing the player could use right now ---
    Interactable *target() const;                          // nullptr if nothing is in reach

    void restock(Counter *counter, int amount);

    // --- the clocks ---
    void start();
    void stop();
    bool isRunning() const;

public slots:
    void frameTick();       // every 16 ms: move the player, work out what is in reach
    void customerTick();    // every 10 ms: every customer takes a step
    void spawnTick();       // every second: maybe a new customer arrives
    void interact();        // the player pressed E

signals:
    void ordersChanged();                               // a customer sat down or was served
    void stockChanged();                                // some counter has more or fewer items
    void orderServed(bool correct, int moneyDelta);     // moneyDelta: +payout or -3
    void counterEmpty();                                // the player tried an empty counter

private slots:
    void hideOutOfStockMessage();

private:
    void buildWorld();
    void addSolid(NonCollidable *item);
    void addInteractable(Interactable *item);
    void addCounter(const QString &image, int x, int y, int reachX, int reachY, PopUpSide side,
                    const QString &itemName, int stock);
    void addChair(const QString &image, int x, int y, bool facesRight, const QList<QPoint> &route);
    void updateTarget();
    void renumberSeated();

    ItemCatalog m_catalog;
    CustomerSprites m_customerSprites;
    SpawnOdds m_spawnOdds;

    // The scene owns all of these items; the lists are just ways to find them.
    Player *m_player;
    QList<Interactable *> m_interactables;
    QList<Counter *> m_counters;
    QList<Chair *> m_chairs;
    QList<QRect> m_solidRects;
    QList<Customer *> m_customers;

    Interactable *m_target = nullptr;
    QGraphicsPixmapItem *m_popUp;
    QGraphicsPixmapItem *m_outOfStockMessage;

    QTimer *m_frameTimer;
    QTimer *m_customerTimer;
    QTimer *m_spawnTimer;
    QTimer *m_messageTimer;
};
