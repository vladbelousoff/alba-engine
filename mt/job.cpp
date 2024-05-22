#include "job.h"

#include <spdlog/spdlog.h>

kiwi::Job::Job(kiwi::StringID name, kiwi::JobPriority priority)
    : name{ name }
    , priority{ priority }
    , delete_job_lock{}
{
}

void kiwi::Job::add_wait(kiwi::Job& job_to_wait)
{
  job_to_wait.delete_job_lock.acquire();
}

void kiwi::Job::add_wait(std::vector<Job>& jobs)
{
  for (Job& job : jobs) {
    add_wait(job);
  }
}

auto kiwi::Job::get_name() const -> kiwi::StringID
{
  return name;
}

auto kiwi::Job::get_priority() const -> kiwi::JobPriority
{
  return priority;
}
