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

    bool MessageLoop::QuitCurrentLoop()
    {
        if (current_run_loop_ && !current_run_loop_->quitted())
        {
            return false;
        }
        return true;
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
        if (!QuitCurrentLoop())
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

    bool MessageLoop::DoDelayedWork()
    {
        if (!QuitCurrentLoop())
        {
            if (delayed_work_queue_.empty())
            {
                return false;
            }

            //////////////////////////////////////////////////////////////////////////

            return true;
        }
        return false;
    }

    bool MessageLoop::DoIdleWord()
    {
        if (!QuitCurrentLoop())
        {
            //////////////////////////////////////////////////////////////////////////
        }
        return false;
    }

}
