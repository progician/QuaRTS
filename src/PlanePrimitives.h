#pragma once

namespace PlanePrimitives {
  struct Location {
    float x, y;
  };

  inline auto operator==(
      Location const& lhs, Location const& rhs
  ) noexcept -> bool
  { return lhs.x == rhs.x && lhs.y == rhs.y; }
}