#include "message_loop.h"
#include "run_loop.h"

#include "logging/logging.h"
#include "threading/thread_util.h"


namespace
{
    mctm::ThreadSingletonInstance<mctm::ThreadLocalPointer<mctm::MessageLoop>> message_loop_singleton =
        THREAD_SINGLETON_INSTANCE_INITIALIZER(mctm::ThreadLocalPointer<mctm::MessageLoop>);
}

namespace mctm
{
    MessageLoopRef MessageLoop::current()
    {
        if (message_loop_singleton.Pointer() && message_loop_singleton.Pointer()->Get())
        {
            return message_loop_singleton.Pointer()->Get()->shared_from_this();
        }
        return nullptr;
    }

    MessageLoop::MessageLoop(Type type)
        : type_(type)
        , incoming_task_queue_(this)
    {
        DCHECK(!current());
        message_loop_singleton.Pointer()->Set(this);
        switch (type)
        {
        case Type::TYPE_IO:
            pump_.reset(new MessagePumpForIO(this));
            break;
        case Type::TYPE_UI:
            pump_.reset(new MessagePumpForUI(this));
        	break;
        default:
            break;
        }
    }

    MessageLoop::~MessageLoop()
    {
        message_loop_singleton.RemoveInstance();
    }

    void MessageLoop::PostTask(const Location& from_here, const Closure& task)
    {
        PostDelayedTask(from_here, task, TimeDelta());
    }

    void MessageLoop::PostDelayedTask(const Location& from_here, const Closure& task, TimeDelta delay)
    {
        incoming_task_queue_.AddToIncomingQueue(from_here, task, delay);
    }

    void MessageLoop::Quit()
    {
        if (current_run_loop_)
        {
            current_run_loop_->Quit();
        }
    }

    void MessageLoop::DoRunLoop()
    {
        pump_->DoRunLoop();
    }

    void MessageLoop::QuitCurrentLoop()
    {
        Quit();
    }

    void MessageLoop::ReloadWorkQueue()
    {
        if (work_queue_.empty())
        {
            incoming_task_queue_.ReloadWorkQueue(&work_queue_);
        }
    }

    bool MessageLoop::DeferOrRunPendingTask(const PendingTask& pending_task)
    {
        RunTask(pending_task);
        return true;
    }

    void MessageLoop::AddToDelayedWorkQueue(const PendingTask& pending_task)
    {
        delayed_work_queue_.push(pending_task);
    }

    void MessageLoop::RunTask(const PendingTask& pending_task)
    {
        pending_task.task.Run();
    }

    void MessageLoop::ScheduleWork(bool pre_task_queue_status_was_empty)
    {
        if (pre_task_queue_status_was_empty)
        {
            pump_->ScheduleWork();
        }
    }

    void MessageLoop::set_run_loop(RunLoop* run_loop)
    {
        current_run_loop_ = run_loop;
    }

    RunLoop* MessageLoop::current_run_loop()
    {
        return current_run_loop_;
    }

    bool MessageLoop::ShouldQuitCurrentLoop()
    {
        if (current_run_loop_)
        {
            return current_run_loop_->quitted();
        }
        return true;
    }

    bool MessageLoop::DoWork()
    {
        if (!ShouldQuitCurrentLoop())
        {
            // 从互斥锁保护的任务队列中一次性把任务提取出来
            ReloadWorkQueue();
            while (!work_queue_.empty())
            {
                PendingTask pending_task = work_queue_.front();
                work_queue_.pop();

                if (!pending_task.delayed_run_time.is_null())
                {
                    AddToDelayedWorkQueue(pending_task);

                    // 如果当前计时任务队列只有这个新插入的任务则通知pump以该任务所需的时间间隔启动计时器，
                    // 这里对pump的ScheduleDelayedWork的调用是loop计时任务处理循环的启动点，只要一直有计时任务，
                    // pump内部会自己循环调用ScheduleDelayedWork形成计时任务处理的自循环
                    if (delayed_work_queue_.top().task.Equals(pending_task.task))
                    {
                        pump_->ScheduleDelayedWork(pending_task.delayed_run_time);
                    }
                }
                else
                {
                    if (DeferOrRunPendingTask(pending_task))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool MessageLoop::DoDelayedWork(TimeTicks* next_delayed_work_time)
    {
        if (!ShouldQuitCurrentLoop())
        {
            if (delayed_work_queue_.empty())
            {
                recent_time_ = *next_delayed_work_time = TimeTicks();
                return false;
            }

            // When we "fall behind," there will be a lot of tasks in the delayed work
            // queue that are ready to run.  To increase efficiency when we fall behind,
            // we will only call Time::Now() intermittently, and then process all tasks
            // that are ready to run before calling it again.  As a result, the more we
            // fall behind (and have a lot of ready-to-run delayed tasks), the more
            // efficient we'll be at handling the tasks.

            TimeTicks next_run_time = delayed_work_queue_.top().delayed_run_time;
            if (next_run_time > recent_time_)
            {
                recent_time_ = TimeTicks::Now();
                if (next_run_time > recent_time_)
                {
                    *next_delayed_work_time = next_run_time;
                    return false;
                }
            }

            PendingTask pending_task = delayed_work_queue_.top();
            delayed_work_queue_.pop();

            if (!delayed_work_queue_.empty())
            {
                *next_delayed_work_time = delayed_work_queue_.top().delayed_run_time;
            }

            return DeferOrRunPendingTask(pending_task);
        }
        return false;
    }

    bool MessageLoop::DoIdleWord()
    {
        if (!ShouldQuitCurrentLoop())
        {
            //////////////////////////////////////////////////////////////////////////
        }
        return false;
    }

}
