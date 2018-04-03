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
    template<size_t N>
    struct InvokeHelper
    {
        template<typename Function, typename Tuple, typename... Args>
        static inline auto Invoke(Function&& func, Tuple&& tpl, Args &&... args)
            -> decltype(InvokeHelper<N - 1>::Invoke(
            std::forward<Function>(func),
            std::forward<Tuple>(tpl),
            std::get<N - 1>(std::forward<Tuple>(tpl)),
            std::forward<Args>(args)...
            ))
        {
            return InvokeHelper<N - 1>::Invoke(
                std::forward<Function>(func),
                std::forward<Tuple>(tpl),
                std::get<N - 1>(std::forward<Tuple>(tpl)),
                std::forward<Args>(args)...
                );
        }

        template<typename Function, typename T, typename Tuple, typename... Args>
        static inline auto InvokeWeak(Function&& func, T* obj, Tuple&& tpl, Args &&... args)
            -> decltype(InvokeHelper<N - 1>::InvokeWeak(
            std::forward<Function>(func),
            obj,
            std::forward<Tuple>(tpl),
            std::get<N - 1>(std::forward<Tuple>(tpl)),
            std::forward<Args>(args)...
            ))
        {
            return InvokeHelper<N - 1>::InvokeWeak(
                std::forward<Function>(func),
                obj,
                std::forward<Tuple>(tpl),
                std::get<N - 1>(std::forward<Tuple>(tpl)),
                std::forward<Args>(args)...
                );
        }
    };

    template<>
    struct InvokeHelper<0>
    {
        template<typename Function, typename Tuple, typename... Args>
        static inline auto Invoke(Function &&func, Tuple&&, Args &&... args)
            -> decltype(std::forward<Function>(func)(std::forward<Args>(args)...))
        {
            return std::forward<Function>(func)(std::forward<Args>(args)...);
        }

        template<typename Function, typename T, typename Tuple, typename... Args>
        static inline auto InvokeWeak(Function &&func, T* obj, Tuple&&, Args &&... args)
            -> decltype((obj->*(std::forward<Function>(func)))(std::forward<Args>(args)...))
        {
            return (obj->*(std::forward<Function>(func)))(std::forward<Args>(args)...);
        }
    };

    template<typename Function, typename Tuple>
    inline auto Invoke(Function&& func, Tuple&& tpl)
        -> decltype(InvokeHelper<std::tuple_size<typename std::decay<Tuple>::type>::value>::
        Invoke(std::forward<Function>(func), std::forward<Tuple>(tpl)))
    {
        return InvokeHelper<std::tuple_size<typename std::decay<Tuple>::type>::value>::
            Invoke(std::forward<Function>(func), std::forward<Tuple>(tpl));
    }

    template<typename Function, typename T, typename Tuple>
    inline auto InvokeWeak(Function&& func, T* obj, Tuple&& tpl)
        -> decltype(InvokeHelper<std::tuple_size<typename std::decay<Tuple>::type>::value>::
        InvokeWeak(std::forward<Function>(func), obj, std::forward<Tuple>(tpl)))
    {
        return InvokeHelper<std::tuple_size<typename std::decay<Tuple>::type>::value>::
            InvokeWeak(std::forward<Function>(func), obj, std::forward<Tuple>(tpl));
    }

    template<size_t N>
    struct Apply
    {
        template<typename Function, typename Tuple, typename... Args>
        static inline auto apply(Function&& func, Tuple&& tpl, Args &&... args)
            -> decltype(
            Apply<N - 1>::apply(
            std::forward<Function>(func), 
            std::forward<Tuple>(tpl),
            std::get<N - 1>(std::forward<Tuple>(tpl)),
            std::forward<Args>(args)...
            ))
        {
            return Apply<N - 1>::apply(
                std::forward<Function>(func),
                std::forward<Tuple>(tpl),
                std::get<N - 1>(std::forward<Tuple>(tpl)),
                std::forward<Args>(args)...
                );
        }
    };

    template<>
    struct Apply<0>
    {
        template<typename Function, typename Tuple, typename... Args>
        static inline auto apply(Function &&func, Tuple&&, Args &&... args)
            -> decltype(std::forward<Function>(func)(std::forward<Args>(args)...))
        {
            return std::forward<Function>(func)(std::forward<Args>(args)...);
        }
    };

    template<typename Function, typename Tuple>
    inline auto apply(Function&& func, Tuple&& tpl)
        -> decltype(Apply<std::tuple_size<typename std::decay<Tuple>::type>::value>::
        apply(std::forward<Function>(func), std::forward<Tuple>(tpl)))
    {
        return Apply<std::tuple_size<typename std::decay<Tuple>::type>::value>::
            apply(std::forward<Function>(func), std::forward<Tuple>(tpl));
    }


    template<size_t N>
    struct MemberApply
    {
        template<typename Function, typename T, typename Tuple, typename... Args>
        static inline auto apply(Function&& func, T* obj, Tuple&& tpl, Args &&... args)
            -> decltype(MemberApply<N - 1>::apply(
            std::forward<Function>(func),
            obj,
            std::forward<Tuple>(tpl),
            std::get<N - 1>(std::forward<Tuple>(tpl)),
            std::forward<Args>(args)...
            ))
        {
            return MemberApply<N - 1>::apply(
                std::forward<Function>(func),
                obj,
                std::forward<Tuple>(tpl),
                std::get<N - 1>(std::forward<Tuple>(tpl)),
                std::forward<Args>(args)...
                );
        }
    };

    template<>
    struct MemberApply<0>
    {
        template<typename Function, typename T, typename Tuple, typename... Args>
        static inline auto apply(Function &&func, T* obj, Tuple&&, Args &&... args)
            -> decltype((obj->*(std::forward<Function>(func)))(std::forward<Args>(args)...))
        {
            return (obj->*(std::forward<Function>(func)))(std::forward<Args>(args)...);
        }
    };

    template<typename Function, typename T, typename Tuple>
    inline auto member_apply(Function&& func, T* obj, Tuple&& tpl)
        -> decltype(MemberApply<std::tuple_size<typename std::decay<Tuple>::type>::value>::
        apply(std::forward<Function>(func), obj, std::forward<Tuple>(tpl)))
    {
        return MemberApply<std::tuple_size<typename std::decay<Tuple>::type>::value>::
            apply(std::forward<Function>(func), obj, std::forward<Tuple>(tpl));
    }


    int print_int(int name)
    {
        printf("print_int : %d\n", name);
        return 10086;
    }

    void print_func(const char *name)
    {
        std::ostringstream text;
        std::this_thread::get_id()._To_text(text);
        printf("thread: %s, func: %s \n", text.str().c_str(), name);
    }

    class ClosureBase
    {
    public:
        virtual ~ClosureBase() = default;
        virtual void Run() = 0;
    };

    // callback
    class CallbackBase
    {
    public:
        virtual ~CallbackBase() = default;
        virtual void Run() = 0;
    };

    template<bool IsMemberFunc, typename ReturnType, typename T, typename... Args>
    class Callback;

    template<typename ReturnType, typename... Args>
    class Callback<false, ReturnType, void, Args...> : public CallbackBase
    {
    public:
        Callback(ReturnType(*func)(Args...), Args&&... args)
            : func_(func)
            , _Mybargs(std::forward<Args>(args)...)
        {
        }

        virtual void Run() override
        {
            RunWithResult();
        }

        ReturnType RunWithResult()
        {
            if (func_)
            {
                return apply(func_, _Mybargs);
            }
            return ReturnType();
        }

    private:
        //typedef typename std::decay<_Fun>::type _Funx;
        typedef std::tuple<typename std::decay<Args>::type...> _Bargs;

        ReturnType(*func_)(Args...);
        _Bargs _Mybargs;	// the bound arguments
    };

    template<typename ReturnType, typename T, typename... Args>
    class Callback<true, ReturnType, T, Args...> : public CallbackBase
    {
        //typedef typename std::decay<_Fun>::type _Funx;
        typedef std::tuple<typename std::decay<Args>::type...> _Bargs;

    public:
        Callback(ReturnType(T::*func)(Args...), std::weak_ptr<T> &weakptr, Args&&... args)
            : func_(func)
            , weakptr_(weakptr)
            , _Mybargs(std::forward<Args>(args)...)
        {
        }

        virtual void Run() override
        {
            RunWithResult();
        }

        ReturnType RunWithResult()
        {
            std::shared_ptr<T> ptr = weakptr_.lock();
            if (ptr)
            {
                if (func_)
                {
                    //member_apply(&WeakptrTest::print_param, tobj.get(), std::move(std::make_tuple(20108)));
                    member_apply(func_, ptr.get(), _Mybargs);
                }
            }
            else
            {
                printf("obj deleted\n");
            }
            return ReturnType();
        }

    private:

        std::weak_ptr<T> weakptr_;
        ReturnType(T::*func_)(Args...);
        _Bargs _Mybargs;	// the bound arguments
    };

    // bind
    template <class ReturnType, typename... Args>
    Callback<false, ReturnType, void, Args...> Bind(ReturnType(*func)(Args...), Args&&... args)
    {
        return Callback<false, ReturnType, void, Args...>(
            func, 
            std::forward<Args>(args)...);
    }

    template <class ReturnType, typename T, typename... Args>
    Callback<true, ReturnType, T, Args...> Bind(ReturnType(T::*func)(Args...), std::weak_ptr<T> &weakptr, Args&&... args)
    {
        return Callback<true, ReturnType, T, Args...>(
            func,
            weakptr,
            std::forward<Args>(args)...);
    }


    template<typename T>
    class Closure : public ClosureBase
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
            std::shared_ptr<ClosureBase> ptr(new Closure<T>(std::move(closure)));
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
                    std::shared_ptr<ClosureBase> task;
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
        std::queue<std::shared_ptr<ClosureBase>> task_list;
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

    void AsyncCall(std::function<void(void)> func)
    {
        //print_func("AsyncCall");

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

        int print_param(int i)
        {
            std::cout << "print_param : " << i << std::endl;
            return 10011;
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
    auto cb = Bind(print_int, 10010);
    cb.Run();
    auto ret = cb.RunWithResult();
    Invoke(print_int, std::make_tuple(10011));

    std::shared_ptr<WeakptrTest> tobj(new WeakptrTest());
    auto cbw = Bind(&WeakptrTest::print_param, std::weak_ptr<WeakptrTest>(tobj), 2018);
    InvokeWeak(&WeakptrTest::print_param, tobj.get(), std::make_tuple(2019));
    cbw.Run();
    auto retw = cbw.RunWithResult();

    /*g_thread_map[0] = std::shared_ptr<CThread>(new CThread());
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
    g_thread_map.clear();*/
}