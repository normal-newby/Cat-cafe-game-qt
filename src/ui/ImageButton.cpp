#include "ui/ImageButton.h"

#include <QPainter>

#include "Assets.h"

// Passing `parent` up to QAbstractButton makes this button a child of that widget: it is
// drawn inside it, moves with it, and is deleted when it is deleted.
ImageButton::ImageButton(const QString &image, const QRect &geometry, QWidget *parent)
    : QAbstractButton(parent)
    , m_image(image)
    , m_normalSize(geometry.size())
    , m_hoverSize(geometry.size())
{
    setGeometry(geometry);

    // Clicking a button normally gives it the keyboard. The game needs the keyboard to
    // stay with the game screen (W A S D), so these buttons never take it.
    setFocusPolicy(Qt::NoFocus);
}

void ImageButton::setImage(const QString &image)
{
    m_image = image;
    update();   // "please repaint me soon": Qt will call paintEvent()
}

void ImageButton::setHoverImage(const QString &image)
{
    m_hoverImage = image;
    update();
}

void ImageButton::setHoverImage(const QString &image, const QSize &size)
{
    m_hoverImage = image;
    m_hoverSize = size;
    update();
}

QString ImageButton::currentImage() const
{
    if (m_mouseOver && !m_hoverImage.isEmpty())
        return m_hoverImage;
    return m_image;
}

void ImageButton::paintEvent(QPaintEvent *)
{
    // A QPainter drawing on `this` is Qt's version of the Graphics object Swing passes
    // to paintComponent. Anything not drawn stays see-through.
    QPainter painter(this);
    painter.drawPixmap(0, 0, Assets::pixmap(currentImage(), size()));
}

void ImageButton::enterEvent(QEnterEvent *)
{
    if (!isEnabled())
        return;   // a button that cannot be used stays silent and does not react

    m_mouseOver = true;
    resize(m_hoverSize);
    update();
    emit hovered();
}

void ImageButton::leaveEvent(QEvent *)
{
    m_mouseOver = false;
    resize(m_normalSize);
    update();
}
