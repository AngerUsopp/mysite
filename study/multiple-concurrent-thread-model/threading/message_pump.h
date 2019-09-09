#pragma once

namespace mctm
{
    class MessagePump
    {
    public:
        class Delegate
        {
        public:
            virtual ~Delegate() = default;

            //************************************
            // Method:    ShouldQuitCurrentLoop
            // Returns:   bool，是否退出当前消息循环
            //************************************
            virtual bool ShouldQuitCurrentLoop() = 0;

            //************************************
            // Method:    DoWork、DoDelayedWork、DoIdleWord
            // Returns:   bool，是否还有更多的任务待执行
            //************************************
            virtual bool DoWork() = 0;
            virtual bool DoDelayedWork() = 0;
            virtual bool DoIdleWord() = 0;
        };

        explicit MessagePump(Delegate* delegate);
        ~MessagePump();

        virtual void DoRunLoop() = 0;

    protected:
        Delegate* delegate_ = nullptr;
    };

    class MessagePumpForUI : public MessagePump
    {
    public:
        explicit MessagePumpForUI(MessagePump::Delegate* delegate);

    protected:
        void DoRunLoop() override;

    private:
        void WaitForWork();

    private:
    };

    class MessagePumpForIO : public MessagePump
    {
    public:
        explicit MessagePumpForIO(MessagePump::Delegate* delegate);

    protected:
        void DoRunLoop() override;

    private:
        void WaitForWork();

    private:
    };
}

