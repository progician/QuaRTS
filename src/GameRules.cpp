#include "GameRules.h"

#include "Variant.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>
#include <variant>

using namespace PlanePrimitives;

namespace GameRules {
  namespace Commands {
    using Idle = std::monostate;
    struct Move {
      Location loc;
    };

    struct Attack {
      Game::UnitRef target;
    };
  }



  using UnitCommand = std::variant<
      Commands::Idle,
      Commands::Move,
      Commands::Attack
  >;


  struct Unit {
    Location location;
    UnitCommand command{Commands::Idle{}};
    UnitProperties props;
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
    units_.at(ref.id)->command = Commands::Move{location};
  }


  void Game::update(Game::Duration d) {
    for (auto& [id, unit_ptr] : units_) {
      auto& unit = *unit_ptr;
      Variant::Match(unit.command,
          [](Commands::Idle const&) {},

          [&](Commands::Move const& move) {
            auto const direction = Normalized(move.loc - unit.location);
            auto const new_location = unit.location + unit.props.velocity() * direction * d.count();
            auto const& unit_radius = std::get<UnitShape::Circle>(unit.props.shape()).radius;
            auto const available_area = PlanePrimitives::ContractedBy(
                Rectangle{map_dimensions_},
                unit_radius
            );
            unit.location = PlanePrimitives::Clip(available_area, new_location);

            auto const displacement = unit.location - move.loc;
            auto const distance_to_target = LengthOf(displacement);
            if (distance_to_target < 0.0001f) {
              unit.command = Commands::Idle{};
            }
          },

          [&](Commands::Attack const& attack) {
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
              unit.location = unit.location + unit.props.velocity() * direction * d.count();
            }
          }
      );
    }
  }

  
  auto Game::active_command_for(UnitRef ref) const -> Command {
    auto const& unit = *units_.at(ref.id);
    return Variant::Match(unit.command,
        [](Commands::Idle const&) -> Command { return Command::None; },
        [](Commands::Move const&) -> Command { return Command::Move; },
        [](Commands::Attack const&) -> Command { return Command::Attack; }
    );
  }

  
  void Game::attack(UnitRef attacker_ref, UnitRef target_ref) {
    auto& attacker = *units_.at(attacker_ref.id);
    attacker.command = Commands::Attack{target_ref};
  }


  auto Game::unit(UnitRef ref) const -> UnitProperties {
    auto const& unit = *units_.at(ref.id);
    return unit.props;
  }
}