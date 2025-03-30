#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include <QPoint>
#include <QString>
#include <QPixmap>
#include <QTransform>

///@brief The Direction enumeration is used to determine the direction of projectile projection
enum class Direction { North, East, South, West };

/// @brief Types of robots that exists, right now we have Scout, tank, and sniper.
enum class RobotType {
    Scout,      // Fast but weak
    Tank,       // Slow but strong
    Sniper      // Long range
};

/// @brief Number of robot power-ups in the arena. We have none, laser, missile, and bomb.
enum class RobotPowerUp {
    None,
    Laser,
    Missile,
    Bomb
};

/// @brief The Robot class holds all the information of the player tank. Such as health, position, or attack damage.
/// @author Group 17
class Robot : public QObject {
    Q_OBJECT
public:
    /// @brief Initalise the robot object
    /// @param type - The type of robot the player uses
    /// @param parent - The parent QObject of this object
    explicit Robot(RobotType type = RobotType::Scout, QObject *parent = nullptr);

    /// @brief Simple method for the robot to move forward
    ///@see useMove()
    void moveForward();
    /// @brief Simple method for the robot to turn
    void turnLeft();
    /// @brief Simple method for the robot to turn
    void turnRight();
    /// @brief Simple method for the robot to undo a move
    void undoLastMove() { movesLeft++; }
    /// @brief Simple method for robot to commit a move
    void useMove() { if (movesLeft > 0) { movesLeft--; emit movesChanged(movesLeft); } }
    /// @brief Sets the health of the robot
    /// @param newHealth - The health our robot now has
    void setHealth(int newHealth) { health = newHealth; emit healthChanged(health); }
    /// @brief Returns the robot type of this robot
    /// @return robot type of the robot
    RobotType getRobotType() const;
    /// @brief Attack
    /// @param target - The target of our attack
    /// @return TRUE if attack performed successfully, FALSE otherwise
    bool attack(Robot* target);
    /// @brief Checks if another robot is in range
    /// @param target - the target robot we want to check is in this robot's range
    /// @return TRUE if target is in range, FALSE otherwise
    bool isInRange(const Robot* target) const;
    
    /// @brief Returns current position of this robot object
    /// @return current position of this robot object
    QPoint getPosition() const { return position; }
    /// @brief Returns the direction this robot is facing
    /// @return direction this robot is currently facing
    Direction getDirection() const { return direction; }
    /// @return health of this robot object
    int getHealth() const { return health; }
    /// @return max health of this robot object
    int getMaxHealth() const { return maxHealth; }
    /// @return the attack damage of this robot object
    int getAttackDamage() const { return attackDamage; }
    /// @brief Set the position of the robot into a predetermined position
    /// @param pos - Said predetermined position, the position this robot object will be change into.
    void setPosition(const QPoint& pos) { position = pos; }
    /// @brief Checks if the robot object is dead
    /// @return TRUE if dead, FALSE otherwise
    bool isDead() const { return health <= 0; }
    /// @brief Checks the type of robot of this object
    /// @return the type of robot this object is
    RobotType getType() const { return type; }
    /// @return The sprite resource of this robot object at top-view as a QString
    QString getTopViewSpriteResource() const;
    /// @return The sprite resource of this robot object at side-view as a QString
    QString getSideViewSpriteResource() const;
    /// @brief Displays this robot character
    QString getDisplayChar() const;
    /// @return Get this sprite of this robot from top-view
    QPixmap getTopViewSprite() const;
    /// @return Get this sprite of this robot from side-view
    QPixmap getSideViewSprite() const;
    /// @brief Function that updates the animation
    void updateAnimation();
    /// @brief Checks if this object is moving
    /// @return TRUE is the object is moving, false otherwise
    bool isMoving() const { return moving; }
    /// @brief Function used to obtain basic info of a robot object instance
    /// @return Returns a discription of this object based on a certain format
    QString getDescription() const;
    /// @brief Used to check the maximum amount of moves this robot object can use in a turn
    /// @return The maximum amount of moves this robot can do in a turn
    int getMaxMoves() const { return maxMovesPerTurn; }
    /// @brief Used to check the amount of moves left in a robot object
    /// @return The number of moves left in the robot this function is invoked upon
    int getMovesLeft() const { return movesLeft; }
    /// @brief Resets the number of moves a robot object can do
    void resetMoves() { movesLeft = maxMovesPerTurn; emit movesChanged(movesLeft); }

    /// @return The current power-up that the robot object has
    RobotPowerUp getPowerUp() const { return currentPowerUp; }
    /// @brief Setter function that allows the robot to replace it's power-up
    /// @param pu - The new power-up that the robot object will obtain
    void setPowerUp(RobotPowerUp pu) { currentPowerUp = pu; }

signals:
    void healthChanged(int newHealth);
    void positionChanged(const QPoint& newPos);
    void directionChanged(Direction newDir);
    void movesChanged(int movesLeft);

private:
    QPoint position;
    Direction direction;
    RobotType type;
    int health;
    int maxHealth;
    int attackRange;
    int attackDamage;
    int maxMovesPerTurn;
    int movesLeft;
    bool moving;
    int animationFrame;

    RobotPowerUp currentPowerUp = RobotPowerUp::None;
};

#endif // ROBOT_H
