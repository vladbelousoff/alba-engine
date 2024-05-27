#include "job_queue.h"

void alba::JobQueue::push_job(Job::UniquePtr job)
{
  std::unique_lock<std::mutex> lock(queue_mutex);
  queue.push(std::move(job));
}

auto alba::JobQueue::pop_job() -> Job::UniquePtr
{
  std::unique_lock<std::mutex> lock(queue_mutex);
  if (queue.empty()) {
    return nullptr;
  }

  Job::UniquePtr job = std::move(queue.front());
  queue.pop();

  return job;
}

bool alba::JobQueue::is_empty()
{
  std::unique_lock<std::mutex> lock(queue_mutex);
  return queue.empty();
}
