#include <QApplication>
#include "gamegrid.h"
#include "mainmenu.h"
#include "gamemanager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Create the game manager
    GameManager* gameManager = new GameManager();
    gameManager->getMainWidget()->show();
    
    return app.exec();
} 