#pragma once

#include <thread>

#include "job_queue.h"

namespace kiwi {

  class WorkerThread
  {
  public:
    explicit WorkerThread(JobQueue& queue);
    void run();
    void stop();

  private:
    JobQueue& job_queue;
    std::thread thread;
    bool stop_flag{ false };
  };

} // namespace kiwi
