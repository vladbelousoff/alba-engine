#include "worker_thread.h"

kiwi::WorkerThread::WorkerThread(kiwi::JobQueue& queue)
    : job_queue(queue)
    , thread(&WorkerThread::run, this)
{
}

void kiwi::WorkerThread::run()
{
}

void kiwi::WorkerThread::stop()
{
  stop_flag = true;
  job_queue.push_job(nullptr);
  thread.join();
}
