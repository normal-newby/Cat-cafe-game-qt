#pragma once

// MainWindow: the game's one window.
// Replaces: Main.java (the JFrame and its CardLayout).
// Guide: chapter 12 (the object tree) and chapter 13 (signals and slots).
//
// It holds a QStackedWidget, which is Qt's CardLayout: several screens stacked on top of
// each other with one showing. It is also the only class that knows about every part of
// the game, so it is where the parts get connected to each other.

#include <QMainWindow>

class GameScreen;
class InstructionsScreen;
class MenuScreen;
class QStackedWidget;
class SoundManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    QWidget *currentScreen() const;
    SoundManager *sounds() const;

public slots:
    void showMenu();
    void showGame();
    void showInstructions();

private:
    QStackedWidget *m_screens;
    MenuScreen *m_menu;
    GameScreen *m_game;
    InstructionsScreen *m_instructions;
    SoundManager *m_sounds;
};
