#include "arch/arm64/mmu.h"
#include "arch/arm64/mmu_config.h"
#include "arch/arm64/cpu_regs_def.h"
#include "arch/arm64/barriers.h"
#include "mm/pgtable.h"
#include "debug/assert.h"
#include "debug/logger.h"
#include "kernel/util.h"
#include "macro.h"

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

/*
 * Default MAIR
 *                  index   attribute
 * DEVICE_nGnRnE      0     0000:0000
 * DEVICE_nGnRE       1     0000:0100
 * DEVICE_GRE         2     0000:1100
 * NORMAL_NC          3     0100:0100
 * NORMAL             4     1111:1111
 * NORMAL_WT          5     1011:1011
 */
constexpr uint64_t xDefaultMairEl2 =
    (0x00UL << (Mmu::PageMemoryAttribute::kDevicenGnRnE * 8)) |
    (0x04UL << (Mmu::PageMemoryAttribute::kDevicenGnRE * 8)) |
    (0x0cUL << (Mmu::PageMemoryAttribute::kDeviceGRE * 8)) |
    (0x44UL << (Mmu::PageMemoryAttribute::kNormalNC * 8)) |
    (0xffUL << (Mmu::PageMemoryAttribute::kNormal * 8));

// Static array to store page tables
uint64_t s_base_xlat_table[NUM_BASE_LEVEL_ENTRIES]
    __attribute__((aligned(NUM_BASE_LEVEL_ENTRIES * sizeof(uint64_t))));

#define PTE_DESC_TYPE_MASK 3
#define PTE_TABLE_DESC 3
#define PTE_INVALID_DESC 0
#define PTE_BLOCK_DESC 1
#define PTE_TABLE_DESC 3
#define PTE_PAGE_DESC 3

#define PTE_BLOCK_DESC_MEMTYPE(x) ((x) << 2)
#define PTE_BLOCK_DESC_NS (1ULL << 5)
#define PTE_BLOCK_DESC_AP_RW (0ULL << 7)
#define PTE_BLOCK_DESC_AP_RO (1ULL << 7)
#define PTE_BLOCK_DESC_AF (1ULL << 10)
#define PTE_BLOCK_DESC_OUTER_SHARE (2ULL << 8)
#define PTE_BLOCK_DESC_INNER_SHARE (3ULL << 8)
#define PTE_BLOCK_DESC_PXN (1ULL << 53)
#define PTE_BLOCK_DESC_UXN (1ULL << 54)

// ref. https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/TCR-EL1--Translation-Control-Register--EL1-
// ref. https://developer.arm.com/documentation/ddi0601/2025-06/AArch64-Registers/TCR-EL2--Translation-Control-Register--EL2-
// ref. https://logmi.jp/main/technology/323864
/* TCR definitions */
#define TCR_EL1_IPS_SHIFT 32U
#define TCR_EL2_PS_SHIFT 16U
#define TCR_EL3_PS_SHIFT 16U

#define TCR_T0SZ_SHIFT 0U
#define TCR_T0SZ(x) ((64 - (x)) << TCR_T0SZ_SHIFT)

#define TCR_IRGN_NC (0ULL << 8)
#define TCR_IRGN_WBWA (1ULL << 8)
#define TCR_IRGN_WT (2ULL << 8)
#define TCR_IRGN_WBNWA (3ULL << 8)
#define TCR_IRGN_MASK (3ULL << 8)
#define TCR_ORGN_NC (0ULL << 10)
#define TCR_ORGN_WBWA (1ULL << 10)
#define TCR_ORGN_WT (2ULL << 10)
#define TCR_ORGN_WBNWA (3ULL << 10)
#define TCR_ORGN_MASK (3ULL << 10)
#define TCR_SHARED_NON (0ULL << 12)
#define TCR_SHARED_OUTER (2ULL << 12)
#define TCR_SHARED_INNER (3ULL << 12)
#define TCR_TG0_4K (0ULL << 14)
#define TCR_TG0_64K (1ULL << 14)
#define TCR_TG0_16K (2ULL << 14)
#define TCR_EPD1_DISABLE (1ULL << 23)

