#ifndef MAPSELECTOR_H
#define MAPSELECTOR_H

#include <QWidget>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>

/// @brief The type of map we have, they can be random, open, maze, or fortress\n
/// **Fortress**: Central fortress with walls
/// **Maze**: Maze-like structure with paths
/// **Open**: Few obstacles, open arena, combat focused
/// **Random**: Random obstacles
enum class MapType {
    Random,     // Random obstacles
    Open,       // Few obstacles, open arena
    Maze,       // Maze-like structure with paths
    Fortress    // Central fortress with walls
};

/// @brief This class selects the map we'll be using for the arena
///@author Group 17
class MapSelector : public QWidget {
    Q_OBJECT
public:
    /// @brief Constructor for the MapSelector object, creates MapType that can be obtained using getSelectedMapType()
    /// @param parent - The parent QWidget of this object
    /// @see getSelectedMapType()
    explicit MapSelector(QWidget *parent = nullptr);

    /// @brief Getter function that returns the map type of
    /// @return MapType value that represents what type of map the arena will be using
    MapType getSelectedMapType() const;

signals:
    void mapSelected(MapType mapType);
    void backButtonClicked();

private:
    QRadioButton* randomBtn;
    QRadioButton* openBtn;
    QRadioButton* mazeBtn;
    QRadioButton* fortressBtn;
    QLabel* descriptionLabel;
    QLabel* previewLabel;
    QPushButton* selectButton;
    QPushButton* backButton;

private slots:
    void updateDescription();
    void onSelectClicked();
    void onBackClicked();
};

#endif // MAPSELECTOR_H 