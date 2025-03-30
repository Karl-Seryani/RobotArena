#ifndef MULTIPLAYERROBOTSELECTOR_H
#define MULTIPLAYERROBOTSELECTOR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include "robot.h"

/// @brief This class is used for multiplayer robot selection
///@author Group 17
class MultiplayerRobotSelector : public QWidget {
    Q_OBJECT
public:
    /// @brief Constructor for the class, displays a list of tanks that Player 1&2 can choose to use
    /// @param parent - The parent QWidget of this object
    explicit MultiplayerRobotSelector(QWidget *parent = nullptr);

signals:
    void robotsSelected(RobotType player1Type, RobotType player2Type);
    void backButtonClicked();

private slots:
    void onSelectClicked();
    void onBackClicked();
    void updatePreview();

private:
    QLabel* createRobotPreview(RobotType type);
    QString getRobotDescription(RobotType type);
    
    // Player 1 selection
    QGroupBox* player1Group;
    QRadioButton* p1ScoutBtn;
    QRadioButton* p1TankBtn;
    QRadioButton* p1SniperBtn;
    QLabel* p1PreviewLabel;
    QLabel* p1DescriptionLabel;
    
    // Player 2 selection
    QGroupBox* player2Group;
    QRadioButton* p2ScoutBtn;
    QRadioButton* p2TankBtn;
    QRadioButton* p2SniperBtn;
    QLabel* p2PreviewLabel;
    QLabel* p2DescriptionLabel;
    
    // Buttons
    QPushButton* selectButton;
    QPushButton* backButton;
};

#endif // MULTIPLAYERROBOTSELECTOR_H 