#define TCR_PS_BITS_4GB 0x0ULL
#define TCR_PS_BITS_64GB 0x1ULL
#define TCR_PS_BITS_1TB 0x2ULL
#define TCR_PS_BITS_4TB 0x3ULL
#define TCR_PS_BITS_16TB 0x4ULL
#define TCR_PS_BITS_256TB 0x5ULL

#if (CONFIG_MMU_PA_BITS == 48)
#define TCR_PS_BITS TCR_PS_BITS_256TB
#elif (CONFIG_MMU_PA_BITS == 44)
#define TCR_PS_BITS TCR_PS_BITS_16TB
#elif (CONFIG_MMU_PA_BITS == 42)
#define TCR_PS_BITS TCR_PS_BITS_4TB
#elif (CONFIG_MMU_PA_BITS == 40)
#define TCR_PS_BITS TCR_PS_BITS_1TB
#elif (CONFIG_MMU_PA_BITS == 36)
#define TCR_PS_BITS TCR_PS_BITS_64GB
#else
#define TCR_PS_BITS TCR_PS_BITS_4GB
#endif

// ***************************************
// VTCR_EL2,  Virtualization Translation Control Register (EL2)
// ***************************************
#define VTCR_NSA (1 << 30)
#define VTCR_NSW (1 << 29)
#define VTCR_VS (0 << 19)
#define VTCR_PS (TCR_PS_BITS << 16)
#define VTCR_TG0 TCR_TG0_4K
#define VTCR_SH0 TCR_SHARED_INNER
#define VTCR_ORGN0 TCR_ORGN_WBWA
#define VTCR_IRGN0 TCR_IRGN_WBWA
#define VTCR_SL0 (1 << 6) // start at level1
#define VTCR_T0SZ TCR_T0SZ(CONFIG_MMU_VA_BITS)

#define VTCR_VALUE                                                 \
  (VTCR_NSA | VTCR_NSW | VTCR_VS | VTCR_PS | VTCR_TG0 | VTCR_SH0 | \
   VTCR_ORGN0 | VTCR_IRGN0 | VTCR_SL0 | VTCR_T0SZ)

// ref. https://developer.arm.com/documentation/101811/0104/The-Memory-Management-Unit--MMU-
// ref. https://zenn.dev/hidenori3/articles/91494881d318a5

uint64_t s_xlat_tables[CONFIG_MMU_MAX_XLAT_TABLES][XLAT_TABLE_ENTRIES]
    __attribute__((aligned(XLAT_TABLE_ENTRIES * sizeof(uint64_t))));

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
  CreatePageTables();

  {
    WRITE_CPU_REG(mair_el2, xDefaultMairEl2); // Cache policies
    WRITE_CPU_REG(tcr_el2, GetTcr(2));
    WRITE_CPU_REG(ttbr0_el2, (uint64_t)s_base_xlat_table);
    WRITE_CPU_REG(vtcr_el2, VTCR_VALUE);
    Arm64Dsb();
    Arm64Isb();
  }

  Enable();
}

void Mmu::Enable()
{
  __asm__ volatile(
      "mrs x0, sctlr_el2\n"
      "orr x0, x0, #1 << 2\n" // set SCTLR.C
      "orr x0, x0, #1 << 0\n" // set SCTLR.M
      "dsb ish\n"
      "isb\n"
      "msr sctlr_el2, x0\n"
      "isb");
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
  for (uint32_t i = 0; i < mmu_regions_guest_.size(); i++)
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
      level = PAGE_TABLE_BASE_LEVEL;
    }
    else if (PteDescType(pte) == PTE_INVALID_DESC)
    {
      uint64_t *new_table = NewXlatTablesEntry();
      SetPteTableDesc(pte, new_table);
      level++;
    }
    else if (PteDescType(pte) == PTE_BLOCK_DESC)
    {
      SplitPteBlockDesc(pte, level);
      level++;
    }
    else if (PteDescType(pte) == PTE_TABLE_DESC)
    {
      level++;
    }
    else
    {
      PANIC("Never reach here!");
    }
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

