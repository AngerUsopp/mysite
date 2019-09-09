#include "message_loop.h"
#include "run_loop.h"
#include "thread_util.h"

#include "logging/logging.h"

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

    /*void MessageLoop::RunLoopInternal()
    {
    }*/

    void MessageLoop::set_run_loop(RunLoopRef run_loop)
    {
        current_run_loop_ = run_loop;
    }

    RunLoopRef MessageLoop::current_run_loop()
    {
        return current_run_loop_;
    }

}
