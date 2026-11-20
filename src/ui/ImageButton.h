#pragma once

// ImageButton: a button that is just a picture, optionally swapping to a second picture
// while the mouse is over it.
// Replaces: every `new JButton(icon)` with setContentAreaFilled(false), and the
//           mouseEntered / mouseExited pairs that swapped icons and played the click.
// Guide: chapter 14 (widgets, events and painting).
//
// In Swing you added a MouseListener to a button from outside. In Qt a widget handles
// its own events by overriding virtual functions (enterEvent, leaveEvent, paintEvent),
// and tells the outside world what happened with a signal.

#include <QAbstractButton>
#include <QSize>
#include <QString>

class ImageButton : public QAbstractButton
{
    Q_OBJECT

public:
    // `image` is an Assets name; `geometry` is where the button sits inside `parent`.
    ImageButton(const QString &image, const QRect &geometry, QWidget *parent);

    void setImage(const QString &image);
    void setHoverImage(const QString &image);
    void setHoverImage(const QString &image, const QSize &size);   // also grow to `size` while hovered

    QString currentImage() const;   // the Assets name of what is being drawn right now

signals:
    void hovered();   // the mouse has just moved onto an enabled button

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QString m_image;
    QString m_hoverImage;
    QSize m_normalSize;
    QSize m_hoverSize;
    bool m_mouseOver = false;
};
