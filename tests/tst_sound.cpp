// Tests for SoundManager: the rules about what is switched on. They check what the
// manager decided to do, not what comes out of the speakers.
#include <QtTest>

#include <QLoggingCategory>
#include <QSignalSpy>

#include "Assets.h"
#include "audio/SoundManager.h"

class TestSound : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // These tests destroy a SoundManager a few milliseconds after starting its music.
        // The music player then reports that loading was cut short: true, and harmless.
        QLoggingCategory::setFilterRules("qt.multimedia.ffmpeg.mediadataholder.warning=false");
    }

    void flagsAndToggles()
    {
        SoundManager m(Assets::musicFilePath());
        QSignalSpy changed(&m, &SoundManager::stateChanged);
        QVERIFY(m.soundOn());
        QVERIFY(m.musicOn());
        QVERIFY(!m.isMusicPlaying());

        m.startMusic();
        QVERIFY(m.isMusicPlaying());

        m.toggleMusic();                 // music off, sound still on
        QVERIFY(!m.musicOn());
        QVERIFY(m.soundOn());
        QVERIFY(!m.isMusicPlaying());

        m.toggleSound();                 // everything off
        QVERIFY(!m.soundOn());
        QVERIFY(!m.musicOn());

        m.toggleMusic();                 // ignored while sound is off
        QVERIFY(!m.musicOn());

        m.toggleSound();                 // sound back on brings the music with it
        QVERIFY(m.soundOn());
        QVERIFY(m.musicOn());
        QVERIFY(m.isMusicPlaying());

        QCOMPARE(changed.count(), 3);
        QCOMPARE(changed.last().at(0).toBool(), true);
        QCOMPARE(changed.last().at(1).toBool(), true);

        m.stopMusic();                   // leaving the game: stops, but stays "on" for next time
        QVERIFY(!m.isMusicPlaying());
        QVERIFY(m.musicOn());
    }

    void startMusicRespectsFlags()
    {
        SoundManager m(Assets::musicFilePath());
        m.toggleMusic();
        m.startMusic();
        QVERIFY(!m.isMusicPlaying());
    }

    void missingMusicFileIsSurvivable()   // Review Focus 1
    {
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("nope"));
        SoundManager m("Z:/nope.wav");
        m.startMusic();
        QVERIFY(!m.isMusicPlaying());

        m.playClick();        // none of these may crash without the music file
        m.playMeow(true);
        m.toggleSound();
        m.toggleSound();
        QVERIFY(m.soundOn());
        QVERIFY(!m.isMusicPlaying());
    }

    void effectsPlayOnlyWhenSoundIsOn()   // fix F3
    {
        SoundManager m(Assets::musicFilePath());
        QSignalSpy played(&m, &SoundManager::effectPlayed);

        m.playClick();
        m.playMeow(true);
        m.playMeow(false);
        QCOMPARE(played.count(), 3);
        QCOMPARE(played.at(0).at(0).toString(), QString("click"));
        QCOMPARE(played.at(1).at(0).toString(), QString("meowHappy"));
        QCOMPARE(played.at(2).at(0).toString(), QString("meowAngry"));

        m.toggleSound();        // muted: nothing more is played
        m.playClick();
        m.playMeow(false);
        QCOMPARE(played.count(), 3);
    }
};

QTEST_MAIN(TestSound)
#include "tst_sound.moc"
