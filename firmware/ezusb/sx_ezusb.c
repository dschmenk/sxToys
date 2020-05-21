/***************************************************************************\

    Copyright (C) 2002 - 2004 David Schmenk

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
    USA

    -----------------------------------------------------------------------

    Version 1.3    Sept 15, 2004

    -----------------------------------------------------------------------

    Compile notice:  must compile with --stack-after-data

\***************************************************************************/

/*
 * Firmware version number.
 * Different major version #s have incompatible command sets.
 * Different minor version #s have bug fixes.
 */
#define FIRMWARE_MAJOR_VERSION  1
#define FIRMWARE_MINOR_VERSION  3

#ifndef NUM_SERIAL_PORTS
#define NUM_SERIAL_PORTS        1
#endif
/*
 * Typdefs for embedded code.
 */
#define bit __bit
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
//#define startup _sdcc_external_startup
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
 * CCD waveform macros.
 */
#define CCD_INIT()                                                          \
    do {                                                                    \
        PORTC_OUT = (ccd_field_xor_mask == HX9_FIELD_MAGIC) ? 0xE3 : 0x00;  \
        PORTA_OUT = ccd_rdy;                                                \
    } while (0)
#define CCD_VCLK()                                                          \
    do {                                                                    \
        PORTA_OUT = (ccd_field_xor_mask == HX9_FIELD_MAGIC) ? 0xDD          \
                  : (ccd_field_xor_mask == HX_FIELD_MAGIC)  ? 0xFE : 0xBE;  \
        PORTA_OUT = ccd_rdy;                                                \
        udelay(5);                                                          \
    } while (0)
#define CCD_HCLK()                                                          \
    do {                                                                    \
        PORTA_OUT = ccd_rdy;                                                \
        PORTA_OUT = ccd_hclk;                                               \
    } while (0)
#define CCD_WIPE()                                                          \
    do {                                                                    \
        if (ccd_field_xor_mask == HX9_FIELD_MAGIC) {                        \
            PORTC_OUT = 0xC3;                                               \
            PORTA_OUT = ccd_rdy;                                            \
            PORTA_OUT = 0xCC;                                               \
            udelay(3);                                                      \
            PORTA_OUT = ccd_rdy;                                            \
            PORTC_OUT = 0xE3;                                               \
        } else {                                                            \
            PORTA_OUT = ccd_rdy;                                            \
            PORTA_OUT = (ccd_field_xor_mask == HX_FIELD_MAGIC) ? 0x7A : 0x3A;\
            udelay(3);                                                      \
            PORTA_OUT = ccd_rdy;                                            \
        }                                                                   \
    } while (0)
#define CCD_AMP_ON()                                                        \
    do {                                                                    \
        PORTC_OUT = (ccd_field_xor_mask == HX9_FIELD_MAGIC) ? 0x0C : 0x08;  \
    } while (0)
#define CCD_AMP_OFF()                                                       \
    do {                                                                    \
       PORTC_OUT = (ccd_field_xor_mask == HX9_FIELD_MAGIC) ? 0xE3 : 0x00;   \
    } while (0)
#define CCD_LATCH_FRAME(flags)                                              \
    do {                                                                    \
        PORTA_OUT = ccd_rdy;                                                \
        if (ccd_field_xor_mask == HX9_FIELD_MAGIC) {                        \
            PORTC_OUT = 0xE7;                                               \
            PORTA_OUT = 0x9C;                                               \
            PORTA_OUT = 0x94;                                               \
            udelay(3);                                                      \
            PORTA_OUT = 0x9C;                                               \
        } else {                                                            \
            PORTC_OUT = 0x10;                                               \
            udelay(2);                                                      \
            if (flags & CCD_EXP_FLAGS_FIELD_EVEN) {   /* Latch even field */\
                PORTA_OUT = (ccd_field_xor_mask == HX_FIELD_MAGIC) ? 0xF2 : 0xB2;\
                udelay(5);                                                  \
                PORTA_OUT = ccd_rdy;                                        \
                udelay(2);                                                  \
            }                                                               \
            if (flags & CCD_EXP_FLAGS_FIELD_ODD) {    /* Latch odd field  */\
                PORTA_OUT = (ccd_field_xor_mask == HX_FIELD_MAGIC) ? 0xEA : 0xAA;\
                udelay(5);                                                  \
                PORTA_OUT = ccd_rdy;                                        \
                udelay(2);                                                  \
            }                                                               \
            if (!(flags & CCD_EXP_FLAGS_TDI)) {                             \
                PORTA_OUT = (ccd_field_xor_mask == HX_FIELD_MAGIC) ? 0xFE : 0xBE;\
                PORTA_OUT = ccd_rdy;                                        \
            }                                                               \
            PORTC_OUT = 0x00;                                               \
            udelay(2);                                                      \
        }                                                                   \
    } while (0)

#define CCD_CNVRT_PIXEL()                                                   \
    do {                                                                    \
        PORTA_OUT = ccd_hclk;                                               \
        PORTA_OUT = ccd_cnvrt;                                              \
        PORTA_OUT = ccd_hclk;                                               \
        udelay(1);                      /* Conversion delay             */  \
    } while (0)
#define CCD_LOAD_PIXEL(pp)                                                  \
    do {                                                                    \
        PORTC_OUT = ccd_ld_adc;                                             \
        PORTC_OUT = ccd_ld_msb;                                             \
        udelay(1);                      /* Delay for ADC read           */  \
        pp        = PORTB_PINS ^ ccd_xor_msb;                               \
        PORTC_OUT = ccd_ld_lsb;                                             \
        pp        = ((pp)<<8) | PORTB_PINS;                                 \
    } while (0)
#define CCD_RESET_PIXEL()                                                   \
    do {                                                                    \
        PORTA_OUT = ccd_clear;                                              \
        PORTA_OUT = ccd_clamp;                                              \
    } while (0)
/*
 * Control request fields.
 */
#define USB_REQ_TYPE                0
#define USB_REQ                     1
#define USB_REQ_VALUE_L             2
#define USB_REQ_VALUE_H             3
#define USB_REQ_INDEX_L             4
#define USB_REQ_INDEX_H             5
#define USB_REQ_LENGTH_L            6
#define USB_REQ_LENGTH_H            7
#define USB_REQ_DIR(r)              ((r)&(1<<7))
#define USB_REQ_DATAOUT             0x00
#define USB_REQ_DATAIN              0x80
#define USB_REQ_KIND(r)             ((r)&(3<<5))
#define USB_REQ_VENDOR              (2<<5)
#define USB_REQ_STD                 0
#define USB_REQ_RECIP(r)            ((r)&31)
#define USB_REQ_DEVICE              0x00
#define USB_REQ_IFACE               0x01
#define USB_REQ_ENDPOINT            0x02
#define USB_DATAIN                  0x80
#define USB_DATAOUT                 0x00
/*
 * CCD camera control commands.
 */
#define SXUSB_GET_FIRMWARE_VERSION  255
#define SXUSB_ECHO                  0
#define SXUSB_CLEAR_PIXELS          1
#define SXUSB_READ_PIXELS_DELAYED   2
#define SXUSB_READ_PIXELS           3
#define SXUSB_SET_TIMER             4
#define SXUSB_GET_TIMER             5
#define SXUSB_RESET                 6
#define SXUSB_SET_CCD               7
#define SXUSB_GET_CCD               8
#define SXUSB_SET_STAR2K            9
#define SXUSB_WRITE_SERIAL_PORT     10
#define SXUSB_READ_SERIAL_PORT      11
#define SXUSB_SET_SERIAL            12
#define SXUSB_GET_SERIAL            13
#define SXUSB_CAMERA_MODEL          14
#define SXUSB_LOAD_EEPROM           15
/*
 * Caps bit definitions.
 */
#define SXUSB_CAPS_STAR2K           0x01
#define SXUSB_CAPS_COMPRESS         0x02
#define SXUSB_CAPS_EEPROM           0x04
/*
 * CCD color representation.
 *  Packed colors allow individual sizes up to 16 bits.
 *  2X2 matrix bits are represented as:
 *      0 1
 *      2 3
 */
