#pragma once

#include "data_encapsulation/smart_pointer.h"

namespace mctm
{
    class RunLoop
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
        RunLoop* previous_run_loop_ = nullptr;
        bool quit_called_ = false;
        bool running_ = false;
        int run_depth_ = 1;
    };
}

