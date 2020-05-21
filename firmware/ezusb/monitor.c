/*
 * Typdefs for embedded code.
 */
typedef unsigned char byte;
typedef unsigned int  word;
typedef unsigned long dword;
/*
 * Register definitions.
 */
#include <8051.h>
#include "ezusb.h"
/*
 * Timer values.
 */
#define TIMER0_RELOAD   -(24000000/12/1000)
#define TIMER1_RELOAD   0xD9
#define startup _sdcc_external_startup
/*
 * Handy macros.
 */
#ifndef min
#define min(a,b)    ((a)>(b)?(b):(a))
#endif
#ifndef max
#define max(a,b)    ((a)>(b)?(a):(b))
#endif
/*
 * Serial port buffers and control.
 */
#define SERIAL_BUF_SIZE             64
#define SERIAL_BUF_SIZE_MASK        (SERIAL_BUF_SIZE-1)
/*
 * Globals.
 */
volatile dword timer_tick;           // Millisecond counter
volatile byte ser_out_head;
volatile byte ser_out_count;
volatile byte ser_in_head;
volatile byte ser_in_count;
byte  cmdline_len;
/*
 * Serial port data buffers. Stick in unused endpoint 4 buffers.
 */
xdata at 0x7D00 byte ser_in_buf[SERIAL_BUF_SIZE];
xdata at 0x7CC0 byte ser_out_buf[SERIAL_BUF_SIZE];
xdata at 0x7C80 byte cmdline_buf[SERIAL_BUF_SIZE];

/***************************************************************************\
*                                                                           *
*                                      ISRs                                 *
*                                                                           *
\***************************************************************************/

/*
 * Timer 0 ISR.
 */
void timer0_isr(void) interrupt TF0_VECTOR using 1
{
    TL0 = TIMER0_RELOAD;
    TH0 = TIMER0_RELOAD >> 8;
    timer_tick++;
}
/*
 * Serial port 0 ISR.
 */
void serial0_isr(void) interrupt SI0_VECTOR using 1
{
    if (RI)
    {
        /*
         * Character received.
         */
        if (ser_in_count < SERIAL_BUF_SIZE)
            ser_in_buf[(ser_in_head + ser_in_count++) & SERIAL_BUF_SIZE_MASK] = SBUF;
        RI = 0;
    }
    if (TI)
    {
        /*
         * Character sent.  Check for more to send.
         */
        TI           = 0;
        ser_out_head = (ser_out_head + 1) & SERIAL_BUF_SIZE_MASK;
        if (--ser_out_count)
            SBUF = ser_out_buf[ser_out_head];
    }
}

/***************************************************************************\
*                                                                           *
*                              Utility functions                            *
*                                                                           *
\***************************************************************************/

/*
 * Delay routines.
 */
/*
 * If called with a constant, one of the clauses gets optimized out.
 */
#define udelay(d)                                                           \
    do {                                                                    \
    if (d == 1) {                                                           \
        _asm                                                                \
            nop                                                             \
            nop                                                             \
            nop                                                             \
            nop                                                             \
            nop                                                             \
            nop                                                             \
        _endasm;                                                            \
    } else {                                                                \
        neg_2udelay(-((d)-1)/2);                                            \
    }                                                                       \
    } while (0)
void neg_2udelay(word usecs) _naked     // Delay for multiples of 2 usecs
{
    _asm
        mov     a, dpl                  ; 2 cycles
        orl     a, dph                  ; 2 cycles
        jz      0002$                   ; 3 cycles
    0001$:
        inc     dptr                    ; 3 cycled
        mov     a, dpl                  ; 2 cycles
        orl     a, dph                  ; 2 cycles
        nop                             ; 1 cycle
        nop                             ; 1 cycle
        jnz     0001$                   ; 3 cycles
                                        ;---------
                                        ; 12 cycles * 4 cycles/clock / 24 MHZ clock = 2 usec
    0002$:
        ret                             ; 4 cycles
    _endasm;
    usecs = usecs;                      // Shut up warning message.
}
void mdelay(word msecs)
{
    while (msecs--)
        udelay(999);
}
/*
 * Write character to serial port buffer.  Prime serial port if buffer empty.
 */
