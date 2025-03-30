#include "gamegrid.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>
#include "robotselector.h"
#include <QGraphicsPixmapItem>
#include "projectile.h"
#include "hitfeedback.h"
#include "laserfeedback.h"
#include <QGraphicsColorizeEffect>

GameGrid::GameGrid(QWidget *parent) : QWidget(parent) {
    // Create a horizontal layout for game and info panels
    mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create game panel (left side)
    gamePanel = new QWidget(this);
    QVBoxLayout* gameLayout = new QVBoxLayout(gamePanel);
    gameLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create info panel (right side)
    infoPanel = new QWidget(this);
    infoPanel->setFixedWidth(INFO_PANEL_WIDTH);
    infoPanel->setStyleSheet("background-color: #2c3e50; color: white;");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoPanel);
    
    // Create graphics scene and view
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFixedSize(CELL_SIZE * GRID_SIZE + 2, CELL_SIZE * GRID_SIZE + 14);
    view->setRenderHint(QPainter::Antialiasing);
    view->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
    view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    view->setFocusPolicy(Qt::NoFocus); // Prevent view from taking focus
    gameLayout->addWidget(view, 0, Qt::AlignCenter);

    // Hit feedback
    feedbackGroup = new QGraphicsItemGroup();
    feedbackGroup->setZValue(1000);
    scene->addItem(feedbackGroup);

    
    // Create title label
    titleLabel = new QLabel("Robot Arena", infoPanel);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin-bottom: 20px; color: #ecf0f1;");
    
    // Create status label
    statusLabel = new QLabel("Select your robot!", infoPanel);
    statusLabel->setWordWrap(true);
    statusLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0; color: #ecf0f1;");
    
    // Create map info label
    mapInfoLabel = new QLabel("Map: Random", infoPanel);
    mapInfoLabel->setStyleSheet("font-size: 16px; margin: 5px 0; color: #ecf0f1;");
    
    // Create controls label
    controlsLabel = new QLabel("Player 1: W:Forward A:Left D:Right Space:Attack", infoPanel);
    controlsLabel->setWordWrap(true);
    controlsLabel->setStyleSheet("font-size: 16px; margin: 10px 0; color: #ecf0f1;");
    
    // Create info message label (for tutorial)
    infoMessageLabel = new QLabel(infoPanel);
    infoMessageLabel->setWordWrap(true);
    infoMessageLabel->setTextFormat(Qt::RichText);
    infoMessageLabel->setStyleSheet("font-size: 16px; background-color: #34495e; padding: 15px; border-radius: 5px; color: #ecf0f1;");
    infoMessageLabel->setMinimumHeight(300);
    infoMessageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    // Add widgets to info layout
    infoLayout->addWidget(titleLabel);
    infoLayout->addWidget(statusLabel);
    infoLayout->addWidget(mapInfoLabel);
    infoLayout->addWidget(controlsLabel);
    infoLayout->addWidget(infoMessageLabel, 1);
    infoLayout->addStretch();
    
    // Add panels to main layout
    mainLayout->addWidget(gamePanel, 1);
    mainLayout->addWidget(infoPanel, 0);
    
    // Create game instance with the new grid size
    game = std::make_unique<Game>(GRID_SIZE);
    
    // Connect signals
    connect(game.get(), &Game::turnComplete, this, &GameGrid::handleTurnComplete);
    connect(game.get(), &Game::gameStateChanged, this, &GameGrid::handleGameStateChanged);
    connect(game.get(), &Game::arenaInitialized, this, &GameGrid::updateGrid);
    connect(game.get(), &Game::projectileFired, this, &GameGrid::spawnProjectile);
    connect(game.get(), &Game::healthPickupCollected, this, [this](const QPoint& pos) {
        Q_UNUSED(pos); // Parameter provided by signal but not needed in handler
        // Update the status label to show health pickup collected
        QString currentStatus = statusLabel->text();
        statusLabel->setText(currentStatus + "\nHealth pickup collected! +20 HP");
        
        // Schedule to reset the status message after 3 seconds
        QTimer::singleShot(3000, this, &GameGrid::updateStatusLabel);
        
        // Update the grid to reflect the change
        updateGrid();
    });
    
    // Initial update
    updateGrid();
    
    // Set strong focus policy for the widget
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    
    // Set minimum size for widescreen layout
    setMinimumSize(1280, 720);
}

