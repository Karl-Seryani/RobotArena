#ifndef DIFFICULTYSELECTOR_H
#define DIFFICULTYSELECTOR_H

#include <QWidget>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

///@brief The game difficulties includes easy, medium, and hard
enum class GameDifficulty {
    Easy,
    Medium,
    Hard
};

/// @brief The difficulty selector defines an interface to allow users to select difficulty.
///@author Group 17
class DifficultySelector : public QWidget {
    Q_OBJECT
public:
    /// @brief This function creates the UI for the difficulty screen, and allows user to select difficulty on the screen
    /// @param parent pointer to the parent of the function, used primarily for ownership
    explicit DifficultySelector(QWidget *parent = nullptr);
    /// @brief Getter method for the difficulty of the game
    /// @return Difficulty of the game
    /// @see DifficultySelector
    GameDifficulty getSelectedDifficulty() const;

signals:
    ///@brief Emits a signal that a difficulty is selected by the user
    void difficultySelected(GameDifficulty difficulty);
    /// @brief Emits a signal when the back button is clicked by the user
    void backButtonClicked();

private:
    QRadioButton* easyBtn;
    QRadioButton* mediumBtn;
    QRadioButton* hardBtn;
    QLabel* descriptionLabel;
    QPushButton* selectButton;
    QPushButton* backButton;

private slots:
    void updateDescription();
    void onSelectClicked();
    void onBackClicked();
};

#endif // DIFFICULTYSELECTOR_H 