/* Identify the type of page table entry (PTE)
 *  0b00 - Invalid descriptor (PTE_INVALID_DESC)
 *  0b01 - Block descriptor (PTE_BLOCK_DESC)
 *  0b11 - Table descriptor (PTE_TABLE_DESC) or Page descriptor (PTE_PAGE_DESC)
 */
inline uint8_t Mmu::PteDescType(uint64_t *pte)
{
  return *pte & PTE_DESC_TYPE_MASK;
}

uint64_t *Mmu::NewXlatTablesEntry()
{
  static uint32_t idx = 0;
  ASSERT(idx < CONFIG_MMU_MAX_XLAT_TABLES, "don't have enough xlat tables anymore");
  return (uint64_t *)(s_xlat_tables[idx++]);
}

inline void Mmu::SetPteTableDesc(uint64_t *pte, uint64_t *table)
{
  *pte = PTE_TABLE_DESC | (uint64_t)table;
}

void Mmu::SetPteBlockDesc(uint64_t *pte, uint64_t pa, uint32_t attrs, uint32_t level)
{
  auto desc = pa;
  desc |= (level == 3) ? PTE_PAGE_DESC : PTE_BLOCK_DESC; // Set as a page descriptor for level 3, and as a block descriptor for other levels.
  desc |= (attrs & PageTableEntryL3Desc::kNonSecure) ? PTE_BLOCK_DESC_NS : 0;
  desc |= (attrs & PageTableEntryL3Desc::kRW) ? PTE_BLOCK_DESC_AP_RW : PTE_BLOCK_DESC_AP_RO;
  desc |= PTE_BLOCK_DESC_AF;

  const uint32_t mem_type = GET_LOWER_3_BITS(attrs);
  desc |= PTE_BLOCK_DESC_MEMTYPE(mem_type);
  switch (mem_type)
  {
  case PageMemoryAttribute::kDevicenGnRnE:
  case PageMemoryAttribute::kDevicenGnRE:
  case PageMemoryAttribute::kDeviceGRE:
  {
    desc |= PTE_BLOCK_DESC_OUTER_SHARE;

    desc |= PTE_BLOCK_DESC_PXN;
    desc |= PTE_BLOCK_DESC_UXN;
    break;
  }

  case PageMemoryAttribute::kNormalNC:
  case PageMemoryAttribute::kNormal:
  case PageMemoryAttribute::kNormalWT:
  {
    if (mem_type == PageMemoryAttribute::kNormal)
    {
      desc |= PTE_BLOCK_DESC_INNER_SHARE;
    }
    else
    {
      desc |= PTE_BLOCK_DESC_OUTER_SHARE;
    }

    /* Make Normal RW memory as execute never */
    if (attrs & PageTableEntryL3Desc::kExecuteNever)
    {
      desc |= PTE_BLOCK_DESC_PXN;
    }
    break;
  }
  default:
    break;
  }

  *pte = desc;
}

void Mmu::SplitPteBlockDesc(uint64_t *pte, uint32_t level)
{
  const uint64_t old_block_desc = *pte;

  const int levelshift = GET_VA_LEVEL_SHIFT(level + 1);

  uint64_t *new_table = NewXlatTablesEntry();
  for (uint32_t i = 0; i < XLAT_TABLE_ENTRIES; i++)
  {
    new_table[i] = old_block_desc | (i << levelshift);
    if (level == 2)
    {
      new_table[i] |= PTE_PAGE_DESC;
    }
  }

  /* Overwrite existing PTE set the new table into effect */
  SetPteTableDesc(pte, new_table);
}

uint64_t Mmu::GetTcr(int el)
{
  const uint64_t va_bits = CONFIG_MMU_VA_BITS;
  const uint64_t tcr_ps_bits = TCR_PS_BITS;

  uint64_t tcr;
  if (el == 1)
  {
    tcr = (tcr_ps_bits << TCR_EL1_IPS_SHIFT);
    tcr |= TCR_EPD1_DISABLE;
  }
  else
  {
    tcr = (tcr_ps_bits << TCR_EL3_PS_SHIFT);
  }

  tcr |= TCR_T0SZ(va_bits);
  tcr |= TCR_TG0_4K | TCR_SHARED_INNER | TCR_ORGN_WBWA | TCR_IRGN_WBWA;

  return tcr;
}
