#include "sniperai.h"
#include "game.h"
#include "robot.h"
#include "logger.h"

#include <QRandomGenerator>
#include <algorithm>

// Constructor
SniperAI::SniperAI(QObject* parent)
    : QObject(parent),
      lastPlayerPosition(-1, -1),
      lastPlayerHealth(0),
      lastAiPosition(-1, -1),
      samePositionCounter(0),
      consecutiveTurnCount(0),
      moveCounter(0),
      turnCounter(0),
      isCirclingClockwise(true)
{
    Logger::log("SniperAI initialized.");
}

/**
 * Main entry point: decide next move for the SniperAI
 */
Command SniperAI::calculateMove(Game* game, Robot* ai, Robot* player)
{
    QPoint aiPos = ai->getPosition();
    Logger::log(QString("=================================================="));
    Logger::log(QString("SniperAI::calculateMove: Sniper at (%1, %2)").arg(aiPos.x()).arg(aiPos.y()));

    // Check if Sniper is stuck (has not moved from lastAiPosition)
    if (aiPos == lastAiPosition) {
        samePositionCounter++;
        Logger::log(QString("Sniper did not move. samePositionCounter increased to %1").arg(samePositionCounter));
    } else {
        samePositionCounter = 0;
        lastAiPosition = aiPos;
        Logger::log("Sniper moved. samePositionCounter reset.");
    }

    // If stuck for 5 consecutive attempts, attempt to break out:
    if (samePositionCounter > 5) {
        Logger::log("Sniper might be stuck. Attempting break-out.");
        samePositionCounter = 0;

        // Check if  wall ahead
        QPoint frontPos = getPositionInDirection(aiPos, ai->getDirection());
        if (game->isValidPosition(frontPos) && (game->getCellType(frontPos) == CellType::Wall)) {
            Logger::log("Wall in front while stuck. Command: Attack.");
            return Command::Attack;
        }

        // Check if player ahead
        QPoint aiPos = ai->getPosition();
        QPoint plrPos = player->getPosition();
        int dx = plrPos.x() - aiPos.x();
        int dy = plrPos.y() - aiPos.y();
        int distance = std::abs(dx) + std::abs(dy);
        bool canSee = hasLineOfSight(game, aiPos, plrPos);
        bool playerInDir = isInDirection(dx, dy, ai->getDirection());
        if (canSee && playerInDir && distance <= 3) {
            Logger::log("Player in front while stuck. Command: Attack.");
            return Command::Attack;
        }

        // If nothing in front, try to move forward if valid
        if (game->isValidMove(frontPos)) {
            Logger::log("Path clear while stuck. Command: MoveForward.");
            return Command::MoveForward;
        }

        // Otherwise, attack randomly (fallback)
        Logger::log("No valid move while stuck. Command: Attack.");
        return Command::Attack;
    }

    // Track player's position and health
    lastPlayerPosition = player->getPosition();
    lastPlayerHealth   = player->getHealth();
    Logger::log(QString("Player at (%1, %2) with health %3")
                .arg(lastPlayerPosition.x()).arg(lastPlayerPosition.y()).arg(lastPlayerHealth));

    // Check for a direct line attack
    Logger::log("Checking for direct line attack opportunity.");
    Command directAttackCmd = directLineAttack(game, ai, player);
    if (directAttackCmd != Command::None) {
        return directAttackCmd;
    }

    // Determine difficulty
    GameDifficulty diff = game->getDifficulty();

    // If Easy, do simpler logic
    if (diff == GameDifficulty::Easy) {
        Logger::log("Using normal Sniper logic for Easy difficulty.");
        return calculateSniperNormal(game, ai, player);
    }

    // For Medium or Hard, do specialized matchup logic
    RobotType enemyType = player->getType();
    switch (enemyType)
    {
    case RobotType::Scout:
        Logger::log("Using specialized logic: vsScout.");
        return vsScout(game, ai, player);
    case RobotType::Tank:
        Logger::log("Using specialized logic: vsTank.");
        return vsTank(game, ai, player);
    case RobotType::Sniper:
        Logger::log("Using specialized logic: vsSniper.");
        return vsSniper(game, ai, player);
    default:
        Logger::log("Unknown enemy type. Defaulting to vsScout.");
        return vsScout(game, ai, player);
    }
}

