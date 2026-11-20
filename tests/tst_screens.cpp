// Tests for the menu and instructions screens.
#include <QtTest>

#include <QSignalSpy>

#include "screens/InstructionsScreen.h"
#include "screens/MenuScreen.h"
#include "ui/ImageButton.h"

class TestScreens : public QObject
{
    Q_OBJECT

private slots:
    void menuButtonsEmit()
    {
        MenuScreen menu;
        QCOMPARE(menu.size(), QSize(1000, 665));

        QSignalSpy play(&menu, &MenuScreen::playRequested);
        QSignalSpy instructions(&menu, &MenuScreen::instructionsRequested);

        auto *playButton = menu.findChild<ImageButton *>("playButton");
        auto *instructionsButton = menu.findChild<ImageButton *>("instructionsButton");
        QVERIFY(playButton != nullptr);
        QVERIFY(instructionsButton != nullptr);
        QCOMPARE(playButton->geometry(), QRect(430, 535, 150, 50));
        QCOMPARE(instructionsButton->geometry(), QRect(430, 600, 150, 50));

        QTest::mouseClick(playButton, Qt::LeftButton);
        QCOMPARE(play.count(), 1);
        QCOMPARE(instructions.count(), 0);

        QTest::mouseClick(instructionsButton, Qt::LeftButton);
        QCOMPARE(instructions.count(), 1);
    }

    void instructionsLoop()
    {
        InstructionsScreen s;
        QCOMPARE(s.size(), QSize(1000, 665));
        QCOMPARE(s.customerY(), 200);
        QVERIFY(s.customerVisible());
        QVERIFY(!s.chairOccupied());

        for (int i = 0; i < 50; ++i)
            s.advance();
        QCOMPARE(s.customerY(), 300);
        QVERIFY(s.customerVisible());
        QVERIFY(!s.chairOccupied());

        s.advance();   // 302 is past the chair: the customer sits
        QCOMPARE(s.customerY(), 200);
        QVERIFY(!s.customerVisible());
        QVERIFY(s.chairOccupied());

        for (int i = 0; i < 33; ++i)   // about one second of sitting
            s.advance();
        QCOMPARE(s.customerY(), 200);
        QVERIFY(s.chairOccupied());

        s.advance();   // and the next customer sets off
        QCOMPARE(s.customerY(), 202);
        QVERIFY(s.customerVisible());
        QVERIFY(!s.chairOccupied());
    }

    void instructionsStartStop()
    {
        InstructionsScreen s;
        QVERIFY(!s.isRunning());
        s.start();
        QVERIFY(s.isRunning());
        s.stop();
        QVERIFY(!s.isRunning());
    }

    void instructionsBackEmits()
    {
        InstructionsScreen s;
        QSignalSpy back(&s, &InstructionsScreen::backRequested);
        auto *backButton = s.findChild<ImageButton *>("backButton");
        QVERIFY(backButton != nullptr);
        QCOMPARE(backButton->geometry(), QRect(870, 10, 100, 50));
        QTest::mouseClick(backButton, Qt::LeftButton);
        QCOMPARE(back.count(), 1);
    }

    // Not a check of behaviour: this writes pictures of the screens so a person can look.
    void screenshots()
    {
        const QString folder = QCoreApplication::applicationDirPath() + "/screenshots";
        QVERIFY(QDir().mkpath(folder));

        MenuScreen menu;
        QVERIFY(menu.grab().save(folder + "/menu.png"));

        InstructionsScreen instructions;
        for (int i = 0; i < 25; ++i)
            instructions.advance();
        QVERIFY(instructions.grab().save(folder + "/instructions.png"));
    }
};

QTEST_MAIN(TestScreens)
#include "tst_screens.moc"
