#include "world/Player.h"

#include <cmath>

#include "Assets.h"

Player::Player()
    : m_stationary("stationary")
    , m_rollRight("rollRight")
    , m_rollLeft("rollLeft")
    , m_rollUp("rollUp")
    , m_rollDown("rollDown")
    , m_heldIcon(new QGraphicsPixmapItem(this))
{
    setZValue(1);   // drawn above the floor, below counters and chairs
    setPos(m_x, m_y);
    setPixmap(m_stationary.next());

    // A child item is positioned relative to its parent and moves with it. (40, 20) is
    // "just right of the cat, halfway down", wherever the cat happens to be.
    m_heldIcon->setPos(40, 20);
    m_heldIcon->hide();
}

void Player::setKeyHeld(int key, bool held)
{
    switch (key) {
    case Qt::Key_W: m_up = held; break;
    case Qt::Key_S: m_down = held; break;
    case Qt::Key_A: m_left = held; break;
    case Qt::Key_D: m_right = held; break;
    default: break;
    }
}

void Player::stop()
{
    m_up = false;
    m_down = false;
    m_left = false;
    m_right = false;
}

void Player::step(const QList<QRect> &solids)
{
    double dx = (m_left ? -m_speed : 0) + (m_right ? m_speed : 0);
    double dy = (m_up ? -m_speed : 0) + (m_down ? m_speed : 0);

    // Moving diagonally would otherwise be faster than moving straight.
    if (dx != 0 && dy != 0) {
        dx = (dx > 0 ? 1 : -1) * m_speed / std::sqrt(2.0);
        dy = (dy > 0 ? 1 : -1) * m_speed / std::sqrt(2.0);
    }

    m_x += dx;
    m_y += dy;

    bool blocked = m_x <= 0 || m_x >= 960 || m_y <= 0 || m_y >= 620;
    const QRect body = bodyRect();
    for (const QRect &solid : solids) {
        if (body.intersects(solid)) {
            blocked = true;
            break;
        }
    }
    if (blocked) {   // undo the whole move, on both axes
        m_x -= dx;
        m_y -= dy;
    }

    // The exact position is kept as two doubles; the picture sits on whole pixels.
    setPos(qRound(m_x), qRound(m_y));
    setPixmap(currentAnimation().next());
}

void Player::moveTo(double x, double y)
{
    m_x = x;
    m_y = y;
    setPos(qRound(m_x), qRound(m_y));
}

QRect Player::bodyRect() const
{
    return QRect(qRound(m_x) + 5, qRound(m_y) + 5, 30, 30);
}

QRect Player::reachRect() const
{
    // Copied from the Java exactly, including its lean to the right and downwards.
    return QRect(qRound(m_x) + 2, qRound(m_y) + 2, 44, 44);
}

Item *Player::heldItem() const
{
    // get() lends out a plain pointer for looking. The player still owns the item.
    return m_held.get();
}

void Player::give(std::unique_ptr<Item> item)
{
    Q_ASSERT(m_held == nullptr);
    // A unique_ptr cannot be copied, only moved: after this line `item` is empty and
    // m_held owns the object. Ownership has changed hands.
    m_held = std::move(item);
    heldItemWasChanged();
}

std::unique_ptr<Item> Player::takeItem()
{
    if (m_held == nullptr)
        return nullptr;

    std::unique_ptr<Item> item = std::move(m_held);
    heldItemWasChanged();
    return item;
}

std::unique_ptr<Cup> Player::takeCup()
{
    // dynamic_cast is C++'s instanceof-and-cast in one: it gives back a Cup* if the item
    // really is a Cup, and nullptr if it is anything else.
    Cup *cup = dynamic_cast<Cup *>(m_held.get());
    if (cup == nullptr)
        return nullptr;

    // m_held is a unique_ptr<Item>; the caller wants a unique_ptr<Cup>. release() makes
    // m_held let go without deleting, and the new unique_ptr takes over the same object.
    m_held.release();
    heldItemWasChanged();
    return std::unique_ptr<Cup>(cup);
}

int Player::money() const
{
    return m_money;
}

void Player::addMoney(int delta)
{
    m_money += delta;
    emit moneyChanged(m_money);   // anyone connected to this signal is told right now
}

int Player::tips() const
{
    return m_tips;
}

void Player::addTip()
{
    ++m_tips;
    emit tipsChanged(m_tips);
}

double Player::speed() const
{
    return m_speed;
}

void Player::addSpeed(double amount)
{
    m_speed += amount;
}

SpriteAnimation &Player::currentAnimation()
{
    if (m_right && !m_left)
        return m_rollRight;
    if (m_left && !m_right)
        return m_rollLeft;
    if (m_up && !m_down)
        return m_rollUp;
    if (m_down && !m_up)
        return m_rollDown;
    return m_stationary;
}

void Player::heldItemWasChanged()
{
    if (m_held != nullptr) {
        m_heldIcon->setPixmap(Assets::pixmap(m_held->image(), QSize(25, 25)));
        m_heldIcon->show();
    } else {
        m_heldIcon->hide();
    }
    emit heldItemChanged();
}