#define SXUSB_COLOR_PACKED_RGB        0x8000
#define SXUSB_COLOR_PACKED_BGR        0x4000
#define SXUSB_COLOR_PACKED_RED_SIZE   0x0F00
#define SXUSB_COLOR_PACKED_GREEN_SIZE 0x00F0
#define SXUSB_COLOR_PACKED_BLUE_SIZE  0x000F
#define SXUSB_COLOR_MATRIX_ALT_EVEN   0x2000
#define SXUSB_COLOR_MATRIX_ALT_ODD    0x1000
#define SXUSB_COLOR_MATRIX_2X2        0x0000
#define SXUSB_COLOR_MATRIX_RED_MASK   0x0F00
#define SXUSB_COLOR_MATRIX_GREEN_MASK 0x00F0
#define SXUSB_COLOR_MATRIX_BLUE_MASK  0x000F
#define SXUSB_COLOR_MONOCHROME        0x0FFF
/*
 * CCD command options.
 */
#define CCD_EXP_FLAGS_FIELD_ODD     1
#define CCD_EXP_FLAGS_FIELD_EVEN    2
#define CCD_EXP_FLAGS_FIELD_BOTH    (CCD_EXP_FLAGS_FIELD_EVEN|CCD_EXP_FLAGS_FIELD_ODD)
#define CCD_EXP_FLAGS_FIELD_MASK    CCD_EXP_FLAGS_FIELD_BOTH
#define CCD_EXP_FLAGS_NOBIN_ACCUM   4
#define CCD_EXP_FLAGS_NOWIPE_FRAME  8
#define CCD_EXP_FLAGS_TDI           32
#define CCD_EXP_FLAGS_NOCLEAR_FRAME 64
/*
 * HX vs MX magic value.
 */
#define HX9_FIELD_MAGIC             0                               // Special HX9 flag
#define HX_FIELD_MAGIC              (CCD_EXP_FLAGS_FIELD_EVEN<<4)   // AND 0, XOR EVEN
#define MX_FIELD_MAGIC              CCD_EXP_FLAGS_FIELD_BOTH        // AND BOTH, XOR 0

/***************************************************************************\
*                                                                           *
*                              CCD characteristics                          *
*                                                                           *
\***************************************************************************/

#define HX5                     0
#define HX5_WIDTH               660
#define HX5_HEIGHT              494
#define HX5_PIX_WIDTH           (int)(7.4*256)
#define HX5_PIX_HEIGHT          (int)(7.4*256)
#define HX5_HFRONT_PORCH        19
#define HX5_HBACK_PORCH         20
#define HX5_VFRONT_PORCH        14
#define HX5_VBACK_PORCH         1
#define HX5_FIELD_XOR_MASK      (CCD_FIELD_EVEN<<4) // AND 0, XOR EVEN
#define HX9                     1
#define HX9_WIDTH               1300
#define HX9_HEIGHT              1030
#define HX9_PIX_WIDTH           (int)(6.7*256)
#define HX9_PIX_HEIGHT          (int)(6.7*256)
#define HX9_HFRONT_PORCH        30
#define HX9_HBACK_PORCH         56
#define HX9_VFRONT_PORCH        2
#define HX9_VBACK_PORCH         1
#define HX9_FIELD_XOR_MASK  (CCD_FIELD_EVEN<<4) // AND 0, XOR EVEN
#define HX_IMAGE_FIELDS         1
#define MX5                     2
#define MX5_WIDTH               500
#define MX5_HEIGHT              290
#define MX5_PIX_WIDTH           (int)(9.8*256)
#define MX5_PIX_HEIGHT          (int)(12.6*256)
#define MX5_HFRONT_PORCH        23
#define MX5_HBACK_PORCH         30
#define MX5_VFRONT_PORCH        8
#define MX5_VBACK_PORCH         1
#define MX5_FIELD_XOR_MASK      CCD_FIELD_BOTH // AND BOTH, XOR 0
#define MX7                     3
#define MX7_WIDTH               752
#define MX7_HEIGHT              290
#define MX7_PIX_WIDTH           (int)(8.6*256)
#define MX7_PIX_HEIGHT          (int)(16.6*256)
#define MX7_HFRONT_PORCH        25
#define MX7_HBACK_PORCH         40
#define MX7_VFRONT_PORCH        7
#define MX7_VBACK_PORCH         1
#define MX7_FIELD_XOR_MASK      CCD_FIELD_BOTH // AND BOTH, XOR 0
#define MX9                     4
#define MX9_WIDTH               752
#define MX9_HEIGHT              290
#define MX9_PIX_WIDTH           (int)(11.6*256)
#define MX9_PIX_HEIGHT          (int)(22.4*256)
#define MX9_HFRONT_PORCH        25
#define MX9_HBACK_PORCH         40
#define MX9_VFRONT_PORCH        7
#define MX9_VBACK_PORCH         1
#define MX9_FIELD_XOR_MASK      CCD_FIELD_BOTH // AND BOTH, XOR 0

__code byte sx_ccd_hfront_porch[]   = {HX5_HFRONT_PORCH, HX9_HFRONT_PORCH, MX5_HFRONT_PORCH, MX7_HFRONT_PORCH, MX9_HFRONT_PORCH};
__code byte sx_ccd_hback_porch[]    = {HX5_HBACK_PORCH,  HX9_HBACK_PORCH,  MX5_HBACK_PORCH,  MX7_HBACK_PORCH,  MX9_HBACK_PORCH};
__code byte sx_ccd_vfront_porch[]   = {HX5_VFRONT_PORCH, HX9_VFRONT_PORCH, MX5_VFRONT_PORCH, MX7_VFRONT_PORCH, MX9_VFRONT_PORCH};
__code byte sx_ccd_vback_porch[]    = {HX5_VBACK_PORCH,  HX9_VBACK_PORCH,  MX5_VBACK_PORCH,  MX7_VBACK_PORCH,  MX9_VBACK_PORCH};
__code word sx_ccd_width[]          = {HX5_WIDTH,        HX9_WIDTH,        MX5_WIDTH,        MX7_WIDTH,        MX9_WIDTH};
__code word sx_ccd_height[]         = {HX5_HEIGHT,       HX9_HEIGHT,       MX5_HEIGHT,       MX7_HEIGHT,       MX9_HEIGHT};
__code word sx_ccd_pix_width[]      = {HX5_PIX_WIDTH,    HX9_PIX_WIDTH,    MX5_PIX_WIDTH,    MX7_PIX_WIDTH,    MX9_PIX_WIDTH};
__code word sx_ccd_pix_height[]     = {HX5_PIX_HEIGHT,   HX9_PIX_HEIGHT,   MX5_PIX_HEIGHT,   MX7_PIX_HEIGHT,   MX9_PIX_HEIGHT};

/*
 * Serial port buffers and control.
 */
#define SERIAL_BUF_SIZE             64
#define SERIAL_BUF_SIZE_MASK        (SERIAL_BUF_SIZE-1)
/*
 * Globals.
 */
bit   usb_configured;       // Are we configured or not
byte  usb_setup_req[8];     // Lo-mem copy of USB_SETUPDAT
byte  ccd_model;
byte  ccd_hfront_porch;     // CCD parameters
byte  ccd_hback_porch;
byte  ccd_vfront_porch;
byte  ccd_vback_porch;
byte  ccd_field_xor_mask;
bit   ccd_color;
bit   ccd_interleave;
word  ccd_width;
word  ccd_height;
word  ccd_pixel_width;
word  ccd_pixel_height;
word  ccd_color_matrix;
byte  ccd_rdy;
byte  ccd_hclk;
byte  ccd_ld_adc;
byte  ccd_cnvrt;
byte  ccd_clear;
byte  ccd_clamp;
byte  ccd_ld_lsb;
byte  ccd_ld_msb;
byte  ccd_xor_msb;
byte  ccd_read_xbin;
byte  ccd_read_ybin;
word  ccd_read_xoffset;
word  ccd_read_yoffset;
word  ccd_read_width;
word  ccd_read_height;
word  ccd_read_flags;
word  ccd_read_binwidth;
word  ccd_read_binwidth_count;
word  ccd_read_row;
byte  ccd_read_dacbits;
bit   ccd_load_pending;     // Current state of pixel load
bit   ccd_load_done;        // Current state of pixel load
volatile dword ccd_timer;
volatile byte ser_out_head;
volatile byte ser_out_count;
volatile byte ser_in_head;
volatile byte ser_in_count;
/*
 * Serial port data buffers. Stick in unused endpoint 4 buffers.
 */
