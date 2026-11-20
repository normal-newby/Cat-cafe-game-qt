#pragma once

// RestockingPanel: buy more of an item for its counter.
// Replaces: RestockingPanel.java.
// Guide: chapter 13 (signals and slots, including lambdas) and chapter 14 (painting).
//
// The Java panel asked Game for the player's money every time it repainted. This panel
// is handed the scene once, and is *told* when money or stock changes, by signals.

#include <QString>
#include <QWidget>

class CafeScene;
class ImageButton;
class QComboBox;

class RestockingPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RestockingPanel(CafeScene *scene, QWidget *parent = nullptr);

    QString selectedItem() const;   // "boba" when the list shows "boba counter"
    int quantity() const;           // how many are about to be bought
    int unitCost() const;
    int totalCost() const;
    bool canAfford() const;
    int remaining() const;          // how many the chosen counter holds right now

public slots:
    void changeQuantity(int delta);   // +1, -1, +10 or -10; never goes below zero
    void restock();                   // buy them, if something is chosen and affordable

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void refresh();

private:
    CafeScene *m_scene;
    QComboBox *m_combo;
    ImageButton *m_restockButton;
    int m_quantity = 0;
};
