#pragma once
#include <minwinbase.h>

#include "data_encapsulation/smart_pointer.h"

namespace mctm
{
    struct IOContext;

    class IOHandler
    {
    public:
        virtual ~IOHandler() {}

        virtual void OnIOCompleted(IOContext* context, DWORD bytes_transfered,
            DWORD error) = 0;
    };

    struct IOContext
    {
        OVERLAPPED overlapped = { 0 };
        IOHandler* handler = nullptr;
    };

    class IOCP
    {
        struct IOItem
        {
            IOHandler* handler = nullptr;
            IOContext* context = nullptr;
            DWORD bytes_transfered = 0;
            DWORD error = 0;

            // In some cases |context| can be a non-pointer value casted to a pointer.
            // |has_valid_io_context| is true if |context| is a valid IOContext
            // pointer, and false otherwise.
            bool has_valid_io_context = false;
        };

    public:
        IOCP();
        ~IOCP();

    private:
        ScopedHandle port_;
    };
}

