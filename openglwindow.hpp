#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <imgui.h>

#include <list>
#include <random>
#include <vector>

#include "abcg.hpp"
#include "camera.hpp"
#include "gamedata.hpp"
#include "model.hpp"

struct BallPosition {
  float position_x;
  float position_z;
  bool wasFound;
};

class OpenGLWindow : public abcg::OpenGLWindow {
 protected:
  void handleEvent(SDL_Event& ev) override;
  void initializeGL() override;
  void paintGL() override;
  void paintUI() override;
  void resizeGL(int width, int height) override;
  void terminateGL() override;

 private:
  GLuint m_VAO{};
  GLuint m_program{};

  int m_viewportWidth{};
  int m_viewportHeight{};

  float m_dollySpeed{0.0f};
  float m_truckSpeed{0.0f};
  float m_panSpeed{0.0f};

  Camera m_camera;
  GameData m_gameData;
  Model m_ball_model;
  Model m_house_model;

  std::list<BallPosition> m_balls;
  std::list<BallPosition> m_wrong_balls;

  std::vector<GLuint> m_programs;
  std::default_random_engine m_randomEngine;
  std::uniform_real_distribution<float> m_randomDist{-2.0f, 2.0f};
  abcg::ElapsedTimer m_game_time;

  glm::mat4 m_modelMatrix{1.0f};
  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};

  // Light and material properties
  glm::vec4 m_lightDir{-1.0f, -1.0f, -1.0f, 0.0f};
  glm::vec4 m_Ia{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 m_Id{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 m_Is{1.0f, 1.0f, 1.0f, 0.5f};
  glm::vec4 m_Ka{0.11f, 0.05f, 0.0f, 0.0f};
  glm::vec4 m_Kd{0.71f, 0.6f, 0.2f, 0.0f};
  glm::vec4 m_Ks{0.99f, 0.86f, 0.47f, 0.0f};
  float m_shininess{25.0f};

  int numberOfFoundItems = 0;
  int m_currentProgramIndex{};
  int m_mappingMode{};

  ImFont* m_font{};

  void update();
  void checkGameCondition();
  void initBalls(int quantity);
  void checkFound();
  void renderHome();
  void renderBalls();
  void paintModels();
  void removeBallInFoundItems();
  bool checkFoundBetweenCameraAndPosition(float position_x, float position_z);
};

#endif