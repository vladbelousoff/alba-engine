#include "job.h"
#include "job_queue.h"

alba::Job::Job(alba::StringID name)
    : name{ name }
    , flag_done{ false }
{
}

auto alba::Job::get_name() const -> alba::StringID
{
  return name;
}

void alba::Job::add_dependency(const SharedPtr& dependency)
{
  dependencies.push_back(dependency);
  dependency->dependants.push_back(shared_from_this());
}

void alba::Job::run(JobQueue* job_queue)
{
  execute();
  flag_done = true;
  for (auto& dependant : dependants) {
    job_queue->push_job(dependant.lock());
  }
}

bool alba::Job::is_done() const
{
  return flag_done;
}

bool alba::Job::is_ready_to_run() const
{
  if (dependencies.empty()) {
    return true;
  }

  return std::all_of(dependencies.begin(), dependencies.end(), [](const WeakPtr& dependency) {
    SharedPtr job = dependency.lock();
    return job->is_done();
  });
}

