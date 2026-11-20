#include "screens/InstructionsScreen.h"

#include <QPainter>
#include <QTimer>

#include "Assets.h"
#include "ui/ImageButton.h"

InstructionsScreen::InstructionsScreen(QWidget *parent)
    : QWidget(parent)
    , m_timer(new QTimer(this))
    , m_customerWalk("customerDown")
    , m_cat("stationary")
{
    setFixedSize(1000, 665);

    m_customerFrame = m_customerWalk.next();
    m_catFrame = m_cat.next();

    auto *back = new ImageButton("backBtn", QRect(870, 10, 100, 50), this);
    back->setHoverImage("backBtnHover", QSize(106, 53));
    back->setObjectName("backButton");
    connect(back, &ImageButton::clicked, this, &InstructionsScreen::backRequested);

    // A QTimer emits timeout() over and over. Connecting it to advance() is the whole
    // animation loop. Swing: new Timer(30, this) plus an actionPerformed that checks
    // e.getSource().
    m_timer->setInterval(30);
    connect(m_timer, &QTimer::timeout, this, &InstructionsScreen::advance);
}

void InstructionsScreen::start()
{
    m_timer->start();
}

void InstructionsScreen::stop()
{
    m_timer->stop();
}

bool InstructionsScreen::isRunning() const
{
    return m_timer->isActive();
}

int InstructionsScreen::customerY() const
{
    return m_customerY;
}

bool InstructionsScreen::customerVisible() const
{
    return m_pauseTicks == 0;
}

bool InstructionsScreen::chairOccupied() const
{
    return m_chairOccupied;
}

void InstructionsScreen::advance()
{
    // Sitting: the customer is not drawn and the chair shows its "seated" picture.
    // 33 ticks of 30 ms is the Java's one-second pause.
    if (m_pauseTicks > 0) {
        --m_pauseTicks;
        return;
    }

    m_chairOccupied = false;
    m_customerY += 2;
    m_customerFrame = m_customerWalk.next();
    m_catFrame = m_cat.next();

    if (m_customerY > 300) {   // reached the chair
        m_customerY = 200;
        m_chairOccupied = true;
        m_pauseTicks = 33;
    }

    update();   // Swing's repaint()
}

void InstructionsScreen::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, Assets::pixmap("instructionsBackground"));
    painter.drawPixmap(90, 100, Assets::pixmap("instructionsImage"));
    painter.drawPixmap(650, 190, m_catFrame);
    painter.drawPixmap(480, 375, Assets::pixmap("restockBtn", QSize(120, 30)));
    painter.drawPixmap(750, 375, Assets::pixmap("ordersBtn", QSize(120, 30)));

    if (customerVisible())
        painter.drawPixmap(440, m_customerY, m_customerFrame);

    const QString chair = m_chairOccupied ? "chairSeatedRight" : "chairRight";
    painter.drawPixmap(420, 300, Assets::pixmap(chair, QSize(75, 75)));
}