/**
 * Simpler Sniper logic: for easy difficulty
 */
Command SniperAI::calculateSniperNormal(Game* game, Robot* ai, Robot* player)
{
    QPoint aiPos = ai->getPosition();
    QPoint plrPos = player->getPosition();
    int dx = plrPos.x() - aiPos.x();
    int dy = plrPos.y() - aiPos.y();
    int distance = std::abs(dx) + std::abs(dy);

    Logger::log(QString("calculateSniperNormal: AI(%1,%2), Player(%3,%4), distance %5")
                .arg(aiPos.x()).arg(aiPos.y())
                .arg(plrPos.x()).arg(plrPos.y())
                .arg(distance));

    bool canSee = hasLineOfSight(game, aiPos, plrPos);
    bool playerInDir = isInDirection(dx, dy, ai->getDirection());
    Q_UNUSED(canSee);
    Q_UNUSED(playerInDir);

    // Move towards the player
    Logger::log("Player not within range. Closing in.");
    Direction towardDir = getDirectionTowards(dx, dy);
    if (ai->getDirection() != towardDir) {
        Logger::log("Not facing player. Command: Turn.");
        return getTurnCommand(ai->getDirection(), towardDir);
    }
    return tryMoveOrBreakWall(game, ai);

    // Fallback
    Logger::log("Fallback reached in calculateSniperNormal. Command: Attack.");
    return Command::Attack;
}

/**
 * Matchup specific logic: vs Scout
 *      - try to add distance and get powerups
 */
Command SniperAI::vsScout(Game* game, Robot* ai, Robot* scout)
{
    QPoint aiPos = ai->getPosition();
    QPoint plrPos = scout->getPosition();
    int dx = plrPos.x() - aiPos.x();
    int dy = plrPos.y() - aiPos.y();
    int distance = std::abs(dx) + std::abs(dy);

    Logger::log(QString("vsScout: AI(%1,%2), Scout(%3,%4), distance %5")
                .arg(aiPos.x()).arg(aiPos.y())
                .arg(plrPos.x()).arg(plrPos.y())
                .arg(distance));

    bool canSee = hasLineOfSight(game, aiPos, plrPos);
    bool playerInDir = isInDirection(dx, dy, ai->getDirection());
    GameDifficulty diff = game->getDifficulty();
    Q_UNUSED(canSee);
    Q_UNUSED(playerInDir);
    Q_UNUSED(diff);

    // If below half health, try to pick up health
    if (ai->getHealth() < ai->getMaxHealth() / 2) {
        Logger::log("vsScout: Sniper below 50% HP. Trying to pick up health.");
        Command c = tryCollectPickup(game, ai, true);
        if (c != Command::None) {
            return c;
        }
        // Continue with normal strategy if no pickup found
        Logger::log("vsScout: No health pickup found. Continuing with normal strategy.");
    }

    // Manage distance: scouts are fast, so try to keep at least distance=3
    if (distance <= 3) {
        Logger::log("vsScout: Too close. Retreating.");
        Direction awayDir = getDirectionAway(dx, dy);
        if (ai->getDirection() != awayDir) {
            return getTurnCommand(ai->getDirection(), awayDir);
        }
        return tryMoveOrBreakWall(game, ai);
    }

    // Within striking range: move towards player
    else if (distance >= 4 && distance <= 7) {
        Logger::log("vsScout: Within striking range. Move towards player.");
        Direction towardDir = getDirectionTowards(dx, dy);
        if (ai->getDirection() != towardDir) {
            return getTurnCommand(ai->getDirection(), towardDir);
        }
        return tryMoveOrBreakWall(game, ai);
    }

    // Far away: get powerup
    else {
        Logger::log("vsScout: Scout far away. Try to get pickup.");
        Command c = tryCollectPickup(game, ai, false);
        if (c != Command::None) {
            return c;
        }
        // Continue with movement if no pickup found
        Logger::log("vsScout: No powerup found. Moving towards player.");
        Direction towardDir = getDirectionTowards(dx, dy);
        if (ai->getDirection() != towardDir) {
            return getTurnCommand(ai->getDirection(), towardDir);
        }
        return tryMoveOrBreakWall(game, ai);
    }
    
    // Default fallback - should not reach here due to the complete else-if chain above
    return Command::None;
}

