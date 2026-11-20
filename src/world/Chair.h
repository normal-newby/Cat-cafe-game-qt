#pragma once

// Chair: a seat a customer walks to. Giving the seated customer a cup serves them.
// Replaces: Chair.java, without the static lists of chairs and free chairs (the scene
//           keeps those) and without moving the customer (customers now move themselves).
// Guide: chapter 8 (forward declarations and include cycles).
//
// Chair needs to know about Customer, and Customer needs to know about Chair. Two
// headers cannot include each other, so each one only *declares* the other class
// (`class Customer;`) and the real #include goes in the .cpp file.

#include <QList>
#include <QPoint>

#include "world/Interactable.h"

class Customer;

class Chair : public Interactable
{
public:
    // `route` is the path from the door to this chair, as a list of corners.
    Chair(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side,
          bool facesRight, const QList<QPoint> &route);

    bool facesRight() const;
    const QList<QPoint> &route() const;

    bool isFree() const;                 // nobody is sitting here or on their way here
    Customer *customer() const;
    void assign(Customer *customer);     // this customer is now heading for this chair

    bool canInteract() const override;   // only once the customer has sat down
    Outcome interact(Player &player) override;

    static constexpr int WrongOrderPenalty = 3;

private:
    bool m_facesRight;
    QList<QPoint> m_route;
    Customer *m_customer = nullptr;      // not owned: the scene owns every customer
};
