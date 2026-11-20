#include "panels/RestockingPanel.h"

#include <QComboBox>
#include <QPainter>

#include "Assets.h"
#include "ui/ImageButton.h"
#include "world/CafeScene.h"
#include "world/Counter.h"
#include "world/Player.h"

RestockingPanel::RestockingPanel(CafeScene *scene, QWidget *parent)
    : QWidget(parent)
    , m_scene(scene)
    , m_combo(new QComboBox(this))
    , m_restockButton(new ImageButton("restockActionBtn", QRect(15, 105, 120, 40), this))
{
    setFixedSize(375, 150);

    QStringList names;
    for (Counter *counter : m_scene->counters())
        names.append(counter->itemName() + " counter");
    names.sort();

    m_combo->setObjectName("counterCombo");
    m_combo->setGeometry(10, 10, 130, 20);
    m_combo->setMaxVisibleItems(5);
    m_combo->setFocusPolicy(Qt::NoFocus);   // the keyboard stays with the game
    m_combo->addItems(names);

    m_restockButton->setObjectName("restockAction");

    auto *plus1 = new ImageButton("plus1", QRect(170, 10, 50, 50), this);
    auto *minus1 = new ImageButton("minus1", QRect(170, 70, 50, 50), this);
    auto *plus10 = new ImageButton("plus10", QRect(230, 10, 50, 50), this);
    auto *minus10 = new ImageButton("minus10", QRect(230, 70, 50, 50), this);
    plus1->setObjectName("plus1");
    minus1->setObjectName("minus1");
    plus10->setObjectName("plus10");
    minus10->setObjectName("minus10");

    // clicked() carries no number, but changeQuantity() needs one. A lambda is a small
    // unnamed function written in place: `[this]` lets it use this panel, and the body
    // is what runs on each click.
    connect(plus1, &ImageButton::clicked, this, [this] { changeQuantity(+1); });
    connect(minus1, &ImageButton::clicked, this, [this] { changeQuantity(-1); });
    connect(plus10, &ImageButton::clicked, this, [this] { changeQuantity(+10); });
    connect(minus10, &ImageButton::clicked, this, [this] { changeQuantity(-10); });
    connect(m_restockButton, &ImageButton::clicked, this, &RestockingPanel::restock);

    // Whenever anything this panel shows might have changed, redraw it.
    connect(m_combo, &QComboBox::currentIndexChanged, this, &RestockingPanel::refresh);
    connect(m_scene, &CafeScene::stockChanged, this, &RestockingPanel::refresh);
    connect(m_scene->player(), &Player::moneyChanged, this, &RestockingPanel::refresh);

    refresh();
}

QString RestockingPanel::selectedItem() const
{
    QString text = m_combo->currentText();
    text.chop(QString(" counter").size());   // "boba counter" -> "boba"
    return text;
}

int RestockingPanel::quantity() const
{
    return m_quantity;
}

int RestockingPanel::unitCost() const
{
    const ItemInfo *info = m_scene->catalog().find(selectedItem());
    return info != nullptr ? info->cost : 0;
}

int RestockingPanel::totalCost() const
{
    return unitCost() * m_quantity;
}

bool RestockingPanel::canAfford() const
{
    return m_scene->player()->money() >= totalCost();
}

int RestockingPanel::remaining() const
{
    const Counter *counter = m_scene->findCounter(selectedItem());
    return counter != nullptr ? counter->stock() : 0;
}

void RestockingPanel::changeQuantity(int delta)
{
    if (m_quantity + delta < 0)
        return;
    m_quantity += delta;
    refresh();
}

void RestockingPanel::restock()
{
    Counter *counter = m_scene->findCounter(selectedItem());
    if (counter == nullptr || m_quantity == 0 || !canAfford())
        return;

    const int cost = totalCost();
    m_scene->restock(counter, m_quantity);
    m_quantity = 0;
    m_scene->player()->addMoney(-cost);   // emits moneyChanged, which calls refresh()
}

void RestockingPanel::refresh()
{
    m_restockButton->setImage(canAfford() ? "restockActionBtn" : "restockActionBtnGrey");
    update();
}

void RestockingPanel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, Assets::pixmap("restockBackground", QSize(375, 150)));

    const ItemInfo *info = m_scene->catalog().find(selectedItem());
    if (info != nullptr)
        painter.drawPixmap(40, 32, Assets::pixmap(info->image, QSize(70, 70)));

    // drawText(x, y, text): y is the line the letters stand on, as in Swing's drawString.
    painter.setFont(Assets::font(14));
    painter.drawText(290, 20, QString("Buying: %1").arg(m_quantity));
    painter.drawText(290, 50, QString("Cost: %1").arg(totalCost()));
    painter.drawText(290, 80, QString("Remaining: %1").arg(remaining()));

    if (canAfford())
        painter.drawText(170, 140, QString("Money after purchase: %1").arg(m_scene->player()->money() - totalCost()));
    else
        painter.drawText(170, 140, "Insufficient money");
}
