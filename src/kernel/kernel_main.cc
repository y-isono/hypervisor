#include "./util.h"
#include "../platforms/qemu/config.h"

extern "C" void kernel_main()
{
    UART uart;
        
    uart.init();
    uart.puts("Test output from UART class\n");

    while (true)
    {
        unsigned char c = uart.getc();
        uart.putc(c);
    }
}