/**
 * Matchup specific logic: vs Tank
 *      - try to add distance and get powerups
 */
Command SniperAI::vsTank(Game* game, Robot* ai, Robot* tank)
{
    QPoint aiPos = ai->getPosition();
    QPoint plrPos = tank->getPosition();
    int dx = plrPos.x() - aiPos.x();
    int dy = plrPos.y() - aiPos.y();
    int distance = std::abs(dx) + std::abs(dy);

    Logger::log(QString("vsTank: AI(%1,%2), Tank(%3,%4), distance %5")
                .arg(aiPos.x()).arg(aiPos.y())
                .arg(plrPos.x()).arg(plrPos.y())
                .arg(distance));

    bool canSee = hasLineOfSight(game, aiPos, plrPos);
    bool playerInDir = isInDirection(dx, dy, ai->getDirection());
    GameDifficulty diff = game->getDifficulty();
    Q_UNUSED(canSee);
    Q_UNUSED(playerInDir);
    Q_UNUSED(diff);

    // If below half health, try to pick up health
    if (ai->getHealth() < ai->getMaxHealth() / 2) {
        Logger::log("vsTank: Sniper below 50% HP. Trying to pick up health.");
        Command c = tryCollectPickup(game, ai, true);
        if (c != Command::None) {
            return c;
        }
        // Continue with normal strategy if no pickup found
        Logger::log("vsTank: No health pickup found. Continuing with normal strategy.");
    }

    // Manage distance: try to keep at least distance=3
    if (distance <= 3) {
        Logger::log("vsTank: Too close. Retreating.");
        Direction awayDir = getDirectionAway(dx, dy);
        if (ai->getDirection() != awayDir) {
            return getTurnCommand(ai->getDirection(), awayDir);
        }
        return tryMoveOrBreakWall(game, ai);
    }

    // Within striking range: move towards player
    else if (distance >= 4 && distance <= 7) {
        Logger::log("vsTank: Within striking range. Move towards player.");
        Direction towardDir = getDirectionTowards(dx, dy);
        if (ai->getDirection() != towardDir) {
            return getTurnCommand(ai->getDirection(), towardDir);
        }
        return tryMoveOrBreakWall(game, ai);
    }

    // Far away: get powerup
    else {
        Logger::log("vsTank: Tank far away. Try to get pickup.");
        Command c = tryCollectPickup(game, ai, false);
        if (c != Command::None) {
            return c;
        }
        // Continue with movement if no pickup found
        Logger::log("vsTank: No powerup found. Moving towards player.");
        Direction towardDir = getDirectionTowards(dx, dy);
        if (ai->getDirection() != towardDir) {
            return getTurnCommand(ai->getDirection(), towardDir);
        }
        return tryMoveOrBreakWall(game, ai);
    }
    
    // Default fallback - should not reach here due to the complete else-if chain above
    return Command::None;
}

/*
 * Matchup specific logic: vs Sniper
 *   - even matchup, try to get powerups
 */
