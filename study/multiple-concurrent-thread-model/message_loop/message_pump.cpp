#include "message_pump.h"

#include <stdio.h>
#include <synchapi.h>
#include <vector>
#include <WinBase.h>

#include "logging/logging.h"

namespace
{
    static const wchar_t kWndClassFormat[] = L"MCTM_MessagePumpWindow_%p";

    // Message sent to get an additional time slice for pumping (processing) another
    // task (a series of such messages creates a continuous task pump).
    static const int kMsgHaveWork = WM_USER + 1;

    HMODULE GetModuleFromAddress(void* address)
    {
        HMODULE instance = NULL;
        if (!::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            static_cast<char*>(address),
            &instance))
        {
            //NOTREACHED();
        }
        return instance;
    }
}

namespace mctm
{
    // MessagePumpForUI
    LRESULT CALLBACK MessagePumpForUI::WndProcThunk(
        HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        switch (message)
        {
        case kMsgHaveWork:
            reinterpret_cast<MessagePumpForUI*>(wparam)->HandleWorkMessage();
            break;
        case WM_TIMER:
            reinterpret_cast<MessagePumpForUI*>(wparam)->HandleTimerMessage();
            break;
        default:
            break;
        }
        return ::DefWindowProc(hwnd, message, wparam, lparam);
    }

    MessagePumpForUI::MessagePumpForUI(MessagePump::Delegate* delegate)
        : MessagePump(delegate)
        , message_filter_(std::make_shared<MessageFilter>())
    {
        InitMessageWnd();
    }
    
    MessagePumpForUI::~MessagePumpForUI()
    {
        DestroyWindow(message_hwnd_);
        message_hwnd_ = nullptr;
        ::UnregisterClass(MAKEINTATOM(atom_), GetModuleFromAddress(&WndProcThunk));
        atom_ = 0;
    }

    void MessagePumpForUI::InitMessageWnd()
    {
        // Generate a unique window class name.
        std::vector<wchar_t> class_name(wcslen(kWndClassFormat) * 2);
        swprintf_s(&class_name[0], class_name.size(), kWndClassFormat, this);

        HINSTANCE instance = GetModuleFromAddress(&WndProcThunk);
        WNDCLASSEX wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = WndProcThunk;
        wc.hInstance = instance;
        wc.lpszClassName = &class_name[0];
        atom_ = RegisterClassEx(&wc);
        DCHECK(atom_);

        message_hwnd_ = CreateWindow(MAKEINTATOM(atom_), 0, 0, 0, 0, 0, 0,
            HWND_MESSAGE, 0, instance, 0);
        DCHECK(message_hwnd_);
    }

    void MessagePumpForUI::DoRunLoop()
    {
        while (true)
        {
            if (delegate_->ShouldQuitCurrentLoop())
            {
                return;
            }

            bool more_work_is_plausible = ProcessNextWindowsMessage();
            if (delegate_->ShouldQuitCurrentLoop())
            {
                return;
            }

            more_work_is_plausible |= delegate_->DoWork();
            if (delegate_->ShouldQuitCurrentLoop())
            {
                return;
            }

            more_work_is_plausible |= delegate_->DoDelayedWork(&delayed_work_time_);
            // If we did not process any delayed work, then we can assume that our
            // existing WM_TIMER if any will fire when delayed work should run.  We
            // don't want to disturb that timer if it is already in flight.  However,
            // if we did do all remaining delayed work, then lets kill the WM_TIMER.
            if (more_work_is_plausible && delayed_work_time_.is_null())
            {
                ::KillTimer(message_hwnd_, reinterpret_cast<UINT_PTR>(this));
            }
            if (delegate_->ShouldQuitCurrentLoop())
            {
                return;
            }

            if (more_work_is_plausible)
            {
                continue;
            }

            more_work_is_plausible |= delegate_->DoIdleWord();
            if (delegate_->ShouldQuitCurrentLoop())
            {
                return;
            }

            if (more_work_is_plausible)
            {
                continue;
            }

            WaitForWork();
        }
    }
    
    void MessagePumpForUI::ScheduleWork()
    {
        BOOL ret = PostMessage(message_hwnd_, kMsgHaveWork,
            reinterpret_cast<WPARAM>(this), 0);
    }
    
