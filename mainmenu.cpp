#include "mainmenu.h"
#include <QFont>
#include <QPixmap>

MainMenu::MainMenu(QWidget *parent) : QWidget(parent) {
    // Create main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(20);
    
    // Create title label
    titleLabel = new QLabel("Robot Arena", this);
    QFont titleFont("Arial", 24, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // Create buttons
    singlePlayerButton = new QPushButton("Single Player", this);
    multiplayerButton = new QPushButton("Multiplayer", this);
    tutorialButton = new QPushButton("Tutorial", this);
    
    // Style buttons
    QFont buttonFont("Arial", 14);
    QString buttonStyle = "QPushButton {"
                         "    background-color: #4CAF50;"
                         "    color: white;"
                         "    border-radius: 5px;"
                         "    padding: 10px 20px;"
                         "    min-width: 200px;"
                         "}"
                         "QPushButton:hover {"
                         "    background-color: #45a049;"
                         "}"
                         "QPushButton:pressed {"
                         "    background-color: #3e8e41;"
                         "}";
    
    singlePlayerButton->setFont(buttonFont);
    multiplayerButton->setFont(buttonFont);
    tutorialButton->setFont(buttonFont);
    
    singlePlayerButton->setStyleSheet(buttonStyle);
    multiplayerButton->setStyleSheet(buttonStyle);
    tutorialButton->setStyleSheet(buttonStyle);
    
    // Add widgets to layout
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(singlePlayerButton);
    mainLayout->addWidget(multiplayerButton);
    mainLayout->addWidget(tutorialButton);
    
    // Connect signals
    connect(singlePlayerButton, &QPushButton::clicked, this, &MainMenu::singlePlayerSelected);
    connect(multiplayerButton, &QPushButton::clicked, this, &MainMenu::multiplayerSelected);
    connect(tutorialButton, &QPushButton::clicked, this, &MainMenu::tutorialSelected);
    
    // Set window properties
    setMinimumSize(400, 500);
    setWindowTitle("Robot Arena");
} 