Command SniperAI::vsSniper(Game* game, Robot* ai, Robot* enemySniper)
{
    QPoint aiPos = ai->getPosition();
    QPoint plrPos = enemySniper->getPosition();
    int dx = plrPos.x() - aiPos.x();
    int dy = plrPos.y() - aiPos.y();
    int distance = std::abs(dx) + std::abs(dy);

    Logger::log(QString("vsSniper: AI(%1,%2), EnemySniper(%3,%4), distance %5")
                .arg(aiPos.x()).arg(aiPos.y())
                .arg(plrPos.x()).arg(plrPos.y())
                .arg(distance));

    bool canSee = hasLineOfSight(game, aiPos, plrPos);
    bool playerInDir = isInDirection(dx, dy, ai->getDirection());
    GameDifficulty diff = game->getDifficulty();
    Q_UNUSED(canSee);
    Q_UNUSED(playerInDir);
    Q_UNUSED(diff);

    // If below half health, try to pick up health
    if (ai->getHealth() < ai->getMaxHealth() / 2) {
        Logger::log("vsSniper: Sniper below 50% HP. Attempting health pickup.");
        Command c = tryCollectPickup(game, ai, true);
        if (c != Command::None) {
            return c;
        }
        // Continue with normal strategy if no pickup found
        Logger::log("vsSniper: No health pickup found. Continuing with normal strategy.");
    }

    // Within striking range: move towards player
    if (distance >= 4 && distance <= 7) {
        Logger::log("vsSniper: Within striking range. Move towards player.");
        Direction towardDir = getDirectionTowards(dx, dy);
        if (ai->getDirection() != towardDir) {
            return getTurnCommand(ai->getDirection(), towardDir);
        }
        return tryMoveOrBreakWall(game, ai);
    }

    // Far away: get powerup
    else {
        Logger::log("vsSniper: Sniper far away. Try to get pickup.");
        Command c = tryCollectPickup(game, ai, false);
        if (c != Command::None) {
            return c;
        }
        // Continue with movement if no pickup found
        Logger::log("vsSniper: No powerup found. Moving towards player.");
        Direction towardDir = getDirectionTowards(dx, dy);
        if (ai->getDirection() != towardDir) {
            return getTurnCommand(ai->getDirection(), towardDir);
        }
        return tryMoveOrBreakWall(game, ai);
    }
    
    // Default fallback - should not reach here due to the complete else-if chain above
    return Command::None;
}

/**
 *   Helper: Attempt to pick up nearest beneficial pickup
 */
Command SniperAI::tryCollectPickup(Game* game, Robot* ai, bool preferHealthIfLow)
{
    QPoint aiPos = ai->getPosition();
    Logger::log("tryCollectPickup: Searching for pickups.");

    // Find nearest health pickup within 5 tiles
    QPoint hpPos = findNearestHealthPickup(game, aiPos, 6);
    int distHP = 999999;
    if (hpPos.x() != -1)
        distHP = manhattanDistance(aiPos, hpPos);

    // Find nearest powerup within 5 tiles
    QPoint puPos = findNearestPowerUp(game, aiPos, 6);
    int distPU = 999999;
    if (puPos.x() != -1)
        distPU = manhattanDistance(aiPos, puPos);

    // If no pickups found, fallback
    if (hpPos.x() == -1 && puPos.x() == -1) {
        Logger::log("tryCollectPickup: No pickups found. Command: None.");
        return Command::None;
    }

    bool goHealth = false;
    if (preferHealthIfLow && hpPos.x() != -1)
        goHealth = true;
    else if (hpPos.x() != -1 && puPos.x() != -1)
        goHealth = (distHP <= distPU);
    else if (puPos.x() == -1)
        goHealth = true;

    QPoint targetPos = goHealth ? hpPos : puPos;
    int dx = targetPos.x() - aiPos.x();
    int dy = targetPos.y() - aiPos.y();
    Direction targetDir = getDirectionTowards(dx, dy);

    Logger::log(QString("tryCollectPickup: %1 pickup selected at (%2, %3)")
                .arg(goHealth ? "Health" : "Powerup")
                .arg(targetPos.x()).arg(targetPos.y()));

    if (ai->getDirection() != targetDir) {
        Logger::log("Not facing pickup direction. Command: Turn.");
        return getTurnCommand(ai->getDirection(), targetDir);
    }
    return tryMoveOrBreakWall(game, ai);

    Logger::log("tryCollectPickup: Fallback reached. Command: None.");
    return Command::None;
}

/**
 *   Movement Helpers
 */
