#include "GameRules.h"

namespace GameRules {
  Match::Match(std::initializer_list<std::string> l) {
    for (auto&& player_name : l) {
      players_.emplace(player_name);
    }
  }

  auto Match::finished() const -> bool {
    return players_.size() == 1;
  }
}