#ifndef DEBUG_ASSERT_H
#define DEBUG_ASSERT_H

#include "debug/logger.h"

#define ASSERT(cond, format, ...)         \
    {                                     \
        if (!(cond))                      \
        {                                 \
            PANIC(format, ##__VA_ARGS__); \
        }                                 \
    }

#endif // DEBUG_ASSERT_H