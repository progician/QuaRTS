#include "GameRules.h"

#include "PlanePrimitives.h"

#include <catch2/catch.hpp>
#include <trompeloeil.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>


namespace GameRules {
  std::ostream& operator <<(std::ostream& lhs, Player const& rhs) {
    lhs << rhs.name();
    return lhs;
  }

  auto operator <<(std::ostream& lhs, Command rhs) -> std::ostream& {
    switch(rhs) {
      case Command::None: lhs <<  "None"; break;
      case Command::Move: lhs << "Move"; break;
      case Command::Attack: lhs << "Attack"; break;
    }

    return lhs;
  }
}

namespace PlanePrimitives {
  std::ostream& operator <<(std::ostream& lhs, Location const& rhs) {
    lhs << "{" << rhs.x << "," << rhs.y << "}";
    return lhs;
  }
}


using namespace GameRules;
using PlanePrimitives::Location;
using namespace std::chrono_literals;


void UpdateTimes(Game& game, int times) {
  constexpr auto TimeResolution = 1s;
  using std::chrono::seconds;
  for (seconds i = 0s; i < seconds(times); i += TimeResolution) {
    game.update(TimeResolution);
  }
}


TEST_CASE("A Match created with a single player is automatically finished", "[GameRules]") {
  static auto const SingularPlayer = std::string{"Some Named Player"};
  auto match = Match{SingularPlayer};
  REQUIRE(match.finished());

  SECTION("and the winner is the singular player") {
    REQUIRE(match.winner().name() == SingularPlayer);
  }
}


TEST_CASE("A Match is ongoing if there is at least two players", "[GameRules]") {
  auto match = Match{"X", "Y"};
  REQUIRE_FALSE(match.finished());
}


TEST_CASE("Match maintains the set of active players", "[GameRules]") {
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
  WonBy(std::string name) : name_{name} {}

  bool match(Match const& rhs) const override {
    return rhs.finished() && rhs.winner().name() == name_;
  }

  std::string describe() const override {
    std::ostringstream ostr;
    ostr << "won by " << name_;
    return ostr.str();
  }
};

TEST_CASE("When the all but one player resigns, the game finishes", "[GameRules]") {
  auto match = Match{"X", "Y"};
  match.resign("X");

  REQUIRE_THAT(match, WonBy("Y"));
}


struct MatchEventsMock : public MatchEvents {
  MAKE_MOCK1(finished, void(std::string const&), override);
};


TEST_CASE("when a match comes to a conclusion, all listeners will be notified", "[GameRules]") {
  auto match_events = std::make_shared<MatchEventsMock>();
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


TEST_CASE("Given a game with a unit spawned at the origin.", "[GameRules]") {
  auto game = Game{};
  auto const SomeLocation = Location{0, 255};
  auto unit = game.spawn_unit_at(SomeLocation);

  SECTION("unit is at position of its origin") {
    REQUIRE(game.position_of(unit) == SomeLocation);
  }

  SECTION("has no command at its pristine state") {
    REQUIRE(game.active_command_for(unit) == Command::None);
  }

  SECTION("the 'move' command will not affect position of the unit") {
    game.move(unit, {0, 0});
    REQUIRE(game.position_of(unit) == SomeLocation);

    SECTION("until the simulation steps were called") {
      UpdateTimes(game, 255);
      REQUIRE(game.position_of(unit) == Location{0, 0});
    }
  }

  SECTION("when a unit gets updated more than it requires to reach its destination") {
    auto const Destination = Location{0, 128};
    game.move(unit, Destination);

    UpdateTimes(game, 255);

    SECTION("then it stays at its destination") {
      REQUIRE(game.position_of(unit) == Destination);
    }

    SECTION("then it is no longer have an active move command") {
      REQUIRE(game.active_command_for(unit) != Command::Move);
    }
  }
}


TEST_CASE("Units are limited to the map's dimensions", "[GameRules]") {
  auto game = Game{128, 128};

  SECTION("cannot spawn unit outside of the boundary") {
    REQUIRE_THROWS_AS(game.spawn_unit_at({200, 200}), InvalidPosition);
  }

  SECTION("Units will stop at map boundaries") {
    auto unit = game.spawn_unit_at({0, 0});
    game.move(unit, {200, 0});

    UpdateTimes(game, 200);

    REQUIRE(game.position_of(unit) == Location{128, 0});
  }
}


struct GameEventsMock : public Game::GameEvents {
  MAKE_MOCK1(damage, void(Game::UnitRef), override);
};


TEST_CASE("Units can attack each other, and inflict specific damage for every"
          "attack cycles", "[GameRules]") {
  auto game = Game{}; 
  auto victim = game.spawn_unit_at({0, 0});
  auto attacker = game.spawn_unit_at({10, 0});

  game.attack(attacker, victim);

  SECTION("The attacker is has the active command of attack") {
    REQUIRE(game.active_command_for(attacker) == Command::Attack);
  }

  SECTION("Notification of damage at simulation cycles") {
    auto game_events = std::make_shared<GameEventsMock>();
    game.listen(game_events);
    REQUIRE_CALL(*game_events, damage(victim));

    UpdateTimes(game, 1);
  }
}


TEST_CASE("In a game units have an attack radius", "[GameRules]") {
  using namespace trompeloeil;

  auto game = Game{}; 
  auto victim = game.spawn_unit_at({5, 5});
  auto attacker = game.spawn_unit_at({100, 5}, 20);

  game.attack(attacker, victim);

  SECTION("outside of which the unit cannot inflict damage to the target") {
    auto game_events = std::make_shared<GameEventsMock>();
    game.listen(game_events);

    FORBID_CALL(*game_events, damage(_));

    UpdateTimes(game, 1);
  }

  SECTION("outside of which that attacker will move toward its target") {
    UpdateTimes(game, 1);
    REQUIRE(game.position_of(attacker) == Location{99, 5});
  }

  SECTION("when reaches the proximity of the target within this radius, stops and causes damage") {
    auto game_events = std::make_shared<GameEventsMock>();
    game.listen(game_events);
    REQUIRE_CALL(*game_events, damage(victim)).TIMES(25);

    UpdateTimes(game, 100);

    REQUIRE(game.position_of(attacker) == Location{25, 5});
  }
}


TEST_CASE("A unit under attack looses HP (hit points)", "[GameRules]") {
  auto game = Game{}; 
  UnitProperties const victim_props = UnitProperties::Make().hit_points(10);
  auto victim = game.spawn_unit_at({5, 5}, victim_props);

  UnitProperties const attacker_props = UnitProperties::Make().attack_damage(2);
  auto attacker = game.spawn_unit_at({100, 5}, attacker_props);
  game.attack(attacker, victim);

  UpdateTimes(game, 1);

  REQUIRE(game.unit(victim).hit_points() == 8);
}
