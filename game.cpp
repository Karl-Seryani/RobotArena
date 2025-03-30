#include "game.h"
#include <QRandomGenerator>
#include "robotai.h"

Game::Game(int size, QObject *parent) 
    : QObject(parent), state(GameState::PlayerTurn), gridSize(size), 
      difficulty(GameDifficulty::Medium), mapType(MapType::Random),
      multiplayerMode(false),
      aiHealthModifier(1.0f), aiDamageModifier(1.0f), aiRandomMoveChance(0.2f) {
    
    playerRobot = std::make_unique<Robot>();
    player2Robot = std::make_unique<Robot>();
    aiRobot = std::make_unique<Robot>();
    robotAI = std::make_unique<RobotAI>();
    arena.resize(gridSize, std::vector<CellType>(gridSize, CellType::Empty));
    wallHealth.resize(gridSize, std::vector<int>(gridSize, 0));

    initializeArena(playerRobot->getRobotType(), aiRobot->getRobotType(), difficulty, mapType);
}

Game::~Game() {
    // Clean up resources if needed
}

void Game::initializeArena(const RobotType& playerType, const RobotType& aiType, 
                           GameDifficulty diff, MapType map) {
    // Set difficulty and map type
    difficulty = diff;
    mapType = map;
    multiplayerMode = false;
    
    // Clear arena
    for (auto& row : arena) {
        std::fill(row.begin(), row.end(), CellType::Empty);
    }
    
    // Clear wall health
    for (auto& row : wallHealth) {
        std::fill(row.begin(), row.end(), 0);
    }

    // Generate map based on selected type
    switch (mapType) {
        case MapType::Random:
            generateObstacles();
            break;
        case MapType::Open:
            generateOpenMap();
            break;
        case MapType::Maze:
            generateMazeMap();
            break;
        case MapType::Fortress:
            generateFortressMap();
            break;
    }

    // Create robots based on the types passed in
    playerRobot = std::make_unique<Robot>(playerType);
    aiRobot = std::make_unique<Robot>(aiType);
    
    // Apply difficulty settings
    applyDifficultySettings();

    // Position robots at opposite corners with clear paths
    playerRobot->setPosition(QPoint(0, gridSize - 1));
    aiRobot->setPosition(QPoint(gridSize - 1, 0));

    // Ensure starting positions are clear
    arena[gridSize - 1][0] = CellType::Empty;
    arena[0][gridSize - 1] = CellType::Empty;

    // Place health pickups randomly on the map
    placeHealthPickups();

    // Place powerups randomly
    placeSpecialPickups();

    // Set initial state
    state = GameState::PlayerTurn;
    
    emit arenaInitialized();
}

void Game::initializeMultiplayerArena(const RobotType& player1Type, const RobotType& player2Type, 
                                     MapType map) {
    // Set map type and enable multiplayer mode
    mapType = map;
    multiplayerMode = true;
    
    // Clear arena
    for (auto& row : arena) {
        std::fill(row.begin(), row.end(), CellType::Empty);
    }
    
    // Clear wall health
    for (auto& row : wallHealth) {
        std::fill(row.begin(), row.end(), 0);
    }

    // Generate map based on selected type
    switch (mapType) {
        case MapType::Random:
            generateObstacles();
            break;
        case MapType::Open:
            generateOpenMap();
            break;
        case MapType::Maze:
            generateMazeMap();
            break;
        case MapType::Fortress:
            generateFortressMap();
            break;
    }

    // Create robots based on the types passed in
    playerRobot = std::make_unique<Robot>(player1Type);
    player2Robot = std::make_unique<Robot>(player2Type);
    
    // Position robots at opposite corners with clear paths
    playerRobot->setPosition(QPoint(0, gridSize - 1));
    player2Robot->setPosition(QPoint(gridSize - 1, 0));

    // Ensure starting positions are clear
    arena[gridSize - 1][0] = CellType::Empty;
    arena[0][gridSize - 1] = CellType::Empty;

    // Place health pickups randomly on the map
    placeHealthPickups();

    // Place powerups randomly
    placeSpecialPickups();

    // Set initial state
    state = GameState::PlayerTurn;
    
    emit arenaInitialized();
}

void Game::setMapType(MapType map) {
    mapType = map;
}

void Game::setMultiplayerMode(bool enabled) {
    multiplayerMode = enabled;
}

void Game::applyDifficultySettings() {
    // Set difficulty modifiers based on selected difficulty
    switch (difficulty) {
        case GameDifficulty::Easy:
            aiHealthModifier = 0.7f;  // 70% of normal health
            aiDamageModifier = 0.7f;  // 70% of normal damage
            aiRandomMoveChance = 0.5f; // 50% chance of random move
            break;
            
        case GameDifficulty::Medium:
            aiHealthModifier = 1.0f;  // Normal health
            aiDamageModifier = 1.0f;  // Normal damage
            aiRandomMoveChance = 0.2f; // 20% chance of random move
            break;
            
        case GameDifficulty::Hard:
            aiHealthModifier = 1.3f;  // 130% of normal health
            aiDamageModifier = 1.3f;  // 130% of normal damage
            aiRandomMoveChance = 0.05f; // Only 5% chance of random move
            break;
    }
    
    // Apply health modifier to AI robot
    int newHealth = static_cast<int>(aiRobot->getMaxHealth() * aiHealthModifier);
    aiRobot->setHealth(newHealth);
}

void Game::setDifficulty(GameDifficulty diff) {
    difficulty = diff;
    applyDifficultySettings();
}

void Game::generateObstacles() {
    // Add walls (25% of grid)
    int numWalls = (gridSize * gridSize) / 4;
    for (int i = 0; i < numWalls; ++i) {
        int x = QRandomGenerator::global()->bounded(gridSize);
        int y = QRandomGenerator::global()->bounded(gridSize);
        if (arena[y][x] == CellType::Empty) {
            arena[y][x] = CellType::Wall;
            wallHealth[y][x] = INITIAL_WALL_HEALTH;
        }
    }
}

