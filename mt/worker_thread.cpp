#include "worker_thread.h"

loki::WorkerThread::WorkerThread(loki::JobQueue& queue, std::size_t number)
    : job_queue(queue)
    , thread(&WorkerThread::run, this)
    , number(number)
{
}

void loki::WorkerThread::run()
{
  while (!stop_flag) {
    Job::SharedPtr job = job_queue.pop_job();
    if (job && job->is_ready_to_run()) {
      spdlog::info("Job {} is executing on worker thread {}", job->get_name(), number);
      job->run(&job_queue);
    }
  }
}

void loki::WorkerThread::stop()
{
  spdlog::info("Stopping worker thread {}", number);

  stop_flag = true;
  thread.join();

  spdlog::info("Worker thread {} stopped", number);
}