    void MessagePumpForUI::ScheduleDelayedWork(const TimeTicks& delayed_work_time)
    {
        // We would *like* to provide high resolution timers.  Windows timers using
        // SetTimer() have a 10ms granularity.  We have to use WM_TIMER as a wakeup
        // mechanism because the application can enter modal windows loops where it
        // is not running our MessageLoop; the only way to have our timers fire in
        // these cases is to post messages there.
        //
        // To provide sub-10ms timers, we process timers directly from our run loop.
        // For the common case, timers will be processed there as the run loop does
        // its normal work.  However, we *also* set the system timer so that WM_TIMER
        // events fire.  This mops up the case of timers not being able to work in
        // modal message loops.  It is possible for the SetTimer to pop and have no
        // pending timers, because they could have already been processed by the
        // run loop itself.
        //
        // We use a single SetTimer corresponding to the timer that will expire
        // soonest.  As new timers are created and destroyed, we update SetTimer.
        // Getting a spurrious SetTimer event firing is benign, as we'll just be
        // processing an empty timer queue.
        //
        delayed_work_time_ = delayed_work_time;

        int delay_msec = GetCurrentDelay();
        //DCHECK_GE(delay_msec, 0);
        if (delay_msec < USER_TIMER_MINIMUM)
            delay_msec = USER_TIMER_MINIMUM;

        // Create a WM_TIMER event that will wake us up to check for any pending
        // timers (in case we are running within a nested, external sub-pump).
        BOOL ret = ::SetTimer(message_hwnd_, reinterpret_cast<UINT_PTR>(this),
            delay_msec, NULL);
        if (ret)
        {
            return;
        }
        // If we can't set timers, we are in big trouble... but cross our fingers for
        // now.
        // TODO(jar): If we don't see this error, use a CHECK() here instead.
        /*UMA_HISTOGRAM_ENUMERATION("Chrome.MessageLoopProblem", SET_TIMER_ERROR,
            MESSAGE_LOOP_PROBLEM_MAX);*/
    }

    void MessagePumpForUI::WaitForWork()
    {
        // Wait until a message is available, up to the time needed by the timer
        // manager to fire the next set of timers.
        int delay = GetCurrentDelay();
        if (delay < 0)  // Negative value means no timers waiting.
            delay = INFINITE;

        DWORD result;
        result = ::MsgWaitForMultipleObjectsEx(0, NULL, delay, QS_ALLINPUT,
            MWMO_INPUTAVAILABLE);

        if (WAIT_OBJECT_0 == result)
        {
            // A WM_* message is available.
            // If a parent child relationship exists between windows across threads
            // then their thread inputs are implicitly attached.
            // This causes the MsgWaitForMultipleObjectsEx API to return indicating
            // that messages are ready for processing (Specifically, mouse messages
            // intended for the child window may appear if the child window has
            // capture).
            // The subsequent PeekMessages call may fail to return any messages thus
            // causing us to enter a tight loop at times.
            // The WaitMessage call below is a workaround to give the child window
            // some time to process its input messages.
            MSG msg = { 0 };
            DWORD queue_status = ::GetQueueStatus(QS_MOUSE);
            if (HIWORD(queue_status) & QS_MOUSE &&
                !::PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE))
            {
                ::WaitMessage();
            }
            return;
        }