void Game::generateOpenMap() {
    // Add just a few walls in the center (10% of grid)
    int numWalls = (gridSize * gridSize) / 10;
    
    // Focus walls in the center area
    int centerX = gridSize / 2;
    int centerY = gridSize / 2;
    int radius = gridSize / 4;
    
    for (int i = 0; i < numWalls; ++i) {
        int x = centerX + QRandomGenerator::global()->bounded(radius * 2) - radius;
        int y = centerY + QRandomGenerator::global()->bounded(radius * 2) - radius;
        
        // Ensure x and y are within bounds
        x = qBound(0, x, gridSize - 1);
        y = qBound(0, y, gridSize - 1);
        
        if (arena[y][x] == CellType::Empty) {
            arena[y][x] = CellType::Wall;
            wallHealth[y][x] = INITIAL_WALL_HEALTH;
        }
    }
}

void Game::generateMazeMap() {
    // Create a maze-like structure with corridors
    
    // Start with some walls as a grid pattern
    for (int y = 0; y < gridSize; y++) {
        for (int x = 0; x < gridSize; x++) {
            if ((x % 2 == 0 && y % 2 == 0) || 
                (x % 2 == 1 && y % 2 == 1)) {
                arena[y][x] = CellType::Wall;
                wallHealth[y][x] = INITIAL_WALL_HEALTH;
            }
        }
    }
    
    // Add some random walls to make it more maze-like
    int numExtraWalls = gridSize * 2;
    for (int i = 0; i < numExtraWalls; ++i) {
        int x = QRandomGenerator::global()->bounded(gridSize);
        int y = QRandomGenerator::global()->bounded(gridSize);
        
        // Don't block the corners where robots start
        if ((x == 0 && y == gridSize - 1) || (x == gridSize - 1 && y == 0)) {
            continue;
        }
        
        if (arena[y][x] == CellType::Empty) {
            arena[y][x] = CellType::Wall;
            wallHealth[y][x] = INITIAL_WALL_HEALTH;
        }
    }
    
    // Ensure there's at least one path through the maze
    // This is a simple approach - for a real maze, you'd use a maze generation algorithm
    for (int i = 1; i < gridSize - 1; i++) {
        // Create a zigzag path
        if (i % 2 == 0) {
            arena[i][i] = CellType::Empty;
            arena[i][i+1] = CellType::Empty;
        } else {
            arena[i][i] = CellType::Empty;
            arena[i+1][i] = CellType::Empty;
        }
    }
}

void Game::generateFortressMap() {
    // Create a fortress in the center with walls around the perimeter
    
    // Calculate center and size of the fortress
    int centerX = gridSize / 2;
    int centerY = gridSize / 2;
    int fortressSize = gridSize / 3;
    
    // Create outer walls
    for (int y = 1; y < gridSize - 1; y++) {
        for (int x = 1; x < gridSize - 1; x++) {
            // Create perimeter walls
            if (x == 1 || x == gridSize - 2 || y == 1 || y == gridSize - 2) {
                arena[y][x] = CellType::Wall;
                wallHealth[y][x] = INITIAL_WALL_HEALTH;
            }
            
            // Create fortress in the center
            if (abs(x - centerX) < fortressSize/2 && abs(y - centerY) < fortressSize/2) {
                arena[y][x] = CellType::Wall;
                wallHealth[y][x] = INITIAL_WALL_HEALTH;
            }
        }
    }
    
    // Create entrances in the perimeter walls
    int entrancePos = gridSize / 2;
    arena[1][entrancePos] = CellType::Empty; // Top entrance
    arena[gridSize - 2][entrancePos] = CellType::Empty; // Bottom entrance
    arena[entrancePos][1] = CellType::Empty; // Left entrance
    arena[entrancePos][gridSize - 2] = CellType::Empty; // Right entrance
}

void Game::setPlayerRobotType(RobotType type) {
    playerRobot = std::make_unique<Robot>(type);
    playerRobot->setPosition(QPoint(0, gridSize - 1));
    
    // Connect robot signals to ensure grid updates
    connect(playerRobot.get(), &Robot::positionChanged, this, [this](const QPoint&) {
        emit turnComplete();
    });
    connect(playerRobot.get(), &Robot::directionChanged, this, [this](Direction) {
        emit turnComplete();
    });
    connect(playerRobot.get(), &Robot::healthChanged, this, [this](int) {
        emit turnComplete();
    });
    connect(playerRobot.get(), &Robot::movesChanged, this, [this](int) {
        emit turnComplete();
    });
}

void Game::setPlayer2RobotType(RobotType type) {
    player2Robot = std::make_unique<Robot>(type);
    player2Robot->setPosition(QPoint(gridSize - 1, 0));
    
    // Connect robot signals to ensure grid updates
    connect(player2Robot.get(), &Robot::positionChanged, this, [this](const QPoint&) {
        emit turnComplete();
    });
    connect(player2Robot.get(), &Robot::directionChanged, this, [this](Direction) {
        emit turnComplete();
    });
    connect(player2Robot.get(), &Robot::healthChanged, this, [this](int) {
        emit turnComplete();
    });
    connect(player2Robot.get(), &Robot::movesChanged, this, [this](int) {
        emit turnComplete();
    });
}

void Game::setAiRobotType(RobotType type) {
    aiRobot = std::make_unique<Robot>(type);
    aiRobot->setPosition(QPoint(gridSize - 1, 0));
    
    // Connect robot signals to ensure grid updates
    connect(aiRobot.get(), &Robot::positionChanged, this, [this](const QPoint&) {
        emit turnComplete();
    });
    connect(aiRobot.get(), &Robot::directionChanged, this, [this](Direction) {
        emit turnComplete();
    });
    connect(aiRobot.get(), &Robot::healthChanged, this, [this](int) {
        emit turnComplete();
    });
    connect(aiRobot.get(), &Robot::movesChanged, this, [this](int) {
        emit turnComplete();
    });
}

