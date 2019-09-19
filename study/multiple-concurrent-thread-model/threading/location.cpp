#include "location.h"

#include <intrin.h>
extern "C" {
    void* _ReturnAddress();
}

namespace mctm
{
    Location::Location(const char* function_name, 
        const char* file_name, 
        int line_number, 
        const void* program_counter)
    {

    }

    Location::~Location()
    {

    }

    std::string Location::ToString() const
    {
        return std::string(function_name_) + "@" + file_name_ + ":" +
            std::to_string(line_number_);
    }

    const void* GetProgramCounter()
    {
        return _ReturnAddress();
    }

}
