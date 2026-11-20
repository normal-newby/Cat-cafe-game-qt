// Tests for customers, the chairs they sit on, and the odds of a new one arriving.
#include <QtTest>

#include <cmath>
#include <memory>

#include "items/Cup.h"
#include "items/ItemCatalog.h"
#include "world/Chair.h"
#include "world/Customer.h"
#include "world/Player.h"
#include "world/SpawnOdds.h"

class TestCustomers : public QObject
{
    Q_OBJECT

    ItemCatalog catalog;
    CustomerSprites sprites;
    const Cup order{QStringList{"milk", "boba"}};   // costs 1 + 2 + 2 = 5

    // The top-right chair of the game and its route from the door.
    static QList<QPoint> route() { return {QPoint(980, 180), QPoint(850, 180), QPoint(850, 100)}; }
    static QRect chairRect() { return QRect(800, 62, 75, 75); }

    // 1 tick to notice it is on the first point, 130 px left, 80 px up, 1 tick to sit.
    static void walkIn(Customer &c)
    {
        for (int i = 0; i < 212; ++i)
            c.step();
    }

private slots:
    void chairGeometry()
    {
        Chair chair("chairRight", chairRect(), 10, 10, PopUpSide::Up, true, route());
        QCOMPARE(chair.solidRect(), QRect(810, 75, 55, 60));
        QCOMPARE(chair.reachRect(), QRect(790, 52, 95, 95));
        QVERIFY(chair.facesRight());
        QCOMPARE(chair.route(), route());
        QVERIFY(chair.isFree());
        QVERIFY(!chair.canInteract());
    }

    void walksInAndSits()
    {
        Chair chair("chairRight", chairRect(), 10, 10, PopUpSide::Up, true, route());
        Customer c(order, 5, chair, sprites);
        chair.assign(&c);
        QCOMPARE(c.pos(), QPointF(980, 180));
        QCOMPARE(c.zValue(), 3.0);
        QCOMPARE(c.pixmap().size(), QSize(40, 40));
        QCOMPARE(c.state(), Customer::State::Arriving);
        QVERIFY(!chair.isFree());
        QCOMPARE(chair.customer(), &c);

        for (int i = 0; i < 211; ++i)
            c.step();
        QCOMPARE(c.state(), Customer::State::Arriving);
        QCOMPARE(c.pos(), QPointF(850, 100));
        QVERIFY(!chair.canInteract());

        c.step();
        QCOMPARE(c.state(), Customer::State::Seated);
        QCOMPARE(c.pos(), QPointF(800, 62));
        QCOMPARE(c.pixmap().size(), QSize(75, 75));
        QVERIFY(chair.canInteract());
    }

    void walksOutAndGoes()
    {
        Chair chair("chairRight", chairRect(), 10, 10, PopUpSide::Up, true, route());
        Customer c(order, 5, chair, sprites);
        chair.assign(&c);
        walkIn(c);

        c.leave();
        QCOMPARE(c.state(), Customer::State::Leaving);
        QCOMPARE(c.pos(), QPointF(850, 100));
        QCOMPARE(c.pixmap().size(), QSize(40, 40));

        for (int i = 0; i < 210; ++i)
            c.step();
        QCOMPARE(c.state(), Customer::State::Leaving);

        c.step();
        QCOMPARE(c.state(), Customer::State::Gone);
        QCOMPARE(c.pos(), QPointF(980, 180));
    }

    void payout()
    {
        Chair chair("chairRight", chairRect(), 10, 10, PopUpSide::Up, true, route());
        Customer a(order, 5, chair, sprites);
        QCOMPARE(a.orderCost(), 5);
        QCOMPARE(a.payout(0), 7);    // 7.5 rounded down
        QCOMPARE(a.payout(2), 9);
        Customer b(order, 7, chair, sprites);
        QCOMPARE(b.payout(0), 10);   // 10.5 rounded down
        QCOMPARE(a.order().ingredients(), QStringList({"milk", "boba"}));
    }

    void rightCupPays()
    {
        Player player;
        Chair chair("chairRight", chairRect(), 10, 10, PopUpSide::Up, true, route());
        Customer c(order, 5, chair, sprites);
        chair.assign(&c);
        walkIn(c);

        player.give(std::make_unique<Cup>(QStringList{"boba", "milk"}));
        QCOMPARE(chair.interact(player), Outcome::ServedRight);
        QCOMPARE(player.money(), 1007);
        QVERIFY(player.heldItem() == nullptr);
        QVERIFY(chair.isFree());
        QVERIFY(!chair.canInteract());
        QCOMPARE(c.state(), Customer::State::Leaving);
    }

