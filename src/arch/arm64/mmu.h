#ifndef ARCH_ARM64_MMU_H_
#define ARCH_ARM64_MMU_H_

#include <array>
#include <cstdint>

class Mmu
{
public:
    static void Disable();
    static Mmu &Get()
    {
        static Mmu instance;
        return instance;
    }

    struct MmuMapRegion
    {
        // Region Name
        const char *name;
        // Region Base Physical Address
        uint64_t base_pa;
        // Region Base Virtual Address
        uint64_t base_va;
        // Region size
        uint64_t size;
        // Region Attributes
        uint64_t attrs;
    };

    Mmu &operator=(Mmu const &) = delete;

    void Init(const std::array<Mmu::MmuMapRegion, 7> &kernel_regions,
              const std::array<Mmu::MmuMapRegion, 2> &guest_region);

private:
    std::array<MmuMapRegion, 7> mmu_regions_kernel_;
    std::array<MmuMapRegion, 2> mmu_regions_guest_;

    void CheckMmuConfigs();
    void CreatePageTables();
    void InitXlatTables(const MmuMapRegion &region);
    uint64_t *FindPteIndex(uint64_t va, uint32_t level);

    // Get index within given virtual address and translation table level
    inline uint64_t GetVaIndex(uint64_t va, uint32_t level);

    inline uint8_t PteDescType(uint64_t *pte);

    void SetPteBlockDesc(uint64_t *pte, uint64_t pa, uint32_t attrs, uint32_t level);
};

#endif // ARCH_ARM64_MMU_H_