// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_THREAD_CONTROLLER_WITH_MESSAGE_PUMP_IMPL_H_
#define BASE_TASK_SEQUENCE_MANAGER_THREAD_CONTROLLER_WITH_MESSAGE_PUMP_IMPL_H_

#include "base/debug/task_annotator.h"
#include "base/message_loop/message_pump.h"
#include "base/task/sequence_manager/sequenced_task_source.h"
#include "base/task/sequence_manager/thread_controller.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread_task_runner_handle.h"

namespace base {
namespace sequence_manager {
namespace internal {

// EXPERIMENTAL ThreadController implementation which doesn't use
// MessageLoop or a task runner to schedule their DoWork calls.
// See https://crbug.com/828835.
class BASE_EXPORT ThreadControllerWithMessagePumpImpl
    : public ThreadController,
      public MessagePump::Delegate,
      public RunLoop::Delegate {
 public:
  explicit ThreadControllerWithMessagePumpImpl(TickClock* time_source);
  ~ThreadControllerWithMessagePumpImpl() override;

  // ThreadController implementation:
  void SetSequencedTaskSource(SequencedTaskSource* task_source) override;
  void SetWorkBatchSize(int work_batch_size) override;
  void WillQueueTask(PendingTask* pending_task) override;
  void ScheduleWork() override;
  void SetNextDelayedDoWork(LazyNow* lazy_now, TimeTicks run_time) override;
  const TickClock* GetClock() override;
  bool RunsTasksInCurrentSequence() override;
  void SetDefaultTaskRunner(
      scoped_refptr<SingleThreadTaskRunner> task_runner) override;
  void RestoreDefaultTaskRunner() override;
  void AddNestingObserver(RunLoop::NestingObserver* observer) override;
  void RemoveNestingObserver(RunLoop::NestingObserver* observer) override;

 private:
  friend class DoWorkScope;
  friend class RunScope;

  // MessagePump::Delegate implementation.
  bool DoWork() override;
  bool DoDelayedWork(TimeTicks* next_run_time) override;
  bool DoIdleWork() override;

  // RunLoop::Delegate implementation.
  void Run(bool application_tasks_allowed) override;
  void Quit() override;
  void EnsureWorkScheduled() override;

  struct MainThreadOnly {
    MainThreadOnly();
    ~MainThreadOnly();

    SequencedTaskSource* task_source = nullptr;            // Not owned.
    RunLoop::NestingObserver* nesting_observer = nullptr;  // Not owned.
    std::unique_ptr<ThreadTaskRunnerHandle> thread_task_runner_handle;

    // Next delayed DoWork time for scheduling de-duplication purpose.
    TimeTicks next_delayed_work;

    // Indicates that we should yield DoWork ASAP.
    bool quit_do_work = false;

    // Number of tasks processed in a single DoWork invocation.
    int batch_size = 1;

    // Number of RunLoop layers currently running.
    int run_depth = 0;

    // Number of DoWork running, but only the inner-most one can take tasks.
    // Must be equal to |run_depth| or |run_depth - 1|.
    int do_work_depth = 0;
  };

  MainThreadOnly& main_thread_only() {
    DCHECK_CALLED_ON_VALID_THREAD(main_thread_checker_);
    return main_thread_only_;
  }

  // Returns true if there's a DoWork running on the inner-most nesting layer.
  bool is_doing_work() const {
    DCHECK_CALLED_ON_VALID_THREAD(main_thread_checker_);
    return main_thread_only_.do_work_depth == main_thread_only_.run_depth &&
           main_thread_only_.do_work_depth != 0;
  }

  MainThreadOnly main_thread_only_;
  const PlatformThreadId main_thread_id_;
  std::unique_ptr<MessagePump> pump_;
  debug::TaskAnnotator task_annotator_;
  TickClock* time_source_;  // Not owned.

  THREAD_CHECKER(main_thread_checker_);
  DISALLOW_COPY_AND_ASSIGN(ThreadControllerWithMessagePumpImpl);
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_THREAD_CONTROLLER_WITH_MESSAGE_PUMP_IMPL_H_