void GameGrid::setInfoPanelMessage(const QString& message) {
    infoMessageLabel->setText(message);
}

void GameGrid::initializeWithRobotType(RobotType playerType, RobotType aiType, 
                                      GameDifficulty difficulty, MapType mapType) {
    // Update map info label
    QString mapName;
    switch (mapType) {
        case MapType::Random:
            mapName = "Random";
            break;
        case MapType::Open:
            mapName = "Open Arena";
            break;
        case MapType::Maze:
            mapName = "Maze";
            break;
        case MapType::Fortress:
            mapName = "Fortress";
            break;
    }
    mapInfoLabel->setText("Map: " + mapName);
    
    // Update controls label for single player
    controlsLabel->setText("W: Move Forward\nA: Turn Left\nD: Turn Right\nSpace: Attack");
    
    // Initialize arena with selected robots, difficulty, and map
    game->initializeArena(playerType, aiType, difficulty, mapType);
    
    // Connect to the robots' moves changed signals
    connect(game->getPlayerRobot(), &Robot::movesChanged, this, [this](int) {
        updateStatusLabel();
    });
    connect(game->getAiRobot(), &Robot::movesChanged, this, [this](int) {
        updateStatusLabel();
    });
}

void GameGrid::initializeMultiplayer(RobotType player1Type, RobotType player2Type, MapType mapType) {
    // Update map info label
    QString mapName;
    switch (mapType) {
        case MapType::Random:
            mapName = "Random";
            break;
        case MapType::Open:
            mapName = "Open Arena";
            break;
        case MapType::Maze:
            mapName = "Maze";
            break;
        case MapType::Fortress:
            mapName = "Fortress";
            break;
    }
    mapInfoLabel->setText("Map: " + mapName);
    
    // Update controls label for multiplayer
    controlsLabel->setText("Player 1:\nW: Forward, A: Left, D: Right, Space: Attack\n\nPlayer 2:\nArrow Keys + Enter");
    
    // Initialize arena with selected robots for multiplayer
    game->initializeMultiplayerArena(player1Type, player2Type, mapType);
    
    // Connect to the robots' moves changed signals
    connect(game->getPlayerRobot(), &Robot::movesChanged, this, [this](int) {
        updateStatusLabel();
    });
    connect(game->getPlayer2Robot(), &Robot::movesChanged, this, [this](int) {
        updateStatusLabel();
    });
}

