#ifndef ROBOTSELECTOR_H
#define ROBOTSELECTOR_H

#include <QWidget>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include "robot.h"

///@brief This class will be used when the player chooses not to play multiplayer
///@author Group 17
class RobotSelector : public QWidget {
    Q_OBJECT
public:
    /// @brief Constructor function
    /// @param parent - QWidget parent of this object
    explicit RobotSelector(QWidget *parent = nullptr);
    /// @brief Getter method that returns what robot the player is using
    /// @return The robot chosen by the player in the robot-selection screen
    RobotType getSelectedPlayerType() const;
    /// @brief Getter function the returns what robot the AI is using
    /// @return The robot the AI chose in the robot-selection screen
    RobotType getSelectedAIType() const;
    /// @brief Getter method that returns whether the AI should be choosing it's own robot
    /// @return TRUE if the AI should choose a randomised robot, FALSE otherwise
    bool isRandomAI() const;

signals:
    void robotsSelected(RobotType playerType, RobotType aiType, bool randomAI);
    void backButtonClicked();

private:
    // Player robot selection
    QGroupBox* playerGroup;
    QRadioButton* playerScoutBtn;
    QRadioButton* playerTankBtn;
    QRadioButton* playerSniperBtn;
    QLabel* playerDescriptionLabel;
    
    // AI robot selection
    QGroupBox* aiGroup;
    QRadioButton* aiScoutBtn;
    QRadioButton* aiTankBtn;
    QRadioButton* aiSniperBtn;
    QRadioButton* aiRandomBtn;
    QLabel* aiDescriptionLabel;
    
    QPushButton* selectButton;
    QPushButton* backButton;

private slots:
    void updatePlayerDescription();
    void updateAIDescription();
    void onSelectClicked();
    void onBackClicked();
};

#endif // ROBOTSELECTOR_H
