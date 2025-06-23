#ifndef UTIL_ASM_H_
#define UTIL_ASM_H_

#define PRIVATE_FUNCTION(sym) \
  sym:

#define GLOBAL_FUNCTION(sym) \
  .global sym;               \
  sym:

#endif  // UTIL_ASM_H_