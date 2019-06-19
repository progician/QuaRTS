#pragma once

#include <chrono>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
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
    std::vector<MatchEventsPtr> listeners_;

  public:
    Match(std::initializer_list<std::string> l);

    auto finished() const -> bool;
    auto winner() const -> Player { return *players_.begin(); }

    void resign(std::string const& player_name);

    using PlayerSet = std::unordered_set<Player>;
    auto active_players() const -> PlayerSet { return players_; }

    void listen(MatchEventsPtr listener);
  };


  struct MatchEvents {
    virtual void finished(std::string const&) = 0;
  };


  struct Location {
    float x, y;
  };

  inline auto operator==(
      Location const& lhs, Location const& rhs
  ) noexcept -> bool
  { return lhs.x == rhs.x && lhs.y == rhs.y; }


  class Game {
    using UnitPtr = std::unique_ptr<struct Unit>;
    std::unordered_map<int, UnitPtr> units_;

  public:
    Game();
    ~Game();

    struct UnitRef { int id; };
    
    auto spawn_unit_at(Location) -> UnitRef;
    auto position_of(UnitRef ref) const -> Location;

    void move(UnitRef, Location);

    using Duration = std::chrono::duration<float>;
    void update(Duration);
  };
}