#include "robotai.h"
#include "game.h"
#include "sniperai.h"
#include "scoutai.h"
#include "tankai.h"
#include "robot.h"

RobotAI::RobotAI(QObject *parent) : QObject(parent) {
    sniperAI = new SniperAI(this);
    scoutAI = new ScoutAI(this);
    tankAI = new TankAI(this);
}

Command RobotAI::calculateMove(Game* game, Robot* ai, Robot* player) {
    // Choose strategy based on robot type
    switch (ai->getType()) {
        case RobotType::Scout:
            return scoutAI->calculateMove(game, ai, player);
            
        case RobotType::Tank:
            return tankAI->calculateMove(game, ai, player);
            
        case RobotType::Sniper:
            return sniperAI->calculateMove(game, ai, player);
            
        default:
            // Default to Scout if an invalid type is provided
            return scoutAI->calculateMove(game, ai, player);
    }
} 