# Atividade-Final-Computacao-Grafica-Jogo-3D-FindBalls
# Jogo 3d FindBalls - Computação Grafica

### Principais Implementações 

### gamedata.hpp :

Estrutura que define o estado atual do jogo

```
#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class State { Playing, GameOver, Win, Init, Menu };

struct GameData {
  State m_state{State::Init};
  };

#endif 
```

m_state pode ser:

    Playing : aplicação em modo jogo
    GameOver : aplicação na tela de finalização do jogo com a mensagem "Você Perdeu" e botão para jogar novamente, neste caso a pessoa e as bolas não são exibidas.
    Win : aplicação na tela de finalização do jogo com a mensagem "Você Ganhou" e botão para jogar novamente, neste caso a pessoa e as bolas não são exibidas.
    Init : aplicação na tela de inicialização aparecendo botão com a mensagem "Jogar".
    Menu : aplicação de escolha da dificuldade do jogo com os botões : Fácil, Médio e Difícil.
    
    

### openglwindow.hpp

```
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
  bool checkFoundDistance(float position_x, float position_z, float position2_x, float position2_z);
};

#endif
#endif

```


BallPosition - define as propriedades das bolas que terão uma posição x e z aleatórias (entre -2 e 2) e uma terceira propriedade que define se o bola foi encontrada pelo usuário.

```
struct BallPosition {
  float position_x;
  float position_z;
  bool wasFound;
};

```

m_balls - lista de BallPosition com as bolas a serem encontradas


m_wrong_balls - lista de BallPosition com as bolas que você deve fugir

```
 std::list<BallPosition> m_balls;
 std::list<BallPosition> m_wrong_balls;
```

numberOfFoundItems - variável que determina o número de bolas encontradas pelo usuário 
```
int numberOfFoundItems = 0;
```
Instâncias da classe Model, que irão construir a bola e casa. 

```
 Model m_ball_model;
 Model m_house_model;
```




### openglwindow.cpp

**OpenGLWindow::initializeGL**

```
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
```
**OpenGLWindow::initBalls**

Primeiramente removemos os atributos da lista de bolas (m_balls), em seguida atribuimos o tamanho da lista de acordo com a quantidade de bolas (essa quantidade será escolhida pelo usuário), por fim populamos a lista, nessa parte adicionamos nos atribuitos position_x e position_z valores aleatórios entre -2 e 2 e adicionamos no atributo wasFound o valor false, tendo em vista que no inicio nenhuma bola foi encontrada.
Em seguida, criamos as bolas que não poderão ser encontradas (note que definimos o tamanho da lista m_wrong_balls de acordo com a quantidade de bolas - 2), o processo é bem parecido com a criação da m_balls, no entanto, checamos se a distancia entre os valores gerados para position da "wrong ball" possuem uma distância inferior ao valor 0.8f de qualquer bola presente na m_balls, caso satisfaça essa condição geramos novavemente os atributos position_x e position_z, fazemos isso para não termos uma "wrong ball" colada com uma bola que precise ser encontrada, pois assim não teriamos como ganhar o jogo. 


```
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
    float position_x = m_randomDist(re);
    float position_z = m_randomDist(re);
    for (auto& ball : m_balls) {
      while (checkFoundDistance(position_x, position_z, ball.position_x,
                                ball.position_z)) {
        position_x = m_randomDist(re);
        position_z = m_randomDist(re);
      }
    }
    wrong_ball.position_x = position_x;
    wrong_ball.position_z = position_z;
    wrong_ball.wasFound = false;
  }
}


```

**OpenGLWindow::paintGL**

```
void OpenGLWindow::paintGL() {
  update();

  // Clear color buffer and depth buffer
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  abcg::glUseProgram(m_program);
  paintModels();

  abcg::glUseProgram(0);
}
```
**OpenGLWindow::paintModels**

```
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

```

Criação das bolas, de acordo com a lista m_balls, nesse processo é atribuído a posição inicial das bolas de acordo com os valores definidos na função initBalls(), além disso, caso o atributo wasFound for true a cor da bola muda para azul se não teremos a cor em vermelho. E as "wrong balls" são criadas na cor amarela.

```
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

```




Criação da casa :
```
  glm::mat4 house_model{1.0f};
  house_model = glm::translate(house_model, glm::vec3(0.5f, 0.0f, 5.0f));
  house_model =
      glm::rotate(house_model, glm::radians(90.0f), glm::vec3(0, 1, 0));
  house_model = glm::scale(house_model, glm::vec3(5.0f));
  glUniform4fv(KaLoc, 1, &m_Ka.x);
  glUniform4fv(KdLoc, 1, &m_Kd.x);

  m_house_model.render(-1);

```

**OpenGLWindow::PaintUI**

```
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
    ImGui::Text(
        "VOCÊ TEM 20 SEGUNDOS PARA ENCONTRAR TODAS AS BOLAS VERMELHAS, \n FUJA "
        "DAS BOLAS AMARELAS !!!!!");
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
```

**Na função paintUI definimos alguns processos importantes** :

-  m_gamedata.m_state igual a Init :
      -  Tela com informações do jogo e botão que leva para o menu :
