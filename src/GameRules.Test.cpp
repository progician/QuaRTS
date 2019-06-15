#include "GameRules.h"

#include <catch2/catch.hpp>
#include <iostream>


namespace GameRules {
  std::ostream& operator <<(std::ostream& lhs, Player const& rhs) {
    lhs << rhs.name();
    return lhs;
  }
}



TEST_CASE("A Match created with a single player is automatically finished") {
  using namespace GameRules;
  static auto const SingularPlayer = std::string{"Some Named Player"};
  auto match = Match{SingularPlayer};
  REQUIRE(match.finished());

  SECTION("and the winner is the singular player") {
    REQUIRE(match.winner().name() == SingularPlayer);
  }
}


TEST_CASE("A Match is ongoing if there is at least two players") {
  using namespace GameRules;
  auto match = Match{"X", "Y"};
  REQUIRE_FALSE(match.finished());
}


TEST_CASE("Match maintains the set of active players") {
  using namespace GameRules;
  auto match = Match{"X", "Y", "Z"};

  SECTION("which is by default all the players") {
    REQUIRE(match.active_players() == Match::PlayerSet{{"X"}, {"Y"}, {"Z"}});
  }

  SECTION("if a player is resign, no longer will be considered active") {
    match.resign("X");
    REQUIRE(match.active_players() == Match::PlayerSet{{"Y"}, {"Z"}});
  }
}


TEST_CASE("When the all but one player resigns, the game finishes") {
  using namespace GameRules;
  auto match = Match{"X", "Y"};
  match.resign("X");

  REQUIRE(match.finished());

  SECTION("and the winner is the remaining player") {
    REQUIRE(match.winner().name() == "Y");
  }
}