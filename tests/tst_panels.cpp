// Tests for the three panels: Restock, Orders and Upgrades.
#include <QtTest>

#include <QComboBox>
#include <memory>

#include "items/Cup.h"
#include "panels/OrderCard.h"
#include "panels/OrderPanel.h"
#include "panels/RestockingPanel.h"
#include "panels/UpgradesPanel.h"
#include "ui/ImageButton.h"
#include "world/CafeScene.h"
#include "world/Chair.h"
#include "world/Counter.h"
#include "world/Customer.h"
#include "world/Player.h"

class TestPanels : public QObject
{
    Q_OBJECT

    static Customer *seat(CafeScene &scene, Chair *chair)
    {
        Customer *customer = scene.seatNewCustomer(chair);
        for (int i = 0; i < 600 && customer->state() != Customer::State::Seated; ++i)
            scene.customerTick();
        return customer;
    }

private slots:
    // ------------------------------------------------------------------ Restock

    void comboListsCountersSorted()
    {
        CafeScene s;
        RestockingPanel p(&s);
        QCOMPARE(p.size(), QSize(375, 150));

        auto *combo = p.findChild<QComboBox *>("counterCombo");
        QVERIFY(combo != nullptr);
        QCOMPARE(combo->focusPolicy(), Qt::NoFocus);
        QStringList shown;
        for (int i = 0; i < combo->count(); ++i)
            shown << combo->itemText(i);
        QCOMPARE(shown, QStringList({"boba counter", "cup counter", "grapefruit counter", "greentea counter",
                                     "ice counter", "jelly counter", "lychee counter", "mango counter",
                                     "milk counter", "sugar counter"}));

        QCOMPARE(p.selectedItem(), QString("boba"));
        QCOMPARE(p.unitCost(), 2);
        QCOMPARE(p.quantity(), 0);
        QCOMPARE(p.remaining(), 1);
    }

    void quantityButtons()
    {
        CafeScene s;
        RestockingPanel p(&s);
        p.changeQuantity(-1);
        QCOMPARE(p.quantity(), 0);
        p.changeQuantity(+10);
        p.changeQuantity(-1);
        QCOMPARE(p.quantity(), 9);
        QCOMPARE(p.totalCost(), 18);
        p.changeQuantity(-10);   // more than there is: ignored
        QCOMPARE(p.quantity(), 9);

        QTest::mouseClick(p.findChild<ImageButton *>("plus1"), Qt::LeftButton);
        QCOMPARE(p.quantity(), 10);
        QTest::mouseClick(p.findChild<ImageButton *>("plus10"), Qt::LeftButton);
        QCOMPARE(p.quantity(), 20);
        QTest::mouseClick(p.findChild<ImageButton *>("minus1"), Qt::LeftButton);
        QCOMPARE(p.quantity(), 19);
        QTest::mouseClick(p.findChild<ImageButton *>("minus10"), Qt::LeftButton);
        QCOMPARE(p.quantity(), 9);
    }

    void restockBuys()
    {
        CafeScene s;
        RestockingPanel p(&s);
        p.changeQuantity(+10);
        p.changeQuantity(-1);
        QTest::mouseClick(p.findChild<ImageButton *>("restockAction"), Qt::LeftButton);
        QCOMPARE(s.findCounter("boba")->stock(), 10);
        QCOMPARE(s.player()->money(), 982);
        QCOMPARE(p.quantity(), 0);

        p.restock();   // nothing chosen: nothing happens
        QCOMPARE(s.player()->money(), 982);
        QCOMPARE(s.findCounter("boba")->stock(), 10);
    }

