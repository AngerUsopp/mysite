#pragma once

#include <mutex>
#include <queue>

#include "data_encapsulation/smart_pointer.h"
#include "message_loop/pending_task.h"
#include "time/time_util.h"

namespace mctm
{
    class IncomingTaskQueue
    {
    public:
        explicit IncomingTaskQueue(MessageLoop* message_loop);
        ~IncomingTaskQueue();

        bool AddToIncomingQueue(const Location& from_here,
            const Closure& task,
            TimeDelta delay);

        void ReloadWorkQueue(TaskQueue* work_queue);

    private:
        MessageLoop* message_loop_;

        std::recursive_mutex incoming_queue_lock_;
        TaskQueue incoming_queue_;

        // The next sequence number to use for delayed tasks.
        int next_sequence_num_ = 0;
    };

}
