#include "openglwindow.hpp"

#include <fmt/core.h>
#include <imgui.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

// Explicit specialization of std::hash for Vertex
namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const& vertex) const noexcept {
    const std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};
}  // namespace std

void OpenGLWindow::handleEvent(SDL_Event& ev) {
  if (ev.type == SDL_KEYDOWN) {
    if (ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_w)
      m_dollySpeed = 1.0f;
    if (ev.key.keysym.sym == SDLK_DOWN || ev.key.keysym.sym == SDLK_s)
      m_dollySpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_a)
      m_panSpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_d)
      m_panSpeed = 1.0f;
    if (ev.key.keysym.sym == SDLK_q) m_truckSpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_e) m_truckSpeed = 1.0f;
  }
  if (ev.type == SDL_KEYUP) {
    if ((ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_w) &&
        m_dollySpeed > 0)
      m_dollySpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_DOWN || ev.key.keysym.sym == SDLK_s) &&
        m_dollySpeed < 0)
      m_dollySpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_a) &&
        m_panSpeed < 0)
      m_panSpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_d) &&
        m_panSpeed > 0)
      m_panSpeed = 0.0f;
    if (ev.key.keysym.sym == SDLK_q && m_truckSpeed < 0) m_truckSpeed = 0.0f;
    if (ev.key.keysym.sym == SDLK_e && m_truckSpeed > 0) m_truckSpeed = 0.0f;
  }
}

void OpenGLWindow::initializeGL() {
  abcg::glClearColor(0, 0, 0, 1);

  // Enable depth buffering
  abcg::glEnable(GL_DEPTH_TEST);

  m_program = createProgramFromFile(getAssetsPath() + "texture.vert",
                                    getAssetsPath() + "texture.frag");

  m_ball_model.loadDiffuseTexture(getAssetsPath() + "/maps/golfe_map.jpg");
  // Load ball model
  m_ball_model.loadFromFile(getAssetsPath() + "soccer ball.obj", m_program,
                            false);

  // Load house model
  m_house_model.loadDiffuseTexture(getAssetsPath() + "/maps/madeira_map.jpg");
  m_house_model.loadFromFile(getAssetsPath() + "cottage_obj.obj", m_program,
                             false);

  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  resizeGL(getWindowSettings().width, getWindowSettings().height);
}

void OpenGLWindow::paintGL() {
  update();

  // Clear color buffer and depth buffer
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  abcg::glUseProgram(m_program);
  paintModels();

  abcg::glUseProgram(0);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();

  const auto size{ImVec2(500, 500)};
  const auto position{ImVec2((m_viewportWidth - size.x) / 2.0f,
                             (m_viewportHeight - size.y) / 2.0f)};
  ImGui::SetNextWindowPos(position);
  ImGui::SetNextWindowSize(size);
  ImGuiWindowFlags flags{};

  // As flags serão relativas ao estado do jogo
  if (m_gameData.m_state == State::Playing) {
    flags = {ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
             ImGuiWindowFlags_NoInputs};

  } else {
    flags = {ImGuiWindowFlags_NoDecoration};
  }

  ImGui::Begin(" ", nullptr, flags);
  ImGui::PushFont(m_font);

  if (m_gameData.m_state == State::Init) {
    ImGui::Text("VOCÊ TEM 20 SEGUNDOS PARA ENCONTRAR TODAS AS BOLAS VERMELHAS, \n FUJA DAS BOLAS AMARELAS !!!!!");
    if (ImGui::Button("Jogar", ImVec2(300, 80))) {
      initBalls(5);
      m_gameData.m_state = State::Menu;
    }
  } else if (m_gameData.m_state == State::Menu) {
    if (ImGui::Button("FÁCIL - 3 BOLAS", ImVec2(300, 80))) {
      m_gameData.m_state = State::Playing;
      m_game_time.restart();
      initBalls(3);
    }
    if (ImGui::Button("MÉDIO - 4 BOLAS", ImVec2(300, 80))) {
      m_gameData.m_state = State::Playing;
      m_game_time.restart();

      initBalls(4);
    }
    if (ImGui::Button("DIFÍCIL- 6 BOLAS ", ImVec2(300, 80))) {
      m_gameData.m_state = State::Playing;
      m_game_time.restart();

      initBalls(6);
    }

  } else if (m_gameData.m_state == State::Playing) {
    std::string text = std::to_string(numberOfFoundItems);
    char const* textFormat = text.c_str();
    ImGui::Text("NÚMERO DE BOLAS ENCONTRADAS :");
    ImGui::Text(textFormat);
  } else {
    const char* message =
        m_gameData.m_state == State::GameOver ? "Você Perdeu" : "Você Ganhou";
    ImGui::Text(message);
    if (ImGui::Button("Jogar Novamente", ImVec2(300, 80))) {
      numberOfFoundItems = 0;
      m_gameData.m_state = State::Menu;
    }
  }

  ImGui::PopFont();
  ImGui::End();
}

