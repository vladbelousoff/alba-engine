#pragma once

#include <GLFW/glfw3.h>

namespace loki {

  template <typename T> class ScopeTimer
  {
  public:
    explicit ScopeTimer(T& seconds)
        : seconds{ seconds }
        , start_timestamp{ glfwGetTime() }
    {
    }

    ~ScopeTimer()
    {
      seconds = static_cast<T>(glfwGetTime() - start_timestamp);
    }

    auto get_start() -> double
    {
      return start_timestamp;
    }

  private:
    double start_timestamp;
    T& seconds;
  };

} // namespace loki
