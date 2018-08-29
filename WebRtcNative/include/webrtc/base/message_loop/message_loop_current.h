// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_MESSAGE_LOOP_CURRENT_H_
#define BASE_MESSAGE_LOOP_MESSAGE_LOOP_CURRENT_H_

#include "base/base_export.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "base/message_loop/message_pump_for_io.h"
#include "base/message_loop/message_pump_for_ui.h"
#include "base/pending_task.h"
#include "base/single_thread_task_runner.h"
#include "build/build_config.h"

namespace base {

class MessageLoop;

// MessageLoopCurrent is a proxy to the public interface of the MessageLoop
// bound to the thread it's obtained on.
//
// MessageLoopCurrent(ForUI|ForIO) is available statically through
// MessageLoopCurrent(ForUI|ForIO)::Get() on threads that have a matching
// MessageLoop instance. APIs intended for all consumers on the thread should be
// on MessageLoopCurrent(ForUI|ForIO), while APIs intended for the owner of the
// instance should be on MessageLoop(ForUI|ForIO).
//
// Why: Historically MessageLoop::current() gave access to the full MessageLoop
// API, preventing both addition of powerful owner-only APIs as well as making
// it harder to remove callers of deprecated APIs (that need to stick around for
// a few owner-only use cases and re-accrue callers after cleanup per remaining
// publicly available).
//
// As such, many methods below are flagged as deprecated and should be removed
// (or moved back to MessageLoop) once all static callers have been migrated.
class BASE_EXPORT MessageLoopCurrent {
 public:
  // MessageLoopCurrent is effectively just a disguised pointer and is fine to
  // copy around.
  MessageLoopCurrent(const MessageLoopCurrent& other) = default;
  MessageLoopCurrent& operator=(const MessageLoopCurrent& other) = default;

  // Returns a proxy object to interact with the MessageLoop running the
  // current thread. It must only be used on the thread it was obtained.
  static MessageLoopCurrent Get();

  // Returns true if the current thread is running a MessageLoop. Prefer this to
  // verifying the boolean value of Get() (so that Get() can ultimately DCHECK
  // it's only invoked when IsSet()).
  static bool IsSet();

  // Allow MessageLoopCurrent to be used like a pointer to support the many
  // callsites that used MessageLoop::current() that way when it was a
  // MessageLoop*.
  MessageLoopCurrent* operator->() { return this; }
  explicit operator bool() const { return !!current_; }

  // TODO(gab): Migrate the types of variables that store MessageLoop::current()
  // and remove this implicit cast back to MessageLoop*.
  operator MessageLoop*() const { return current_; }

  // A DestructionObserver is notified when the current MessageLoop is being
  // destroyed.  These observers are notified prior to MessageLoop::current()
  // being changed to return NULL.  This gives interested parties the chance to
  // do final cleanup that depends on the MessageLoop.
  //
  // NOTE: Any tasks posted to the MessageLoop during this notification will
  // not be run.  Instead, they will be deleted.
  //
  // Deprecation note: Prefer SequenceLocalStorageSlot<std::unique_ptr<Foo>> to
  // DestructionObserver to bind an object's lifetime to the current
  // thread/sequence.
  class BASE_EXPORT DestructionObserver {
   public:
    virtual void WillDestroyCurrentMessageLoop() = 0;

   protected:
    virtual ~DestructionObserver() = default;
  };

  // Add a DestructionObserver, which will start receiving notifications
  // immediately.
  void AddDestructionObserver(DestructionObserver* destruction_observer);

  // Remove a DestructionObserver.  It is safe to call this method while a
  // DestructionObserver is receiving a notification callback.
  void RemoveDestructionObserver(DestructionObserver* destruction_observer);

  // Forwards to MessageLoop::task_runner().
  // DEPRECATED(https://crbug.com/616447): Use ThreadTaskRunnerHandle::Get()
  // instead of MessageLoopCurrent::Get()->task_runner().
  const scoped_refptr<SingleThreadTaskRunner>& task_runner() const;