__xdata __at 0x7D00 byte ser_in_buf[SERIAL_BUF_SIZE];
__xdata __at 0x7CC0 byte ser_out_buf[SERIAL_BUF_SIZE];
/*
 * External global values that can be set/read by the host.
 * These fall in the endpoint 5 buffers which aren't used
 * in this configuration.  This gives 64 bytes of host/8051
 * accessible debug buffer.
 */
__xdata __at 0x7C80 byte ex_usb_dbg[USB_BUF_SIZE];

/***************************************************************************\
*                                                                           *
*                         USB descriptor definitions                        *
*                                                                           *
\***************************************************************************/

__code byte usb_device_descriptor[] =
{
    18,         // Length
    1,          // Type
    0x10, 0x01, // USB Rev 1.1
    0, 0, 0,    // Class, Subclass, Protocol
    USB_BUF_SIZE,
    0x78, 0x12, // Vendor ID
    0x00, 0x01, // Product ID
    0x00, 0x00, // Version
    1, 2, 0,    // Manufacturer, Product, Serial # string ID
    1           // Number of configurations
};
__code byte usb_config_descriptor[] =
{
    9,          // Length
    2,          // Type
   ((sizeof(usb_config_descriptor)
   + sizeof(usb_blockio_interface_descriptor)
   + sizeof(usb_blockio_in_endpoint_descriptor)
   + sizeof(usb_blockio_out_endpoint_descriptor)) & 0xFF),
   ((sizeof(usb_config_descriptor)
   + sizeof(usb_blockio_interface_descriptor)
   + sizeof(usb_blockio_in_endpoint_descriptor)
   + sizeof(usb_blockio_out_endpoint_descriptor)) >> 8),
    1,          // Number of interfaces
    1,          // Configuration Number
    3,          // String ID
    0x80,       // Attributes = bus powered
    100         // Max power is 100x2 = 200mA
};
__code byte usb_blockio_interface_descriptor[] =
{
    9,          // Length
    4,          // Type
    1,          // ID
    0,          // No alternate settings
    2,          // Block IO has 2 endpoints
    0xFF,       // Class = Vendor defined
    0, 0,       // Subclass and protocol
    4           // Interface string ID
};
__code byte usb_blockio_in_endpoint_descriptor[] =
{
    7,          // Length
    5,          // Type
    0x82,       // Address = IN2
    0x02,       // Bulk
    USB_BUF_SIZE, 0,
    0           // Ignorred
};
__code byte usb_blockio_out_endpoint_descriptor[] =
{
    7,          // Length
    5,          // Type
    0x02,       // Address = OUT2
    0x02,       // Bulk
    USB_BUF_SIZE, 0,
    0           // Ignorred
};
__code byte usb_string_unicode[] =
{
    4,          // Length
    3,          // Type
    9, 4        // LANGID - only English strings supported
};
__code byte usb_string_manufacturer[] =
{
    sizeof(usb_string_manufacturer),
    3,          // Type
    'S', 0, 't', 0, 'a', 0, 'r', 0, 'l', 0, 'i', 0, 'g', 0, 'h', 0, 't', 0, ' ', 0, 'X', 0, 'p', 0, 'r', 0, 'e', 0, 's', 0, 's', 0
};
__code byte usb_string_product[] =
{
    sizeof(usb_string_product),
    3,          // Type
    'E', 0, 'C', 0, 'H', 0, 'O', 0, '2', 0
};
__code byte usb_string_configuration[] =
{
    sizeof(usb_string_configuration),
    3,          // Type
    'B', 0, 'l', 0, 'o', 0, 'c', 0, 'k', 0, 'I', 0, 'O', 0
};
__code byte usb_string_interface[] =
{
    sizeof(usb_string_interface),
    3,          // Type
    'B', 0, 'l', 0, 'o', 0, 'c', 0, 'k', 0, 'I', 0, 'O', 0, '0', 0
};
__code byte * __code usb_strings[] =
{
    usb_string_unicode,
    usb_string_manufacturer,
    usb_string_product,
    usb_string_configuration,
    usb_string_interface
};

/***************************************************************************\
*                                                                           *
*                                      ISRs                                 *
*                                                                           *
\***************************************************************************/

/*
 * Timer 0 ISR.
 */
void timer0_isr(void) __interrupt TF0_VECTOR __using 1
{
    TL0 = TIMER0_RELOAD;
    TH0 = TIMER0_RELOAD >> 8;
    if (--ccd_timer == 0)          // Don't count below 0
        TR0 = 0;                        // Stop timer0
}
/*
 * Serial port 0 ISR.
 */
void serial0_isr(void) __interrupt SI0_VECTOR __using 1
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
/*
 * WakeUp ISR.
 */
void wakeup_isr(void) __interrupt 6 __using 1
{
    EICON_5 = 0;                        // Disable wake-up interrupt
    EICON_4 = 0;                        // Clear interrupt
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
        __asm                                                               \
            nop                                                             \
            nop                                                             \
            nop                                                             \
            nop                                                             \
            nop                                                             \
            nop                                                             \
        __endasm;                                                           \
    } else {                                                                \
        neg_2udelay(-((d)-1)/2);                                            \
    }                                                                       \
    } while (0)
