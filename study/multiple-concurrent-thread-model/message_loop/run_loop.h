#pragma once

#include "data_encapsulation/smart_pointer.h"

namespace mctm
{
    class RunLoop : public std::enable_shared_from_this<RunLoop>
    {
    public:
        RunLoop();
        ~RunLoop();

        void Run();
        void Quit();
        bool quitted() const;
        bool running() const;

    private:
        bool BeforeRun();
        void AfterRun();

    private:
        MessageLoopRef message_loop_;
        RunLoopRef previous_run_loop_;
        bool quit_called_ = false;
        bool running_ = false;
        int run_depth_ = 1;
    };
}

