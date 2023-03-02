
#define MMIO_BASE 0x3F000000

#define GPFSEL0         ((volatile unsigned int*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile unsigned int*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile unsigned int*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile unsigned int*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))
#define GPSET0          ((volatile unsigned int*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile unsigned int*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile unsigned int*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile unsigned int*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile unsigned int*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile unsigned int*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile unsigned int*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))
#define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))

/* Auxilary mini UART reg */
#define AUX_ENABLE  ((volatile unsigned int*)(MMIO_BASE+0x00215004)) // Enable mini UART
#define AUX_MU_CNTL ((volatile unsigned int*)(MMIO_BASE+0x00215060)) // Control various setting such as the data format, flow control and receiver/transmitter enable
#define AUX_MU_IER  ((volatile unsigned int*)(MMIO_BASE+0x00215044)) // Interrupt Enable/disable Reg
#define AUX_MU_LCR  ((volatile unsigned int*)(MMIO_BASE+0x0021504C)) // Line Control setting Reg
#define AUX_MU_MCR  ((volatile unsigned int*)(MMIO_BASE+0x00215050)) // Modern Control setting Reg
#define AUX_MU_BAUD ((volatile unsigned int*)(MMIO_BASE+0x00215068)) // Baud rate
#define AUX_MU_IIR  ((volatile unsigned int*)(MMIO_BASE+0x00215048)) // Interrupt Information Reg
#define AUX_MU_IO   ((volatile unsigned int*)(MMIO_BASE+0x00215040)) // I/O
#define AUX_MU_LSR  ((volatile unsigned int*)(MMIO_BASE+0x00215054)) // Line Status Reg

char buffer[64] = {'\0'};

void uart_init(){
    *AUX_ENABLE |= 1;   // Enable mini UART
    *AUX_MU_CNTL = 0;   // Disable Tx, Rx During initialization
    *AUX_MU_IER = 0;    // Disable interrupt
    *AUX_MU_LCR = 3;    // Set the data size to 8 bits (char)
    *AUX_MU_MCR = 0;    // Don't need auto flow control
    *AUX_MU_BAUD = 270; // After booting, the system clock is 250MHz
    *AUX_MU_IIR = 6;    // No FIFO.
    *AUX_MU_CNTL = 3;   // Enable Tx, Rx.

    register unsigned int r;
    r = *GPFSEL1;
    r &= ~((0b111 << 12) | (0b111 << 15)); // gpio14, gpio15
    r |= (0b010 << 12) | (0b010 << 15);    // alt5
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r = 150; while(r--) asm volatile("nop");
    *GPPUDCLK0 = (0b001 << 14) | (0b001 << 15);
    r = 150; while(r--) asm volatile("nop");
    *GPPUDCLK0 = 0;        // flush GPIO setup
    *AUX_MU_CNTL = 3;      // enable Tx, Rx
}

/**
 * Receive a character
 */
void uart_send(unsigned int c) {
    /* wait until we can send */
    do{
        asm volatile("nop");
    }while(!(*AUX_MU_LSR & 0x20));
    /* write the character to the buffer */
    *AUX_MU_IO = c;
}

char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{
        asm volatile("nop");
    }while(!(*AUX_MU_LSR & 0x01));
    /* read it and return */

    r = (char)(*AUX_MU_IO);
    uart_send(r);
    /* convert carriage return to newline */
    return r == '\r'?'\n':r; // if "r == '\r'" condition is true, return '\n', otherwise return char r.
}

void uart_puts(char *s) {
    while(*s) {
        /* convert newline to carriage return + newline */
        if(*s == '\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

/**
 * Display a binary value in hexadecimal
 */
void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    uart_puts("0x");
    for(c = 28 ; c >= 0 ; c -= 4) {
        // get highest tetrad
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n > 9 ? 0x37 : 0x30;
        uart_send(n);
    }
}

int _strcmp(char *target){
    char *a = buffer;
    while(*a && *target){
        if(*a++ != *target++)
            return 0;
    }
    return 1;
}

/* mailbox setting */
volatile unsigned int  __attribute__((aligned(16))) mbox[8];

#define MAILBOX_BASE    MMIO_BASE + 0xb880  // 0x0000B880

#define MAILBOX_READ        ((volatile unsigned int *)(MAILBOX_BASE))
#define MAILBOX_STATUS      ((volatile unsigned int *)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE       ((volatile unsigned int *)(MAILBOX_BASE + 0x20))

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000

#define GET_BOARD_REVISION  0x00010002 // need
#define GET_ARM_MEMORY      0x00010005 // need
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

#define MAILBOX_CH_PROP    0x8  // we only use channel 8 in here

int mailbox_call(unsigned char ch)
{
    unsigned int r = (((unsigned int)((unsigned long) & mbox) & ~0xF) | (ch & 0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MAILBOX_WRITE = r;
    
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_EMPTY);
        /* is it a response to our message? */
        if(r == *MAILBOX_READ)
            /* is it a valid successful response? */
            return mbox[1] == REQUEST_SUCCEED;
    }
    return 0;
}

void get_board_revision(){
    mbox[0] = 7 * 4; // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = GET_BOARD_REVISION; // tag identifier
    mbox[3] = 4; // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = END_TAG;

    unsigned int a = mailbox_call(MAILBOX_CH_PROP); // message passing procedure call, you should implement it following the 6 steps provided above.
    
    uart_hex(mbox[5]); // it should be 0xa020d3 for rpi3 b+
    uart_puts("\r\n");
}

void get_ARM_memory(){

    mbox[0] = 8 * 4; // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = GET_ARM_MEMORY; // tag identifier
    mbox[3] = 8; // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    mbox[6] = 0; // value buffer
    // tags end
    mbox[7] = END_TAG;

    unsigned int a = mailbox_call(MAILBOX_CH_PROP); // message passing procedure call, you should implement it following the 6 steps provided above.
    uart_puts("Base Address :");
    uart_hex(mbox[5]);
    uart_puts("\nMemory Size :");
    uart_hex(mbox[6]);
    uart_puts("\r\n");
}

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}

int main(){
    uart_init();
    char *buf = buffer;
    while(*buf) *buf++ = '\0';
    uart_puts("> ");
    while(1){
        *buf++ = uart_getc();
        if(*(buf-1) == '\r' || *(buf-1) == '\n'){
            *buf = '\0';
            if(_strcmp("help\n")){
                uart_puts("\r\n");
                uart_puts("help\t: print all available commands\n");
                uart_puts("hello\t: print Hello World!\n");
                uart_puts("board\t: print board revision info.\n");
                uart_puts("memory\t: print memory info.\n");
                uart_puts("reboot\t: reboot the device\n");
            }
            else if(_strcmp("hello\n")){
                uart_puts("\r\n");
                uart_puts("Hello, World!\r\n");
            }
            else if(_strcmp("memory\n")){
                uart_puts("\r\n");
                uart_puts("# ARM Memory Info #\n");
                get_ARM_memory();
            }
            else if(_strcmp("board\n")){
                uart_puts("\r\n");
                uart_puts("# Board Revision Info #\n");
                get_board_revision();
            }
            else if(_strcmp("reboot\n")){
                uart_puts("start reboot...\n");
                reset(1000);
            }
            else {
                uart_puts("\r\n");
                uart_puts(buffer);
            }
            buf = buffer; // buf = &buffer[0];
            *buf = '\0';
            uart_puts("> ");
        }
    }
    return 0;
}