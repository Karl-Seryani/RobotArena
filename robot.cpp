#include "robot.h"
#include <QDebug>

Robot::Robot(RobotType type, QObject *parent) : QObject(parent),
    position(0, 0),
    direction(Direction::East),
    type(type),
    moving(false),
    animationFrame(0) {
    
    // Set stats based on robot type
    switch (type) {
        case RobotType::Scout:
            maxHealth = 70;
            attackRange = 1;
            attackDamage = 15;
            maxMovesPerTurn = 3;
            break;
            
        case RobotType::Tank:
            maxHealth = 150;
            attackRange = 1;
            attackDamage = 25;
            maxMovesPerTurn = 2;
            break;
            
        case RobotType::Sniper:
            maxHealth = 80;
            attackRange = 3;
            attackDamage = 35;
            maxMovesPerTurn = 2;
            break;
            
        default:
            // Default to Scout if an invalid type is provided
            maxHealth = 70;
            attackRange = 1;
            attackDamage = 15;
            maxMovesPerTurn = 3;
            break;
    }
    
    health = maxHealth;
    movesLeft = maxMovesPerTurn;
}


RobotType Robot::getRobotType() const {
    return type;
}
void Robot::moveForward() {
    if (movesLeft <= 0) return;
    
    QPoint newPos = position;
    switch (direction) {
        case Direction::North: newPos.setY(position.y() - 1); break;
        case Direction::South: newPos.setY(position.y() + 1); break;
        case Direction::East: newPos.setX(position.x() + 1); break;
        case Direction::West: newPos.setX(position.x() - 1); break;
    }
    position = newPos;
    movesLeft--;
    moving = true;
    updateAnimation();
    emit positionChanged(position);
    emit movesChanged(movesLeft);
}

void Robot::turnLeft() {
    switch (direction) {
        case Direction::North: direction = Direction::West; break;
        case Direction::West: direction = Direction::South; break;
        case Direction::South: direction = Direction::East; break;
        case Direction::East: direction = Direction::North; break;
    }
    emit directionChanged(direction);
}

void Robot::turnRight() {
    switch (direction) {
        case Direction::North: direction = Direction::East; break;
        case Direction::East: direction = Direction::South; break;
        case Direction::South: direction = Direction::West; break;
        case Direction::West: direction = Direction::North; break;
    }
    emit directionChanged(direction);
}

bool Robot::attack(Robot* target) {
    if (!target || !isInRange(target)) return false;
    
    target->health = std::max(0, target->health - attackDamage);
    emit target->healthChanged(target->health);
    return true;
}

bool Robot::isInRange(const Robot* target) const {
    if (!target) return false;
    
    // Get positions
    int dx = target->position.x() - position.x();
    int dy = target->position.y() - position.y();
    
    // Check if robots are in the same row or column (line of sight)
    bool inLineOfSight = (dx == 0 || dy == 0);
    if (!inLineOfSight) return false;
    
    // Calculate Manhattan distance
    int distance = abs(dx) + abs(dy);
    
    // Sniper can attack from 3 blocks away, others only from 1 block
    int maxRange = (type == RobotType::Sniper) ? 3 : 1;
    
    return distance <= maxRange;
}

QString Robot::getTopViewSpriteResource() const {
    switch (type) {
        case RobotType::Scout:  return ":/sprites/Sprite/Top view/robot_3Dblue.png";
        case RobotType::Tank:   return ":/sprites/Sprite/Top view/robot_3Dred.png";
        case RobotType::Sniper: return ":/sprites/Sprite/Top view/robot_3Dgreen.png";
        default:                return ":/sprites/Sprite/Top view/robot_3Dblue.png"; // Default to Scout
    }
}

QString Robot::getSideViewSpriteResource() const {
    QString baseColor;
    switch (type) {
        case RobotType::Scout:  baseColor = "blue"; break;
        case RobotType::Tank:   baseColor = "red"; break;
        case RobotType::Sniper: baseColor = "green"; break;
        default:                baseColor = "blue"; break; // Default to Scout
    }
    
    if (moving) {
        return QString(":/sprites/Sprite/Side view/robot_%1Drive%2.png")
            .arg(baseColor)
            .arg(animationFrame + 1);
    } else {
        return QString(":/sprites/Sprite/Side view/robot_%1Body.png")
            .arg(baseColor);
    }
}

QPixmap Robot::getTopViewSprite() const {
    QPixmap sprite(getTopViewSpriteResource());
    // The sprite initially faces East (right), so adjust rotations accordingly
    QTransform transform;
    switch (direction) {
        case Direction::East: break; // No rotation needed as sprite already faces right
        case Direction::South: transform.rotate(90); break;
        case Direction::West: transform.rotate(180); break;
        case Direction::North: transform.rotate(270); break;
    }
    return sprite.transformed(transform);
}

QPixmap Robot::getSideViewSprite() const {
    QPixmap sprite(getSideViewSpriteResource());
    // For side view, we only need to mirror the sprite for west direction
    if (direction == Direction::West) {
        return sprite.transformed(QTransform().scale(-1, 1));
    }
    return sprite;
}

void Robot::updateAnimation() {
    if (moving) {
        animationFrame = (animationFrame + 1) % 2; // Toggle between 0 and 1
        emit positionChanged(position); // Trigger a redraw with new animation frame
        moving = false;
    }
}

QString Robot::getDescription() const {
    switch (type) {
        case RobotType::Scout:
            return "Scout - Fast and agile (3 moves/turn), but low health and damage";
        case RobotType::Tank:
            return "Tank - High health and good damage with decent mobility (2 moves/turn)";
        case RobotType::Sniper:
            return "Sniper - Long range (3 tiles) and high damage with decent mobility (2 moves/turn)";
        default:
            return "Unknown robot type";
    }
}

QString Robot::getDisplayChar() const {
    switch (type) {
        case RobotType::Scout:  return "S"; // Character for Scout
        case RobotType::Tank:   return "T"; // Character for Tank
        case RobotType::Sniper: return "N"; // Character for Sniper
        default:                return "?"; // Unknown type
    }
}
