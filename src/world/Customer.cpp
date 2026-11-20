#include "world/Customer.h"

#include "Assets.h"
#include "world/Chair.h"

// Things in an unnamed namespace can only be used inside this file.
namespace {

int sign(int value)
{
    if (value > 0)
        return 1;
    if (value < 0)
        return -1;
    return 0;
}

} // namespace

CustomerSprites::CustomerSprites()
    : up("customerUp")
    , down("customerDown")
    , left("customerLeft")
    , right("customerRight")
    , seatedLeft(Assets::pixmap("chairSeatedLeft", QSize(75, 75)))
    , seatedRight(Assets::pixmap("chairSeatedRight", QSize(75, 75)))
{
    for (int number = 1; number <= 8; ++number)
        numberBubbles.append(Assets::pixmap(QString("customer%1PopUp").arg(number), QSize(50, 50)));
}

Customer::Customer(const Cup &order, int orderCost, const Chair &chair, const CustomerSprites &sprites)
    : m_order(order)
    , m_orderCost(orderCost)
    , m_route(chair.route())
    , m_seat(chair.rect().topLeft())
    , m_seatFacesRight(chair.facesRight())
    , m_sprites(sprites)
    , m_bubble(new QGraphicsPixmapItem(this))
{
    setZValue(3);   // above the furniture
    setPos(m_route.first());
    setPixmap(m_sprites.left.next());   // customers come in from the right, heading left

    m_bubble->setPos(0, -50);
    m_bubble->hide();
}

Customer::State Customer::state() const
{
    return m_state;
}

const Cup &Customer::order() const
{
    return m_order;
}

int Customer::orderCost() const
{
    return m_orderCost;
}

int Customer::payout(int tips) const
{
    return m_orderCost * 3 / 2 + tips;   // one and a half times the cost, rounded down
}

void Customer::step()
{
    if (m_state == State::Arriving) {
        if (m_target == m_route.size()) {   // past the last corner: sit
            sitDown();
            return;
        }
        if (walkTowards(m_route.at(m_target)))
            ++m_target;
    } else if (m_state == State::Leaving) {
        if (walkTowards(m_route.at(m_target))) {
            if (m_target == 0)
                m_state = State::Gone;      // back at the door; the scene will remove it
            else
                --m_target;
        }
    }
    // Seated and Gone customers do not move.
}

void Customer::leave()
{
    if (m_state != State::Seated)
        return;

    m_state = State::Leaving;
    m_target = m_route.size() - 1;
    setPos(m_route.last());

    // Stand up facing the way it is about to walk.
    if (m_route.size() >= 2) {
        const QPoint next = m_route.at(m_route.size() - 2);
        face(sign(next.x() - m_route.last().x()), sign(next.y() - m_route.last().y()));
    } else {
        face(1, 0);
    }
    showNumber();
}

void Customer::setNumber(int number)
{
    m_number = number;
    showNumber();
}

int Customer::number() const
{
    return m_number;
}

bool Customer::walkTowards(const QPoint &target)
{
    const QPoint here = pos().toPoint();
    const int dx = sign(target.x() - here.x());
    const int dy = sign(target.y() - here.y());

    face(dx, dy);
    setPos(here.x() + dx, here.y() + dy);
    return pos().toPoint() == target;
}

void Customer::face(int dx, int dy)
{
    if (dx > 0)
        setPixmap(m_sprites.right.next());
    else if (dx < 0)
        setPixmap(m_sprites.left.next());
    else if (dy < 0)
        setPixmap(m_sprites.up.next());
    else if (dy > 0)
        setPixmap(m_sprites.down.next());
    // not moving: keep the picture it has
}

void Customer::sitDown()
{
    m_state = State::Seated;
    setPos(m_seat);   // exactly over the chair, drawn as "chair with a customer in it"
    setPixmap(m_seatFacesRight ? m_sprites.seatedRight : m_sprites.seatedLeft);
    showNumber();
}

void Customer::showNumber()
{
    const bool show = m_state == State::Seated && m_number >= 1 && m_number <= m_sprites.numberBubbles.size();
    if (show)
        m_bubble->setPixmap(m_sprites.numberBubbles.at(m_number - 1));
    m_bubble->setVisible(show);
}