bool Game::attackWall(const QPoint& pos, int damage) {
    if (!isValidPosition(pos) || arena[pos.y()][pos.x()] != CellType::Wall) {
        return false;
    }

    wallHealth[pos.y()][pos.x()] -= damage;
    
    if (wallHealth[pos.y()][pos.x()] <= 0) {
        arena[pos.y()][pos.x()] = CellType::Empty;
        wallHealth[pos.y()][pos.x()] = 0;
        emit wallDestroyed(pos);
        return true;
    }
    return false;
}

int Game::getWallHealth(const QPoint& pos) const {
    if (!isValidPosition(pos) || arena[pos.y()][pos.x()] != CellType::Wall) {
        return 0;
    }
    return wallHealth[pos.y()][pos.x()];
}

void Game::placeHealthPickups() {
    // Place NUM_HEALTH_PICKUPS health pickups randomly on empty cells
    int pickupsPlaced = 0;
    
    while (pickupsPlaced < NUM_HEALTH_PICKUPS) {
        int x = QRandomGenerator::global()->bounded(gridSize);
        int y = QRandomGenerator::global()->bounded(gridSize);
        
        // Check if the cell is empty and not a robot position
        QPoint pos(x, y);
        if (arena[y][x] == CellType::Empty && 
            pos != playerRobot->getPosition() && 
            pos != (multiplayerMode ? player2Robot->getPosition() : aiRobot->getPosition())) {
            
            arena[y][x] = CellType::HealthPickup;
            pickupsPlaced++;
        }
    }
}

void Game::spawnHealthPickup(int count) {
    // Clear existing health pickups
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            if (arena[y][x] == CellType::HealthPickup) {
                arena[y][x] = CellType::Empty;
            }
        }
    }
    
    // Place specified number of health pickups
    int pickupsPlaced = 0;
    
    while (pickupsPlaced < count) {
        int x = QRandomGenerator::global()->bounded(gridSize);
        int y = QRandomGenerator::global()->bounded(gridSize);
        
        // Check if the cell is empty and not a robot position
        QPoint pos(x, y);
        if (arena[y][x] == CellType::Empty && 
            pos != playerRobot->getPosition() && 
            pos != (multiplayerMode ? player2Robot->getPosition() : aiRobot->getPosition())) {
            
            arena[y][x] = CellType::HealthPickup;
            pickupsPlaced++;
        }
    }
}

bool Game::placePowerUpAtPosition(const QPoint& pos, CellType powerUpType) {
    if (!isValidPosition(pos)) {
        return false;
    }
    
    // Check if the cell is empty or already contains a pickup
    if (arena[pos.y()][pos.x()] == CellType::Empty || 
        arena[pos.y()][pos.x()] == CellType::HealthPickup ||
        arena[pos.y()][pos.x()] == CellType::LaserPowerUp ||
        arena[pos.y()][pos.x()] == CellType::MissilePowerUp ||
        arena[pos.y()][pos.x()] == CellType::BombPowerUp) {
        
        // Check that it's not on top of any robot
        if (pos != playerRobot->getPosition() &&
            (multiplayerMode 
             ? (pos != player2Robot->getPosition())
             : (pos != aiRobot->getPosition()))) {
            
            arena[pos.y()][pos.x()] = powerUpType;
            return true;
        }
    }
    
    return false;
}

void Game::collectHealthPickup(const QPoint& pos, Robot* robot) {
    if (!isValidPosition(pos) || arena[pos.y()][pos.x()] != CellType::HealthPickup || !robot) {
        return;
    }
    
    // Add health to the robot
    int maxHealth = robot->getMaxHealth();
    
    // If this is the AI robot and we're not in multiplayer mode, apply the difficulty modifier
    if (!multiplayerMode && robot == aiRobot.get()) {
        maxHealth = static_cast<int>(maxHealth * aiHealthModifier);
    }
    
    int newHealth = std::min(robot->getHealth() + HEALTH_PICKUP_AMOUNT, maxHealth);
    robot->setHealth(newHealth);
    
    // Remove the health pickup
    arena[pos.y()][pos.x()] = CellType::Empty;
    
    // Emit signal that a health pickup was collected
    emit healthPickupCollected(pos);
}

