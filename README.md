To run and test the program you need to:
- Run `qmake RobotArena.pro` on terminal
- Run `make`
- Run `./robot_arena` (Windows) or Double click robot_arena.app (Mac)
- Open another terminal
- Run `qmake tests.pro`
- Run `make`
- Run `./robot_arena_tests`

To cleanup output files:
- Run `qmake tests.pro`
- Run `make clean`
- Run `qmake RobotArena.pro`
- Run `make clean`
- Delete `./robot_arena` or `robot_arena.app` and `./robot_arena_tests`

To open the Doxygen html document:
- Go to Doxygen/Html
- Run anything with .html in it