```             
ImGui::Text("VOCÊ TEM 20 SEGUNDOS PARA ENCONTRAR TODAS AS BOLAS");
             if (ImGui::Button("Jogar", ImVec2(300, 80))) {
                 initBalls(5);
                 m_gameData.m_state = State::Menu;
            }
```

- m_gamedata.m_state igual a Menu :
       Tela com 3 botões que definirá a dificuldade do jogo, isto é, qual o número de bolas que precisarão ser adicionadas na lista m_balls, definido assim a quantidade de bolas renderizadas :
```
if (ImGui::Button("FÁCIL - 3 BOLAS", ImVec2(300, 80))) {
      m_gameData.m_state = State::Playing;
      initBalls(3);
    }
    if (ImGui::Button("MÉDIO - 4 BOLAS", ImVec2(300, 80))) {
      m_gameData.m_state = State::Playing;
      initBalls(4);
    }
    if (ImGui::Button("DIFÍCIL- 6 BOLAS ", ImVec2(300, 80))) {
      m_gameData.m_state = State::Playing;
      initBalls(6);
    }

```
- m_gamedata.m_state igual a Playing :
   - Mostrará o número de bolas que foram encontradas até o momento :
```
  std::string text = std::to_string(numberOfFoundItems);
    char const* textFormat = text.c_str();
    ImGui::Text("NÚMERO DE BOLAS ENCONTRADAS :");
    ImGui::Text(textFormat);
```
- m_gamedata.m_state igual a GameOver ou Win :
  - Tela que mostrará se o usuário ganhou ou perdeu o jogo e botão de jogar novamente que além de mudar a variável m_gamedata para Menu, reinicializa a variável numberOfFoundItems :
```
 const char* message =
        m_gameData.m_state == State::GameOver ? "Você Perdeu" : "Você Ganhou";
    ImGui::Text(message);
    if (ImGui::Button("Jogar Novamente", ImVec2(300, 80))) {
      numberOfFoundItems = 0;
      m_gameData.m_state = State::Menu;
    }

```
**OpenGLWindow::checkFoundDistance**

Função que compara as posições dos atributos passados por parâmetro e retorna se essa distância é inferior ou não a 0.8f. (Já vimos ela na criação da m_wrong_balls) 

```

bool OpenGLWindow::checkFoundDistance(float position_x, float position_z,
                                      float position2_x, float position2_z) {
  const auto distance{glm::distance(glm::vec3(position_x, 0, position_z),
                                    glm::vec3(position2_x, 0, position2_z))};

  return distance < 0.8f;
}

```

**OpenGLWindow::checkFound** 

Essa função verifica se algumas das bolas foi encontrada, para isso é realizado uma interação na lista m_balls e posteriormente checamos se a bola ainda não foi encontrada, caso o atributo wasFound for igual a false, então chamamos a função checkFoundDistance passando a posição da bola e da câmera, caso essa distância for menor que 0.8f, atribuímos o valor do wasFound como true e modificamos o contador de items encontrados (váriavel numberOfFoundItems).
Além disso, verificamos também se a alguma "wrong ball" foi encontrada, caso o usuário encontre a "wrong ball" chamamos a função removeBallInFoundItems que modifica o contador de items encontrados para 0 e intera a lista m_balls modificando os atributos wasFound que são true para false.

```
void OpenGLWindow::checkFound() {
  for (auto& ball : m_balls) {
    if (!ball.wasFound) {
      bool found = checkFoundDistance(ball.position_x, ball.position_z,
                                      m_camera.m_eye.x, m_camera.m_eye.z);
      if (found) {
        ball.wasFound = true;
        numberOfFoundItems++;
      }
    }
  }

  for (auto& wrong_ball : m_wrong_balls) {
    bool found =
        checkFoundDistance(wrong_ball.position_x, wrong_ball.position_z,
                           m_camera.m_eye.x, m_camera.m_eye.z);
    if (found) {
      wrong_ball.wasFound = true;
      removeBallInFoundItems();
    }
  }
}
```

```
void OpenGLWindow::removeBallInFoundItems() {
  for (auto& ball : m_balls) {
    if (ball.wasFound) ball.wasFound = false;
  }
  numberOfFoundItems = 0;
}
``

**OpenGLWindow::checkGameCondition**

Essa função verifica as condições do jogo, assim temos que caso a variável numberOfFoundItems (contador de bolas encontradas) for igual ao tamanho da lista m_ball, significa que todas as bolas foram encontradas pelo usuário, então mudamos o m_gameData.m_state para Win e reiniciamos o tempo. Caso o tempo de jogo ultrapasse os 20 seg e o usuário não encontrou todas as bolas, mudamos o m_gameData.m_state para GameOver e reiniciamos o tempo.

```
void OpenGLWindow::checkGameCondition() {
  if (m_game_time.elapsed() > 20) {
    m_gameData.m_state = State::GameOver;
    m_game_time.restart();
  }

  if (numberOfFoundItems == m_balls.size()) {
    m_gameData.m_state = State::Win;
    m_game_time.restart();
  }
}
```

### Model e Câmera
Essas classes foram criadas com base nas aulas 7, 9 e 10, assim usamos o código apresentado em aula. 