void OpenGLWindow::paintModels() {
  // Get location of uniform variables (could be precomputed)
  // Get location of uniform variables
  const GLint viewMatrixLoc{
      abcg::glGetUniformLocation(m_program, "viewMatrix")};
  const GLint projMatrixLoc{
      abcg::glGetUniformLocation(m_program, "projMatrix")};
  const GLint modelMatrixLoc{
      abcg::glGetUniformLocation(m_program, "modelMatrix")};
  const GLint normalMatrixLoc{
      abcg::glGetUniformLocation(m_program, "normalMatrix")};
  const GLint lightDirLoc{
      abcg::glGetUniformLocation(m_program, "lightDirWorldSpace")};
  const GLint shininessLoc{abcg::glGetUniformLocation(m_program, "shininess")};
  const GLint IaLoc{abcg::glGetUniformLocation(m_program, "Ia")};
  const GLint IdLoc{abcg::glGetUniformLocation(m_program, "Id")};
  const GLint IsLoc{abcg::glGetUniformLocation(m_program, "Is")};
  const GLint KaLoc{abcg::glGetUniformLocation(m_program, "Ka")};
  const GLint KdLoc{abcg::glGetUniformLocation(m_program, "Kd")};
  const GLint KsLoc{abcg::glGetUniformLocation(m_program, "Ks")};
  const GLint diffuseTexLoc{
      abcg::glGetUniformLocation(m_program, "diffuseTex")};
  const GLint mappingModeLoc{
      abcg::glGetUniformLocation(m_program, "mappingMode")};

  // Set uniform variables for viewMatrix and projMatrix
  // These matrices are used for every scene object
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE,
                           &m_camera.m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE,
                           &m_camera.m_projMatrix[0][0]);
  abcg::glUniform1i(diffuseTexLoc, 0);
  abcg::glUniform1i(mappingModeLoc, m_mappingMode);

  abcg::glBindVertexArray(m_VAO);

  auto lightDirRotated{m_lightDir};
  glUniform4fv(lightDirLoc, 1, &lightDirRotated.x);
  glUniform1f(shininessLoc, 12.5f);
  glUniform4fv(IaLoc, 1, &m_Ia.x);
  glUniform4fv(IdLoc, 1, &m_Id.x);
  glUniform4fv(IsLoc, 1, &m_Is.x);
  glUniform4fv(KsLoc, 1, &m_Ks.x);

  for (auto& ball : m_balls) {
    glm::mat4 ball_model{1.0f};
    // Draw ball
    ball_model = glm::translate(ball_model,
                                glm::vec3(ball.position_x, 0, ball.position_z));
    ball_model =
        glm::rotate(ball_model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    ball_model = glm::scale(ball_model, glm::vec3(0.1f));

    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &ball_model[0][0]);
    auto modelViewMatrix{glm::mat3(m_camera.m_viewMatrix * ball_model)};
    glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);

    if (ball.wasFound) {
      glm::vec4 ka{0.0f, 0.8f, 1.0f, 1.0f};
      glm::vec4 kd{0.0f, 1.0f, 0.0f, 1.0f};
      glUniform4fv(KaLoc, 1, &ka.x);
      glUniform4fv(KdLoc, 1, &kd.x);
    } else {
      glm::vec4 ka{1.0f, 0.25f, 0.25f, 1.0f};
      glm::vec4 kd{1.0f, 0.25f, 0.25f, 1.0f};
      glUniform4fv(KaLoc, 1, &ka.x);
      glUniform4fv(KdLoc, 1, &kd.x);
    }
    m_ball_model.render(-1);
  }

  for (auto& wrong_ball : m_wrong_balls) {
    glm::mat4 wrong_ball_model{1.0f};
    // Draw ball
    wrong_ball_model = glm::translate(
        wrong_ball_model,
        glm::vec3(wrong_ball.position_x, 0, wrong_ball.position_z));
    wrong_ball_model =
        glm::rotate(wrong_ball_model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    wrong_ball_model = glm::scale(wrong_ball_model, glm::vec3(0.1f));
    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE,
                             &wrong_ball_model[0][0]);
    auto modelViewMatrix{glm::mat3(m_camera.m_viewMatrix * wrong_ball_model)};
    glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);

    glUniform4fv(KaLoc, 1, &m_Ka.x);
    glUniform4fv(KdLoc, 1, &m_Kd.x);

    m_ball_model.render(-1);
  }

  glm::mat4 house_model{1.0f};
  house_model = glm::translate(house_model, glm::vec3(0.5f, 0.0f, 5.0f));
  house_model =
      glm::rotate(house_model, glm::radians(90.0f), glm::vec3(0, 1, 0));
  house_model = glm::scale(house_model, glm::vec3(5.0f));
  glUniform4fv(KaLoc, 1, &m_Ka.x);
  glUniform4fv(KdLoc, 1, &m_Kd.x);

  m_house_model.render(-1);
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  m_camera.computeProjectionMatrix(width, height);
}

