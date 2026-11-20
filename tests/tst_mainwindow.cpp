// Tests for MainWindow: does clicking around take you to the right screen?
#include <QtTest>

#include <QEnterEvent>
#include <QLoggingCategory>
#include <QSignalSpy>
#include <memory>

#include "MainWindow.h"
#include "audio/SoundManager.h"
#include "items/Cup.h"
#include "world/Chair.h"
#include "world/Customer.h"
#include "screens/GameScreen.h"
#include "screens/InstructionsScreen.h"
#include "screens/MenuScreen.h"
#include "ui/ImageButton.h"
#include "world/CafeScene.h"
#include "world/Player.h"

class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // These tests destroy a window a few milliseconds after starting its music. The
        // music player then reports that loading was cut short, which is true and harmless.
        QLoggingCategory::setFilterRules("qt.multimedia.ffmpeg.mediadataholder.warning=false");
    }

    void startsOnMenu()
    {
        MainWindow w;
        QCOMPARE(w.size(), QSize(1000, 665));
        QCOMPARE(w.windowTitle(), QString("Cat Cafe"));
        QVERIFY(qobject_cast<MenuScreen *>(w.currentScreen()) != nullptr);
    }

    void instructionsAndBack()
    {
        MainWindow w;
        QTest::mouseClick(w.findChild<ImageButton *>("instructionsButton"), Qt::LeftButton);

        auto *instructions = qobject_cast<InstructionsScreen *>(w.currentScreen());
        QVERIFY(instructions != nullptr);
        QVERIFY(instructions->isRunning());

        QTest::mouseClick(instructions->findChild<ImageButton *>("backButton"), Qt::LeftButton);
        QVERIFY(qobject_cast<MenuScreen *>(w.currentScreen()) != nullptr);
        QVERIFY(!instructions->isRunning());
    }

    void playBackPlay()   // Review Focus 4: the same game carries on
    {
        MainWindow w;
        QTest::mouseClick(w.findChild<ImageButton *>("playButton"), Qt::LeftButton);
        auto *game = qobject_cast<GameScreen *>(w.currentScreen());
        QVERIFY(game != nullptr);
        QVERIFY(game->scene()->isRunning());
        game->scene()->player()->addMoney(-1);

        QTest::mouseClick(game->findChild<ImageButton *>("backButton"), Qt::LeftButton);
        QVERIFY(qobject_cast<MenuScreen *>(w.currentScreen()) != nullptr);
        QVERIFY(!game->scene()->isRunning());

        QTest::mouseClick(w.findChild<ImageButton *>("playButton"), Qt::LeftButton);
        QVERIFY(qobject_cast<GameScreen *>(w.currentScreen()) == game);
        QVERIFY(game->scene()->isRunning());
        QCOMPARE(game->scene()->player()->money(), 999);
    }

    void musicFollowsTheScreens()
    {
        MainWindow w;
        QVERIFY(!w.sounds()->isMusicPlaying());

        QTest::mouseClick(w.findChild<ImageButton *>("playButton"), Qt::LeftButton);
        QVERIFY(w.sounds()->isMusicPlaying());

        // Two screens have a "backButton", so ask the game screen for its own buttons.
        auto *game = qobject_cast<GameScreen *>(w.currentScreen());
        QTest::mouseClick(game->findChild<ImageButton *>("soundButton"), Qt::LeftButton);
        QVERIFY(!w.sounds()->soundOn());
        QVERIFY(!w.sounds()->isMusicPlaying());
        QCOMPARE(game->findChild<ImageButton *>("soundButton")->currentImage(), QString("soundOff"));
        QCOMPARE(game->findChild<ImageButton *>("musicButton")->currentImage(), QString("musicOff"));

        QTest::mouseClick(game->findChild<ImageButton *>("soundButton"), Qt::LeftButton);
        QVERIFY(w.sounds()->isMusicPlaying());
        QCOMPARE(game->findChild<ImageButton *>("musicButton")->currentImage(), QString("musicOn"));

        QTest::mouseClick(game->findChild<ImageButton *>("musicButton"), Qt::LeftButton);
        QVERIFY(!w.sounds()->isMusicPlaying());
        QCOMPARE(game->findChild<ImageButton *>("musicButton")->currentImage(), QString("musicOff"));
        QTest::mouseClick(game->findChild<ImageButton *>("musicButton"), Qt::LeftButton);
        QVERIFY(w.sounds()->isMusicPlaying());

        QTest::mouseClick(game->findChild<ImageButton *>("backButton"), Qt::LeftButton);
        QVERIFY(!w.sounds()->isMusicPlaying());
    }

    void everyImageButtonClicksOnHover()   // fix F10: wired in one place, for all of them
    {
        MainWindow w;
        QSignalSpy played(w.sounds(), &SoundManager::effectPlayed);

        // 2 on the menu, 1 on the instructions, 6 around the game, 5 in Restock, 4 in Upgrades.
        const QList<ImageButton *> buttons = w.findChildren<ImageButton *>();
        QCOMPARE(buttons.size(), 18);
        for (ImageButton *button : buttons) {
            QEnterEvent enter(QPointF(2, 2), QPointF(2, 2), QPointF(2, 2));
            QApplication::sendEvent(button, &enter);
        }
        QCOMPARE(played.count(), buttons.size());
        QCOMPARE(played.first().at(0).toString(), QString("click"));
    }

    void servingMeows()
    {
        MainWindow w;
        QTest::mouseClick(w.findChild<ImageButton *>("playButton"), Qt::LeftButton);
        auto *game = qobject_cast<GameScreen *>(w.currentScreen());
        CafeScene *s = game->scene();

        Customer *customer = s->seatNewCustomer(s->chairs().at(4));
        for (int i = 0; i < 600 && customer->state() != Customer::State::Seated; ++i)
            s->customerTick();
        s->player()->give(std::make_unique<Cup>(customer->order().ingredients()));
        s->player()->moveTo(640, 20);
        s->frameTick();

        QSignalSpy played(w.sounds(), &SoundManager::effectPlayed);
        s->interact();
        QCOMPARE(played.count(), 1);
        QCOMPARE(played.first().at(0).toString(), QString("meowHappy"));
    }
};

QTEST_MAIN(TestMainWindow)
#include "tst_mainwindow.moc"
