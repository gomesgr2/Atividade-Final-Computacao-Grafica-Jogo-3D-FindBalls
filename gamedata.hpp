#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class State { Playing, GameOver, Win, Init, Menu };

struct GameData {
  State m_state{State::Init};
};

#endif