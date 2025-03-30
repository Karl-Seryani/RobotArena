#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <memory>
#include <vector>
#include "robot.h"
#include "difficultyselector.h"
#include "mapselector.h"
#include "robotai.h"

enum class GameState { PlayerTurn, Player2Turn, AiTurn, GameOver };
enum class Command { MoveForward, TurnLeft, TurnRight, Attack, None };
enum class CellType { Empty, Wall, HealthPickup, LaserPowerUp, MissilePowerUp, BombPowerUp };
enum class PowerUpType { Normal, Laser, Missile, Bomb };

class RobotAI; ///< Forward declaration

/**
 * @brief This class manages everything that happens in-game. 
 * 
 * It's roles includes making sure nothing goes out of boundary, keeping track of walls and player location, and many many more.
 * 
 * @author Group 17
 */
class Game : public QObject {
    Q_OBJECT
public:
    /// Walls take 3 hits from scout
    static const int INITIAL_WALL_HEALTH = 3; 
    /// Amount of health gained from a pickup   
    static const int HEALTH_PICKUP_AMOUNT = 20;
    /// Number of health pickups to place on the map
    static const int NUM_HEALTH_PICKUPS = 5; 
    /// The number of laser powerups in the game
    static const int NUM_LASER_POWERUPS = 1;
    /// The number of missile powerups in the game
    static const int NUM_MISSILE_POWERUPS = 1;
    ///The number of bomb powerups in the game
    static const int NUM_BOMB_POWERUPS = 1;
    
    /// Function used to initalise the game
    /// @param gridSize - the size of the grids in the game
    /// @param parent - pointer
    explicit Game(int gridSize = 8, QObject *parent = nullptr);
    /// @brief Function used to delete the game object
    ~Game();

    ///@brief Simple function for the game to execute a command
    ///@param cmd - the command to be executed
    void executeCommand(Command cmd); 
    ///@brief Simple function for the game AI to execute a turn
    void executeAiTurn(); 
    ///@brief Simple function to check for position validity
    ///@param pos - current position
    ///@return TRUE if successfully executed, FALSE otherwise
    bool isValidPosition(const QPoint& pos) const;
    ///@brief Simple function to check if the move is valid
    ///@param pos - the position after the move
    ///@return TRUE if successfully executed, FALSE otherwise
    bool isValidMove(const QPoint& pos) const;
    ///@brief Simple function for AI to see if it has a line of sight
    ///@param from - the position **from which** the AI is at
    ///@param to - the position **to which** the AI should target
    ///@return TRUE if successfully executed, FALSE otherwise
    bool hasLineOfSight(const QPoint& from, const QPoint& to) const;
    ///@brief Simple function that indicates an attack on the wall
    ///@param pos - the position of the attack
    ///@param damage - the damage said attack will be doing
    ///@return TRUE if successfully executed, FALSE otherwise
    bool attackWall(const QPoint& pos, int damage);
    ///@brief Simple function in indicate an attack
    ///@param attacker - the attacker this attack is from
    ///@param target  - the target of this attack
    ///@return TRUE if successfully executed, FALSE otherwise
    bool attack(Robot* attacker, Robot* target);
    ///@brief Simple function for health pickup
    ///@param pos  - position of the health pickup
    ///@param robot  - the robot we are healing
    void collectHealthPickup(const QPoint& pos, Robot* robot);
    ///@brief Simple function to place health pickup, automatically determines where the health pickup is placed
    void placeHealthPickups();
    
    /// @brief For tutorial - Spawn a specific number of health pickups
    /// @param count - the number of health pickups we want to spawn
    void spawnHealthPickup(int count);
    
    /// @brief For tutorial - Place a specific powerup at a position
    /// @param pos  - the spawm position of the power-up 
    ///@param powerUpType  - specifier for the power-up type we want to place in the map
    ///@return TRUE if successfully executed, FALSE otherwise
    bool placePowerUpAtPosition(const QPoint& pos, CellType powerUpType);
    

