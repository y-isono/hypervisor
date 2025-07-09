#include "arch/arm64/boot_el2.h"
#include "arch/arm64/mmu.h"
#include "arch/arm64/cpu_regs_def.h"
#include "arch/arm64/barriers.h"
#include "macro.h"
#include <cstdint>

/* SCTLR_EL2, System Control Register (EL2) */
// ref. https://developer.arm.com/documentation/ddi0601/2025-06/AArch64-Registers/SCTLR-EL2--System-Control-Register--EL2-
constexpr uint64_t kSctlrEl2LSMAOE = BIT64(29);
constexpr uint64_t kSctlrEl2nTLSMD = BIT64(28);
constexpr uint64_t kSctlrEl2SPAN = BIT64(23);
constexpr uint64_t kSctlrEl2EIS = BIT64(22);
constexpr uint64_t kSctlrEl2WFE_TRAP = BIT64(18);
constexpr uint64_t kSctlrEl2WfiTrap = BIT64(16);
constexpr uint64_t kSctlrEl2ICache = BIT64(12);
constexpr uint64_t kSctlrEl2Eos = BIT64(11);
constexpr uint64_t kSctlrEl2SctlrEl1 = BIT64(5);
constexpr uint64_t kSctlrEl2SaEl1 = BIT64(4);
constexpr uint64_t kSctlrEl2SaEl2 = BIT64(3);
constexpr uint64_t kSctlrEl2InitVal =
    kSctlrEl2LSMAOE | kSctlrEl2nTLSMD | kSctlrEl2SPAN | kSctlrEl2EIS |
    kSctlrEl2WFE_TRAP | kSctlrEl2WfiTrap | kSctlrEl2ICache | kSctlrEl2Eos |
    kSctlrEl2SctlrEl1 | kSctlrEl2SaEl1 | kSctlrEl2SaEl2;

/* HCR_EL2, Hypervisor Configuration Register */
// ref. https://developer.arm.com/documentation/ddi0601/2025-06/AArch64-Registers/HCR-EL2--Hypervisor-Configuration-Register
constexpr uint64_t kHcrEl2RW = BIT64(31);
constexpr uint64_t kHcrEl2TACR = BIT64(21);
constexpr uint64_t kHcrEl2TID3 = BIT64(18);
constexpr uint64_t kHcrEl2TID2 = BIT64(17);
constexpr uint64_t kHcrEl2TID1 = BIT64(16);
constexpr uint64_t kHcrEl2TWE = BIT64(14);
constexpr uint64_t kHcrEl2TWI = BIT64(13);
constexpr uint64_t kHcrEl2AMO = BIT64(5);
constexpr uint64_t kHcrEl2IMO = BIT64(4);
constexpr uint64_t kHcrEl2FMO = BIT64(3);
constexpr uint64_t kHcrEl2SWIO = BIT64(1);
constexpr uint64_t kHcrEl2VM = BIT64(0); // Enable Stage-2 MMU
constexpr uint64_t kHcrEl2InitVal =
    (kHcrEl2RW | kHcrEl2TACR | kHcrEl2TID3 | kHcrEl2TID2 | kHcrEl2TID1 |
     kHcrEl2TWE | kHcrEl2TWI | kHcrEl2AMO | kHcrEl2IMO | kHcrEl2FMO |
     kHcrEl2SWIO | kHcrEl2VM);

void init_hypervisor_registers()
{
    WRITE_CPU_REG(sctlr_el2, kSctlrEl2InitVal);

    Arm64Isb();

    WRITE_CPU_REG(hcr_el2, kHcrEl2InitVal);
}

void boot_el2()
{
    Mmu::Disable();
    init_hypervisor_registers();

    // TODO
    // Mmu::Get().Init(kMmuKernelRegions, kMmuGuestRegions);

    // TODO
}
