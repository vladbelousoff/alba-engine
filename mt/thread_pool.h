#pragma once

#include "worker_thread.h"

namespace kiwi {

  class ThreadPool
  {
  public:
    explicit ThreadPool(std::size_t num_threads);
    ~ThreadPool();

    void submit_job(const std::shared_ptr<Job>& job);

  private:
    std::vector<std::unique_ptr<WorkerThread>> workers;
    JobQueue job_queue;
  };

} // namespace kiwi
