#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @brief MainMenu interface where the user can choose what to do in this game
 * @author Group 17
 */
class MainMenu : public QWidget {
    Q_OBJECT
public:
    /// @brief Constructor for the MainMenu interface
    /// @param parent - The parent object to which the menu is based upon
    explicit MainMenu(QWidget *parent = nullptr);

signals:
    void singlePlayerSelected();
    void multiplayerSelected();
    void tutorialSelected();

private:
    QPushButton* singlePlayerButton;
    QPushButton* multiplayerButton;
    QPushButton* tutorialButton;
    QLabel* titleLabel;
};

#endif // MAINMENU_H 