void serial_put(byte c)
{
    while (ser_out_count >= SERIAL_BUF_SIZE);
    ES = 0;                             // Disable serial0 interrupt
    if (ser_out_count)
        ser_out_buf[(ser_out_head + ser_out_count) & SERIAL_BUF_SIZE_MASK] = c; // Save in buffer
    else
        SBUF = c;                   // Prime serial pump
    ser_out_count++;
    ES = 1;                             // Re-enable interrupt
}
void serial_puts(byte *str)
{
    while (*str)
        serial_put(*str++);
}
/*
 * Read character from serial port buffer.  Return -1 if no data available.
 */
int serial_get(void)
{
    int c;

    if (ser_in_count)                   // Data available
    {
        ES          = 0;                // Disable serial0 interrupt
        c           = ser_in_buf[ser_in_head];
        ser_in_head = (ser_in_head + 1) & SERIAL_BUF_SIZE_MASK;
        ser_in_count--;
        ES = 1;                         // Re-enable interrupt
    }
    else
        c = -1;
    return (c);
}
/***************************************************************************\
*                                                                           *
*                            Process command line                           *
*                                                                           *
\***************************************************************************/
/*
 * Opcode string indeces.
 */
#define STR_ADD     (0*4)
#define STR_ADDC    (1*4)
#define STR_SUBB    (2*4)
#define STR_INC     (3*4)
#define STR_DEC     (4*4)
#define STR_MUL     (5*4)
#define STR_DIV     (6*4)
#define STR_DA      (7*4)
#define STR_ANL     (8*4)
#define STR_ORL     (9*4)
#define STR_XORL    (10*4)
#define STR_CLR     (11*4)
#define STR_CPL     (12*4)
#define STR_SWAP    (13*4)
#define STR_RL      (14*4)
#define STR_RLC     (15*4)
#define STR_RR      (16*4)
#define STR_RRC     (17*4)
#define STR_MOV     (18*4)
#define STR_MOVC    (19*4)
#define STR_MOVX    (20*4)
#define STR_PUSH    (21*4)
#define STR_POP     (22*4)
#define STR_XCH     (23*4)
#define STR_SETB    (24*4)
#define STR_ACALL   (25*4)
#define STR_LCALL   (26*4)
#define STR_RET     (27*4)
#define STR_RETI    (28*4)
#define STR_AJMP    (29*4)
#define STR_SJMP    (30*4)
#define STR_LJMP    (31*4)
#define STR_JC      (32*4)
#define STR_JNC     (33*4)
#define STR_JB      (34*4)
#define STR_JNB     (35*4)
#define STR_JBC     (36*4)
#define STR_JMP     (37*4)
#define STR_JZ      (38*4)
#define STR_JNZ     (39*4)
#define STR_CJNE    (40*4)
#define STR_DJNZ    (41*4)
code char opcode_strs[] = 
{
    'a', 'd', 'd', ' ',
    'a', 'd', 'd', 'c',
    's', 'u', 'b', 'b',
    'i', 'n', 'c', ' ',
    'd', 'e', 'c', ' ',
    'm', 'u', 'l', ' ',
    'd', 'i', 'v', ' ',
    'd', 'a', ' ', ' ',
    'a', 'n', 'l', ' ',
    'o', 'r', 'l', ' ',
    'x', 'o', 'r', 'l',
    'c', 'l', 'r', ' ',
    'c', 'p', 'l', ' ',
    's', 'w', 'a', 'p',
    'r', 'l', ' ', ' ',
    'r', 'l', 'c', ' ',
    'r', 'r', ' ', ' ',
    'r', 'r', 'c', ' ',
    'm', 'o', 'v', ' ',
    'm', 'o', 'v', 'c',
    'm', 'o', 'v', 'x',
    'p', 'u', 's', 'h',
    'p', 'o', 'p', ' ',
    'x', 'c', 'h', ' ',
    's', 'e', 't', 'b',
    'a', 'c', 'a', 'l' | 0x80, // Flag to repeat last character
    'l', 'c', 'a', 'l' | 0x80, // Flag to repeat last character
    'r', 'e', 't', ' ',
    'r', 'e', 't', 'i',
    'a', 'j', 'm', 'p',
    's', 'j', 'm', 'p',
    'l', 'j', 'm', 'p',
    'j', 'c', ' ', ' ',
    'j', 'n', 'c', ' ',
    'j', 'b', ' ', ' ',
    'j', 'n', 'b', ' ',
    'j', 'b', 'c', ' ',
    'j', 'm', 'p', ' ',
    'j', 'z', ' ', ' ',
    'j', 'n', 'z', ' ',
    'c', 'j', 'n', 'e',
    'd', 'j', 'n', 'z'
};
/*
 * Addressing modes.
 */
