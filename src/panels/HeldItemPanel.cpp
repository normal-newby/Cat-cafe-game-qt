#include "panels/HeldItemPanel.h"

#include <QPainter>

#include "Assets.h"
#include "items/Cup.h"
#include "world/Player.h"

HeldItemPanel::HeldItemPanel(Player *player, QWidget *parent)
    : QWidget(parent)
    , m_player(player)
{
    setFixedSize(375, 150);
    connect(m_player, &Player::heldItemChanged, this, &HeldItemPanel::refresh);
}

QString HeldItemPanel::shownName() const
{
    const Item *item = m_player->heldItem();
    return item != nullptr ? item->name() : "Nothing";
}

void HeldItemPanel::refresh()
{
    update();
}

void HeldItemPanel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, Assets::pixmap("currentItemBackground", QSize(375, 150)));

    painter.setPen(QColor("#5097B9"));
    painter.setFont(Assets::font(48));
    painter.drawText(20, 45, "Current Item: ");
    painter.drawText(20, 105, shownName());

    const Item *item = m_player->heldItem();
    if (item == nullptr)
        return;
    painter.drawPixmap(280, 25, Assets::pixmap(item->image(), QSize(90, 90)));

    // Only a cup has contents to list.
    const Cup *cup = dynamic_cast<const Cup *>(item);
    if (cup == nullptr)
        return;

    painter.setFont(Assets::font(36));
    painter.drawText(120, 85, "Ingredients: ");

    // Two columns of two: the first two names on the left, the rest on the right.
    painter.setFont(Assets::font(24));
    const QStringList &ingredients = cup->ingredients();
    for (int i = 0; i < ingredients.size(); ++i) {
        const int x = i < 2 ? 100 : 200;
        const int y = i % 2 == 0 ? 115 : 140;
        painter.drawText(x, y, ingredients.at(i));
    }
}
