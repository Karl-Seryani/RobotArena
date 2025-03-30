#include "robotselector.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>

RobotSelector::RobotSelector(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Select Robots");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create title label
    QLabel* titleLabel = new QLabel("Select Your Robots", this);
    QFont titleFont("Arial", 18, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Create horizontal layout for player and AI selection
    QHBoxLayout* selectionLayout = new QHBoxLayout();
    
    // ===== PLAYER ROBOT SELECTION =====
    QVBoxLayout* playerLayout = new QVBoxLayout();
    
    // Create player robot selection group
    playerGroup = new QGroupBox("Player Robot");
    QVBoxLayout* playerGroupLayout = new QVBoxLayout();
    
    playerScoutBtn = new QRadioButton("Scout " + Robot(RobotType::Scout).getDisplayChar());
    playerTankBtn = new QRadioButton("Tank " + Robot(RobotType::Tank).getDisplayChar());
    playerSniperBtn = new QRadioButton("Sniper " + Robot(RobotType::Sniper).getDisplayChar());
    playerScoutBtn->setChecked(true);
    
    playerGroupLayout->addWidget(playerScoutBtn);
    playerGroupLayout->addWidget(playerTankBtn);
    playerGroupLayout->addWidget(playerSniperBtn);
    playerGroup->setLayout(playerGroupLayout);
    
    // Create player description label
    playerDescriptionLabel = new QLabel(Robot(RobotType::Scout).getDescription(), this);
    playerDescriptionLabel->setWordWrap(true);
    playerDescriptionLabel->setStyleSheet("padding: 10px; background-color: #f0f0f0; border-radius: 5px;");
    
    playerLayout->addWidget(playerGroup);
    playerLayout->addWidget(playerDescriptionLabel);
    
    // ===== AI ROBOT SELECTION =====
    QVBoxLayout* aiLayout = new QVBoxLayout();
    
    // Create AI robot selection group
    aiGroup = new QGroupBox("AI Robot");
    QVBoxLayout* aiGroupLayout = new QVBoxLayout();
    
    aiScoutBtn = new QRadioButton("Scout " + Robot(RobotType::Scout).getDisplayChar());
    aiTankBtn = new QRadioButton("Tank " + Robot(RobotType::Tank).getDisplayChar());
    aiSniperBtn = new QRadioButton("Sniper " + Robot(RobotType::Sniper).getDisplayChar());
    aiRandomBtn = new QRadioButton("Random");
    aiRandomBtn->setChecked(true);
    
    aiGroupLayout->addWidget(aiScoutBtn);
    aiGroupLayout->addWidget(aiTankBtn);
    aiGroupLayout->addWidget(aiSniperBtn);
    aiGroupLayout->addWidget(aiRandomBtn);
    aiGroup->setLayout(aiGroupLayout);
    
    // Create AI description label
    aiDescriptionLabel = new QLabel("Random: AI will use a randomly selected robot type.", this);
    aiDescriptionLabel->setWordWrap(true);
    aiDescriptionLabel->setStyleSheet("padding: 10px; background-color: #f0f0f0; border-radius: 5px;");
    
    aiLayout->addWidget(aiGroup);
    aiLayout->addWidget(aiDescriptionLabel);
    
    // Add player and AI layouts to selection layout
    selectionLayout->addLayout(playerLayout);
    selectionLayout->addLayout(aiLayout);
    
    // Add selection layout to main layout
    mainLayout->addLayout(selectionLayout);
    
    // Create buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    selectButton = new QPushButton("Select", this);
    backButton = new QPushButton("Back", this);
    
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(selectButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals for player robot
    connect(playerScoutBtn, &QRadioButton::toggled, this, &RobotSelector::updatePlayerDescription);
    connect(playerTankBtn, &QRadioButton::toggled, this, &RobotSelector::updatePlayerDescription);
    connect(playerSniperBtn, &QRadioButton::toggled, this, &RobotSelector::updatePlayerDescription);
    
    // Connect signals for AI robot
    connect(aiScoutBtn, &QRadioButton::toggled, this, &RobotSelector::updateAIDescription);
    connect(aiTankBtn, &QRadioButton::toggled, this, &RobotSelector::updateAIDescription);
    connect(aiSniperBtn, &QRadioButton::toggled, this, &RobotSelector::updateAIDescription);
    connect(aiRandomBtn, &QRadioButton::toggled, this, &RobotSelector::updateAIDescription);
    
    connect(selectButton, &QPushButton::clicked, this, &RobotSelector::onSelectClicked);
    connect(backButton, &QPushButton::clicked, this, &RobotSelector::onBackClicked);
    
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
    
    setMinimumWidth(600);
}

RobotType RobotSelector::getSelectedPlayerType() const {
    if (playerScoutBtn->isChecked()) return RobotType::Scout;
    if (playerTankBtn->isChecked()) return RobotType::Tank;
    if (playerSniperBtn->isChecked()) return RobotType::Sniper;
    return RobotType::Scout; // Default to Scout
}

RobotType RobotSelector::getSelectedAIType() const {
    if (aiScoutBtn->isChecked()) return RobotType::Scout;
    if (aiTankBtn->isChecked()) return RobotType::Tank;
    if (aiSniperBtn->isChecked()) return RobotType::Sniper;
    // For random, we'll return Scout and handle the randomization elsewhere
    return RobotType::Scout;
}

bool RobotSelector::isRandomAI() const {
    return aiRandomBtn->isChecked();
}

void RobotSelector::updatePlayerDescription() {
    Robot temp(getSelectedPlayerType());
    playerDescriptionLabel->setText(temp.getDescription());
}

void RobotSelector::updateAIDescription() {
    if (aiRandomBtn->isChecked()) {
        aiDescriptionLabel->setText("Random: AI will use a randomly selected robot type.");
    } else {
        Robot temp(getSelectedAIType());
        aiDescriptionLabel->setText(temp.getDescription());
    }
}

void RobotSelector::onSelectClicked() {
    emit robotsSelected(getSelectedPlayerType(), getSelectedAIType(), isRandomAI());
}

void RobotSelector::onBackClicked() {
    emit backButtonClicked();
}
