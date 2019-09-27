#pragma once
#include <memory>
#include <windef.h>
#include <WinUser.h>

#include "data_encapsulation/smart_pointer.h"
#include "time/time_util.h"
#include "synchronization/waitable_event.h"

namespace mctm
{
    class WaitableEvent;

    class MessagePump
    {
    public:
        class Delegate
        {
        public:
            virtual ~Delegate() = default;

            //************************************
            // Method:    ShouldQuitCurrentLoop
            // Returns:   bool���Ƿ��˳���ǰ��Ϣѭ��
            //************************************
            virtual bool ShouldQuitCurrentLoop() = 0;

            //************************************
            // Method:    DoWork��DoDelayedWork��DoIdleWord
            // Returns:   bool���Ƿ��и���������ִ��
            //************************************
            virtual bool DoWork() = 0;
            virtual bool DoDelayedWork(TimeTicks* next_delayed_work_time) = 0;
            virtual bool DoIdleWord() = 0;

            //************************************
            // Method:    QuitCurrentLoop
            // Returns:   �˳���ǰ��Ϣѭ��
            //************************************
            virtual void QuitCurrentLoop() = 0;

            //************************************
            // Method:    QuitLoopRecursive
            // Returns:   �����̣߳��ݹ��˳�ȫ��Ƕ�׵���Ϣѭ��
            //************************************
            virtual void QuitLoopRecursive() = 0;
        };

        explicit MessagePump(Delegate* delegate);
        virtual ~MessagePump() = default;

        virtual void DoRunLoop() = 0;
        virtual void ScheduleWork() = 0;
        virtual void ScheduleDelayedWork(const TimeTicks& delayed_work_time) = 0;

    protected:
        int GetCurrentDelay() const;

    protected:
        Delegate* delegate_ = nullptr;
        TimeTicks delayed_work_time_;
    };

    // ��EventΪ�źŵ����ѭ���ı�
    class MessagePumpDefault : public MessagePump
    {
    public:
        explicit MessagePumpDefault(Delegate* delegate);
        virtual ~MessagePumpDefault();

    protected:
        // MessagePump
        void DoRunLoop() override;
        void ScheduleWork() override;
        void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;

    private:
        void WaitForWork();

    private:
        WaitableEvent event_;
    };

    // ��ϵͳ��Ϣ��GetMessage/PeekMessage��Ϊ�źŵ����ѭ���ı�
    class MessagePumpForUI : public MessagePump
    {
    public:
        class MessageFilter
        {
        public:
            virtual ~MessageFilter() = default;

            virtual bool DoPeekMessage(LPMSG lpMsg,
                HWND hWnd,
                UINT wMsgFilterMin,
                UINT wMsgFilterMax,
                UINT wRemoveMsg);
            virtual bool ProcessMessage(const MSG& msg);
        };
        
        explicit MessagePumpForUI(MessagePump::Delegate* delegate);
        virtual ~MessagePumpForUI();

    protected:
        // MessagePump
        void DoRunLoop() override;
        void ScheduleWork() override;
        void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;

    private:
        static LRESULT CALLBACK WndProcThunk(
            HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
        void InitMessageWnd();
        void WaitForWork();
        bool ProcessNextWindowsMessage();
        bool ProcessMessageHelper(const MSG& msg);
        void WillProcessMessage(const MSG& msg);
        void DidProcessMessage(const MSG& msg);
        bool ProcessPumpScheduleWorkMessage();
        void HandleWorkMessage();
        void HandleTimerMessage();

    private:
        std::shared_ptr<MessageFilter> message_filter_;
        ATOM atom_ = 0;
        HWND message_hwnd_ = nullptr;
    };

    // ��I/O��ɶ˿ڣ�IOCP��Ϊ�źŵ����ѭ���ı�
    class MessagePumpForIO : public MessagePump
    {
    public:
        explicit MessagePumpForIO(MessagePump::Delegate* delegate);

    protected:
        // MessagePump
        void DoRunLoop() override;
        void ScheduleWork() override;
        void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;

    private:
        void WaitForWork();

    private:
        ScopedHandle iocp_;
    };
}

