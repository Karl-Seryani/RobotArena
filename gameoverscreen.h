#ifndef GAMEOVERSCREEN_H
#define GAMEOVERSCREEN_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @brief This class creates a game-over screen whenever it is called
 * @author Group 17
 */
class GameOverScreen : public QWidget {
    Q_OBJECT
public:
    /// @brief Displays the game-over screen
    /// @param playerWon - The player that won the game
    /// @param winnerText - The text for the player who won the game
    /// @param parent - the parent widget of this object
    explicit GameOverScreen(bool playerWon, const QString& winnerText = "", QWidget *parent = nullptr);

signals:
    void playAgainSelected();
    void newRobotsSelected();
    void mainMenuSelected();

private slots:
    void onPlayAgainClicked();
    void onNewRobotsClicked();
    void onMainMenuClicked();

private:
    QPushButton* playAgainButton;
    QPushButton* newRobotsButton;
    QPushButton* mainMenuButton;
};

#endif // GAMEOVERSCREEN_H 