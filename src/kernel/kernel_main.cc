#include "./util.h"
#include "../platforms/qemu/config.h"

extern "C" void kernel_main() {
    uart_init();
    uart_puts("Test from UART\n");

    while (true) {
        unsigned char c = uart_getc();
        uart_putc(c);
    }
}