// Tests for the two small building blocks: SpriteAnimation and ImageButton.
#include <QtTest>

#include <QEnterEvent>
#include <QSignalSpy>

#include "ui/ImageButton.h"
#include "world/SpriteAnimation.h"

class TestWidgets : public QObject
{
    Q_OBJECT

    // Pretend the mouse moved onto / off a widget. Nothing is shown on screen.
    static void mouseEnters(QWidget *widget)
    {
        QEnterEvent event(QPointF(5, 5), QPointF(5, 5), QPointF(5, 5));
        QApplication::sendEvent(widget, &event);
    }
    static void mouseLeaves(QWidget *widget)
    {
        QEvent event(QEvent::Leave);
        QApplication::sendEvent(widget, &event);
    }

private slots:
    void sheetsCutIntoFrames()
    {
        QCOMPARE(SpriteAnimation("stationary").frameCount(), 3);
        QCOMPARE(SpriteAnimation("rollLeft").frameCount(), 14);
        QCOMPARE(SpriteAnimation("rollUp").frameCount(), 28);
        QCOMPARE(SpriteAnimation("customerLeft").frameCount(), 21);
        QCOMPARE(SpriteAnimation("customerDown").frameCount(), 28);
        QCOMPARE(SpriteAnimation("stationary").frame(0).size(), QSize(40, 40));
    }

    void nextStartsAtFrameOneAndWraps()
    {
        SpriteAnimation a("stationary");
        QCOMPARE(a.next().cacheKey(), a.frame(1).cacheKey());
        QCOMPARE(a.next().cacheKey(), a.frame(2).cacheKey());
        QCOMPARE(a.next().cacheKey(), a.frame(0).cacheKey());
    }

    void copiesAnimateIndependently()
    {
        SpriteAnimation a("stationary");
        SpriteAnimation b = a;
        a.next();
        QCOMPARE(b.next().cacheKey(), b.frame(1).cacheKey());
    }

    void buttonBasics()
    {
        QWidget parent;
        ImageButton b("playImg", QRect(430, 535, 150, 50), &parent);
        QCOMPARE(b.geometry(), QRect(430, 535, 150, 50));
        QCOMPARE(b.focusPolicy(), Qt::NoFocus);
        QCOMPARE(b.currentImage(), QString("playImg"));

        QSignalSpy clicked(&b, &ImageButton::clicked);
        QTest::mouseClick(&b, Qt::LeftButton);
        QCOMPARE(clicked.count(), 1);

        b.setImage("playHoverImg");
        QCOMPARE(b.currentImage(), QString("playHoverImg"));
    }

    void hoverSwapsImageAndEmits()
    {
        QWidget parent;
        ImageButton b("playImg", QRect(430, 535, 150, 50), &parent);
        QSignalSpy hovered(&b, &ImageButton::hovered);

        mouseEnters(&b);                       // no hover image yet: still announces the hover
        QCOMPARE(hovered.count(), 1);
        QCOMPARE(b.currentImage(), QString("playImg"));
        mouseLeaves(&b);

        b.setHoverImage("playHoverImg");
        mouseEnters(&b);
        QCOMPARE(hovered.count(), 2);
        QCOMPARE(b.currentImage(), QString("playHoverImg"));
        QCOMPARE(b.geometry(), QRect(430, 535, 150, 50));
        mouseLeaves(&b);
        QCOMPARE(b.currentImage(), QString("playImg"));
    }

    void hoverCanGrow()
    {
        QWidget parent;
        ImageButton b("backBtn", QRect(870, 10, 100, 50), &parent);
        b.setHoverImage("backBtnHover", QSize(106, 53));
        mouseEnters(&b);
        QCOMPARE(b.geometry(), QRect(870, 10, 106, 53));
        mouseLeaves(&b);
        QCOMPARE(b.geometry(), QRect(870, 10, 100, 50));
    }

    void disabledButtonIsQuiet()
    {
        QWidget parent;
        ImageButton b("playImg", QRect(430, 535, 150, 50), &parent);
        b.setHoverImage("playHoverImg");
        b.setEnabled(false);

        QSignalSpy hovered(&b, &ImageButton::hovered);
        QSignalSpy clicked(&b, &ImageButton::clicked);
        mouseEnters(&b);
        QTest::mouseClick(&b, Qt::LeftButton);
        QCOMPARE(hovered.count(), 0);
        QCOMPARE(clicked.count(), 0);
        QCOMPARE(b.currentImage(), QString("playImg"));
    }
};

QTEST_MAIN(TestWidgets)
#include "tst_widgets.moc"
