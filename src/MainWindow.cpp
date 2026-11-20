#include "MainWindow.h"

#include <QStackedWidget>

#include "Assets.h"
#include "audio/SoundManager.h"
#include "screens/GameScreen.h"
#include "screens/InstructionsScreen.h"
#include "screens/MenuScreen.h"
#include "ui/ImageButton.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_screens(new QStackedWidget(this))
    , m_menu(new MenuScreen)
    , m_game(new GameScreen)
    , m_instructions(new InstructionsScreen)
    , m_sounds(new SoundManager(Assets::musicFilePath(), this))
{
    setWindowTitle("Cat Cafe");
    setCentralWidget(m_screens);

    // addWidget() makes each screen a child of the stack, so the stack owns it.
    m_screens->addWidget(m_menu);
    m_screens->addWidget(m_game);
    m_screens->addWidget(m_instructions);

    // The art is 1000 x 665. In Qt a widget's size does not include the title bar and
    // borders, so this is exactly the drawable area.
    setFixedSize(1000, 665);

    // --- Wiring: who reacts to what. Every cross-part connection lives here. ---

    // Moving between screens.
    connect(m_menu, &MenuScreen::playRequested, this, &MainWindow::showGame);
    connect(m_menu, &MenuScreen::instructionsRequested, this, &MainWindow::showInstructions);
    connect(m_game, &GameScreen::backRequested, this, &MainWindow::showMenu);
    connect(m_instructions, &InstructionsScreen::backRequested, this, &MainWindow::showMenu);

    // Sound. Every object with a parent is part of one big tree hanging off this window.
    // findChildren searches that whole tree, so this finds every ImageButton on every
    // screen and panel, however deeply nested, and gives each one the hover click.
    const QList<ImageButton *> buttons = findChildren<ImageButton *>();
    for (ImageButton *button : buttons)
        connect(button, &ImageButton::hovered, m_sounds, &SoundManager::playClick);

    connect(m_game, &GameScreen::orderServed, m_sounds, &SoundManager::playMeow);
    connect(m_game, &GameScreen::soundToggleRequested, m_sounds, &SoundManager::toggleSound);
    connect(m_game, &GameScreen::musicToggleRequested, m_sounds, &SoundManager::toggleMusic);
    connect(m_sounds, &SoundManager::stateChanged, m_game, &GameScreen::showAudioState);
}

QWidget *MainWindow::currentScreen() const
{
    return m_screens->currentWidget();
}

SoundManager *MainWindow::sounds() const
{
    return m_sounds;
}

void MainWindow::showMenu()
{
    m_game->stop();
    m_instructions->stop();
    m_sounds->stopMusic();
    m_screens->setCurrentWidget(m_menu);
}

void MainWindow::showGame()
{
    m_screens->setCurrentWidget(m_game);
    m_game->start();
    m_sounds->startMusic();
}

void MainWindow::showInstructions()
{
    m_screens->setCurrentWidget(m_instructions);
    m_instructions->start();
}
