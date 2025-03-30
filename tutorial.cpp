#include "tutorial.h"
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QDebug>
#include <QKeyEvent>
#include <QScreen>
#include <QApplication>
#include <QSurfaceFormat>

Tutorial::Tutorial(QWidget *parent) : QWidget(parent), 
    currentStep(TutorialStep::Welcome), 
    waitingForAction(false) {
    
    // Set up software rendering to avoid OpenGL issues
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(format);
    
    // Create main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Create stacked widget for tutorial steps
    stackedWidget = new QStackedWidget(this);
    
    // Setup welcome widget
    setupWelcomeWidget();
    
    // Setup game widget
    setupGameWidget();
    
    // Add widgets to stacked widget
    stackedWidget->addWidget(welcomeWidget);
    stackedWidget->addWidget(gameWidget);
    
    // Add stacked widget to main layout
    mainLayout->addWidget(stackedWidget);
    
    // Create instruction timer
    instructionTimer = new QTimer(this);
    connect(instructionTimer, &QTimer::timeout, this, &Tutorial::showNextInstruction);
    
    // Set window properties for 1920x1080 resolution
    setMinimumSize(1280, 720);
    
    // Try to set the window to fullscreen or maximize it
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        setGeometry(screenGeometry);
    }
    
    setWindowTitle("Robot Arena Interactive Tutorial");
    
    // Show welcome screen initially
    stackedWidget->setCurrentWidget(welcomeWidget);
}

Tutorial::~Tutorial() {
    // Clean up if needed
}

void Tutorial::setupWelcomeWidget() {
    welcomeWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(welcomeWidget);
    
    // Create title
    welcomeTitleLabel = new QLabel("Learn Robot Arena By Playing", welcomeWidget);
    QFont titleFont("Arial", 32, QFont::Bold);
    welcomeTitleLabel->setFont(titleFont);
    welcomeTitleLabel->setAlignment(Qt::AlignCenter);
    welcomeTitleLabel->setStyleSheet("color: #2c3e50;");
    
    // Create description
    welcomeDescriptionLabel = new QLabel(welcomeWidget);
    welcomeDescriptionLabel->setWordWrap(true);
    welcomeDescriptionLabel->setTextFormat(Qt::RichText);
    welcomeDescriptionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    QFont descFont("Arial", 16);
    welcomeDescriptionLabel->setFont(descFont);
    welcomeDescriptionLabel->setStyleSheet("color: #2c3e50;");
    
    welcomeDescriptionLabel->setText(
        "<h2>Welcome to the Interactive Tutorial!</h2>"
        "<p>In this tutorial, you'll learn how to play Robot Arena by actually playing the game. "
        "You'll receive guidance and instructions as you play, making it easy to learn the game mechanics.</p>"
        "<p>Robot Arena is a turn-based strategy game where you control a robot and battle against an opponent. "
        "The goal is to defeat the enemy robot by reducing its health to zero.</p>"
        "<p>Click 'Start Tutorial' to begin your journey!</p>"
    );
    
    // Create buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    startButton = new QPushButton("Start Tutorial", welcomeWidget);
    backButton = new QPushButton("Back to Main Menu", welcomeWidget);
    
    // Style buttons
    QString buttonStyle = "QPushButton {"
                         "    background-color: #4CAF50;"
                         "    color: white;"
                         "    border-radius: 5px;"
                         "    padding: 15px 30px;"
                         "    min-width: 200px;"
                         "    font-size: 18px;"
                         "}"
                         "QPushButton:hover {"
                         "    background-color: #45a049;"
                         "}"
                         "QPushButton:pressed {"
                         "    background-color: #3e8e41;"
                         "}";
    
    startButton->setStyleSheet(buttonStyle);
    backButton->setStyleSheet(buttonStyle);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    
    // Add widgets to layout with spacing for 1080p
    layout->addSpacing(50);
    layout->addWidget(welcomeTitleLabel);
    layout->addSpacing(30);
    layout->addWidget(welcomeDescriptionLabel, 1);
    layout->addSpacing(50);
    layout->addLayout(buttonLayout);
    layout->addSpacing(100);
    
    // Set background color
    welcomeWidget->setStyleSheet("background-color: #ecf0f1;");
    
    // Connect signals
    connect(startButton, &QPushButton::clicked, this, &Tutorial::handleStartTutorial);
    connect(backButton, &QPushButton::clicked, this, &Tutorial::backButtonClicked);
}

