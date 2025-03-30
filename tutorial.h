#ifndef TUTORIAL_H
#define TUTORIAL_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QTimer>
#include "gamegrid.h"
#include "robot.h"

///@brief Enum to track tutorial steps
enum class TutorialStep {
    Welcome,
    RobotTypes,
    Movement,
    Turning,
    Attacking,
    Walls,
    Powerups,
    Strategy,
    Complete
};

/// @brief This class shows the tutorial of the game
class Tutorial : public QWidget {
    Q_OBJECT
public:
    /// @brief Constructor class
    /// @param parent - The Qwidget parent of the object
    explicit Tutorial(QWidget *parent = nullptr);
    ~Tutorial();

signals:
    void backButtonClicked();

private slots:
    void handleNextStep();
    void handlePreviousStep();
    void handleGameOver(bool playerWon);
    void handleStartTutorial();
    void showNextInstruction();
    void handleKeyPress(int key);

private:
    QStackedWidget* stackedWidget;
    QWidget* welcomeWidget;
    QWidget* gameWidget;
    
    // Welcome screen widgets
    QLabel* welcomeTitleLabel;
    QLabel* welcomeDescriptionLabel;
    QPushButton* startButton;
    QPushButton* backButton;
    
    // Game screen widgets
    GameGrid* gameGrid;
    QPushButton* nextButton;
    QPushButton* prevButton;
    
    TutorialStep currentStep;
    bool waitingForAction;
    QTimer* instructionTimer;
    
    void setupWelcomeWidget();
    void setupGameWidget();
    void startTutorialGame();
    void resetTutorialGame();
    void updateInstructions();
    
    // Tutorial step handlers
    void setupWelcomeStep();
    void setupRobotTypesStep();
    void setupMovementStep();
    void setupTurningStep();
    void setupAttackingStep();
    void setupWallsStep();
    void setupPowerupsStep();
    void setupStrategyStep();
    void setupCompleteStep();
    
    // Check if user completed the required action
    bool checkMovementAction(int key);
    bool checkTurningAction(int key);
    bool checkAttackAction(int key);
};

#endif // TUTORIAL_H 