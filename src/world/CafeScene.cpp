#include "world/CafeScene.h"

#include <QGraphicsPixmapItem>
#include <QRandomGenerator>
#include <QTimer>

#include "Assets.h"
#include "world/Chair.h"
#include "world/Counter.h"
#include "world/Decoration.h"
#include "world/Player.h"
#include "world/PrepTable.h"
#include "world/Trashbin.h"

namespace {

QString popUpImage(PopUpSide side)
{
    switch (side) {
    case PopUpSide::Up: return "counterPopUp";
    case PopUpSide::Down: return "counterPopDown";
    case PopUpSide::Left: return "counterPopLeft";
    case PopUpSide::Right: return "counterPopRight";
    }
    return QString();
}

} // namespace

CafeScene::CafeScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_player(new Player)
    , m_frameTimer(new QTimer(this))
    , m_customerTimer(new QTimer(this))
    , m_spawnTimer(new QTimer(this))
    , m_messageTimer(new QTimer(this))
{
    setSceneRect(0, 0, 1000, 665);

    // Z-values decide what is drawn on top of what, whatever order things are added in:
    // floor 0, player 1, furniture 2, customers 3, pop-up 4, message 6.
    QGraphicsPixmapItem *floor = addPixmap(Assets::pixmap("background"));
    floor->setZValue(0);

    buildWorld();
    addItem(m_player);   // from here on the scene owns the player and will delete it

    // Only one thing can be in reach at a time, so one pop-up is moved around.
    m_popUp = addPixmap(QPixmap());
    m_popUp->setZValue(4);
    m_popUp->hide();

    m_outOfStockMessage = addPixmap(Assets::pixmap("insufficientMessage", QSize(500, 100)));
    m_outOfStockMessage->setPos(250, 300);
    m_outOfStockMessage->setZValue(6);
    m_outOfStockMessage->hide();

    // The three game clocks. Swing had one actionPerformed() that asked "which timer was
    // it?"; here each timer is connected to the one function it is for.
    m_frameTimer->setInterval(16);
    m_customerTimer->setInterval(10);
    m_spawnTimer->setInterval(1000);
    // The default timer type is allowed to be a few percent off to save power. A game
    // wants its ticks on time.
    m_frameTimer->setTimerType(Qt::PreciseTimer);
    m_customerTimer->setTimerType(Qt::PreciseTimer);
    m_spawnTimer->setTimerType(Qt::PreciseTimer);
    connect(m_frameTimer, &QTimer::timeout, this, &CafeScene::frameTick);
    connect(m_customerTimer, &QTimer::timeout, this, &CafeScene::customerTick);
    connect(m_spawnTimer, &QTimer::timeout, this, &CafeScene::spawnTick);

    // A single-shot timer fires once and stops: "hide the message one second from now".
    m_messageTimer->setInterval(1000);
    m_messageTimer->setSingleShot(true);
    connect(m_messageTimer, &QTimer::timeout, this, &CafeScene::hideOutOfStockMessage);
}

// ---------------------------------------------------------------------------------------
// Building the cafe. This is the list of `new Counter(...)`, `new Chair(...)` lines from
// the Game constructor. Rectangles are x, y, width, height (the Java had height first).
// ---------------------------------------------------------------------------------------