#define ADDR_NONE   0   // No addressing mode
#define ADDR_IMP    0   // Implicit in opcode
#define ADDR_ACC    1   // Encoded in opcode
#define ADDR_AB     2   // Encoded in opcode
#define ADDR_C      3   // Encoded in opcode
#define ADDR_DPTRIND 4  // Encoded in opcode
#define ADDR_DPTR   5   // Encoded in opcode
#define ADDR_REG    6   // Encoded in opcode
#define ADDR_REGIND 7   // Encoded in opcode
#define ADDR_D8_REL 8   // Encoded in next 2 bytes
#define ADDR_DIR_REL 9  // Encoded in next 2 bytes
#define ADDR_DIRECT 10  // Encoded in next byte
#define ADDR_REL    11  // Encoded in next byte
#define ADDR_NOTBIT 12  // Encoded in mext byte
#define ADDR_BIT    13  // Encoded in next byte
#define ADDR_DATA8  14  // Encoded in next byte
#define ADDR_ADDR11 15  // Encoded in opcode and next byte
#define ADDR_DATA16 16  // Encoded in next 2 bytes
#define ADDR_ADDR16 17  // Encoded in next 2 bytes
#define ADDR_A_DPTR 18  // Encoded in opcode
#define ADDR_A_PC   19  // Encoded in opcode
code byte opcode_mask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF};
/*
 * Addressing modes with byte count added
 */
