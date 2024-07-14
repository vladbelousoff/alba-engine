#include "job_queue.h"

void loki::JobQueue::push_job(const Job::SharedPtr& job)
{
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    queue.push(job);
  }

  {
    std::unique_lock<std::mutex> lock(jobs_mutex);
    jobs_submitted.insert(job.get());
  }
}

auto loki::JobQueue::pop_job() -> Job::SharedPtr
{
  std::unique_lock<std::mutex> lock(queue_mutex);
  if (queue.empty()) {
    return nullptr;
  }

  Job::SharedPtr job = queue.front();
  queue.pop();

  return job;
}

void loki::JobQueue::mark_job_as_done(loki::Job* job)
{
  std::unique_lock<std::mutex> lock(jobs_mutex);
  jobs_done.insert(job);
  jobs_condition.notify_one();
}

void loki::JobQueue::wait_all()
{
  std::unique_lock<std::mutex> lock(jobs_mutex);
  jobs_condition.wait(lock, [this]() {
    return jobs_done.size() == jobs_submitted.size();
  });
}

void loki::JobQueue::reset()
{
  std::unique_lock<std::mutex> lock(jobs_mutex);

  for (auto* job : jobs_submitted) {
    job->reset();
  }

  jobs_done.clear();
  jobs_submitted.clear();
}
