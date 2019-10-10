#include "stdafx.h"

#include <conio.h>
#include <windows.h>

#include "ipc/ipc_channel.h"
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

    void ReplyGlobalFunction()
    {
        std::cout << "ReplyGlobalFunction" << std::endl;
    }

    mctm::Thread thread("mctm_def_thread");

    void TestPipe()
    {
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

                    //main_thread->message_loop()->Quit();
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
            case 0x34://4
                {
                    thread.message_loop()->PostTaskAndReply(FROM_HERE,
                        mctm::Bind(&GlobalFunction, "PostTaskAndReply"),
                        mctm::Bind(&ReplyGlobalFunction));
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

    void TestIPC()
    {
        enum
        {
            IPC_MSG_REQ = 1001,
            IPC_MSG_RSP = 1002,
        };

        mctm::Thread::Options option;
        option.type = mctm::MessageLoop::Type::TYPE_IO;
        thread.StartWithOptions(option);

        std::unique_ptr<mctm::IPCChannel> srv = std::make_unique<mctm::IPCChannel>
            (L"\\\\.\\pipe\\chrome", mctm::IPCChannel::MODE_SERVER, nullptr);
        std::unique_ptr<mctm::IPCChannel> clt = std::make_unique<mctm::IPCChannel>
            (L"\\\\.\\pipe\\chrome", mctm::IPCChannel::MODE_CLIENT, nullptr);

        int input_ch = 0;
        do
        {
            input_ch = ::_getch();

            switch (input_ch)
            {
            case VK_ESCAPE:
                {
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Close, srv.get()));
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Close, clt.get()));
                    thread.message_loop()->DeleteSoon(FROM_HERE, srv.release());
                    thread.message_loop()->DeleteSoon(FROM_HERE, clt.release());

                    //main_thread->message_loop()->Quit();
                }
                break;
            case 0x31://1
                {
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Connect, srv.get()));
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Connect, clt.get()));
                }
                break;
            case 0x32://2
                {
                    std::unique_ptr<mctm::IPCMessage> msg = std::make_unique<mctm::IPCMessage>(
                        mctm::MSG_ROUTING_NONE,
                        IPC_MSG_REQ,
                        mctm::IPCMessage::PRIORITY_NORMAL);

                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Send, srv.get(), msg.release()));
                }
                break;
            case 0x33://3
                {
                    std::unique_ptr<mctm::IPCMessage> msg = std::make_unique<mctm::IPCMessage>(
                        mctm::MSG_ROUTING_NONE,
                        IPC_MSG_RSP,
                        mctm::IPCMessage::PRIORITY_NORMAL);

                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Send, clt.get(), msg.release()));
                }
                break;
            default:
                break;
            }

        } while (input_ch != VK_ESCAPE);

        thread.Stop();
    }
}

void mctm_example()
{
    DLOG(ERROR) << "mctm_example";
    std::unique_ptr<mctm::Thread> main_thread = mctm::Thread::AttachCurrentThread("main_mctm_thread", mctm::MessageLoop::Type::TYPE_UI);

    //TestPipe();
    TestIPC();

    /*if (main_thread)
    {
        mctm::RunLoop run_loop;
        run_loop.Run();
    }*/
}