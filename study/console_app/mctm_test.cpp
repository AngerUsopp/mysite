#include "stdafx.h"

#include <conio.h>
#include <windows.h>

#include "logging/logging.h"
#include "message_loop/message_loop.h"
#include "message_loop/run_loop.h"
#include "threading/thread.h"
#include "threading/thread_util.h"

namespace
{
    int GlobalFunction(const std::string& str)
    {
        std::cout << "GlobalFunction: " << str.c_str() << std::endl;
        return 111;
    }


}

void mctm_example()
{
    DLOG(ERROR) << "mctm_example";
    /*std::unique_ptr<mctm::Thread> thread = mctm::Thread::AttachCurrentThread("main_mctm_thread", mctm::MessageLoop::Type::TYPE_UI);
    if (thread)
    {
        mctm::RunLoop run_loop;
        run_loop.Run();
    }*/

    mctm::Thread thread("mctm_def_thread");
    mctm::Thread::Options option;
    option.type = mctm::MessageLoop::Type::TYPE_IO;
    thread.StartWithOptions(option);

    while (::_getch() != VK_ESCAPE)
    {
        thread.message_loop()->PostTask(FROM_HERE, mctm::Bind(GlobalFunction, "mctm_thread post task"));
        thread.message_loop()->PostDelayedTask(FROM_HERE,
            mctm::Bind(GlobalFunction, "mctm_thread post delayed task"),
            mctm::TimeDelta::FromMilliseconds(2000));
    }
}