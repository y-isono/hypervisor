#include <stdint.h>

static inline void mmio_write(uint32_t reg, uint32_t data);
static inline uint32_t mmio_read(uint32_t reg);
void uart_init();
void uart_putc(unsigned char c);
void uart_puts(const char *str);
unsigned char uart_getc();