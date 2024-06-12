#include "job_queue.h"

void alba::JobQueue::push_job(const Job::SharedPtr& job)
{
  std::unique_lock<std::mutex> lock(queue_mutex);
  queue.push(job);
}

auto alba::JobQueue::pop_job() -> Job::SharedPtr
{
  std::unique_lock<std::mutex> lock(queue_mutex);
  if (queue.empty()) {
    return nullptr;
  }

  Job::SharedPtr job = queue.front();
  queue.pop();

  return job;
}

bool alba::JobQueue::is_empty()
{
  std::unique_lock<std::mutex> lock(queue_mutex);
  return queue.empty();
}
