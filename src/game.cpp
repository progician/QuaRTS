#include "game.h"
#include "geometry.h"

#include "variant.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>
#include <variant>

using namespace geometry;

namespace game {
  namespace commands {
    using Idle = std::monostate;
    struct Move {
      Location loc;
    };

    struct Attack {
      Game::UnitRef target;
    };
  }



  using UnitCommand = std::variant<
      commands::Idle,
      commands::Move,
      commands::Attack
  >;


  struct Unit {
    Location location;
    UnitCommand command{commands::Idle{}};
    UnitProperties props;
    float velocity_{0.0f};
    Unit(Location l, UnitProperties p) : location{l}, props{p} {}

    void take_damage(int v) {
      props.hit_points_ -= v;
    }
  };


  Game::Game() = default;
  Game::Game(float width, float height) : map_dimensions_{width, height} {}
  Game::~Game() = default;

  auto Game::position_of(UnitRef ref) const -> Location {
    return units_.at(ref.id)->location;
  }

  auto Game::spawn_unit_at(Location location, UnitProperties const& props) -> UnitRef {
    static auto CurrentUnitID = std::atomic_int32_t{0};
    if (CurrentUnitID == std::numeric_limits<int>::max()) {
      throw std::runtime_error("Unit ID overflow, cannot create a new one!");
    }

    auto const rect = Rectangle{map_dimensions_};
    if (!Contains(rect, location)) {
      throw InvalidPosition{};
    }

    auto const ref = UnitRef{CurrentUnitID++};
    units_[ref.id] = std::make_unique<Unit>(location, props);
    return ref;
  }


  void Game::move(UnitRef ref, Location location) {
    units_.at(ref.id)->command = commands::Move{location};
  }


  void Game::be_idle(Unit&) {}

  void Game::do_move(Unit& unit, commands::Move const& move) {
    auto const direction = Normalized(move.loc - unit.location);
    unit.velocity_ = std::min(
        unit.velocity_ + unit.props.acceleration(),
        unit.props.velocity()
    );
    auto const new_location = unit.location + unit.velocity_ * direction;
    auto const unit_radius = std::get<UnitShape::Circle>(unit.props.shape()).radius;
    auto const available_area = geometry::ContractedBy(
        Rectangle{map_dimensions_},
        unit_radius
    );
    unit.location = geometry::Clip(available_area, new_location);

    auto const displacement = unit.location - move.loc;
    auto const distance_to_target = LengthOf(displacement);
    if (distance_to_target < 0.0001f) {
      unit.command = commands::Idle{};
    }
  }


  void Game::do_attack(Unit& unit, commands::Attack const& attack) {
    auto& target = *units_.at(attack.target.id);
    auto const distance = LengthOf(target.location - unit.location);
    if (distance <= unit.props.attack_radius()) {
      target.take_damage(unit.props.attack_damage());
      if (target.props.hit_points() <= 0) {
        if (listener_) {
          listener_->casualty(attack.target);
          units_.erase(attack.target.id);
        }
      }
      else if (listener_) {
        listener_->damage(attack.target);
      }
    }
    else {
      auto const direction = Normalized(target.location - unit.location);
      unit.velocity_ = std::min(
          unit.velocity_ + unit.props.acceleration(),
          unit.props.velocity()
      );
      unit.location = unit.location + unit.velocity_ * direction;
    }
  }


  void Game::update() {
    for (auto& [id, unit_ptr] : units_) {
      auto& unit = *unit_ptr;
      variant::Match(unit.command,
          [this, &unit](commands::Idle const& idle) { be_idle(unit); },
          [this, &unit](commands::Move const& move) { do_move(unit, move); },
          [this, &unit](commands::Attack const& attack) { do_attack(unit, attack); }
      );
    }
  }

  
  auto Game::active_command_for(UnitRef ref) const -> Command {
    auto const& unit = *units_.at(ref.id);
    return variant::Match(unit.command,
        [](commands::Idle const&) -> Command { return Command::None; },
        [](commands::Move const&) -> Command { return Command::Move; },
        [](commands::Attack const&) -> Command { return Command::Attack; }
    );
  }

  
  void Game::attack(UnitRef attacker_ref, UnitRef target_ref) {
    auto& attacker = *units_.at(attacker_ref.id);
    attacker.command = commands::Attack{target_ref};
  }


  auto Game::unit(UnitRef ref) const -> UnitProperties {
    auto const& unit = *units_.at(ref.id);
    return unit.props;
  }
}