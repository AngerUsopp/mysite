#pragma once
#include <queue>

#include "functional/callback.h"
#include "message_loop/location.h"
#include "time/time_util.h"

namespace mctm
{
    struct PendingTask
    {
        PendingTask();
        PendingTask(const Location& posted_from,
            const Closure& task,
            TimeTicks delayed_run_time);
        ~PendingTask();

        Closure task;
        // The site this PendingTask was posted from.
        Location posted_from;
        // Secondary sort key for run time.
        int sequence_num = 0;

        // Time when the related task was posted.
        TimeTicks time_posted;

        // The time when the task should be run.
        TimeTicks delayed_run_time;
    };

    using TaskQueue = std::queue<PendingTask>;
    using DelayedTaskQueue = std::priority_queue<PendingTask>;
}
