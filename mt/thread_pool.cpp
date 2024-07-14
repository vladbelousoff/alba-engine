#include "thread_pool.h"

#include <gtest/gtest.h>

loki::ThreadPool::ThreadPool(std::size_t num_threads)
{
  for (std::size_t i = 0; i < num_threads; ++i) {
    workers.emplace_back(std::make_unique<WorkerThread>(job_queue, i));
  }
}

loki::ThreadPool::~ThreadPool()
{
  for (auto& worker : workers) {
    worker->stop();
  }
}

void loki::ThreadPool::submit_job(const Job::SharedPtr& job)
{
  job_queue.push_job(job);
}

void loki::ThreadPool::wait_all()
{
  job_queue.wait_all();
}

void loki::ThreadPool::reset()
{
  job_queue.reset();
}

TEST(ThreadPool, BasicCounter)
{
  class DefaultJob : public loki::Job
  {
  public:
    explicit DefaultJob(loki::StringID name, std::atomic<int>& counter)
        : loki::Job(name)
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
  loki::ThreadPool thread_pool{ 4 };

  for (int i = 0; i < 1'000; ++i) {
    loki::Job::SharedPtr job = std::make_unique<DefaultJob>(loki::StringID{ std::to_string(i) }, counter);
    thread_pool.submit_job(job);
  }

  thread_pool.wait_all();
  EXPECT_EQ(counter.load(), 1'000);
}

TEST(ThreadPool, Dependencies)
{
  static std::mutex mutex;

  class DefaultJob : public loki::Job
  {
  public:
    explicit DefaultJob(loki::StringID name, std::vector<loki::StringID>& names)
        : loki::Job(name)
        , names(names)
    {
    }

    void execute() override
    {
      std::unique_lock<std::mutex> lock(mutex);
      spdlog::info("{}", name);
      names.push_back(name);
    }

  private:
    std::vector<loki::StringID>& names;
  };

  std::vector<loki::StringID> names;
  std::vector<loki::Job::SharedPtr> jobs;
  loki::ThreadPool thread_pool{ 3 };

  for (int i = 0; i < 10; ++i) {
    loki::Job::SharedPtr job = std::make_unique<DefaultJob>(loki::StringID{ std::to_string(i) }, names);
    jobs.push_back(job);
  }

  jobs[1]->add_dependency(jobs[0]);
  jobs[4]->add_dependency(jobs[0]);
  jobs[8]->add_dependency(jobs[0]);
  jobs[2]->add_dependency(jobs[1]);
  jobs[3]->add_dependency(jobs[2]);
  jobs[3]->add_dependency(jobs[4]);
  jobs[5]->add_dependency(jobs[3]);
  jobs[6]->add_dependency(jobs[5]);
  jobs[7]->add_dependency(jobs[5]);
  jobs[9]->add_dependency(jobs[6]);
  jobs[9]->add_dependency(jobs[7]);
  jobs[9]->add_dependency(jobs[8]);

  for (int i = 9; i >= 0; --i) {
    thread_pool.submit_job(jobs[i]);
  }

  thread_pool.wait_all();
  EXPECT_EQ(names.size(), 10);

  // Checks for proper dependency handling
  auto find_pos = [&](const std::string& name) {
    return std::find(names.begin(), names.end(), loki::StringID{ name }) - names.begin();
  };

  EXPECT_LT(find_pos("0"), find_pos("1"));
  EXPECT_LT(find_pos("0"), find_pos("4"));
  EXPECT_LT(find_pos("0"), find_pos("8"));
  EXPECT_LT(find_pos("1"), find_pos("2"));
  EXPECT_LT(find_pos("2"), find_pos("3"));
  EXPECT_LT(find_pos("4"), find_pos("3"));
  EXPECT_LT(find_pos("3"), find_pos("5"));
  EXPECT_LT(find_pos("5"), find_pos("6"));
  EXPECT_LT(find_pos("5"), find_pos("7"));
  EXPECT_LT(find_pos("6"), find_pos("9"));
  EXPECT_LT(find_pos("7"), find_pos("9"));
  EXPECT_LT(find_pos("8"), find_pos("9"));
}
