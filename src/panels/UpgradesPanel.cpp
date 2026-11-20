#include "panels/UpgradesPanel.h"

#include <QPainter>

#include "Assets.h"
#include "ui/ImageButton.h"
#include "world/CafeScene.h"
#include "world/Player.h"

UpgradesPanel::UpgradesPanel(CafeScene *scene, QWidget *parent)
    : QWidget(parent)
    , m_scene(scene)
{
    setFixedSize(400, 400);

    // Added in the same order as the Upgrade enum, so an Upgrade can be used as an index.
    addEntry("advertisementButton", QRect(63, 183, 100, 25), 20, 5, QPoint(47, 159), Upgrade::Advertisement);
    addEntry("energyButton", QRect(237, 183, 100, 25), 10, 5, QPoint(220, 159), Upgrade::Energy);
    addEntry("cutenessButton", QRect(63, 331, 100, 25), 50, 15, QPoint(47, 306), Upgrade::Cuteness);
    addEntry("meowmaxButton", QRect(237, 331, 100, 25), 100, 100, QPoint(220, 306), Upgrade::Meowmax);

    connect(m_scene->player(), &Player::moneyChanged, this, &UpgradesPanel::refreshButtons);
    refreshButtons();
}

void UpgradesPanel::addEntry(const QString &buttonName, const QRect &buttonRect, int firstCost, int costStep,
                             const QPoint &textAt, Upgrade upgrade)
{
    auto *button = new ImageButton("upgrade", buttonRect, this);
    button->setObjectName(buttonName);

    // `[this, upgrade]` copies `upgrade` into the lambda, so each button remembers
    // which upgrade it is for.
    connect(button, &ImageButton::clicked, this, [this, upgrade] { buy(upgrade); });

    m_entries.append(Entry{0, firstCost, costStep, textAt, button});
}

const UpgradesPanel::Entry &UpgradesPanel::entry(Upgrade upgrade) const
{
    // static_cast converts between related types on request; here, an enum to its number.
    return m_entries.at(static_cast<int>(upgrade));
}

int UpgradesPanel::level(Upgrade upgrade) const
{
    return entry(upgrade).level;
}

int UpgradesPanel::cost(Upgrade upgrade) const
{
    return entry(upgrade).cost;
}

int UpgradesPanel::maxLevel(Upgrade upgrade) const
{
    return upgrade == Upgrade::Meowmax ? 2 : m_sharedMaxLevel;
}

void UpgradesPanel::buy(Upgrade upgrade)
{
    Entry &bought = m_entries[static_cast<int>(upgrade)];   // a reference: changes go into the list
    Player *player = m_scene->player();

    if (player->money() < bought.cost || bought.level >= maxLevel(upgrade))
        return;

    const int price = bought.cost;
    bought.level += 1;
    bought.cost += bought.costStep;

    switch (upgrade) {
    case Upgrade::Advertisement:
        m_scene->spawnOdds().setAdvertisementLevel(bought.level);
        break;
    case Upgrade::Energy:
        player->addSpeed(0.3);
        break;
    case Upgrade::Cuteness:
        player->addTip();
        break;
    case Upgrade::Meowmax:
        m_sharedMaxLevel += 3;
        break;
    }

    player->addMoney(-price);   // emits moneyChanged, which refreshes the buttons
    update();
}

void UpgradesPanel::refreshButtons()
{
    const int money = m_scene->player()->money();

    for (int i = 0; i < m_entries.size(); ++i) {
        const Entry &e = m_entries.at(i);
        const bool atLimit = e.level >= maxLevel(static_cast<Upgrade>(i));
        const bool tooDear = money < e.cost;

        if (atLimit)
            e.button->setImage("max");
        else if (tooDear)
            e.button->setImage("insufficient");
        else
            e.button->setImage("upgrade");

        // A disabled button ignores clicks and stays quiet when the mouse passes over it.
        e.button->setEnabled(!atLimit && !tooDear);
    }
    update();
}

void UpgradesPanel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, Assets::pixmap("upgradesBackground", QSize(400, 400)));

    painter.setFont(Assets::font(12));
    painter.setPen(Qt::white);

    for (int i = 0; i < m_entries.size(); ++i) {
        const Entry &e = m_entries.at(i);
        const int limit = maxLevel(static_cast<Upgrade>(i));

        painter.drawText(e.textAt, QString("Upgrade Level: %1/%2").arg(e.level).arg(limit));
        if (e.level >= limit)
            painter.drawText(e.textAt + QPoint(0, 15), "MAX LEVEL");
        else
            painter.drawText(e.textAt + QPoint(0, 15), QString("Cost to Upgrade: %1").arg(e.cost));
    }
}
