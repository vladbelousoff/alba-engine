#include "job_queue.h"

void kiwi::JobQueue::push_job(const std::shared_ptr<Job>& job)
{
  std::unique_lock<std::mutex> lock(queue_mutex);
  queue.push(job);
  condition.notify_one();
}

auto kiwi::JobQueue::pop_job() -> std::shared_ptr<kiwi::Job>
{
  std::unique_lock<std::mutex> lock(queue_mutex);
  while (queue.empty()) {
    condition.wait(lock);
  }
  auto job = queue.front();
  queue.pop();
  return job;
}

bool kiwi::JobQueue::is_empty()
{
  std::unique_lock<std::mutex> lock(queue_mutex);
  return queue.empty();
}
