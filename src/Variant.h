#pragma once

#include <utility>
#include <variant>

namespace Variant {
  namespace detail {
    template<class... Ts> struct overloaded : Ts... {
      using Ts::operator()...;
    };

    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
  }

  template<typename Variant, typename... Matchers>
  auto Match(Variant&& variant, Matchers&&... matchers) {
      return std::visit(
          detail::overloaded{std::forward<Matchers>(matchers)...},
          std::forward<Variant>(variant)
      );
  }
}