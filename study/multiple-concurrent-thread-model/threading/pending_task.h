#pragma once
#include "functional/callback.h"
#include "location.h"

namespace mctm
{
    struct PendingTask
    {
        PendingTask();
        ~PendingTask();

        Closure task;

        // The site this PendingTask was posted from.
        Location posted_from;
    };

}
