#pragma once

#include <atomic>
#include <functional>

#include "spin_lock.h"
#include "string_manager.h"

namespace alba {

  class Job
  {
  public:
    using UniquePtr = std::unique_ptr<Job>;

  public:
    explicit Job(StringID name);
    virtual ~Job() = default;

    Job(const Job& other) = delete;
    Job(Job&& other) = delete;
    Job& operator=(const Job& other) = delete;
    Job& operator=(Job&& other) = delete;

    auto get_name() const -> StringID;

    virtual void execute() = 0;

  protected:
    StringID name;
  };

} // namespace alba
