#ifndef COMMON_CSTRING_MEMCPY_H_
#define COMMON_CSTRING_MEMCPY_H_

#include <cstddef>

#ifdef __cplusplus
extern "C"
{
#endif

    void memcpy(void *buf1, const void *buf2, size_t n);

#ifdef __cplusplus
}
#endif

#endif // COMMON_CSTRING_MEMCPY_H_