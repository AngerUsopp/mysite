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
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>


/*
 * 
 */

using namespace std::placeholders;

// https://www.cnblogs.com/qicosmos/p/4325949.html 可变模版参数
namespace
{
    class TestBindCls
    {
    public:
        int Run(const std::string& str)
        {
            std::cout << "TestBindCls::Run -> '" << str << "'" << std::endl;
            return -2233;
        }
    };

    class CallbackBase
    {
    public:
        virtual ~CallbackBase() = default;
        
        virtual void Run() = 0;
    };

    template <class Sig>
    class Callback;

    template <class Method, class R, class T>
    class Callback : public CallbackBase
    {
    public:
        typedef typename std::decay<Method>::type RunType;

        Callback(Method& method, std::weak_ptr<T> weak_ptr)
            : method_(method)
            , weak_ptr_(weak_ptr)
        {
        }

        template <class... Args>
        R Run(Args&& ...args)
        {
            if (weak_ptr_.expired())
            {
                return (R)0;
            }

            return method_(args...);
        }

        void Run() override
        {
            if (!weak_ptr_.expired())
            {
                method_();
            }
        }

    private:
        RunType method_;
        std::weak_ptr<T> weak_ptr_;
    };

    template <class Method, class R, class T>
    class Callback : public CallbackBase
    {
    public:
        typedef typename std::decay<Method>::type RunType;

        Callback(Method& method, T* object_ptr)
            : method_(method)
            , object_ptr_(object_ptr)
        {
        }

        template <class... Args>
        R Run(Args&& ...args)
        {
            return method_(args...);
        }

        void Run() override
        {
            if (!weak_ptr_.expired())
            {
                method_();
            }
        }

    private:
        RunType method_;
        T* object_ptr_ = nullptr;
    };

    template <class Functor, class R>
    class Callback : public CallbackBase
    {
    public:
        typedef typename std::decay<Functor>::type RunType;

        explicit Callback(Functor& functor)
            : functor_(functor)
        {
        }

        template <class... Args>
        R Run(Args&& ...args)
        {
            return functor_(args...);
        }

        void Run() override
        {
            functor_();
        }

    private:
        RunType functor_;
    };

    class Closure
    {
    public:
        explicit Closure(CallbackBase* callback)
            : callback_(callback)
        {
        }

        void Run()
        {
            if (callback_)
            {
                callback_->Run();
            }
        }

    private:
        std::unique_ptr<CallbackBase> callback_;
    };

    template <class R, class T, class... FnArgs, class... Args>
    Closure Bind(R(T::*method)(FnArgs...), std::weak_ptr<T> wp, Args&& ...args)
    {
        auto std_binder = std::bind(method, wp.lock(), std::forward<Args>(args)...);
        using CallbackType = Callback<decltype(std_binder), R, T>;
        std::unique_ptr<CallbackType> cb = std::make_unique<CallbackType>(std_binder, wp);
        return Closure(cb.release());
    }

    template <class R, class T, class... FnArgs, class... Args>
    Closure Bind(R(T::*method)(FnArgs...), T* ptr, Args&& ...args)
    {
        auto std_binder = std::bind(method, ptr, std::forward<Args>(args)...);
        using CallbackType = Callback<decltype(std_binder), R, T>;
        std::unique_ptr<CallbackType> cb = std::make_unique<CallbackType>(std_binder, wp);
        return Closure(cb.release());
    }
}


void thread_std_bind_task_study()
{
    std::shared_ptr<TestBindCls> st = std::make_shared<TestBindCls>();
    std::weak_ptr<TestBindCls> wp(st);
    /*auto sfn(std::bind(&TestBindCls::Run, st, _1));
    sfn("asd");*/
    auto fn = Bind(&TestBindCls::Run, wp, "12312");
    fn.Run();
    //std::cout << "RunResult -> '" << ret << "'" << std::endl;
    return;
}