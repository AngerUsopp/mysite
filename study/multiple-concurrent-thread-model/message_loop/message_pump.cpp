#include "message_pump.h"

#include <synchapi.h>


namespace mctm
{
    MessagePump::MessagePump(Delegate* delegate)
        : delegate_(delegate)
    {
    }

    MessagePump::~MessagePump()
    {
    }


    // MessagePumpForUI
    MessagePumpForUI::MessagePumpForUI(MessagePump::Delegate* delegate)
        : MessagePump(delegate)
    {
    }

    void MessagePumpForUI::DoRunLoop()
    {
        while (true)
        {
            if (delegate_->ShouldQuitCurrentLoop())
            {
                return;
            }

            bool more_work_is_plausible = delegate_->DoWork();
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
    
    void MessagePumpForUI::WaitForWork()
    {
        ::Sleep(100);
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

    void MessagePumpForIO::WaitForWork()
    {
        ::Sleep(100);
    }

}
