#pragma once

#include <numeric>
#include <unordered_set>
#include <vector>

#include "component.h"
#include "string_manager.h"

namespace kiwi {

  class Archetype
  {
    using Type = std::vector<ComponentID>;
    using TypeSet = std::unordered_set<ComponentID>;

  private:
    Type type;
    TypeSet type_set;
  };

} // namespace kiwi
