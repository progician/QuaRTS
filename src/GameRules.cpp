#include "GameRules.h"

#include <atomic>
#include <limits>
#include <stdexcept>

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

  auto Game::spawn_unit_at(Location location) -> UnitRef {
    static auto CurrentUnitID = std::atomic_int32_t{0};
    if (CurrentUnitID == std::numeric_limits<int>::max()) {
      throw std::runtime_error("Unit ID overflow, cannot create a new one!");
    }

    auto const ref = UnitRef{CurrentUnitID++};
    units_[ref.id] = location;
    return ref;
  }
}