Command SniperAI::tryMoveOrBreakWall(Game* game, Robot* ai)
{
    QPoint aiPos = ai->getPosition();
    Direction currentDir = ai->getDirection();

    QPoint forwardPos = getPositionInDirection(aiPos, currentDir);
    if (game->isValidMove(forwardPos)) {
        Logger::log("Forward move valid. Command: MoveForward.");
        return Command::MoveForward;
    }
    if (game->isValidPosition(forwardPos) &&
        game->getCellType(forwardPos) == CellType::Wall)
    {
        Logger::log("Wall directly ahead. Command: Attack.");
        return Command::Attack;
    }

    // Try turning left/right to find a valid path.
    for (int i = 0; i < 4; i++) {
        Direction rightDir = static_cast<Direction>((static_cast<int>(currentDir) + 1) % 4);
        QPoint rightPos = getPositionInDirection(aiPos, rightDir);

        Direction leftDir = static_cast<Direction>((static_cast<int>(currentDir) + 3) % 4);
        QPoint leftPos = getPositionInDirection(aiPos, leftDir);

        if (game->isValidMove(leftPos) && game->isValidMove(rightPos)) {
            Logger::log("Valid move found to the left and right. Command: Random Turn.");
            return (QRandomGenerator::global()->bounded(2) == 0)
               ? getTurnCommand(currentDir, leftDir)
               : getTurnCommand(currentDir, rightDir);
        }

        if (game->isValidMove(leftPos)) {
            Logger::log("Valid move found to the left. Command: TurnLeft.");
            return getTurnCommand(currentDir, leftDir);
        }
        if (game->isValidPosition(leftPos) &&
            game->getCellType(leftPos) == CellType::Wall)
        {
            Logger::log("Wall detected on the left. Command: TurnLeft.");
            return getTurnCommand(currentDir, leftDir);
        }

        if (game->isValidMove(rightPos)) {
            Logger::log("Valid move found to the right. Command: TurnRight.");
            return getTurnCommand(currentDir, rightDir);
        }
        if (game->isValidPosition(rightPos) &&
            game->getCellType(rightPos) == CellType::Wall)
        {
            Logger::log("Wall detected on the right. Command: TurnRight.");
            return getTurnCommand(currentDir, rightDir);
        }

        currentDir = static_cast<Direction>((static_cast<int>(currentDir) + 1) % 4);
    }

    Logger::log("tryMoveOrBreakWall: No valid moves found. Fallback: Attack.");
    return Command::Attack;
}

Command SniperAI::huntPlayerPosition(Game* game, Robot* ai, int dx, int dy)
{
    Direction desiredDir = getDirectionTowards(dx, dy);
    Direction currentDir = ai->getDirection();
    Logger::log("huntPlayerPosition: Hunting last known player position.");
    if (currentDir != desiredDir) {
        Logger::log("Not facing desired direction. Command: Turn.");
        return getTurnCommand(currentDir, desiredDir);
    }
    Command attemptMove = tryMoveOrBreakWall(game, ai);
    Logger::log("huntPlayerPosition: Forwarding movement command.");
    return attemptMove; 
}

/**
 *   Utility Methods
 */
bool SniperAI::hasLineOfSight(Game* game, const QPoint& from, const QPoint& to)
{
    return game->hasLineOfSight(from, to);
}

bool SniperAI::isInDirection(int dx, int dy, Direction dir)
{
    switch (dir) {
    case Direction::North:
        return (dy < 0) && (std::abs(dy) >= std::abs(dx));
    case Direction::East:
        return (dx > 0) && (std::abs(dx) >= std::abs(dy));
    case Direction::South:
        return (dy > 0) && (std::abs(dy) >= std::abs(dx));
    case Direction::West:
        return (dx < 0) && (std::abs(dx) >= std::abs(dy));
    }
    return false;
}

int SniperAI::manhattanDistance(const QPoint& p1, const QPoint& p2)
{
    return std::abs(p1.x() - p2.x()) + std::abs(p1.y() - p2.y());
}

Direction SniperAI::getDirectionTowards(int dx, int dy)
{
    if (std::abs(dx) > std::abs(dy)) {
        return (dx > 0) ? Direction::East : Direction::West;
    } else {
        return (dy > 0) ? Direction::South : Direction::North;
    }
}

Direction SniperAI::getDirectionAway(int dx, int dy)
{
    if (std::abs(dx) > std::abs(dy)) {
        return (dx > 0) ? Direction::West : Direction::East;
    } else {
        return (dy > 0) ? Direction::North : Direction::South;
    }
}

