#ifndef GAMEGRID_H
#define GAMEGRID_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QRandomGenerator>
#include <QTimer>
#include <QFrame>
#include <memory>
#include "game.h"
#include "difficultyselector.h"
#include "mapselector.h"

/// This class is primarily used to create the grid for the game arena. Whenever a user starts the game,
/// this class will be used to set-up the arena and handle what happens inside the arena
/// @see game.h
///@author Group 17
class GameGrid : public QWidget {
    Q_OBJECT
public:

    /// @brief Constructor for the game grid
    /// @param parent - The parent of this object, use to represent ownership
    explicit GameGrid(QWidget *parent = nullptr);
    /// @brief Function that initialises the arena as a fight with an AI
    /// @param playerType - Robot type of player
    /// @param aiType - Robot type of AI
    /// @param difficulty - Difficulty setting
    /// @param mapType - Type of map that will be used
    void initializeWithRobotType(RobotType playerType, RobotType aiType = RobotType::Scout, 
                                GameDifficulty difficulty = GameDifficulty::Medium,
                                MapType mapType = MapType::Random);
    /// @brief Function that initialises the arena as a fight with an AI
    /// @param playerType - Robot type of player 1
    /// @param player2Type - Robot type of player 2
    /// @param difficulty - Difficulty setting
    /// @param mapType - Type of map that will be used, typically randomized unless it is particularly specified
    void initializeMultiplayer(RobotType player1Type, RobotType player2Type,
                              MapType mapType = MapType::Random);
    
    /// Set a custom info panel message (for tutorial)
    /// @param message - the message the panel will have
    void setInfoPanelMessage(const QString& message);
    
    ///@brief Public access to info message label for tutorial
    QLabel* infoMessageLabel;
    
    /// Get the game object (for tutorial)
    ///@return the game object this tutorial will take place in
    Game* getGame() { return game.get(); }
    
    ///@brief Make updateGrid accessible to tutorial
    void updateGrid();

signals:
    void gameOver(bool playerWon);
    void keyPressed(int key); // Signal for tutorial to track key presses

protected:
    ///Function that reacts to a event where a key is pressed on the keyboard
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void handleTurnComplete();
    void handleGameStateChanged(GameState state);
    void updateStatusLabel();

public slots:

void spawnProjectile(const QPoint &start, const QPoint &end, Direction direction, bool actualHit, PowerUpType powerUpUsed);

private:
    void initializeGrid();
    void initializeControls();
    void setupWideScreenLayout();
    void drawRobot(Robot* robot, bool isPlayer);
    void drawCell(int x, int y, CellType cellType);

    std::unique_ptr<Game> game;
    QGraphicsScene* scene;
    QGraphicsView* view;
    
    // Main layout components
    QHBoxLayout* mainLayout;
    QWidget* gamePanel;
    QWidget* infoPanel;
    
    // Info panel components
    QLabel* titleLabel;
    QLabel* statusLabel;
    QLabel* controlsLabel;
    QLabel* mapInfoLabel;
    
    static const int CELL_SIZE = 60; // Size of each grid cell in pixels (reduced from 64)
    static const int GRID_SIZE = 12; // Size of the grid (increased from 8)
    static const int INFO_PANEL_WIDTH = 400; // Width of the info panel

    QGraphicsItemGroup* feedbackGroup;
};

#endif // GAMEGRID_H
