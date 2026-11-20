#include "panels/OrderCard.h"

#include <QPainter>

#include "Assets.h"

OrderCard::OrderCard(int number, const QStringList &ingredients, int cost, int payout, QWidget *parent)
    : QWidget(parent)
    , m_number(number)
    , m_ingredients(ingredients)
    , m_cost(cost)
    , m_payout(payout)
{
    setFixedSize(350, 100);
}

int OrderCard::number() const
{
    return m_number;
}

int OrderCard::payout() const
{
    return m_payout;
}

void OrderCard::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, Assets::pixmap("orderDisplayBox", QSize(350, 100)));

    painter.setFont(Assets::font(48));
    painter.drawText(30, 65, QString("#%1").arg(m_number));

    painter.setFont(Assets::font(18));
    for (int i = 0; i < m_ingredients.size(); ++i)
        painter.drawText(110, 45 + 15 * i, m_ingredients.at(i));
    painter.drawText(250, 45, QString("$%1").arg(m_cost));
    painter.drawText(250, 75, QString("$%1").arg(m_payout));
}