void Game::executeCommand(Command cmd) {
    // Determine which robot is active based on the current state
    Robot* activeRobot = nullptr;
    Robot* targetRobot = nullptr;
    
    if (state == GameState::PlayerTurn) {
        activeRobot = playerRobot.get();
        targetRobot = multiplayerMode ? player2Robot.get() : aiRobot.get();
    } else if (state == GameState::Player2Turn && multiplayerMode) {
        activeRobot = player2Robot.get();
        targetRobot = playerRobot.get();
    } else {
        return; // Not a valid state for player commands
    }

    QPoint oldPos = activeRobot->getPosition();
    bool commandExecuted = false;

    switch (cmd) {
        case Command::MoveForward: {
            // Calculate new position before moving
            QPoint newPos = oldPos;
            switch (activeRobot->getDirection()) {
                case Direction::North: newPos.setY(newPos.y() - 1); break;
                case Direction::East:  newPos.setX(newPos.x() + 1); break;
                case Direction::South: newPos.setY(newPos.y() + 1); break;
                case Direction::West:  newPos.setX(newPos.x() - 1); break;
            }
            
            // Only move if the new position is valid
            if (isValidMove(newPos)) {
                activeRobot->setPosition(newPos);
                activeRobot->useMove();
                
                // Check if the robot moved onto a health pickup
                if (arena[newPos.y()][newPos.x()] == CellType::HealthPickup) {
                    collectHealthPickup(newPos, activeRobot);
                }
                // Check if robot moved to powerup tile
                else if (arena[newPos.y()][newPos.x()] == CellType::LaserPowerUp || 
                    arena[newPos.y()][newPos.x()] == CellType::MissilePowerUp ||
                    arena[newPos.y()][newPos.x()] == CellType::BombPowerUp) {
                    collectPowerUp(newPos, activeRobot, arena[newPos.y()][newPos.x()]);
                }
                
                commandExecuted = true;
            }
            break;
        }
        case Command::TurnLeft:
            activeRobot->turnLeft();
            // No move cost for turning
            commandExecuted = true;
            break;
        case Command::TurnRight:
            activeRobot->turnRight();
            // No move cost for turning
            commandExecuted = true;
            break;
        case Command::Attack: {
            RobotPowerUp powerUp = getRobotPowerUp(activeRobot);
            if (powerUp == RobotPowerUp::None) {
                // Determine attack range: Sniper has 3, others have 1
                int attackRange = (activeRobot->getType() == RobotType::Sniper) ? 3 : 1;
                QPoint startPos = activeRobot->getPosition();
                QPoint hitPos = startPos;
                bool actualHit = false;

                // Calculate the hit tile along the active robot's facing direction
                for (int step = 1; step <= attackRange; ++step) {
                    QPoint nextPos = startPos;
                    switch (activeRobot->getDirection()) {
                        case Direction::North: nextPos.setY(startPos.y() - step); break;
                        case Direction::East:  nextPos.setX(startPos.x() + step); break;
                        case Direction::South: nextPos.setY(startPos.y() + step); break;
                        case Direction::West:  nextPos.setX(startPos.x() - step); break;
                    }
                    if (!isValidPosition(nextPos))
                        break;
                    
                    // If there's a wall, mark as a hit
                    if (arena[nextPos.y()][nextPos.x()] == CellType::Wall) {
                        hitPos = nextPos;
                        actualHit = true;
                        break;
                    }
                    
                    // If there's a robot, mark as hit
                    if ((!multiplayerMode && targetRobot && nextPos == targetRobot->getPosition()) ||
                        (multiplayerMode && targetRobot && nextPos == targetRobot->getPosition())) {
                        hitPos = nextPos;
                        actualHit = true;
                        break;
                    }
                    
                    // Otherwise, continue moving
                    hitPos = nextPos;
                }
                
                // Emit the signal with the hit flag
                emit projectileFired(startPos, hitPos, activeRobot->getDirection(), actualHit, PowerUpType::Normal);
                
                // Then apply damage
                if (isValidPosition(hitPos) && arena[hitPos.y()][hitPos.x()] == CellType::Wall) {
                    int wallDamage = (activeRobot->getType() == RobotType::Tank) ? 3 :
                                        (activeRobot->getType() == RobotType::Sniper ? 2 : 1);
                    attackWall(hitPos, wallDamage);
                }
                else if (hasLineOfSight(activeRobot->getPosition(), targetRobot->getPosition()) &&
                            ((activeRobot->getDirection() == Direction::North && targetRobot->getPosition().y() < activeRobot->getPosition().y()) ||
                            (activeRobot->getDirection() == Direction::South && targetRobot->getPosition().y() > activeRobot->getPosition().y()) ||
                            (activeRobot->getDirection() == Direction::East && targetRobot->getPosition().x() > activeRobot->getPosition().x()) ||
                            (activeRobot->getDirection() == Direction::West && targetRobot->getPosition().x() < activeRobot->getPosition().x()))) {
                    attack(activeRobot, targetRobot);
                }
            }
            else {
                // if powerup is active, replace attack with it once, then remove it
                switch (powerUp) {
                case RobotPowerUp::Laser: {
                    // Laser: instant line effect, 15 damage to everything in line, no projectile
                    QPoint startPos = activeRobot->getPosition();
                    int dx = 0, dy = 0;
                    switch (activeRobot->getDirection()) {
                    case Direction::North: dy = -1; break;
                    case Direction::South: dy = 1;  break;
                    case Direction::East:  dx = 1;  break;
                    case Direction::West:  dx = -1; break;
                    }
    
                    // Step until out of bounds
                    QPoint cur = startPos;
                    while (true) {
                        cur.setX(cur.x() + dx);
                        cur.setY(cur.y() + dy);
                        if (!isValidPosition(cur)) break; // out of bounds
    
                        // If it's a wall, damage by 15
                        if (arena[cur.y()][cur.x()] == CellType::Wall) {
                            attackWall(cur, 15);
                        }
                        else {
                            // If there's a robot there (check for multiplayer or AI):
                            if (multiplayerMode) {
                                if (cur == playerRobot->getPosition()) {
                                    int newHealth = std::max(0, playerRobot->getHealth() - 15);
                                    playerRobot->setHealth(newHealth);
                                }
                                if (cur == player2Robot->getPosition()) {
                                    int newHealth = std::max(0, player2Robot->getHealth() - 15);
                                    player2Robot->setHealth(newHealth);
                                }
                            }
                            else {
                                if (cur == playerRobot->getPosition()) {
                                    int newHealth = std::max(0, playerRobot->getHealth() - 15);
                                    playerRobot->setHealth(newHealth);
                                }
                                if (cur == aiRobot->getPosition()) {
                                    // Apply AI damage mod if you want
                                    int dmg = static_cast<int>(15 * aiDamageModifier);
                                    int newHealth = std::max(0, aiRobot->getHealth() - dmg);
                                    aiRobot->setHealth(newHealth);
                                }
                            }
                        }
                    }
                    emit projectileFired(startPos, cur, activeRobot->getDirection(), true, PowerUpType::Laser);
    
                    // Clear the powerup
                    setRobotPowerUp(activeRobot, RobotPowerUp::None);
                    break;
                }
                case RobotPowerUp::Missile: {
                    // Missile: unlimited range, 20 damage, stops on first impact (wall or robot)
                    QPoint startPos = activeRobot->getPosition();
                    QPoint hitPos = startPos;
                    bool hitSomething = false;
    
                    int dx = 0, dy = 0;
                    switch (activeRobot->getDirection()) {
                    case Direction::North: dy = -1; break;
                    case Direction::South: dy = 1; break;
                    case Direction::East:  dx = 1; break;
                    case Direction::West:  dx = -1; break;
                    }
    
                    while (true) {
                        QPoint nextPos = QPoint(hitPos.x() + dx, hitPos.y() + dy);
                        if (!isValidPosition(nextPos)) {
                            // out of bounds
                            break;
                        }
                        hitPos = nextPos;
                        // if we find a wall or robot, break
                        if (arena[hitPos.y()][hitPos.x()] == CellType::Wall) {
                            // damage wall for 20
                            attackWall(hitPos, 20);
                            hitSomething = true;
                            break;
                        }
                        // Check if it's the other robot
                        if (multiplayerMode) {
                            if (hitPos == playerRobot->getPosition() || hitPos == player2Robot->getPosition()) {
                                // apply 20 damage
                                if (hitPos == playerRobot->getPosition()) {
                                    playerRobot->setHealth(std::max(0, playerRobot->getHealth() - 20));
                                } else {
                                    player2Robot->setHealth(std::max(0, player2Robot->getHealth() - 20));
                                }
                                hitSomething = true;
                                break;
                            }
                        }
                        else {
                            // single player
                            if (hitPos == playerRobot->getPosition()) {
                                playerRobot->setHealth(std::max(0, playerRobot->getHealth() - 20));
                                hitSomething = true;
                                break;
                            }
                            else if (hitPos == aiRobot->getPosition()) {
                                int dmg = static_cast<int>(20 * aiDamageModifier);
                                aiRobot->setHealth(std::max(0, aiRobot->getHealth() - dmg));
                                hitSomething = true;
                                break;
                            }
                        }
                    }
    
                    emit projectileFired(startPos, hitPos, activeRobot->getDirection(), hitSomething, PowerUpType::Missile);
    
                    // Clear the powerup
                    setRobotPowerUp(activeRobot, RobotPowerUp::None);
                    break;
                }
                case RobotPowerUp::Bomb: {
                    // Bomb: behaves like the missile (unlimited range), but upon hitting or going out of bounds
                    // it deals 30 damage in a 3x3 area around the final position (hitPos)
                    QPoint startPos = activeRobot->getPosition();
                    QPoint hitPos = startPos;
                    bool bombDetonated = false;
    
                    int dx = 0, dy = 0;
                    switch (activeRobot->getDirection()) {
                    case Direction::North: dy = -1; break;
                    case Direction::South: dy = 1; break;
                    case Direction::East:  dx = 1; break;
                    case Direction::West:  dx = -1; break;
                    }
    
                    while (true) {
                        QPoint nextPos = QPoint(hitPos.x() + dx, hitPos.y() + dy);
                        if (!isValidPosition(nextPos)) {
                            // out of bounds -> bomb detonates
                            break;
                        }
                        hitPos = nextPos;
                        // if we find a wall or robot, break
                        if (arena[hitPos.y()][hitPos.x()] == CellType::Wall) {
                            // we don't do direct damage yet, the bomb AoE will handle that
                            bombDetonated = true;
                            break;
                        }
                        if (multiplayerMode) {
                            if (hitPos == playerRobot->getPosition() || hitPos == player2Robot->getPosition()) {
                                bombDetonated = true;
                                break;
                            }
                        } else {
                            if (hitPos == playerRobot->getPosition() || hitPos == aiRobot->getPosition()) {
                                bombDetonated = true;
                                break;
                            }
                        }
                    }
    
                    // Fire a projectile for bomb
                    emit projectileFired(startPos, hitPos, activeRobot->getDirection(), bombDetonated, PowerUpType::Bomb);
    
                    // Now apply 30 damage in a 3x3 around hitPos (include the center)
                    for (int dy2 = -1; dy2 <= 1; dy2++) {
                        for (int dx2 = -1; dx2 <= 1; dx2++) {
                            QPoint areaPos(hitPos.x() + dx2, hitPos.y() + dy2);
                            if (!isValidPosition(areaPos)) continue;
    
                            // If there's a wall, damage it
                            if (arena[areaPos.y()][areaPos.x()] == CellType::Wall) {
                                attackWall(areaPos, 30);
                            }
                            else {
                                // If a robot is there, do 30
                                if (multiplayerMode) {
                                    if (areaPos == playerRobot->getPosition()) {
                                        playerRobot->setHealth(std::max(0, playerRobot->getHealth() - 30));
                                    }
                                    if (areaPos == player2Robot->getPosition()) {
                                        player2Robot->setHealth(std::max(0, player2Robot->getHealth() - 30));
                                    }
                                }
                                else {
                                    if (areaPos == playerRobot->getPosition()) {
                                        playerRobot->setHealth(std::max(0, playerRobot->getHealth() - 30));
                                    }
                                    if (areaPos == aiRobot->getPosition()) {
                                        int dmg = static_cast<int>(30 * aiDamageModifier);
                                        aiRobot->setHealth(std::max(0, aiRobot->getHealth() - dmg));
                                    }
                                }
                            }
                        }
                    }
    
                    // Clear the powerup
                    setRobotPowerUp(activeRobot, RobotPowerUp::None);
                    break;
                }
                default:
                    break;
                }
            }
            activeRobot->useMove();
            commandExecuted = true;
            break;
        }
        case Command::None:
            checkGameOver();
            break;
    }

    // Only proceed if a command was actually executed
    if (commandExecuted) {
        // Check if we need to switch turns
        if (activeRobot->getMovesLeft() <= 0) {
            checkGameOver();
            if (state != GameState::GameOver) {
                switchTurn();
            }
        }
        
        // Always emit turnComplete to update the UI
        emit turnComplete();
    }
}

