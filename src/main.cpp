// Where the program starts.
// Replaces: Main.main() in Main.java.
// Guide: chapter 11 (QApplication and the event loop).

#include <QApplication>

#include "Assets.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    // Exactly one QApplication must exist before any widget, pixmap or font is created.
    QApplication app(argc, argv);

    // Every widget that does not choose its own font uses this one.
    QApplication::setFont(Assets::font(14));

    MainWindow window;
    window.show();

    // exec() runs the event loop: it waits for clicks, key presses and timer ticks and
    // hands each one to the object it belongs to. It only returns when the window closes.
    return app.exec();
}
