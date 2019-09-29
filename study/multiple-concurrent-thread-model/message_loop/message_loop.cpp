#include "message_loop.h"
#include "run_loop.h"

#include "logging/logging.h"
#include "threading/thread_util.h"


namespace
{
    mctm::ThreadSingletonInstance<mctm::ThreadLocalPointer<mctm::MessageLoop>> message_loop_singleton =
        THREAD_SINGLETON_INSTANCE_INITIALIZER(mctm::ThreadLocalPointer<mctm::MessageLoop>);
}

namespace mctm
{
    MessageLoopRef MessageLoop::current()
    {
        if (message_loop_singleton.Pointer() && message_loop_singleton.Pointer()->Get())
        {
            return message_loop_singleton.Pointer()->Get()->shared_from_this();
        }
        return nullptr;
    }

    MessageLoop::MessageLoop(Type type)
        : type_(type)
        , incoming_task_queue_(this)
    {
        DCHECK(!current());
        message_loop_singleton.Pointer()->Set(this);
        switch (type)
        {
        case Type::TYPE_IO:
            pump_.reset(new MessagePumpForIO(this));
            break;
        case Type::TYPE_UI:
            pump_.reset(new MessagePumpForUI(this));
        	break;
        default:
            pump_.reset(new MessagePumpDefault(this));
            break;
        }
    }

    MessageLoop::~MessageLoop()
    {
        message_loop_singleton.RemoveInstance();
    }

    void MessageLoop::PostTask(const Location& from_here, const Closure& task)
    {
        PostDelayedTask(from_here, task, TimeDelta());
    }

    void MessageLoop::PostDelayedTask(const Location& from_here, const Closure& task, TimeDelta delay)
    {
        incoming_task_queue_.AddToIncomingQueue(from_here, task, delay);
    }

    void MessageLoop::PostIdleTask(const Location& from_here, const Closure& task)
    {

    }

    void MessageLoop::Quit()
    {
        QuitCurrentLoop();
    }

    void MessageLoop::QuitThread()
    {
        QuitLoopRecursive();

        if (type_ == Type::TYPE_UI)
        {
            // ������mctm������Ϣѭ�����Ƶ��˳�֮�⻹Ҫ�ٵ���ϵͳ������Ϣѭ�����Ƶ��˳���
            // ��ֹ��ǰ��mctm��ѭ���������Ƕ����ϵͳ����Ϣѭ������������ʾ��ϵͳ��MesssageBox��
            // ����mctm����Ϣѭ���޷�����ж������Ƿ���Ҫ�˳���ʱ���Ӷ������߳��޷������˳�������
            ::PostQuitMessage(0);
        }
    }

    void MessageLoop::DoRunLoop()
    {
        pump_->DoRunLoop();
    }

    void MessageLoop::QuitCurrentLoop()
    {
        if (current_run_loop_)
        {
            current_run_loop_->Quit();
        }
    }

    void MessageLoop::QuitLoopRecursive()
    {
        thorough_quit_run_loop_ = true;
    }

    void MessageLoop::ReloadWorkQueue()
    {
        if (work_queue_.empty())
        {
            incoming_task_queue_.ReloadWorkQueue(&work_queue_);
        }
    }

    bool MessageLoop::DeferOrRunPendingTask(const PendingTask& pending_task)
    {
        RunTask(pending_task);
        return true;
    }

    void MessageLoop::AddToDelayedWorkQueue(const PendingTask& pending_task)
    {
        delayed_work_queue_.push(pending_task);
    }

    void MessageLoop::RunTask(const PendingTask& pending_task)
    {
        pending_task.task.Run();
    }

    void MessageLoop::ScheduleWork(bool pre_task_queue_status_was_empty)
    {
        if (pre_task_queue_status_was_empty)
        {
            pump_->ScheduleWork();
        }
    }

    void MessageLoop::set_run_loop(RunLoop* run_loop)
    {
        current_run_loop_ = run_loop;
    }

    RunLoop* MessageLoop::current_run_loop()
    {
        return current_run_loop_;
    }

    bool MessageLoop::ShouldQuitCurrentLoop()
    {
        if (thorough_quit_run_loop_)
        {
            return true;
        }

        if (current_run_loop_)
        {
            return current_run_loop_->quitted();
        }
        return true;
    }

    bool MessageLoop::DoWork()
    {
        // �����������ѭ����Ϊ�˷�ֹ�����ǰ���������ȫ�Ǽ�ʱ����Ļ���
        // ��ôҪȷ���˴�DoWorkҪ��ȫ���ļ�ʱ����ȡ����
        // �����һ��ѭ��ȡ�˼�ʱ����֮������һ���߳��ֲ����µļ�ʱ���������
        // ��ôͨ���ڶ���ѭ������������ļ�ʱ������ȡ���������Լ�ʱ�Ŀ�����ʱ�����
        // ��ʱ�߼�����߼�ʱ����ľ���
        while (true)
        {
            // �ӻ��������������������һ���԰�������ȡ����
            ReloadWorkQueue();
            if (work_queue_.empty())
            {
                break;
            }

            do
            {
                PendingTask pending_task = work_queue_.front();
                work_queue_.pop();

                if (!pending_task.delayed_run_time.is_null())
                {
                    AddToDelayedWorkQueue(pending_task);

                    // �����ǰ��ʱ�������ֻ������²����������֪ͨpump�Ը����������ʱ����������ʱ����
                    // �����pump��ScheduleDelayedWork�ĵ�����loop��ʱ������ѭ���������㣬ֻҪһֱ�м�ʱ����
                    // pump�ڲ����Լ�ѭ������ScheduleDelayedWork�γɼ�ʱ���������ѭ��
                    if (delayed_work_queue_.top().task.Equals(pending_task.task))
                    {
                        pump_->ScheduleDelayedWork(pending_task.delayed_run_time);
                    }
                }
                else
                {
                    if (DeferOrRunPendingTask(pending_task))
                    {
                        return true;
                    }
                }
            } while (!work_queue_.empty());
        }
        return false;
    }

    bool MessageLoop::DoDelayedWork(TimeTicks* next_delayed_work_time)
    {
        if (delayed_work_queue_.empty())
        {
            recent_time_ = *next_delayed_work_time = TimeTicks();
            return false;
        }

        // When we "fall behind," there will be a lot of tasks in the delayed work
        // queue that are ready to run.  To increase efficiency when we fall behind,
        // we will only call Time::Now() intermittently, and then process all tasks
        // that are ready to run before calling it again.  As a result, the more we
        // fall behind (and have a lot of ready-to-run delayed tasks), the more
        // efficient we'll be at handling the tasks.

        TimeTicks next_run_time = delayed_work_queue_.top().delayed_run_time;
        if (next_run_time > recent_time_)
        {
            recent_time_ = TimeTicks::Now();
            if (next_run_time > recent_time_)
            {
                *next_delayed_work_time = next_run_time;
                return false;
            }
        }

        PendingTask pending_task = delayed_work_queue_.top();
        delayed_work_queue_.pop();

        if (!delayed_work_queue_.empty())
        {
            *next_delayed_work_time = delayed_work_queue_.top().delayed_run_time;
        }

        return DeferOrRunPendingTask(pending_task);
    }

    bool MessageLoop::DoIdleWord()
    {
        if (idle_work_queue_.empty())
        {
            return false;
        }

        PendingTask pending_task = idle_work_queue_.front();
        idle_work_queue_.pop();

        RunTask(pending_task);
        return true;
    }

}