void Tutorial::setupGameWidget() {
    gameWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(gameWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Create game grid
    gameGrid = new GameGrid(gameWidget);
    
    // Create navigation buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    prevButton = new QPushButton("Previous Tip", gameWidget);
    nextButton = new QPushButton("Next Tip", gameWidget);
    
    // Style buttons
    QString buttonStyle = "QPushButton {"
                         "    background-color: #2196F3;"
                         "    color: white;"
                         "    border-radius: 5px;"
                         "    padding: 10px 20px;"
                         "    min-width: 150px;"
                         "    font-size: 16px;"
                         "}"
                         "QPushButton:hover {"
                         "    background-color: #0b7dda;"
                         "}"
                         "QPushButton:pressed {"
                         "    background-color: #0a6fc2;"
                         "}"
                         "QPushButton:disabled {"
                         "    background-color: #cccccc;"
                         "    color: #666666;"
                         "}";
    
    prevButton->setStyleSheet(buttonStyle);
    nextButton->setStyleSheet(buttonStyle);
    
    buttonLayout->addWidget(prevButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(nextButton);
    
    // Create a container for the buttons
    QWidget* buttonContainer = new QWidget(gameWidget);
    buttonContainer->setLayout(buttonLayout);
    buttonContainer->setFixedHeight(60);
    buttonContainer->setStyleSheet("background-color: #2c3e50;");
    
    // Add widgets to layout
    layout->addWidget(gameGrid, 1);
    layout->addWidget(buttonContainer, 0);
    
    // Connect signals
    connect(prevButton, &QPushButton::clicked, this, &Tutorial::handlePreviousStep);
    connect(nextButton, &QPushButton::clicked, this, &Tutorial::handleNextStep);
    connect(gameGrid, &GameGrid::gameOver, this, &Tutorial::handleGameOver);
    
    // Connect to key press events from game grid
    connect(gameGrid, &GameGrid::keyPressed, this, &Tutorial::handleKeyPress);
}

void Tutorial::handleStartTutorial() {
    // Start the tutorial from the beginning
    currentStep = TutorialStep::RobotTypes;
    startTutorialGame();
}

void Tutorial::startTutorialGame() {
    // Reset and initialize the game
    resetTutorialGame();
    
    // Show the game widget
    stackedWidget->setCurrentWidget(gameWidget);
    
    // Update instructions for the current step
    updateInstructions();
    
    // Set focus to the game grid
    if (gameGrid) {
        gameGrid->setFocus();
    }
    
    // Start with previous button disabled
    prevButton->setEnabled(false);
}

void Tutorial::handleNextStep() {
    // Move to next tutorial step
    switch (currentStep) {
        case TutorialStep::Welcome:
            currentStep = TutorialStep::RobotTypes;
            break;
        case TutorialStep::RobotTypes:
            currentStep = TutorialStep::Movement;
            break;
        case TutorialStep::Movement:
            currentStep = TutorialStep::Turning;
            break;
        case TutorialStep::Turning:
            currentStep = TutorialStep::Attacking;
            break;
        case TutorialStep::Attacking:
            currentStep = TutorialStep::Walls;
            break;
        case TutorialStep::Walls:
            currentStep = TutorialStep::Powerups;
            break;
        case TutorialStep::Powerups:
            currentStep = TutorialStep::Strategy;
            break;
        case TutorialStep::Strategy:
            // Handle transition to complete step carefully
            currentStep = TutorialStep::Complete;
            break;
        case TutorialStep::Complete:
            // Return to main menu
            emit backButtonClicked();
            return;
    }
    
    // Reset waiting state
    waitingForAction = false;
    
    try {
        // Reset and update the game for the new step
        resetTutorialGame();
        
        // Update UI for the new step
        updateInstructions();
        
        // Set focus to the game grid
        if (gameGrid) {
            gameGrid->setFocus();
        }
    } catch (...) {
        // If any error occurs during initialization, go back to main menu
        emit backButtonClicked();
        return;
    }
    
    // Enable/disable navigation buttons
    prevButton->setEnabled(currentStep != TutorialStep::RobotTypes);
    nextButton->setEnabled(true);
}

void Tutorial::handlePreviousStep() {
    // Move to previous tutorial step
    switch (currentStep) {
        case TutorialStep::Welcome:
            // Already at first step
            break;
        case TutorialStep::RobotTypes:
            // Go back to welcome screen
            stackedWidget->setCurrentWidget(welcomeWidget);
            return;
        case TutorialStep::Movement:
            currentStep = TutorialStep::RobotTypes;
            break;
        case TutorialStep::Turning:
            currentStep = TutorialStep::Movement;
            break;
        case TutorialStep::Attacking:
            currentStep = TutorialStep::Turning;
            break;
        case TutorialStep::Walls:
            currentStep = TutorialStep::Attacking;
            break;
        case TutorialStep::Powerups:
            currentStep = TutorialStep::Walls;
            break;
        case TutorialStep::Strategy:
            currentStep = TutorialStep::Powerups;
            break;
        case TutorialStep::Complete:
            currentStep = TutorialStep::Strategy;
            break;
    }
    
    // Reset waiting state
    waitingForAction = false;
    
    // Reset and update the game for the new step
    resetTutorialGame();
    
    // Update UI for the new step
    updateInstructions();
    
    // Set focus to the game grid
    if (gameGrid) {
        gameGrid->setFocus();
    }
    
    // Enable/disable navigation buttons
    prevButton->setEnabled(currentStep != TutorialStep::RobotTypes);
    nextButton->setEnabled(true);
}

void Tutorial::handleGameOver(bool playerWon) {
    // Show game over message
    QString message;
    if (playerWon) {
        message = "<h3>Congratulations! You Won!</h3>"
                  "<p>You've successfully completed this part of the tutorial.</p>"
                  "<p>Click 'Next Tip' to continue learning.</p>";
    } else {
        message = "<h3>You Were Defeated!</h3>"
                  "<p>Don't worry, learning takes practice.</p>"
                  "<p>Click 'Next Tip' to continue or try again with the same lesson.</p>";
    }
    
    gameGrid->setInfoPanelMessage(message);
    
    // Reset waiting state
    waitingForAction = false;
}

void Tutorial::handleKeyPress(int key) {
    if (!waitingForAction) return;
    
    bool actionCompleted = false;
    
    // Check if the user completed the required action based on current step
    switch (currentStep) {
        case TutorialStep::Movement:
            actionCompleted = checkMovementAction(key);
            break;
        case TutorialStep::Turning:
            actionCompleted = checkTurningAction(key);
            break;
        case TutorialStep::Attacking:
            actionCompleted = checkAttackAction(key);
            break;
        default:
            // Other steps don't have specific key actions to check
            break;
    }
    
    // If action completed, show success message and move to next instruction
    if (actionCompleted) {
        waitingForAction = false;
        
        // Update the info panel message with success
        QString currentMessage = gameGrid->infoMessageLabel->text();
        gameGrid->setInfoPanelMessage(currentMessage + "<br><br><b>Great job!</b> You've completed this action.");
        
        // Schedule next instruction after a short delay
        instructionTimer->start(2000);
    }
}

bool Tutorial::checkMovementAction(int key) {
    // Check if the user pressed W to move forward
    return key == Qt::Key_W;
}

bool Tutorial::checkTurningAction(int key) {
    // Check if the user pressed A or D to turn
    return key == Qt::Key_A || key == Qt::Key_D;
}

bool Tutorial::checkAttackAction(int key) {
    // Check if the user pressed Space to attack
    return key == Qt::Key_Space;
}

void Tutorial::showNextInstruction() {
    // Stop the timer
    instructionTimer->stop();
    
    // Move to the next instruction within the current step
    // This is a simplified version - in a real implementation, 
    // you might have multiple instructions per step
    handleNextStep();
}

void Tutorial::resetTutorialGame() {
    // Safely disconnect signals before deleting
    if (gameGrid) {
        disconnect(gameGrid, &GameGrid::gameOver, this, &Tutorial::handleGameOver);
        disconnect(gameGrid, &GameGrid::keyPressed, this, &Tutorial::handleKeyPress);
        
        // Remove from layout
        static_cast<QVBoxLayout*>(gameWidget->layout())->removeWidget(gameGrid);
        
        // Delete the old grid
        gameGrid->deleteLater();
    }
    
    // Create new game grid
    gameGrid = new GameGrid(gameWidget);
    static_cast<QVBoxLayout*>(gameWidget->layout())->insertWidget(0, gameGrid, 1);
    
    // Set focus to the new game grid
    gameGrid->setFocus();
    
    // Connect signals
    connect(gameGrid, &GameGrid::gameOver, this, &Tutorial::handleGameOver);
    connect(gameGrid, &GameGrid::keyPressed, this, &Tutorial::handleKeyPress);
    
    // Special case for Complete step - don't try to initialize an actual game
    if (currentStep == TutorialStep::Complete) {
        // Just show the completion message
        updateInstructions();
        return;
    }
    
    // Initialize game based on current tutorial step
    RobotType playerType = RobotType::Scout;
    RobotType aiType = RobotType::Scout;
    GameDifficulty difficulty = GameDifficulty::Easy;
    MapType mapType = MapType::Open;
    
    switch (currentStep) {
        case TutorialStep::RobotTypes:
            // Use Scout robot vs. easy AI
            playerType = RobotType::Scout;
            aiType = RobotType::Scout;
            difficulty = GameDifficulty::Easy;
            mapType = MapType::Open;
            break;
            
        case TutorialStep::Movement:
            // Use scout (fast movement) vs. easy AI
            playerType = RobotType::Scout;
            aiType = RobotType::Tank;
            difficulty = GameDifficulty::Easy;
            mapType = MapType::Open;
            break;
            
        case TutorialStep::Turning:
            // Use Scout robot in a maze map
            playerType = RobotType::Scout;
            aiType = RobotType::Scout;
            difficulty = GameDifficulty::Easy;
            mapType = MapType::Maze;
            break;
            
        case TutorialStep::Attacking:
            // Use sniper for long-range attacks
            playerType = RobotType::Sniper;
            aiType = RobotType::Scout;
            difficulty = GameDifficulty::Easy;
            mapType = MapType::Open;
            break;
            
        case TutorialStep::Walls:
            // Use tank to break walls
            playerType = RobotType::Tank;
            aiType = RobotType::Scout;
            difficulty = GameDifficulty::Easy;
            mapType = MapType::Fortress;
            break;
            
        case TutorialStep::Powerups:
            // Use Scout in a map with pickups
            playerType = RobotType::Scout;
            aiType = RobotType::Scout;
            difficulty = GameDifficulty::Easy;
            mapType = MapType::Open;  // Open map for better visibility of pickups
            break;
            
        case TutorialStep::Strategy:
            // Use Scout robot vs. medium AI
            playerType = RobotType::Scout;
            aiType = RobotType::Scout;
            difficulty = GameDifficulty::Medium;
            mapType = MapType::Random;
            break;
            
        default:
            break;
    }
    
    // Initialize the game
    gameGrid->initializeWithRobotType(playerType, aiType, difficulty, mapType);
    
    // For the Powerups tutorial step, make sure to add pickups to the map
    if (currentStep == TutorialStep::Powerups) {
        // Ensure the game has sufficient pickups for demonstration
        auto game = gameGrid->getGame();
        if (game) {
            // Add health pickups
            game->spawnHealthPickup(3);
            
            // Add weapon powerups (one of each type)
            QPoint pos(5, 5);
            game->placePowerUpAtPosition(pos, CellType::LaserPowerUp);
            
            pos = QPoint(8, 5);
            game->placePowerUpAtPosition(pos, CellType::MissilePowerUp);
            
            pos = QPoint(11, 5);
            game->placePowerUpAtPosition(pos, CellType::BombPowerUp);
            
            // Update grid to show the new pickups
            gameGrid->updateGrid();
        }
    }
    
    // Update the info panel with the appropriate instructions
    updateInstructions();
}

void Tutorial::updateInstructions() {
    // Update instructions based on current step
    switch (currentStep) {
        case TutorialStep::RobotTypes:
            setupRobotTypesStep();
            break;
        case TutorialStep::Movement:
            setupMovementStep();
            break;
        case TutorialStep::Turning:
            setupTurningStep();
            break;
        case TutorialStep::Attacking:
            setupAttackingStep();
            break;
        case TutorialStep::Walls:
            setupWallsStep();
            break;
        case TutorialStep::Powerups:
            setupPowerupsStep();
            break;
        case TutorialStep::Strategy:
            setupStrategyStep();
            break;
        case TutorialStep::Complete:
            setupCompleteStep();
            break;
        default:
            break;
    }
}

void Tutorial::setupWelcomeStep() {
    // This is handled by the welcome widget
}

void Tutorial::setupRobotTypesStep() {
    gameGrid->setInfoPanelMessage(
        "<h3>Robot Types</h3>"
        "<p>You're controlling a <b>Scout Robot</b> (blue), which is fast with 3 moves per turn.</p>"
        "<p>Other robot types include Scout (fast), Tank (strong), and Sniper (long-range).</p>"
    );
    
    // Not waiting for a specific action in this step
    waitingForAction = false;
}

void Tutorial::setupMovementStep() {
    gameGrid->setInfoPanelMessage(
        "<h3>Movement</h3>"
        "<p>Now you're controlling a <b>Scout Robot</b> (blue), which is fast with 3 moves per turn.</p>"
        "<p><b>Press W</b> to move forward in the direction your robot is facing.</p>"
        "<p>Try moving around the arena now!</p>"
    );
    
    // Wait for the user to press W
    waitingForAction = true;
}

void Tutorial::setupTurningStep() {
    gameGrid->setInfoPanelMessage(
        "<h3>Turning</h3>"
        "<p>You're in a maze! To navigate, you need to turn your robot.</p>"
        "<p><b>Press A</b> to turn left (counter-clockwise).</p>"
        "<p><b>Press D</b> to turn right (clockwise).</p>"
        "<p>Turning doesn't consume moves, so you can freely change direction.</p>"
        "<p>Try turning now!</p>"
    );
    
    // Wait for the user to press A or D
    waitingForAction = true;
}

void Tutorial::setupAttackingStep() {
    gameGrid->setInfoPanelMessage(
        "<h3>Attacking</h3>"
        "<p>You're controlling a <b>Sniper Robot</b> (green), which can attack from 3 tiles away!</p>"
        "<p>The Sniper has 2 moves per turn and high damage, making it excellent for hit-and-run tactics.</p>"
        "<p><b>Press Space</b> to attack in the direction you're facing.</p>"
        "<p>You need a clear line of sight to hit the enemy (no walls in between).</p>"
        "<p>Try attacking the enemy robot!</p>"
    );
    
    // Wait for the user to press Space
    waitingForAction = true;
}

void Tutorial::setupWallsStep() {
    gameGrid->setInfoPanelMessage(
        "<h3>Walls and Obstacles</h3>"
        "<p>You're controlling a <b>Tank Robot</b> (red), which has high health and damage.</p>"
        "<p>Walls block movement and line of sight, but you can destroy them by attacking.</p>"
        "<p>Position yourself facing a wall, then press Space to attack it.</p>"
        "<p>The Tank is slow (1 move per turn) but can break walls in a single hit!</p>"
    );
    
    // Not waiting for a specific action in this step
    waitingForAction = false;
}

void Tutorial::setupPowerupsStep() {
    gameGrid->setInfoPanelMessage(
        "<h3>Health Pickups & Weapon Powerups</h3>"
        "<p>The green tiles are <b>Health Pickups</b> that restore 20 HP when collected.</p>"
        "<p>The colored tiles are <b>Weapon Powerups</b> that grant special attacks:</p>"
        "<ul>"
        "<li><b>Laser (Red)</b>: Fires in a straight line, damaging everything in its path.</li>"
        "<li><b>Missile (Orange)</b>: Long-range attack that deals high damage.</li>"
        "<li><b>Bomb (Purple)</b>: Explosive attack that damages all adjacent tiles.</li>"
        "</ul>"
        "<p>To collect these items, simply move your robot onto their tile.</p>"
        "<p>Once you have a powerup, press Space to use it instead of your normal attack.</p>"
        "<p>Explore the map and try to collect these items!</p>"
    );
    
    // Not waiting for a specific action in this step
    waitingForAction = false;
}

void Tutorial::setupStrategyStep() {
    gameGrid->setInfoPanelMessage(
        "<h3>Strategy Tips</h3>"
        "<p>Now you're facing a medium difficulty AI. Use everything you've learned!</p>"
        "<ul>"
        "<li>Use walls for cover when needed</li>"
        "<li>Collect health pickups when your health is low</li>"
        "<li>Use weapon powerups strategically</li>"
        "<li>Try to attack when the enemy can't immediately retaliate</li>"
        "<li>Plan your moves to maximize efficiency</li>"
        "<li>Different robots have different strengths - choose based on your playstyle</li>"
        "</ul>"
        "<p>Good luck!</p>"
    );
    
    // Not waiting for a specific action in this step
    waitingForAction = false;
}

void Tutorial::setupCompleteStep() {
    if (!gameGrid) {
        return; // Safety check
    }
    
    gameGrid->setInfoPanelMessage(
        "<h3>Congratulations!</h3>"
        "<p>You've completed the interactive tutorial for Robot Arena!</p>"
        "<p>You now know how to:</p>"
        "<ul>"
        "<li>Choose different robot types</li>"
        "<li>Move and turn your robot</li>"
        "<li>Attack enemies and destroy walls</li>"
        "<li>Collect health pickups and use special weapons</li>"
        "<li>Use strategy to win battles</li>"
        "</ul>"
        "<p>Click 'Next Tip' to return to the main menu and start playing!</p>"
    );
    
    // Not waiting for a specific action in this step
    waitingForAction = false;
} 