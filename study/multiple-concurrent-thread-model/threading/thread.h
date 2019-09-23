#pragma once

#include <string>
#include <thread>

#include "data_encapsulation/smart_pointer.h"
#include "synchronization/waitable_event.h"

#include "message_loop/message_loop.h"

/*  Thread�౾��ֱ�ӽ���taskͶ�ݣ���ͨ����MesssageLoop������Щ���������ô˽ṹ��ԭ���ǣ�
 *  1��Threadʵ����������������װ���߳��б������ģ���ô�߳���Threadʵ���Ķ�Ӧ�����ͱ����Ӻ�
 *     �ŵ�Threadʵ�������װ���̵߳�ִ�з���ThreadMain�У����ʱ��ſ���ͨ���߳�ID��Ψһ�Ա�ʶ����
 *     ���̺߳�Threadʵ��������������
 *  
 *
 *
 *
 **/

namespace mctm
{
    class StdThreadDelegate
    {
    public:
        virtual ~StdThreadDelegate() = default;

        virtual void ThreadMain() = 0;
    };

    class Thread : protected StdThreadDelegate
    {
    public:
        enum ComStatus
        {
            NONE,
            STA,
            MTA,
        };

        struct Options 
        {
            MessageLoop::Type type = MessageLoop::Type::TYPE_DEFAULT;
            ComStatus com = ComStatus::NONE;
        };

        static std::unique_ptr<Thread> AttachCurrentThread(const char* thread_name, MessageLoop::Type type);

        explicit Thread(const char* thread_name);
        ~Thread();

        bool Start();
        bool StartWithOptions(const Options& options);

    protected:
        void set_message_loop(MessageLoopRef message_loop);

        // StdThreadDelegate
        void ThreadMain() override;

        virtual void Init();
        virtual void Run();
        virtual void CleanUp();

    private:
        struct StartupData
        {
            StartupData()
                : wait_for_run_event(false, false)
            {
            }
            
            Options options;
            WaitableEvent wait_for_run_event;
        };

    private:
        std::string thread_name_;
        StartupData startup_data_;
        std::thread thread_;
        MessageLoopRef message_loop_;
    };
}

