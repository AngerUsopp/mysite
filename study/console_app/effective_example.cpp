#include "stdafx.h"
#include <string>


namespace
{
    class BaseClass
    {
    public:
        explicit BaseClass(const char* str)
            : str_(str)
        {
        }

        virtual ~BaseClass() = default;

        void print() const
        {
            printf("%s \n", str_.c_str());
        }

        std::string* operator->()
        {
            return &str_;
        }

        operator std::string()
        {
            return str_;
        }

        virtual void fly() = 0
        {
            printf("BaseClass::fly() \n");
        }
        virtual void fly(int dst)
        {
            printf("fly to dst \n");
        }

    protected:

    private:
        std::string str_;
    };

    void test_BaseClass_func(const BaseClass &base)
    {
        base.print();
    }

    void test_string_func(const std::string &str)
    {
        printf("%s \n", str.c_str());
    }

    class DervClass : public BaseClass
    {
    public:
        using BaseClass::fly;

        explicit DervClass(const char* str)
            : BaseClass(str)
        {
        }

        virtual void fly() override
        {
            BaseClass::fly();

            printf("DervClass::fly() \n");
        }

    protected:
    private:
    };
}


void effective_example()
{
    //test_BaseClass_func("effect"); // error, BaseClass's contruct with "explicit" flag

    DervClass basec("test_string_func");
    basec.fly();
    basec.fly(1);
    basec.print();
    test_string_func(basec); // operator std::string()
    printf("%s \n", basec->c_str()); // std::string* operator->()

}