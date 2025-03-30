#include "difficultyselector.h"
#include <QGroupBox>
#include <QHBoxLayout>

DifficultySelector::DifficultySelector(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Select Difficulty");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create title label
    QLabel* titleLabel = new QLabel("Select Difficulty Level", this);
    QFont titleFont("Arial", 18, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // Create difficulty selection group
    QGroupBox* difficultyGroup = new QGroupBox("Difficulty Levels");
    QVBoxLayout* groupLayout = new QVBoxLayout;
    
    easyBtn = new QRadioButton("Easy", this);
    mediumBtn = new QRadioButton("Medium", this);
    hardBtn = new QRadioButton("Hard", this);
    mediumBtn->setChecked(true);
    
    groupLayout->addWidget(easyBtn);
    groupLayout->addWidget(mediumBtn);
    groupLayout->addWidget(hardBtn);
    difficultyGroup->setLayout(groupLayout);
    
    // Create description label
    descriptionLabel = new QLabel("Medium difficulty: Balanced AI behavior with moderate aggressiveness and strategy.", this);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setStyleSheet("padding: 10px; background-color: #f0f0f0; border-radius: 5px;");
    
    // Create buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    selectButton = new QPushButton("Select", this);
    backButton = new QPushButton("Back", this);
    
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(selectButton);
    
    // Add everything to main layout
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(difficultyGroup);
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(easyBtn, &QRadioButton::toggled, this, &DifficultySelector::updateDescription);
    connect(mediumBtn, &QRadioButton::toggled, this, &DifficultySelector::updateDescription);
    connect(hardBtn, &QRadioButton::toggled, this, &DifficultySelector::updateDescription);
    
    connect(selectButton, &QPushButton::clicked, this, &DifficultySelector::onSelectClicked);
    connect(backButton, &QPushButton::clicked, this, &DifficultySelector::onBackClicked);
    
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
    
    setMinimumWidth(400);
    updateDescription();
}

GameDifficulty DifficultySelector::getSelectedDifficulty() const {
    if (easyBtn->isChecked()) return GameDifficulty::Easy;
    if (hardBtn->isChecked()) return GameDifficulty::Hard;
    return GameDifficulty::Medium;
}

void DifficultySelector::updateDescription() {
    if (easyBtn->isChecked()) {
        descriptionLabel->setText("Easy difficulty: AI makes more random moves (50% chance), deals less damage (70%), and has reduced health (70%). Good for beginners.");
    } else if (mediumBtn->isChecked()) {
        descriptionLabel->setText("Medium difficulty: Balanced AI behavior with standard health and damage. AI makes occasional random moves (20% chance).");
    } else if (hardBtn->isChecked()) {
        descriptionLabel->setText("Hard difficulty: AI uses advanced strategies, rarely makes random moves (5%), has increased health (130%), and deals more damage (130%). A true challenge!");
    }
}

void DifficultySelector::onSelectClicked() {
    emit difficultySelected(getSelectedDifficulty());
}

void DifficultySelector::onBackClicked() {
    emit backButtonClicked();
} 