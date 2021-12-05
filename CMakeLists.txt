project(findBalls)
add_executable(${PROJECT_NAME} camera.cpp main.cpp model.cpp openglwindow.cpp)
enable_abcg(${PROJECT_NAME})
