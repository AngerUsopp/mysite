#include "stdafx.h"

#include <iostream>           // std::cout
#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <windows.h>
#include <conio.h>
#include <vector>
#include <atomic>
#include <sstream>
#include <memory>
#include <queue>
#include <map>


// https://www.cnblogs.com/qicosmos/p/4325949.html 可变模版参数
namespace
{
    void print_func(const char *name)
    {
        std::ostringstream text;
        std::this_thread::get_id()._To_text(text);
        printf("thread: %s, func: %s \n", text.str().c_str(), name);
    }

    class CallbackBase
    {
    public:
        virtual ~CallbackBase() = default;
        virtual void Run() = 0;
    protected:
    private:
    };

    template<typename T, typename R, typename... Args>
    class Callback : public CallbackBase
    {
    public:
        Callback(R(T::*func)(Args...), std::weak_ptr<T> &weakptr, Args&&... _Args)
            : func_(func)
            , weakptr_(weakptr)
            , _Mybargs(std::forward<Args>(_Args)...)
        {
        }

        virtual void Run() override
        {
            std::shared_ptr<T> ptr = weakptr_.lock();
            if (ptr)
            {
                if (func_)
                {
                    (ptr->*func_)(_Mybargs);
                }
            }
            else
            {
                printf("obj deleted\n");
            }
        }

    protected:
    private:
        std::weak_ptr<T> weakptr_;
        R(T::*func_)(Args...);

        typedef typename std::decay<_Fun>::type _Funx;
        typedef std::tuple<typename std::decay<Args>::type...> _Bargs;

        _Bargs _Mybargs;	// the bound arguments
    };

    template<typename T>
    class Closure : public CallbackBase
    {
    public:
        explicit Closure(std::weak_ptr<T> weakptr, std::function<void(void)> func)
            : weakptr_(weakptr)
            , func_(func)
        {
        }

        Closure(Closure &&right)
        {
            *this = std::move(right);
        }

        Closure& operator=(Closure &&right)
        {
            if (this != &right)
            {
                weakptr_ = right.weakptr_;
                func_ = right.func_;
            }

            return *this;
        }

        virtual void Run() override
        {
            std::shared_ptr<T> ptr = weakptr_.lock();
            if (ptr)
            {
                if (!func_._Empty())
                {
                    func_();
                }
            }
            else
            {
                printf("obj deleted\n");
            }
        }

    protected:
    private:
        std::weak_ptr<T> weakptr_;
        std::function<void(void)> func_;
    };

    class CThread : public std::enable_shared_from_this<CThread>
    {
    public:
        CThread()
        {
        }

        virtual ~CThread()
        {
            StopSoon();
        }

        CThread(CThread &right) = delete;

        CThread& operator=(CThread &right) = delete;

        template<typename T>
        void PostTask(Closure<T> &&closure)
        {
            std::shared_ptr<CallbackBase> ptr(new Closure<T>(std::move(closure)));
            std::unique_lock<std::mutex> lock(task_mutex);
            task_list.push(ptr);

            cv_.notify_one();
        }

        void Run()
        {
            thread_.reset(new std::thread(&CThread::ThreadFunc, std::weak_ptr<CThread>(shared_from_this())));
        }

        void Stop()
        {
            keep_working_ = false;
            std::unique_lock<std::mutex> lock(task_mutex);
            cv_.notify_one();
        }

        void Join()
        {
            if (thread_->joinable())
            {
                thread_->join();
            }
        }

        void StopSoon()
        {
            Stop();
            Join();
        }

    protected:

    private:
        static void ThreadFunc(std::weak_ptr<CThread> weakptr)
        {
            std::shared_ptr<CThread> pThis = weakptr.lock();
            if (pThis)
            {
                pThis->_ThreadFunc();            
            }
        }

        void _ThreadFunc()
        {
            while (keep_working_)
            {
                {
                    std::unique_lock<std::mutex> lock(task_mutex);
                    cv_.wait(lock);
                }

                if (keep_working_)
                {
                    std::shared_ptr<CallbackBase> task;
                    {
                        std::unique_lock<std::mutex> lock(task_mutex);
                        if (!task_list.empty())
                        {
                            task = task_list.front();
                            task_list.pop();
                        }
                    }
                    if (task)
                    {
                        task->Run();
                    }
                }
            }
        }

    private:
        std::shared_ptr<std::thread> thread_;
        std::mutex task_mutex;
        std::condition_variable cv_;
        std::queue<std::shared_ptr<CallbackBase>> task_list;
        bool keep_working_ = true;
    };

    std::map<int, std::shared_ptr<CThread>> g_thread_map;

    template<typename T>
    void PostTask(int tid, Closure<T> &&closure)
    {
        if (g_thread_map.find(tid) != g_thread_map.end())
        {
            g_thread_map[tid]->PostTask(std::move(closure));
        }
    }

    template<typename T, typename ..._Types>
    Closure Bind(_Types... _Args)
    {

    }

    void AsyncCall(std::function<void(void)> func)
    {
        print_func("AsyncCall");

        func();
    }

    class WeakptrTest : public std::enable_shared_from_this<WeakptrTest>
    {
    public:
        WeakptrTest() = default;
        virtual ~WeakptrTest()
        {
        }

        void print_id()
        {
            print_func("weakptr_print");
        }

    protected:
        void OnAsyncCompleted()
        {
            std::shared_ptr<WeakptrTest> ptr = shared_from_this();
            if (ptr)
            {
                print_func("OnAsyncCompleted");
            }
            else
            {
                print_func("OnAsyncCompleted deleted");
            }
        }

    private:
        std::atomic<long> prt_id_ = 0;
    };
}

void thread_atomic_study()
{
    g_thread_map[0] = std::shared_ptr<CThread>(new CThread());
    g_thread_map[1] = std::shared_ptr<CThread>(new CThread());
    g_thread_map[2] = std::shared_ptr<CThread>(new CThread());
    for (auto &thd : g_thread_map)
    {
        thd.second->Run();
    }

    bool work = true;
    int index = 0;
    while (work)
    {
        int cc = _getch();
        switch (cc)
        {
        case VK_ESCAPE:
        {
            work = false;
        }
        break;
        default:
        {
            std::shared_ptr<WeakptrTest> tobj(new WeakptrTest());
            auto at = Closure<WeakptrTest>(tobj, std::bind(&WeakptrTest::print_id, tobj));
            PostTask(index++ % 3, std::move(at));
        }
        break;
        }
    }

    for (auto &thd : g_thread_map)
    {
        thd.second->StopSoon();
    }
    g_thread_map.clear();
}