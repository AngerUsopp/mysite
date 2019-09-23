#include "stdafx.h"

#include <windows.h>

#include "logging/logging.h"
#include "message_loop/message_loop.h"
#include "message_loop/run_loop.h"
#include "threading/thread.h"
#include "threading/thread_util.h"


void mctm_example()
{
    DCHECK(false) << "mctm_example";
    std::unique_ptr<mctm::Thread> thread = mctm::Thread::AttachCurrentThread("main_mctm_thread", mctm::MessageLoop::Type::TYPE_UI);
    if (thread)
    {
        mctm::RunLoop run_loop;
        run_loop.Run();
    }
}