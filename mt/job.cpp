#include "job.h"

#include <spdlog/spdlog.h>

kiwi::Job::Job(kiwi::StringID name)
    : name{ name }
{
}

auto kiwi::Job::get_name() const -> kiwi::StringID
{
  return name;
}