void CafeScene::buildWorld()
{
    addCounter("cupCounter", 235, 150, 10, 10, PopUpSide::Up, "cup", 3);

    addCounter("bobaCounter", 150, 0, 0, 10, PopUpSide::Down, "boba", 1);
    addCounter("milkCounter", 225, 0, 0, 10, PopUpSide::Down, "milk", 1);
    addCounter("sugarCounter", 300, 0, 0, 10, PopUpSide::Down, "sugar", 2);

    addCounter("iceCounter", 0, 225, 10, 0, PopUpSide::Right, "ice", 2);
    addCounter("grapefruitCounter", 0, 75, 10, 0, PopUpSide::Right, "grapefruit", 1);
    addCounter("greenteaCounter", 0, 150, 10, 0, PopUpSide::Right, "greentea", 1);

    addCounter("jellyCounter", 150, 300, 0, 10, PopUpSide::Up, "jelly", 1);
    addCounter("lycheeCounter", 225, 300, 0, 10, PopUpSide::Up, "lychee", 1);
    addCounter("mangoCounter", 300, 300, 0, 10, PopUpSide::Up, "mango", 1);

    addInteractable(new PrepTable("prepTable", QRect(150, 150, 75, 75), 10, 10, PopUpSide::Up));
    addInteractable(new PrepTable("prepTable", QRect(320, 150, 75, 75), 10, 10, PopUpSide::Up));

    addInteractable(new Trashbin("trashbin", QRect(0, 325, 50, 50), 10, 10, PopUpSide::Right));
    addInteractable(new Trashbin("trashbin", QRect(75, 0, 50, 50), 10, 10, PopUpSide::Down));

    // Every route starts at the door on the right-hand edge.
    const QPoint door(980, 180);

    addSolid(new Decoration("longTable", QRect(575, 475, 400, 100)));
    addChair("chairLeft", 575, 385, false, {door, {900, 180}, {900, 350}, {600, 350}, {600, 380}});
    addChair("chairLeft", 675, 385, false, {door, {900, 180}, {900, 350}, {700, 350}, {700, 380}});
    addChair("chairRight", 775, 385, true, {door, {900, 180}, {900, 350}, {800, 350}, {800, 380}});
    addChair("chairRight", 875, 385, true, {door, {900, 180}, {900, 350}, {900, 380}});

    addSolid(new Decoration("table", QRect(700, 50, 100, 100)));
    addChair("chairLeft", 625, 62, false, {door, {620, 180}, {620, 100}});
    addChair("chairRight", 800, 62, true, {door, {850, 180}, {850, 100}});

    addSolid(new Decoration("table", QRect(700, 230, 100, 100)));
    addChair("chairLeft", 625, 242, false, {door, {620, 180}, {620, 260}});
    addChair("chairRight", 800, 242, true, {door, {850, 180}, {850, 260}});

    addSolid(new Decoration("wall", QRect(0, 423, 450, 4)));
    addSolid(new Decoration("wall", QRect(448, 75, 4, 275)));
    addSolid(new Decoration("wall", QRect(448, 425, 4, 165)));
    addSolid(new Decoration("wall", QRect(450, 588, 550, 4)));
}

void CafeScene::addSolid(NonCollidable *item)
{
    addItem(item);   // the scene takes ownership
    m_solidRects.append(item->solidRect());
}

void CafeScene::addInteractable(Interactable *item)
{
    addSolid(item);
    m_interactables.append(item);
}

void CafeScene::addCounter(const QString &image, int x, int y, int reachX, int reachY, PopUpSide side,
                           const QString &itemName, int stock)
{
    auto *counter = new Counter(image, QRect(x, y, 75, 75), reachX, reachY, side, m_catalog, itemName, stock);
    addInteractable(counter);
    m_counters.append(counter);
}

void CafeScene::addChair(const QString &image, int x, int y, bool facesRight, const QList<QPoint> &route)
{
    auto *chair = new Chair(image, QRect(x, y, 75, 75), 10, 10, PopUpSide::Up, facesRight, route);
    addInteractable(chair);
    m_chairs.append(chair);
}

// ---------------------------------------------------------------------------------------
// Looking things up
// ---------------------------------------------------------------------------------------

Player *CafeScene::player() const
{
    return m_player;
}

const ItemCatalog &CafeScene::catalog() const
{
    return m_catalog;
}

const QList<Counter *> &CafeScene::counters() const
{
    return m_counters;
}

Counter *CafeScene::findCounter(const QString &itemName) const
{
    for (Counter *counter : m_counters) {
        if (counter->itemName() == itemName)
            return counter;
    }
    return nullptr;
}

const QList<Chair *> &CafeScene::chairs() const
{
    return m_chairs;
}

const QList<Interactable *> &CafeScene::interactables() const
{
    return m_interactables;
}

QList<QRect> CafeScene::solidRects() const
{
    return m_solidRects;
}

QList<Customer *> CafeScene::seatedCustomers() const
{
    QList<Customer *> seated;
    for (Customer *customer : m_customers) {
        if (customer->state() == Customer::State::Seated)
            seated.append(customer);
    }
    return seated;
}

int CafeScene::customerCount() const
{
    return m_customers.size();
}

SpawnOdds &CafeScene::spawnOdds()
{
    return m_spawnOdds;
}

Interactable *CafeScene::target() const
{
    return m_target;
}

// ---------------------------------------------------------------------------------------
// Things that happen
// ---------------------------------------------------------------------------------------