void Game::executeAiTurn() {
    if (state != GameState::AiTurn) return;

    Robot* ai = aiRobot.get();
    
    if (ai->getMovesLeft() > 0) {
        // Use the RobotAI class to calculate the next move
        Command aiMove = robotAI->calculateMove(this, ai, playerRobot.get());
        QPoint oldPos = ai->getPosition();
        bool commandExecuted = false;

        switch (aiMove) {
            case Command::MoveForward: {
                // Calculate new position before moving
                QPoint newPos = oldPos;
                switch (ai->getDirection()) {
                    case Direction::North: newPos.setY(newPos.y() - 1); break;
                    case Direction::East:  newPos.setX(newPos.x() + 1); break;
                    case Direction::South: newPos.setY(newPos.y() + 1); break;
                    case Direction::West:  newPos.setX(newPos.x() - 1); break;
                }
                
                // Only move if the new position is valid
                if (isValidMove(newPos)) {
                    ai->setPosition(newPos);
                    ai->useMove();
                    
                    // Check if the AI moved onto a health pickup
                    if (arena[newPos.y()][newPos.x()] == CellType::HealthPickup) {
                        collectHealthPickup(newPos, ai);
                    }
                    // Check if AI moved to powerup tile
                    else if (arena[newPos.y()][newPos.x()] == CellType::LaserPowerUp || 
                        arena[newPos.y()][newPos.x()] == CellType::MissilePowerUp ||
                        arena[newPos.y()][newPos.x()] == CellType::BombPowerUp) {
                        collectPowerUp(newPos, ai, arena[newPos.y()][newPos.x()]);
                    }
                    
                    commandExecuted = true;
                }
                break;
            }
            case Command::TurnLeft:
                ai->turnLeft();
                // No move cost for turning
                commandExecuted = true;
                break;
            case Command::TurnRight:
                ai->turnRight();
                // No move cost for turning
                commandExecuted = true;
                break;
            case Command::Attack: {
                RobotPowerUp powerUp = getRobotPowerUp(ai);

                if (powerUp == RobotPowerUp::None) {
                    int attackRange = (ai->getType() == RobotType::Sniper) ? 3 : 1;
                    QPoint startPos = ai->getPosition();
                    QPoint hitPos = startPos;
                    bool actualHit = false;

                    // Calculate the hit tile along the AI's facing direction
                    for (int step = 1; step <= attackRange; ++step) {
                        QPoint nextPos = startPos;
                        switch (ai->getDirection()) {
                            case Direction::North: nextPos.setY(startPos.y() - step); break;
                            case Direction::East:  nextPos.setX(startPos.x() + step); break;
                            case Direction::South: nextPos.setY(startPos.y() + step); break;
                            case Direction::West:  nextPos.setX(startPos.x() - step); break;
                        }
                        if (!isValidPosition(nextPos))
                            break;
                        
                        // If there's a wall at this tile, mark it as a hit
                        if (arena[nextPos.y()][nextPos.x()] == CellType::Wall) {
                            hitPos = nextPos;
                            actualHit = true;
                            break;
                        }
                        
                        // If the player's robot is present, mark it as a hit
                        if (nextPos == playerRobot->getPosition()) {
                            hitPos = nextPos;
                            actualHit = true;
                            break;
                        }
                        
                        // Otherwise, continue
                        hitPos = nextPos;
                    }
                    
                    // Emit the projectile signal with the hit flag
                    emit projectileFired(startPos, hitPos, ai->getDirection(), actualHit, PowerUpType::Normal);
                    
                    // Then apply the damage
                    if (isValidPosition(hitPos) && arena[hitPos.y()][hitPos.x()] == CellType::Wall) {
                        int wallDamage = (ai->getType() == RobotType::Tank) ? 3 :
                                            (ai->getType() == RobotType::Sniper ? 2 : 1);
                        attackWall(hitPos, wallDamage);
                    }
                    else if (hasLineOfSight(ai->getPosition(), playerRobot->getPosition()) &&
                                ((ai->getDirection() == Direction::North && playerRobot->getPosition().y() < ai->getPosition().y()) ||
                                (ai->getDirection() == Direction::South && playerRobot->getPosition().y() > ai->getPosition().y()) ||
                                (ai->getDirection() == Direction::East  && playerRobot->getPosition().x() > ai->getPosition().x()) ||
                                (ai->getDirection() == Direction::West  && playerRobot->getPosition().x() < ai->getPosition().x()))) {
                        attack(ai, playerRobot.get());
                    }
                }
                else {
                    // Special powerup; use it ONCE, then remove it
                    switch (powerUp) {
                    case RobotPowerUp::Laser: {
                        // Laser: instant line effect, 15 damage to everything in line, no projectile
                        QPoint startPos = ai->getPosition();
                        int dx = 0, dy = 0;
                        switch (ai->getDirection()) {
                        case Direction::North: dy = -1; break;
                        case Direction::South: dy = 1;  break;
                        case Direction::East:  dx = 1;  break;
                        case Direction::West:  dx = -1; break;
                        }
            
                        // Step until out of bounds
                        QPoint cur = startPos;
                        while (true) {
                            cur.setX(cur.x() + dx);
                            cur.setY(cur.y() + dy);
                            if (!isValidPosition(cur)) break; // out of bounds
            
                            // If it's a wall, damage by 15
                            if (arena[cur.y()][cur.x()] == CellType::Wall) {
                                attackWall(cur, 15);
                            }
                            else {
                                // If there's a robot there (check for multiplayer or AI):
                                if (multiplayerMode) {
                                    if (cur == playerRobot->getPosition()) {
                                        int newHealth = std::max(0, playerRobot->getHealth() - 15);
                                        playerRobot->setHealth(newHealth);
                                    }
                                    if (cur == player2Robot->getPosition()) {
                                        int newHealth = std::max(0, player2Robot->getHealth() - 15);
                                        player2Robot->setHealth(newHealth);
                                    }
                                }
                                else {
                                    if (cur == playerRobot->getPosition()) {
                                        int newHealth = std::max(0, playerRobot->getHealth() - 15);
                                        playerRobot->setHealth(newHealth);
                                    }
                                    if (cur == aiRobot->getPosition()) {
                                        int dmg = static_cast<int>(15 * aiDamageModifier);
                                        int newHealth = std::max(0, aiRobot->getHealth() - dmg);
                                        aiRobot->setHealth(newHealth);
                                    }
                                }
                            }
                        }
                        emit projectileFired(startPos, cur, ai->getDirection(), true, PowerUpType::Laser);
            
                        // Clear the powerup
                        setRobotPowerUp(ai, RobotPowerUp::None);
                        break;
                    }
                    case RobotPowerUp::Missile: {
                        // Missile: unlimited range, 20 damage, stops on first impact (wall or robot).
                        QPoint startPos = ai->getPosition();
                        QPoint hitPos = startPos;
                        bool hitSomething = false;
            
                        int dx = 0, dy = 0;
                        switch (ai->getDirection()) {
                        case Direction::North: dy = -1; break;
                        case Direction::South: dy = 1; break;
                        case Direction::East:  dx = 1; break;
                        case Direction::West:  dx = -1; break;
                        }
            
                        while (true) {
                            QPoint nextPos = QPoint(hitPos.x() + dx, hitPos.y() + dy);
                            if (!isValidPosition(nextPos)) {
                                // out of bounds
                                break;
                            }
                            hitPos = nextPos;
                            // if we find a wall or robot, break
                            if (arena[hitPos.y()][hitPos.x()] == CellType::Wall) {
                                // damage wall for 20
                                attackWall(hitPos, 20);
                                hitSomething = true;
                                break;
                            }
                            // Check if it's the other robot
                            if (multiplayerMode) {
                                if (hitPos == playerRobot->getPosition() || hitPos == player2Robot->getPosition()) {
                                    // apply 20 damage
                                    if (hitPos == playerRobot->getPosition()) {
                                        playerRobot->setHealth(std::max(0, playerRobot->getHealth() - 20));
                                    } else {
                                        player2Robot->setHealth(std::max(0, player2Robot->getHealth() - 20));
                                    }
                                    hitSomething = true;
                                    break;
                                }
                            }
                            else {
                                // single player
                                if (hitPos == playerRobot->getPosition()) {
                                    playerRobot->setHealth(std::max(0, playerRobot->getHealth() - 20));
                                    hitSomething = true;
                                    break;
                                }
                                else if (hitPos == aiRobot->getPosition()) {
                                    int dmg = static_cast<int>(20 * aiDamageModifier);
                                    aiRobot->setHealth(std::max(0, aiRobot->getHealth() - dmg));
                                    hitSomething = true;
                                    break;
                                }
                            }
                        }
            
                        emit projectileFired(startPos, hitPos, ai->getDirection(), hitSomething, PowerUpType::Missile);
            
                        // Clear the powerup
                        setRobotPowerUp(ai, RobotPowerUp::None);
                        break;
                    }
                    case RobotPowerUp::Bomb: {
                        // Bomb: behaves like the missile (unlimited range), but upon hitting or going out of bounds
                        // it deals 30 damage in a 3x3 area around the final position (hitPos)
                        QPoint startPos = ai->getPosition();
                        QPoint hitPos = startPos;
                        bool bombDetonated = false;
            
                        int dx = 0, dy = 0;
                        switch (ai->getDirection()) {
                        case Direction::North: dy = -1; break;
                        case Direction::South: dy = 1; break;
                        case Direction::East:  dx = 1; break;
                        case Direction::West:  dx = -1; break;
                        }
            
                        while (true) {
                            QPoint nextPos = QPoint(hitPos.x() + dx, hitPos.y() + dy);
                            if (!isValidPosition(nextPos)) {
                                // out of bounds -> bomb detonates
                                break;
                            }
                            hitPos = nextPos;
                            // if we find a wall or robot, break
                            if (arena[hitPos.y()][hitPos.x()] == CellType::Wall) {
                                // no direct damage yet, the bomb AoE will handle that
                                bombDetonated = true;
                                break;
                            }
                            if (multiplayerMode) {
                                if (hitPos == playerRobot->getPosition() || hitPos == player2Robot->getPosition()) {
                                    bombDetonated = true;
                                    break;
                                }
                            } else {
                                if (hitPos == playerRobot->getPosition() || hitPos == aiRobot->getPosition()) {
                                    bombDetonated = true;
                                    break;
                                }
                            }
                        }
            
                        // Fire a projectile for bomb
                        emit projectileFired(startPos, hitPos, ai->getDirection(), bombDetonated, PowerUpType::Bomb);
            
                        // Now apply 30 damage in a 3x3 around hitPos (include the center)
                        for (int dy2 = -1; dy2 <= 1; dy2++) {
                            for (int dx2 = -1; dx2 <= 1; dx2++) {
                                QPoint areaPos(hitPos.x() + dx2, hitPos.y() + dy2);
                                if (!isValidPosition(areaPos)) continue;
            
                                // If there's a wall, damage it
                                if (arena[areaPos.y()][areaPos.x()] == CellType::Wall) {
                                    attackWall(areaPos, 30);
                                }
                                else {
                                    // If a robot is there, do 30
                                    if (multiplayerMode) {
                                        if (areaPos == playerRobot->getPosition()) {
                                            playerRobot->setHealth(std::max(0, playerRobot->getHealth() - 30));
                                        }
                                        if (areaPos == player2Robot->getPosition()) {
                                            player2Robot->setHealth(std::max(0, player2Robot->getHealth() - 30));
                                        }
                                    }
                                    else {
                                        if (areaPos == playerRobot->getPosition()) {
                                            playerRobot->setHealth(std::max(0, playerRobot->getHealth() - 30));
                                        }
                                        if (areaPos == aiRobot->getPosition()) {
                                            int dmg = static_cast<int>(30 * aiDamageModifier);
                                            aiRobot->setHealth(std::max(0, aiRobot->getHealth() - dmg));
                                        }
                                    }
                                }
                            }
                        }
            
                        // Clear the powerup
                        setRobotPowerUp(ai, RobotPowerUp::None);
                        break;
                    }
                    default:
                        break;
                    }
                }
                ai->useMove();
                commandExecuted = true;
                break;
            }
            case Command::None:
                // Do nothing for None command
                break;
        }

        if (commandExecuted) {
            if (ai->getMovesLeft() <= 0) {
                checkGameOver();
                if (state != GameState::GameOver) {
                    switchTurn();
                }
            }
            emit turnComplete();
        }
    }
}

