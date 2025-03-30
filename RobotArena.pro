QT += widgets
QMAKE_CXXFLAGS += -std=c++17

SOURCES += \
    gamegrid.cpp \
    game.cpp \
    robot.cpp \
    robotselector.cpp \
    multiplayerrobotselector.cpp \
    mainmenu.cpp \
    gamemanager.cpp \
    tutorial.cpp \
    difficultyselector.cpp \
    gameoverscreen.cpp \
    mapselector.cpp \
    robotai.cpp \
    main.cpp \
    projectile.cpp \
    hitfeedback.cpp \
    laserfeedback.cpp \
    sniperai.cpp \
    scoutai.cpp \
    tankai.cpp \
    logger.cpp

HEADERS += \
    gamegrid.h \
    game.h \
    robot.h \
    robotselector.h \
    multiplayerrobotselector.h \
    mainmenu.h \
    gamemanager.h \
    tutorial.h \
    difficultyselector.h \
    gameoverscreen.h \
    mapselector.h \
    robotai.h \
    projectile.h \
    hitfeedback.h \
    laserfeedback.h \
    aiinterface.h \
    sniperai.h \
    scoutai.h \
    tankai.h \
    logger.h

TARGET = robot_arena
TEMPLATE = app

RESOURCES += \
    resources.qrc
