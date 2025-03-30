#include "multiplayerrobotselector.h"
#include <QPixmap>

MultiplayerRobotSelector::MultiplayerRobotSelector(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Select Robots for Multiplayer");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create title label
    QLabel* titleLabel = new QLabel("Select Robots for Multiplayer", this);
    QFont titleFont("Arial", 18, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Create player selection layout
    QHBoxLayout* playerSelectionLayout = new QHBoxLayout();
    
    // Player 1 selection
    QVBoxLayout* player1Layout = new QVBoxLayout();
    QLabel* player1Label = new QLabel("Player 1", this);
    player1Label->setFont(QFont("Arial", 14, QFont::Bold));
    player1Label->setAlignment(Qt::AlignCenter);
    player1Layout->addWidget(player1Label);
    
    player1Group = new QGroupBox("Robot Type");
    QVBoxLayout* p1GroupLayout = new QVBoxLayout();
    
    p1ScoutBtn = new QRadioButton("Scout", this);
    p1TankBtn = new QRadioButton("Tank", this);
    p1SniperBtn = new QRadioButton("Sniper", this);
    p1ScoutBtn->setChecked(true);
    
    p1GroupLayout->addWidget(p1ScoutBtn);
    p1GroupLayout->addWidget(p1TankBtn);
    p1GroupLayout->addWidget(p1SniperBtn);
    player1Group->setLayout(p1GroupLayout);
    player1Layout->addWidget(player1Group);
    
    p1PreviewLabel = createRobotPreview(RobotType::Scout);
    player1Layout->addWidget(p1PreviewLabel);
    
    p1DescriptionLabel = new QLabel(getRobotDescription(RobotType::Scout), this);
    p1DescriptionLabel->setWordWrap(true);
    p1DescriptionLabel->setStyleSheet("padding: 10px; background-color: #f0f0f0; border-radius: 5px;");
    player1Layout->addWidget(p1DescriptionLabel);
    
    // Player 2 selection
    QVBoxLayout* player2Layout = new QVBoxLayout();
    QLabel* player2Label = new QLabel("Player 2", this);
    player2Label->setFont(QFont("Arial", 14, QFont::Bold));
    player2Label->setAlignment(Qt::AlignCenter);
    player2Layout->addWidget(player2Label);
    
    player2Group = new QGroupBox("Robot Type");
    QVBoxLayout* p2GroupLayout = new QVBoxLayout();
    
    p2ScoutBtn = new QRadioButton("Scout", this);
    p2TankBtn = new QRadioButton("Tank", this);
    p2SniperBtn = new QRadioButton("Sniper", this);
    p2TankBtn->setChecked(true); // Default to a different robot for player 2
    
    p2GroupLayout->addWidget(p2ScoutBtn);
    p2GroupLayout->addWidget(p2TankBtn);
    p2GroupLayout->addWidget(p2SniperBtn);
    player2Group->setLayout(p2GroupLayout);
    player2Layout->addWidget(player2Group);
    
    p2PreviewLabel = createRobotPreview(RobotType::Tank);
    player2Layout->addWidget(p2PreviewLabel);
    
    p2DescriptionLabel = new QLabel(getRobotDescription(RobotType::Tank), this);
    p2DescriptionLabel->setWordWrap(true);
    p2DescriptionLabel->setStyleSheet("padding: 10px; background-color: #f0f0f0; border-radius: 5px;");
    player2Layout->addWidget(p2DescriptionLabel);
    
    // Add player layouts to selection layout
    playerSelectionLayout->addLayout(player1Layout);
    playerSelectionLayout->addLayout(player2Layout);
    mainLayout->addLayout(playerSelectionLayout);
    
    // Create buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    backButton = new QPushButton("Back", this);
    selectButton = new QPushButton("Select", this);
    
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(selectButton);
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(p1ScoutBtn, &QRadioButton::toggled, this, &MultiplayerRobotSelector::updatePreview);
    connect(p1TankBtn, &QRadioButton::toggled, this, &MultiplayerRobotSelector::updatePreview);
    connect(p1SniperBtn, &QRadioButton::toggled, this, &MultiplayerRobotSelector::updatePreview);
    
    connect(p2ScoutBtn, &QRadioButton::toggled, this, &MultiplayerRobotSelector::updatePreview);
    connect(p2TankBtn, &QRadioButton::toggled, this, &MultiplayerRobotSelector::updatePreview);
    connect(p2SniperBtn, &QRadioButton::toggled, this, &MultiplayerRobotSelector::updatePreview);
    
    connect(selectButton, &QPushButton::clicked, this, &MultiplayerRobotSelector::onSelectClicked);
    connect(backButton, &QPushButton::clicked, this, &MultiplayerRobotSelector::onBackClicked);
    
    // Style buttons
    QString buttonStyle = "QPushButton {"
                         "    background-color: #4CAF50;"
                         "    color: white;"
                         "    border-radius: 5px;"
                         "    padding: 8px 16px;"
                         "}"
                         "QPushButton:hover {"
                         "    background-color: #45a049;"
                         "}"
                         "QPushButton:pressed {"
                         "    background-color: #3e8e41;"
                         "}";
    
    selectButton->setStyleSheet(buttonStyle);
    backButton->setStyleSheet(buttonStyle.replace("#4CAF50", "#f44336")
                                      .replace("#45a049", "#e53935")
                                      .replace("#3e8e41", "#d32f2f"));
    
    setMinimumWidth(700);
    updatePreview();
}

QLabel* MultiplayerRobotSelector::createRobotPreview(RobotType type) {
    QLabel* previewLabel = new QLabel(this);
    previewLabel->setFixedSize(150, 150);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setStyleSheet("border: 2px solid #006064; background-color: #E0F7FA;");
    
    // Set the robot image based on type
    Robot robot(type);
    QPixmap robotImage = robot.getSideViewSprite();
    robotImage = robotImage.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    previewLabel->setPixmap(robotImage);
    
    return previewLabel;
}

QString MultiplayerRobotSelector::getRobotDescription(RobotType type) {
    switch (type) {
        case RobotType::Scout:
            return "Scout: Fast robot with 3 moves per turn. Low health (70) and attack damage (15).";
        case RobotType::Tank:
            return "Tank: Powerful robot with 2 moves per turn. High health (150) and attack damage (25).";
        case RobotType::Sniper:
            return "Sniper: Long-range robot with 2 moves per turn. Medium health (80) and high attack damage (35). Can attack from 3 tiles away.";
        default:
            return "Unknown robot type";
    }
}

void MultiplayerRobotSelector::updatePreview() {
    // Update Player 1 preview
    RobotType p1Type = RobotType::Scout;
    if (p1ScoutBtn->isChecked()) p1Type = RobotType::Scout;
    else if (p1TankBtn->isChecked()) p1Type = RobotType::Tank;
    else if (p1SniperBtn->isChecked()) p1Type = RobotType::Sniper;
    
    Robot p1Robot(p1Type);
    QPixmap p1Image = p1Robot.getSideViewSprite();
    p1Image = p1Image.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p1PreviewLabel->setPixmap(p1Image);
    p1DescriptionLabel->setText(getRobotDescription(p1Type));
    
    // Update Player 2 preview
    RobotType p2Type = RobotType::Tank;
    if (p2ScoutBtn->isChecked()) p2Type = RobotType::Scout;
    else if (p2TankBtn->isChecked()) p2Type = RobotType::Tank;
    else if (p2SniperBtn->isChecked()) p2Type = RobotType::Sniper;
    
    Robot p2Robot(p2Type);
    QPixmap p2Image = p2Robot.getSideViewSprite();
    p2Image = p2Image.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p2PreviewLabel->setPixmap(p2Image);
    p2DescriptionLabel->setText(getRobotDescription(p2Type));
}

void MultiplayerRobotSelector::onSelectClicked() {
    // Determine selected robot types
    RobotType player1Type = RobotType::Scout;
    if (p1ScoutBtn->isChecked()) player1Type = RobotType::Scout;
    else if (p1TankBtn->isChecked()) player1Type = RobotType::Tank;
    else if (p1SniperBtn->isChecked()) player1Type = RobotType::Sniper;
    
    RobotType player2Type = RobotType::Tank;
    if (p2ScoutBtn->isChecked()) player2Type = RobotType::Scout;
    else if (p2TankBtn->isChecked()) player2Type = RobotType::Tank;
    else if (p2SniperBtn->isChecked()) player2Type = RobotType::Sniper;
    
    emit robotsSelected(player1Type, player2Type);
}

void MultiplayerRobotSelector::onBackClicked() {
    emit backButtonClicked();
} 