        //DCHECK_NE(WAIT_FAILED, result) << GetLastError();
    }
    
    int MessagePumpForUI::GetCurrentDelay() const
    {
        if (delayed_work_time_.is_null())
        {
            return -1;
        }

        // Be careful here.  TimeDelta has a precision of microseconds, but we want a
        // value in milliseconds.  If there are 5.5ms left, should the delay be 5 or
        // 6?  It should be 6 to avoid executing delayed work too early.
        double timeout =
            ceil((delayed_work_time_ - TimeTicks::Now()).InMillisecondsF());

        // If this value is negative, then we need to run delayed work soon.
        int delay = static_cast<int>(timeout);
        if (delay < 0)
        {
            delay = 0;
        }

        return delay;
    }

    bool MessagePumpForUI::ProcessNextWindowsMessage()
    {
        bool sent_messages_in_queue = false;
        DWORD queue_status = GetQueueStatus(QS_SENDMESSAGE);
        if (HIWORD(queue_status) & QS_SENDMESSAGE)
        {
            sent_messages_in_queue = true;
        }

        MSG msg = { 0 };
        if (message_filter_->DoPeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            return ProcessMessageHelper(msg);
        }

        return sent_messages_in_queue;
    }

    bool MessagePumpForUI::ProcessMessageHelper(const MSG& msg)
    {
        if (WM_QUIT == msg.message)
        {
            // Repost the QUIT message so that it will be retrieved by the primary
            // GetMessage() loop.
            delegate_->QuitCurrentLoop();
            ::PostQuitMessage(static_cast<int>(msg.wParam));
            return false;
        }

        // While running our main message pump, we discard kMsgHaveWork messages.
        if (msg.message == kMsgHaveWork && msg.hwnd == message_hwnd_)
        {
            return ProcessPumpScheduleWorkMessage();
        }

        //if (CallMsgFilter(const_cast<MSG*>(&msg), kMessageFilterCode))
        //    return true;

        WillProcessMessage(msg);

        if (!message_filter_->ProcessMessage(msg))
        {
            /*if (state_->dispatcher)
            {
                if (!state_->dispatcher->Dispatch(msg))
                    state_->should_quit = true;
            }
            else*/
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }

        DidProcessMessage(msg);
        return true;
    }

    bool MessagePumpForUI::ProcessPumpScheduleWorkMessage()
    {
        bool have_message = false;
        MSG msg = { 0 };
        have_message = !!message_filter_->DoPeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

        if (!have_message)
        {
            return false;
        }

        ScheduleWork();
        return ProcessMessageHelper(msg);
    }
    
    void MessagePumpForUI::HandleWorkMessage()
    {
        ProcessPumpScheduleWorkMessage();

        // Now give the delegate a chance to do some work.  He'll let us know if he
        // needs to do more work.
        if (delegate_->DoWork())
        {
            ScheduleWork();
        }
    }

    void MessagePumpForUI::HandleTimerMessage()
    {
        ::KillTimer(message_hwnd_, reinterpret_cast<UINT_PTR>(this));

        // If we are being called outside of the context of Run, then don't do
        // anything.  This could correspond to a MessageBox call or something of
        // that sort.
        /*if (!state_)
            return;*/

        // 通知message_loop本次计时截止，让message_loop获得一个处理其计时任务的时机，
        // 并且message_loop将下一次要进行的计时间隔返回来，pump以这个间隔再开计时器，
        // 从而形成计时任务执行流程的自循环
        delegate_->DoDelayedWork(&delayed_work_time_);
        
        // message_loop还有计时任务，或者当前的计时任务在此次计时截止时并未达到其时间间隔要求，
        // 则继续以新任务所需的时间间隔或旧任务基于当前时间相减所剩下的时间间隔为间隔再启计时器
        if (!delayed_work_time_.is_null())
        {
            // A bit gratuitous to set delayed_work_time_ again, but oh well.
            ScheduleDelayedWork(delayed_work_time_);
        }
    }

    void MessagePumpForUI::WillProcessMessage(const MSG& msg)
    {
    }

    void MessagePumpForUI::DidProcessMessage(const MSG& msg)
    {
    }
    
    // MessagePumpForUI::MessageFilter
    bool MessagePumpForUI::MessageFilter::DoPeekMessage(
        LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
    {
        return !!::PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    }

    bool MessagePumpForUI::MessageFilter::ProcessMessage(const MSG& msg)
    {
        return false;
    }


    // MessagePumpForIO
    MessagePumpForIO::MessagePumpForIO(MessagePump::Delegate* delegate)
        : MessagePump(delegate)
    {
    }

    void MessagePumpForIO::DoRunLoop()
    {
        while (true)
        {
            bool more_work_is_plausible = true;

            more_work_is_plausible |= delegate_->DoWork();
            if (delegate_->ShouldQuitCurrentLoop())
            {
                return;
            }

            more_work_is_plausible |= delegate_->DoDelayedWork();
            if (delegate_->ShouldQuitCurrentLoop())
            {
                return;
            }

            if (more_work_is_plausible)
            {
                continue;
            }

            more_work_is_plausible |= delegate_->DoIdleWord();
            if (delegate_->ShouldQuitCurrentLoop())
            {
                return;
            }

            if (more_work_is_plausible)
            {
                continue;
            }

            WaitForWork();
        }
    }
    
    void MessagePumpForIO::ScheduleWork()
    {
    }

    void MessagePumpForIO::ScheduleDelayedWork(const TimeTicks& delayed_work_time)
    {

    }

    void MessagePumpForIO::WaitForWork()
    {
        ::Sleep(100);
    }


}
