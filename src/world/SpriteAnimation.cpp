#include "world/SpriteAnimation.h"

#include "Assets.h"

SpriteAnimation::SpriteAnimation(const QString &sheet)
{
    const int cell = 200;    // size of one picture on the sheet
    const int frame = 40;    // size it is drawn at

    const QPixmap all = Assets::pixmap(sheet);
    const int columns = all.width() / cell;
    const int rows = all.height() / cell;

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const QPixmap one = all.copy(column * cell, row * cell, cell, cell);
            m_frames.append(one.scaled(frame, frame, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
    }
}

int SpriteAnimation::frameCount() const
{
    return m_frames.size();
}

const QPixmap &SpriteAnimation::frame(int index) const
{
    return m_frames.at(index);
}

const QPixmap &SpriteAnimation::next()
{
    if (m_frames.isEmpty()) {
        static const QPixmap nothing;   // the sheet was missing; draw nothing rather than crash
        return nothing;
    }
    m_current = (m_current + 1) % m_frames.size();
    return m_frames.at(m_current);
}