  // Forwards to MessageLoop::SetTaskRunner().
  // DEPRECATED(https://crbug.com/825327): only owners of the MessageLoop
  // instance should replace its TaskRunner.
  void SetTaskRunner(scoped_refptr<SingleThreadTaskRunner> task_runner);

  // A TaskObserver is an object that receives task notifications from the
  // MessageLoop.
  //
  // NOTE: A TaskObserver implementation should be extremely fast!
  class BASE_EXPORT TaskObserver {
   public:
    // This method is called before processing a task.
    virtual void WillProcessTask(const PendingTask& pending_task) = 0;

    // This method is called after processing a task.
    virtual void DidProcessTask(const PendingTask& pending_task) = 0;

   protected:
    virtual ~TaskObserver() = default;
  };

  // Forwards to MessageLoop::(Add|Remove)TaskObserver.
  // DEPRECATED(https://crbug.com/825327): only owners of the MessageLoop
  // instance should add task observers on it.
  void AddTaskObserver(TaskObserver* task_observer);
  void RemoveTaskObserver(TaskObserver* task_observer);

  // Enables or disables the recursive task processing. This happens in the case
  // of recursive message loops. Some unwanted message loops may occur when
  // using common controls or printer functions. By default, recursive task
  // processing is disabled.
  //
  // Please use |ScopedNestableTaskAllower| instead of calling these methods
  // directly.  In general, nestable message loops are to be avoided.  They are
  // dangerous and difficult to get right, so please use with extreme caution.
  //
  // The specific case where tasks get queued is:
  // - The thread is running a message loop.
  // - It receives a task #1 and executes it.
  // - The task #1 implicitly starts a message loop, like a MessageBox in the
  //   unit test. This can also be StartDoc or GetSaveFileName.
  // - The thread receives a task #2 before or while in this second message
  //   loop.
  // - With NestableTasksAllowed set to true, the task #2 will run right away.
  //   Otherwise, it will get executed right after task #1 completes at "thread
  //   message loop level".
  //
  // DEPRECATED(https://crbug.com/750779): Use RunLoop::Type on the relevant
  // RunLoop instead of these methods.
  // TODO(gab): Migrate usage and delete these methods.
  void SetNestableTasksAllowed(bool allowed);
  bool NestableTasksAllowed() const;

  // Enables nestable tasks on the current MessageLoop while in scope.
  // DEPRECATED(https://crbug.com/750779): This should not be used when the
  // nested loop is driven by RunLoop (use RunLoop::Type::kNestableTasksAllowed
  // instead). It can however still be useful in a few scenarios where re-
  // entrancy is caused by a native message loop.
  // TODO(gab): Remove usage of this class alongside RunLoop and rename it to
  // ScopedApplicationTasksAllowedInNativeNestedLoop(?) for remaining use cases.
  class BASE_EXPORT ScopedNestableTaskAllower {
   public:
    ScopedNestableTaskAllower();
    ~ScopedNestableTaskAllower();

   private:
    MessageLoop* const loop_;
    const bool old_state_;
  };

  // Returns true if the message loop is idle (ignoring delayed tasks). This is
  // the same condition which triggers DoWork() to return false: i.e.
  // out of tasks which can be processed at the current run-level -- there might
  // be deferred non-nestable tasks remaining if currently in a nested run
  // level.
  bool IsIdleForTesting();

  // Binds |current| to the current thread. It will from then on be the
  // MessageLoop driven by MessageLoopCurrent on this thread. This is only meant
  // to be invoked by the MessageLoop itself.
  static void BindToCurrentThreadInternal(MessageLoop* current);

  // Unbinds |current| from the current thread. Must be invoked on the same
  // thread that invoked |BindToCurrentThreadInternal(current)|. This is only
  // meant to be invoked by the MessageLoop itself.
  static void UnbindFromCurrentThreadInternal(MessageLoop* current);

  // Returns true if |message_loop| is bound to MessageLoopCurrent on the
  // current thread. This is only meant to be invoked by the MessageLoop itself.
  static bool IsBoundToCurrentThreadInternal(MessageLoop* message_loop);

 protected:
  explicit MessageLoopCurrent(MessageLoop* current) : current_(current) {}

