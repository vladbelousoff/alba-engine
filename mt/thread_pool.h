#pragma once

#include "worker_thread.h"

namespace loki {

  class ThreadPool
  {
  public:
    explicit ThreadPool(std::size_t num_threads);
    ~ThreadPool();

    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool(ThreadPool&& other) = delete;
    ThreadPool& operator=(const ThreadPool& other) = delete;
    ThreadPool& operator=(ThreadPool&& other) = delete;

    void submit_job(const Job::SharedPtr& job);
    void wait_all();
    void reset();

  private:
    std::vector<WorkerThread::UniquePtr> workers;
    JobQueue job_queue;
  };

} // namespace loki
