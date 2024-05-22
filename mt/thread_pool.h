#pragma once

#include "worker_thread.h"

namespace kiwi {

  class ThreadPool
  {
  public:
    explicit ThreadPool(std::size_t num_threads);
    ~ThreadPool();

    ThreadPool(const Job& other) = delete;
    ThreadPool(ThreadPool&& other) = delete;
    ThreadPool& operator=(const ThreadPool& other) = delete;
    ThreadPool& operator=(ThreadPool&& other) = delete;

    void submit_job(Job::UniquePtr job);
    void wait_for_all_jobs();

  private:
    std::vector<WorkerThread::UniquePtr> workers;
    JobQueue job_queue;
  };

} // namespace kiwi
