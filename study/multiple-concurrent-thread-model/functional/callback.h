#pragma once

#include <memory>

namespace mctm
{
    class Closure;
    class CallbackBase
    {
    public:
        virtual ~CallbackBase() = default;

    private:
        friend class Closure;
        virtual void BaseRun() = 0;
    };

    template <class Sig, class Functor>
    class Callback;

    template <class R, class T, class... FnArgs, class Method>
    class Callback<R(T::*)(FnArgs...), Method> : public CallbackBase
    {
    public:
        Callback(Method& method, std::weak_ptr<T> weak_ptr)
            : method_(method)
            , weak_ptr_(weak_ptr)
        {
        }

        template <class... Args>
        R Run(Args&& ...args)
        {
            std::shared_ptr<T> sp = weak_ptr_.lock();
            if (!sp)
            {
                return R();
            }

            return method_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        Method method_;
        std::weak_ptr<T> weak_ptr_;
    };

    template <class R, class T, class... FnArgs, class Method>
    class Callback<R(T::*)(FnArgs...)const, Method> : public CallbackBase
    {
    public:
        Callback(Method& method, std::weak_ptr<T> weak_ptr)
            : method_(method)
            , weak_ptr_(weak_ptr)
        {
        }

        template <class... Args>
        R Run(Args&& ...args) const
        {
            std::shared_ptr<T> sp = weak_ptr_.lock();
            if (!sp)
            {
                return R();
            }

            return method_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        Method method_;
        std::weak_ptr<T> weak_ptr_;
    };

    template <class R, class... FnArgs, class Functor>
    class Callback<R(*)(FnArgs...), Functor> : public CallbackBase
    {
    public:
        explicit Callback(Functor& functor)
            : functor_(functor)
        {
        }

        template <class... Args>
        R Run(Args&& ...args)
        {
            return functor_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        Functor functor_;
    };

    class Closure
    {
    public:
        Closure() = default;

        template <class R, class T, class... FnArgs, class Method>
        Closure(const Callback<R(T::*)(FnArgs...), Method>& callback)
        {
            callback_ = std::make_shared<Callback<R(T::*)(FnArgs...), Method>>(callback);
        }

        template <class R, class T, class... FnArgs, class Method>
        Closure(const Callback<R(T::*)(FnArgs...)const, Method>& callback)
        {
            callback_ = std::make_shared<Callback<R(T::*)(FnArgs...)const, Method>>(callback);
        }

        template <class R, class... FnArgs, class Functor>
        Closure(const Callback<R(*)(FnArgs...), Functor>& callback)
        {
            callback_ = std::make_shared<Callback<R(*)(FnArgs...), Functor>>(callback);
        }

        void Run() const
        {
            if (callback_)
            {
                callback_->BaseRun();
            }
        }
        
        bool Equals(const Closure& other) const
        {
            return callback_.get() == other.callback_.get();
        }

    private:
        std::shared_ptr<CallbackBase> callback_;
    };

    template <class R, class T, class... FnArgs, class... Args>
    auto Bind(R(T::*method)(FnArgs...), std::weak_ptr<T> wp, Args&& ...args)
    {
        auto std_binder = std::bind(method, wp.lock().get(), std::forward<Args>(args)...);
        using CallbackType = Callback<R(T::*)(FnArgs...), decltype(std_binder)>;
        return CallbackType(std_binder, wp);
    }

    template <class R, class T, class... FnArgs, class... Args>
    auto Bind(R(T::*method)(FnArgs...)const, std::weak_ptr<T> wp, Args&& ...args)
    {
        auto std_binder = std::bind(method, wp.lock().get(), std::forward<Args>(args)...);
        using CallbackType = Callback<R(T::*)(FnArgs...)const, decltype(std_binder)>;
        return CallbackType(std_binder, wp);
    }

    template <class R, class... FnArgs, class... Args>
    auto Bind(R(*method)(FnArgs...), Args&& ...args)
    {
        auto std_binder = std::bind(method, std::forward<Args>(args)...);
        using CallbackType = Callback<R(*)(FnArgs...), decltype(std_binder)>;
        return CallbackType(std_binder);
    }
}