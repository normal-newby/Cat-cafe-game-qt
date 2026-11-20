// Tests for CafeScene: the cafe as a whole. The timers are not started here; each test
// calls the tick functions itself, so nothing depends on real time.
#include <QtTest>

#include <QEventLoop>
#include <QSignalSpy>
#include <QTimer>
#include <memory>

#include "items/Cup.h"
#include "world/CafeScene.h"
#include "world/Chair.h"
#include "world/Counter.h"
#include "world/Customer.h"
#include "world/Player.h"

class TestCafeScene : public QObject
{
    Q_OBJECT

    // Sends a new customer to `chair` and runs the customer clock until it has sat down.
    static Customer *seat(CafeScene &scene, Chair *chair)
    {
        Customer *customer = scene.seatNewCustomer(chair);
        for (int i = 0; i < 600 && customer->state() != Customer::State::Seated; ++i)
            scene.customerTick();
        return customer;
    }

    // Runs a real event loop for a while, exactly as app.exec() would. (QTest::qWait is
    // not used here: it naps for 10 ms between rounds, which holds back a 10 ms timer.)
    static void runEventLoopFor(int milliseconds)
    {
        QEventLoop loop;
        QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
        loop.exec();
    }

private slots:
    void worldIsBuilt()
    {
        CafeScene s;
        QCOMPARE(s.sceneRect(), QRectF(0, 0, 1000, 665));
        QCOMPARE(s.counters().size(), 10);
        QCOMPARE(s.chairs().size(), 8);
        QCOMPARE(s.interactables().size(), 22);
        QCOMPARE(s.solidRects().size(), 29);

        QCOMPARE(s.findCounter("cup")->stock(), 3);
        QCOMPARE(s.findCounter("sugar")->stock(), 2);
        QCOMPARE(s.findCounter("ice")->stock(), 2);
        QCOMPARE(s.findCounter("boba")->stock(), 1);
        QVERIFY(s.findCounter("tea") == nullptr);

        QVERIFY(s.solidRects().contains(QRect(448, 75, 4, 275)));     // a wall
        QVERIFY(s.solidRects().contains(QRect(0, 423, 450, 4)));      // another wall
        QVERIFY(s.solidRects().contains(QRect(575, 475, 400, 100)));  // the long table
        QVERIFY(s.solidRects().contains(QRect(585, 398, 55, 60)));    // the first chair

        QCOMPARE(s.interactables().at(0)->rect(), QRect(235, 150, 75, 75));    // cup counter
        QCOMPARE(s.interactables().at(10)->rect(), QRect(150, 150, 75, 75));   // first prep table
        QCOMPARE(s.interactables().at(12)->rect(), QRect(0, 325, 50, 50));     // first bin
        QCOMPARE(s.interactables().at(14)->rect(), QRect(575, 385, 75, 75));   // first chair
        QCOMPARE(s.chairs().at(5)->rect(), QRect(800, 62, 75, 75));
        QCOMPARE(s.chairs().at(5)->route(), QList<QPoint>({QPoint(980, 180), QPoint(850, 180), QPoint(850, 100)}));
        QVERIFY(s.chairs().at(5)->facesRight());
        QVERIFY(!s.chairs().at(4)->facesRight());

        QCOMPARE(s.player()->pos(), QPointF(100, 100));
        QCOMPARE(s.customerCount(), 0);
    }

    void startingSpotIsInReachOfThePrepTable()
    {
        CafeScene s;
        s.frameTick();
        QVERIFY(s.target() == s.interactables().at(10));
    }

    void nothingInReach()
    {
        CafeScene s;
        s.player()->moveTo(500, 500);
        s.frameTick();
        QVERIFY(s.target() == nullptr);
    }

    void popUpFollowsTheTarget()
    {
        CafeScene s;
        s.frameTick();

        QGraphicsItem *popUp = nullptr;
        for (QGraphicsItem *item : s.items()) {
            if (item->zValue() == 4)
                popUp = item;
        }
        QVERIFY(popUp != nullptr);
        QVERIFY(popUp->isVisible());
        QCOMPARE(popUp->pos(), QPointF(162, 100));   // above the first prep table

        s.player()->moveTo(500, 500);
        s.frameTick();
        QVERIFY(!popUp->isVisible());
    }

    void interactWithNothingIsHarmless()   // Review Focus 5
    {
        CafeScene s;
        s.player()->moveTo(500, 500);
        s.frameTick();
        QSignalSpy stock(&s, &CafeScene::stockChanged);
        QSignalSpy served(&s, &CafeScene::orderServed);
        s.interact();
        QCOMPARE(stock.count(), 0);
        QCOMPARE(served.count(), 0);
        QVERIFY(s.player()->heldItem() == nullptr);
    }

    void emptyChairDoesNotBlockItsNeighbour()   // fix F5
    {
        CafeScene s;
        s.player()->moveTo(630, 340);   // in reach of the first two long-table chairs
        s.frameTick();
        QVERIFY(s.target() == nullptr); // both empty: nothing to use

        seat(s, s.chairs().at(1));
        s.frameTick();
        QVERIFY(s.target() == s.chairs().at(1));   // the empty first chair no longer hides it
    }

    void takingFromACounter()
    {
        CafeScene s;
        s.player()->moveTo(250, 100);
        s.frameTick();
        QVERIFY(s.target() == s.interactables().at(0));

        QSignalSpy stock(&s, &CafeScene::stockChanged);
        s.interact();
        QCOMPARE(stock.count(), 1);
        QCOMPARE(s.findCounter("cup")->stock(), 2);
        QVERIFY(s.player()->heldItem() != nullptr);
    }

