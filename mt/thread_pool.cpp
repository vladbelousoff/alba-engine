#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "thread_pool.h"

kiwi::ThreadPool::ThreadPool(std::size_t num_threads)
{
  for (std::size_t i = 0; i < num_threads; ++i) {
    workers.emplace_back(std::make_unique<WorkerThread>(job_queue));
  }
}

kiwi::ThreadPool::~ThreadPool()
{
  for (auto& worker : workers) {
    worker->stop();
  }
}

void kiwi::ThreadPool::submit_job(const std::shared_ptr<Job>& job)
{
  job_queue.push_job(job);
}
