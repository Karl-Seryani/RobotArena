#include "scoutai.h"
#include "game.h"
#include "robot.h"
#include "logger.h"

#include <QRandomGenerator>
#include <algorithm>

// Constructor
ScoutAI::ScoutAI(QObject* parent)
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
    Logger::log("ScoutAI initialized.");
}

/**
 * Main entry point: decide next move for the ScoutAI
 */
Command ScoutAI::calculateMove(Game* game, Robot* ai, Robot* player)
{
    QPoint aiPos = ai->getPosition();
    Logger::log(QString("=================================================="));
    Logger::log(QString("ScoutAI::calculateMove: Scout at (%1, %2), moves left: %3/%4")
                .arg(aiPos.x()).arg(aiPos.y())
                .arg(ai->getMovesLeft()).arg(ai->getMaxMoves()));

    // If this is the start of a new turn (full moves), reset turnCounter
    if (ai->getMovesLeft() == ai->getMaxMoves()) {
        turnCounter = 0;
        Logger::log("New turn with full moves. Reset turn counter.");
    }

    // Check if Scout is stuck (has not moved from lastAiPosition)
    if (aiPos == lastAiPosition) {
        samePositionCounter++;
        Logger::log(QString("Scout did not move. samePositionCounter increased to %1").arg(samePositionCounter));
    } else {
        samePositionCounter = 0;
        lastAiPosition = aiPos;
        Logger::log("Scout moved. samePositionCounter reset.");
    }

    // If stuck for 5 consecutive attempts, attempt to break out:
    if (samePositionCounter > 5) {
        Logger::log("Scout might be stuck. Attempting aggressive break-out.");
        samePositionCounter = 0;
        
        // Get opponent reference
        Robot* opponent = game->getPlayerRobot();
        if (ai == game->getPlayerRobot()) {
            opponent = game->isMultiplayerMode() ? game->getPlayer2Robot() : game->getAiRobot();
        }
        
        QPoint opponentPos = opponent->getPosition();
        int manhattanDistToOpponent = manhattanDistance(aiPos, opponentPos);
        Logger::log(QString("Current distance to opponent: %1").arg(manhattanDistToOpponent));
        
        // If the opponent is adjacent and we're facing them, just attack directly
        if (manhattanDistToOpponent == 1 && 
            isInDirection(opponentPos.x() - aiPos.x(), opponentPos.y() - aiPos.y(), ai->getDirection())) {
            Logger::log("Opponent is adjacent and we're facing them. Command: Attack.");
            return Command::Attack;
        }
        
        // Try all directions to find ANY valid move
        Logger::log("Trying ANY valid direction to move...");
        for (int i = 0; i < 4; i++) {
            Direction testDir = static_cast<Direction>(i);
            QPoint newPos = getPositionInDirection(aiPos, testDir);
            
            if (game->isValidMove(newPos)) {
                Logger::log(QString("Found valid move in direction %1. Trying that.").arg(i));
                if (ai->getDirection() != testDir) {
                    return getTurnCommand(ai->getDirection(), testDir);
                } else {
                    return Command::MoveForward;
                }
            }
        }
        
        // If we can't move in any direction, try attacking in the current direction
        QPoint frontPos = getPositionInDirection(aiPos, ai->getDirection());
        if (game->isValidPosition(frontPos) && game->getCellType(frontPos) == CellType::Wall) {
            Logger::log("Completely stuck with wall in front. Command: Attack to break wall.");
            return Command::Attack;
        }
        
        // As a last resort, just turn in a different random direction
        Logger::log("Completely stuck with no valid moves. Making random turn as absolute last resort.");
        Direction currentDir = ai->getDirection();
        Direction randomDir;
        do {
            randomDir = static_cast<Direction>(QRandomGenerator::global()->bounded(4));
        } while (randomDir == currentDir);
        
        return getTurnCommand(currentDir, randomDir);
    }

    // Track player position and health
    QPoint playerPos = player->getPosition();
    int playerHealth = player->getHealth();
    
    if (playerPos != lastPlayerPosition || playerHealth != lastPlayerHealth) {
        if (lastPlayerPosition != QPoint(-1, -1)) {
            Logger::log(QString("Player moved from (%1,%2) to (%3,%4) or health changed from %5 to %6")
                        .arg(lastPlayerPosition.x()).arg(lastPlayerPosition.y())
                        .arg(playerPos.x()).arg(playerPos.y())
                        .arg(lastPlayerHealth).arg(playerHealth));
        }
        lastPlayerPosition = playerPos;
        lastPlayerHealth = playerHealth;
    }

    // Choose strategy based on robot matchup
    switch (player->getType()) {
        case RobotType::Scout:
            return vsScout(game, ai, player);
        case RobotType::Tank:
            return vsTank(game, ai, player);
        case RobotType::Sniper:
            return vsSniper(game, ai, player);
        default:
            return calculateScoutNormal(game, ai, player);
    }
}

/**
 * Default strategy if matchup-specific not available
 */
