#ifndef ROBOTAI_H
#define ROBOTAI_H

#include <QObject>

/// Forward declarations
class Game;
/// Forward declarations
class Robot;
/// Forward declarations
class SniperAI;
/// Forward declarations
class ScoutAI;
/// Forward declarations
class TankAI;
/// Forward declarations
enum class Command;


///@brief RobotAI - Factory/manager class that delegates AI decision making to specialized.
/// AI classes (ScoutAI, TankAI, SniperAI) based on the robot type.
///@author Group 17
class RobotAI : public QObject {
    Q_OBJECT
public:
    /// @brief Creates a new RobotAI object
    /// @param parent - The QObject parent of the initalised object
    explicit RobotAI(QObject *parent = nullptr);
    
    ///Main method to calculate the next AI move
    ///@param game Current game state
    ///@param ai The AI robot making the move
    ///@param player The player/opponent robot
    ///@return Command to execute
    Command calculateMove(Game* game, Robot* ai, Robot* player);
    
private:
    /// Specialized AI instances
    SniperAI* sniperAI;
    ScoutAI* scoutAI;
    TankAI* tankAI;
};

#endif // ROBOTAI_H 