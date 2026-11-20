#pragma once

// InstructionsScreen: the how-to-play page, with a small looping animation of a customer
// walking to a chair and sitting down.
// Replaces: Instructions.java.
// Guide: chapter 14 (painting) and chapter 15 (timers).
//
// This screen is drawn the Swing way: one paintEvent() that draws everything, and a
// timer that changes some numbers and asks for a repaint. Compare it with GameScreen,
// where a scene of objects does the drawing instead.

#include <QPixmap>
#include <QWidget>

#include "world/SpriteAnimation.h"

class QTimer;

class InstructionsScreen : public QWidget
{
    Q_OBJECT

public:
    explicit InstructionsScreen(QWidget *parent = nullptr);

    void start();            // begin animating (called when the screen is shown)
    void stop();             // stop animating (called when it is left)
    bool isRunning() const;

    int customerY() const;
    bool customerVisible() const;
    bool chairOccupied() const;

public slots:
    void advance();          // one step of the animation; the timer calls this every 30 ms

signals:
    void backRequested();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QTimer *m_timer;

    SpriteAnimation m_customerWalk;
    SpriteAnimation m_cat;
    QPixmap m_customerFrame;
    QPixmap m_catFrame;

    int m_customerY = 200;
    int m_pauseTicks = 0;         // > 0 while the customer is "sitting" and not drawn
    bool m_chairOccupied = false;
};