    void tooExpensiveIsGreyAndRefused()
    {
        CafeScene s;
        RestockingPanel p(&s);
        auto *action = p.findChild<ImageButton *>("restockAction");
        QCOMPARE(action->currentImage(), QString("restockActionBtn"));

        for (int i = 0; i < 60; ++i)
            p.changeQuantity(+10);   // 600 x $2 = $1200
        QVERIFY(!p.canAfford());
        QCOMPARE(action->currentImage(), QString("restockActionBtnGrey"));

        p.restock();
        QCOMPARE(s.findCounter("boba")->stock(), 1);
        QCOMPARE(s.player()->money(), 1000);
    }

    void selectionChangesCost()
    {
        CafeScene s;
        RestockingPanel p(&s);
        auto *combo = p.findChild<QComboBox *>("counterCombo");
        p.changeQuantity(+1);

        combo->setCurrentText("sugar counter");
        QCOMPARE(p.selectedItem(), QString("sugar"));
        QCOMPARE(p.unitCost(), 1);
        QCOMPARE(p.remaining(), 2);
        QCOMPARE(p.quantity(), 1);   // the amount survives a change of counter

        combo->setCurrentText("cup counter");
        QCOMPARE(p.selectedItem(), QString("cup"));
        QCOMPARE(p.unitCost(), 1);
        QCOMPARE(p.remaining(), 3);
    }

    void remainingIsLive()   // fix F9
    {
        CafeScene s;
        RestockingPanel p(&s);
        QCOMPARE(p.remaining(), 1);
        s.player()->moveTo(160, 45);   // at the boba counter
        s.frameTick();
        s.interact();                  // takes the only boba
        QCOMPARE(p.remaining(), 0);
    }

    // ------------------------------------------------------------------- Orders

    void cardSize()
    {
        OrderCard card(1, {"milk", "boba"}, 5, 7);
        QCOMPARE(card.size(), QSize(350, 100));
        QCOMPARE(card.number(), 1);
        QCOMPARE(card.payout(), 7);
    }

    void oneCardPerSeatedCustomer()
    {
        CafeScene s;
        OrderPanel p(&s);
        QCOMPARE(p.size(), QSize(375, 150));
        QCOMPARE(p.cardCount(), 0);

        Customer *a = seat(s, s.chairs().at(5));
        QCOMPARE(p.cardCount(), 1);
        QCOMPARE(p.card(0)->number(), 1);
        QCOMPARE(p.card(0)->payout(), a->payout(0));

        seat(s, s.chairs().at(4));
        QCOMPARE(p.cardCount(), 2);
        QCOMPARE(p.card(1)->number(), 2);
    }

    void tipsRaiseTheShownPayout()   // fix F4: the card shows what will really be paid
    {
        CafeScene s;
        OrderPanel p(&s);
        Customer *a = seat(s, s.chairs().at(5));
        s.player()->addTip();
        QCOMPARE(p.cardCount(), 1);
        QCOMPARE(p.card(0)->payout(), a->payout(1));
    }

    void cardsDoNotOutliveTheirCustomer()   // Review Focus 3
    {
        CafeScene s;
        OrderPanel p(&s);
        Customer *a = seat(s, s.chairs().at(4));
        QCOMPARE(p.cardCount(), 1);

        s.player()->give(std::make_unique<Cup>(a->order().ingredients()));
        s.player()->moveTo(640, 20);
        s.frameTick();
        s.interact();                    // served: the customer starts to leave
        QCOMPARE(p.cardCount(), 0);

        for (int i = 0; i < 500; ++i)
            s.customerTick();            // the customer reaches the door and is deleted
        QCOMPARE(s.customerCount(), 0);
        QVERIFY(!p.grab().isNull());     // painting the panel afterwards must be safe
    }

    // ----------------------------------------------------------------- Upgrades

