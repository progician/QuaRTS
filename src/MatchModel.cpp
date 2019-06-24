
#include "MatchModel.h"

#include <initializer_list>
#include <string>


namespace MatchModel {
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


  void Match::listen(EventsPtr ptr) { listeners_.push_back(ptr); }
}