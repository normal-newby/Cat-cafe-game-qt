#pragma once

// GameScreen: the screen you play on. It shows the cafe, turns key presses into player
// movement, and holds the controls that sit around and on top of the cafe.
// Replaces: Game.java (the JLayeredPane): its KeyListener, its buttons and its panels.
//           The world itself moved to CafeScene.
// Guide: chapter 14 (events) and chapter 16 (Graphics View: scene, view, items).
//
// A scene cannot be seen by itself. A QGraphicsView is the widget that shows one. This
// screen is a plain QWidget containing that view, with ordinary widgets placed over it.

#include <QString>
#include <QWidget>

class CafeScene;
class HeldItemPanel;
class ImageButton;
class OrderPanel;
class QGraphicsView;
class QLabel;
class QPropertyAnimation;
class RestockingPanel;
class UpgradesPanel;

class GameScreen : public QWidget
{
    Q_OBJECT

public:
    enum class Panel { None, Restock, Orders, Upgrades };

    explicit GameScreen(QWidget *parent = nullptr);

    CafeScene *scene() const;

    void start();   // start the cafe's clocks and take the keyboard
    void stop();    // stop them and let go of every key

    Panel openPanel() const;
    QString bannerImage() const;   // the picture of the banner beside the Current Item box

public slots:
    void togglePanel(Panel panel);                      // open it, or close it if it is the open one
    void showAudioState(bool soundOn, bool musicOn);    // set the two toggle buttons' pictures

signals:
    void backRequested();
    void soundToggleRequested();
    void musicToggleRequested();
    void orderServed(bool correct);   // passed on from the scene, for the meows

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private slots:
    void showMoney(int money);
    void showMoneyChange(bool correct, int moneyDelta);

private:
    CafeScene *m_scene;
    QGraphicsView *m_view;

    HeldItemPanel *m_heldItemPanel;
    QLabel *m_banner;
    QLabel *m_moneyLabel;
    QLabel *m_moneyDeltaLabel;
    QPropertyAnimation *m_moneyDeltaDrift;
    ImageButton *m_soundButton;
    ImageButton *m_musicButton;

    RestockingPanel *m_restockPanel;
    OrderPanel *m_orderPanel;
    UpgradesPanel *m_upgradesPanel;
    Panel m_openPanel = Panel::None;
};
