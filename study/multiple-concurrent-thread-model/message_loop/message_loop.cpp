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
        if (message_loop_singleton.Pointer())
        {
            return message_loop_singleton.Pointer()->Get()->shared_from_this();
        }
        return nullptr;
    }

    MessageLoop::MessageLoop(Type type)
        : type_(type)
        , incoming_task_queue_(shared_from_this())
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
            //////////////////////////////////////////////////////////////////////////
            return true;
        }
        return false;
    }

    bool MessageLoop::DoDelayedWork()
    {
        if (!QuitCurrentLoop())
        {
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
            return true;
        }
        return false;
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
        incoming_task_queue_.ReloadWorkQueue(&work_queue_);
    }

    void MessageLoop::set_run_loop(RunLoopRef run_loop)
    {
        current_run_loop_ = run_loop;
    }

    RunLoopRef MessageLoop::current_run_loop()
    {
        return current_run_loop_;
    }

}
