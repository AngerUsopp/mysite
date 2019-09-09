#pragma once
#include <assert.h>


#ifndef DCHECK

#define DCHECK(condition)\
    assert(condition);

#endif