  MessageLoop* const current_;
};

#if !defined(OS_NACL)

// ForUI extension of MessageLoopCurrent.
class BASE_EXPORT MessageLoopCurrentForUI : public MessageLoopCurrent {
 public:
  // Returns an interface for the MessageLoopForUI of the current thread.
  // Asserts that IsSet().
  static MessageLoopCurrentForUI Get();

  // Returns true if the current thread is running a MessageLoopForUI.
  static bool IsSet();

  MessageLoopCurrentForUI* operator->() { return this; }

#if defined(USE_OZONE) && !defined(OS_FUCHSIA) && !defined(OS_WIN)
  // Please see MessagePumpLibevent for definition.
  static_assert(std::is_same<MessagePumpForUI, MessagePumpLibevent>::value,
                "MessageLoopCurrentForUI::WatchFileDescriptor is not supported "
                "when MessagePumpForUI is not a MessagePumpLibevent.");
  bool WatchFileDescriptor(int fd,
                           bool persistent,
                           MessagePumpForUI::Mode mode,
                           MessagePumpForUI::FdWatchController* controller,
                           MessagePumpForUI::FdWatcher* delegate);
#endif

#if defined(OS_IOS)
  // Forwards to MessageLoopForUI::Attach().
  // TODO(https://crbug.com/825327): Plumb the actual MessageLoopForUI* to
  // callers and remove ability to access this method from
  // MessageLoopCurrentForUI.
  void Attach();
#endif

#if defined(OS_ANDROID)
  // Forwards to MessageLoopForUI::Abort().
  // TODO(https://crbug.com/825327): Plumb the actual MessageLoopForUI* to
  // callers and remove ability to access this method from
  // MessageLoopCurrentForUI.
  void Abort();
#endif

 private:
  MessageLoopCurrentForUI(MessageLoop* current, MessagePumpForUI* pump)
      : MessageLoopCurrent(current), pump_(pump) {
    DCHECK(pump_);
  }

  MessagePumpForUI* const pump_;
};

#endif  // !defined(OS_NACL)

// ForIO extension of MessageLoopCurrent.
class BASE_EXPORT MessageLoopCurrentForIO : public MessageLoopCurrent {
 public:
  // Returns an interface for the MessageLoopForIO of the current thread.
  // Asserts that IsSet().
  static MessageLoopCurrentForIO Get();

  // Returns true if the current thread is running a MessageLoopForIO.
  static bool IsSet();

  MessageLoopCurrentForIO* operator->() { return this; }

#if !defined(OS_NACL_SFI)

#if defined(OS_WIN)
  // Please see MessagePumpWin for definitions of these methods.
  HRESULT RegisterIOHandler(HANDLE file, MessagePumpForIO::IOHandler* handler);
  bool RegisterJobObject(HANDLE job, MessagePumpForIO::IOHandler* handler);
  bool WaitForIOCompletion(DWORD timeout, MessagePumpForIO::IOHandler* filter);
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
  // Please see WatchableIOMessagePumpPosix for definition.
  // Prefer base::FileDescriptorWatcher for non-critical IO.
  bool WatchFileDescriptor(int fd,
                           bool persistent,
                           MessagePumpForIO::Mode mode,
                           MessagePumpForIO::FdWatchController* controller,
                           MessagePumpForIO::FdWatcher* delegate);
#endif  // defined(OS_WIN)

#if defined(OS_FUCHSIA)
  // Additional watch API for native platform resources.
  bool WatchZxHandle(zx_handle_t handle,
                     bool persistent,
                     zx_signals_t signals,
                     MessagePumpForIO::ZxHandleWatchController* controller,
                     MessagePumpForIO::ZxHandleWatcher* delegate);
#endif  // defined(OS_FUCHSIA)

#endif  // !defined(OS_NACL_SFI)

 private:
  MessageLoopCurrentForIO(MessageLoop* current, MessagePumpForIO* pump)
      : MessageLoopCurrent(current), pump_(pump) {
    DCHECK(pump_);
  }

  MessagePumpForIO* const pump_;
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_LOOP_CURRENT_H_