void OpenGLWindow::terminateGL() {
  m_ball_model.terminateGL();
  m_house_model.terminateGL();
  abcg::glDeleteProgram(m_program);
}

void OpenGLWindow::update() {
  const float deltaTime{static_cast<float>(getDeltaTime())};

  if (m_gameData.m_state == State::Playing) {
    checkGameCondition();
    checkFound();
  }

  // Update LookAt camera
  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);
}

void OpenGLWindow::initBalls(int quantity) {
  m_balls.clear();
  m_balls.resize(quantity);
  m_wrong_balls.clear();
  m_wrong_balls.resize(quantity - 2);

  auto& re{m_randomEngine};
  for (auto& ball : m_balls) {
    ball.position_x = m_randomDist(re);
    ball.position_z = m_randomDist(re);
    ball.wasFound = false;
  }

  for (auto& wrong_ball : m_wrong_balls) {
    wrong_ball.position_x = m_randomDist(re);
    wrong_ball.position_z = m_randomDist(re);
    wrong_ball.wasFound = false;
  }
}

void OpenGLWindow::checkFound() {
  for (auto& ball : m_balls) {
    if (!ball.wasFound) {
      bool found =
          checkFoundBetweenCameraAndPosition(ball.position_x, ball.position_z);
      if (found) {
        ball.wasFound = true;
        numberOfFoundItems++;
      }
    }
  }

  for (auto& wrong_ball : m_wrong_balls) {
      bool found = checkFoundBetweenCameraAndPosition(wrong_ball.position_x,
                                                      wrong_ball.position_z);
      if (found) {
        wrong_ball.wasFound = true;
        removeBallInFoundItems();
      }
    }
}

void OpenGLWindow::removeBallInFoundItems() {
  for (auto& ball : m_balls) {
    if (ball.wasFound) {
      ball.wasFound = false;
      numberOfFoundItems = 0;
      return;
    }
  }
}

bool OpenGLWindow::checkFoundBetweenCameraAndPosition(float position_x,
                                                      float position_z) {
  const auto distance{
      glm::distance(glm::vec3(position_x, 0, position_z),
                    glm::vec3(m_camera.m_eye.x, 0, m_camera.m_eye.z))};

  return distance < 0.8f;
}

void OpenGLWindow::checkGameCondition() {
  if (m_game_time.elapsed() > 45) {
    m_gameData.m_state = State::GameOver;
    m_game_time.restart();
  }

  if (numberOfFoundItems == m_balls.size()) {
    m_gameData.m_state = State::Win;
    m_game_time.restart();
  }
}