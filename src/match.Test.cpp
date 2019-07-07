#include "match.h"


#include <catch2/catch.hpp>
#include <trompeloeil.hpp>


namespace match {
  std::ostream& operator <<(std::ostream& lhs, Player const& rhs) {
    lhs << rhs.name();
    return lhs;
  }
}


using namespace match;


TEST_CASE("A Match created with a single player is automatically finished") {
  static auto const SingularPlayer = std::string{"Some Named Player"};
  auto match = Match{SingularPlayer};
  REQUIRE(match.finished());

  SECTION("and the winner is the singular player") {
    REQUIRE(match.winner().name() == SingularPlayer);
  }
}


TEST_CASE("A Match is ongoing if there is at least two players") {
  auto match = Match{"X", "Y"};
  REQUIRE_FALSE(match.finished());
}


TEST_CASE("Match maintains the set of active players") {
  auto match = Match{"X", "Y", "Z"};

  SECTION("which is by default all the players") {
    REQUIRE(match.active_players() == Match::PlayerSet{{"X"}, {"Y"}, {"Z"}});
  }

  SECTION("if a player is resign, no longer will be considered active") {
    match.resign("X");
    REQUIRE(match.active_players() == Match::PlayerSet{{"Y"}, {"Z"}});
  }
}


using namespace Catch::Matchers;
class WonBy : public Catch::MatcherBase<Match> {
  std::string const name_;
public:
  WonBy(std::string name) : name_(std::move(name)) {}

  auto match(Match const& rhs) const -> bool override {
    return rhs.finished() && rhs.winner().name() == name_;
  }

  auto describe() const -> std::string override {
    std::ostringstream ostr;
    ostr << "won by " << name_;
    return ostr.str();
  }
};

TEST_CASE("When the all but one player resigns, the game finishes") {
  auto match = Match{"X", "Y"};
  match.resign("X");

  REQUIRE_THAT(match, WonBy("Y"));
}


struct EventsMock : public Events {
  MAKE_MOCK1(finished, void(std::string const&), override);
};


TEST_CASE("when a match comes to a conclusion, all listeners will be notified") {
  auto match_events = std::make_shared<EventsMock>();
  auto match = Match{"X", "Y"};

  constexpr auto ArbitraryNumberOfListeners = 7;
  for (auto i = 0; i < ArbitraryNumberOfListeners; ++i) {
    match.listen(match_events);
  }

  REQUIRE_CALL(*match_events, finished(std::string{"Y"}))
      .TIMES(ArbitraryNumberOfListeners)
  ;

  match.resign("X");
}