#include "stdafx.h"

#include <conio.h>
#include <windows.h>

#include "logging/logging.h"
#include "message_loop/message_loop.h"
#include "message_loop/run_loop.h"
#include "net/pipe/pipe.h"
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

    mctm::PipeServer srv(L"\\\\.\\pipe\\chrome", nullptr, 1);
    mctm::PipeClient cli(L"\\\\.\\pipe\\chrome", nullptr);

    int input_ch = 0;
    do 
    {
        input_ch = ::_getch();

        switch (input_ch)
        {
        case VK_ESCAPE:
            {
                thread.message_loop()->PostTask(FROM_HERE,
                    mctm::Bind(&mctm::PipeServer::Stop, &srv));
                thread.message_loop()->PostTask(FROM_HERE,
                    mctm::Bind(&mctm::PipeClient::Close, &cli));
            }
            break;
        case 0x31://1
            {
                thread.message_loop()->PostTask(FROM_HERE,
                    mctm::Bind(&mctm::PipeServer::Start, &srv));
                thread.message_loop()->PostTask(FROM_HERE,
                    mctm::Bind(&mctm::PipeClient::Connect, &cli));
            }
            break;
        case 0x32://2
            {
                static char data[] = "mctm::PipeServer::Send";
                static ULONG_PTR client_key = 0;
                thread.message_loop()->PostTask(FROM_HERE,
                    mctm::Bind(&mctm::PipeServer::Send, &srv, client_key, data, strlen(data)));
            }
            break;
        case 0x33://3
            {
                static char data[] = "mctm::PipeClient::Send";
                ULONG_PTR client_key = 0;
                thread.message_loop()->PostTask(FROM_HERE,
                    mctm::Bind(&mctm::PipeClient::Send, &cli, data, strlen(data)));
            }
            break;
        default:
            {
            }
            break;
        }

    } while (input_ch != VK_ESCAPE);

    thread.Stop();
}