void GameGrid::initializeGrid() {
    // Set up the graphics view
    scene->setSceneRect(0, 0, CELL_SIZE * GRID_SIZE, CELL_SIZE * GRID_SIZE);
    view->setScene(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setBackgroundBrush(QBrush(Qt::white));
}

void GameGrid::initializeControls() {
    statusLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    controlsLabel->setStyleSheet("font-size: 14px;");
}

void GameGrid::keyPressEvent(QKeyEvent* event) {
    // Emit key press signal for tutorial
    emit keyPressed(event->key());
    
    if (game->getState() == GameState::GameOver && event->key() == Qt::Key_R) {
        // Reset game with a random robot type
        RobotType playerType = static_cast<RobotType>(QRandomGenerator::global()->bounded(3)); // Changed from 4 to 3 since we removed Balanced
        RobotType aiType = static_cast<RobotType>(QRandomGenerator::global()->bounded(3));
        
        // Initialize arena with selected robots
        game->initializeArena(playerType, aiType);
        setFocus(); // Maintain focus
        return;
    }
    
    // Handle Player 1 controls (when it's Player 1's turn)
    if (game->getState() == GameState::PlayerTurn) {
        switch (event->key()) {
            case Qt::Key_W:
                game->executeCommand(Command::MoveForward);
                break;
            case Qt::Key_A:
                game->executeCommand(Command::TurnLeft);
                break;
            case Qt::Key_D:
                game->executeCommand(Command::TurnRight);
                break;
            case Qt::Key_Space:
                game->executeCommand(Command::Attack);
                break;
        }
        setFocus(); // Maintain focus after command execution
    }
    
    // Handle Player 2 controls (when it's Player 2's turn in multiplayer mode)
    if (game->isMultiplayerMode() && game->getState() == GameState::Player2Turn) {
        switch (event->key()) {
            case Qt::Key_Up:
                game->executeCommand(Command::MoveForward);
                break;
            case Qt::Key_Left:
                game->executeCommand(Command::TurnLeft);
                break;
            case Qt::Key_Right:
                game->executeCommand(Command::TurnRight);
                break;
            case Qt::Key_Return:
            case Qt::Key_Enter:
                game->executeCommand(Command::Attack);
                break;
        }
        setFocus(); // Maintain focus after command execution
    }
    
    // Always accept the event to prevent it from being propagated
    event->accept();
}

void GameGrid::updateGrid() {
    // Remove all items except those in the feedback group.
    QList<QGraphicsItem*> allItems = scene->items();
    for (QGraphicsItem* item : allItems) {
        // Check if the item is not the feedbackGroup and not a child of it.
        if (item != feedbackGroup && !feedbackGroup->childItems().contains(item)) {
            scene->removeItem(item);
            delete item;
        }
    }

    int size = game->getGridSize();
    
    // Draw the grid and terrain
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            QPoint pos(col, row);
            CellType cellType = game->getCellType(pos);
            drawCell(col, row, cellType);
        }
    }
    
    // Draw robots
    Robot* player = game->getPlayerRobot();
    if (player) {
        drawRobot(player, true);
    }
    
    if (game->isMultiplayerMode()) {
        Robot* player2 = game->getPlayer2Robot();
        if (player2) {
            drawRobot(player2, true);
        }
    } else {
        Robot* ai = game->getAiRobot();
        if (ai) {
            drawRobot(ai, false);
        }
    }
    
    // Update status label
    updateStatusLabel();
    
    // Ensure the widget maintains focus
    setFocus();
}

