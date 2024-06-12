#pragma once

#include <memory>
#include <mutex>
#include <queue>

#include "job.h"

namespace alba {

  class JobQueue
  {
  public:
    void push_job(const Job::SharedPtr& job);
    auto pop_job() -> Job::SharedPtr;
    bool is_empty();

  private:
    std::queue<Job::SharedPtr> queue;
    std::mutex queue_mutex;
  };

} // namespace alba