bool Game::isValidPosition(const QPoint& pos) const {
    return pos.x() >= 0 && pos.x() < gridSize && 
           pos.y() >= 0 && pos.y() < gridSize;
}

bool Game::isValidMove(const QPoint& pos) const {
    if (!isValidPosition(pos)) return false;
    
    // Check if the cell is a wall
    CellType cellType = arena[pos.y()][pos.x()];
    if (cellType == CellType::Wall) return false;
    
    // Check if the cell is occupied by another robot
    if (multiplayerMode) {
        if (playerRobot && player2Robot) {
            if (pos == playerRobot->getPosition() || pos == player2Robot->getPosition()) {
                return false;
            }
        }
    } else {
        if (playerRobot && aiRobot) {
            if (pos == playerRobot->getPosition() || pos == aiRobot->getPosition()) {
                return false;
            }
        }
    }
    
    return true;
}

CellType Game::getCellType(const QPoint& pos) const {
    if (!isValidPosition(pos)) return CellType::Wall;
    return arena[pos.y()][pos.x()];
}

void Game::checkGameOver() {
    if (multiplayerMode) {
        if (playerRobot->isDead() || player2Robot->isDead()) {
            state = GameState::GameOver;
            emit gameStateChanged(state);
        }
    } else {
        if (playerRobot->isDead() || aiRobot->isDead()) {
            state = GameState::GameOver;
            emit gameStateChanged(state);
        }
    }
}

