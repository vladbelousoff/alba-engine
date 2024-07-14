#pragma once

#include <atomic>
#include <mutex>

namespace loki {

  class SpinLock
  {
    friend class std::lock_guard<SpinLock>;

  public:
    SpinLock() = default;

    SpinLock(const SpinLock&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    SpinLock& operator=(SpinLock&&) = delete;

    void acquire();
    void release();

  private:
    void backoff() const;

    void lock()
    {
      acquire();
    }

    void unlock()
    {
      release();
    }

  private:
    std::atomic_flag flag{ ATOMIC_FLAG_INIT };
    int retries{ 0 };
  };

} // namespace loki

