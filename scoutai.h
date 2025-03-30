#ifndef SCOUTAI_H
#define SCOUTAI_H

#include <QObject>
#include <QPoint>

// Forward declarations
class Game;
class Robot;
enum class Command;
enum class Direction;
enum class CellType;
enum class RobotType;
enum class GameDifficulty;

/**
 * @brief ScoutAI handles the strategy and movement decisions for Scout robots
 * 
 * The ScoutAI implements a "hit-and-dash" strategy, designed to leverage the Scout's
 * speed advantage by moving quickly, attacking opponents when advantageous, and then
 * retreating to avoid counterattacks.
 * 
 * @author Group 17
 */
class ScoutAI : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructor for ScoutAI
     * @param parent The parent QObject
     */
    explicit ScoutAI(QObject* parent = nullptr);

    /**
     * @brief Main entry point to calculate the Scout's next move
     * 
     * This method delegates to the appropriate strategy method based on the opponent type.
     * 
     * @param game Pointer to the current game state
     * @param ai Pointer to the Scout robot
     * @param player Pointer to the opponent robot
     * @return Command to execute
     */
    Command calculateMove(Game* game, Robot* ai, Robot* player);

private:
    QPoint lastPlayerPosition;  ///< Last known position of the opponent
    int    lastPlayerHealth;    ///< Last known health of the opponent

    QPoint lastAiPosition;      ///< Last position of the AI
    int    samePositionCounter; ///< Counter for how many turns the AI has been in the same position

    int consecutiveTurnCount;   ///< Counter for consecutive turns
    int moveCounter;            ///< Counter for movement tracking
    int turnCounter;            ///< Counter to detect excessive turning without movement
    bool isCirclingClockwise;   ///< Flag for circling direction

