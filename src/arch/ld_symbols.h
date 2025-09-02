#ifndef ARCH_LD_SYMBOLS_H_
#define ARCH_LD_SYMBOLS_H_

#include <cstdint>

// ref. src/arch/arm64/scripts/qemu/linker.ld
extern uint8_t __text_start[] __attribute__((visibility("hidden")));
extern uint8_t __text_end[] __attribute__((visibility("hidden")));
extern uint8_t __text_size[] __attribute__((visibility("hidden")));

extern uint8_t __rodata_start[] __attribute__((visibility("hidden")));
extern uint8_t __rodata_end[] __attribute__((visibility("hidden")));
extern uint8_t __rodata_size[] __attribute__((visibility("hidden")));

extern uint8_t __data_start[] __attribute__((visibility("hidden")));
extern uint8_t __data_end[] __attribute__((visibility("hidden")));
extern uint8_t __data_size[] __attribute__((visibility("hidden")));

extern uint8_t __bss_start[] __attribute__((visibility("hidden")));
extern uint8_t __bss_end[] __attribute__((visibility("hidden")));
extern uint8_t __bss_size[] __attribute__((visibility("hidden")));

extern uint8_t __heap_start[] __attribute__((visibility("hidden")));
extern uint8_t __heap_end[] __attribute__((visibility("hidden")));
extern uint8_t __heap_size[] __attribute__((visibility("hidden")));

extern uint8_t __uncached_space_start[] __attribute__((visibility("hidden")));
extern uint8_t __uncached_space_end[] __attribute__((visibility("hidden")));
extern uint8_t __uncached_space_size[] __attribute__((visibility("hidden")));

extern uint8_t __user_space_start[] __attribute__((visibility("hidden")));
extern uint8_t __user_space_end[] __attribute__((visibility("hidden")));
extern uint8_t __user_space_size[] __attribute__((visibility("hidden")));

extern uint8_t __stack_start[] __attribute__((visibility("hidden")));
extern uint8_t __stack_end[] __attribute__((visibility("hidden")));
extern uint8_t __stack_size[] __attribute__((visibility("hidden")));

#endif // ARCH_LD_SYMBOLS_H