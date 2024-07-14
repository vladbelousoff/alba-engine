#include "job.h"
#include "job_queue.h"

loki::Job::Job()
    : flag_done{ false }
{
}

loki::Job::Job(loki::StringID name)
    : flag_done{ false }
    , name{ name }
{
}

auto loki::Job::get_name() const -> loki::StringID
{
  return name;
}

void loki::Job::add_dependency(const SharedPtr& dependency)
{
  dependencies.push_back(dependency);
  dependency->dependants.push_back(shared_from_this());
}

void loki::Job::run(JobQueue* job_queue)
{
  execute();
  flag_done = true;
  job_queue->mark_job_as_done(this);
  for (auto& dependant : dependants) {
    job_queue->push_job(dependant.lock());
  }
}

bool loki::Job::is_done() const
{
  return flag_done;
}

bool loki::Job::is_ready_to_run() const
{
  if (dependencies.empty()) {
    return true;
  }

  return std::all_of(dependencies.begin(), dependencies.end(), [](const WeakPtr& dependency) {
    SharedPtr job = dependency.lock();
    return job->is_done();
  });
}

void loki::Job::reset()
{
  flag_done = false;
}

