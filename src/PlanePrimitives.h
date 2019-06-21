#pragma once

#include <algorithm>
#include <cmath>

namespace PlanePrimitives {
  struct Location {
    float x;
    float y;
  };

  inline auto operator==(
      Location const& lhs, Location const& rhs
  ) noexcept -> bool
  { return lhs.x == rhs.x && lhs.y == rhs.y; }


  struct Vector {
    float x;
    float y;

    Vector(float px, float py) noexcept : x{px}, y{py} {}
    Vector(Location const& loc) noexcept : x{loc.x}, y{loc.y} {}
    Vector(Vector const&) noexcept = default;
    Vector(Vector&&) = default;
  };


  inline auto operator -(Vector const& lhs, Vector const& rhs) noexcept -> Vector {
    return {
      lhs.x - rhs.x,
      lhs.y - rhs.y
    };
  }


  inline auto operator +(Location const& lhs, Vector const& rhs) noexcept -> Location {
    return {
      lhs.x + rhs.x,
      lhs.y + rhs.y
    };
  }


  inline auto LengthOf(Vector const& v) noexcept -> float {
    return std::sqrt(v.x * v.x + v.y * v.y);
  }


  inline auto operator /(Vector const& lhs, float rhs) noexcept -> Vector {
    return {
      lhs.x / rhs,
      lhs.y / rhs
    };
  }


  inline auto operator *(Vector const& lhs, float rhs) noexcept -> Vector {
    return {
      lhs.x * rhs,
      lhs.y * rhs
    };
  }


  inline auto operator *(float lhs, Vector const& rhs) noexcept -> Vector {
    return {
      lhs * rhs.x,
      lhs * rhs.y
    };
  }


  inline auto Normalized(Vector const& v) -> Vector {
    return v / LengthOf(v);
  }


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
    , right{r}
    , top{t}
    , bottom{b} {}

    Rectangle(Size const& sz)
    : left{0}
    , right{sz.width}
    , top{0}
    , bottom{sz.height} {}

    Rectangle(Location const& loc, Size const& sz)
    : left{loc.x}
    , right{loc.x + sz.width}
    , top{loc.y}
    , bottom{loc.y + sz.height} {}
  };

  inline auto Contains(Rectangle const& rect, Location const& loc) noexcept -> bool {
    if (loc.x < rect.left || loc.x > rect.right)
      return false;
    if (loc.y < rect.top || loc.y > rect.bottom)
      return false;
    return true;
  }


  inline auto Clip(Rectangle const& rect, Location const& loc) noexcept -> Location {
    return Location{
      std::min(rect.right, std::max(rect.left, loc.x)),
      std::min(rect.bottom, std::max(rect.top, loc.y)),
    };
  }
}