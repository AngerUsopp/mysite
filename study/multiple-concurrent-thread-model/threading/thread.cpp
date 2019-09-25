#include "thread.h"
#include "thread_util.h"

#include "message_loop/run_loop.h"

namespace
{
    void ThreadFunc(void* params)
    {
        mctm::StdThreadDelegate* thread = static_cast<mctm::StdThreadDelegate*>(params);
        thread->ThreadMain();
    }

    void ThreadQuitHelper()
    {
        mctm::MessageLoop::current()->QuitThread();
    }
}

namespace mctm
{
    std::unique_ptr<mctm::Thread> Thread::AttachCurrentThread(const char* thread_name, MessageLoop::Type type)
    {
        if (!MessageLoop::current())
        {
            std::unique_ptr<mctm::Thread> thread = std::make_unique<mctm::Thread>(thread_name);
            SetThreadName(::GetCurrentThreadId(), thread_name);
            MessageLoopRef message_loop = std::make_shared<MessageLoop>(type);
            thread->set_message_loop(message_loop);
            return std::move(thread);
        }
        return nullptr;
    }

    Thread::Thread(const char* thread_name)
        : thread_name_(thread_name)
    {
    }

    Thread::~Thread()
    {
        Stop();
    }

    bool Thread::Start()
    {
        Options options;
        return StartWithOptions(options);
    }

    bool Thread::StartWithOptions(const Options& options)
    {
        startup_data_.options = options;
        thread_ = std::thread(ThreadFunc, this);
        if (thread_.joinable())
        {
            startup_data_.wait_for_run_event.Wait();
        }

        return thread_.joinable();
    }
    
    void Thread::Stop()
    {
        if (message_loop_)
        {
            message_loop_->PostTask(FROM_HERE, Bind(ThreadQuitHelper));
        }

        if (thread_.joinable())
        {
            thread_.join();
        }
    }

    void Thread::set_message_loop(MessageLoopRef message_loop)
    {
        message_loop_ = message_loop;
    }

    void Thread::ThreadMain()
    {
        SetThreadName(::GetCurrentThreadId(), thread_name_.c_str());

        std::unique_ptr<ScopedCOMInitializer> com;
        if (startup_data_.options.com != NONE)
        {
            com = std::make_unique<ScopedCOMInitializer>(
                startup_data_.options.com == STA ? COINIT_APARTMENTTHREADED : COINIT_MULTITHREADED);
        }

        message_loop_ = std::make_shared<MessageLoop>(startup_data_.options.type);

        Init();
        
        startup_data_.wait_for_run_event.Signal();

        Run();

        CleanUp();

        message_loop_ = nullptr;
    }

    void Thread::Init()
    {
    }

    void Thread::Run()
    {
        RunLoop run_loop;
        run_loop.Run();
    }

    void Thread::CleanUp()
    {
    }

}