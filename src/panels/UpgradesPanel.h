#pragma once

// UpgradesPanel: spend money to bring customers faster, walk faster, earn tips, or raise
// the limit on the other three.
// Replaces: UpgradesPanel.java.
// Guide: chapter 13 (signals and slots, including lambdas).
//
// The Java kept four sets of variables (advertisementLevel, energyLevel, ...) and four
// near-identical branches. Here the four upgrades are four rows of the same small struct.

#include <QList>
#include <QPoint>
#include <QWidget>

class CafeScene;
class ImageButton;

enum class Upgrade { Advertisement, Energy, Cuteness, Meowmax };

class UpgradesPanel : public QWidget
{
    Q_OBJECT

public:
    explicit UpgradesPanel(CafeScene *scene, QWidget *parent = nullptr);

    int level(Upgrade upgrade) const;
    int cost(Upgrade upgrade) const;
    int maxLevel(Upgrade upgrade) const;   // 5 (8, then 11, after Meowmax); always 2 for Meowmax

public slots:
    void buy(Upgrade upgrade);             // does nothing if it is too dear or already at its limit

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void refreshButtons();

private:
    // Everything one upgrade needs to know about itself.
    struct Entry
    {
        int level;
        int cost;
        int costStep;           // how much dearer it gets each time
        QPoint textAt;          // where its two lines of text are drawn
        ImageButton *button;
    };

    void addEntry(const QString &buttonName, const QRect &buttonRect, int firstCost, int costStep,
                  const QPoint &textAt, Upgrade upgrade);
    const Entry &entry(Upgrade upgrade) const;

    CafeScene *m_scene;
    QList<Entry> m_entries;      // in the same order as the Upgrade enum
    int m_sharedMaxLevel = 5;    // the limit for everything except Meowmax
};
