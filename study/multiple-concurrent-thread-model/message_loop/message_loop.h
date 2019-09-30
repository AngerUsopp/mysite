#pragma once

#include "location.h"
#include "message_pump.h"

#include "data_encapsulation/smart_pointer.h"
#include "functional/callback.h"
#include "incoming_task_queue.h"
#include "time/time_util.h"

namespace mctm
{
    class RunLoop;
    class MessageLoop
        : MessagePump::Delegate
        , public std::enable_shared_from_this<MessageLoop>
    {
    public:
        enum class Type
        {
            TYPE_DEFAULT,
            TYPE_UI,
            TYPE_IO,
        };

        static MessageLoopRef current();

        explicit MessageLoop(Type type);
        ~MessageLoop();

        Type type() const { return type_; }

        void PostTask(const Location& from_here,
            const Closure& task);

        void PostDelayedTask(const Location& from_here,
            const Closure& task,
            TimeDelta delay);

        void PostIdleTask(const Location& from_here,
            const Closure& task);

        void Quit();
        void QuitThread();
        
    protected:
        // MessagePump::Delegate
        bool ShouldQuitCurrentLoop() override;
        bool DoWork() override;
        bool DoDelayedWork(TimeTicks* next_delayed_work_time) override;
        bool DoIdleWord() override;
        void QuitCurrentLoop() override;
        void QuitLoopRecursive() override;

    private:
        void DoRunLoop();
        void ReloadWorkQueue();
        bool DeferOrRunPendingTask(const PendingTask& pending_task);
        void AddToDelayedWorkQueue(const PendingTask& pending_task);
        void RunTask(const PendingTask& pending_task);
        void ScheduleWork(bool pre_task_queue_status_was_empty);

        // call by RunLoop
        void set_run_loop(RunLoop* run_loop);
        RunLoop* current_run_loop();

    protected:
        ScopedMessagePump pump_;

    private:
        friend class RunLoop;
        friend class IncomingTaskQueue;

        Type type_ = Type::TYPE_DEFAULT;
        RunLoop* current_run_loop_ = nullptr;
        IncomingTaskQueue incoming_task_queue_;
        TaskQueue work_queue_;
        TaskQueue idle_work_queue_;
        DelayedTaskQueue delayed_work_queue_;
        TimeTicks recent_time_;
        bool thorough_quit_run_loop_ = false;
    };

    class MessageLoopForUI : public MessageLoop
    {
    public:
        static MessageLoopForUIRef current();

    protected:
        MessagePumpForUI* pump_ui();

    private:
    };

    class MessageLoopForIO : public MessageLoop
    {
    public:
        static MessageLoopForIORef current();

        bool RegisterIOHandler(HANDLE file_handle, MessagePumpForIO::IOHandler* handler);
        bool RegisterJobObject(HANDLE job_handle, MessagePumpForIO::IOHandler* handler);
        bool WaitForIOCompletion(DWORD timeout, MessagePumpForIO::IOHandler* filter);

    protected:
        MessagePumpForIO* pump_io();
    };
}

