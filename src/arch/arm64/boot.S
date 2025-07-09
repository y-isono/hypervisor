#include "arch/util_asm.h"

/***
   GLOBAL_FUNCTION
 ***/

.align 4
.section ".text.start"
GLOBAL_FUNCTION(_start)
    // Disable all interrupts
    // DAIF: https://developer.arm.com/documentation/102670/0301/AArch64-registers/AArch64-register-descriptions/AArch64-Generic-System-control-register-description/DAIF--Interrupt-Mask-Bits
    // msr, mrs: https://www.mztn.org/dragon/arm6408cond.html
    msr DAIFSet, #0xf

    // Set stack pointer
    ldr x0, =__stack_end
    mov sp, x0

    // Use only CPU core-0
    // get core id from mpidr_el1: https://developer.arm.com/documentation/ddi0601/2025-03/AArch64-Registers/MPIDR-EL1--Multiprocessor-Affinity-Register
    // get under 8 bit from x0 and it shows CPU core 0
    mrs x0, mpidr_el1
    and x0, x0, #0xff

    // if x0 is zero, then jump to kernel_boot_el2, if not zero, jump to proc_hlt
    // cbz: https://developer.arm.com/documentation/ddi0602/2025-03/Base-Instructions/CBZ--Compare-and-branch-on-zero-?lang=en
    cbz x0, kernel_boot_el2
    b proc_hlt

/***
   PRIVATE_FUNCTION
 ***/

.section ".text"
PRIVATE_FUNCTION(proc_hlt)
    // wfe: wait for event
    // https://developer.arm.com/documentation/ddi0602/2025-03/Base-Instructions/WFE--Wait-for-event-?lang=en
    // entering a low-power state, and looping
    wfe
    b proc_hlt

PRIVATE_FUNCTION(kernel_boot_el2)
    bl bss_clear
    bl boot_el2
    bl kernel_main
    b finish

PRIVATE_FUNCTION(bss_clear)
    // for-loop to set 0 of bss section
    ldr x1, =__bss_start
    ldr x2, =__bss_end
.L__bss_loop:
    sub x3, x2, x1
    cbz x3, .L__bss_loop_done
.L__bss_loop_16:
    cmp x3, #16
    blt .L__bss_loop_8
    stp xzr, xzr, [x1]
    add x1, x1, #16
    b .L__bss_loop
.L__bss_loop_8:
    str xzr, [x1]
    add x1, x1, #8
    b .L__bss_loop
.L__bss_loop_done:
    ret

PRIVATE_FUNCTION(finish)
    wfe
    b finish