Command SniperAI::getTurnCommand(Direction currentDir, Direction targetDir)
{
    int c = static_cast<int>(currentDir);
    int t = static_cast<int>(targetDir);
    int diff = (t - c + 4) % 4;

    if (diff == 1) {
        return Command::TurnRight;
    } else if (diff == 2) {
        return Command::TurnRight;
    } else {
        return Command::TurnLeft;
    }
}

QPoint SniperAI::getPositionInDirection(const QPoint& pos, Direction dir, int steps)
{
    QPoint p = pos;
    switch (dir) {
    case Direction::North:
        p.setY(p.y() - steps);
        break;
    case Direction::East:
        p.setX(p.x() + steps);
        break;
    case Direction::South:
        p.setY(p.y() + steps);
        break;
    case Direction::West:
        p.setX(p.x() - steps);
        break;
    }
    return p;
}

/**
 *   Pickup-Finding Methods
 */
QPoint SniperAI::findNearestHealthPickup(Game* game, const QPoint& pos, int searchRadius)
{
    QPoint best(-1, -1);
    int minDist = 999999;
    int size = game->getGridSize();

    for (int y = std::max(0, pos.y() - searchRadius); y <= std::min(size - 1, pos.y() + searchRadius); ++y) {
        for (int x = std::max(0, pos.x() - searchRadius); x <= std::min(size - 1, pos.x() + searchRadius); ++x) {
            QPoint check(x, y);
            if (game->getCellType(check) == CellType::HealthPickup) {
                int d = manhattanDistance(pos, check);
                if (d < minDist) {
                    minDist = d;
                    best = check;
                }
            }
        }
    }
    return best;
}

QPoint SniperAI::findNearestPowerUp(Game* game, const QPoint& pos, int searchRadius)
{
    QPoint best(-1, -1);
    int minDist = 999999;
    int size = game->getGridSize();

    for (int y = std::max(0, pos.y() - searchRadius); y <= std::min(size - 1, pos.y() + searchRadius); ++y) {
        for (int x = std::max(0, pos.x() - searchRadius); x <= std::min(size - 1, pos.x() + searchRadius); ++x) {
            QPoint check(x, y);
            CellType ctype = game->getCellType(check);
            if (ctype == CellType::LaserPowerUp ||
                ctype == CellType::MissilePowerUp ||
                ctype == CellType::BombPowerUp)
            {
                int d = manhattanDistance(pos, check);
                if (d < minDist) {
                    minDist = d;
                    best = check;
                }
            }
        }
    }
    return best;
}

/**
 *   (7) Direct Line Attack Method
 */
Command SniperAI::directLineAttack(Game* game, Robot* ai, Robot* player)
{
    QPoint aiPos = ai->getPosition();
    QPoint playerPos = player->getPosition();
    int dx = playerPos.x() - aiPos.x();
    int dy = playerPos.y() - aiPos.y();
    int distance = std::abs(dx) + std::abs(dy);

    if (distance <= 3 && (aiPos.x() == playerPos.x() || aiPos.y() == playerPos.y())) {
        Direction desiredDir = getDirectionTowards(dx, dy);
        if (ai->getHealth() < static_cast<int>(ai->getMaxHealth() * 0.5)) {
            // Check if health pickup is available within 5 tiles
            QPoint hpPos = findNearestHealthPickup(game, aiPos, 5);
            if (hpPos.x() != -1) {
                Logger::log("directLineAttack: Very low health and health pickup available. Try to collect health.");
                return tryCollectPickup(game, ai, true);
            }
        }
        if (ai->getDirection() != desiredDir) {
            Logger::log("directLineAttack: Not facing player. Command: Turn.");
            return getTurnCommand(ai->getDirection(), desiredDir);
        }
        if (player->getHealth() < ai->getAttackDamage()) {
            Logger::log("directLineAttack: Sure kill. Command: Attack.");
            return Command::Attack;
        }

        Logger::log("directLineAttack: Conditions met. Command: Attack.");
        return Command::Attack;
    }
    
    return Command::None;
}