void GameGrid::drawCell(int x, int y, CellType cellType) {
    // Base tile is always white
    QColor color = QColor(255, 255, 255);
    
    // Create the base cell
    scene->addRect(
        x * CELL_SIZE, y * CELL_SIZE, 
        CELL_SIZE, CELL_SIZE, 
        QPen(Qt::black), 
        QBrush(color)
    );
    
    // If it's a wall, add darker background based on damage
    if (cellType == CellType::Wall) {
        // Get wall health and calculate color
        int health = game->getWallHealth(QPoint(x, y));
        int maxHealth = Game::INITIAL_WALL_HEALTH;
        
        // Calculate grey value from light (220) to dark (64) based on damage
        // Full health = light grey (220)
        // No health = dark grey (64)
        int greyValue = 220 - ((220 - 64) * (maxHealth - health) / maxHealth);
        color = QColor(greyValue, greyValue, greyValue);
        
        // Add the colored background for the wall
        scene->addRect(
            x * CELL_SIZE, y * CELL_SIZE, 
            CELL_SIZE, CELL_SIZE, 
            QPen(Qt::black), 
            QBrush(color)
        );
        
        // Load and draw the rock icon
        QPixmap rockIcon(":/sprites/Sprite/Icons/rock.jpg");
        if (!rockIcon.isNull()) {
            rockIcon = rockIcon.scaled(CELL_SIZE * 0.8, CELL_SIZE * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QGraphicsPixmapItem* rockItem = scene->addPixmap(rockIcon);
            rockItem->setPos(
                x * CELL_SIZE + (CELL_SIZE - rockIcon.width()) / 2,
                y * CELL_SIZE + (CELL_SIZE - rockIcon.height()) / 2
            );
        }
    }
    // If it's a health pickup, add the heal icon
    else if (cellType == CellType::HealthPickup) {
        // Add a light green background for the health pickup
        color = QColor(200, 255, 200); // Light green
        scene->addRect(
            x * CELL_SIZE, y * CELL_SIZE, 
            CELL_SIZE, CELL_SIZE, 
            QPen(Qt::black), 
            QBrush(color)
        );
        
        // Load and draw the heal icon
        QPixmap healIcon(":/sprites/Sprite/Icons/Heal.png");
        if (!healIcon.isNull()) {
            healIcon = healIcon.scaled(CELL_SIZE * 0.8, CELL_SIZE * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QGraphicsPixmapItem* healItem = scene->addPixmap(healIcon);
            healItem->setPos(
                x * CELL_SIZE + (CELL_SIZE - healIcon.width()) / 2,
                y * CELL_SIZE + (CELL_SIZE - healIcon.height()) / 2
            );
        }
    }
    else if (cellType == CellType::LaserPowerUp) {
        // Base gray background for the entire cell with border
        color = QColor(100, 100, 100);
        // Draw the full cell with a border
        scene->addRect(
            x * CELL_SIZE, y * CELL_SIZE,
            CELL_SIZE, CELL_SIZE,
            QPen(Qt::black),
            QBrush(color)
        );
        
        // Define a margin so the pulsating effect doesn't cover the border
        const int margin = 1;
        // Create a smaller rectangle inset within the cell
        QGraphicsRectItem* powerupRect = scene->addRect(
            x * CELL_SIZE + margin, y * CELL_SIZE + margin,
            CELL_SIZE - 1 * margin, CELL_SIZE - 1 * margin,
            QPen(Qt::NoPen),  // No border for the effect rect
            QBrush(color)
        );
        
        // Create and set up a colorize effect to tint with white
        QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect();
        effect->setColor(Qt::white);
        powerupRect->setGraphicsEffect(effect);
        
        // Animate the effect's strength property to pulse
        QPropertyAnimation* animation = new QPropertyAnimation(effect, "strength");
        animation->setDuration(1000); // 1 second cycle duration
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setLoopCount(-1); // Loop indefinitely
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
        
        // Load and draw the laser icon on top of the effects
        QPixmap laserIcon(":/sprites/Sprite/Icons/laser.png");
        if (!laserIcon.isNull()) {
            laserIcon = laserIcon.scaled(CELL_SIZE * 0.8, CELL_SIZE * 0.8, Qt::KeepAspectRatio);
            QGraphicsPixmapItem* laserItem = scene->addPixmap(laserIcon);
            laserItem->setPos(
                x * CELL_SIZE + (CELL_SIZE - laserIcon.width()) / 2,
                y * CELL_SIZE + (CELL_SIZE - laserIcon.height()) / 2
            );
        }
    }
    else if (cellType == CellType::MissilePowerUp) {
        // Base gray background for the entire cell with border
        color = QColor(100, 100, 100);
        // Draw the full cell with a border
        scene->addRect(
            x * CELL_SIZE, y * CELL_SIZE,
            CELL_SIZE, CELL_SIZE,
            QPen(Qt::black),
            QBrush(color)
        );
        
        // Define a margin so the pulsating effect doesn't cover the border
        const int margin = 1;
        // Create a smaller rectangle inset within the cell
        QGraphicsRectItem* powerupRect = scene->addRect(
            x * CELL_SIZE + margin, y * CELL_SIZE + margin,
            CELL_SIZE - 1 * margin, CELL_SIZE - 1 * margin,
            QPen(Qt::NoPen),  // No border for the effect rect
            QBrush(color)
        );
        
        // Create and set up a colorize effect to tint with white
        QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect();
        effect->setColor(Qt::white);
        powerupRect->setGraphicsEffect(effect);
        
        // Animate the effect's strength property to pulse
        QPropertyAnimation* animation = new QPropertyAnimation(effect, "strength");
        animation->setDuration(1000); // 1 second cycle duration
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setLoopCount(-1); // Loop indefinitely
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
        
        QPixmap missileIcon(":/sprites/Sprite/Icons/missile.png");
        if (!missileIcon.isNull()) {
            missileIcon = missileIcon.scaled(CELL_SIZE * 0.8, CELL_SIZE * 0.8);
            QGraphicsPixmapItem* missileItem = scene->addPixmap(missileIcon);
            missileItem->setPos(
                x * CELL_SIZE + (CELL_SIZE - missileIcon.width()) / 2,
                y * CELL_SIZE + (CELL_SIZE - missileIcon.height()) / 2
            );
        }
    }
    else if (cellType == CellType::BombPowerUp) {
        // Base gray background for the entire cell with border
        color = QColor(100, 100, 100);
        // Draw the full cell with a border
        scene->addRect(
            x * CELL_SIZE, y * CELL_SIZE,
            CELL_SIZE, CELL_SIZE,
            QPen(Qt::black),
            QBrush(color)
        );
        
        // Define a margin so the pulsating effect doesn't cover the border
        const int margin = 1;
        // Create a smaller rectangle inset within the cell
        QGraphicsRectItem* powerupRect = scene->addRect(
            x * CELL_SIZE + margin, y * CELL_SIZE + margin,
            CELL_SIZE - 1 * margin, CELL_SIZE - 1 * margin,
            QPen(Qt::NoPen),  // No border for the effect rect
            QBrush(color)
        );
        
        // Create and set up a colorize effect to tint with white
        QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect();
        effect->setColor(Qt::white);
        powerupRect->setGraphicsEffect(effect);
        
        // Animate the effect's strength property to pulse
        QPropertyAnimation* animation = new QPropertyAnimation(effect, "strength");
        animation->setDuration(1000); // 1 second cycle duration
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setLoopCount(-1); // Loop indefinitely
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        animation->start(QAbstractAnimation::DeleteWhenStopped);

        QPixmap bombIcon(":/sprites/Sprite/Icons/bomb.png");
        if (!bombIcon.isNull()) {
            bombIcon = bombIcon.scaled(CELL_SIZE * 0.8, CELL_SIZE * 0.8);
            QGraphicsPixmapItem* bombItem = scene->addPixmap(bombIcon);
            bombItem->setPos(
                x * CELL_SIZE + (CELL_SIZE - bombIcon.width()) / 2,
                y * CELL_SIZE + (CELL_SIZE - bombIcon.height()) / 2
            );
        }
    }
}

void GameGrid::drawRobot(Robot* robot, bool isPlayer) {
    Q_UNUSED(isPlayer); // Parameter kept for API consistency
    if (!robot) return;
    
    QPoint pos = robot->getPosition();
    int x = pos.x() * CELL_SIZE;
    int y = pos.y() * CELL_SIZE;
    
    // Create a QGraphicsPixmapItem with the robot's sprite
    QPixmap sprite = robot->getTopViewSprite();
    sprite = sprite.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    QGraphicsPixmapItem* robotItem = scene->addPixmap(sprite);
    robotItem->setPos(x, y);
    
    // Draw health bar
    float healthPercent = static_cast<float>(robot->getHealth()) / robot->getMaxHealth();
    QColor healthColor = QColor::fromHsvF(healthPercent * 0.3, 1.0, 1.0); // Red to green
    
    // Health bar background
    scene->addRect(
        x, y - 10, 
        CELL_SIZE, 5,
        QPen(Qt::black),
        QBrush(Qt::lightGray)
    );
    
    // Health bar fill
    scene->addRect(
        x, y - 10,
        CELL_SIZE * healthPercent, 5,
        QPen(Qt::transparent),
        QBrush(healthColor)
    );
}

void GameGrid::handleTurnComplete() {
    updateGrid(); // Make sure grid updates after each turn
    if (game->getState() == GameState::AiTurn) {
        // Add a short delay before AI's next move
        QTimer::singleShot(500, [this]() {
            if (game->getState() == GameState::AiTurn) {
                game->executeAiTurn();
                setFocus(); // Maintain focus after AI turn
            }
        });
    } else {
        // For player turns, ensure focus is maintained
        setFocus();
    }
}

void GameGrid::handleGameStateChanged(GameState state) {
    Robot* player1 = game->getPlayerRobot();
    
    if (game->isMultiplayerMode()) {
        Robot* player2 = game->getPlayer2Robot();
        
        switch (state) {
            case GameState::PlayerTurn:
                statusLabel->setText(QString("Player 1's Turn\nHealth: %1\nMoves: %2/%3").arg(
                    player1->getHealth()).arg(player1->getMovesLeft()).arg(player1->getMaxMoves()));
                break;
            case GameState::Player2Turn:
                statusLabel->setText(QString("Player 2's Turn\nHealth: %1\nMoves: %2/%3").arg(
                    player2->getHealth()).arg(player2->getMovesLeft()).arg(player2->getMaxMoves()));
                break;
            case GameState::AiTurn:
                // Not used in multiplayer mode
                break;
            case GameState::GameOver:
                bool player1Won = !player1->isDead();
                statusLabel->setText(QString("Game Over - %1 Wins!").arg(player1Won ? "Player 1" : "Player 2"));
                
                // Emit game over signal after a short delay to allow final state to be shown
                QTimer::singleShot(1500, [this, player1Won]() {
                    emit gameOver(player1Won);
                });
                break;
        }
    } else {
        Robot* ai = game->getAiRobot();
        
        switch (state) {
            case GameState::PlayerTurn:
                statusLabel->setText(QString("Player's Turn\nHealth: %1\nMoves: %2/%3").arg(
                    player1->getHealth()).arg(player1->getMovesLeft()).arg(player1->getMaxMoves()));
                break;
            case GameState::AiTurn:
                statusLabel->setText(QString("AI's Turn\nHealth: %1\nMoves: %2/%3").arg(
                    ai->getHealth()).arg(ai->getMovesLeft()).arg(ai->getMaxMoves()));
                break;
            case GameState::Player2Turn:
                // Not used in single player mode
                break;
            case GameState::GameOver:
                bool playerWon = !player1->isDead();
                statusLabel->setText(QString("Game Over - %1 Wins!").arg(playerWon ? "Player" : "AI"));
                
                // Emit game over signal after a short delay to allow final state to be shown
                QTimer::singleShot(1500, [this, playerWon]() {
                    emit gameOver(playerWon);
                });
                break;
        }
    }
}

void GameGrid::updateStatusLabel() {
    if (game->isMultiplayerMode()) {
        Robot* currentRobot = (game->getState() == GameState::PlayerTurn) ? 
                              game->getPlayerRobot() : game->getPlayer2Robot();
        QString turnStr = (game->getState() == GameState::PlayerTurn) ? "Player 1's" : "Player 2's";
        
        statusLabel->setText(QString("%1 Turn\nHealth: %2\nMoves: %3/%4").arg(
            turnStr).arg(currentRobot->getHealth()).arg(currentRobot->getMovesLeft()).arg(currentRobot->getMaxMoves()));
    } else {
        Robot* currentRobot = (game->getState() == GameState::PlayerTurn) ? 
                              game->getPlayerRobot() : game->getAiRobot();
        QString turnStr = (game->getState() == GameState::PlayerTurn) ? "Player's" : "AI's";
        
        statusLabel->setText(QString("%1 Turn\nHealth: %2\nMoves: %3/%4").arg(
            turnStr).arg(currentRobot->getHealth()).arg(currentRobot->getMovesLeft()).arg(currentRobot->getMaxMoves()));
    }
}

void GameGrid::spawnProjectile(const QPoint &start, const QPoint &end, Direction direction, bool actualHit, PowerUpType powerUpUsed) {
    // Convert grid coordinates to scene coordinates using cell centers.
    QPointF sceneStart(start.x() * CELL_SIZE + CELL_SIZE / 2.0,
                         start.y() * CELL_SIZE + CELL_SIZE / 2.0);
    QPointF sceneEnd(end.x() * CELL_SIZE + CELL_SIZE / 2.0,
                       end.y() * CELL_SIZE + CELL_SIZE / 2.0);
    
    ProjectileType projType = ProjectileType::Normal;

    if (powerUpUsed == PowerUpType::Laser) {
        LaserFeedback *lf = new LaserFeedback(sceneStart, sceneEnd);
        feedbackGroup->addToGroup(lf); 
        return;
    }
    if (powerUpUsed == PowerUpType::Missile) {
        projType = ProjectileType::Missile;
    }
    else if (powerUpUsed == PowerUpType::Bomb) {
        projType = ProjectileType::Bomb;
    }
    // Create the projectile and add it to the scene.
    Projectile *proj = new Projectile(sceneStart, sceneEnd, direction, CELL_SIZE, projType);
    feedbackGroup->addToGroup(proj);
    
    // Only connect to spawn hit feedback if the attack hit something.
    if (actualHit) {
        connect(proj, &Projectile::hitReached, this, [this, powerUpUsed](const QPointF &hitPos) {
            int effectSize = (powerUpUsed == PowerUpType::Bomb) ? (CELL_SIZE * 3) : (CELL_SIZE);
            HitFeedback *feedback = new HitFeedback(hitPos, effectSize);
            feedbackGroup->addToGroup(feedback);
        });
    }
    
    proj->startAnimation();
}
