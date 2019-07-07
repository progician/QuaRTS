#include "geometry.h"

#include <catch2/catch.hpp>

using namespace geometry;

TEST_CASE("Contracting reduces the area of a rectangle, preserving the shape and center") {
  auto const original = Rectangle{10, 10, 100, 100};
  auto const contracted = ContractedBy(original, 10);
  REQUIRE(CenterOf(contracted) == CenterOf(original));
}