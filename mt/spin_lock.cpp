#include "spin_lock.h"

#include <gtest/gtest.h>
#include <thread>

void loki::SpinLock::backoff() const
{
  const int max_retries = 8;
  if (retries < max_retries) {
    std::this_thread::yield();
  } else {
    auto delay = std::chrono::microseconds(1 << (retries - max_retries));
    std::this_thread::sleep_for(delay);
  }
}

void loki::SpinLock::acquire()
{
  retries = 0;
  while (flag.test_and_set(std::memory_order_acquire)) {
    // spin until the lock is released
    backoff();
    retries++;
  }
}

void loki::SpinLock::release()
{
  flag.clear(std::memory_order_release);
}

TEST(SpinLock, Basic)
{
  loki::SpinLock lock;

  auto increment_counter = [&lock](int& counter, int operations) {
    for (int i = 0; i < operations; i++) {
      lock.acquire();
      counter++;
      lock.release();
    }
  };

  const int threads_num = 100;
  const int ops_per_thread = 1'000;

  int counter = 0;

  std::thread threads[threads_num];

  for (auto& thread : threads) {
    thread = std::thread(increment_counter, std::ref(counter), ops_per_thread);
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_EQ(counter, threads_num * ops_per_thread);
}
