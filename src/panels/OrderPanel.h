#pragma once

// OrderPanel: a scrolling list with one card for each customer who is sitting and waiting.
// Replaces: OrderPanel.java.
// Guide: chapter 17 (layouts and scroll areas).
//
// Almost everything in this game is placed at fixed pixel positions, because the art was
// drawn that way. This panel is the exception: it uses a layout, which is how Qt programs
// normally arrange widgets. You hand widgets to the layout and it positions them.

#include <QList>
#include <QWidget>

class CafeScene;
class OrderCard;
class QVBoxLayout;

class OrderPanel : public QWidget
{
    Q_OBJECT

public:
    explicit OrderPanel(CafeScene *scene, QWidget *parent = nullptr);

    int cardCount() const;
    const OrderCard *card(int index) const;

public slots:
    void rebuild();   // throw the cards away and make new ones from who is seated now

private:
    CafeScene *m_scene;
    QWidget *m_content;          // the tall widget inside the scroll area
    QVBoxLayout *m_cardLayout;   // stacks the cards top to bottom
    QList<OrderCard *> m_cards;
};
