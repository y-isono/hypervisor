#ifndef MACRO_H_
#define MACRO_H_

#define BIT64(n) ((1ULL) << (n))
#define GET_LOWER_3_BITS(x) ((x) & 0x07)

#define KB(x) ((x) << 10)
#define MB(x) (KB(x) << 10)

#endif // MACRO_H_