#pragma once

// Assets: one place that knows how to find the game's pictures, font and music.
// Replaces: Main.getResizedImage(), Main.getCustomFont() and every `new ImageIcon("x.png")`.
// Guide: chapter 10 (the resource system).
//
// These are free functions inside a namespace, not a class: there is nothing to
// construct and no state the caller needs to hold. Java would use a class of statics.

#include <QFont>
#include <QPixmap>
#include <QSize>
#include <QString>

namespace Assets {

// The image at its natural size. `name` has no folder and no ".png": Assets::pixmap("boba").
// Returns an empty ("null") pixmap and prints a warning if there is no such image.
QPixmap pixmap(const QString &name);

// The same image smoothly scaled to `size`. Each (name, size) pair is scaled once and
// then remembered, so asking again is free.
QPixmap pixmap(const QString &name, const QSize &size);

// The game's font (Waterlily) at a height in pixels.
QFont font(int pixelSize);

// Where the background music file should be: beside the .exe, not inside it.
QString musicFilePath();

} // namespace Assets
