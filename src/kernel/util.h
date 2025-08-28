#include <stdint.h>

class UART {
public:

    // パブリックメソッド
    void init();
    void putc(unsigned char c);
    void puts(const char* str);
    unsigned char getc();

private:

    // プライベートメソッド
    void mmio_write(uint32_t reg, uint32_t data);
    uint32_t mmio_read(uint32_t reg);
};