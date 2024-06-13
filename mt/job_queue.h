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
    bool all_jobs_done();
    void reset();

  private:
    std::mutex queue_mutex;
    std::queue<Job::SharedPtr> queue;
    std::shared_mutex jobs_mutex;
    std::unordered_set<Job*> jobs_submitted;
    std::unordered_set<Job*> jobs_done;
  };

} // namespace alba
