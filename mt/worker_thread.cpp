#include "worker_thread.h"

kiwi::WorkerThread::WorkerThread(kiwi::JobQueue& queue, std::size_t number)
    : job_queue(queue)
    , thread(&WorkerThread::run, this)
    , number(number)
{
}

void kiwi::WorkerThread::run()
{
  while (!stop_flag) {
    Job::UniquePtr job = job_queue.pop_job();
    if (job) {
      spdlog::info("Job {} is executing on worker thread {}", job->get_name(), number);
      job->execute();
    }
  }
}

void kiwi::WorkerThread::stop()
{
  spdlog::info("Stopping worker thread {}", number);

  stop_flag = true;
  thread.join();

  spdlog::info("Worker thread {} stopped", number);
}