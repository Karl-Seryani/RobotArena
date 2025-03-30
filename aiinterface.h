#ifndef AIINTERFACE_H
#define AIINTERFACE_H

class Game;
class Robot;
enum class Command;
enum class Direction;
enum class CellType;
enum class RobotType;
enum class GameDifficulty;

/// Simple interface for all robotAI, this will be implemented wherever a robotAI is being used
/// @author Group 17
class AIInterface
{
public:
    /// @brief simple method to delete the interface
    virtual ~AIInterface() = default;
    /// @brief calculate what move the AI should do based on it's behaviorial patterns
    virtual Command calculateMove(Game* game, Robot* ai, Robot* player) = 0;
};

#endif // AIINTERFACE_H
