#include "Assets.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFontDatabase>
#include <QHash>

QPixmap Assets::pixmap(const QString &name)
{
    // A `static` local variable is created the first time the function runs and then
    // lives until the program ends. It is this function's private memory.
    static QHash<QString, QPixmap> cache;

    if (cache.contains(name))
        return cache.value(name);

    // A path starting with ":/" is read from the resources compiled into the program.
    const QPixmap loaded(":/images/" + name + ".png");
    if (loaded.isNull()) {
        qWarning() << "Assets: there is no image called" << name;
        return loaded;
    }

    cache.insert(name, loaded);
    return loaded;
}

QPixmap Assets::pixmap(const QString &name, const QSize &size)
{
    static QHash<QString, QPixmap> cache;

    const QString key = QString("%1@%2x%3").arg(name).arg(size.width()).arg(size.height());
    if (cache.contains(key))
        return cache.value(key);

    const QPixmap original = pixmap(name);
    if (original.isNull())
        return original;

    const QPixmap scaled = original.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    cache.insert(key, scaled);
    return scaled;
}

QFont Assets::font(int pixelSize)
{
    // The font file has to be registered with Qt once before it can be used by name.
    static QString family;
    if (family.isEmpty()) {
        const int id = QFontDatabase::addApplicationFont(":/fonts/WaterlilyScript.ttf");
        const QStringList families = QFontDatabase::applicationFontFamilies(id);
        if (families.isEmpty())
            qWarning() << "Assets: the game font could not be loaded";
        else
            family = families.first();
    }

    QFont result(family);
    result.setPixelSize(pixelSize);
    return result;
}

QString Assets::musicFilePath()
{
    return QCoreApplication::applicationDirPath() + "/assets/sounds/backgroundMusic.wav";
}
