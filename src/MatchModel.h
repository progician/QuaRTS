#pragma once

#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>


namespace MatchModel {
  class Player {
    std::string const name_;
  public:
    Player(std::string const& name) : name_{name} {}

    auto name() const { return name_; }
    auto operator==(Player const& rhs) const { return name_ == rhs.name_; }
  };
}


namespace std {
  template<> struct hash<MatchModel::Player> {
    std::size_t operator()(MatchModel::Player const& player) const {
      return std::hash<std::string>{}(player.name());
    }
  };
}


namespace MatchModel {
  using EventsPtr = std::shared_ptr<struct Events>;

  class Match {
    std::unordered_set<Player> players_;
    std::vector<EventsPtr> listeners_;

  public:
    Match(std::initializer_list<std::string> l);

    auto finished() const -> bool;
    auto winner() const -> Player { return *players_.begin(); }

    void resign(std::string const& player_name);

    using PlayerSet = std::unordered_set<Player>;
    auto active_players() const -> PlayerSet { return players_; }

    void listen(EventsPtr listener);
  };


  struct Events {
    virtual void finished(std::string const&) = 0;
  };
}