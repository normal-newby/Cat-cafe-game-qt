// Tests for Player: movement, collisions, what it carries, and its money.
#include <QtTest>

#include <QSignalSpy>
#include <memory>

#include "items/Cup.h"
#include "items/ItemCatalog.h"
#include "world/Player.h"

class TestPlayer : public QObject
{
    Q_OBJECT

private slots:
    void startsWhereTheJavaDoes()
    {
        Player p;
        QCOMPARE(p.pos(), QPointF(100, 100));
        QCOMPARE(p.money(), 1000);
        QCOMPARE(p.tips(), 0);
        QCOMPARE(p.speed(), 2.2);
        QVERIFY(p.heldItem() == nullptr);
        QCOMPARE(p.zValue(), 1.0);
        QCOMPARE(p.bodyRect(), QRect(105, 105, 30, 30));
        QCOMPARE(p.reachRect(), QRect(102, 102, 44, 44));
        QCOMPARE(p.pixmap().size(), QSize(40, 40));
    }

    void movesRight()
    {
        Player p;
        p.setKeyHeld(Qt::Key_D, true);
        for (int i = 0; i < 10; ++i)
            p.step({});
        QCOMPARE(p.pos(), QPointF(122, 100));
    }

    void diagonalIsNormalised()
    {
        Player p;
        p.setKeyHeld(Qt::Key_D, true);
        p.setKeyHeld(Qt::Key_S, true);
        for (int i = 0; i < 10; ++i)
            p.step({});
        QCOMPARE(p.pos(), QPointF(116, 116));   // 10 * 2.2 / sqrt(2) = 15.56 each way
    }

    void opposingKeysCancel()
    {
        Player p;
        p.setKeyHeld(Qt::Key_A, true);
        p.setKeyHeld(Qt::Key_D, true);
        p.step({});
        QCOMPARE(p.pos(), QPointF(100, 100));
    }

    void releasingAKeyStopsThatDirection()
    {
        Player p;
        p.setKeyHeld(Qt::Key_W, true);
        p.step({});
        p.setKeyHeld(Qt::Key_W, false);
        p.step({});
        QCOMPARE(p.pos(), QPointF(100, 98));
    }

    void stopsAtSolids()
    {
        Player p;
        p.setKeyHeld(Qt::Key_D, true);
        for (int i = 0; i < 20; ++i)
            p.step({QRect(140, 90, 50, 60)});
        QCOMPARE(p.pos(), QPointF(104, 100));
    }

    void staysInBounds()
    {
        Player a;
        a.setKeyHeld(Qt::Key_A, true);
        for (int i = 0; i < 100; ++i)
            a.step({});
        QCOMPARE(a.pos(), QPointF(1, 100));

        Player w;
        w.setKeyHeld(Qt::Key_W, true);
        for (int i = 0; i < 100; ++i)
            w.step({});
        QCOMPARE(w.pos(), QPointF(100, 1));

        Player s;
        s.setKeyHeld(Qt::Key_S, true);
        for (int i = 0; i < 400; ++i)
            s.step({});
        QCOMPARE(s.pos(), QPointF(100, 619));

        Player d;
        d.setKeyHeld(Qt::Key_D, true);
        for (int i = 0; i < 600; ++i)
            d.step({});
        QCOMPARE(d.pos(), QPointF(958, 100));
    }

    void stopReleasesEverything()
    {
        Player p;
        p.setKeyHeld(Qt::Key_D, true);
        p.stop();
        p.step({});
        QCOMPARE(p.pos(), QPointF(100, 100));
    }

    void moveTo()
    {
        Player p;
        p.moveTo(500, 500);
        QCOMPARE(p.pos(), QPointF(500, 500));
        QCOMPARE(p.reachRect(), QRect(502, 502, 44, 44));
    }

    void carriesOneItem()
    {
        Player p;
        ItemCatalog catalog;
        QSignalSpy changed(&p, &Player::heldItemChanged);

        p.give(catalog.make("milk"));
        QCOMPARE(changed.count(), 1);

        QVERIFY(p.takeCup() == nullptr);          // milk is not a cup: nothing happens
        QVERIFY(p.heldItem() != nullptr);
        QCOMPARE(changed.count(), 1);

        std::unique_ptr<Item> milk = p.takeItem();
        QCOMPARE(milk->name(), QString("milk"));
        QVERIFY(p.heldItem() == nullptr);
        QCOMPARE(changed.count(), 2);

        p.give(std::make_unique<Cup>(QStringList{"milk"}));
        std::unique_ptr<Cup> cup = p.takeCup();
        QVERIFY(cup != nullptr);
        QCOMPARE(cup->ingredients(), QStringList{"milk"});
        QVERIFY(p.heldItem() == nullptr);
        QCOMPARE(changed.count(), 4);

        QVERIFY(p.takeItem() == nullptr);         // empty hands: nothing to hand over
        QCOMPARE(changed.count(), 4);
    }

    void moneyTipsSpeedAnnounce()
    {
        Player p;
        QSignalSpy money(&p, &Player::moneyChanged);
        QSignalSpy tips(&p, &Player::tipsChanged);

        p.addMoney(-3);
        QCOMPARE(p.money(), 997);
        QCOMPARE(money.last().at(0).toInt(), 997);

        p.addTip();
        QCOMPARE(p.tips(), 1);
        QCOMPARE(tips.last().at(0).toInt(), 1);

        p.addSpeed(0.3);
        QCOMPARE(p.speed(), 2.5);
    }
};

QTEST_MAIN(TestPlayer)
#include "tst_player.moc"