#define ADSZ_NONE   (0|(0<<6))
#define ADSZ_IMP    (0|(0<<6))
#define ADSZ_ACC    (1|(0<<6))
#define ADSZ_AB     (2|(0<<6))
#define ADSZ_C      (3|(0<<6))
#define ADSZ_DPTRIND (4|(0<<6))
#define ADSZ_DPTR   (5|(0<<6))
#define ADSZ_REG    (6|(0<<6))
#define ADSZ_REGIND (7|(0<<6))
#define ADSZ_D8_REL (8|(2<<6))
#define ADSZ_DIR_REL (9|(2<<6))
#define ADSZ_DIRECT (10|(1<<6))
#define ADSZ_REL    (11|(1<<6))
#define ADSZ_NOTBIT (12|(1<<6))
#define ADSZ_BIT    (13|(1<<6))
#define ADSZ_DATA8  (14|(1<<6))
#define ADSZ_ADDR11 (15|(1<<6))
#define ADSZ_DATA16 (16|(2<<6))
#define ADSZ_ADDR16 (17|(2<<6))
#define ADSZ_A_DPTR (18|(0<<6))
#define ADSZ_A_PC   (19|(0<<6))
#define ADDR_MODE(a)  ((a)&0x1F)
#define ADDR_SIZE(a)    ((a)>>6)
code byte opcode_recs[] =
{
    /* ADD */
    0x28, ADSZ_ACC,     ADSZ_REG,       STR_ADD,
    0x25, ADSZ_ACC,     ADSZ_DIRECT,    STR_ADD,
    0x26, ADSZ_ACC,     ADSZ_REGIND,    STR_ADD,
    0x24, ADSZ_ACC,     ADSZ_DATA8,     STR_ADD,
    /* ADDC */
    0x38, ADSZ_ACC,     ADSZ_REG,       STR_ADDC,
    0x35, ADSZ_ACC,     ADSZ_DIRECT,    STR_ADDC,
    0x36, ADSZ_ACC,     ADSZ_REGIND,    STR_ADDC,
    0x34, ADSZ_ACC,     ADSZ_DATA8,     STR_ADDC,
    /* SUBB */
    0x98, ADSZ_ACC,     ADSZ_REG,       STR_SUBB,
    0x95, ADSZ_ACC,     ADSZ_DIRECT,    STR_SUBB,
    0x96, ADSZ_ACC,     ADSZ_REGIND,    STR_SUBB,
    0x94, ADSZ_ACC,     ADSZ_DATA8,     STR_SUBB,
    /* INC */
    0x04, ADSZ_ACC,     0,              STR_INC,
    0x08, ADSZ_REG,     0,              STR_INC,
    0x05, ADSZ_DIRECT,  0,              STR_INC,
    0x06, ADSZ_REGIND,  0,              STR_INC,
    0xA3, ADSZ_DPTR,    0,              STR_INC,
    /* DEC */
    0x14, ADSZ_ACC,     0,              STR_DEC,
    0x18, ADSZ_REG,     0,              STR_DEC,
    0x15, ADSZ_DIRECT,  0,              STR_DEC,
    0x16, ADSZ_REGIND,  0,              STR_DEC,
    /* MUL */
    0xA4, ADSZ_AB,      0,              STR_MUL,
    /* DIV */
    0x84, ADSZ_AB,      0,              STR_DIV,
    /* DA */
    0xD4, ADSZ_ACC,     0,              STR_DA,
    /* ANL */
    0x58, ADSZ_ACC,     ADSZ_REG,       STR_ANL,
    0x55, ADSZ_ACC,     ADSZ_DIRECT,    STR_ANL,
    0x56, ADSZ_ACC,     ADSZ_REGIND,    STR_ANL,
    0x54, ADSZ_ACC,     ADSZ_DATA8 ,    STR_ANL,
    0x52, ADSZ_DIRECT,  ADSZ_ACC,       STR_ANL,
    0x53, ADSZ_DIRECT,  ADSZ_DATA8,     STR_ANL,
    /* ORL */
    0x48, ADSZ_ACC,     ADSZ_REG,       STR_ORL,
    0x45, ADSZ_ACC,     ADSZ_DIRECT,    STR_ORL,
    0x46, ADSZ_ACC,     ADSZ_REGIND,    STR_ORL,
    0x44, ADSZ_ACC,     ADSZ_DATA8,     STR_ORL,
    0x42, ADSZ_DIRECT,  ADSZ_ACC,       STR_ORL,
    0x43, ADSZ_DIRECT,  ADSZ_DATA8,     STR_ORL,
    /* XORL */
    0x68, ADSZ_ACC,     ADSZ_REG,       STR_XORL,
    0x65, ADSZ_ACC,     ADSZ_DIRECT,    STR_XORL,
    0x66, ADSZ_ACC,     ADSZ_REGIND,    STR_XORL,
    0x64, ADSZ_ACC,     ADSZ_DATA8,     STR_XORL,
    0x62, ADSZ_DIRECT,  ADSZ_ACC,       STR_XORL,
    0x63, ADSZ_DIRECT,  ADSZ_DATA8,     STR_XORL,
    /* CLR */
    0xE4, ADSZ_ACC,     0,              STR_CLR,
    /* CPL */
    0xF4, ADSZ_ACC,     0,              STR_CPL,
    /* SWAP */
    0xC4, ADSZ_ACC,     0,              STR_SWAP,
    /* RL */
    0x23, ADSZ_ACC,     0,              STR_RL,
    /* RLC */
    0x33, ADSZ_ACC,     0,              STR_RLC,
    /* RR */
    0x03, ADSZ_ACC,     0,              STR_RR,
    /* RRC */
    0x13, ADSZ_ACC,     0,              STR_RRC,
    /* MOV */
    0xE8, ADSZ_ACC,     ADSZ_REG,       STR_MOV,
    0xE5, ADSZ_ACC,     ADSZ_DIRECT,    STR_MOV,
    0xE6, ADSZ_ACC,     ADSZ_REGIND,    STR_MOV,
    0x74, ADSZ_ACC,     ADSZ_DATA8,     STR_MOV,
    0xF8, ADSZ_REG,     ADSZ_ACC,       STR_MOV,
    0xA8, ADSZ_REG,     ADSZ_DIRECT,    STR_MOV,
    0x78, ADSZ_REG,     ADSZ_DATA8,     STR_MOV,
    0xF5, ADSZ_DIRECT,  ADSZ_ACC,       STR_MOV,
    0x88, ADSZ_REG,     ADSZ_DIRECT,    STR_MOV,
    0x85, ADSZ_DIRECT,  ADSZ_DIRECT,    STR_MOV,
    0x86, ADSZ_REGIND,  ADSZ_DIRECT,    STR_MOV,
    0x75, ADSZ_DIRECT,  ADSZ_DATA8,     STR_MOV,
    0xF6, ADSZ_REGIND,  ADSZ_ACC,       STR_MOV,
    0xA6, ADSZ_REGIND,  ADSZ_DIRECT,    STR_MOV,
    0x76, ADSZ_REGIND,  ADSZ_DATA8,     STR_MOV,
    0x90, ADSZ_DPTR,    ADSZ_DATA16,    STR_MOV,
    /* MOVC */
    0x93, ADSZ_ACC,     ADSZ_A_DPTR,    STR_MOVC,
    0x83, ADSZ_ACC,     ADSZ_A_PC,      STR_MOVC,
    /* MOVX */
    0xE2, ADSZ_ACC,     ADSZ_REGIND,    STR_MOVX,
    0xE0, ADSZ_ACC,     ADSZ_DPTRIND,   STR_MOVX,
    0xF2, ADSZ_REGIND,  ADSZ_ACC,       STR_MOVX,
    0xF0, ADSZ_DPTRIND, ADSZ_ACC,       STR_MOVX,
    /* PUSH */
    0xC0, ADSZ_DIRECT,  0,              STR_PUSH,
    /* POP */
    0xD0, ADSZ_DIRECT,  0,              STR_POP,
    /* XCH */
    0xC8, ADSZ_ACC,     ADSZ_REG,       STR_XCH,
    0xC5, ADSZ_ACC,     ADSZ_DIRECT,    STR_XCH,
    0xC6, ADSZ_ACC,     ADSZ_REGIND,    STR_XCH,
    0xD6, ADSZ_ACC,     ADSZ_REGIND,    STR_XCH,
    /* CLR */
    0xC3, ADSZ_C,       0,              STR_CLR,
    0xC2, ADSZ_BIT,     0,              STR_CLR,
    /* SETB */
    0xD3, ADSZ_C,       0,              STR_SETB,
    0xD2, ADSZ_BIT,     0,              STR_SETB,
    /* CPL */
    0xB3, ADSZ_C,       0,              STR_CPL,
    0xB2, ADSZ_BIT,     0,              STR_CPL,
    /* ANL C*/
    0x82, ADSZ_C,       ADSZ_BIT,       STR_ANL,
    0xB0, ADSZ_C,       ADSZ_NOTBIT,    STR_ANL,
    /* ORL C*/
    0x72, ADSZ_C,       ADSZ_BIT,       STR_ORL,
    0xA0, ADSZ_C,       ADSZ_NOTBIT,    STR_ORL,
    /* MOV C*/
    0xA2, ADSZ_C,       ADSZ_BIT,       STR_MOV,
    0x92, ADSZ_BIT,     ADSZ_C,         STR_MOV,
    /* CALL */
    0x11, ADSZ_ADDR11,  0,              STR_ACALL,
    0x12, ADSZ_ADDR16,  0,              STR_LCALL,
    /* RET */
    0x22, ADSZ_IMP,     0,              STR_RET,
    0x32, ADSZ_IMP,     0,              STR_RETI,
    /* JMP */
    0x01, ADSZ_ADDR11,  0,              STR_AJMP,
    0x02, ADSZ_ADDR16,  0,              STR_LJMP,
    0x80, ADSZ_REL,     0,              STR_SJMP,
    0x40, ADSZ_REL,     0,              STR_JC,
    0x50, ADSZ_REL,     0,              STR_JNC,
    0x20, ADSZ_BIT,     ADSZ_REL,       STR_JB,
    0x30, ADSZ_BIT,     ADSZ_REL,       STR_JNB,
    0x10, ADSZ_BIT,     ADSZ_REL,       STR_JBC,
    0x73, ADSZ_A_DPTR,  0,              STR_JMP,
    0x60, ADSZ_REL,     0,              STR_JZ,
    0x70, ADSZ_REL,     0,              STR_JNZ,
    /* CJNE */
    0xB5, ADSZ_ACC,     ADSZ_DIR_REL,   STR_CJNE,
    0xB4, ADSZ_ACC,     ADSZ_D8_REL,    STR_CJNE,
    0xB8, ADSZ_REG,     ADSZ_D8_REL,    STR_CJNE,
    0xB6, ADSZ_REGIND,  ADSZ_D8_REL,    STR_CJNE,
    /* DJNZ */
    0xD8, ADSZ_REG,     ADSZ_REL,       STR_DJNZ,
    0xD5, ADSZ_DIRECT,  ADSZ_REL,       STR_DJNZ,
    /* NOP */
    0x00
};
void skip_whitespace(void)
{
    while (cmdline_buf[cmdline_len] && (cmdline_buf[cmdline_len] == ' ' || cmdline_buf[cmdline_len] == '\t'))
           cmdline_len++;
}
void crln(void)
{
    serial_put('\r'); // Carriage return
    serial_put('\n'); // Linefeed
}
void space(byte count)
{
    while (count--)
        serial_put(' ');
}
void hex_put(byte h)
{
    byte nybble = h >> 4;
    serial_put(nybble > 9 ? nybble + 'A' - 10 : nybble + '0');
    nybble = h & 0x0F;
    serial_put(nybble > 9 ? nybble + 'A' - 10 : nybble + '0');
}
void lhex_put(word h)
{
    hex_put(h >> 8);
    hex_put(h);
}
word lhex_get(void)
{
    word value = 0;
    skip_whitespace();
    do
    {
        if (cmdline_buf[cmdline_len] >= '0' && cmdline_buf[cmdline_len] <= '9')
            value = (value << 4) | cmdline_buf[cmdline_len++] - '0';
        else if (cmdline_buf[cmdline_len] >= 'a' && cmdline_buf[cmdline_len] <= 'f')
            value = (value << 4) | cmdline_buf[cmdline_len++] - 'a' + 10;
        else if (cmdline_buf[cmdline_len] >= 'A' && cmdline_buf[cmdline_len] <= 'F')
            value = (value << 4) | cmdline_buf[cmdline_len++] - 'A' + 10;
        else
            return (value);
    } while (1);
}
void direct_put(byte addr)
{
    if (addr == 0x81)
    {
        serial_put('S');
        serial_put('P');
    }
    else if (addr == 0x82)
    {
        serial_put('D');
        serial_put('P');
        serial_put('L');
    }
    else if (addr == 0x83)
    {
        serial_put('D');
        serial_put('P');
        serial_put('H');
    }
    else if (addr == 0xD0)
    {
        serial_put('P');
        serial_put('S');
        serial_put('W');
    }
    else if (addr == 0xE0)
    {
        serial_put('A');
        serial_put('C');
        serial_put('C');
    }
    else if (addr == 0xF0)
    {
        serial_put('B');
    }
    else
    {
        hex_put(addr);
    }
}
void operand_put(byte code *cptr, byte addr_mode)
{
    switch (addr_mode)
    {
        case ADDR_IMP: // Do nothing
            break;
        case ADDR_ACC: // A
            serial_put('A');
            break;
        case ADDR_AB:
            serial_put('A');
            serial_put('B');
            break;
        case ADDR_C:
            serial_put('C');
            break;
        case ADDR_DPTRIND:
            serial_put('@');
        case ADDR_DPTR:
            serial_put('D');
            serial_put('P');
            serial_put('T');
            serial_put('R');
            break;
        case ADDR_REG:
            serial_put('R');
            serial_put((cptr[0] & 0x07) + '0');
            break;
        case ADDR_REGIND:
            serial_put('@');
            serial_put('R');
            serial_put((cptr[0] & 0x01) + '0');
            break;
        case ADDR_D8_REL:
            serial_put('#');
            hex_put(cptr[1]);
            serial_put(',');
            serial_put(' ');
            lhex_put((word)cptr + (signed char)cptr[2] + 2);
            break;
        case ADDR_DIR_REL:
            direct_put(cptr[1]);
            serial_put(',');
            serial_put(' ');
            lhex_put((word)cptr + (signed char)cptr[2] + 2);
            break;
        case ADDR_DIRECT:
            direct_put(cptr[1]);
            break;
        case ADDR_REL:
            lhex_put((word)cptr + (signed char)cptr[1] + 2);
            break;
        case ADDR_NOTBIT:
            serial_put('/');
        case ADDR_BIT:
            if (cptr[1] < 0x80)
            {
                hex_put((cptr[1] >> 3) + 0x20);
                serial_put('.');
                serial_put((cptr[1] & 0x7) + '0');
            }
            else
            {
                direct_put(cptr[1] & 0xF8);
                serial_put('.');
                serial_put((cptr[1] & 0x7) + '0');
            }
            break;
        case ADDR_DATA8:
            serial_put('#');
            hex_put(cptr[1]);
            break;
        case ADDR_ADDR11:
            lhex_put((((word)cptr[0] & 0xE0) << 3) | cptr[1] | (((word)cptr + 2) & 0xF800));
            break;
        case ADDR_DATA16:
        case ADDR_ADDR16:
            hex_put(cptr[1]);
            hex_put(cptr[2]);
            break;
        case ADDR_A_DPTR:
            serial_put('@');
            serial_put('A');
            serial_put('+');
            serial_put('D');
            serial_put('P');
            serial_put('T');
            serial_put('R');
            break;
        case ADDR_A_PC:
            serial_put('@');
            serial_put('A');
            serial_put('+');
            serial_put('P');
            serial_put('C');
            break;
    }
}
byte opcode_put(byte code *cptr)
{
    word i;
    byte j, k = 1;
    hex_put(cptr[0]);
    serial_put(' ');
    for (i = 0; opcode_recs[i] != 0; i += 4)
    {
        if (opcode_recs[i] == (cptr[0] & (opcode_mask[ADDR_MODE(opcode_recs[i + 1])] & opcode_mask[ADDR_MODE(opcode_recs[i + 2])])))
        {
            /*
             * Found opcode.
             */
            j = ADDR_SIZE(opcode_recs[i + 1]);
            if (j)
            {
                j--;
                hex_put(cptr[k++]);
                serial_put(' ');
            }
            if (j)
            {
                hex_put(cptr[k++]);
                serial_put(' ');
            }
            j = ADDR_SIZE(opcode_recs[i + 2]);
            if (j)
            {
                j--;
                hex_put(cptr[k++]);
                serial_put(' ');
            }
            if (j)
            {
                hex_put(cptr[k++]);
                serial_put(' ');
            }
            for (j = 5 - k; j; j--)
                space(3);
            serial_put(opcode_strs[opcode_recs[i + 3] + 0]);
            serial_put(opcode_strs[opcode_recs[i + 3] + 1]);
            serial_put(opcode_strs[opcode_recs[i + 3] + 2]);
            serial_put(opcode_strs[opcode_recs[i + 3] + 3] & 0x7F);
            if (opcode_strs[opcode_recs[i + 3] + 3] & 0x80)
                serial_put(opcode_strs[opcode_recs[i + 3] + 3] & 0x7F);
            else
                serial_put(' ');
            space(2);
            operand_put(cptr, ADDR_MODE(opcode_recs[i + 1]));
            if (opcode_recs[i + 2])
            {
                serial_put(',');
                serial_put(' ');
                operand_put(cptr + ADDR_SIZE(opcode_recs[i + 1]), ADDR_MODE(opcode_recs[i + 2]));
            }
            return (1 + ADDR_SIZE(opcode_recs[i + 1]) + ADDR_SIZE(opcode_recs[i + 2]));
        }
    }
    space(12);
    if (cptr[0] == 0)
    {
        serial_put('n');
        serial_put('o');
        serial_put('p');
    }
    else
    {
        serial_put('?');
        serial_put('?');
        serial_put('?');
    }
    return (1);
}
void do_cmd(void)
{
    byte i, j;
    byte       *dptr;
    static byte xdata *xptr = 0;
    static byte code  *cptr = 0;

    cmdline_len = 0;
    skip_whitespace();
    switch (cmdline_buf[cmdline_len++])
    {
        case 'd': // Dump memory
            dptr = (data byte *)0;
            for (j = 0; j < 8; j++)
            {
                lhex_put(j * 16);
                serial_put(':');
                for (i = 0; i < 16; i++)
                {
                    serial_put(' ');
                    hex_put(dptr[i]);
                }
                crln();
                dptr += 16;
            }
            break;
        case 'x': // Dump external memory
            skip_whitespace();
            if (cmdline_buf[cmdline_len])
                xptr = (xdata byte *)lhex_get();
            for (j = 0; j < 16; j++)
            {
                lhex_put((word)xptr);
                serial_put(':');
                for (i = 0; i < 16; i++)
                {
                    serial_put(' ');
                    hex_put(xptr[i]);
                }
                serial_put(' ');
                serial_put('|');
                serial_put(' ');
                for (i = 0; i < 16; i++)
                    serial_put((xptr[i] >= 32 && xptr[i] <= 128) ? xptr[i] : '.');
                crln();
                xptr += 16;
            }
            break;
        case 'u': // Unassemble code memory
            skip_whitespace();
            if (cmdline_buf[cmdline_len])
                cptr = (code byte *)lhex_get();
            for (j = 0; j < 16; j++)
            {
                lhex_put((word)cptr);
                serial_put(':');
                serial_put(' ');
                cptr += opcode_put(cptr);
                crln();
            }
            break;
        default:
            serial_put('?');
    }
}
void serial_process_cmdline(void)
{
    /*
     * Serial input line editor.
     */
    if (ser_in_count)
    {
        byte c = serial_get();
        switch (c)
        {
            case '\b': // Backspace
                if (cmdline_len) // Don't back up too far
                {
                    serial_put('\b');
                    serial_put(' ');
                    serial_put('\b');
                    cmdline_len--;
                }
                break;
            case '\r': // Carriage return
                crln();
                cmdline_buf[cmdline_len++] = '\0';
                do_cmd();
                cmdline_len = 0;
                crln();
                serial_put(':');
                break;
            default:
                cmdline_buf[cmdline_len++] = c;
                serial_put(c); // Echo
        }
    }
}

