QT += testlib
QT += widgets
QMAKE_CXXFLAGS += -std=c++17
CONFIG += console
CONFIG -= app_bundle

SOURCES += tests/test.cpp

SOURCES += \
    tests/test_difficulty_levels.cpp \
    tests/test_game_completion.cpp \
    tests/test_game_controls.cpp \
    tests/test_map_selection.cpp \
    tests/test_multiplayer_robot_selection.cpp \
    tests/test_powerups_environment.cpp \
    tests/test_robot_selection.cpp

HEADERS += \
    tests/test_difficulty_levels.h \
    tests/test_game_completion.h \
    tests/test_game_controls.h \
    tests/test_map_selection.h \
    tests/test_multiplayer_robot_selection.h \
    tests/test_powerups_environment.h \
    tests/test_robot_selection.h

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

RESOURCES += \
    resources.qrc

TARGET = robot_arena_tests