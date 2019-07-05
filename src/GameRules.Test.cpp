#include "GameRules.h"

#include "PlanePrimitives.h"

#include <catch2/catch.hpp>
#include <trompeloeil.hpp>

#include <chrono>
#include <iostream>
#include <memory>


namespace GameRules {
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
using PlanePrimitives::Vector;
using namespace std::chrono_literals;


void UpdateTimes(Game& game, int times) {
  constexpr auto TimeResolution = 1s;
  using std::chrono::seconds;
  for (seconds i = 0s; i < seconds(times); i += TimeResolution) {
    game.update(TimeResolution);
  }
}





TEST_CASE("Given a game with a unit spawned at the origin.", "[GameRules]") {
  auto game = Game{};
  auto const SomeLocation = Location{0, 255};
  auto unit = game.spawn_unit_at(SomeLocation, {});

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
    REQUIRE_THROWS_AS(game.spawn_unit_at({200, 200}, {}), InvalidPosition);
  }

  SECTION("Units will stop at map boundaries") {
    auto unit = game.spawn_unit_at({0, 0}, {});
    game.move(unit, {200, 0});

    UpdateTimes(game, 200);

    REQUIRE(game.position_of(unit) == Location{128, 0});
  }
}


struct GameEventsMock : public Game::GameEvents {
  MAKE_MOCK1(damage, void(Game::UnitRef), override);
  MAKE_MOCK1(casualty, void(Game::UnitRef), override);
};


TEST_CASE("Units can attack each other, and inflict specific damage for every"
          "attack cycles", "[GameRules]") {
  auto game = Game{}; 
  auto victim = game.spawn_unit_at({0, 0}, {});
  auto attacker = game.spawn_unit_at({10, 0}, {});

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
  auto victim = game.spawn_unit_at({5, 5}, {});
  auto attacker = game.spawn_unit_at({100, 5},
      UnitProperties{UnitProperties::Make().attack_radius(20)}
  );

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

  SECTION("with during updates") {
    UpdateTimes(game, 1);
    REQUIRE(game.unit(victim).hit_points() == 8);
  }

  SECTION("and if the hit-points reach 0, the unit will die") {
    auto game_events = std::make_shared<GameEventsMock>();
    game.listen(game_events);

    REQUIRE_CALL(*game_events, damage(victim)).TIMES(4);
    REQUIRE_CALL(*game_events, casualty(victim));

    UpdateTimes(game, 5);
  }
}


class CloseTo : public Catch::MatcherBase<Location> {
  Location location_;
  float epsilon_;
public:
  CloseTo(Location loc, float epsilon = 0.0001f)
      : location_{loc}
      , epsilon_{epsilon} {}
  
  auto match(Location const& rhs) const -> bool override {
    auto const displacement = location_ - rhs;
    return LengthOf(displacement) < epsilon_;
  }

  auto describe() const -> std::string override {
    auto ostr = std::stringstream{};
    ostr << "is within " <<  epsilon_ << " distance of " << location_;
    return ostr.str();
  }
};


TEST_CASE("Units move through the map with a velocity based on their properties") {
  auto game = Game{};
  UnitProperties const unit_props =
      UnitProperties::Make()
          .velocity(2)
          .attack_radius(0.0f)
  ;
  auto const original_position = Location{20, 54};
  auto const unit = game.spawn_unit_at(original_position, unit_props);

  SECTION("when using Move command") {
    game.move(unit, original_position + Vector{8, 6});
    UpdateTimes(game, 5);
    REQUIRE_THAT(game.position_of(unit),
        CloseTo(original_position + Vector{8, 6})
    );
  }

  SECTION("when using Attack command") {
    auto const victim = game.spawn_unit_at(original_position + Vector{16, 12}, {});
    game.attack(unit, victim);
    UpdateTimes(game, 10);
    REQUIRE_THAT(game.position_of(unit),
        CloseTo(original_position + Vector{16, 12})
    );
  }
}


TEST_CASE("Units can have a shape") {
  auto game = Game{256, 128};
  auto const circular_unit = game.spawn_unit_at({12, 21},
      UnitProperties::Make()
          .shape(UnitShape::Circle{10.0f})
  );

  SECTION("which restricts to the movement of the unit") {
    game.move(circular_unit, {12, 1});
    UpdateTimes(game, 20);
    REQUIRE_THAT(game.position_of(circular_unit), CloseTo({12, 10}));
  }
}


TEST_CASE("Units have acceleration") {
  auto game = Game{32, 64};
  auto const unit = game.spawn_unit_at({0, 0},
      UnitProperties::Make()
          .acceleration(0.1f)
          .velocity(10)
  );

  SECTION("which will be applied until they reach their velocity") {
    game.move(unit, {6, 8});
    UpdateTimes(game, 10);
    REQUIRE_THAT(game.position_of(unit), CloseTo({3.3, 4.4}));
  }
}