Customer *CafeScene::seatNewCustomer(Chair *chair)
{
    const Cup order = m_catalog.randomOrder(*QRandomGenerator::global());
    auto *customer = new Customer(order, m_catalog.orderCost(order), *chair, m_customerSprites);
    addItem(customer);
    m_customers.append(customer);
    chair->assign(customer);
    return customer;
}

void CafeScene::restock(Counter *counter, int amount)
{
    counter->addStock(amount);
    emit stockChanged();
}

void CafeScene::start()
{
    m_frameTimer->start();
    m_customerTimer->start();
    m_spawnTimer->start();
}

void CafeScene::stop()
{
    m_frameTimer->stop();
    m_customerTimer->stop();
    m_spawnTimer->stop();
}

bool CafeScene::isRunning() const
{
    return m_frameTimer->isActive();
}

void CafeScene::frameTick()
{
    m_player->step(m_solidRects);
    updateTarget();
}

void CafeScene::customerTick()
{
    bool someoneSatDown = false;
    for (Customer *customer : m_customers) {
        const bool wasSeated = customer->state() == Customer::State::Seated;
        customer->step();
        if (!wasSeated && customer->state() == Customer::State::Seated)
            someoneSatDown = true;
    }

    // Customers who reached the door are finished with. Going through the list backwards
    // means removing one does not shift the ones still to be looked at.
    for (int i = m_customers.size() - 1; i >= 0; --i) {
        Customer *customer = m_customers.at(i);
        if (customer->state() == Customer::State::Gone) {
            m_customers.removeAt(i);
            removeItem(customer);   // the scene gives ownership back to us...
            delete customer;        // ...so now it is ours to delete
        }
    }

    if (someoneSatDown) {
        renumberSeated();
        emit ordersChanged();
    }
}

void CafeScene::spawnTick()
{
    QList<Chair *> freeChairs;
    for (Chair *chair : m_chairs) {
        if (chair->isFree())
            freeChairs.append(chair);
    }

    if (!freeChairs.isEmpty()) {
        QRandomGenerator *random = QRandomGenerator::global();
        if (m_spawnOdds.roll(random->generateDouble())) {
            m_spawnOdds.customerSpawned(m_customers.size());
            seatNewCustomer(freeChairs.at(random->bounded(freeChairs.size())));
        }
    }
    renumberSeated();
}

void CafeScene::interact()
{
    if (m_target == nullptr)
        return;

    const int moneyBefore = m_player->money();

    // One virtual call. Which interact() runs depends on what m_target really is:
    // a Counter, a PrepTable, a Trashbin or a Chair.
    const Outcome outcome = m_target->interact(*m_player);

    switch (outcome) {
    case Outcome::TookItem:
        emit stockChanged();
        break;
    case Outcome::CounterEmpty:
        m_outOfStockMessage->show();
        m_messageTimer->start();   // restarts the second if the message is already showing
        emit counterEmpty();
        break;
    case Outcome::ServedRight:
    case Outcome::ServedWrong:
        renumberSeated();
        emit orderServed(outcome == Outcome::ServedRight, m_player->money() - moneyBefore);
        emit ordersChanged();
        break;
    default:
        break;   // nothing else in the game needs to hear about the other outcomes
    }

    updateTarget();   // a chair that has just been served can no longer be used
}

void CafeScene::hideOutOfStockMessage()
{
    m_outOfStockMessage->hide();
}

void CafeScene::updateTarget()
{
    m_target = nullptr;
    const QRect reach = m_player->reachRect();

    // The first one that is in reach AND usable wins. (The Java stopped at the first one
    // in reach even if it was an empty chair, which could hide the chair beside it.)
    for (Interactable *candidate : m_interactables) {
        if (candidate->canInteract() && candidate->reachRect().intersects(reach)) {
            m_target = candidate;
            break;
        }
    }

    if (m_target != nullptr) {
        m_popUp->setPixmap(Assets::pixmap(popUpImage(m_target->popUpSide()), QSize(50, 50)));
        m_popUp->setPos(m_target->popUpRect().topLeft());
        m_popUp->show();
    } else {
        m_popUp->hide();
    }
}

void CafeScene::renumberSeated()
{
    int number = 1;
    for (Customer *customer : m_customers) {
        if (customer->state() == Customer::State::Seated)
            customer->setNumber(number++);
    }
}
