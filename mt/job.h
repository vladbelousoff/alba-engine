#pragma once

#include <atomic>
#include <functional>

#include "spin_lock.h"
#include "string_manager.h"

namespace alba {

  class JobQueue;

  class Job : public std::enable_shared_from_this<Job>
  {
    friend class ThreadPool;

  public:
    using SharedPtr = std::shared_ptr<Job>;
    using WeakPtr = std::weak_ptr<Job>;

  public:
    explicit Job();
    explicit Job(StringID name);
    virtual ~Job() = default;

    Job(const Job& other) = delete;
    Job(Job&& other) = delete;
    Job& operator=(const Job& other) = delete;
    Job& operator=(Job&& other) = delete;

    auto get_name() const -> StringID;
    bool is_done() const;
    bool is_ready_to_run() const;
    void run(JobQueue* job_queue);
    void add_dependency(const SharedPtr& dependency);
    void reset();

  protected:
    virtual void execute() = 0;

  protected:
    StringID name;
    std::atomic_bool flag_done;
    std::vector<WeakPtr> dependencies;
    std::vector<WeakPtr> dependants;
  };

} // namespace alba
