// Tests for GameScreen: the keyboard, the view, and the controls around the cafe.
#include <QtTest>

#include <QComboBox>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QLabel>
#include <QSignalSpy>
#include <memory>

#include "items/Cup.h"
#include "panels/HeldItemPanel.h"
#include "panels/OrderPanel.h"
#include "panels/RestockingPanel.h"
#include "panels/UpgradesPanel.h"
#include "screens/GameScreen.h"
#include "ui/ImageButton.h"
#include "world/CafeScene.h"
#include "world/Chair.h"
#include "world/Customer.h"
#include "world/Player.h"

class TestGameScreen : public QObject
{
    Q_OBJECT

    // Delivers one key event straight to a widget, as if the keyboard had sent it.
    static void sendKey(QWidget *widget, QEvent::Type type, Qt::Key key, bool autoRepeat = false)
    {
        QKeyEvent event(type, key, Qt::NoModifier, QString(), autoRepeat);
        QApplication::sendEvent(widget, &event);
    }

private slots:
    void viewIsInert()
    {
        GameScreen g;
        QCOMPARE(g.size(), QSize(1000, 665));
        QCOMPARE(g.focusPolicy(), Qt::StrongFocus);

        auto *view = g.findChild<QGraphicsView *>();
        QVERIFY(view != nullptr);
        QCOMPARE(view->geometry(), QRect(0, 0, 1000, 665));
        QCOMPARE(view->focusPolicy(), Qt::NoFocus);
        QCOMPARE(view->horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(view->verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(view->scene(), g.scene());
    }

    void keysDriveThePlayer()
    {
        GameScreen g;
        sendKey(&g, QEvent::KeyPress, Qt::Key_D);
        g.scene()->frameTick();
        QVERIFY(g.scene()->player()->pos().x() > 100);

        const QPointF where = g.scene()->player()->pos();
        sendKey(&g, QEvent::KeyRelease, Qt::Key_D);
        g.scene()->frameTick();
        QCOMPARE(g.scene()->player()->pos(), where);
    }

    void eInteractsOncePerPress()   // fix F8
    {
        GameScreen g;
        g.scene()->player()->moveTo(250, 100);   // at the cup counter
        g.scene()->frameTick();

        sendKey(&g, QEvent::KeyPress, Qt::Key_E, true);   // a held key repeating: ignored
        QVERIFY(g.scene()->player()->heldItem() == nullptr);

        sendKey(&g, QEvent::KeyPress, Qt::Key_E);
        QVERIFY(g.scene()->player()->heldItem() != nullptr);
    }

    void losingFocusStopsThePlayer()   // Review Focus 2
    {
        GameScreen g;
        sendKey(&g, QEvent::KeyPress, Qt::Key_D);
        QFocusEvent out(QEvent::FocusOut);
        QApplication::sendEvent(&g, &out);
        g.scene()->frameTick();
        QCOMPARE(g.scene()->player()->pos(), QPointF(100, 100));
    }

    void backButtonEmits()
    {
        GameScreen g;
        QSignalSpy back(&g, &GameScreen::backRequested);
        auto *button = g.findChild<ImageButton *>("backButton");
        QVERIFY(button != nullptr);
        QCOMPARE(button->geometry(), QRect(870, 10, 100, 50));
        QTest::mouseClick(button, Qt::LeftButton);
        QCOMPARE(back.count(), 1);
    }

    void stopClearsHeldKeys()   // Review Focus 4
    {
        GameScreen g;
        g.start();
        QVERIFY(g.scene()->isRunning());
        sendKey(&g, QEvent::KeyPress, Qt::Key_D);
        g.stop();
        QVERIFY(!g.scene()->isRunning());
        g.scene()->frameTick();
        QCOMPARE(g.scene()->player()->pos(), QPointF(100, 100));
    }

    // ------------------------------------------------- the controls around the cafe

    void heldItemPanelShowsTheName()
    {
        GameScreen g;
        auto *panel = g.findChild<HeldItemPanel *>();
        QVERIFY(panel != nullptr);
        QCOMPARE(panel->geometry(), QRect(0, 435, 375, 150));
        QCOMPARE(panel->shownName(), QString("Nothing"));

        g.scene()->player()->give(g.scene()->catalog().make("milk"));
        QCOMPARE(panel->shownName(), QString("milk"));
    }

    void panelsToggle()
    {
        GameScreen g;
        auto *restock = g.findChild<RestockingPanel *>();
        auto *orders = g.findChild<OrderPanel *>();
        auto *upgrades = g.findChild<UpgradesPanel *>();
        QCOMPARE(restock->geometry(), QRect(0, 435, 375, 150));
        QCOMPARE(orders->geometry(), QRect(0, 435, 375, 150));
        QCOMPARE(upgrades->geometry(), QRect(300, 150, 400, 400));

        QCOMPARE(g.openPanel(), GameScreen::Panel::None);
        QCOMPARE(g.bannerImage(), QString("itemBanner"));
        QVERIFY(restock->isHidden());
        QVERIFY(orders->isHidden());
        QVERIFY(upgrades->isHidden());

        QTest::mouseClick(g.findChild<ImageButton *>("restockButton"), Qt::LeftButton);
        QCOMPARE(g.openPanel(), GameScreen::Panel::Restock);
        QCOMPARE(g.bannerImage(), QString("restockBanner"));
        QVERIFY(!restock->isHidden());
        QVERIFY(orders->isHidden());

        g.togglePanel(GameScreen::Panel::Orders);       // opening one closes the other
        QVERIFY(restock->isHidden());
        QVERIFY(!orders->isHidden());
        QCOMPARE(g.bannerImage(), QString("ordersBanner"));

        g.togglePanel(GameScreen::Panel::Orders);       // toggling the open one closes it
        QCOMPARE(g.openPanel(), GameScreen::Panel::None);
        QCOMPARE(g.bannerImage(), QString("itemBanner"));
        QVERIFY(orders->isHidden());

        QTest::mouseClick(g.findChild<ImageButton *>("upgradesButton"), Qt::LeftButton);
        QVERIFY(!upgrades->isHidden());
        QCOMPARE(g.bannerImage(), QString("upgradesBanner"));
    }

    void openingAPanelStopsThePlayer()
    {
        GameScreen g;
        sendKey(&g, QEvent::KeyPress, Qt::Key_D);
        QTest::mouseClick(g.findChild<ImageButton *>("ordersButton"), Qt::LeftButton);
        g.scene()->frameTick();
        QCOMPARE(g.scene()->player()->pos(), QPointF(100, 100));
    }

    void keysWorkWithAPanelOpen()   // Review Focus 5
    {
        GameScreen g;
        g.togglePanel(GameScreen::Panel::Restock);
        sendKey(&g, QEvent::KeyPress, Qt::Key_D);
        g.scene()->frameTick();
        QVERIFY(g.scene()->player()->pos().x() > 100);
        QCOMPARE(g.findChild<QComboBox *>("counterCombo")->focusPolicy(), Qt::NoFocus);

        g.togglePanel(GameScreen::Panel::Orders);
        const qreal before = g.scene()->player()->pos().x();
        g.scene()->frameTick();
        QVERIFY(g.scene()->player()->pos().x() > before);
    }

    void upgradesPanelLocksTheControls()   // Open Question Q1: as the Java behaves
    {
        GameScreen g;
        g.scene()->player()->moveTo(250, 100);   // at the cup counter
        g.scene()->frameTick();
        g.togglePanel(GameScreen::Panel::Upgrades);

        sendKey(&g, QEvent::KeyPress, Qt::Key_D);
        sendKey(&g, QEvent::KeyPress, Qt::Key_E);
        g.scene()->frameTick();
        QCOMPARE(g.scene()->player()->pos(), QPointF(250, 100));
        QVERIFY(g.scene()->player()->heldItem() == nullptr);

        g.togglePanel(GameScreen::Panel::Upgrades);   // closed again: keys work
        sendKey(&g, QEvent::KeyPress, Qt::Key_E);
        QVERIFY(g.scene()->player()->heldItem() != nullptr);
    }

    void moneyLabelFollows()
    {
        GameScreen g;
        auto *label = g.findChild<QLabel *>("moneyLabel");
        QVERIFY(label != nullptr);
        QCOMPARE(label->text(), QString("Money: $1000"));
        g.scene()->player()->addMoney(-3);
        QCOMPARE(label->text(), QString("Money: $997"));
    }

    void servingShowsTheDelta()
    {
        GameScreen g;
        CafeScene *s = g.scene();
        auto *delta = g.findChild<QLabel *>("moneyDeltaLabel");
        QVERIFY(delta != nullptr);
        QVERIFY(delta->isHidden());

        Customer *customer = s->seatNewCustomer(s->chairs().at(4));
        for (int i = 0; i < 600 && customer->state() != Customer::State::Seated; ++i)
            s->customerTick();
        s->player()->give(std::make_unique<Cup>(QStringList{"sugar"}));   // not what they ordered
        s->player()->moveTo(640, 20);
        s->frameTick();

        QSignalSpy served(&g, &GameScreen::orderServed);
        sendKey(&g, QEvent::KeyPress, Qt::Key_E);
        QCOMPARE(delta->text(), QString("-$3"));
        QVERIFY(!delta->isHidden());
        QCOMPARE(served.count(), 1);
        QCOMPARE(served.first().at(0).toBool(), false);
        QCOMPARE(g.findChild<QLabel *>("moneyLabel")->text(), QString("Money: $997"));
    }

    void audioIcons()
    {
        GameScreen g;
        auto *sound = g.findChild<ImageButton *>("soundButton");
        auto *music = g.findChild<ImageButton *>("musicButton");
        QCOMPARE(sound->geometry(), QRect(920, 600, 50, 50));
        QCOMPARE(music->geometry(), QRect(850, 600, 50, 50));
        QCOMPARE(sound->currentImage(), QString("soundOn"));
        QCOMPARE(music->currentImage(), QString("musicOn"));

        g.showAudioState(false, false);
        QCOMPARE(sound->currentImage(), QString("soundOff"));
        QCOMPARE(music->currentImage(), QString("musicOff"));
        g.showAudioState(true, false);
        QCOMPARE(sound->currentImage(), QString("soundOn"));
        QCOMPARE(music->currentImage(), QString("musicOff"));
    }

    void toggleButtonsOnlyAsk()
    {
        GameScreen g;
        QSignalSpy sound(&g, &GameScreen::soundToggleRequested);
        QSignalSpy music(&g, &GameScreen::musicToggleRequested);
        QTest::mouseClick(g.findChild<ImageButton *>("soundButton"), Qt::LeftButton);
        QTest::mouseClick(g.findChild<ImageButton *>("musicButton"), Qt::LeftButton);
        QCOMPARE(sound.count(), 1);
        QCOMPARE(music.count(), 1);
        QCOMPARE(g.findChild<ImageButton *>("soundButton")->currentImage(), QString("soundOn"));
    }

    // Not a check of behaviour: writes pictures of the screen so a person can look.
    void screenshots()
    {
        const QString folder = QCoreApplication::applicationDirPath() + "/screenshots";
        QVERIFY(QDir().mkpath(folder));

        GameScreen g;
        CafeScene *s = g.scene();
        s->frameTick();
        QVERIFY(g.grab().save(folder + "/game.png"));

        // A busier moment: two customers waiting, a full cup in paw, a wrong order just served.
        for (int chair : {4, 5, 0}) {
            Customer *customer = s->seatNewCustomer(s->chairs().at(chair));
            for (int i = 0; i < 600 && customer->state() != Customer::State::Seated; ++i)
                s->customerTick();
        }
        s->player()->give(std::make_unique<Cup>(QStringList{"sugar"}));
        s->player()->moveTo(640, 20);
        s->frameTick();
        s->interact();
        s->player()->give(std::make_unique<Cup>(QStringList{"grapefruit", "jelly", "sugar", "ice"}));
        s->player()->moveTo(250, 100);
        s->frameTick();
        QVERIFY(g.grab().save(folder + "/game_busy.png"));

        g.togglePanel(GameScreen::Panel::Restock);
        QVERIFY(g.grab().save(folder + "/game_restock.png"));
        g.togglePanel(GameScreen::Panel::Orders);
        QVERIFY(g.grab().save(folder + "/game_orders.png"));
        g.togglePanel(GameScreen::Panel::Upgrades);
        QVERIFY(g.grab().save(folder + "/game_upgrades.png"));
    }
};

QTEST_MAIN(TestGameScreen)
#include "tst_gamescreen.moc"
