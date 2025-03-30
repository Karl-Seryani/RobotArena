#ifndef SNIPERAI_H
#define SNIPERAI_H

#include <QObject>
#include "aiinterface.h"
#include <QPoint>

// Forward declarations
class Game;
class Robot;
enum class Command;
enum class Direction;
enum class CellType;
enum class RobotType;
enum class GameDifficulty;

/** @brief Behaviour of the AI if it has chosen to use the sniper robot
 * 
 * The sniper AI implements a "Kite" strategy where it hangs back and try to chip off damage from it's oppoents.
 * This is designed to leverage the sniper's advantage of having longer range where the sniper can attack it's opponents from distances
 * the opponent cannot reach.
 * 
 * @author Group 17
 */
class SniperAI : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructor for ScoutAI
     * @param parent - The parent QObject
     */
    explicit SniperAI(QObject* parent = nullptr);

    /**
     * @brief Main entry point to calculate the Sniper's next move
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
    QPoint lastPlayerPosition;
    int    lastPlayerHealth;

    QPoint lastAiPosition;
    int    samePositionCounter;

    int consecutiveTurnCount;
    int moveCounter;
    int turnCounter;
    bool isCirclingClockwise;

private:
    Command calculateSniperNormal(Game* game, Robot* ai, Robot* player);

    Command vsScout(Game* game, Robot* ai, Robot* scout);
    Command vsTank(Game* game, Robot* ai, Robot* tank);
    Command vsSniper(Game* game, Robot* ai, Robot* enemySniper);

    Command tryCollectPickup(Game* game, Robot* ai, bool preferHealthIfLow = true);

    bool   hasLineOfSight(Game* game, const QPoint& from, const QPoint& to);

    bool   isInDirection(int dx, int dy, Direction dir);

    int    manhattanDistance(const QPoint& p1, const QPoint& p2);

    Direction getDirectionTowards(int dx, int dy);
    Direction getDirectionAway(int dx, int dy);
    Command   getTurnCommand(Direction currentDir, Direction targetDir);
    QPoint    getPositionInDirection(const QPoint& pos, Direction dir, int steps = 1);

    Command tryMoveOrBreakWall(Game* game, Robot* ai);

    Command huntPlayerPosition(Game* game, Robot* ai, int dx, int dy);


    Command directLineAttack(Game* game, Robot* ai, Robot* player);

    QPoint findNearestHealthPickup(Game* game, const QPoint& pos, int searchRadius);
    QPoint findNearestPowerUp(Game* game, const QPoint& pos, int searchRadius);
};
 
 #endif // SNIPERAI_H