#pragma once

#include <memory>
#include <mutex>
#include <queue>

#include "job.h"

namespace alba {

  class JobQueue
  {
  public:
    void push_job(Job::UniquePtr job);
    auto pop_job() -> Job::UniquePtr;
    bool is_empty();

  private:
    std::queue<Job::UniquePtr> queue;
    std::mutex queue_mutex;
  };

} // namespace alba
