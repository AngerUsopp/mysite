#include "thread.h"
#include "run_loop.h"
#include "thread_util.h"

namespace
{
    void ThreadFunc(void* params)
    {
        mctm::StdThreadDelegate* thread = static_cast<mctm::StdThreadDelegate*>(params);
        thread->ThreadMain();
    }
}

namespace mctm
{
    Thread::Thread(const char* thread_name)
        : thread_name_(thread_name)
    {
    }

    Thread::~Thread()
    {
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
    }

    void Thread::Init()
    {
    }

    void Thread::Run()
    {
        RunLoopRef run_loop(new RunLoop());
        run_loop->Run();
    }

    void Thread::CleanUp()
    {
    }

}