#include "./util.h"
#include "../platforms/qemu/config.h"

// プライベートメソッドの実装
void UART::mmio_write(uint32_t reg, uint32_t data) {
    *(volatile uint32_t*)reg = data;
}

uint32_t UART::mmio_read(uint32_t reg) {
    return *(volatile uint32_t*)reg;
}

// パブリックメソッドの実装
void UART::init() {
    // UARTを無効化
    mmio_write(UART0_BASE + UART_CR, 0x0);

    // ボーレート設定 (115200)
    // UART_CLK = 48MHz の場合
    // Divisor = UART_CLK / (16 * Baud_rate)
    // 48000000 / (16 * 115200) = 26.0416666...
    mmio_write(UART0_BASE + UART_IBRD, 26);
    mmio_write(UART0_BASE + UART_FBRD, 3);

    // 8bit, FIFOを有効化, パリティなし
    mmio_write(UART0_BASE + UART_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    // UARTを有効化、送受信を有効化
    mmio_write(UART0_BASE + UART_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void UART::putc(unsigned char c) {
    // 送信FIFOが空くのを待つ
    while (mmio_read(UART0_BASE + UART_FR) & FR_TXFF);
    mmio_write(UART0_BASE + UART_DR, c);
}

void UART::puts(const char* str) {
    while (*str) {
        putc(*str++);
    }
}

unsigned char UART::getc() {
    // 受信FIFOにデータが来るのを待つ
    while (mmio_read(UART0_BASE + UART_FR) & FR_RXFE);
    return mmio_read(UART0_BASE + UART_DR);
}
