#pragma once

// MenuScreen: the title screen with Play and Instructions.
// Replaces: Menu.java.
// Guide: chapter 13 (signals and slots).
//
// The Java menu reached into Main to flip the CardLayout itself. This screen does not
// know that other screens exist: it only announces what the player asked for, and
// MainWindow decides what that means.

#include <QWidget>

class MenuScreen : public QWidget
{
    Q_OBJECT

public:
    explicit MenuScreen(QWidget *parent = nullptr);

signals:
    void playRequested();
    void instructionsRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
};
