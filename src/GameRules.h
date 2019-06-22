#pragma once

#include "PlanePrimitives.h"

#include <chrono>
#include <initializer_list>
#include <limits>
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


  class InvalidPosition : public std::runtime_error {
  public:
    InvalidPosition() : std::runtime_error("Position is out of bounds!") {}
  };


  enum class Command {
    None,
    Move,
    Attack,
  };

  
  class Game {
  public:
    struct UnitRef { int id; };
    
    struct GameEvents {
      virtual void damage(UnitRef) = 0;
    };
    using GameEventsPtr = std::shared_ptr<GameEvents>;

  private:
    using UnitPtr = std::unique_ptr<struct Unit>;
    std::unordered_map<int, UnitPtr> units_;

    PlanePrimitives::Size const map_dimensions_{
      std::numeric_limits<float>::infinity(),
      std::numeric_limits<float>::infinity(),
    };

    GameEventsPtr listener_;

  public:
    Game();
    Game(float, float);
    ~Game();

    auto spawn_unit_at(PlanePrimitives::Location) -> UnitRef;
    auto position_of(UnitRef ref) const -> PlanePrimitives::Location;
    auto active_command_for(UnitRef ref) const -> Command;

    void move(UnitRef, PlanePrimitives::Location);
    void attack(UnitRef, UnitRef);

    using Duration = std::chrono::duration<float>;
    void update(Duration);

    void listen(GameEventsPtr l) { listener_ = l; }
  };

  inline auto operator ==(Game::UnitRef lhs, Game::UnitRef rhs) noexcept -> bool {
    return lhs.id == rhs.id;
  }
}