void Game::switchTurn() {
    if (multiplayerMode) {
        // In multiplayer mode, switch between Player 1 and Player 2
        if (state == GameState::PlayerTurn) {
            state = GameState::Player2Turn;
            player2Robot->resetMoves();
        } else {
            state = GameState::PlayerTurn;
            playerRobot->resetMoves();
        }
    } else {
        // In single player mode, switch between Player and AI
        if (state == GameState::PlayerTurn) {
            state = GameState::AiTurn;
            aiRobot->resetMoves();
        } else {
            state = GameState::PlayerTurn;
            playerRobot->resetMoves();
        }
    }
    emit gameStateChanged(state);
}

bool Game::hasLineOfSight(const QPoint& from, const QPoint& to) const {
    // If not in same row or column, no line of sight
    if (from.x() != to.x() && from.y() != to.y()) {
        return false;
    }
    
    // Check for obstacles between points
    if (from.x() == to.x()) {
        // Vertical line
        int startY = std::min(from.y(), to.y());
        int endY = std::max(from.y(), to.y());
        for (int y = startY + 1; y < endY; ++y) {
            if (arena[y][from.x()] == CellType::Wall) {
                return false;
            }
        }
    } else {
        // Horizontal line
        int startX = std::min(from.x(), to.x());
        int endX = std::max(from.x(), to.x());
        for (int x = startX + 1; x < endX; ++x) {
            if (arena[to.y()][x] == CellType::Wall) {
                return false;
            }
        }
    }
    return true;
}

