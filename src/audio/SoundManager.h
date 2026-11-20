#pragma once

// SoundManager: every sound the game makes, and the two switches (sound, music).
// Replaces: Sounds.java (a class of statics that anyone could call from anywhere).
// Guide: chapter 19 (sound) and chapter 13 (signals and slots).
//
// Nothing in the game calls this class directly. MainWindow connects other objects'
// signals to these slots: "a button was hovered" -> playClick(), "an order was served"
// -> playMeow(). So the buttons and the cafe do not know that sound exists.

#include <QObject>
#include <QString>

class QAudioOutput;
class QMediaPlayer;
class QSoundEffect;

class SoundManager : public QObject
{
    Q_OBJECT

public:
    // `musicFile` is the path of the background music on disk. If there is no such
    // file the game simply has no music.
    explicit SoundManager(const QString &musicFile, QObject *parent = nullptr);

    bool soundOn() const;          // the master switch
    bool musicOn() const;          // the music switch
    bool isMusicPlaying() const;   // whether this object has asked for music to play

public slots:
    void playClick();
    void playMeow(bool happy);

    void startMusic();     // entering the game; plays only if both switches are on
    void stopMusic();      // leaving the game; the switches keep their positions

    void toggleSound();    // off silences everything; on brings the music back too
    void toggleMusic();    // does nothing while sound is off

signals:
    void stateChanged(bool soundOn, bool musicOn);
    void effectPlayed(const QString &effect);   // "click", "meowHappy" or "meowAngry"

private:
    void playEffect(QSoundEffect *effect, const QString &name);
    void setMusicPlaying(bool playing);

    QSoundEffect *m_click;
    QSoundEffect *m_meowHappy;
    QSoundEffect *m_meowAngry;

    QMediaPlayer *m_music;
    QAudioOutput *m_musicOutput;
    QString m_musicFile;
    bool m_musicAvailable = false;

    bool m_soundOn = true;
    bool m_musicOn = true;
    bool m_musicPlaying = false;
};
