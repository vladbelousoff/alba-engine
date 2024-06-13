#include "job_queue.h"

void alba::JobQueue::push_job(const Job::SharedPtr& job)
{
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    queue.push(job);
  }

  {
    std::unique_lock<std::shared_mutex> lock(jobs_mutex);
    jobs_submitted.insert(job.get());
  }
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

void alba::JobQueue::mark_job_as_done(alba::Job* job)
{
  std::unique_lock<std::shared_mutex> lock(jobs_mutex);
  jobs_done.insert(job);
}

bool alba::JobQueue::all_jobs_done()
{
  std::shared_lock<std::shared_mutex> lock(jobs_mutex);
  return jobs_done.size() == jobs_submitted.size();
}

void alba::JobQueue::reset()
{
  std::shared_lock<std::shared_mutex> lock(jobs_mutex);
  
  for (auto* job : jobs_submitted) {
    job->reset();
  }

  jobs_done.clear();
  jobs_submitted.clear();
}
