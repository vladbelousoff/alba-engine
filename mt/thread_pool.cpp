#include "thread_pool.h"

#include <gtest/gtest.h>
#include <utility>

kiwi::ThreadPool::ThreadPool(std::size_t num_threads)
{
  for (std::size_t i = 0; i < num_threads; ++i) {
    workers.emplace_back(std::make_unique<WorkerThread>(job_queue, i));
  }
}

kiwi::ThreadPool::~ThreadPool()
{
  for (auto& worker : workers) {
    worker->stop();
  }
}

void kiwi::ThreadPool::submit_job(Job::UniquePtr job)
{
  job_queue.push_job(std::move(job));
}

void kiwi::ThreadPool::wait_for_jobs()
{
  while (!job_queue.is_empty()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

TEST(ThreadPool, BasicCounter)
{
  class DefaultJob : public kiwi::Job
  {
  public:
    explicit DefaultJob(kiwi::StringID name, std::atomic<int>& counter)
        : kiwi::Job(name)
        , counter(counter)
    {
    }

    void execute() override
    {
      counter++;
    }

  private:
    std::atomic<int>& counter;
  };

  std::atomic<int> counter{ 0 };
  kiwi::ThreadPool thread_pool{ 4 };

  for (int i = 0; i < 1'000; ++i) {
    kiwi::Job::UniquePtr job = std::make_unique<DefaultJob>(kiwi::StringID{ std::to_string(i) }, counter);
    thread_pool.submit_job(std::move(job));
  }

  thread_pool.wait_for_jobs();
  EXPECT_EQ(counter.load(), 1'000);
}
