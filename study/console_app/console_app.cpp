// console_app.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


#include <iostream>           // std::cout
#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <windows.h>
#include <conio.h>
#include <vector>

std::mutex mtx;
std::condition_variable cv;

//std::recursive_mutex mtx;
//std::timed_mutex mtx;
//std::recursive_timed_mutex mtx;

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

    Pred(const Pred &r)
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

void print_id(int id, const std::string &str, float ff)
{
    std::cout << "thread start " << id << " p1=" << str.c_str() << " ff=" << ff << '\n';
    //mtx.lock();
    std::unique_lock<std::mutex> lck(mtx);
    //std::lock_guard<std::mutex> lck(mtx);
    std::cout << "thread lock " << id << '\n';
    {
        Pred p;
        Sleep(400);
        std::cout << "thread begin wait " << id << '\n';
        cv.wait(lck/*, p*/);   // 释放锁的同时阻塞等待信号
        std::cout << "thread end wait " << id << '\n';
    }
    //mtx.unlock();
    std::cout << "thread exit " << id << '\n';
}

void go()
{
    /*while (_getche() != VK_ESCAPE)
    {
        std::unique_lock<std::mutex> lck(mtx);
        cv.notify_one();
    }*/

    _getche();
    std::cout << "notify_all-------------" << '\n';
    std::unique_lock<std::mutex> lck(mtx);
    cv.notify_all();
}

int condition_variable_main()
{
    std::thread threads[10];
    for (int i = 0; i < 10; ++i)
    {
        Sleep(100);
        threads[i] = std::thread(print_id, i, "string_param", i * 1.0f);
    }

    std::cout << "ten threads ready to race--------------\n";
    go();

    for (auto& th : threads)
    {
        th.join();
    }

    return 0;
}

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

int _tmain(int argc, _TCHAR* argv[])
{
    condition_variable_main();
    system("pause");
	return 0;
}

