#include "./util.h"
#include "../platforms/qemu/config.h"

// メモリマップドI/O用の関数
static inline void mmio_write(uint32_t reg, uint32_t data) {
    *(volatile uint32_t*)reg = data;
}

static inline uint32_t mmio_read(uint32_t reg) {
    return *(volatile uint32_t*)reg;
}

// UART初期化関数
void uart_init() {
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

// 1文字送信
void uart_putc(unsigned char c) {
    // 送信FIFOが空くのを待つ
    while (mmio_read(UART0_BASE + UART_FR) & FR_TXFF);
    mmio_write(UART0_BASE + UART_DR, c);
}

// 文字列送信
void uart_puts(const char* str) {
    while (*str) {
        uart_putc(*str++);
    }
}

// 1文字受信
unsigned char uart_getc() {
    // 受信FIFOにデータが来るのを待つ
    while (mmio_read(UART0_BASE + UART_FR) & FR_RXFE);
    return mmio_read(UART0_BASE + UART_DR);
}