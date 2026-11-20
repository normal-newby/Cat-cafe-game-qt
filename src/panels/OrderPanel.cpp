#include "panels/OrderPanel.h"

#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

#include "panels/OrderCard.h"
#include "world/CafeScene.h"
#include "world/Customer.h"
#include "world/Player.h"

OrderPanel::OrderPanel(CafeScene *scene, QWidget *parent)
    : QWidget(parent)
    , m_scene(scene)
    , m_content(new QWidget)
    , m_cardLayout(new QVBoxLayout(m_content))   // giving a layout a widget installs it on that widget
{
    setFixedSize(375, 150);

    // The pink behind the cards.
    QPalette pink = m_content->palette();
    pink.setColor(QPalette::Window, QColor("#FFE2DF"));
    m_content->setPalette(pink);
    m_content->setAutoFillBackground(true);

    m_cardLayout->setContentsMargins(0, 0, 0, 0);
    m_cardLayout->setSpacing(0);
    m_cardLayout->setAlignment(Qt::AlignTop);   // cards gather at the top, not spread out

    // A QScrollArea shows part of a bigger widget and adds scroll bars when needed.
    // Swing: new JScrollPane(ordersDisplay).
    auto *scroll = new QScrollArea(this);
    scroll->setGeometry(0, 0, 375, 150);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setFocusPolicy(Qt::NoFocus);
    scroll->setWidgetResizable(true);           // let the content be as wide as the area
    scroll->verticalScrollBar()->setSingleStep(8);
    scroll->setWidget(m_content);               // the scroll area now owns m_content

    connect(m_scene, &CafeScene::ordersChanged, this, &OrderPanel::rebuild);
    connect(m_scene->player(), &Player::tipsChanged, this, &OrderPanel::rebuild);

    rebuild();
}

int OrderPanel::cardCount() const
{
    return m_cards.size();
}

const OrderCard *OrderPanel::card(int index) const
{
    return m_cards.at(index);
}

void OrderPanel::rebuild()
{
    // Deleting a widget also takes it out of the layout it was in.
    qDeleteAll(m_cards);
    m_cards.clear();

    const QList<Customer *> seated = m_scene->seatedCustomers();
    const int tips = m_scene->player()->tips();

    for (int i = 0; i < seated.size(); ++i) {
        const Customer *customer = seated.at(i);
        auto *card = new OrderCard(i + 1, customer->order().ingredients(), customer->orderCost(),
                                   customer->payout(tips), m_content);
        m_cardLayout->addWidget(card, 0, Qt::AlignHCenter);
        m_cards.append(card);
    }
}
