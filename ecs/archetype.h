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

    Type type;
    std::unordered_set<ComponentID> type_set;
  };

} // namespace kiwi
