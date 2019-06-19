#pragma once

namespace PlanePrimitives {
  struct Location {
    float x, y;
  };

  inline auto operator==(
      Location const& lhs, Location const& rhs
  ) noexcept -> bool
  { return lhs.x == rhs.x && lhs.y == rhs.y; }


  struct Size {
    float width;
    float height;

    Size(float w, float h) : width(w), height(h) {}
  };

  struct Rectangle {
    float left;
    float right;
    float top;
    float bottom;

    Rectangle(float l, float t, float r, float b)
    : left{l}
    , right(r)
    , top(t)
    , bottom(b) {}

    Rectangle(Size const& sz)
    : left{0}
    , right(sz.width)
    , top(0)
    , bottom(sz.height) {}

    Rectangle(Location const& loc, Size const& sz)
    : left{loc.x}
    , right(loc.x + sz.width)
    , top(loc.y)
    , bottom(loc.y + sz.height) {}
  };

  inline auto Contains(Rectangle const& rect, Location const& loc) -> bool {
    if (loc.x < rect.left || loc.x > rect.right)
      return false;
    if (loc.y < rect.top || loc.y > rect.bottom)
      return false;
    return true;
  }
}