#include "incoming_task_queue.h"

namespace
{
    mctm::TimeTicks CalculateDelayedRuntime(mctm::TimeDelta delay)
    {
        mctm::TimeTicks delayed_run_time;
        if (delay > mctm::TimeDelta())
        {
            delayed_run_time = mctm::TimeTicks::Now() + delay;

#if defined(OS_WIN)
            if (high_resolution_timer_expiration_.is_null())
            {
                // Windows timers are granular to 15.6ms.  If we only set high-res
                // timers for those under 15.6ms, then a 18ms timer ticks at ~32ms,
                // which as a percentage is pretty inaccurate.  So enable high
                // res timers for any timer which is within 2x of the granularity.
                // This is a tradeoff between accuracy and power management.
                bool needs_high_res_timers = delay.InMilliseconds() <
                    (2 * Time::kMinLowResolutionThresholdMs);
                if (needs_high_res_timers)
                {
                    if (Time::ActivateHighResolutionTimer(true))
                    {
                        high_resolution_timer_expiration_ = TimeTicks::Now() +
                            TimeDelta::FromMilliseconds(
                                MessageLoop::kHighResolutionTimerModeLeaseTimeMs);
                    }
                }
            }
#endif
        }
        else
        {
            //DCHECK_EQ(delay.InMilliseconds(), 0) << "delay should not be negative";
        }

#if defined(OS_WIN)
        if (!high_resolution_timer_expiration_.is_null())
        {
            if (TimeTicks::Now() > high_resolution_timer_expiration_)
            {
                Time::ActivateHighResolutionTimer(false);
                high_resolution_timer_expiration_ = TimeTicks();
            }
        }
#endif

        return delayed_run_time;
    }
}

namespace mctm
{
    IncomingTaskQueue::IncomingTaskQueue(MessageLoopRef message_loop)
        : message_loop_(message_loop)
    {
    }

    IncomingTaskQueue::~IncomingTaskQueue()
    {
    }

    bool IncomingTaskQueue::AddToIncomingQueue(const Location& from_here, 
        const Closure& task, TimeDelta delay)
    {
        PendingTask pending_task(from_here, task, CalculateDelayedRuntime(delay));
        std::lock_guard<std::recursive_mutex> lock(incoming_queue_lock_);
        incoming_queue_.push(std::move(pending_task));
        return true;
    }

    void IncomingTaskQueue::ReloadWorkQueue(TaskQueue* work_queue)
    {
        std::lock_guard<std::recursive_mutex> lock(incoming_queue_lock_);
        incoming_queue_.swap(*work_queue);
    }

}
