#pragma once

// OrderCard: one row of the Orders panel: a customer's number, what they ordered, what
// it costs to make and what they will pay.
// Replaces: OrderDisplayPanel.java.
// Guide: chapter 4 (who owns what).
//
// The Java card kept a reference to its Customer. In C++ that customer may be deleted
// while the card is still on screen, and a pointer to a deleted object is a crash
// waiting to happen. So the card copies the few values it shows and points at nothing.

#include <QStringList>
#include <QWidget>

class OrderCard : public QWidget
{
    Q_OBJECT

public:
    OrderCard(int number, const QStringList &ingredients, int cost, int payout, QWidget *parent = nullptr);

    int number() const;
    int payout() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_number;
    QStringList m_ingredients;
    int m_cost;
    int m_payout;
};
