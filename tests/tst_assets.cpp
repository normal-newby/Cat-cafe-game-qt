// Tests for Assets: are the images, the font and the sounds where the game expects them?
#include <QtTest>

#include "Assets.h"

class TestAssets : public QObject
{
    Q_OBJECT

private slots:
    void imagesAreEmbedded()
    {
        QCOMPARE(QDir(":/images").entryList(QDir::Files).size(), 86);
    }

    void pixmapLoadsNaturalSize()
    {
        QCOMPARE(Assets::pixmap("background").size(), QSize(1000, 665));
    }

    void pixmapScales()
    {
        QCOMPARE(Assets::pixmap("boba", QSize(25, 25)).size(), QSize(25, 25));
    }

    void missingPixmapIsNull()
    {
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("nope"));
        QVERIFY(Assets::pixmap("nope").isNull());
    }

    void fontIsTheGameFont()
    {
        const QFont f = Assets::font(48);
        QCOMPARE(f.pixelSize(), 48);
        QVERIFY(f.family().contains("Waterlily"));
    }

    void shortSoundsAreEmbedded()
    {
        QVERIFY(QFile::exists(":/sounds/click.wav"));
        QVERIFY(QFile::exists(":/sounds/meowHappy.wav"));
        QVERIFY(QFile::exists(":/sounds/meowAngry.wav"));
        QVERIFY(!QFile::exists(":/sounds/backgroundMusic.wav"));
    }

    void musicPathIsBesideExe()
    {
        QVERIFY(Assets::musicFilePath().endsWith("/assets/sounds/backgroundMusic.wav"));
        QVERIFY(QFile::exists(Assets::musicFilePath()));
    }
};

QTEST_MAIN(TestAssets)
#include "tst_assets.moc"
