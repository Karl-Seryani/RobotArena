#include "mapselector.h"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QRandomGenerator>

MapSelector::MapSelector(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Select Map");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create title label
    QLabel* titleLabel = new QLabel("Select Arena Map", this);
    QFont titleFont("Arial", 18, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // Create map selection group
    QGroupBox* mapGroup = new QGroupBox("Map Types");
    QVBoxLayout* groupLayout = new QVBoxLayout;
    
    randomBtn = new QRadioButton("Random", this);
    openBtn = new QRadioButton("Open Arena", this);
    mazeBtn = new QRadioButton("Maze", this);
    fortressBtn = new QRadioButton("Fortress", this);
    randomBtn->setChecked(true);
    
    groupLayout->addWidget(randomBtn);
    groupLayout->addWidget(openBtn);
    groupLayout->addWidget(mazeBtn);
    groupLayout->addWidget(fortressBtn);
    mapGroup->setLayout(groupLayout);
    
    // Create preview label
    previewLabel = new QLabel(this);
    previewLabel->setFixedSize(300, 300);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setStyleSheet("border: 2px solid #006064; background-color: #E0F7FA;");
    
    // Create description label
    descriptionLabel = new QLabel("Random: A randomly generated arena with walls scattered throughout.", this);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setStyleSheet("padding: 10px; background-color: #f0f0f0; border-radius: 5px;");
    
    // Create buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    selectButton = new QPushButton("Select", this);
    backButton = new QPushButton("Back", this);
    
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(selectButton);
    
    // Create layout for map preview and description
    QHBoxLayout* previewLayout = new QHBoxLayout;
    previewLayout->addWidget(previewLabel);
    
    QVBoxLayout* rightLayout = new QVBoxLayout;
    rightLayout->addWidget(mapGroup);
    rightLayout->addWidget(descriptionLabel);
    rightLayout->addStretch();
    
    previewLayout->addLayout(rightLayout);
    
    // Add everything to main layout
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(previewLayout);
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(randomBtn, &QRadioButton::toggled, this, &MapSelector::updateDescription);
    connect(openBtn, &QRadioButton::toggled, this, &MapSelector::updateDescription);
    connect(mazeBtn, &QRadioButton::toggled, this, &MapSelector::updateDescription);
    connect(fortressBtn, &QRadioButton::toggled, this, &MapSelector::updateDescription);
    
    connect(selectButton, &QPushButton::clicked, this, &MapSelector::onSelectClicked);
    connect(backButton, &QPushButton::clicked, this, &MapSelector::onBackClicked);
    
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
    updateDescription();
}

MapType MapSelector::getSelectedMapType() const {
    if (openBtn->isChecked()) return MapType::Open;
    if (mazeBtn->isChecked()) return MapType::Maze;
    if (fortressBtn->isChecked()) return MapType::Fortress;
    return MapType::Random;
}

void MapSelector::updateDescription() {
    // Update description based on selected map type
    if (randomBtn->isChecked()) {
        descriptionLabel->setText("Random: A randomly generated arena with walls scattered throughout. "
                                 "Each game will have a different layout.");
        
        // Create a simple preview of a random map
        QPixmap preview(300, 300);
        preview.fill(Qt::white);
        QPainter painter(&preview);
        painter.setPen(Qt::black);
        
        // Draw grid
        for (int i = 0; i <= 8; i++) {
            painter.drawLine(i * 37, 0, i * 37, 300);
            painter.drawLine(0, i * 37, 300, i * 37);
        }
        
        // Draw random walls
        painter.setBrush(QBrush(Qt::darkGray));
        for (int i = 0; i < 20; i++) {
            int x = QRandomGenerator::global()->bounded(8) * 37;
            int y = QRandomGenerator::global()->bounded(8) * 37;
            painter.fillRect(x + 1, y + 1, 36, 36, Qt::darkGray);
        }
        
        previewLabel->setPixmap(preview);
    } 
    else if (openBtn->isChecked()) {
        descriptionLabel->setText("Open Arena: A mostly open arena with few obstacles. "
                                 "Good for direct combat and fast-paced games.");
        
        // Create a preview of an open map
        QPixmap preview(300, 300);
        preview.fill(Qt::white);
        QPainter painter(&preview);
        painter.setPen(Qt::black);
        
        // Draw grid
        for (int i = 0; i <= 8; i++) {
            painter.drawLine(i * 37, 0, i * 37, 300);
            painter.drawLine(0, i * 37, 300, i * 37);
        }
        
        // Draw a few walls
        painter.setBrush(QBrush(Qt::darkGray));
        painter.fillRect(3 * 37 + 1, 3 * 37 + 1, 36, 36, Qt::darkGray);
        painter.fillRect(4 * 37 + 1, 3 * 37 + 1, 36, 36, Qt::darkGray);
        painter.fillRect(3 * 37 + 1, 4 * 37 + 1, 36, 36, Qt::darkGray);
        painter.fillRect(4 * 37 + 1, 4 * 37 + 1, 36, 36, Qt::darkGray);
        
        previewLabel->setPixmap(preview);
    } 
    else if (mazeBtn->isChecked()) {
        descriptionLabel->setText("Maze: A complex maze-like structure with narrow corridors and multiple paths. "
                                 "Requires strategic movement and planning.");
        
        // Create a preview of a maze map
        QPixmap preview(300, 300);
        preview.fill(Qt::white);
        QPainter painter(&preview);
        painter.setPen(Qt::black);
        
        // Draw grid
        for (int i = 0; i <= 8; i++) {
            painter.drawLine(i * 37, 0, i * 37, 300);
            painter.drawLine(0, i * 37, 300, i * 37);
        }
        
        // Draw maze-like walls
        painter.setBrush(QBrush(Qt::darkGray));
        
        // Horizontal walls
        for (int i = 0; i < 7; i += 2) {
            for (int j = 0; j < 7; j++) {
                if (j != i % 3) {
                    painter.fillRect(j * 37 + 1, i * 37 + 1, 36, 36, Qt::darkGray);
                }
            }
        }
        
        // Vertical walls
        for (int i = 1; i < 7; i += 2) {
            for (int j = 0; j < 7; j++) {
                if (j != (i+1) % 3) {
                    painter.fillRect(i * 37 + 1, j * 37 + 1, 36, 36, Qt::darkGray);
                }
            }
        }
        
        previewLabel->setPixmap(preview);
    } 
    else if (fortressBtn->isChecked()) {
        descriptionLabel->setText("Fortress: A central fortress surrounded by walls with a few entry points. "
                                 "Encourages defensive play and strategic positioning.");
        
        // Create a preview of a fortress map
        QPixmap preview(300, 300);
        preview.fill(Qt::white);
        QPainter painter(&preview);
        painter.setPen(Qt::black);
        
        // Draw grid
        for (int i = 0; i <= 8; i++) {
            painter.drawLine(i * 37, 0, i * 37, 300);
            painter.drawLine(0, i * 37, 300, i * 37);
        }
        
        // Draw fortress walls
        painter.setBrush(QBrush(Qt::darkGray));
        
        // Outer walls
        for (int i = 1; i < 7; i++) {
            // Top and bottom walls
            if (i != 3) {
                painter.fillRect(i * 37 + 1, 1 * 37 + 1, 36, 36, Qt::darkGray);
                painter.fillRect(i * 37 + 1, 6 * 37 + 1, 36, 36, Qt::darkGray);
            }
            
            // Left and right walls
            if (i != 3) {
                painter.fillRect(1 * 37 + 1, i * 37 + 1, 36, 36, Qt::darkGray);
                painter.fillRect(6 * 37 + 1, i * 37 + 1, 36, 36, Qt::darkGray);
            }
        }
        
        // Inner fortress
        for (int i = 3; i <= 4; i++) {
            for (int j = 3; j <= 4; j++) {
                painter.fillRect(i * 37 + 1, j * 37 + 1, 36, 36, Qt::darkGray);
            }
        }
        
        previewLabel->setPixmap(preview);
    }
}

void MapSelector::onSelectClicked() {
    emit mapSelected(getSelectedMapType());
}

void MapSelector::onBackClicked() {
    emit backButtonClicked();
} 