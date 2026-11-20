#include "screens/GameScreen.h"

#include <QFontMetrics>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QLabel>
#include <QPropertyAnimation>

#include "Assets.h"
#include "panels/HeldItemPanel.h"
#include "panels/OrderPanel.h"
#include "panels/RestockingPanel.h"
#include "panels/UpgradesPanel.h"
#include "ui/ImageButton.h"
#include "world/CafeScene.h"
#include "world/Player.h"

namespace {

const QColor moneyGreen(21, 120, 17);

// A label showing 24-pixel game text, placed so the line its letters stand on is at
// (x, baseline). Swing's drawString and QPainter::drawText take that line directly;
// a QLabel is positioned by its top-left corner, so the font's ascent is subtracted.
QLabel *makeTextLabel(const QString &name, int x, int baseline, QWidget *parent)
{
    const QFont font = Assets::font(24);
    const QFontMetrics metrics(font);

    auto *label = new QLabel(parent);
    label->setObjectName(name);
    label->setFont(font);
    label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    label->setGeometry(x, baseline - metrics.ascent(), 1000 - x, metrics.height());
    return label;
}

void setTextColor(QLabel *label, const QColor &color)
{
    QPalette palette = label->palette();
    palette.setColor(QPalette::WindowText, color);
    label->setPalette(palette);
}

} // namespace

GameScreen::GameScreen(QWidget *parent)
    : QWidget(parent)
    , m_scene(new CafeScene(this))               // a QObject child: deleted with this screen
    , m_view(new QGraphicsView(m_scene, this))   // a child widget showing that scene
{
    setFixedSize(1000, 665);

    // The view is only a window onto the scene: no border, no scroll bars, no mouse
    // handling, and it must not take the keyboard away from this screen.
    m_view->setGeometry(0, 0, 1000, 665);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setFocusPolicy(Qt::NoFocus);
    m_view->setInteractive(false);
    m_view->setRenderHint(QPainter::SmoothPixmapTransform);

    // Key presses go to whichever widget has "focus". This screen accepts it, and none
    // of its buttons do, so W A S D keep working after any click. (The Java version had
    // to call addNotify() after every button press to win the keyboard back.)
    setFocusPolicy(Qt::StrongFocus);

    // Widgets created later are drawn on top of ones created earlier, so everything
    // made from here on sits over the view, and the three panels sit over the rest.
    Player *player = m_scene->player();

    m_heldItemPanel = new HeldItemPanel(player, this);
    m_heldItemPanel->move(0, 435);

    m_banner = new QLabel(this);
    m_banner->setGeometry(375, 435, 75, 150);

    auto *restockButton = new ImageButton("restockBtn", QRect(20, 600, 200, 50), this);
    auto *ordersButton = new ImageButton("ordersBtn", QRect(240, 600, 200, 50), this);
    auto *upgradesButton = new ImageButton("upgradesBtn", QRect(460, 600, 200, 50), this);
    restockButton->setObjectName("restockButton");
    ordersButton->setObjectName("ordersButton");
    upgradesButton->setObjectName("upgradesButton");

    m_musicButton = new ImageButton("musicOn", QRect(850, 600, 50, 50), this);
    m_soundButton = new ImageButton("soundOn", QRect(920, 600, 50, 50), this);
    m_musicButton->setObjectName("musicButton");
    m_soundButton->setObjectName("soundButton");

    m_moneyLabel = makeTextLabel("moneyLabel", 700, 650, this);
    setTextColor(m_moneyLabel, moneyGreen);
    showMoney(player->money());

    // The "+$7" that drifts down for two seconds after an order is served.
    m_moneyDeltaLabel = makeTextLabel("moneyDeltaLabel", 780, 620, this);
    m_moneyDeltaLabel->hide();

    // A QPropertyAnimation changes one property of one object a little at a time. Here:
    // the label's "pos", from where it starts to 13 pixels lower, over 2000 ms.
    m_moneyDeltaDrift = new QPropertyAnimation(m_moneyDeltaLabel, "pos", this);
    m_moneyDeltaDrift->setDuration(2000);
    m_moneyDeltaDrift->setStartValue(m_moneyDeltaLabel->pos());
    m_moneyDeltaDrift->setEndValue(m_moneyDeltaLabel->pos() + QPoint(0, 13));
    connect(m_moneyDeltaDrift, &QPropertyAnimation::finished, m_moneyDeltaLabel, &QLabel::hide);

    m_restockPanel = new RestockingPanel(m_scene, this);
    m_restockPanel->move(0, 435);
    m_restockPanel->hide();

    m_orderPanel = new OrderPanel(m_scene, this);
    m_orderPanel->move(0, 435);
    m_orderPanel->hide();

    m_upgradesPanel = new UpgradesPanel(m_scene, this);
    m_upgradesPanel->move(300, 150);
    m_upgradesPanel->hide();

    auto *back = new ImageButton("backBtn", QRect(870, 10, 100, 50), this);
    back->setHoverImage("backBtnHover", QSize(106, 53));
    back->setObjectName("backButton");

    m_banner->setPixmap(Assets::pixmap(bannerImage()));

    // --- what the controls do ---
    connect(back, &ImageButton::clicked, this, &GameScreen::backRequested);
    connect(m_soundButton, &ImageButton::clicked, this, &GameScreen::soundToggleRequested);
    connect(m_musicButton, &ImageButton::clicked, this, &GameScreen::musicToggleRequested);

    connect(restockButton, &ImageButton::clicked, this, [this] {
        m_scene->player()->stop();
        togglePanel(Panel::Restock);
    });
    connect(ordersButton, &ImageButton::clicked, this, [this] {
        m_scene->player()->stop();
        togglePanel(Panel::Orders);
    });
    connect(upgradesButton, &ImageButton::clicked, this, [this] {
        m_scene->player()->stop();
        togglePanel(Panel::Upgrades);
    });

    // --- what the controls show ---
    connect(player, &Player::moneyChanged, this, &GameScreen::showMoney);
    connect(m_scene, &CafeScene::orderServed, this, &GameScreen::showMoneyChange);
}