bool Game::attack(Robot* attacker, Robot* target) {
    if (!attacker || !target) return false;
    
    // Get positions
    QPoint attackerPos = attacker->getPosition();
    QPoint targetPos = target->getPosition();
    
    // Calculate Manhattan distance
    int dx = targetPos.x() - attackerPos.x();
    int dy = targetPos.y() - attackerPos.y();
    int distance = abs(dx) + abs(dy);
    
    // Check if target is in range based on robot type
    int maxRange = (attacker->getType() == RobotType::Sniper) ? 3 : 1;
    
    // Check if in range and has line of sight
    if (distance <= maxRange && hasLineOfSight(attackerPos, targetPos)) {
        int damage = attacker->getAttackDamage();
        
        // Apply damage modifier if AI is attacking
        if (attacker == aiRobot.get()) {
            damage = static_cast<int>(damage * aiDamageModifier);
        }
        
        int newHealth = std::max(0, target->getHealth() - damage);
        target->setHealth(newHealth);
        return true;
    }
    
    return false;
}

void Game::setRobotPowerUp(Robot* robot, RobotPowerUp powerUp) {
    if (!robot) return;
    robot->setPowerUp(powerUp);
}

RobotPowerUp Game::getRobotPowerUp(const Robot* robot) const {
    if (!robot) return RobotPowerUp::None;
    return robot->getPowerUp();
}

void Game::placeSpecialPickups() {
    for (int i = 0; i < NUM_LASER_POWERUPS; ++i) {
        placeSinglePowerUp(CellType::LaserPowerUp);
    }
    for (int i = 0; i < NUM_MISSILE_POWERUPS; ++i) {
        placeSinglePowerUp(CellType::MissilePowerUp);
    }
    for (int i = 0; i < NUM_BOMB_POWERUPS; ++i) {
        placeSinglePowerUp(CellType::BombPowerUp);
    }
}

bool Game::placeSinglePowerUp(CellType powerUpType) {
    while (true) {
        int x = QRandomGenerator::global()->bounded(gridSize);
        int y = QRandomGenerator::global()->bounded(gridSize);
        QPoint pos(x, y);

        // Must be empty and not on top of any robot
        if (arena[y][x] == CellType::Empty &&
            pos != playerRobot->getPosition() &&
            (multiplayerMode
                ? (pos != player2Robot->getPosition())
                : (pos != aiRobot->getPosition())))
        {
            arena[y][x] = powerUpType;
            return true;
        }
    }
    return false;
}

void Game::collectPowerUp(const QPoint& pos, Robot* robot, CellType cellType) {
    if (!isValidPosition(pos) || !robot) return;

    // Overwrite any existing powerup with the newly collected one
    switch(cellType) {
    case CellType::LaserPowerUp:
        setRobotPowerUp(robot, RobotPowerUp::Laser);
        break;
    case CellType::MissilePowerUp:
        setRobotPowerUp(robot, RobotPowerUp::Missile);
        break;
    case CellType::BombPowerUp:
        setRobotPowerUp(robot, RobotPowerUp::Bomb);
        break;
    default:
        return;
    }

    // Remove the powerup from the arena
    arena[pos.y()][pos.x()] = CellType::Empty;
}