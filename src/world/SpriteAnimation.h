#pragma once

// SpriteAnimation: a sprite sheet cut into frames, plus "which frame am I on".
// Replaces: Animations.java.
// Guide: chapter 3 (values, references and pointers).
//
// This class is meant to be copied. QPixmap shares its pixels between copies, so a copy
// of a SpriteAnimation costs almost nothing and gets its own frame position. That is how
// every customer can animate at its own pace from one set of loaded pictures.

#include <QList>
#include <QPixmap>
#include <QString>

class SpriteAnimation
{
public:
    SpriteAnimation() = default;

    // `sheet` is the name of an image made of 200 x 200 cells, read left to right, top to
    // bottom. Each cell becomes a 40 x 40 frame.
    explicit SpriteAnimation(const QString &sheet);

    int frameCount() const;
    const QPixmap &frame(int index) const;

    // Moves on to the following frame and returns it, going back to the start after the
    // last. (So the very first call returns frame 1, as the Java did.)
    const QPixmap &next();

private:
    QList<QPixmap> m_frames;
    int m_current = 0;
};
