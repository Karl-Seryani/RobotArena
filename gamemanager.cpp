#include "gamemanager.h"
#include "logger.h"
#include <QRandomGenerator>
#include <QPlainTextEdit>

GameManager::GameManager(QObject *parent) : QObject(parent) {
    // Create the main stacked widget
    mainWidget = new QStackedWidget();
    mainWidget->setWindowTitle("Robot Arena");
    
    // Create screens
    mainMenu = new MainMenu();
    robotSelector = new RobotSelector();
    multiplayerRobotSelector = new MultiplayerRobotSelector();
    tutorial = new Tutorial();
    difficultySelector = new DifficultySelector();
    mapSelector = new MapSelector();
    gameOverScreen = nullptr; // Will be created when needed
    gameGrid = nullptr; // Will be created when needed
    
    // Add screens to stacked widget
    mainWidget->addWidget(mainMenu);
    mainWidget->addWidget(robotSelector);
    mainWidget->addWidget(multiplayerRobotSelector);
    mainWidget->addWidget(tutorial);
    mainWidget->addWidget(difficultySelector);
    mainWidget->addWidget(mapSelector);
    
    // Connect signals
    connect(mainMenu, &MainMenu::singlePlayerSelected, this, &GameManager::handleSinglePlayerSelected);
    connect(mainMenu, &MainMenu::multiplayerSelected, this, &GameManager::handleMultiplayerSelected);
    connect(mainMenu, &MainMenu::tutorialSelected, this, &GameManager::handleTutorialSelected);
    
    connect(robotSelector, &RobotSelector::robotsSelected, this, &GameManager::handleRobotsSelected);
    connect(robotSelector, &RobotSelector::backButtonClicked, this, &GameManager::handleBackButtonClicked);
    
    connect(multiplayerRobotSelector, &MultiplayerRobotSelector::robotsSelected, this, &GameManager::handleMultiplayerRobotsSelected);
    connect(multiplayerRobotSelector, &MultiplayerRobotSelector::backButtonClicked, this, &GameManager::handleMultiplayerRobotSelectorBackClicked);
    
    connect(tutorial, &Tutorial::backButtonClicked, this, &GameManager::handleTutorialBackButtonClicked);
    
    connect(difficultySelector, &DifficultySelector::difficultySelected, this, &GameManager::handleDifficultySelected);
    connect(difficultySelector, &DifficultySelector::backButtonClicked, this, &GameManager::handleDifficultySelectorBackClicked);
    
    connect(mapSelector, &MapSelector::mapSelected, this, &GameManager::handleMapSelected);
    connect(mapSelector, &MapSelector::backButtonClicked, this, &GameManager::handleMapSelectorBackClicked);
    
    // Set initial screen
    showMainMenu();
    
    // Set window size
    mainWidget->setMinimumSize(600, 600);
    
    // Initialize flags
    isMultiplayerMode = false;
    
    // Create and show the logging window
    logWindow = new QPlainTextEdit();
    logWindow->setWindowTitle("AI Decision Log");
    logWindow->setReadOnly(true);
    logWindow->resize(400, 600);
    logWindow->show();
    
    // Set the logging widget in the Logger
    Logger::setLogWidget(logWindow);
}

GameManager::~GameManager() {
    delete mainWidget; // This will delete all child widgets
    delete logWindow;
}

void GameManager::showMainMenu() {
    mainWidget->setCurrentWidget(mainMenu);
}

void GameManager::handleSinglePlayerSelected() {
    isMultiplayerMode = false;
    mainWidget->setCurrentWidget(robotSelector);
}

void GameManager::handleMultiplayerSelected() {
    isMultiplayerMode = true;
    mainWidget->setCurrentWidget(multiplayerRobotSelector);
}

void GameManager::handleTutorialSelected() {
    mainWidget->setCurrentWidget(tutorial);
}

void GameManager::handleRobotsSelected(RobotType playerType, RobotType aiType, bool randomAI) {
    selectedPlayerType = playerType;
    selectedAIType = aiType;
    isRandomAI = randomAI;
    mainWidget->setCurrentWidget(difficultySelector);
}

void GameManager::handleMultiplayerRobotsSelected(RobotType player1Type, RobotType player2Type) {
    selectedPlayerType = player1Type;
    selectedPlayer2Type = player2Type;
    mainWidget->setCurrentWidget(mapSelector);
}

