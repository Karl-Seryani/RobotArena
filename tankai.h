#ifndef TANKAI_H
#define TANKAI_H

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

/** @brief Behaviours of the AI if it is using a tank
 *  
 *  The tank AI implements a "Everything-must-die" strategy where it doesn't care if it gets hit if it can trade some damage.
 *  This strategy is designed to abuse tank's larger health pool and decent damage to slowly trade off the enemy's health by taking
 *  more risky damage options.
 * 
 *  @author Group 17
 */
class TankAI : public QObject
{
    Q_OBJECT
public:
    /// @brief Constructor of the tank AI
    /// @param parent - QObject parent of the AI
    explicit TankAI(QObject* parent = nullptr);

    /// @brief Behaviour calcuation of the tank AI
    /// @param game Pointer to the current game state
    /// @param ai Pointer to the Scout robot
    /// @param player Pointer to the opponent robot
    /// @return Command to execute
    Command calculateMove(Game* game, Robot* ai, Robot* player);

private:
    QPoint lastPlayerPosition;
    int    lastPlayerHealth;

    QPoint lastAiPosition;
    int    samePositionCounter;

    int consecutiveTurnCount;
    int moveCounter;
    int turnCounter;
    bool isCirclingClockwise;

private:
    Command calculateTankNormal(Game* game, Robot* ai, Robot* player);

    Command vsScout(Game* game, Robot* ai, Robot* scout);
    Command vsSniper(Game* game, Robot* ai, Robot* sniper);
    Command vsTank(Game* game, Robot* ai, Robot* otherTank);

    Command tryCollectPickup(Game* game, Robot* ai, bool preferHealthIfLow = true);

    bool hasLineOfSight(Game* game, const QPoint& from, const QPoint& to);

    bool isInDirection(int dx, int dy, Direction dir);

    int manhattanDistance(const QPoint& p1, const QPoint& p2);

    Direction getDirectionTowards(int dx, int dy);

    Direction getDirectionAway(int dx, int dy);

    Command getTurnCommand(Direction currentDir, Direction targetDir);

    QPoint getPositionInDirection(const QPoint& pos, Direction dir, int steps = 1);

    Command tryMoveOrBreakWall(Game* game, Robot* ai);

    Command huntPlayerPosition(Game* game, Robot* ai, int dx, int dy);

    Command directLineAttack(Game* game, Robot* ai, Robot* player);

    QPoint findNearestHealthPickup(Game* game, const QPoint& pos, int searchRadius);
    QPoint findNearestPowerUp(Game* game, const QPoint& pos, int searchRadius);
};

#endif // TANKAI_H
