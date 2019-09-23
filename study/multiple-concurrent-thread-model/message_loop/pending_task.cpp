#include "pending_task.h"


namespace mctm
{

    PendingTask::PendingTask()
    {

    }

    PendingTask::PendingTask(const Location & posted_from, const Closure& task, TimeTicks delayed_run_time)
    {
        this->posted_from = posted_from;
        this->task = task;
        this->delayed_run_time = delayed_run_time;
        time_posted = TimeTicks::Now();
    }

    PendingTask::~PendingTask()
    {
    }

}