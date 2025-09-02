#ifndef ARCH_ARM64_CPU_REGS_DEF_H_
#define ARCH_ARM64_CPU_REGS_DEF_H_

#define STRINGIFY(x) #x

#define WRITE_CPU_REG(reg, val) \
  ({ __asm__ volatile("msr " STRINGIFY(reg) ", %0" : : "r"(val) : "memory"); })

#endif // ARCH_ARM64_CPU_REGS_DEF_H_