    ///@brief Getter method that returns player robot 1
    /// @return The robot of player 1
    Robot* getPlayerRobot() { return playerRobot.get(); }
    /// @brief Getter method that returns player robot 2
    /// @return The robot of player 2
    Robot* getPlayer2Robot() { return player2Robot.get(); }
    /// @brief getter method that returns the AI robot,  should the player choose to play against an AI
    /// @return The robot of the AI
    Robot* getAiRobot() { return aiRobot.get(); }
    /// @brief Getter method that returns the state of the game
    /// @return State of the game
    GameState getState() const { return state; }
    /// @brief Getter method for the grid size of the game
    /// @return Int variable that is the grid size of the game
    int getGridSize() const { return gridSize; }
    ///@brief returns the cell type of a given location, for example, it can be a cell for a wall
    ///@param pos - the position of the cell
    CellType getCellType(const QPoint& pos) const;
    ///@brief returns the health of a wall in a given location
    ///@param pos - the position of the wall we want to get the health of
    int getWallHealth(const QPoint& pos) const;
    /// @brief Set the robot type of player 1
    void setPlayerRobotType(RobotType type);
    /// @brief Set the robot type of player 2
    void setPlayer2RobotType(RobotType type);
    /// @brief Set the AI robot type, should the player chooses to play against an AI
    void setAiRobotType(RobotType type);
    /// @brief Sets the difficulty of the game, the default value is medium
    /// @param difficulty - the desired difficulty setting of the game
    void setDifficulty(GameDifficulty difficulty);
    /// @brief Getter function that returns the difficulty level of the game
    GameDifficulty getDifficulty() const { return difficulty; }
    /// @brief Setter function to change the type of map the game takes place on
    void setMapType(MapType mapType);
    /// @brief Setter function to enable multplayer mode
    void setMultiplayerMode(bool enabled);
    /// @brief Checks if we are currently in multplayer mode
    bool isMultiplayerMode() const { return multiplayerMode; }


    /// @brief **Initalises** the arena of the game, includes 1 player and one AI.
    /// @param playerType - The type of robot the player is using
    /// @param aiType - The type of robot the AI is using
    /// @param difficulty - Difficulty setting of the game
    /// @param mapType - the type of map that will be used
    void initializeArena(const RobotType& playerType, const RobotType& aiType, 
                         GameDifficulty difficulty = GameDifficulty::Medium,
                         MapType mapType = MapType::Random);
    /// @brief **Initalises** the arena of the game, includes two players
    /// @param player1Type - The type of robot player 1 will be using
    /// @param player2Type - The type of robot player 2 will be using
    /// @param mapType - The type of map that the arena will take place in
    void initializeMultiplayerArena(const RobotType& player1Type, const RobotType& player2Type,
                                   MapType mapType = MapType::Random);

    /// @brief Gives the robot a powerup
    /// @param robot - The target robot of this power-up
    /// @param powerUp - The type of power-up the robot will receive
    void setRobotPowerUp(Robot* robot, RobotPowerUp powerUp);

    /// @brief Place a special pick-up in the arena
    void placeSpecialPickups();
    /// @brief Places a single power-up inside the arena
    /// @param powerUpType - The type of power-up we want to place inside the arena
    /// @return TRUE is successfully executed, FALSE otherwise
    bool placeSinglePowerUp(CellType powerUpType);
    /// @brief Function that indicates a robot has picked up a power-up in a given location
    /// @param pos - The position of the power-up
    /// @param robot - The robot who picked up the power-up
    /// @param cellType - The cell type of the pick-up location
    void collectPowerUp(const QPoint& pos, Robot* robot, CellType cellType);
    /// @brief Getter method that returns the power-up the robot has picked up
    /// @param robot - The robot who picked up the power-up
    /// @return The power-up that the robot has picked-up
    RobotPowerUp getRobotPowerUp(const Robot* robot) const;

signals:
    void gameStateChanged(GameState newState);
    void turnComplete();
    void arenaInitialized();
    void wallDestroyed(const QPoint& pos);
    void healthPickupCollected(const QPoint& pos);
    void projectileFired(const QPoint &start, const QPoint &end, Direction direction, bool hit, PowerUpType powerUpUsed);

private:
    void checkGameOver();
    void switchTurn();
    void generateObstacles();
    void generateOpenMap();
    void generateMazeMap();
    void generateFortressMap();
    bool canMoveBetween(const QPoint& from, const QPoint& to) const;
    void applyDifficultySettings();

    std::unique_ptr<Robot> playerRobot;
    std::unique_ptr<Robot> player2Robot;
    std::unique_ptr<Robot> aiRobot;
    std::unique_ptr<RobotAI> robotAI;
    GameState state;
    int gridSize;
    std::vector<std::vector<CellType>> arena;
    std::vector<std::vector<int>> wallHealth;
    GameDifficulty difficulty;
    MapType mapType;
    bool multiplayerMode;
    
    // AI difficulty modifiers
    float aiHealthModifier;
    float aiDamageModifier;
    float aiRandomMoveChance;
};

#endif // GAME_H
