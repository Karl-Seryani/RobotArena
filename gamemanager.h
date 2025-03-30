#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QObject>
#include <QStackedWidget>
#include <QPlainTextEdit>
#include "mainmenu.h"
#include "gamegrid.h"
#include "robotselector.h"
#include "multiplayerrobotselector.h"
#include "tutorial.h"
#include "difficultyselector.h"
#include "gameoverscreen.h"
#include "mapselector.h"

/**
 * @brief This class handles the all major actions that happens inside the game.
 * 
 * This includes actions like game-over, starting the game again, choosing another map or changing difficulty.
 * 
 * @author Group 17
 */
class GameManager : public QObject {
    Q_OBJECT
public:
    /// @brief Constructor for the Game Manager
    /// @param parent - the parent of the gamemanager
    explicit GameManager(QObject *parent = nullptr);
    /// @brief Delete function for this object
    ~GameManager();
    
    /// @brief Shows main menu
    void showMainMenu();

    /// @brief Get the main widget in the widget stack
    /// @return Main widget in the widget stack
    QStackedWidget* getMainWidget() { return mainWidget; }

private slots:
    void handleSinglePlayerSelected();
    void handleMultiplayerSelected();
    void handleTutorialSelected();
    void handleRobotsSelected(RobotType playerType, RobotType aiType, bool randomAI);
    void handleMultiplayerRobotsSelected(RobotType player1Type, RobotType player2Type);
    void handleDifficultySelected(GameDifficulty difficulty);
    void handleMapSelected(MapType mapType);
    void handleMultiplayerMapSelected(MapType mapType);
    void handleBackButtonClicked();
    void handleTutorialBackButtonClicked();
    void handleDifficultySelectorBackClicked();
    void handleMapSelectorBackClicked();
    void handleMultiplayerRobotSelectorBackClicked();
    void handleMultiplayerMapSelectorBackClicked();
    void handleGameOver(bool playerWon);
    void handlePlayAgain();
    void handleNewRobots();
    void handleMainMenuFromGameOver();

private:
    QStackedWidget* mainWidget;
    MainMenu* mainMenu;
    GameGrid* gameGrid;
    RobotSelector* robotSelector;
    MultiplayerRobotSelector* multiplayerRobotSelector;
    Tutorial* tutorial;
    DifficultySelector* difficultySelector;
    MapSelector* mapSelector;
    GameOverScreen* gameOverScreen;
    
    // Store selected game settings
    RobotType selectedPlayerType;
    RobotType selectedAIType;
    RobotType selectedPlayer2Type;
    bool isRandomAI;
    bool isMultiplayerMode;
    GameDifficulty selectedDifficulty;
    MapType selectedMapType;
    
    // Logging window
    QPlainTextEdit* logWindow;
};

#endif // GAMEMANAGER_H 