    void upgradesStartingState()
    {
        CafeScene s;
        UpgradesPanel p(&s);
        QCOMPARE(p.size(), QSize(400, 400));
        QCOMPARE(p.cost(Upgrade::Advertisement), 20);
        QCOMPARE(p.cost(Upgrade::Energy), 10);
        QCOMPARE(p.cost(Upgrade::Cuteness), 50);
        QCOMPARE(p.cost(Upgrade::Meowmax), 100);
        QCOMPARE(p.level(Upgrade::Energy), 0);
        QCOMPARE(p.maxLevel(Upgrade::Energy), 5);
        QCOMPARE(p.maxLevel(Upgrade::Meowmax), 2);

        auto *energy = p.findChild<ImageButton *>("energyButton");
        QCOMPARE(energy->geometry(), QRect(237, 183, 100, 25));
        QCOMPARE(energy->currentImage(), QString("upgrade"));
        QVERIFY(energy->isEnabled());
    }

    void energy()
    {
        CafeScene s;
        UpgradesPanel p(&s);
        auto *button = p.findChild<ImageButton *>("energyButton");
        QTest::mouseClick(button, Qt::LeftButton);
        QCOMPARE(p.level(Upgrade::Energy), 1);
        QCOMPARE(p.cost(Upgrade::Energy), 15);
        QCOMPARE(s.player()->money(), 990);
        QCOMPARE(s.player()->speed(), 2.5);

        for (int i = 0; i < 10; ++i)
            p.buy(Upgrade::Energy);   // 15 + 20 + 25 + 30 more, then it is at its limit
        QCOMPARE(p.level(Upgrade::Energy), 5);
        QCOMPARE(s.player()->money(), 900);
        QCOMPARE(button->currentImage(), QString("max"));
        QVERIFY(!button->isEnabled());
    }

    void advertisementAppliesTheNewLevel()   // fix F2
    {
        CafeScene s;
        UpgradesPanel p(&s);
        p.buy(Upgrade::Advertisement);
        QCOMPARE(s.spawnOdds().divisor(), 1.1);
        QCOMPARE(s.player()->money(), 980);
        QCOMPARE(p.cost(Upgrade::Advertisement), 25);
        QCOMPARE(p.level(Upgrade::Advertisement), 1);
    }

    void cuteness()
    {
        CafeScene s;
        UpgradesPanel p(&s);
        p.buy(Upgrade::Cuteness);
        QCOMPARE(s.player()->tips(), 1);
        QCOMPARE(s.player()->money(), 950);
        QCOMPARE(p.cost(Upgrade::Cuteness), 65);
    }

    void meowmaxRaisesTheOthers()
    {
        CafeScene s;
        UpgradesPanel p(&s);
        p.buy(Upgrade::Meowmax);
        QCOMPARE(p.maxLevel(Upgrade::Energy), 8);
        QCOMPARE(p.cost(Upgrade::Meowmax), 200);
        QCOMPARE(s.player()->money(), 900);

        p.buy(Upgrade::Meowmax);
        QCOMPARE(p.maxLevel(Upgrade::Cuteness), 11);
        QCOMPARE(s.player()->money(), 700);

        p.buy(Upgrade::Meowmax);   // already at its own limit of 2
        QCOMPARE(p.level(Upgrade::Meowmax), 2);
        QCOMPARE(s.player()->money(), 700);
        QCOMPARE(p.findChild<ImageButton *>("meowmaxButton")->currentImage(), QString("max"));
    }

    void tooPoor()
    {
        CafeScene s;
        UpgradesPanel p(&s);
        s.player()->addMoney(-995);   // $5 left
        auto *button = p.findChild<ImageButton *>("energyButton");
        QCOMPARE(button->currentImage(), QString("insufficient"));
        QVERIFY(!button->isEnabled());

        p.buy(Upgrade::Energy);
        QCOMPARE(p.level(Upgrade::Energy), 0);
        QCOMPARE(s.player()->money(), 5);

        s.player()->addMoney(5);      // $10: affordable again
        QCOMPARE(button->currentImage(), QString("upgrade"));
        QVERIFY(button->isEnabled());
    }
};

QTEST_MAIN(TestPanels)
#include "tst_panels.moc"