/***************************************************************************\
*                                                                           *
*                                Startup routine                            *
*                                                                           *
\***************************************************************************/

byte startup(void)
{
    PORTA_CFG  = 0x00;                                              // Port A all output
    PORTA_OE   = 0xFF;
    PORTB_CFG  = 0x00;                                              // Port B all input
    PORTB_OE   = 0x00;
    PORTC_CFG  = 0x03;                                              // Port C I/O and serial port
    PORTC_OE   = 0xFE;
    TCON       = 0x00;                                              // Stop all timers
    PCON       = 0x80;                                              // Set SMOD (baud/2)
    CKCON      = 0x10;                                              // Set T1M (CLK24/4), fast external mem access
    SCON       = 0x50;                                              // Set serial port0 to mode 1.  Enable receive
    TMOD       = 0x21;                                              // Set timer1 to mode 2 (auto-reload) timer0 to mode 1
    TH0        = TIMER0_RELOAD >> 8;                                // USe timer0 for 1msec system timer
    TL0        = TIMER0_RELOAD;
    TR0        = 1;                                                 // Set timer0 to run
    TH1        = TIMER1_RELOAD;                                     // Generate timing for 9600 baud
    TL1        = 0x00;                                              // Clear timer1
    TR1        = 1;                                                 // Set timer1 to run
    EIE        = 0x00;                                              // Disable external INT2 (USBINT) - we use polling
    IE         = 0x92;                                              // Global, serial0, and timer0 interrupt enable
    MPAGE      = 0x7F;                                              // Set MPAGE to 0x7F - EZUSB register page
    DPSEL      = 0x00;                                              // Set DPSEL to 0
    /*
     * Init external and internal variables.
     */
    timer_tick       = 0;
    ser_out_head     = 0;
    ser_out_count    = 0;
    ser_in_head      = 0;
    ser_in_count     = 0;
    cmdline_len      = 0;
    crln();
    serial_put(':');
    /*
     * All done with initialization.
     */
    return (0);
}

/***************************************************************************\
*                                                                           *
*                               Main loop                                   *
*                                                                           *
\***************************************************************************/

void main(void)
{
    do
        serial_process_cmdline();
    while (1);
}   