Command ScoutAI::calculateScoutNormal(Game* game, Robot* ai, Robot* player)
{
    QPoint aiPos = ai->getPosition();
    QPoint playerPos = player->getPosition();
    int dx = playerPos.x() - aiPos.x();
    int dy = playerPos.y() - aiPos.y();
    int distance = std::abs(dx) + std::abs(dy);

    Logger::log(QString("calculateScoutNormal: Scout(%1,%2), Player(%3,%4), distance %5")
                .arg(aiPos.x()).arg(aiPos.y())
                .arg(playerPos.x()).arg(playerPos.y())
                .arg(distance));

    bool canSee = hasLineOfSight(game, aiPos, playerPos);
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
 *   - even matchup, try to get powerups
 */
Command ScoutAI::vsScout(Game* game, Robot* ai, Robot* otherScout)
{
    QPoint aiPos = ai->getPosition();
    QPoint plrPos = otherScout->getPosition();
    int dx = plrPos.x() - aiPos.x();
    int dy = plrPos.y() - aiPos.y();
    int distance = std::abs(dx) + std::abs(dy);
    int totalMovesAtStartOfTurn = ai->getMaxMoves();
    int movesUsedThisTurn = totalMovesAtStartOfTurn - ai->getMovesLeft();

    Logger::log(QString("vsScout: Scout(%1,%2), EnemyScout(%3,%4), distance %5, moves used: %6/%7")
                .arg(aiPos.x()).arg(aiPos.y())
                .arg(plrPos.x()).arg(plrPos.y())
                .arg(distance)
                .arg(movesUsedThisTurn)
                .arg(totalMovesAtStartOfTurn));

    bool canSee = hasLineOfSight(game, aiPos, plrPos);
    bool playerInDir = isInDirection(dx, dy, ai->getDirection());
    GameDifficulty diff = game->getDifficulty();
    Q_UNUSED(canSee);
    Q_UNUSED(diff);
    
    // Check if the other scout can be killed with one more attack
    bool canKillWithOneAttack = (otherScout->getHealth() <= ai->getAttackDamage());
    if (canKillWithOneAttack) {
        Logger::log(QString("vsScout: Enemy Scout has %1 health and our attack is %2. Can kill with one hit!")
                   .arg(otherScout->getHealth()).arg(ai->getAttackDamage()));
    }

    // If we start adjacent to opponent, attack and then move away (hit and dash)
    if (distance == 1) {
        // Check if we're stuck in a turn cycle
        if (samePositionCounter >= 3) {
            Logger::log("vsScout: Detected potential turn cycle. Breaking out of pattern.");
            
            // Try to move in ANY direction that's valid to break out
            for (int i = 0; i < 4; i++) {
                Direction testDir = static_cast<Direction>(i);
                QPoint newPos = getPositionInDirection(aiPos, testDir);
                
                if (game->isValidMove(newPos)) {
                    if (ai->getDirection() != testDir) {
                        Logger::log(QString("vsScout: Breaking cycle - turning to ANY valid direction %1").arg(i));
                        return getTurnCommand(ai->getDirection(), testDir);
                    } else {
                        Logger::log("vsScout: Breaking cycle - already facing valid direction, moving forward");
                        return Command::MoveForward;
                    }
                }
            }
            
            // If no valid moves and facing opponent, attack as a last resort
            if (playerInDir) {
                Logger::log("vsScout: Breaking cycle - no valid moves, attacking as last resort");
                return Command::Attack;
            } else {
                // Turn toward scout
                Direction towardDir = getDirectionTowards(dx, dy);
                return getTurnCommand(ai->getDirection(), towardDir);
            }
        }
        
        // First, make sure we're facing the scout
        if (!playerInDir) {
            Logger::log("vsScout: Adjacent to scout but not facing them. Turning to attack.");
            Direction towardDir = getDirectionTowards(dx, dy);
            return getTurnCommand(ai->getDirection(), towardDir);
        }
        
        // Check if we've already attacked this turn
        bool hasAttackedThisTurn = (movesUsedThisTurn > 0);
        
        // If we can kill the other scout with one attack, prioritize attacking regardless
        if (canKillWithOneAttack && playerInDir) {
            Logger::log("vsScout: Enemy Scout can be killed with one attack! Attacking for the kill!");
            return Command::Attack;
        }
        
        // Against other scouts, attack once then move away
        if (!hasAttackedThisTurn) {
            // If we have multiple moves, always attack
            if (ai->getMovesLeft() >= 2) {
                Logger::log("vsScout: Adjacent to scout and facing them with multiple moves. Attacking once!");
                return Command::Attack;
            }
            
            // If we only have one move left, attack only if we have a health advantage or can kill
            if (ai->getMovesLeft() == 1) {
                if (ai->getHealth() > otherScout->getHealth() || canKillWithOneAttack) {
                    Logger::log("vsScout: Last move and have health advantage or can kill. Attacking!");
                    return Command::Attack;
                } else {
                    Logger::log("vsScout: Last move and no health advantage. Moving away instead.");
                }
            }
        }
        
        // If we've already attacked but can still kill the opponent, do it
        if (hasAttackedThisTurn && canKillWithOneAttack && playerInDir) {
            Logger::log("vsScout: Already attacked but can kill enemy Scout with one more attack! Finishing it off!");
            return Command::Attack;
        }
        
        // After attacking once, always try to retreat
        Logger::log("vsScout: Already attacked or chose to retreat. Moving away.");
        Direction awayDir = getDirectionAway(dx, dy);
        
        // Check if we can actually move away before turning
        QPoint awayPos = getPositionInDirection(aiPos, awayDir);
        bool canMoveAway = game->isValidMove(awayPos);
        
        // If we can't move directly away, try alternative directions
        if (!canMoveAway) {
            Logger::log("vsScout: Can't move directly away. Trying alternative directions.");
            
            // Try all directions except toward the opposing scout
            Direction towardDir = getDirectionTowards(dx, dy);
            for (int i = 0; i < 4; i++) {
                Direction testDir = static_cast<Direction>(i);
                if (testDir == towardDir) continue; // Skip direction toward opponent
                
                QPoint testPos = getPositionInDirection(aiPos, testDir);
                if (game->isValidMove(testPos)) {
                    if (ai->getDirection() != testDir) {
                        Logger::log(QString("vsScout: Found alternative escape direction %1. Turning.").arg(i));
                        return getTurnCommand(ai->getDirection(), testDir);
                    } else {
                        Logger::log("vsScout: Already facing alternative escape direction. Moving.");
                        return Command::MoveForward;
                    }
                }
            }
            
            // If we can't move away and haven't attacked, attack as a last resort
            if (!hasAttackedThisTurn && playerInDir) {
                // If the attack will kill, make that clear in the log
                if (canKillWithOneAttack) {
                    Logger::log("vsScout: No escape possible but can kill! Attacking for the kill!");
                } else {
                    Logger::log("vsScout: No escape possible. Attacking as last resort.");
                }
                return Command::Attack;
            } else if (!playerInDir) {
                // Turn to face the scout if we can't move away
                Direction towardDir = getDirectionTowards(dx, dy);
                return getTurnCommand(ai->getDirection(), towardDir);
            }
        }
        
        // We can move directly away, so turn if needed
        if (ai->getDirection() != awayDir) {
            Logger::log(QString("vsScout: Turning to escape direction %1").arg(static_cast<int>(awayDir)));
            return getTurnCommand(ai->getDirection(), awayDir);
        }
        
        // Already facing away, so move
        Logger::log("vsScout: Moving away from scout in escape direction");
        return Command::MoveForward;
    }

    // If below half health and have no advantage, try to pick up health
    // Skip healing if we can kill the opponent
    if (ai->getHealth() < ai->getMaxHealth() / 2 && ai->getHealth() <= otherScout->getHealth() && !canKillWithOneAttack) {
        Logger::log("vsScout: Scout below 50% HP with no advantage. Attempting health pickup.");
        Command c = tryCollectPickup(game, ai, true);
        if (c != Command::None) {
            return c;
        }
        Logger::log("vsScout: No health pickup found.");
    }

    // If opponent can be killed and we're close, prioritize closing distance
    if (canKillWithOneAttack && distance <= 3) {
        Logger::log("vsScout: Enemy Scout within range and can be killed! Moving to attack position.");
        Direction towardDir = getDirectionTowards(dx, dy);
        if (ai->getDirection() != towardDir) {
            return getTurnCommand(ai->getDirection(), towardDir);
        }
        return tryMoveOrBreakWall(game, ai);
    }

    // Find a path that doesn't end adjacent to the player
    Command moveCommand = findSafePath(game, ai, otherScout);
    if (moveCommand != Command::None) {
        return moveCommand;
    }

    // If we get here, use default scout behavior
    return calculateScoutNormal(game, ai, otherScout);
}

/**
 * Matchup specific logic: vs Sniper
 *   - try to surprise the sniper
 */
Command ScoutAI::vsSniper(Game* game, Robot* ai, Robot* sniper)
{
    QPoint aiPos = ai->getPosition();
    QPoint plrPos = sniper->getPosition();
    int dx = plrPos.x() - aiPos.x();
    int dy = plrPos.y() - aiPos.y();
    int distance = std::abs(dx) + std::abs(dy);
    int totalMovesAtStartOfTurn = ai->getMaxMoves();
    int movesUsedThisTurn = totalMovesAtStartOfTurn - ai->getMovesLeft();

    Logger::log(QString("vsSniper: Scout(%1,%2), Sniper(%3,%4), distance %5, moves used: %6/%7")
                .arg(aiPos.x()).arg(aiPos.y())
                .arg(plrPos.x()).arg(plrPos.y())
                .arg(distance)
                .arg(movesUsedThisTurn)
                .arg(totalMovesAtStartOfTurn));

    bool canSee = hasLineOfSight(game, aiPos, plrPos);
    bool playerInDir = isInDirection(dx, dy, ai->getDirection());
    GameDifficulty diff = game->getDifficulty();
    Q_UNUSED(canSee);
    Q_UNUSED(diff);
    
    // Check if the sniper can be killed with one more attack
    bool canKillWithOneAttack = (sniper->getHealth() <= ai->getAttackDamage());
    if (canKillWithOneAttack) {
        Logger::log(QString("vsSniper: Sniper has %1 health and our attack is %2. Can kill with one hit!")
                   .arg(sniper->getHealth()).arg(ai->getAttackDamage()));
    }

    // If we start adjacent to Sniper, attack and then move away (hit and dash)
    if (distance == 1) {
        // Check if we're stuck in a turn cycle
        if (samePositionCounter >= 3) {
            Logger::log("vsSniper: Detected potential turn cycle. Breaking out of pattern.");
            
            // Try to move in ANY direction that's valid to break out
            for (int i = 0; i < 4; i++) {
                Direction testDir = static_cast<Direction>(i);
                QPoint newPos = getPositionInDirection(aiPos, testDir);
                
                if (game->isValidMove(newPos)) {
                    if (ai->getDirection() != testDir) {
                        Logger::log(QString("vsSniper: Breaking cycle - turning to ANY valid direction %1").arg(i));
                        return getTurnCommand(ai->getDirection(), testDir);
                    } else {
                        Logger::log("vsSniper: Breaking cycle - already facing valid direction, moving forward");
                        return Command::MoveForward;
                    }
                }
            }
            
            // If no valid moves and facing sniper, attack as a last resort
            if (playerInDir) {
                Logger::log("vsSniper: Breaking cycle - no valid moves, attacking as last resort");
                return Command::Attack;
            } else {
                // Turn toward sniper
                Direction towardDir = getDirectionTowards(dx, dy);
                return getTurnCommand(ai->getDirection(), towardDir);
            }
        }
        
        // First, make sure we're facing the sniper
        if (!playerInDir) {
            Logger::log("vsSniper: Adjacent to sniper but not facing them. Turning to attack.");
            Direction towardDir = getDirectionTowards(dx, dy);
            return getTurnCommand(ai->getDirection(), towardDir);
        }
        
        // Check if we've already attacked this turn
        bool hasAttackedThisTurn = (movesUsedThisTurn > 0);
        
        // If we can kill the sniper with one attack, always do it
        if (canKillWithOneAttack && playerInDir) {
            Logger::log("vsSniper: Sniper can be killed with one attack! Attacking for the kill!");
            return Command::Attack;
        }
        
        // Against snipers, always attack once when adjacent (they're vulnerable up close)
        if (!hasAttackedThisTurn) {
            Logger::log("vsSniper: Adjacent to sniper and facing them. Attacking once!");
            return Command::Attack;
        }
        
        // If we've already attacked but can kill the sniper, finish them off
        if (hasAttackedThisTurn && canKillWithOneAttack && playerInDir) {
            Logger::log("vsSniper: Already attacked but can kill Sniper with one more attack! Finishing it off!");
            return Command::Attack;
        }
        
        // After attacking once, always try to move away
        Logger::log("vsSniper: Already attacked this turn. Moving away to reposition.");
        Direction awayDir = getDirectionAway(dx, dy);
        
        // Check if we can actually move away before turning
        QPoint awayPos = getPositionInDirection(aiPos, awayDir);
        bool canMoveAway = game->isValidMove(awayPos);
        
        // If we can't move directly away, try alternative directions
        if (!canMoveAway) {
            Logger::log("vsSniper: Can't move directly away. Trying alternative directions.");
            
            // Try all directions except toward the sniper
            Direction towardDir = getDirectionTowards(dx, dy);
            for (int i = 0; i < 4; i++) {
                Direction testDir = static_cast<Direction>(i);
                if (testDir == towardDir) continue; // Skip direction toward sniper
                
                QPoint testPos = getPositionInDirection(aiPos, testDir);
                if (game->isValidMove(testPos)) {
                    if (ai->getDirection() != testDir) {
                        Logger::log(QString("vsSniper: Found alternative escape direction %1. Turning.").arg(i));
                        return getTurnCommand(ai->getDirection(), testDir);
                    } else {
                        Logger::log("vsSniper: Already facing alternative escape direction. Moving.");
                        return Command::MoveForward;
                    }
                }
            }
            
            // If we can't move away and haven't attacked, attack as a last resort
            if (!hasAttackedThisTurn && playerInDir) {
                // If the attack will kill, make that clear in the logs
                if (canKillWithOneAttack) {
                    Logger::log("vsSniper: No escape possible but can kill Sniper! Attacking for the kill!");
                } else {
                    Logger::log("vsSniper: No escape possible. Attacking as last resort.");
                }
                return Command::Attack;
            } else if (!playerInDir) {
                // Turn to face the sniper if we can't move away
                Direction towardDir = getDirectionTowards(dx, dy);
                return getTurnCommand(ai->getDirection(), towardDir);
            }
        }
        
        // We can move directly away, so turn if needed
        if (ai->getDirection() != awayDir) {
            Logger::log(QString("vsSniper: Turning to escape direction %1").arg(static_cast<int>(awayDir)));
            return getTurnCommand(ai->getDirection(), awayDir);
        }
        
        // Already facing away, so move
        Logger::log("vsSniper: Moving away from sniper in escape direction");
        return Command::MoveForward;
    }

    // If below half health, try to pick up health
    // Skip healing if we can kill the opponent
    if (ai->getHealth() < ai->getMaxHealth() / 2 && !canKillWithOneAttack) {
        Logger::log("vsSniper: Scout below 50% HP. Attempting health pickup.");
        Command c = tryCollectPickup(game, ai, true);
        if (c != Command::None) {
            return c;
        }
        Logger::log("vsSniper: No health pickup found.");
    }
    
    // If opponent can be killed and we're relatively close, prioritize closing distance
    if (canKillWithOneAttack && distance <= 4) {
        Logger::log("vsSniper: Sniper within range and can be killed! Moving to attack position.");
        Direction towardDir = getDirectionTowards(dx, dy);
        if (ai->getDirection() != towardDir) {
            return getTurnCommand(ai->getDirection(), towardDir);
        }
        return tryMoveOrBreakWall(game, ai);
    }

    // Find a path that gets to the sniper but doesn't end adjacent if it's the last move
    Command moveCommand = findSafePath(game, ai, sniper);
    if (moveCommand != Command::None) {
        return moveCommand;
    }
    
    // If we can't find a safe path, be more aggressive - get closer to the sniper
    // Snipers are dangerous at range, less dangerous up close
    Logger::log("vsSniper: No safe path found. Moving aggressively towards sniper.");
    Direction towardDir = getDirectionTowards(dx, dy);
    if (ai->getDirection() != towardDir) {
        return getTurnCommand(ai->getDirection(), towardDir);
    }
    return tryMoveOrBreakWall(game, ai);
}

/**
 * Matchup specific logic: vs Tank
 *   - try to get powerups
 */
Command ScoutAI::vsTank(Game* game, Robot* ai, Robot* tank)
{
    QPoint aiPos = ai->getPosition();
    QPoint plrPos = tank->getPosition();
    int dx = plrPos.x() - aiPos.x();
    int dy = plrPos.y() - aiPos.y();
    int distance = std::abs(dx) + std::abs(dy);
    int totalMovesAtStartOfTurn = ai->getMaxMoves();
    int movesUsedThisTurn = totalMovesAtStartOfTurn - ai->getMovesLeft();

    Logger::log(QString("vsTank: Scout(%1,%2), Tank(%3,%4), distance %5, moves used: %6/%7")
                .arg(aiPos.x()).arg(aiPos.y())
                .arg(plrPos.x()).arg(plrPos.y())
                .arg(distance)
                .arg(movesUsedThisTurn)
                .arg(totalMovesAtStartOfTurn));

    bool canSee = hasLineOfSight(game, aiPos, plrPos);
    bool playerInDir = isInDirection(dx, dy, ai->getDirection());
    GameDifficulty diff = game->getDifficulty();
    Q_UNUSED(canSee);
    Q_UNUSED(diff);
    
    // Check if the tank can be killed with one more attack
    bool canKillWithOneAttack = (tank->getHealth() <= ai->getAttackDamage());
    if (canKillWithOneAttack) {
        Logger::log(QString("vsTank: Tank has %1 health and our attack is %2. Can kill with one hit!")
                   .arg(tank->getHealth()).arg(ai->getAttackDamage()));
    }
    
    // If we start adjacent to opponent, attack and then move away (hit and dash)
    if (distance == 1) {
        // Check if we're stuck in a turn cycle (adjacent to tank and keep turning)
        if (samePositionCounter >= 3) {
            Logger::log("vsTank: Detected potential turn cycle. Breaking out of pattern.");
            
            // If we've been in the same position for several turns, more aggressively break out
            // Try to move in ANY direction that's valid, even if it's toward the opponent
            for (int i = 0; i < 4; i++) {
                Direction testDir = static_cast<Direction>(i);
                QPoint newPos = getPositionInDirection(aiPos, testDir);
                
                if (game->isValidMove(newPos)) {
                    if (ai->getDirection() != testDir) {
                        Logger::log(QString("vsTank: Breaking cycle - turning to ANY valid direction %1").arg(i));
                        return getTurnCommand(ai->getDirection(), testDir);
                    } else {
                        Logger::log("vsTank: Breaking cycle - already facing valid direction, moving forward");
                        return Command::MoveForward;
                    }
                }
            }
            
            // If no valid moves, attack as a last resort
            if (playerInDir) {
                Logger::log("vsTank: Breaking cycle - no valid moves, attacking as absolute last resort");
                return Command::Attack;
            } else {
                // Turn toward tank
                Direction towardDir = getDirectionTowards(dx, dy);
                return getTurnCommand(ai->getDirection(), towardDir);
            }
        }
        
        // First, make sure we're facing the tank
        if (!playerInDir) {
            Logger::log("vsTank: Adjacent to tank but not facing them. Turning to attack.");
            Direction towardDir = getDirectionTowards(dx, dy);
            return getTurnCommand(ai->getDirection(), towardDir);
        }
        
        // Check if we've already attacked this turn (by checking if moves were used)
        bool hasAttackedThisTurn = (movesUsedThisTurn > 0);
        
        // If we can kill the tank with one attack, prioritize attacking regardless of state
        if (canKillWithOneAttack && playerInDir) {
            Logger::log("vsTank: Tank can be killed with one attack! Attacking for the kill!");
            return Command::Attack;
        }
        
        // Against tanks, if we're adjacent and facing them, attack once and then try to move away
        if (!hasAttackedThisTurn) {
            Logger::log("vsTank: Adjacent to tank and facing them. Attacking once!");
            return Command::Attack;
        }
        
        // If we've already attacked this turn but can still kill the tank with another attack, do it
        if (hasAttackedThisTurn && canKillWithOneAttack && playerInDir) {
            Logger::log("vsTank: Already attacked but Tank can be killed with one more attack! Finishing it off!");
            return Command::Attack;
        }
        
        // After attacking once, always try to escape regardless of remaining moves
        Logger::log("vsTank: Already attacked this turn. Moving away to safety.");
        Direction awayDir = getDirectionAway(dx, dy);
        
        // Check if we can actually move away before turning
        QPoint awayPos = getPositionInDirection(aiPos, awayDir);
        bool canMoveAway = game->isValidMove(awayPos);
        
        // If we can't move away, try any other valid direction except toward the tank
        if (!canMoveAway) {
            Logger::log("vsTank: Can't move directly away from tank. Trying alternative directions.");
            
            // Try all directions except the one toward the tank
            Direction towardDir = getDirectionTowards(dx, dy);
            for (int i = 0; i < 4; i++) {
                Direction testDir = static_cast<Direction>(i);
                if (testDir == towardDir) continue; // Skip direction toward tank
                
                QPoint testPos = getPositionInDirection(aiPos, testDir);
                if (game->isValidMove(testPos)) {
                    if (ai->getDirection() != testDir) {
                        Logger::log(QString("vsTank: Found alternative escape direction %1. Turning.").arg(i));
                        return getTurnCommand(ai->getDirection(), testDir);
                    } else {
                        Logger::log("vsTank: Already facing alternative escape direction. Moving.");
                        return Command::MoveForward;
                    }
                }
            }
            
            // If we absolutely can't move in any direction other than toward the tank,
            // attack again as a last resort
            // If attack will kill the tank, make this clearer in the log
            if (canKillWithOneAttack) {
                Logger::log("vsTank: No escape paths but can kill tank! Attacking for the kill!");
            } else {
                Logger::log("vsTank: Absolutely no escape paths. Attacking again as last resort.");
            }
            return Command::Attack;
        }
        
        // We can move directly away, so turn if needed
        if (ai->getDirection() != awayDir) {
            Logger::log(QString("vsTank: Turning to escape direction %1").arg(static_cast<int>(awayDir)));
            return getTurnCommand(ai->getDirection(), awayDir);
        }
        
        // Already facing away direction, so move
        Logger::log("vsTank: Moving away from tank in escape direction");
        return Command::MoveForward;
    }

    // If below half health, try to pick up health
    // But if the tank can be killed, prioritize that over healing
    if (ai->getHealth() < ai->getMaxHealth() / 2 && !canKillWithOneAttack) {
        Logger::log("vsTank: Scout below 50% HP. Attempting health pickup.");
        Command c = tryCollectPickup(game, ai, true);
        if (c != Command::None) {
            return c;
        }
        // Continue with normal strategy if no pickup found
        Logger::log("vsTank: No health pickup found. Continuing with normal strategy.");
    }

    // Within striking range: attack
    if (distance >= 2 && distance <= 5) {
        // If we're not adjacent but close, and the tank can be killed in one hit,
        // prioritize moving closer to kill it
        if (canKillWithOneAttack && distance == 2) {
            Logger::log("vsTank: Tank within range and can be killed! Moving to attack position.");
            Direction towardDir = getDirectionTowards(dx, dy);
            if (ai->getDirection() != towardDir) {
                return getTurnCommand(ai->getDirection(), towardDir);
            }
            return Command::MoveForward;
        }
        
        Logger::log("vsTank: Within striking range. Move towards player.");
        
        // Find a path that doesn't end adjacent to the player
        Command moveCommand = findSafePath(game, ai, tank);
        if (moveCommand != Command::None) {
            return moveCommand;
        }
        
        // If we can't find a safe path, default to standard approach
        Direction towardDir = getDirectionTowards(dx, dy);
        if (ai->getDirection() != towardDir) {
            return getTurnCommand(ai->getDirection(), towardDir);
        }
        return tryMoveOrBreakWall(game, ai);
    }

    // Far away: get powerup
    else {
        // If the tank can be killed in one hit, prioritize closing distance
        if (canKillWithOneAttack) {
            Logger::log("vsTank: Tank far away but can be killed in one hit! Moving toward it.");
            Direction towardDir = getDirectionTowards(dx, dy);
            if (ai->getDirection() != towardDir) {
                return getTurnCommand(ai->getDirection(), towardDir);
            }
            return tryMoveOrBreakWall(game, ai);
        }
        
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

/**
 *   Helper: Attempt to pick up nearest beneficial pickup
 */
Command ScoutAI::tryCollectPickup(Game* game, Robot* ai, bool preferHealthIfLow)
{
    QPoint aiPos = ai->getPosition();
    Logger::log("tryCollectPickup: Searching for pickups.");

    // Find nearest health pickup within 5 tiles
    QPoint hpPos = findNearestHealthPickup(game, aiPos, 5);
    int distHP = 999999;
    if (hpPos.x() != -1)
        distHP = manhattanDistance(aiPos, hpPos);

    // Find nearest powerup within 5 tiles
    QPoint puPos = findNearestPowerUp(game, aiPos, 5);
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
Command ScoutAI::tryMoveOrBreakWall(Game* game, Robot* ai)
{
    QPoint aiPos = ai->getPosition();
    Direction currentDir = ai->getDirection();
    Robot* opponent = game->getPlayerRobot();
    
    // If this is an AI vs AI game, use the first player as the opponent
    if (ai == game->getPlayerRobot()) {
        opponent = game->isMultiplayerMode() ? game->getPlayer2Robot() : game->getAiRobot();
    }

    QPoint forwardPos = getPositionInDirection(aiPos, currentDir);
    
    // Check if moving forward would place us adjacent to the opponent on our last move
    if (game->isValidMove(forwardPos)) {
        if (wouldEndAdjacentToOpponent(game, ai, opponent, forwardPos)) {
            Logger::log("Would end adjacent to opponent with last move. Looking for safer option.");
            
            // Try to find a non-adjacent move
            Direction leftDir = static_cast<Direction>((static_cast<int>(currentDir) + 3) % 4);
            QPoint leftPos = getPositionInDirection(aiPos, leftDir);
            
            Direction rightDir = static_cast<Direction>((static_cast<int>(currentDir) + 1) % 4);
            QPoint rightPos = getPositionInDirection(aiPos, rightDir);
            
            // Check left turn
            if (game->isValidMove(leftPos) && !wouldEndAdjacentToOpponent(game, ai, opponent, leftPos)) {
                Logger::log("Safe move found to the left. Command: TurnLeft.");
                return getTurnCommand(currentDir, leftDir);
            }
            
            // Check right turn
            if (game->isValidMove(rightPos) && !wouldEndAdjacentToOpponent(game, ai, opponent, rightPos)) {
                Logger::log("Safe move found to the right. Command: TurnRight.");
                return getTurnCommand(currentDir, rightDir);
            }
            
            // If no safe move found, try backing away from opponent if possible
            Direction awayDir = getDirectionAway(opponent->getPosition().x() - aiPos.x(), 
                                               opponent->getPosition().y() - aiPos.y());
            
            if (awayDir != currentDir) {
                Logger::log("No safe move found. Trying to turn away from opponent.");
                return getTurnCommand(currentDir, awayDir);
            }
            
            // As a last resort, if the scout has more than one move left, still move forward
            // rather than wasting the turn
            if (ai->getMovesLeft() > 1) {
                Logger::log("No safe alternatives found, but still have moves left. Moving forward anyway.");
                return Command::MoveForward;
            }
            
            // If it's really the last move and no good options, just end turn (return None)
            Logger::log("Last move with no safe options. Ending turn early.");
            return Command::None;
        }
        
        Logger::log("Forward move is valid and safe. Command: MoveForward.");
        return Command::MoveForward;
    }
    
    if (game->isValidPosition(forwardPos) &&
        game->getCellType(forwardPos) == CellType::Wall)
    {
        Logger::log("Wall detected directly ahead. Command: Attack.");
        return Command::Attack;
    }

    // Try turning left/right to find a valid path.
    for (int i = 0; i < 4; i++) {
        Direction rightDir = static_cast<Direction>((static_cast<int>(currentDir) + 1) % 4);
        QPoint rightPos = getPositionInDirection(aiPos, rightDir);

        Direction leftDir = static_cast<Direction>((static_cast<int>(currentDir) + 3) % 4);
        QPoint leftPos = getPositionInDirection(aiPos, leftDir);

        bool rightSafe = game->isValidMove(rightPos) && !wouldEndAdjacentToOpponent(game, ai, opponent, rightPos);
        bool leftSafe = game->isValidMove(leftPos) && !wouldEndAdjacentToOpponent(game, ai, opponent, leftPos);

        if (leftSafe && rightSafe) {
            Logger::log("Valid move found to the left and right. Command: Random Turn.");
            return (QRandomGenerator::global()->bounded(2) == 0)
               ? getTurnCommand(currentDir, leftDir)
               : getTurnCommand(currentDir, rightDir);
        }

        if (leftSafe) {
            Logger::log("Valid, safe move found to the left. Command: TurnLeft.");
            return getTurnCommand(currentDir, leftDir);
        }
        
        if (game->isValidMove(leftPos) && ai->getMovesLeft() > 1) {
            // Not last move, so okay to go there even if adjacent
            Logger::log("Valid move found to the left (not last move). Command: TurnLeft.");
            return getTurnCommand(currentDir, leftDir);
        }

        if (game->isValidPosition(leftPos) &&
            game->getCellType(leftPos) == CellType::Wall)
        {
            Logger::log("Wall detected on the left. Command: TurnLeft.");
            return getTurnCommand(currentDir, leftDir);
        }

        if (rightSafe) {
            Logger::log("Valid, safe move found to the right. Command: TurnRight.");
            return getTurnCommand(currentDir, rightDir);
        }
        
        if (game->isValidMove(rightPos) && ai->getMovesLeft() > 1) {
            // Not last move, so okay to go there even if adjacent
            Logger::log("Valid move found to the right (not last move). Command: TurnRight.");
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

    // If it's the last move and we couldn't find a safe option, end turn
    if (ai->getMovesLeft() == 1) {
        Logger::log("Last move with no safe paths found. Ending turn early.");
        return Command::None;
    }

    Logger::log("tryMoveOrBreakWall: No valid moves found. Fallback: Attack.");
    return Command::Attack;
}

Command ScoutAI::huntPlayerPosition(Game* game, Robot* ai, int dx, int dy)
{
    Direction desiredDir = getDirectionTowards(dx, dy);
    Direction currentDir = ai->getDirection();
    Logger::log("huntPlayerPosition: Hunting enemy based on last known position.");
    if (currentDir != desiredDir) {
        Logger::log("Not facing desired direction. Command: Turning.");
        return getTurnCommand(currentDir, desiredDir);
    }
    Logger::log("Facing desired direction. Attempting to move or break wall.");
    return tryMoveOrBreakWall(game, ai);
}

/**
 *   Utility Methods
 */
bool ScoutAI::hasLineOfSight(Game* game, const QPoint& from, const QPoint& to)
{
    return game->hasLineOfSight(from, to);
}

bool ScoutAI::isInDirection(int dx, int dy, Direction dir)
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

int ScoutAI::manhattanDistance(const QPoint& p1, const QPoint& p2)
{
    return std::abs(p1.x() - p2.x()) + std::abs(p1.y() - p2.y());
}

Direction ScoutAI::getDirectionTowards(int dx, int dy)
{
    if (std::abs(dx) > std::abs(dy))
        return (dx > 0) ? Direction::East : Direction::West;
    else
        return (dy > 0) ? Direction::South : Direction::North;
}

Direction ScoutAI::getDirectionAway(int dx, int dy)
{
    if (std::abs(dx) > std::abs(dy))
        return (dx > 0) ? Direction::West : Direction::East;
    else
        return (dy > 0) ? Direction::North : Direction::South;
}

Command ScoutAI::getTurnCommand(Direction currentDir, Direction targetDir)
{
    int c = static_cast<int>(currentDir);
    int t = static_cast<int>(targetDir);
    int diff = (t - c + 4) % 4;

    if (diff == 1)
        return Command::TurnRight;
    else if (diff == 2)
        return Command::TurnRight; // For 180° rotation, choose TurnRight arbitrarily.
    else // diff == 3
        return Command::TurnLeft;
}

QPoint ScoutAI::getPositionInDirection(const QPoint& pos, Direction dir, int steps)
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
QPoint ScoutAI::findNearestHealthPickup(Game* game, const QPoint& pos, int searchRadius)
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

QPoint ScoutAI::findNearestPowerUp(Game* game, const QPoint& pos, int searchRadius)
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

Command ScoutAI::directLineAttack(Game* game, Robot* ai, Robot* player)
{
    QPoint aiPos = ai->getPosition();
    QPoint playerPos = player->getPosition();
    int dx = playerPos.x() - aiPos.x();
    int dy = playerPos.y() - aiPos.y();
    int distance = std::abs(dx) + std::abs(dy);

    if (distance <= 1 && (aiPos.x() == playerPos.x() || aiPos.y() == playerPos.y())) {
        Direction desiredDir = getDirectionTowards(dx, dy);
        if (ai->getHealth() < static_cast<int>(ai->getMaxHealth() * 0.4)) {
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

/**
 * Check if moving to a position would place the scout adjacent to the opponent
 * and if this is the last move of the turn
 */
bool ScoutAI::wouldEndAdjacentToOpponent(Game* game, Robot* ai, Robot* opponent, const QPoint& newPos) {
    if (ai->getMovesLeft() > 1) {
        // Not the last move, so it's okay to be adjacent
        return false;
    }
    
    QPoint opponentPos = opponent->getPosition();
    int newDistance = manhattanDistance(newPos, opponentPos);
    
    Logger::log(QString("Checking if move would end adjacent to opponent: distance=%1").arg(newDistance));
    
    // Check if this would place us adjacent (distance=1) to the opponent
    return (newDistance == 1);
}

/**
 * Find a safe path that moves towards the opponent but doesn't end adjacent
 * Returns a command to execute or Command::None if no safe path found
 */
Command ScoutAI::findSafePath(Game* game, Robot* ai, Robot* opponent) {
    QPoint aiPos = ai->getPosition();
    QPoint opponentPos = opponent->getPosition();
    Direction currentDir = ai->getDirection();
    int movesLeft = ai->getMovesLeft();
    
    // Track if we've tried to turn recently but need to move forward now
    static Direction lastTurnDir = Direction::North;  // Initial value doesn't matter
    static bool justTurned = false;
    
    // If we just turned last time, we should move forward now
    if (justTurned && currentDir == lastTurnDir) {
        Logger::log("findSafePath: Just turned last time, now moving forward to avoid loop.");
        justTurned = false;  // Reset the flag
        
        // Check if moving forward is valid
        QPoint forwardPos = getPositionInDirection(aiPos, currentDir);
        if (game->isValidMove(forwardPos)) {
            // Reset turn counter since we're moving
            turnCounter = 0;
            return Command::MoveForward;
        }
    }
    
    // Increase turn counter to detect infinite turning
    turnCounter++;
    
    // If we've been turning too much without moving, force forward movement
    if (turnCounter > 3) {  // Reduced from 5 to be more aggressive about breaking loops
        Logger::log(QString("findSafePath: Detected excessive turning (%1 turns). Forcing movement.").arg(turnCounter));
        turnCounter = 0;
        
        // Try starting with the current direction
        QPoint forwardPos = getPositionInDirection(aiPos, currentDir);
        if (game->isValidMove(forwardPos)) {
            Logger::log("findSafePath: Can move forward in current direction. Moving forward.");
            return Command::MoveForward;
        }
        
        // Try all directions to find any valid move
        for (int i = 0; i < 4; i++) {
            Direction testDir = static_cast<Direction>(i);
            if (testDir == currentDir) continue;  // Skip current direction, already checked
            
            QPoint newPos = getPositionInDirection(aiPos, testDir);
            
            if (game->isValidMove(newPos)) {
                Logger::log(QString("findSafePath: Forcing turn to direction %1 to break loop.").arg(i));
                lastTurnDir = testDir;
                justTurned = true;  // Mark that we just turned
                return getTurnCommand(currentDir, testDir);
            }
        }
        
        // If absolutely no valid move exists, try to attack
        Logger::log("findSafePath: No valid move found. Trying to attack if there's a wall.");
        QPoint frontPos = getPositionInDirection(aiPos, currentDir);
        if (game->isValidPosition(frontPos) && 
            game->getCellType(frontPos) == CellType::Wall) {
            return Command::Attack;
        }
    }
    
    Logger::log(QString("findSafePath: Scout has %1 moves left").arg(movesLeft));
    
    // Only worry about adjacency on the last move
    if (movesLeft > 1) {
        // If we have multiple moves, we can move directly towards the opponent
        Direction towardDir = getDirectionTowards(opponentPos.x() - aiPos.x(), 
                                                 opponentPos.y() - aiPos.y());
                                                 
        // If we need to turn, do that first
        if (currentDir != towardDir) {
            Logger::log(QString("findSafePath: Multiple moves left, turning towards opponent from dir %1 to %2.")
                       .arg(static_cast<int>(currentDir))
                       .arg(static_cast<int>(towardDir)));
            
            lastTurnDir = towardDir;
            justTurned = true;  // Mark that we just turned
            return getTurnCommand(currentDir, towardDir);
        }
        
        // Already facing the right direction, so move forward
        QPoint forwardPos = getPositionInDirection(aiPos, currentDir);
        if (game->isValidMove(forwardPos)) {
            Logger::log("findSafePath: Multiple moves left, moving forward.");
            justTurned = false;  // Reset the flag since we're moving
            // Reset turn counter since we're moving
            turnCounter = 0;
            return Command::MoveForward;
        }
    }
    
    // For the last move, need to find a position that:
    // 1. Moves towards the opponent if possible
    // 2. Doesn't end adjacent to the opponent
    // 3. Is a valid move
    
    // Get the distance to the opponent
    int currentDistance = manhattanDistance(aiPos, opponentPos);
    Logger::log(QString("findSafePath: Current distance to opponent = %1").arg(currentDistance));
    
    // If we're already at distance 2, we want to stay there (ideal attack position)
    if (currentDistance == 2) {
        // First try to move forward if we're already facing correctly
        QPoint forwardPos = getPositionInDirection(aiPos, currentDir);
        if (game->isValidMove(forwardPos) && manhattanDistance(forwardPos, opponentPos) == 2) {
            Logger::log("findSafePath: Already facing direction that maintains distance 2. Moving forward.");
            // Reset turn counter since we're moving
            turnCounter = 0;
            justTurned = false;  // Reset the flag since we're moving
            return Command::MoveForward;
        }
        
        // Try to find a move that maintains distance 2
        for (int i = 0; i < 4; i++) {
            Direction testDir = static_cast<Direction>(i);
            if (testDir == currentDir) continue;  // Skip current direction, already checked
            
            QPoint newPos = getPositionInDirection(aiPos, testDir);
            
            if (game->isValidMove(newPos) && manhattanDistance(newPos, opponentPos) == 2) {
                Logger::log(QString("findSafePath: Found direction %1 that maintains distance 2. Turning.").arg(i));
                lastTurnDir = testDir;
                justTurned = true;  // Mark that we just turned
                return getTurnCommand(currentDir, testDir);
            }
        }
    }
    
    // If we're at distance > 2, try to get closer but not adjacent
    if (currentDistance > 2) {
        // First, check if moving forward reduces the distance without making us adjacent
        QPoint forwardPos = getPositionInDirection(aiPos, currentDir);
        if (game->isValidMove(forwardPos)) {
            int forwardDist = manhattanDistance(forwardPos, opponentPos);
            if ((forwardDist == 2) || (forwardDist < currentDistance && forwardDist > 1)) {
                Logger::log("findSafePath: Moving forward reduces distance appropriately. Moving forward.");
                // Reset turn counter since we're moving
                turnCounter = 0;
                justTurned = false;  // Reset the flag since we're moving
                return Command::MoveForward;
            }
        }
        
        // Try each direction to see if it brings us closer without making us adjacent
        Direction bestDir = currentDir;
        int bestDist = 999;  // Use a high value, not currentDistance, to ensure we find something
        bool foundBetter = false;
        
        for (int i = 0; i < 4; i++) {
            Direction testDir = static_cast<Direction>(i);
            if (testDir == currentDir) continue;  // Skip current direction, already checked
            
            QPoint newPos = getPositionInDirection(aiPos, testDir);
            
            if (game->isValidMove(newPos)) {
                int newDist = manhattanDistance(newPos, opponentPos);
                
                // Prefer distance of 2 over anything else
                if (newDist == 2) {
                    bestDir = testDir;
                    bestDist = newDist;
                    foundBetter = true;
                    break;  // Distance 2 is optimal, stop searching
                }
                
                // Otherwise prefer moves that get us closer but not adjacent
                else if (newDist > 1 && newDist < bestDist) {
                    bestDir = testDir;
                    bestDist = newDist;
                    foundBetter = true;
                }
            }
        }
        
        if (foundBetter) {
            // We found a better direction, so turn that way
            Logger::log(QString("findSafePath: Found better direction %1 with distance %2. Turning.")
                       .arg(static_cast<int>(bestDir))
                       .arg(bestDist));
            lastTurnDir = bestDir;
            justTurned = true;  // Mark that we just turned
            return getTurnCommand(currentDir, bestDir);
        }
    }
    
    // If we're at distance 1, move away to distance 2+
    if (currentDistance == 1) {
        Direction awayDir = getDirectionAway(opponentPos.x() - aiPos.x(), 
                                           opponentPos.y() - aiPos.y());
        
        // If already facing away, move forward
        if (currentDir == awayDir) {
            QPoint awayPos = getPositionInDirection(aiPos, awayDir);
            if (game->isValidMove(awayPos)) {
                Logger::log("findSafePath: Already facing away from opponent. Moving forward.");
                // Reset turn counter since we're moving
                turnCounter = 0;
                justTurned = false;  // Reset the flag since we're moving
                return Command::MoveForward;
            }
        } else {
            // Need to turn away
            Logger::log("findSafePath: Too close. Turning away from opponent.");
            lastTurnDir = awayDir;
            justTurned = true;  // Mark that we just turned
            return getTurnCommand(currentDir, awayDir);
        }
    }
    
    // If we've reached here, we need to find ANY valid move as a fallback
    
    // First, try moving forward if possible
    QPoint forwardPos = getPositionInDirection(aiPos, currentDir);
    if (game->isValidMove(forwardPos)) {
        Logger::log("findSafePath: Fallback - already facing a valid direction. Moving forward.");
        turnCounter = 0;
        justTurned = false;  // Reset the flag since we're moving
        return Command::MoveForward;
    }
    
    // Otherwise, find any other valid direction
    for (int i = 0; i < 4; i++) {
        Direction testDir = static_cast<Direction>(i);
        if (testDir == currentDir) continue;  // Skip current direction, already checked
        
        QPoint newPos = getPositionInDirection(aiPos, testDir);
        
        if (game->isValidMove(newPos)) {
            Logger::log(QString("findSafePath: Fallback - turning to valid direction %1.").arg(i));
            lastTurnDir = testDir;
            justTurned = true;  // Mark that we just turned
            return getTurnCommand(currentDir, testDir);
        }
    }
    
    // If we can't find a good path, indicate failure
    Logger::log("findSafePath: No path found at all.");
    // Reset turn counter since we're giving up
    turnCounter = 0;
    justTurned = false;  // Reset the flag since we're giving up
    return Command::None;
}