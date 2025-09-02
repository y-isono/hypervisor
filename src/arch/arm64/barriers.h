#ifndef ARCH_ARM64_BARRIERS_H_
#define ARCH_ARM64_BARRIERS_H_

// ref. https://developer.arm.com/documentation/dui0489/c/arm-and-thumb-instructions/miscellaneous-instructions/dmb--dsb--and-isb
static inline void Arm64Isb()
{
  __asm__ volatile("isb");
}

static inline void Arm64Dsb()
{
  __asm__ volatile("dsb sy");
}

#endif // ARCH_ARM64_BARRIERS_H_