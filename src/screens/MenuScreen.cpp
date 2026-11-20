#include "screens/MenuScreen.h"

#include <QPainter>

#include "Assets.h"
#include "ui/ImageButton.h"

MenuScreen::MenuScreen(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(1000, 665);

    // `new` with `this` as the parent: the button belongs to this screen from now on and
    // is deleted with it. That is why there is no matching `delete` anywhere.
    auto *play = new ImageButton("playImg", QRect(430, 535, 150, 50), this);
    play->setHoverImage("playHoverImg");
    play->setObjectName("playButton");   // a name other code (and the tests) can look it up by

    auto *instructions = new ImageButton("instructionsImg", QRect(430, 600, 150, 50), this);
    instructions->setHoverImage("instructionsHoverImg");
    instructions->setObjectName("instructionsButton");

    // connect(who, which signal, who, which function): "when the button is clicked, emit
    // my own playRequested signal". A signal can be connected straight to another signal.
    connect(play, &ImageButton::clicked, this, &MenuScreen::playRequested);
    connect(instructions, &ImageButton::clicked, this, &MenuScreen::instructionsRequested);
}

void MenuScreen::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, Assets::pixmap("menuBackground"));
}
