#pragma once

#include <atomic>
#include <functional>

#include "spin_lock.h"
#include "string_manager.h"

namespace kiwi {

  enum class JobPriority
  {
    NORMAL,
    HIGH,
    CRITICAL,
  };

  class Job
  {
  public:
    using UniqPtr = std::unique_ptr<Job>;

  public:
    explicit Job(StringID name, JobPriority priority = JobPriority::NORMAL);
    virtual ~Job() = default;

    Job(const Job& other) = delete;
    Job(Job&& other) = delete;
    Job& operator=(const Job& other) = delete;
    Job& operator=(Job&& other) = delete;

    void add_wait(Job& job_to_wait);
    void add_wait(std::vector<Job>& jobs);

    auto get_name() const -> StringID;
    auto get_priority() const -> JobPriority;

    virtual void execute() = 0;

  private:
    StringID name;
    JobPriority priority;
    SpinLock delete_job_lock{};
  };

} // namespace kiwi