    void wrongCupCosts()
    {
        Player player;
        Chair chair("chairRight", chairRect(), 10, 10, PopUpSide::Up, true, route());
        Customer c(order, 5, chair, sprites);
        chair.assign(&c);
        walkIn(c);

        player.give(std::make_unique<Cup>(QStringList{"milk"}));
        QCOMPARE(chair.interact(player), Outcome::ServedWrong);
        QCOMPARE(player.money(), 997);
        QVERIFY(player.heldItem() == nullptr);
        QVERIFY(chair.isFree());
        QCOMPARE(c.state(), Customer::State::Leaving);
    }

    void tipsAreAdded()
    {
        Player player;
        player.addTip();
        player.addTip();
        Chair chair("chairRight", chairRect(), 10, 10, PopUpSide::Up, true, route());
        Customer c(order, 5, chair, sprites);
        chair.assign(&c);
        walkIn(c);

        player.give(std::make_unique<Cup>(QStringList{"milk", "boba"}));
        QCOMPARE(chair.interact(player), Outcome::ServedRight);
        QCOMPARE(player.money(), 1009);
    }

    void chairRefuses()
    {
        Player player;
        Chair chair("chairRight", chairRect(), 10, 10, PopUpSide::Up, true, route());
        Customer c(order, 5, chair, sprites);
        chair.assign(&c);

        // Still walking in: even the right cup is refused.
        player.give(std::make_unique<Cup>(QStringList{"milk", "boba"}));
        QCOMPARE(chair.interact(player), Outcome::Nothing);
        QVERIFY(player.heldItem() != nullptr);
        player.takeItem();

        walkIn(c);
        QCOMPARE(chair.interact(player), Outcome::Nothing);      // empty paws

        player.give(catalog.make("milk"));
        QCOMPARE(chair.interact(player), Outcome::Nothing);      // an ingredient is not a drink
        QVERIFY(player.heldItem() != nullptr);
        QCOMPARE(c.state(), Customer::State::Seated);
        QCOMPARE(player.money(), 1000);
    }

    void freedChairIsReusedWhileTheOldCustomerLeaves()   // fix F1
    {
        Player player;
        Chair chair("chairRight", chairRect(), 10, 10, PopUpSide::Up, true, route());
        Customer first(order, 5, chair, sprites);
        chair.assign(&first);
        walkIn(first);
        player.give(std::make_unique<Cup>(QStringList{"milk", "boba"}));
        chair.interact(player);
        QVERIFY(chair.isFree());

        Customer second(order, 5, chair, sprites);
        chair.assign(&second);
        for (int i = 0; i < 211; ++i) {
            first.step();
            second.step();
        }
        QCOMPARE(first.state(), Customer::State::Gone);          // the old one made it out
        QCOMPARE(first.pos(), QPointF(980, 180));
        QCOMPARE(second.state(), Customer::State::Arriving);     // the new one is at the chair
        QCOMPARE(second.pos(), QPointF(850, 100));

        second.step();
        QCOMPARE(second.state(), Customer::State::Seated);
        QVERIFY(chair.canInteract());
    }

    void numberBubble()
    {
        Chair chair("chairRight", chairRect(), 10, 10, PopUpSide::Up, true, route());
        Customer c(order, 5, chair, sprites);
        chair.assign(&c);
        QCOMPARE(c.childItems().size(), 1);
        QGraphicsItem *bubble = c.childItems().first();

        c.setNumber(3);
        QCOMPARE(c.number(), 3);
        QVERIFY(!bubble->isVisible());      // not while walking in

        walkIn(c);
        QVERIFY(bubble->isVisible());
        QCOMPARE(bubble->pos(), QPointF(0, -50));

        c.leave();
        QVERIFY(!bubble->isVisible());
    }

    void spawnOdds()
    {
        SpawnOdds o;
        QCOMPARE(o.chance(), 10);
        QCOMPARE(o.divisor(), 1.0);

        QVERIFY(!o.roll(0.5));    // int(0.5 * 10) = 5
        QCOMPARE(o.chance(), 9);
        QVERIFY(o.roll(0.05));    // int(0.05 * 9) = 0
        QCOMPARE(o.chance(), 8);

        o.customerSpawned(0);
        QCOMPARE(o.chance(), 100);
        o.customerSpawned(2);
        QCOMPARE(o.chance(), 300);

        o.setAdvertisementLevel(1);   // fix F2: the first level already helps
        QCOMPARE(o.divisor(), 1.1);
        o.setAdvertisementLevel(5);
        QCOMPARE(o.divisor(), 1 + std::pow(5, 1.1) / 10);

        SpawnOdds z;
        for (int i = 0; i < 11; ++i)
            z.roll(0.99);             // 10, 9, 8 ... decays to 0
        QCOMPARE(z.chance(), 0);
        QVERIFY(z.roll(0.99));        // at 0 a customer always comes
    }
};

QTEST_MAIN(TestCustomers)
#include "tst_customers.moc"
