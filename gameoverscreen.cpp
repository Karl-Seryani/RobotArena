#include "gameoverscreen.h"
#include <QFont>

GameOverScreen::GameOverScreen(bool playerWon, const QString& winnerText, QWidget *parent) : QWidget(parent) {
    setWindowTitle("Game Over");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create title label
    QLabel* titleLabel = new QLabel("Game Over", this);
    QFont titleFont("Arial", 24, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // Create result label
    QLabel* resultLabel = new QLabel(winnerText.isEmpty() ? 
                                    (playerWon ? "Player Wins!" : "AI Wins!") : 
                                    winnerText, this);
    QFont resultFont("Arial", 18);
    resultLabel->setFont(resultFont);
    resultLabel->setAlignment(Qt::AlignCenter);
    
    // Create buttons
    playAgainButton = new QPushButton("Play Again", this);
    newRobotsButton = new QPushButton("Select New Robots", this);
    mainMenuButton = new QPushButton("Main Menu", this);
    
    // Add widgets to layout
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(resultLabel);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(playAgainButton);
    mainLayout->addWidget(newRobotsButton);
    mainLayout->addWidget(mainMenuButton);
    
    // Connect signals
    connect(playAgainButton, &QPushButton::clicked, this, &GameOverScreen::onPlayAgainClicked);
    connect(newRobotsButton, &QPushButton::clicked, this, &GameOverScreen::onNewRobotsClicked);
    connect(mainMenuButton, &QPushButton::clicked, this, &GameOverScreen::onMainMenuClicked);
    
    // Style buttons
    QString buttonStyle = "QPushButton {"
                         "    background-color: #4CAF50;"
                         "    color: white;"
                         "    border-radius: 5px;"
                         "    padding: 8px 16px;"
                         "    font-size: 16px;"
                         "}"
                         "QPushButton:hover {"
                         "    background-color: #45a049;"
                         "}"
                         "QPushButton:pressed {"
                         "    background-color: #3e8e41;"
                         "}";
    
    playAgainButton->setStyleSheet(buttonStyle);
    
    newRobotsButton->setStyleSheet(buttonStyle.replace("#4CAF50", "#2196F3")
                                          .replace("#45a049", "#1976D2")
                                          .replace("#3e8e41", "#0D47A1"));
    
    mainMenuButton->setStyleSheet(buttonStyle.replace("#2196F3", "#f44336")
                                         .replace("#1976D2", "#e53935")
                                         .replace("#0D47A1", "#d32f2f"));
    
    // Set background color
    setStyleSheet("background-color: #E0F7FA;");
}

void GameOverScreen::onPlayAgainClicked() {
    emit playAgainSelected();
}

void GameOverScreen::onNewRobotsClicked() {
    emit newRobotsSelected();
}

void GameOverScreen::onMainMenuClicked() {
    emit mainMenuSelected();
} 