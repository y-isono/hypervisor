#ifndef ARCH_ARM64_MMU_CONFIG_H_
#define ARCH_ARM64_MMU_CONFIG_H_

// Physical address space size
// ref. https://developer.arm.com/documentation/100095/0003/Memory-Management-Unit/About-the-MMU
#define CONFIG_MMU_PA_BITS 36

// Virtual address space size
#define CONFIG_MMU_VA_BITS 36

// The bit numbers of page table (= page size)
// Currently support only 4KB page table
#define CONFIG_MMU_PAGE_SIZE_BITS 12

// Maximum level of page table
#define CONFIG_MMU_TABLE_LEVEL_MAX 3

#endif