#define PERIPHERAL_BASE    0x08000000
#define UART0_BASE        (PERIPHERAL_BASE + 0x01000000)  // 0x09000000

// UART レジスタオフセット
#define UART_DR        0x00 // データレジスタ
#define UART_FR        0x18 // フラグレジスタ
#define UART_IBRD      0x24 // 整数ボーレートレジスタ
#define UART_FBRD      0x28 // 小数ボーレートレジスタ
#define UART_LCRH      0x2C // ラインコントロールレジスタ
#define UART_CR        0x30 // コントロールレジスタ
#define UART_IMSC      0x38 // 割り込みマスクレジスタ

// フラグレジスタのビット
#define FR_TXFF        (1 << 5) // 送信FIFOフル
#define FR_RXFE        (1 << 4) // 受信FIFOエンプティ
