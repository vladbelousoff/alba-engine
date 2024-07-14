#pragma once

#include <gtest/gtest.h>
#include <type_traits>

#include "string_manager.h"

namespace loki {

  using ComponentID = StringID;

  template <typename Self> struct ComponentWrapper
  {
    explicit ComponentWrapper()
    {
      static_assert(std::is_trivially_copyable<Self>::value);
    }

    static auto get_id() -> ComponentID
    {
      return id;
    }

  private:
    static ComponentID id;
  };

  template <typename Self> ComponentID ComponentWrapper<Self>::id = StringID{ typeid(Self).name() };

} // namespace loki
