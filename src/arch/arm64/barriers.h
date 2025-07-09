#ifndef ARCH_ARM64_BARRIERS_H_
#define ARCH_ARM64_BARRIERS_H_

static inline void Arm64Isb()
{
  __asm__ volatile("isb");
}

#endif // ARCH_ARM64_BARRIERS_H_