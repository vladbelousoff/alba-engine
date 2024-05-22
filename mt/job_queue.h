#pragma once

#include <memory>
#include <mutex>
#include <queue>

#include "job.h"

namespace kiwi {

  class JobQueue
  {
  public:
    void push_job(const std::shared_ptr<Job>& job);
    auto pop_job() -> std::shared_ptr<Job>;
    bool is_empty();

  private:
    std::queue<std::shared_ptr<Job>> queue;
    std::mutex queue_mutex;
    std::condition_variable condition;
  };

} // namespace kiwi
