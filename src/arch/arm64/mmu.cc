#include "arch/arm64/mmu.h"
#include "arch/arm64/mmu_config.h"
#include "mm/pgtable.h"
#include "debug/assert.h"
#include "debug/logger.h"
#include "kernel/util.h"

#include <cassert>
#include <cstddef>
#include <cstdint>

#define PAGE_TABLE_BASE_LEVEL 1U

#define XLAT_TABLE_ENTRIES_SHIFT (CONFIG_MMU_PAGE_SIZE_BITS - 3 /*Each table entry is 8 bytes*/)
#define XLAT_TABLE_ENTRIES (1 << XLAT_TABLE_ENTRIES_SHIFT)

// Calculate the number of page table entries
#define GET_NUM_BASE_LEVEL_ENTRIES(va_bits) (1U << (va_bits - GET_VA_LEVEL_SHIFT(PAGE_TABLE_BASE_LEVEL)))

// Defines the number of page table entries at the base level
#define NUM_BASE_LEVEL_ENTRIES GET_NUM_BASE_LEVEL_ENTRIES(CONFIG_MMU_VA_BITS)

// Calculates the amount of bit shift for a virtual address at the level
#define GET_VA_LEVEL_SHIFT(level) \
  (CONFIG_MMU_PAGE_SIZE_BITS +    \
   (XLAT_TABLE_ENTRIES_SHIFT * (CONFIG_MMU_TABLE_LEVEL_MAX - (level))))

// Static array to store page tables
uint64_t s_base_xlat_table[NUM_BASE_LEVEL_ENTRIES]
    __attribute__((aligned(NUM_BASE_LEVEL_ENTRIES * sizeof(uint64_t))));

#define PTE_DESC_TYPE_MASK 3
#define PTE_TABLE_DESC 3
#define PTE_INVALID_DESC 0
#define PTE_BLOCK_DESC 1
#define PTE_TABLE_DESC 3

// ref. https://developer.arm.com/documentation/101811/0104/The-Memory-Management-Unit--MMU-
// ref. https://zenn.dev/hidenori3/articles/91494881d318a5

// ref. https://zenn.dev/junkawa/articles/fuchsia-zircon-qemu-boot-shim-arm64
// ref. https://developer.arm.com/documentation/ddi0595/2020-12/AArch64-Registers/SCTLR-EL2--System-Control-Register--EL2-?lang=en#fieldset_0-2_2
void Mmu::Disable()
{
  __asm__ volatile(
      "mrs x0, sctlr_el2\n"   // road sctlr_el2
      "bic x0, x0, #1 << 0\n" // clear SCTLR.M
      "bic x0, x0, #1 << 2\n" // clear SCTLR.C
      "msr sctlr_el2, x0\n"   // write sctlr_el2
      "isb");
}

void Mmu::Init(const std::array<MmuMapRegion, 7> &kernel_regions,
               const std::array<MmuMapRegion, 2> &guest_regions)
{
  mmu_regions_kernel_ = kernel_regions;
  mmu_regions_guest_ = guest_regions;

  CheckMmuConfigs();
}

/*
  Validate MMU configuration against hardware constraints.
*/
void Mmu::CheckMmuConfigs()
{
  // Initialization of max address value
  uint64_t max_pa = 0;
  uint64_t max_va = 0;

  // Check address space size
  for (uint32_t i = 0; i < mmu_regions_kernel_.size(); i++)
  {
    auto *region = &mmu_regions_guest_[i];
    max_pa = std::max(max_pa, region->base_pa + region->size);
    max_va = std::max(max_va, region->base_va + region->size);
  }

  // If the maximum address is exceeded, assert
  ASSERT(max_pa <= (1ULL << CONFIG_MMU_PA_BITS),
         "Maximum physical address is over a supported range");
  ASSERT(max_va <= (1ULL << CONFIG_MMU_VA_BITS),
         "Maximum virtual address is over a supported range");
}

/*
  Create translation tables
*/
void Mmu::CreatePageTables()
{
  for (uint8_t i = 0; i < mmu_regions_guest_.size(); i++)
  {
    auto &region = mmu_regions_guest_[i];
    InitXlatTables(region);
  }

  for (uint8_t i = 0; i < mmu_regions_kernel_.size(); i++)
  {
    auto &region = mmu_regions_kernel_[i];
    InitXlatTables(region);
  }
}

/*
  Initialize MMU Translation Tables
*/
void Mmu::InitXlatTables(const MmuMapRegion &region)
{
  uint64_t pa = region.base_pa;
  uint64_t va = region.base_va;
  uint64_t size = region.size;
  uint64_t attrs = region.attrs;
  uint32_t level = PAGE_TABLE_BASE_LEVEL;

  // Check memory alignment
  ASSERT((va & (PAGE_SIZE - 1)) == 0, "virtual address is not page aligned");
  ASSERT((size & (PAGE_SIZE - 1)) == 0, "memory size is not page aligned");

  // Page table mapping from virtual to physical addresses
  while (size > 0)
  {
    ASSERT(level <= CONFIG_MMU_TABLE_LEVEL_MAX, "max translation table level exceeded");
  }

  // Locate PTE for given virtual address and page table level
  uint64_t *pte = FindPteIndex(va, level);
  ASSERT(pte != NULL, "Couln't find PTE");

  const uint64_t level_size = 1ULL << GET_VA_LEVEL_SHIFT(level);
  if (size >= level_size && !(va & (level_size - 1)))
  {
    // Given range fits into level size, create block/page descriptor
    SetPteBlockDesc(pte, pa, attrs, level);
    va += level_size;
    pa += level_size;
    size -= level_size;
  }
}

uint64_t *Mmu::FindPteIndex(uint64_t va, uint32_t level)
{
  uint64_t *pte = NULL;

  // Traverse all translation tables
  pte = (uint64_t *)s_base_xlat_table;
  for (uint32_t i = PAGE_TABLE_BASE_LEVEL; i <= CONFIG_MMU_TABLE_LEVEL_MAX; i++)
  {
    pte += GetVaIndex(va, i);

    if (i == level)
      return pte;

    if (PteDescType(pte) != PTE_TABLE_DESC)
      return NULL;

    // Move on to the next translation table level
    pte = (uint64_t *)(*pte & 0x0000fffffffff000);
  }

  return NULL;
}

inline uint64_t Mmu::GetVaIndex(uint64_t va, uint32_t level)
{
  return (va >> GET_VA_LEVEL_SHIFT(level)) & (XLAT_TABLE_ENTRIES - 1);
}

inline uint8_t Mmu::PteDescType(uint64_t *pte)
{
  return *pte & PTE_DESC_TYPE_MASK;
}

void Mmu::SetPteBlockDesc(uint64_t *pte, uint64_t pa, uint32_t attrs, uint32_t level)
{
  // TODO
}