void GameManager::handleDifficultySelected(GameDifficulty difficulty) {
    selectedDifficulty = difficulty;
    mainWidget->setCurrentWidget(mapSelector);
}

void GameManager::handleMapSelected(MapType mapType) {
    selectedMapType = mapType;
    
    if (isMultiplayerMode) {
        handleMultiplayerMapSelected(mapType);
        return;
    }
    
    if (gameGrid) {
        disconnect(gameGrid, &GameGrid::gameOver, this, &GameManager::handleGameOver);
        mainWidget->removeWidget(gameGrid);
        delete gameGrid;
    }
    
    gameGrid = new GameGrid();
    mainWidget->addWidget(gameGrid);
    
    connect(gameGrid, &GameGrid::gameOver, this, &GameManager::handleGameOver);
    
    RobotType finalAIType;
    if (isRandomAI) {
        finalAIType = static_cast<RobotType>(QRandomGenerator::global()->bounded(4));
    } else {
        finalAIType = selectedAIType;
    }
    
    gameGrid->initializeWithRobotType(selectedPlayerType, finalAIType, selectedDifficulty, selectedMapType);
    mainWidget->setCurrentWidget(gameGrid);
}

void GameManager::handleMultiplayerMapSelected(MapType mapType) {
    selectedMapType = mapType;
    
    if (gameGrid) {
        disconnect(gameGrid, &GameGrid::gameOver, this, &GameManager::handleGameOver);
        mainWidget->removeWidget(gameGrid);
        delete gameGrid;
    }
    
    gameGrid = new GameGrid();
    mainWidget->addWidget(gameGrid);
    
    connect(gameGrid, &GameGrid::gameOver, this, &GameManager::handleGameOver);
    
    gameGrid->initializeMultiplayer(selectedPlayerType, selectedPlayer2Type, selectedMapType);
    mainWidget->setCurrentWidget(gameGrid);
}

void GameManager::handleGameOver(bool playerWon) {
    if (gameOverScreen) {
        mainWidget->removeWidget(gameOverScreen);
        delete gameOverScreen;
    }
    
    QString winnerText;
    if (isMultiplayerMode) {
        winnerText = playerWon ? "Player 1 Wins!" : "Player 2 Wins!";
    } else {
        winnerText = playerWon ? "Player Wins!" : "AI Wins!";
    }
    
    gameOverScreen = new GameOverScreen(playerWon, winnerText);
    mainWidget->addWidget(gameOverScreen);
    
    connect(gameOverScreen, &GameOverScreen::playAgainSelected, this, &GameManager::handlePlayAgain);
    connect(gameOverScreen, &GameOverScreen::newRobotsSelected, this, &GameManager::handleNewRobots);
    connect(gameOverScreen, &GameOverScreen::mainMenuSelected, this, &GameManager::handleMainMenuFromGameOver);
    
    mainWidget->setCurrentWidget(gameOverScreen);
}

void GameManager::handlePlayAgain() {
    if (isMultiplayerMode) {
        handleMultiplayerMapSelected(selectedMapType);
    } else {
        handleMapSelected(selectedMapType);
    }
}

void GameManager::handleNewRobots() {
    if (isMultiplayerMode) {
        mainWidget->setCurrentWidget(multiplayerRobotSelector);
    } else {
        mainWidget->setCurrentWidget(robotSelector);
    }
}

void GameManager::handleMainMenuFromGameOver() {
    showMainMenu();
}

void GameManager::handleBackButtonClicked() {
    showMainMenu();
}

void GameManager::handleMultiplayerRobotSelectorBackClicked() {
    showMainMenu();
}

void GameManager::handleTutorialBackButtonClicked() {
    showMainMenu();
}

void GameManager::handleDifficultySelectorBackClicked() {
    mainWidget->setCurrentWidget(robotSelector);
}

void GameManager::handleMapSelectorBackClicked() {
    if (isMultiplayerMode) {
        mainWidget->setCurrentWidget(multiplayerRobotSelector);
    } else {
        mainWidget->setCurrentWidget(difficultySelector);
    }
}

void GameManager::handleMultiplayerMapSelectorBackClicked() {
    mainWidget->setCurrentWidget(multiplayerRobotSelector);
}
