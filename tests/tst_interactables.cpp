// Tests for the things in the cafe the player can bump into and use:
// decorations, counters, prep tables and bins.
#include <QtTest>

#include <memory>

#include "items/Cup.h"
#include "items/ItemCatalog.h"
#include "world/Counter.h"
#include "world/Decoration.h"
#include "world/Player.h"
#include "world/PrepTable.h"
#include "world/Trashbin.h"

class TestInteractables : public QObject
{
    Q_OBJECT

    ItemCatalog catalog;

private slots:
    void geometry()
    {
        Counter cup("cupCounter", QRect(235, 150, 75, 75), 10, 10, PopUpSide::Up, catalog, "cup", 3);
        QCOMPARE(cup.pos(), QPointF(235, 150));
        QCOMPARE(cup.pixmap().size(), QSize(75, 75));
        QCOMPARE(cup.zValue(), 2.0);
        QCOMPARE(cup.rect(), QRect(235, 150, 75, 75));
        QCOMPARE(cup.solidRect(), QRect(235, 150, 75, 75));
        QCOMPARE(cup.reachRect(), QRect(225, 140, 95, 95));
        QCOMPARE(cup.popUpRect(), QRect(247, 100, 50, 50));
        QCOMPARE(cup.popUpSide(), PopUpSide::Up);
        QVERIFY(cup.canInteract());

        Counter boba("bobaCounter", QRect(150, 0, 75, 75), 0, 10, PopUpSide::Down, catalog, "boba", 1);
        QCOMPARE(boba.reachRect(), QRect(150, -10, 75, 95));
        QCOMPARE(boba.popUpRect(), QRect(162, 75, 50, 50));

        Counter ice("iceCounter", QRect(0, 225, 75, 75), 10, 0, PopUpSide::Right, catalog, "ice", 2);
        QCOMPARE(ice.popUpRect(), QRect(75, 237, 50, 50));

        Trashbin left("trashbin", QRect(200, 200, 50, 50), 10, 10, PopUpSide::Left);
        QCOMPARE(left.popUpRect(), QRect(150, 200, 50, 50));

        Decoration wall("wall", QRect(448, 75, 4, 275));
        QCOMPARE(wall.solidRect(), QRect(448, 75, 4, 275));
        QCOMPARE(wall.pixmap().size(), QSize(4, 275));
    }

    void counterHandsOutStock()
    {
        Player player;
        Counter c("cupCounter", QRect(235, 150, 75, 75), 10, 10, PopUpSide::Up, catalog, "cup", 3);
        QCOMPARE(c.itemName(), QString("cup"));

        QCOMPARE(c.interact(player), Outcome::TookItem);
        QCOMPARE(c.stock(), 2);
        QVERIFY(dynamic_cast<Cup *>(player.heldItem()) != nullptr);

        QCOMPARE(c.interact(player), Outcome::Nothing);   // paws are full
        QCOMPARE(c.stock(), 2);

        c.addStock(9);
        QCOMPARE(c.stock(), 11);
    }

    void counterMakesIngredients()
    {
        Player player;
        Counter c("milkCounter", QRect(225, 0, 75, 75), 0, 10, PopUpSide::Down, catalog, "milk", 1);
        QCOMPARE(c.interact(player), Outcome::TookItem);
        QCOMPARE(player.heldItem()->name(), QString("milk"));
        QVERIFY(dynamic_cast<Cup *>(player.heldItem()) == nullptr);
        QCOMPARE(c.stock(), 0);
    }

    void emptyCounterSaysSo()
    {
        Player player;
        Counter c("milkCounter", QRect(225, 0, 75, 75), 0, 10, PopUpSide::Down, catalog, "milk", 0);
        QCOMPARE(c.interact(player), Outcome::CounterEmpty);
        QVERIFY(player.heldItem() == nullptr);
        QCOMPARE(c.stock(), 0);
    }

    void prepTableHandOver()
    {
        Player player;
        PrepTable t("prepTable", QRect(150, 150, 75, 75), 10, 10, PopUpSide::Up);
        QVERIFY(t.cup() == nullptr);

        player.give(catalog.make("cup"));
        QCOMPARE(t.interact(player), Outcome::PutCupDown);
        QVERIFY(player.heldItem() == nullptr);
        QVERIFY(t.cup() != nullptr);
        QVERIFY(t.cup()->ingredients().isEmpty());

        player.give(catalog.make("milk"));
        QCOMPARE(t.interact(player), Outcome::AddedIngredient);
        QVERIFY(player.heldItem() == nullptr);
        QCOMPARE(t.cup()->ingredients(), QStringList{"milk"});

        QCOMPARE(t.interact(player), Outcome::PickedCupUp);
        QVERIFY(t.cup() == nullptr);
        QCOMPARE(player.heldItem()->image(), QString("filledCup"));
    }

    void prepTableRefusals()
    {
        Player player;
        PrepTable t("prepTable", QRect(150, 150, 75, 75), 10, 10, PopUpSide::Up);
        QCOMPARE(t.interact(player), Outcome::Nothing);   // nothing held, nothing on the table

        player.give(catalog.make("milk"));
        QCOMPARE(t.interact(player), Outcome::Nothing);   // an ingredient needs a cup to go into
        QVERIFY(player.heldItem() != nullptr);

        player.takeItem();
        player.give(catalog.make("cup"));
        t.interact(player);                               // the table now has a cup
        player.give(catalog.make("cup"));
        QCOMPARE(t.interact(player), Outcome::Nothing);   // no room for a second cup
        QVERIFY(player.heldItem() != nullptr);
    }

    void trashbin()
    {
        Player player;
        Trashbin b("trashbin", QRect(0, 325, 50, 50), 10, 10, PopUpSide::Right);
        QCOMPARE(b.interact(player), Outcome::Nothing);

        player.give(catalog.make("milk"));
        QCOMPARE(b.interact(player), Outcome::Discarded);
        QVERIFY(player.heldItem() == nullptr);
    }
};

QTEST_MAIN(TestInteractables)
#include "tst_interactables.moc"
