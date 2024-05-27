#include "job.h"

#include <spdlog/spdlog.h>

alba::Job::Job(alba::StringID name)
    : name{ name }
{
}

auto alba::Job::get_name() const -> alba::StringID
{
  return name;
}