    void emptyCounterIsReported()
    {
        CafeScene s;
        s.player()->moveTo(160, 45);    // at the boba counter, which starts with one
        s.frameTick();
        QSignalSpy empty(&s, &CafeScene::counterEmpty);

        s.interact();
        QCOMPARE(empty.count(), 0);
        s.player()->takeItem();         // put it down somewhere, as far as the test cares
        s.interact();
        QCOMPARE(empty.count(), 1);
        QVERIFY(s.player()->heldItem() == nullptr);
    }

    void restockAnnounces()
    {
        CafeScene s;
        QSignalSpy stock(&s, &CafeScene::stockChanged);
        s.restock(s.findCounter("boba"), 9);
        QCOMPARE(s.findCounter("boba")->stock(), 10);
        QCOMPARE(stock.count(), 1);
    }

    void seatingAnnouncesAndNumbers()
    {
        CafeScene s;
        QSignalSpy orders(&s, &CafeScene::ordersChanged);
        Customer *a = seat(s, s.chairs().at(5));
        Customer *b = seat(s, s.chairs().at(4));
        QCOMPARE(orders.count(), 2);
        QCOMPARE(s.seatedCustomers(), QList<Customer *>({a, b}));
        QCOMPARE(a->number(), 1);
        QCOMPARE(b->number(), 2);
        QCOMPARE(a->orderCost(), s.catalog().orderCost(a->order()));
        QCOMPARE(s.customerCount(), 2);
    }

    void servingAnnouncesRenumbersAndCleansUp()
    {
        CafeScene s;
        Customer *a = seat(s, s.chairs().at(4));
        Customer *b = seat(s, s.chairs().at(5));
        const int pay = a->payout(0);

        s.player()->give(std::make_unique<Cup>(a->order().ingredients()));
        s.player()->moveTo(640, 20);
        s.frameTick();
        QVERIFY(s.target() == s.chairs().at(4));

        QSignalSpy served(&s, &CafeScene::orderServed);
        QSignalSpy orders(&s, &CafeScene::ordersChanged);
        s.interact();

        QCOMPARE(served.count(), 1);
        QCOMPARE(served.first().at(0).toBool(), true);
        QCOMPARE(served.first().at(1).toInt(), pay);
        QCOMPARE(orders.count(), 1);
        QCOMPARE(s.player()->money(), 1000 + pay);
        QCOMPARE(s.seatedCustomers(), QList<Customer *>({b}));
        QCOMPARE(b->number(), 1);
        QCOMPARE(s.customerCount(), 2);   // a is still walking out
        QVERIFY(s.target() == nullptr);   // the chair is empty again

        for (int i = 0; i < 500; ++i)
            s.customerTick();
        QCOMPARE(s.customerCount(), 1);   // a has gone and was removed
    }

    void wrongOrderAnnouncesALoss()
    {
        CafeScene s;
        seat(s, s.chairs().at(4));
        s.player()->give(std::make_unique<Cup>(QStringList{"sugar"}));
        s.player()->moveTo(640, 20);
        s.frameTick();

        QSignalSpy served(&s, &CafeScene::orderServed);
        s.interact();
        QCOMPARE(served.count(), 1);
        QCOMPARE(served.first().at(0).toBool(), false);
        QCOMPARE(served.first().at(1).toInt(), -3);
        QCOMPARE(s.player()->money(), 997);
    }

    void spawnTickSeatsSomeone()
    {
        CafeScene s;
        // The odds improve with every roll and are certain by the tenth.
        for (int i = 0; i < 12 && s.customerCount() == 0; ++i)
            s.spawnTick();
        QCOMPARE(s.customerCount(), 1);
        QCOMPARE(s.spawnOdds().chance(), 100);
    }

    void noSpawnWhenFull()
    {
        CafeScene s;
        for (Chair *chair : s.chairs())
            s.seatNewCustomer(chair);
        for (int i = 0; i < 50; ++i)
            s.spawnTick();
        QCOMPARE(s.customerCount(), 8);
    }

    void startStop()
    {
        CafeScene s;
        QVERIFY(!s.isRunning());
        s.start();
        s.start();
        QVERIFY(s.isRunning());
        s.stop();
        QVERIFY(!s.isRunning());
    }

    // The one test that uses real time: do the real clocks drive the game at the
    // intended pace? Every other test calls the tick functions itself.
    void clocksDriveTheGame()
    {
        CafeScene s;
        s.player()->setKeyHeld(Qt::Key_D, true);
        Customer *customer = s.seatNewCustomer(s.chairs().at(5));

        s.start();
        runEventLoopFor(1000);

        // One second is about 62 ticks of 16 ms, each moving the cat 2.2 px: 100 -> ~237.
        const qreal catX = s.player()->pos().x();
        QVERIFY2(catX > 220 && catX < 250, qPrintable(QString("the cat is at x = %1").arg(catX)));

        // ...and about 100 ticks of 10 ms, each moving the customer 1 px left: 980 -> ~881.
        const qreal customerX = customer->pos().x();
        QVERIFY2(customerX > 870 && customerX < 892,
                 qPrintable(QString("the customer is at x = %1").arg(customerX)));

        // The whole walk to this chair is 212 ticks, 2.12 s.
        runEventLoopFor(1400);
        QVERIFY(customer->state() == Customer::State::Seated);
        s.stop();
    }
};

QTEST_MAIN(TestCafeScene)
#include "tst_cafescene.moc"
