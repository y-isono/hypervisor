OUTPUT_ARCH(aarch64)
ENTRY(_start)

MEMORY
{
  /*
   * U-BOOT loadd address: 0x40280000
   * Physical memory size: 4GB
   */
  RAM      (xrw)  : ORIGIN = 0x40280000, LENGTH = 4 * 1024 * 1024 * 1024
}

SECTIONS {
  __start = .;

  /* text region for Hypervisor */
  .text : ALIGN(0x1000) {
    __text_start = .;
    KEEP(*(.text.start))
    *(.text*)
    . = ALIGN(0x1000);
    __text_end = .;
  } > RAM
  __text_size = __text_end - __text_start;

  /* rodata region for Hypervisor */
  .rodata : ALIGN(0x1000) {
    __rodata_start = .;
    *(.rodata*)
    . = ALIGN(0x1000);
    __rodata_end = .;
  } > RAM
  __rodata_size = __rodata_end - __rodata_start;

  /* data region for Hypervisor */
  .data : ALIGN(0x1000) {
    __data_start = .;
    *(.data*)
    . = ALIGN(0x1000);
    __data_end = .;
  } > RAM
  __data_size = __data_end - __data_start;

  /* bss region for Hypervisor */
  .bss : ALIGN(0x1000) {
    __bss_start = .;
    *(.bss*)
    . = ALIGN(0x1000);
    __bss_end = .;
  } > RAM
  __bss_size = __bss_end - __bss_start;

  /* 64MB stack region for Hypervisor */
  .stack : ALIGN(0x1000) {
    __stack_start = .;
    . = ALIGN(0x1000);
    __stack_end = __stack_start + (1024 * 1024) * 64;
  } > RAM
  __stack_size = __stack_end - __stack_start;
}