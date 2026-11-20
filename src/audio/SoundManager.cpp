#include "audio/SoundManager.h"

#include <QAudioOutput>
#include <QDebug>
#include <QFile>
#include <QMediaPlayer>
#include <QSoundEffect>
#include <QUrl>

SoundManager::SoundManager(const QString &musicFile, QObject *parent)
    : QObject(parent)
    , m_click(new QSoundEffect(this))
    , m_meowHappy(new QSoundEffect(this))
    , m_meowAngry(new QSoundEffect(this))
    , m_music(new QMediaPlayer(this))
    , m_musicOutput(new QAudioOutput(this))
    , m_musicFile(musicFile)
{
    // QSoundEffect is for short sounds that must start instantly. It takes a URL;
    // "qrc:/..." is the URL form of the ":/..." resource paths used for pictures.
    m_click->setSource(QUrl("qrc:/sounds/click.wav"));
    m_meowHappy->setSource(QUrl("qrc:/sounds/meowHappy.wav"));
    m_meowAngry->setSource(QUrl("qrc:/sounds/meowAngry.wav"));

    // QMediaPlayer is for long audio. It decodes the file; a QAudioOutput is the
    // speaker it plays through, and where the volume is set.
    m_music->setAudioOutput(m_musicOutput);
    m_musicOutput->setVolume(0.32);   // about -10 dB, as in the Java
    m_music->setLoops(QMediaPlayer::Infinite);

    // The file is only checked here. It is opened the first time music is wanted, so
    // starting the program does not read 13 MB that the menu never plays.
    m_musicAvailable = QFile::exists(musicFile);
    if (!m_musicAvailable)
        qWarning() << "SoundManager: no music file at" << musicFile << "- playing without music";
}

bool SoundManager::soundOn() const
{
    return m_soundOn;
}

bool SoundManager::musicOn() const
{
    return m_musicOn;
}

bool SoundManager::isMusicPlaying() const
{
    return m_musicPlaying;
}

void SoundManager::playClick()
{
    playEffect(m_click, "click");
}

void SoundManager::playMeow(bool happy)
{
    if (happy)
        playEffect(m_meowHappy, "meowHappy");
    else
        playEffect(m_meowAngry, "meowAngry");
}

void SoundManager::startMusic()
{
    setMusicPlaying(m_soundOn && m_musicOn);
}

void SoundManager::stopMusic()
{
    setMusicPlaying(false);
}

void SoundManager::toggleSound()
{
    m_soundOn = !m_soundOn;
    m_musicOn = m_soundOn;   // the music follows the master switch, in both directions
    setMusicPlaying(m_musicOn);
    emit stateChanged(m_soundOn, m_musicOn);
}

void SoundManager::toggleMusic()
{
    if (!m_soundOn)
        return;

    m_musicOn = !m_musicOn;
    setMusicPlaying(m_musicOn);
    emit stateChanged(m_soundOn, m_musicOn);
}

void SoundManager::playEffect(QSoundEffect *effect, const QString &name)
{
    if (!m_soundOn)
        return;   // the master switch silences effects too

    effect->play();
    emit effectPlayed(name);
}

void SoundManager::setMusicPlaying(bool playing)
{
    if (playing && m_musicAvailable) {
        if (m_music->source().isEmpty())
            m_music->setSource(QUrl::fromLocalFile(m_musicFile));
        m_music->play();
        m_musicPlaying = true;
    } else {
        m_music->pause();   // pause, not stop: it carries on from the same place later
        m_musicPlaying = false;
    }
}
