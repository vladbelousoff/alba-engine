#pragma once

#include <thread>

#include "job_queue.h"

namespace alba {

  class WorkerThread
  {
  public:
    using UniquePtr = std::unique_ptr<WorkerThread>;

  public:
    explicit WorkerThread(alba::JobQueue& queue, std::size_t number);

    void run();
    void stop();

  private:
    JobQueue& job_queue;
    std::thread thread;
    bool stop_flag{ false };
    std::size_t number;
  };

} // namespace alba