void neg_2udelay(word usecs) __naked     // Delay for multiples of 2 usecs
{
    __asm
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
    __endasm;
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
void serial_put(byte port, byte c)
{
    if (port == 0)
    {
        while (ser_out_count >= SERIAL_BUF_SIZE);
        ES = 0;                         // Disable serial0 interrupt
        if (ser_out_count)
            ser_out_buf[(ser_out_head + ser_out_count) & SERIAL_BUF_SIZE_MASK] = c; // Save in buffer
        else
            SBUF = c;                   // Prime serial pump
        ser_out_count++;
        ES = 1;                         // Re-enable interrupt
    }
}
/*
 * Read character from serial port buffer.  Return -1 if no data available.
 */
int serial_get(byte port)
{
    int c;

    if (port == 0 && ser_in_count)      // Data available
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
*                             Basic USB functions                           *
*                                                                           *
\***************************************************************************/

void usb_stall(byte ep)
{
    if (((ep) & 0x7F) == 0x00)
        USB_EP0CS |= 0x03;
    else if (ep == 0x02)
        USB_OUT2CS = USB_CSOUTSTL | USB_CSOUTBSY;
    else if (ep == 0x82)
        USB_IN2CS = USB_CSINSTL | USB_CSINBSY;
}
void usb_hsnak(byte ep)
{
    if (((ep) & 0x7F) == 0)
        USB_EP0CS |= 0x02;
}
void usb_ctl_reply(byte count)
{
    if (count)
        USB_IN0BC = count;              // Initiate transfer
    USB_EP0CS    |= 0x02;               // Handshake
}
void usb_reset_endpoint(byte ep)
{
    if ((ep & 0x7F) == 2)
    {
        if (ep & USB_DATAIN)
        {
            USB_IN3CS = USB_CSINSTL | USB_CSINBSY;
            USB_IN2CS = USB_CSINSTL | USB_CSINBSY;
            USB_IN3BC = 0;
            USB_IN2BC = 0;
            USB_IN2BC = 0;
            USB_IN3CS = USB_CSINBSY;
            USB_IN2CS = USB_CSINBSY;
        }
        else
        {
            USB_OUT2CS = USB_CSOUTBSY;
            USB_OUT3CS = USB_CSOUTBSY;
        }
    }
}
void usb_std_request(void)
{
    word desc_addr;
    bit  stall        = 0;
    byte reply_length = 0;
    switch (usb_setup_req[USB_REQ])
    {
        case 0: // GET_STATUS
            if (USB_REQ_DIR(usb_setup_req[USB_REQ_TYPE])   == USB_REQ_DATAIN
             && USB_REQ_RECIP(usb_setup_req[USB_REQ_TYPE]) == USB_REQ_DEVICE)
            {
                USB_IN0BUF[0] = 1;
                reply_length  = 1;
            }
            else if (USB_REQ_DIR(usb_setup_req[USB_REQ_TYPE])   == USB_REQ_DATAIN
                 && (USB_REQ_RECIP(usb_setup_req[USB_REQ_TYPE]) == USB_REQ_IFACE
                  || USB_REQ_RECIP(usb_setup_req[USB_REQ_TYPE]) == USB_REQ_ENDPOINT))
            {
                USB_IN0BUF[0] = 0;
                USB_IN0BUF[1] = 0;
                reply_length  = 2;
            }
            else
                stall = 1;
            break;
        case 1: // CLEAR_FEATURE
        case 2: // Reserved for future use
        case 3: // SET_FEATURE
        case 4: // Reserved for future use
            stall = 1;
            break;
        case 5: // SET_ADDRESS - handled by EZUSB core
            break;
        case 6: // GET_DESCRIPTOR
            if (USB_REQ_DIR(usb_setup_req[USB_REQ_TYPE])   == USB_REQ_DATAIN
             && USB_REQ_RECIP(usb_setup_req[USB_REQ_TYPE]) == USB_REQ_DEVICE)
            {
                reply_length = usb_setup_req[USB_REQ_LENGTH_L] | ((word)usb_setup_req[USB_REQ_LENGTH_H] << 8);
                if (usb_setup_req[USB_REQ_VALUE_H] == 1) // DEVICE
                {
                    desc_addr = (word)usb_device_descriptor;
                }
                else if (usb_setup_req[USB_REQ_VALUE_H] == 2) // CONFIGURATION
                {
                    desc_addr = (word)usb_config_descriptor;
                }
                else if (usb_setup_req[USB_REQ_VALUE_H] == 3) // STRING
                {
                    if (usb_setup_req[USB_REQ_VALUE_L] <= 4)
                        desc_addr = (word)usb_strings[usb_setup_req[USB_REQ_VALUE_L]];
                    else
                        stall = 1;
                }
                else
                    stall = 1;
            }
            else
                stall = 1;
            if (!stall)
            {
                USB_SUDPTRH  = desc_addr >> 8;
                USB_SUDPTRL  = desc_addr;
                reply_length = 0;
            }
            break;
        case 7: // SET_DESCRIPTOR
            stall = 1;
            break;
        case 8: // GET_CONFIGURATION
            if (USB_REQ_DIR(usb_setup_req[USB_REQ_TYPE])   == USB_REQ_DATAIN
             && USB_REQ_KIND(usb_setup_req[USB_REQ_TYPE])  == 0
             && USB_REQ_RECIP(usb_setup_req[USB_REQ_TYPE]) == 0
             && usb_setup_req[USB_REQ_VALUE_L]             == 0
             && usb_setup_req[USB_REQ_VALUE_H]             == 0
             && usb_setup_req[USB_REQ_INDEX_L]             == 0
             && usb_setup_req[USB_REQ_INDEX_H]             == 0
             && usb_setup_req[USB_REQ_LENGTH_H]            == 0)
            {
                USB_IN0BUF[0] = usb_configured;
                reply_length  = usb_setup_req[USB_REQ_LENGTH_L];
            }
            else
                stall = 1;
            break;
        case 9: // SET_CONFIGURATION
            if (USB_REQ_DIR(usb_setup_req[USB_REQ_TYPE])   == USB_REQ_DATAOUT
             && USB_REQ_KIND(usb_setup_req[USB_REQ_TYPE])  == 0
             && USB_REQ_RECIP(usb_setup_req[USB_REQ_TYPE]) == 0
             && usb_setup_req[USB_REQ_VALUE_L]             <  2 // Only allow 0 and 1
             && usb_setup_req[USB_REQ_VALUE_H]             == 0
             && usb_setup_req[USB_REQ_INDEX_L]             == 0
             && usb_setup_req[USB_REQ_INDEX_H]             == 0
             && usb_setup_req[USB_REQ_LENGTH_L]            == 0
             && usb_setup_req[USB_REQ_LENGTH_H]            == 0)
            {
                usb_configured = usb_setup_req[USB_REQ_VALUE_L];
                USB_OUT2BC = 0; // Accept data on EP2
            }
            else
                stall = 1;
            break;
        case 10: // GET_INTERFACE
            break;
        case 11: // SET_INTERFACE
            break;
        case 12: // SYNCH_FRAME
        default:
            stall = 1;
    }
    if (stall)
        usb_stall(0);
    else
        usb_ctl_reply(reply_length);
}

/***************************************************************************\
*                                                                           *
*                             CCD camera functions                          *
*                                                                           *
\***************************************************************************/

/*
 * Predeclare some routines.
 */
void ccd_end_frame(void);
/*
 * Clear horizontal shift registers.
 */
void ccd_clear_row(void)
{
    __asm
        mov     a, _ccd_hfront_porch
        add     a, _ccd_hback_porch
        add     a, _ccd_width
        mov     r2, a
        clr     a
        addc    a, (_ccd_width + 1)
        mov     r3, a
        mov     r0, #_PORTA_OUT
        mov     r1, #16
        mov     r5, #0
        mov     r6, _ccd_rdy
        mov     r7, _ccd_hclk
        clr     c
    0010$:
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, _ccd_clear
        movx    @r0, a
        mov     a, _ccd_clamp
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, _ccd_clear
        movx    @r0, a
        mov     a, _ccd_clamp
        movx    @r0, a
        mov     a, r2
        subb    a, r1
        mov     r2, a
        mov     a, r3
        subb    a, r5
        mov     r3, a
        jnc     0010$
        mov     a, r7
        movx    @r0, a
    __endasm;
}
/*
 * Begin a new row.
 */
void ccd_begin_row()
{
    byte i;
    ccd_read_binwidth_count = ccd_read_binwidth - 1;
    ccd_read_row           += ccd_read_ybin;
    i                       = ccd_read_ybin;                         // Bin vertically
    do
    {
        CCD_VCLK();
    } while (--i);
    __asm
        mov     a, _ccd_hfront_porch
        jz      0000$
        dec     a
    0000$:
        add     a, _ccd_read_xoffset
        mov     r2, a
        clr     a
        addc    a, (_ccd_read_xoffset + 1)
        mov     r3, a
        mov     r0, #_PORTA_OUT
        mov     r4, #0xFF
        mov     r1, _ccd_clear
        mov     r5, _ccd_clamp
        mov     r6, _ccd_rdy
        mov     r7, _ccd_hclk
    0001$:
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r1
        movx    @r0, a
        mov     a, r5
        movx    @r0, a
        mov     a, r4
        add     a, r2
        mov     r2, a
        mov     a, r4
        addc    a, r3
        mov     r3, a
        jc      0001$
        mov     a, r7
        movx    @r0, a
    __endasm;

//    if ((ccd_model & 0x0F) == 0x05) // MX-5 or HX-5
    {
        /*
         * The A/D converter will digitise the current charge when requested.
         * However, the data subsequently read out is the result of the PREVIOUS
         * digitisation. So at the start of a row, do a dummy read to put everything
         * back in sync.
         * EBS 21 July 2003
         */
        __asm
            clr     _EA                                             ; Turn global interrupts off
            mov     r0, #_PORTA_OUT
            mov     a, _ccd_clear                                   ; RESET pixel output
            movx    @r0, a
            mov     a, _ccd_clamp                                   ; CLAMP pixel output
            movx    @r0, a
            mov     r5, _ccd_rdy
            mov     r6, _ccd_hclk
            mov     r7, _ccd_read_xbin
        0002$:
            mov     a, r5                                           ; Shift first pixel into readout register
            movx    @r0, a
            mov     a, r6
            movx    @r0, a
            djnz    r7, 0002$
            mov     dptr, #_PORTC_OUT
            mov     r1, #_PORTB_PINS
            mov     r0, #_PORTA_OUT
            mov     a, _ccd_cnvrt                                   ; Convert pixel
            movx    @r0, a
            mov     a, _ccd_clear
            movx    @r0, a
            mov     a, _ccd_clamp                                   ; CLAMP pixel output
            movx    @r0, a
            mov     a, _ccd_ld_adc                                  ; Load pixel from ADC and select LSB
            movx    @dptr, a
            movx    a, @r1
            mov     a, _ccd_ld_msb
            movx    @dptr, a
            movx    a, @r1
            xrl     a, _ccd_xor_msb
            setb    _EA                                             ; Turn global interrupts back on
        __endasm;
    }                                    
}
/*
 * End of row.
 */
void ccd_end_row(void)
{
    __asm
        mov     a, _ccd_hback_porch
        add     a, _ccd_width
        mov     r2, a
        clr     a
        addc    a, (_ccd_width + 1)
        mov     r3, a
        mov     a, r2
        subb    a, _ccd_read_xoffset
        mov     r2, a
        mov     a, r3
        subb    a, (_ccd_read_xoffset + 1)
        mov     r3, a
        jc      0002$
        mov     a, r2
        subb    a, _ccd_read_width
        mov     r2, a
        mov     a, r3
        subb    a, (_ccd_read_width + 1)
        mov     r3, a
        jc      0002$
        mov     r0, #_PORTA_OUT
        mov     r1, #8
        mov     r4, #0
        mov     r5, _ccd_clear
        mov     r6, _ccd_rdy
        mov     r7, _ccd_hclk
        clr     c
    0001$:
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r5
        movx    @r0, a
        mov     a, _ccd_clamp
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        mov     a, r7
        movx    @r0, a
        mov     a, r5
        movx    @r0, a
        mov     a, _ccd_clamp
        movx    @r0, a
        mov     a, r2
        subb    a, r1
        mov     r2, a
        mov     a, r3
        subb    a, r4
        mov     r3, a
        jnc     0001$
    0002$:
    __endasm;
    if (ccd_read_row >= ccd_read_height)                             // Check for complete frame
        ccd_end_frame();
    else
        ccd_begin_row();
}
/*
 * Wipe the pixel charge.
 */
void ccd_wipe_frame(void)
{
    CCD_AMP_OFF();
    CCD_WIPE();
}
/*
 * Clock out frame row by row.
 */
void ccd_clear_frame(void)
{
    word row = ccd_vfront_porch + ccd_height + ccd_vback_porch;
    CCD_AMP_ON();
    while (row--)
    {
        CCD_VCLK();                                                 // Double clock the vertical registers
        CCD_VCLK();                                                 // Ensure clean frame with bright light
        if (!((byte)row & 0x1F))
            ccd_clear_row();
    }
    ccd_clear_row();
    CCD_AMP_OFF();
}
/*
 * Latch a frame and prepare for reading.
 */
void ccd_latch_frame(word flags)
{
    byte field_bits = ((byte)flags & (ccd_field_xor_mask & 0x0F)) ^ (ccd_field_xor_mask >> 4);
    if (field_bits == 0)
        field_bits = ccd_field_xor_mask & 0x0F;
    field_bits |= (byte)flags & CCD_EXP_FLAGS_TDI;
    CCD_LATCH_FRAME(field_bits);
}
/*
 * Begin reading frame. Clock out rows until window Y offset reached unless in TDI mode.
 */
void ccd_begin_frame(void)
{
    word offset;

    ccd_load_pending = 0;
    ccd_load_done    = 0;
    ccd_read_row     = ccd_read_ybin - 1;
    if (!(ccd_read_flags & (CCD_EXP_FLAGS_TDI | CCD_EXP_FLAGS_NOCLEAR_FRAME)))
        ccd_clear_frame();
    ccd_latch_frame(ccd_read_flags);
    CCD_AMP_ON();
    if (!(ccd_read_flags & CCD_EXP_FLAGS_TDI))
    {
        /*
         * Clear every line in front porch.
         * The MX7 really holds a charge here.
         */
        offset = ccd_vfront_porch - ((ccd_field_xor_mask == HX9_FIELD_MAGIC) ? 0 : 1);
        while (offset--)
        {
            CCD_VCLK();
            ccd_clear_row();
        }
        offset = ccd_read_yoffset;
        while (offset--)
        {
            CCD_VCLK();
            if (!((byte)offset & 0x0F))
                ccd_clear_row();
        }
    }
    offset = 12;
    while (offset--)
        ccd_clear_row();
    ccd_begin_row();
}
/*
 * End reading a frame.
 */
void ccd_end_frame(void)
{
    CCD_AMP_OFF();
    ccd_load_done = 1;
}
/*
 * Load as many pixels as will fit into a transfer buffer.
 * On the last transfer, copy as many as will fit and turn off amplifier.
 *
 * Does windowing and binning.
 */
void ccd_load_pixels(void)
{
    /*
     * Read bulk buffer number of pixels.
     */
    __asm
        clr     _EA                                                 ; Turn global interrupts off
        mov     r0, #_PORTA_OUT
        mov     a, _ccd_clear                                       ; RESET pixel output
        movx    @r0, a
        mov     a, _ccd_clamp                                       ; CLAMP pixel output
        movx    @r0, a
        mov     r5, _ccd_rdy
        mov     r6, _ccd_hclk
        mov     r7, _ccd_read_xbin
    0000$:
        mov     a, r5                                               ; Shift first pixel into readout register
        movx    @r0, a
        mov     a, r6
        movx    @r0, a
        djnz    r7, 0000$
        mov     r0, #_AUTOPTRH                                      ; Setup AUTOPTR to point to IN2BUF
        mov     a, #(_USB_IN2BUF >> 8)
        movx    @r0, a
        inc     r0
        mov     a, #_USB_IN2BUF
        movx    @r0, a
        mov     dptr, #_PORTC_OUT
        mov     r1, #_PORTB_PINS
        mov     r2, #(USB_BUF_SIZE/2 - 1)                           ; Special processing on last pixel
        mov     r3, _ccd_read_binwidth_count
        mov     r4, (_ccd_read_binwidth_count + 1)
        inc     r4
        sjmp    0011$
        0010$:
            mov     r0, #_AUTODATA
            movx    a, @r1
            movx    @r0, a
            mov     a, _ccd_ld_msb
            movx    @dptr, a
            movx    a, @r1
            xrl     a, _ccd_xor_msb
            movx    @r0, a
        0011$:
            mov     r0, #_PORTA_OUT
            mov     a, _ccd_cnvrt                                   ; Convert pixel
            movx    @r0, a
            mov     a, _ccd_clear
            movx    @r0, a
            mov     a, _ccd_clamp                                   ; CLAMP pixel output
            movx    @r0, a
            mov     a, _ccd_ld_adc                                  ; Load pixel from ADC and select LSB
            movx    @dptr, a
            dec     r3
            mov     r7, _ccd_read_xbin
        0012$:                                                      ; Bin horizontally
            mov     a, r5
            movx    @r0, a
            mov     a, r6
            movx    @r0, a
            djnz    r7, 0012$
            ;
            ; Insert loop logic here to overlap with ADC.
            ;
            cjne    r3, #0xFF, 0013$
                djnz    r4, 0013$
                    sjmp    0014$
        0013$:
            djnz    r2, 0010$
            mov     r0, #_AUTODATA
            movx    a, @r1
            movx    @r0, a
            mov     a, _ccd_ld_msb
            movx    @dptr, a
            movx    a, @r1
            xrl     a, _ccd_xor_msb
            movx    @r0, a
            sjmp    0020$
                ;
                ; End of row processing
                ;
            0014$:
                mov     r0, #_AUTODATA
                movx    a, @r1
                movx    @r0, a
                mov     a, _ccd_ld_msb
                movx    @dptr, a
                movx    a, @r1
                xrl     a, _ccd_xor_msb
                movx    @r0, a
                push    ar2
                setb    _EA                                         ; Turn global interrupts back on
                lcall   _ccd_end_row
                pop     ar2
                jnb     _ccd_load_done, 0015$
                    ;
                    ; Pixel read complete. Send partial buffer.
                    ;
                    mov     r0, #_USB_IN2BC                         ; USB_IN2BC = buf_size
                    mov     a,  #USB_BUF_SIZE/2
                    clr     c
                    subb    a, r2
                    rl      a
                    movx    @r0, a
                    ret
            0015$:
                clr     _EA                                         ; Turn global interrupts off
                mov     r0, #_PORTA_OUT
                mov     a, _ccd_clear                               ; RESET pixel output
                movx    @r0, a
                mov     a, _ccd_clamp                               ; CLAMP pixel output
                movx    @r0, a
                mov     r5, _ccd_rdy
                mov     r6, _ccd_hclk
                mov     r7, _ccd_read_xbin                          ; Shift first pixel into readout register
            0016$:
                mov     a, r5
                movx    @r0, a
                mov     a, r6
                movx    @r0, a
                djnz    r7, 0016$
                mov     dptr, #_PORTC_OUT
                mov     r1, #_PORTB_PINS
                mov     r3, _ccd_read_binwidth_count
                mov     r4, (_ccd_read_binwidth_count + 1)
                inc     r4
                djnz    r2, 0011$
        ;
        ; Last pixel in buffer.
        ;
    0020$:
        mov     r0, #_PORTA_OUT
        mov     a, _ccd_cnvrt                                       ; Convert pixel
        movx    @r0, a
        mov     a, _ccd_clear                                       ; RESET pixel output
        movx    @r0, a
        mov     a, _ccd_clamp                                       ; CLAMP pixel output
        movx    @r0, a
        mov     a, _ccd_ld_adc                                      ; Load pixel from ADC and select LSB
        movx    @dptr, a
        dec     r2                                                  ; r2 => 0xFF
        mov     a, _ccd_hclk
        movx    @r0, a
        mov     a, r2
        add     a, r3                                               ; binwidth += -1
        mov     _ccd_read_binwidth_count, a
        mov     a, r2
        dec     r4
        addc    a, r4
        mov     (_ccd_read_binwidth_count + 1), a
        anl     a, _ccd_read_binwidth_count
        add     a, #2
        mov     r2, a
        mov     r0, #_AUTODATA
        movx    a, @r1
        movx    @r0, a
        mov     a, _ccd_ld_msb
        movx    @dptr, a
        movx    a, @r1
        xrl     a, _ccd_xor_msb
        movx    @r0, a
        mov     r0, #_USB_IN2BC                                     ; USB_IN2BC = bulk_buf_size
        mov     a,  #USB_BUF_SIZE
        movx    @r0, a
        djnz    r2, 0021$
            ;
            ; Last pixel in buf also last pixel in row.
            ;
            lcall   _ccd_end_row
    0021$:
        setb    _EA                                                 ; Turn global interrupts back on
    __endasm;
}
void ccd_reset(void)
{
    TR0              = 0;
    ccd_timer        = 0;
    ccd_load_pending = 0;
    ccd_load_done    = 1;
    ccd_clear_frame();
    ccd_wipe_frame();
}
void ccd_set_default_params(void)
{
    byte i;
    /*
     * Standard camera pinouts.
     */
    ccd_rdy     = 0xBA;
    ccd_hclk    = 0xFA;
    ccd_cnvrt   = 0xF8;
    ccd_clear   = 0xFA;  // Older cameras don't have seperate reset/clamp but must do to trigger conversion
    ccd_clamp   = 0xDA;  // Reset and clamp at the same time
    ccd_ld_adc  = 0x2C;
    ccd_ld_lsb  = 0x28;
    ccd_ld_msb  = 0x08;
    ccd_xor_msb = 0x00;
    switch (ccd_model & 0x7F)
    {
        case 0x05:
            i = HX5;
            /*
             * The phase of the horizontal clocking is the opposite
             * for the HX516 - EBS 12 March 2004.
             */
            ccd_rdy     = 0xFA;
            ccd_hclk    = 0xBA;
            ccd_cnvrt   = 0xB8;
            ccd_clear   = 0xBA;
            ccd_clamp   = 0x9A;            
            ccd_field_xor_mask = HX_FIELD_MAGIC;
            ccd_interleave     = 0;
            break;
        case 0x09:
            i = HX9;
            /*
             * The HX9 uses a different pinout than the other cameras.
             */
            ccd_rdy            = 0xFC;
            ccd_hclk           = 0xFE;
            ccd_cnvrt          = 0x7E;
            ccd_clear          = 0xFA;
            ccd_clamp          = 0xDE;
            ccd_ld_adc         = 0x04;
            ccd_ld_lsb         = 0x0C;
            ccd_ld_msb         = 0x1C;
            ccd_xor_msb        = 0x80;
            ccd_field_xor_mask = HX9_FIELD_MAGIC;
            ccd_interleave     = 0;
            break;
        case 0x47:
            i = MX7;
            ccd_field_xor_mask = MX_FIELD_MAGIC;
            ccd_interleave     = 1;
            break;
        case 0x49:
            i = MX9;
            ccd_field_xor_mask = MX_FIELD_MAGIC;
            ccd_interleave     = 1;
            break;
        case 0x45:
        default:
            i = MX5;
            ccd_field_xor_mask = MX_FIELD_MAGIC;
            ccd_interleave     = 1;
            break;
    }
    ccd_hfront_porch = sx_ccd_hfront_porch[i];
    ccd_hback_porch  = sx_ccd_hback_porch[i];
    ccd_width        = sx_ccd_width[i];
    ccd_vfront_porch = sx_ccd_vfront_porch[i];
    ccd_vback_porch  = sx_ccd_vback_porch[i];
    ccd_height       = sx_ccd_height[i];
    ccd_pixel_width  = sx_ccd_pix_width[i];
    ccd_pixel_height = sx_ccd_pix_height[i];
    ccd_color        = ccd_model & 0x80 ? 1 : 0;

    ccd_color_matrix = ccd_color ? (ccd_vfront_porch & 1 ? (/*SXUSB_COLOR_MATRIX_2X2 | */SXUSB_COLOR_MATRIX_ALT_ODD | 0x5B6) : (/*SXUSB_COLOR_MATRIX_2X2 | */SXUSB_COLOR_MATRIX_ALT_ODD | 0x6B5))
                                 : SXUSB_COLOR_MONOCHROME;
}
signed char sx_request(__xdata byte *param_buf, __xdata byte *in_buf)
{
    signed char count = 0;
    if (usb_configured)
        switch (usb_setup_req[USB_REQ])
        {
            case SXUSB_ECHO: // Always send data ack EP2
                while (USB_IN2CS & USB_CSINBSY);
                if (USB_REQ_DIR(usb_setup_req[USB_REQ_TYPE]) == USB_REQ_DATAOUT && usb_setup_req[USB_REQ_LENGTH_L])
                {
                    if (usb_setup_req[USB_REQ_LENGTH_L] > USB_BUF_SIZE)
                        usb_setup_req[USB_REQ_LENGTH_L] = USB_BUF_SIZE;
                    for (count = 0; count < usb_setup_req[USB_REQ_LENGTH_L]; count++)
                        USB_OUT2BUF[count] = param_buf[count];
                }
                else
                {
                    USB_OUT2BUF[0] = usb_setup_req[USB_REQ_VALUE_L];
                    USB_OUT2BUF[1] = usb_setup_req[USB_REQ_VALUE_H];
                    count     = 2;
                }
                USB_IN2BC = count;                              // Send data to host
                count     = 0;
                break;
            case SXUSB_CLEAR_PIXELS:
                if (!(usb_setup_req[USB_REQ_VALUE_L] & (CCD_EXP_FLAGS_TDI | CCD_EXP_FLAGS_NOCLEAR_FRAME)))
                {
                    if (usb_setup_req[USB_REQ_VALUE_L] & (ccd_field_xor_mask & 0x0F))
                        ccd_latch_frame((word)usb_setup_req[USB_REQ_VALUE_L]);
                    ccd_clear_frame();
                    if (!(usb_setup_req[USB_REQ_VALUE_L] & CCD_EXP_FLAGS_NOWIPE_FRAME))
                        ccd_wipe_frame();
                }
                break;
            case SXUSB_READ_PIXELS_DELAYED:
                if (!ccd_load_done)                                 // Hmmm, still outstanding pixels
                {
                    TR0 = 0;
                    usb_reset_endpoint(2 | USB_DATAIN);             //   I guess we'll reset for now.
                    ccd_load_done = 1;
                }
                ccd_load_pending = 0;
                ccd_timer        = *((__xdata dword *)&param_buf[10]);
                /*
                 * Make sure timer is turned off.
                 */
                TR0 = 0;
                /*
                 * Clear CCD.
                 */
                if (!(usb_setup_req[USB_REQ_VALUE_L] & (CCD_EXP_FLAGS_TDI | CCD_EXP_FLAGS_NOCLEAR_FRAME)))
                {
                    if (usb_setup_req[USB_REQ_VALUE_L] & (ccd_field_xor_mask & 0x0F))
                        ccd_latch_frame((word)usb_setup_req[USB_REQ_VALUE_L]);
                    ccd_clear_frame();
                    if (!(usb_setup_req[USB_REQ_VALUE_L] & CCD_EXP_FLAGS_NOWIPE_FRAME))
                        ccd_wipe_frame();
                }
                if (ccd_timer)
                {
                    ccd_load_pending = 1;
                    /*
                     * Turn on millisecond timer if delay.
                     */
                    TL0 = (byte)TIMER0_RELOAD;
                    TH0 = (byte)(TIMER0_RELOAD >> 8);
                    TR0 = 1;
                }
                /*
                 * Fall through to READ_PIXELS.
                 */
            case SXUSB_READ_PIXELS:
                if (!ccd_load_done)                                 // Hmmm, still outstanding pixels
                {
                    TR0 = 0;
                    usb_reset_endpoint(2 | USB_DATAIN);             //   I guess we'll reset for now.
                    ccd_load_pending = 0;
                    ccd_load_done    = 1;
                }
                ccd_read_flags   = usb_setup_req[USB_REQ_VALUE_L] | ((word)usb_setup_req[USB_REQ_VALUE_H] << 8);
                ccd_read_dacbits = usb_setup_req[USB_REQ_INDEX_L];
                ccd_read_xoffset = *((__xdata word *)&param_buf[0]);
                ccd_read_yoffset = *((__xdata word *)&param_buf[2]);
                ccd_read_width   = *((__xdata word *)&param_buf[4]);
                ccd_read_height  = *((__xdata word *)&param_buf[6]);
                ccd_read_xbin    = param_buf[8];
                ccd_read_ybin    = param_buf[9];
                if (ccd_read_xbin == 0)
                    ccd_read_xbin = 1;
                if (ccd_read_ybin == 0)
                    ccd_read_ybin = 1;
                ccd_read_binwidth = ccd_read_width / ccd_read_xbin;
                if (!ccd_read_binwidth)
                    ccd_read_binwidth = 1;
                if (!ccd_load_pending)
                {
                    ccd_begin_frame();
                    ccd_load_pixels();
                }
                break;
            case SXUSB_SET_TIMER:
                ccd_timer = *((__xdata dword *)param_buf);
                if (ccd_timer)
                {
                    TL0 = (byte)TIMER0_RELOAD;
                    TH0 = (byte)(TIMER0_RELOAD >> 8);
                    TR0 = 1;
                }
                else
                    TR0 = 0;
                break;
            case SXUSB_GET_TIMER:
                *(__xdata dword *)in_buf = ccd_timer;
                count                  = 4;
                break;
            case SXUSB_RESET:
                usb_reset_endpoint(2 | USB_DATAIN);
                ccd_reset();
                break;
            case SXUSB_SET_CCD:
                ccd_hfront_porch   = param_buf[0];
                ccd_hback_porch    = param_buf[1];
                ccd_width          = *((__xdata word *)&param_buf[2]);
                ccd_vfront_porch   = param_buf[4];
                ccd_vback_porch    = param_buf[5];
                ccd_height         = *((__xdata word *)&param_buf[6]);
                ccd_pixel_width    = *((__xdata word *)&param_buf[8]);
                ccd_pixel_height   = *((__xdata word *)&param_buf[10]);
                ccd_color_matrix   = *((__xdata word *)&param_buf[12]);
                if (!ccd_interleave && ccd_width > 1000)
                {
                    /*
                     * The HX9 uses a different pinout than the other cameras.
                     */
                    ccd_field_xor_mask = HX9_FIELD_MAGIC;
                    ccd_rdy            = 0xFC;
                    ccd_hclk           = 0xFE;
                    ccd_cnvrt          = 0x7E;
                    ccd_clear          = 0xFA;
                    ccd_clamp          = 0xDE;
                    ccd_ld_adc         = 0x04;
                    ccd_ld_lsb         = 0x0C;
                    ccd_ld_msb         = 0x1C;
                    ccd_xor_msb        = 0x80;
                }
                else
                {
                    /*
                     * Standard camera pinouts.
                     */
                    ccd_field_xor_mask = ccd_interleave ? MX_FIELD_MAGIC : HX_FIELD_MAGIC;
                    ccd_rdy            = 0xBA;
                    ccd_hclk           = 0xFA;
                    ccd_cnvrt          = 0xF8;
                    ccd_clear          = 0xFA;  // Older cameras don't have seperate reset/clamp but must do to trigger conversion
                    ccd_clamp          = 0xDA;  // Reset and clamp at the same time
                    ccd_ld_adc         = 0x2C;
                    ccd_ld_lsb         = 0x28;
                    ccd_ld_msb         = 0x08;
                    ccd_xor_msb        = 0x00;
                }
                ccd_reset();
                break;
            case SXUSB_GET_CCD:
                in_buf[0]  = ccd_hfront_porch;
                in_buf[1]  = ccd_hback_porch;
                in_buf[2]  = ccd_width;
                in_buf[3]  = ccd_width >> 8;
                in_buf[4]  = ccd_vfront_porch;
                in_buf[5]  = ccd_vback_porch;
                in_buf[6]  = ccd_height;
                in_buf[7]  = ccd_height >> 8;
                in_buf[8]  = ccd_pixel_width;
                in_buf[9]  = ccd_pixel_width >> 8;
                in_buf[10] = ccd_pixel_height;
                in_buf[11] = ccd_pixel_height >> 8;
                in_buf[12] = ccd_color_matrix;
                in_buf[13] = ccd_color_matrix >> 8;
                in_buf[14] = 16;                                    // Bits per pixel
                in_buf[15] = NUM_SERIAL_PORTS;                      // Serial port count
                in_buf[16] = 0;                                     // No extra caps
                count      = 17;
                break;
            case SXUSB_SET_STAR2K:
                break;
            case SXUSB_WRITE_SERIAL_PORT:
                if (usb_setup_req[USB_REQ_LENGTH_L] > SERIAL_BUF_SIZE)
                    usb_setup_req[USB_REQ_LENGTH_L] = SERIAL_BUF_SIZE;
                for (count = 0; count < usb_setup_req[USB_REQ_LENGTH_L]; count++)
                    serial_put(usb_setup_req[USB_REQ_INDEX_L], param_buf[count]);
                if (usb_setup_req[USB_REQ_VALUE_L] == 1)            // Flush data before accepting new commands
                    while (ser_out_count);
                count = 0;
                break;
            case SXUSB_READ_SERIAL_PORT:
                if (usb_setup_req[USB_REQ_LENGTH_H])
                    usb_setup_req[USB_REQ_LENGTH_L] = 63;
                else
                    usb_setup_req[USB_REQ_LENGTH_L] = min(63, usb_setup_req[USB_REQ_LENGTH_L]);
                for (count = 0; count < usb_setup_req[USB_REQ_LENGTH_L]; count++)
                    in_buf[count] = serial_get(usb_setup_req[USB_REQ_INDEX_L]);
                break;
            case SXUSB_SET_SERIAL:                                  // Nothing implemented yet
                break;
            case SXUSB_GET_SERIAL:
                if (usb_setup_req[USB_REQ_INDEX_L] == 0)
                {
                    if (usb_setup_req[USB_REQ_VALUE_L] == 0)        // Available output buffer space
                        count = SERIAL_BUF_SIZE - ser_out_count;
                    else if (usb_setup_req[USB_REQ_VALUE_L] == 1)   // Available input data
                        count = ser_in_count;
                    in_buf[0] = count;
                }
                else
                {
                    in_buf[0] = 0x00;
                }
                in_buf[1] = 0x00;
                count     = 2;
                break;
            case SXUSB_CAMERA_MODEL:
                if (USB_REQ_DIR(usb_setup_req[USB_REQ_TYPE]) == USB_REQ_DATAOUT && usb_setup_req[USB_REQ_LENGTH_L] == 0)
                {
                    ccd_model = usb_setup_req[USB_REQ_VALUE_L];
                    ccd_set_default_params();
                }
                else if (USB_REQ_DIR(usb_setup_req[USB_REQ_TYPE]) == USB_REQ_DATAIN && usb_setup_req[USB_REQ_LENGTH_L] == 2)
                {
                    in_buf[0] = ccd_model;
                    in_buf[1] = ccd_model == 0xFF ? 0xFF : 0x00;
                    count     = 2;
                }
                break;
            default:
                if (usb_setup_req[USB_REQ] == SXUSB_GET_FIRMWARE_VERSION)
                {
                    in_buf[0] = (byte)FIRMWARE_MINOR_VERSION;
                    in_buf[1] = (byte)(FIRMWARE_MINOR_VERSION >> 8);
                    in_buf[2] = (byte)FIRMWARE_MAJOR_VERSION;
                    in_buf[3] = (byte)(FIRMWARE_MAJOR_VERSION >> 8);
                    count     = 4;
                }
                else
                    count = -1;
        }
    else
        count = -1;
    return (count);
}

/***************************************************************************\
*                                                                           *
*                                Startup routine                            *
*                                                                           *
\***************************************************************************/

byte startup(void)
{
    /*
     * Init all EZUSB specific registers.
     */
    mdelay(1);                                                      // Wait for EZUSB to complete last transaction
    /*
     * Renumerate and set initial parameters.
     */
    USBCS      = USBCS_DISCON;                                      // Simulate disconnect
    mdelay(1000);                                                   // Wait for a second
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
    TH0        = (byte)(TIMER0_RELOAD >> 8);                        // USe timer0 for 1msec system timer
    TL0        = (byte)TIMER0_RELOAD;
    TH1        = (byte)TIMER1_RELOAD;                               // Generate timing for 9600 baud
    TL1        = 0x00;                                              // Clear timer1
    TR1        = 1;                                                 // Set timer1 to run
    USB_PAIR   = USB_PAIR_PR2IN | USB_PAIR_PR2OUT;
    USB_INIEN  = 0x05;                                              // Enable EP0 and EP2 input ints
    USB_OUTIEN = 0x05;                                              // Enable EP0 and EP2 output ints
    USB_IEN    = 0x19;                                              // Enable SUDAV, SUSP and URES ints
    USB_IBNIEN = 0x00;                                              // Disable IBN int
    USB_BAV    = 0x00;                                              // Disable auto-vectoring
    IP         = 0x01;                                              // Set USB priority high
    EIE        = 0x00;                                              // Disable external INT2 (USBINT) - we use polling
    IE         = 0x92;                                              // Global, serial0, and timer0 interrupt enable
    MPAGE      = 0x7F;                                              // Set MPAGE to 0x7F - EZUSB register page
    DPSEL      = 0x00;                                              // Set DPSEL to 0
    /*
     * Init external and internal variables.
     */
    usb_configured     = 0;
#ifdef CAMERA_MODEL
    ccd_model          = CAMERA_MODEL;
#else
    ccd_model          = 0xFF;
#endif
    ccd_set_default_params();
    ccd_timer          = 0;
    ccd_load_pending   = 0;
    ccd_load_done      = 1;
    ser_out_head       = 0;
    ser_out_count      = 0;
    ser_in_head        = 0;
    ser_in_count       = 0;
    /*
     * All done with initialization.
     */
    USBCS      = USBCS_DISCOE | USBCS_RENUM;                        // Renumerate
    return (0);
}

/***************************************************************************\
*                                                                           *
*                               Main loop                                   *
*                                                                           *
\***************************************************************************/

void main(void)
{
    signed char req_ret;

    startup();
    /*
     * Poll for something to do.
     */
    do
    {
        if (EXIF & 0x10)                                            // Check for pending USB service
        {
            EXIF = 0xEF;                                            // Clear external interrupt 2
            switch (USB_IVEC >> 2)                                  // Service source
            {
                case 0: // SUDAV - setup data available
                    USB_IRQ = 0x01;                                 // Clear interrupt source
                    __asm
                            mov     dptr, #_USB_SETUPDAT
                            mov     r0,   #_usb_setup_req
                            mov     r1,   #8
                        0010$:
                            movx    a, @dptr
                            mov     @r0, a
                            inc     dptr
                            inc     r0
                            djnz    r1, 0010$
                    __endasm;
                    if (USB_REQ_DIR(usb_setup_req[USB_REQ_TYPE]) == USB_REQ_DATAOUT && (usb_setup_req[USB_REQ_LENGTH_L] | usb_setup_req[USB_REQ_LENGTH_H]))
                    {
                        byte timeout = 200;                         // 20 msec
                        USB_OUT0BC   = 0;                           // Accept data
                        while (timeout-- && (USB_EP0CS & USB_CS0OUTBSY))   // Wait for data packet to arrive
                            udelay(100);
                        if (timeout == 0xFF)
                        {
                            usb_stall(0);
                            break;
                        }
                    }
                    if (USB_REQ_KIND(usb_setup_req[USB_REQ_TYPE])  == USB_REQ_VENDOR
                     && USB_REQ_RECIP(usb_setup_req[USB_REQ_TYPE]) == 0)// Vendor (CCD) request
                    {
                        if ((req_ret = sx_request(USB_OUT0BUF, USB_IN0BUF)) < 0)
                            usb_stall(0);
                        else
                            usb_ctl_reply(req_ret);
                    }
                    else if (USB_REQ_KIND(usb_setup_req[USB_REQ_TYPE]) == USB_REQ_STD)// Standard request
                        usb_std_request();
                    else                                            // Error
                        usb_stall(0);
                    break;
                case 1: // SOF - start of frame
                    USB_IRQ = 0x02;                                 // Clear interrupt source
                    break;
                case 2: // SUTOK - setup token
                    USB_IRQ = 0x04;                                 // Clear interrupt source
                    break;
                case 3: // SUSP - suspend
                    USB_IRQ = 0x08;                                 // Clear interrupt source
                    EICON_4 = 0;                                    // Clear  wake-up interrupt
                    EICON_5 = 1;                                    // Enable wake-up interrupt
                    PCON   &= 0xFE;                                 // Turn off oscillator and enter idle mode
                    __asm
                        nop
                        nop
                        nop
                    __endasm;
                    EICON_5 = 0;                                    // Disable wake-up IRQ
                    break;
                case 4: // URES - USB reset
                    USB_IRQ          = 0x10;                        // Clear interrupt source
                    usb_configured   = 0;
                    ccd_load_pending = 0;
                    ccd_load_done    = 1;
                    CCD_INIT();                                     // Reset camera
                    usb_reset_endpoint(2 | USB_DATAIN);
                    break;
                case 5: // IBN INT
                    USB_IRQ = 0x20;                                 // Clear interrupt source
                    break;
                case 6: // EP0 IN
                    USB_INIRQ = 0x01;                               // Clear interrupt source
                    break;
                case 7: // EP0 OUT
                    USB_OUTIRQ = 0x01;                              // Clear interrupt source
                    break;
                case 8: // EP1 IN - not used
                    USB_INIRQ = 0x02;                               // Clear interrupt source
                    break;
                case 9: //  EP1 OUT - not used
                    USB_OUTIRQ = 0x02;                              // Clear interrupt source
                    break;
                case 10: // EP2 IN
                    USB_INIRQ = 0x04;                               // Clear interrupt source
                    if (!usb_configured)
                        usb_stall(USB_DATAIN | 2);
                    else if (!ccd_load_done)
                        ccd_load_pixels();
                    break;
                case 11: // EP2 OUT
                    USB_OUTIRQ = 0x04;                              // Clear interrupt source
                    if (!usb_configured || (USB_OUT2BC < 8))
                        usb_stall(USB_DATAOUT | 2);
                    else
                    {
                        /*
                         * Only commands for the CCD are expected here.
                         * Fake out a vendor control request.
                         */
                        __asm
                                mov     dptr, #_USB_OUT2BUF
                                mov     r0,   #_usb_setup_req
                                mov     r1,   #8
                            0020$:
                                movx    a, @dptr
                                mov     @r0, a
                                inc     dptr
                                inc     r0
                                djnz    r1, 0020$
                        __endasm;
                        while (USB_IN2CS & USB_CSINBSY);
                        if ((req_ret = sx_request(&USB_OUT2BUF[8], USB_IN2BUF)) < 0) // Parameters should follow setup data
                        {
                            usb_stall(USB_DATAOUT | 2);
                        }
                        else
                        {
                            USB_OUT2BC = 0;                         // Accept more data
                            if (req_ret > 0)
                                USB_IN2BC = req_ret;                // Send data to host
                        }
                    }
                    break;
            }
        }
        else if (ccd_load_pending && ccd_timer == 0)
        {
            ccd_begin_frame();
            ccd_load_pixels();
        }
    } while (1);
}
