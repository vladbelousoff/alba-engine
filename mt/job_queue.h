#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <unordered_set>

#include "job.h"

namespace alba {

  class JobQueue
  {
  public:
    void push_job(const Job::SharedPtr& job);
    auto pop_job() -> Job::SharedPtr;
    void mark_job_as_done(Job* job);
    void wait_all();
    void reset();

  private:
    std::mutex queue_mutex;
    std::queue<Job::SharedPtr> queue;
    std::mutex jobs_mutex;
    std::condition_variable jobs_condition;
    std::unordered_set<Job*> jobs_submitted;
    std::unordered_set<Job*> jobs_done;
  };

} // namespace alba
