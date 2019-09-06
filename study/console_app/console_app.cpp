// console_app.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <windows.h>
#include <vector>
#include <iostream>

class Pred
{
public:
    Pred()
    {
        static int i = 0;
        id_ = ++i;
        copy_ = false;
        copy_id_ = 0;
    }

    Pred(const Pred &r)/* = delete;*/
    {
        static int i = 0;
        copy_ = true;
        copy_id_ = r.id_;
        id_ = --i;
    }

    Pred(const Pred &&r)
    {
        static int i = 0;
        copy_ = true;
        copy_id_ = r.id_;
        id_ = --i;
    }

    void print() const
    {
        printf("id = %d\n", id_);
    }

    void set_p(int p)
    {
    }

    // 函数对象
    bool operator()()
    {
        return false;
    }

private:

private:
    int id_;
    int copy_id_;
    bool copy_;
};

void vector_study()
{
    std::vector<Pred> vct;
    Pred p, p1, p2;
    vct.push_back(p);
    vct.push_back(p1);
    vct.push_back(p2);
    for (Pred &th : vct)
    {
        th.print();
        th.set_p(1);
    }
    // for each非引用枚举会引起对象拷贝构造，而引用枚举只能用const
    for each (const Pred &var in vct)
    {
        var.print();
        //var.set_p(1);// const枚举不能调用
    }
}

void right_ref_study();
void auto_decltype_study();
void thread_atomic_study();
void variadic_templates_example();
void thread_post_task_study();
void effective_example();
void thread_message_example();
void dead_lock_example();
void ipv6_example();

int _tmain(int argc, _TCHAR* argv[])
{
    std::cout << std::boolalpha;
    //vector_study();
    //right_ref_study();
    //auto_decltype_study();
    variadic_templates_example();
    //thread_atomic_study();
    //thread_post_task_study();
    //effective_example();
    //thread_message_example();
    //dead_lock_example();
    //ipv6_example();
    system("pause");
	return 0;
}