private:
    /**
     * @brief Default strategy when no specialized strategy is available
     * @param game Pointer to the current game state
     * @param ai Pointer to the Scout robot
     * @param player Pointer to the opponent robot
     * @return Command to execute
     */
    Command calculateScoutNormal(Game* game, Robot* ai, Robot* player);

    /**
     * @brief Specialized strategy for facing a Sniper
     * 
     * Implements a hit-and-dash approach, attacking once when adjacent and then moving away.
     * 
     * @param game Pointer to the current game state
     * @param ai Pointer to the Scout robot
     * @param sniper Pointer to the Sniper robot
     * @return Command to execute
     */
    Command vsSniper(Game* game, Robot* ai, Robot* sniper);
    
    /**
     * @brief Specialized strategy for facing a Tank
     * 
     * Implements a hit-and-dash approach, attacking once when adjacent and then moving away.
     * 
     * @param game Pointer to the current game state
     * @param ai Pointer to the Scout robot
     * @param tank Pointer to the Tank robot
     * @return Command to execute
     */
    Command vsTank(Game* game, Robot* ai, Robot* tank);
    
    /**
     * @brief Specialized strategy for facing another Scout
     * 
     * Implements a hit-and-dash approach, attacking once when adjacent and then moving away.
     * 
     * @param game Pointer to the current game state
     * @param ai Pointer to the Scout robot
     * @param otherScout Pointer to the opponent Scout robot
     * @return Command to execute
     */
    Command vsScout(Game* game, Robot* ai, Robot* otherScout);

    /**
     * @brief Attempt to collect a pickup (health or powerup)
     * @param game Pointer to the current game state
     * @param ai Pointer to the Scout robot
     * @param preferHealthIfLow Whether to prioritize health pickups when low on health
     * @return Command to execute or Command::None if no pickup is available
     */
    Command tryCollectPickup(Game* game, Robot* ai, bool preferHealthIfLow = true);

    /**
     * @brief Check if there is line of sight between two positions
     * @param game Pointer to the current game state
     * @param from The starting position
     * @param to The target position
     * @return true if there is line of sight, false otherwise
     */
    bool hasLineOfSight(Game* game, const QPoint& from, const QPoint& to);

    /**
     * @brief Check if a point is in a specific direction
     * @param dx X-coordinate difference
     * @param dy Y-coordinate difference
     * @param dir Direction to check
     * @return true if the point is in the specified direction, false otherwise
     */
    bool isInDirection(int dx, int dy, Direction dir);

    /**
     * @brief Calculate the Manhattan distance between two points
     * @param p1 First point
     * @param p2 Second point
     * @return Manhattan distance (|x1-x2| + |y1-y2|)
     */
    int manhattanDistance(const QPoint& p1, const QPoint& p2);

    /**
     * @brief Get the direction towards a target
     * @param dx X-coordinate difference
     * @param dy Y-coordinate difference
     * @return Direction towards the target
     */
    Direction getDirectionTowards(int dx, int dy);
    
    /**
     * @brief Get the direction away from a target
     * @param dx X-coordinate difference
     * @param dy Y-coordinate difference
     * @return Direction away from the target
     */
    Direction getDirectionAway(int dx, int dy);
    
    /**
     * @brief Get the command to turn from current direction to target direction
     * @param currentDir Current direction
     * @param targetDir Target direction
     * @return Command to execute (TurnLeft or TurnRight)
     */
    Command   getTurnCommand(Direction currentDir, Direction targetDir);
    
    /**
     * @brief Get the position in a specific direction from a starting position
     * @param pos Starting position
     * @param dir Direction to move
     * @param steps Number of steps to take
     * @return New position
     */
    QPoint    getPositionInDirection(const QPoint& pos, Direction dir, int steps = 1);

    /**
     * @brief Try to move forward or break a wall if movement is blocked
     * @param game Pointer to the current game state
     * @param ai Pointer to the Scout robot
     * @return Command to execute
     */
    Command tryMoveOrBreakWall(Game* game, Robot* ai);

    /**
     * @brief Hunt the player based on their last known position
     * @param game Pointer to the current game state
     * @param ai Pointer to the Scout robot
     * @param dx X-coordinate difference to player
     * @param dy Y-coordinate difference to player
     * @return Command to execute
     */
    Command huntPlayerPosition(Game* game, Robot* ai, int dx, int dy);

    /**
     * @brief Attempt a direct line attack if possible
     * @param game Pointer to the current game state
     * @param ai Pointer to the Scout robot
     * @param player Pointer to the opponent robot
     * @return Command to execute
     */
    Command directLineAttack(Game* game, Robot* ai, Robot* player);

    /**
     * @brief Find the nearest health pickup within a search radius
     * @param game Pointer to the current game state
     * @param pos Starting position for the search
     * @param searchRadius Radius to search within
     * @return Position of the nearest health pickup or (-1,-1) if none found
     */
    QPoint findNearestHealthPickup(Game* game, const QPoint& pos, int searchRadius);
    
    /**
     * @brief Find the nearest powerup within a search radius
     * @param game Pointer to the current game state
     * @param pos Starting position for the search
     * @param searchRadius Radius to search within
     * @return Position of the nearest powerup or (-1,-1) if none found
     */
    QPoint findNearestPowerUp(Game* game, const QPoint& pos, int searchRadius);
    
    /**
     * @brief Check if moving to a position would place the scout adjacent to the opponent
     * 
     * This is used to implement the "safe path" strategy, where the Scout avoids ending
     * its movement adjacent to an opponent if it doesn't have enough moves left to attack
     * and then move away.
     * 
     * @param game Pointer to the current game state
     * @param ai Pointer to the Scout robot
     * @param opponent Pointer to the opponent robot
     * @param newPos The potential new position
     * @return true if the new position would be adjacent to the opponent, false otherwise
     */
    bool wouldEndAdjacentToOpponent(Game* game, Robot* ai, Robot* opponent, const QPoint& newPos);
    
    /**
     * @brief Find a safe path that moves towards the opponent but doesn't end adjacent
     * 
     * This method implements the Scout's core pathfinding strategy, attempting to maintain
     * an ideal distance of 2 from opponents. It tries to avoid ending a turn adjacent to
     * an opponent unless the Scout has enough moves left to attack and then move away.
     * 
     * @param game Pointer to the current game state
     * @param ai Pointer to the Scout robot
     * @param opponent Pointer to the opponent robot
     * @return Command to execute or Command::None if no safe path found
     */
    Command findSafePath(Game* game, Robot* ai, Robot* opponent);
};

#endif // SCOUTAI_H
