#include "GameRules.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <optional>
#include <stdexcept>

using namespace PlanePrimitives;

namespace GameRules {
  Match::Match(std::initializer_list<std::string> l) {
    for (auto&& player_name : l) {
      players_.emplace(player_name);
    }
  }


  auto Match::finished() const -> bool {
    return players_.size() == 1;
  }


  void Match::resign(std::string const& player_name) {
    players_.erase(players_.find({player_name}));
    if (players_.size() == 1) {
      for (auto listener : listeners_) {
        listener->finished(players_.begin()->name());
      }
    }
  }


  void Match::listen(MatchEventsPtr ptr) { listeners_.push_back(ptr); }


  struct Unit {
    Location location;
    std::optional<Location> destination;
    explicit Unit(Location l) : location{l} {}
  };


  Game::Game() = default;
  Game::Game(float width, float height) : map_dimensions_{width, height} {}
  Game::~Game() = default;

  auto Game::position_of(UnitRef ref) const -> Location {
    return units_.at(ref.id)->location;
  }

  auto Game::spawn_unit_at(Location location) -> UnitRef {
    static auto CurrentUnitID = std::atomic_int32_t{0};
    if (CurrentUnitID == std::numeric_limits<int>::max()) {
      throw std::runtime_error("Unit ID overflow, cannot create a new one!");
    }

    auto const rect = Rectangle{map_dimensions_};
    if (!Contains(rect, location)) {
      throw InvalidPosition{};
    }

    auto const ref = UnitRef{CurrentUnitID++};
    units_[ref.id] = std::make_unique<Unit>(location);
    return ref;
  }

  void Game::move(UnitRef ref, Location location) {
    units_.at(ref.id)->destination = location;
  }

  void Game::update(Game::Duration d) {
    for (auto& [id, unit_ptr] : units_) {
      auto& unit = *unit_ptr;
      if (!unit.destination || unit.location == unit.destination) {
        continue;
      }

      const auto translation = Location {
        unit.destination->x - unit.location.x,
        unit.destination->y - unit.location.y
      };

      const auto length = std::sqrt(translation.x*translation.x + translation.y*translation.y);
      const float dx = translation.x / length; 
      const float dy = translation.y / length; 
      unit.location.x = std::min(
          unit.location.x + dx * d.count(),
          map_dimensions_.width
      );
      unit.location.y = std::min(
          unit.location.y + dy * d.count(),
          map_dimensions_.height
      );
    }
  }
}