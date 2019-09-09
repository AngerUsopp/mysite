#pragma once

#include "message_pump.h"

#include "data_encapsulation/smart_pointer.h"

namespace mctm
{
    class MessageLoop
        : MessagePump::Delegate
        , public std::enable_shared_from_this<MessageLoop>
    {
    public:
        enum class Type
        {
            TYPE_DEFAULT,
            TYPE_IO = TYPE_DEFAULT,
            TYPE_UI,
        };

        static MessageLoopRef current();

        explicit MessageLoop(Type type);
        ~MessageLoop();
        
    protected:
        // MessagePump::Delegate
        bool ShouldQuitCurrentLoop() override;
        bool DoWork() override;
        bool DoDelayedWork() override;
        bool DoIdleWord() override;

    private:
        void DoRunLoop();
        bool QuitCurrentLoop();
        //void RunLoopInternal();

        // call by RunLoop
        void set_run_loop(RunLoopRef run_loop);
        RunLoopRef current_run_loop();

    private:
        friend class RunLoop;

        Type type_ = Type::TYPE_DEFAULT;
        ScopedMessagePump pump_;
        RunLoopRef current_run_loop_;
    };
}

