#pragma once

// HeldItemPanel: the "Current Item" box in the bottom-left corner.
// Replaces: the second half of Player.paint() in Player.java, which drew this box, its
//           text and the held item's picture.
// Guide: chapter 14 (widgets, events and painting).
//
// In the Java the player drew its own status box. Here the player only says "what I am
// holding changed" (a signal), and this widget, which knows how to draw the box, listens.

#include <QString>
#include <QWidget>

class Player;

class HeldItemPanel : public QWidget
{
    Q_OBJECT

public:
    explicit HeldItemPanel(Player *player, QWidget *parent = nullptr);

    QString shownName() const;   // the held item's name, or "Nothing"

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void refresh();

private:
    Player *m_player;   // not owned: the scene owns the player
};
