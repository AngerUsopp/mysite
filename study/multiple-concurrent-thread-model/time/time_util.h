#pragma once

namespace mctm
{
    class TimeTicks;
    // ʱ����
    class TimeDelta
    {
    public:
        TimeDelta();
        ~TimeDelta();

        static TimeDelta FromMilliseconds(__int64 ms);

    private:
        explicit TimeDelta(__int64 ms);

    private:
        friend class TimeTicks;
        // Delta in microseconds.
        __int64 delta_in_ms_ = 0;
    };

    // ϵͳ���������ĺ�����
    class TimeTicks
    {
    public:
        TimeTicks();
        ~TimeTicks();

        static TimeTicks Now();

        TimeTicks operator+(TimeDelta delta) const
        {
            return TimeTicks(ticks_in_ms_ + delta.delta_in_ms_);
        }
        TimeTicks operator-(TimeDelta delta) const
        {
            return TimeTicks(ticks_in_ms_ - delta.delta_in_ms_);
        }

    private:
        explicit TimeTicks(__int64 ms);

    private:
        // Delta in microseconds.
        __int64 ticks_in_ms_ = 0;
    };
}
