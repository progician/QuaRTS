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
  static auto const SingularPlayer = Player{"Some Named Player"};
  auto match = Match{SingularPlayer};
  REQUIRE(match.finished());

  SECTION("and the winner is the singular player") {
    REQUIRE(match.winner() == SingularPlayer);
  }
}