// console_app.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <windows.h>
#include <vector>

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

    Pred(const Pred &r)/* = delete;*/   // delete��ʾ����
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

    // ��������
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
    // for each������ö�ٻ�������󿽱����죬������ö��ֻ����const
    for each (const Pred &var in vct)
    {
        var.print();
        //var.set_p(1);// constö�ٲ��ܵ���
    }
}

void right_ref_study();
void auto_decltype_study();
void thread_atomic_study();

int _tmain(int argc, _TCHAR* argv[])
{
    //vector_study();
    //right_ref_study();
    //auto_decltype_study();
    thread_atomic_study();
    system("pause");
	return 0;
}