CafeScene *GameScreen::scene() const
{
    return m_scene;
}

void GameScreen::start()
{
    m_scene->start();
    setFocus();
}

void GameScreen::stop()
{
    m_scene->stop();
    m_scene->player()->stop();
}

GameScreen::Panel GameScreen::openPanel() const
{
    return m_openPanel;
}

QString GameScreen::bannerImage() const
{
    switch (m_openPanel) {
    case Panel::Restock: return "restockBanner";
    case Panel::Orders: return "ordersBanner";
    case Panel::Upgrades: return "upgradesBanner";
    case Panel::None: break;
    }
    return "itemBanner";
}

void GameScreen::togglePanel(Panel panel)
{
    m_openPanel = (m_openPanel == panel) ? Panel::None : panel;

    m_restockPanel->setVisible(m_openPanel == Panel::Restock);
    m_orderPanel->setVisible(m_openPanel == Panel::Orders);
    m_upgradesPanel->setVisible(m_openPanel == Panel::Upgrades);
    m_banner->setPixmap(Assets::pixmap(bannerImage()));
}

void GameScreen::showAudioState(bool soundOn, bool musicOn)
{
    m_soundButton->setImage(soundOn ? "soundOn" : "soundOff");
    m_musicButton->setImage(musicOn ? "musicOn" : "musicOff");
}

void GameScreen::showMoney(int money)
{
    m_moneyLabel->setText(QString("Money: $%1").arg(money));
}

void GameScreen::showMoneyChange(bool correct, int moneyDelta)
{
    const QString sign = moneyDelta >= 0 ? "+" : "-";
    m_moneyDeltaLabel->setText(QString("%1$%2").arg(sign).arg(qAbs(moneyDelta)));
    setTextColor(m_moneyDeltaLabel, moneyDelta >= 0 ? moneyGreen : QColor(Qt::red));
    m_moneyDeltaLabel->show();

    m_moneyDeltaDrift->stop();    // in case the last one is still drifting
    m_moneyDeltaDrift->start();

    emit orderServed(correct);
}

void GameScreen::keyPressEvent(QKeyEvent *event)
{
    // Holding a key down makes the keyboard send the press again and again. Those
    // repeats are not new presses.
    if (event->isAutoRepeat())
        return;

    const int key = event->key();
    const bool isGameKey = key == Qt::Key_W || key == Qt::Key_A || key == Qt::Key_S || key == Qt::Key_D
                           || key == Qt::Key_E;
    if (!isGameKey) {
        QWidget::keyPressEvent(event);   // not ours: let the normal handling have it
        return;
    }

    // As in the original, the cat stands still while the Upgrades shop is open.
    if (m_openPanel == Panel::Upgrades)
        return;

    if (key == Qt::Key_E)
        m_scene->interact();
    else
        m_scene->player()->setKeyHeld(key, true);
}

void GameScreen::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_A:
    case Qt::Key_S:
    case Qt::Key_D:
        m_scene->player()->setKeyHeld(event->key(), false);
        break;
    default:
        QWidget::keyReleaseEvent(event);
        break;
    }
}

void GameScreen::focusOutEvent(QFocusEvent *event)
{
    // If the window loses the keyboard while a key is down (alt-tab), the release never
    // arrives. Without this the cat would keep running.
    m_scene->player()->stop();
    QWidget::focusOutEvent(event);
}
