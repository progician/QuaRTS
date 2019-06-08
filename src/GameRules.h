#pragma once

#include <string>

namespace GameRules {
  class Player {
    std::string const name_;
  public:
    Player(std::string const& name) : name_{name} {}

    auto name() const { return name_; }
    auto operator==(Player const& rhs) const { return name_ == rhs.name_; }
  };


  class Match {
    Player player_;
  public:
    Match(Player player) : player_{player} {}

    auto finished() const -> bool { return true; }
    auto winner() const -> Player { return player_; }
  };
}