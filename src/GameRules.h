#pragma once

#include <unordered_set>
#include <string>
#include <vector>

namespace GameRules {
  class Player {
    std::string const name_;
  public:
    Player(std::string const& name) : name_{name} {}

    auto name() const { return name_; }
    auto operator==(Player const& rhs) const { return name_ == rhs.name_; }
  };
}

namespace std {
  template<> struct hash<GameRules::Player> {
    std::size_t operator()(GameRules::Player const& player) const {
      return std::hash<std::string>{}(player.name());
    }
  };
}

namespace GameRules {
  class Match {
  public:
    using MatchEventsPtr = std::shared_ptr<struct MatchEvents>;
  private:
    std::unordered_set<Player> players_;
    MatchEventsPtr listener_;

  public:
    Match(std::initializer_list<std::string> l);

    auto finished() const -> bool;
    auto winner() const -> Player { return *players_.begin(); }

    void resign(std::string const& player_name);

    using PlayerSet = std::unordered_set<Player>;
    auto active_players() const -> PlayerSet { return players_; }

    void listen(MatchEventsPtr listener) { listener_ = listener; }
  };


  struct MatchEvents {
    virtual void finished(std::string const&) = 0;
  };
}