#pragma once

#include "PlanePrimitives.h"

#include <chrono>
#include <limits>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace GameRules {
  class InvalidPosition : public std::runtime_error {
  public:
    InvalidPosition() : std::runtime_error("Position is out of bounds!") {}
  };


  enum class Command {
    None,
    Move,
    Attack,
  };


  class Unit;
  class UnitPropertiesBuilder;

  class UnitProperties {
    int hit_points_{std::numeric_limits<int>::max()};
    float attack_radius_{std::numeric_limits<float>::infinity()};
    int attack_damage_{0};

  public:
    friend class Unit;
    friend class UnitPropertiesBuilder;

    auto hit_points() const -> int { return hit_points_; }
    auto attack_radius() const -> float { return attack_radius_; }
    auto attack_damage() const -> float { return attack_damage_; }

    static auto Make() -> UnitPropertiesBuilder;
  };

  
  class UnitPropertiesBuilder {
    UnitProperties props;

  public:
    using ThisType = UnitPropertiesBuilder;

    auto hit_points(int value) -> ThisType& {
      props.hit_points_ = value;
      return *this;
    }

    auto attack_radius(float value) -> ThisType& {
      props.attack_radius_ = value;
      return *this;
    }

    auto attack_damage(int value) -> ThisType& {
      props.attack_damage_ = value;
      return *this;
    }

    operator UnitProperties&&() { return std::move(props); }
  };


  inline auto UnitProperties::Make() -> UnitPropertiesBuilder {
    return UnitPropertiesBuilder{};
  }


  class Game {
  public:
    struct UnitRef { int id; };
    
    struct GameEvents {
      virtual void damage(UnitRef) = 0;
      virtual void casualty(UnitRef) = 0;
    };
    using GameEventsPtr = std::shared_ptr<GameEvents>;

  private:
    using UnitPtr = std::unique_ptr<Unit>;
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

    auto spawn_unit_at(PlanePrimitives::Location, UnitProperties const&) -> UnitRef;
    auto position_of(UnitRef ref) const -> PlanePrimitives::Location;
    auto unit(UnitRef ref) const -> UnitProperties;
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