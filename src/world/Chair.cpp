#include "world/Chair.h"

#include "items/Cup.h"
#include "world/Customer.h"
#include "world/Player.h"

Chair::Chair(const QString &image, const QRect &rect, int reachX, int reachY, PopUpSide side,
             bool facesRight, const QList<QPoint> &route)
    : Interactable(image, rect, reachX, reachY, side)
    , m_facesRight(facesRight)
    , m_route(route)
{
    // The picture is bigger than the part you bump into.
    setSolidRect(QRect(rect.x() + 10, rect.y() + 13, rect.width() - 20, rect.height() - 15));
}

bool Chair::facesRight() const
{
    return m_facesRight;
}

const QList<QPoint> &Chair::route() const
{
    return m_route;
}

bool Chair::isFree() const
{
    return m_customer == nullptr;
}

Customer *Chair::customer() const
{
    return m_customer;
}

void Chair::assign(Customer *customer)
{
    m_customer = customer;
}

bool Chair::canInteract() const
{
    return m_customer != nullptr && m_customer->state() == Customer::State::Seated;
}

Outcome Chair::interact(Player &player)
{
    if (!canInteract())
        return Outcome::Nothing;

    Cup *cup = dynamic_cast<Cup *>(player.heldItem());
    if (cup == nullptr)
        return Outcome::Nothing;   // empty paws, or carrying an ingredient

    const bool right = cup->matches(m_customer->order());
    if (right)
        player.addMoney(m_customer->payout(player.tips()));
    else
        player.addMoney(-WrongOrderPenalty);

    player.takeCup();        // the customer keeps the drink: it is not stored, so it is destroyed
    m_customer->leave();     // the customer walks itself out...
    m_customer = nullptr;    // ...and this chair is free straight away

    return right ? Outcome::ServedRight : Outcome::ServedWrong;
}
