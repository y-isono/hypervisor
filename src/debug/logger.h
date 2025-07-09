#ifndef DEBUG_LOGGER_H_
#define DEBUG_LOGGER_H_

#include "../kernel/util.h"

#define _LOG_COMMON(level, format, ...) \
    {                                   \
        uart_puts(level);               \
        uart_puts("[");                 \
        uart_puts(__FILE__);            \
        uart_puts("] ");                \
        uart_puts(format);              \
    }

#define PANIC(format, ...)     \
    {                          \
        uart_puts("[PANIC]["); \
        uart_puts(__FILE__);   \
        uart_puts("] ");       \
        uart_puts(format);     \
        while (1)              \
            ;                  \
    }

#define LOG_ERROR(format, ...) \
    _LOG_COMMON("[ERROR]", format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) \
    _LOG_COMMON("[WARN]", format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) \
    _LOG_COMMON("[INFO]", format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) \
    _LOG_COMMON("[DEBUG]", format, ##__VA_ARGS__)
#define LOG_TRACE(format, ...) \
    _LOG_COMMON("[TRACE]", format, ##__VA_ARGS__)

#endif // DEBUG_LOGGER_H_
