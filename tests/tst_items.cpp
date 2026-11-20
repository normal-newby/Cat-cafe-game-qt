// Tests for the things the player carries: the catalog of item definitions, ingredients
// and cups. None of this needs a window.
#include <QtTest>

#include <QRandomGenerator>
#include <QSet>
#include <memory>

#include "items/Cup.h"
#include "items/Ingredient.h"
#include "items/Item.h"
#include "items/ItemCatalog.h"

class TestItems : public QObject
{
    Q_OBJECT

    ItemCatalog catalog;

private slots:
    void catalogKnowsTheItems()
    {
        QCOMPARE(catalog.find("boba")->cost, 2);
        QCOMPARE(catalog.find("boba")->kind, ItemKind::Topping);
        QCOMPARE(catalog.find("sugar")->cost, 1);
        QCOMPARE(catalog.find("sugar")->kind, ItemKind::Extra);
        QCOMPARE(catalog.find("cup")->cost, 1);
        QCOMPARE(catalog.find("cup")->image, QString("emptyCup"));
        QVERIFY(catalog.find("tea") == nullptr);
        QCOMPARE(catalog.namesOf(ItemKind::Liquid),
                 QStringList({"milk", "grapefruit", "greentea", "lychee", "mango"}));
        QCOMPARE(catalog.namesOf(ItemKind::Topping), QStringList({"boba", "jelly"}));
    }

    void makeBuildsTheRightClass()
    {
        QVERIFY(dynamic_cast<Cup *>(catalog.make("cup").get()) != nullptr);
        QVERIFY(dynamic_cast<Ingredient *>(catalog.make("milk").get()) != nullptr);
        QCOMPARE(catalog.make("milk")->name(), QString("milk"));
        QCOMPARE(catalog.make("milk")->image(), QString("milk"));
        QVERIFY(catalog.make("tea") == nullptr);
    }

    void cupImageFollowsContents()
    {
        Cup c;
        QCOMPARE(c.image(), QString("emptyCup"));
        QCOMPARE(c.name(), QString("cup"));
        c.add("milk");
        QCOMPARE(c.image(), QString("filledCup"));
        QCOMPARE(c.name(), QString("cup"));
        QCOMPARE(c.ingredients(), QStringList{"milk"});
    }

    void cupsMatchInAnyOrder()
    {
        QVERIFY(Cup({"milk", "boba", "ice"}).matches(Cup({"ice", "milk", "boba"})));
        QVERIFY(!Cup({"milk", "boba"}).matches(Cup({"milk", "boba", "ice"})));
        QVERIFY(!Cup({"milk", "milk", "boba"}).matches(Cup({"milk", "boba", "boba"})));
        QVERIFY(Cup().matches(Cup()));
    }

    void orderCost()
    {
        QCOMPARE(catalog.orderCost(Cup({"milk", "boba"})), 5);
        QCOMPARE(catalog.orderCost(Cup({"mango", "jelly", "sugar", "ice"})), 7);
        QCOMPARE(catalog.orderCost(Cup()), 1);
        QCOMPARE(catalog.orderCost(Cup({"milk", "tea"})), 3);   // an unknown ingredient is free
    }

    void randomOrdersAreWellFormed()
    {
        QRandomGenerator rng(1234);
        const QStringList liquids = catalog.namesOf(ItemKind::Liquid);
        const QStringList toppings = catalog.namesOf(ItemKind::Topping);

        QSet<QString> seen;
        int withSugar = 0;
        int withoutSugar = 0;
        for (int i = 0; i < 300; ++i) {
            const QStringList order = catalog.randomOrder(rng).ingredients();
            QVERIFY(order.size() >= 2 && order.size() <= 4);
            QVERIFY(liquids.contains(order.at(0)));
            QVERIFY(toppings.contains(order.at(1)));

            const QStringList extras = order.mid(2);
            QVERIFY(extras.isEmpty() || extras == QStringList{"sugar"} || extras == QStringList{"ice"}
                    || extras == QStringList({"sugar", "ice"}));

            seen.insert(order.at(0));
            seen.insert(order.at(1));
            if (extras.contains("sugar"))
                ++withSugar;
            else
                ++withoutSugar;
        }
        QCOMPARE(seen.size(), liquids.size() + toppings.size());
        QVERIFY(withSugar > 0);
        QVERIFY(withoutSugar > 0);
    }
};

QTEST_MAIN(TestItems)
#include "tst_items.moc"
