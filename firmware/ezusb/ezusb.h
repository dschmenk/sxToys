/***************************************************************************\
    
    Copyright (c) 2002 David Schmenk
    
    All rights reserved.
    
    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, and/or sell copies of the Software, and to permit persons
    to whom the Software is furnished to do so, provided that the above
    copyright notice(s) and this permission notice appear in all copies of
    the Software and that both the above copyright notice(s) and this
    permission notice appear in supporting documentation.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
    OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
    HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
    INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
    FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
    NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
    WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
    
    Except as contained in this notice, the name of a copyright holder
    shall not be used in advertising or otherwise to promote the sale, use
    or other dealings in this Software without prior written authorization
    of the copyright holder.
    
\***************************************************************************/
/*
 * Definitions for EZ-USB
 */
#ifndef __EZUSB_H__
#define SBIT(addr, name)    __sbit __at addr volatile name
#define SFR(addr, name)     __sfr __at addr volatile name
#define XREG(addr, name)    __xdata __at addr volatile unsigned char name
/*
 * BIT registers.
 */
/* EICON */
SBIT(0xD8,  EICON_0);
SBIT(0xD9,  EICON_1);
SBIT(0xDA,  EICON_2);
SBIT(0xDB,  EICON_3);
SBIT(0xDC,  EICON_4);
SBIT(0xDD,  EICON_5);
SBIT(0xDE,  EICON_6);
SBIT(0xDF,  EICON_7);
/* EIE */
SBIT(0xE8,  EIE_0);
SBIT(0xE9,  EIE_1);
SBIT(0xEA,  EIE_2);
SBIT(0xEB,  EIE_3);
SBIT(0xEC,  EIE_4);
SBIT(0xED,  EIE_5);
SBIT(0xEE,  EIE_6);
SBIT(0xEF,  EIE_7);
/*
 * BYTE registers.
 */
SFR(0x82, DPL0);        // DPTR0
SFR(0x83, DPH0); 
SFR(0x84, DPL1);        // DPTR1
SFR(0x85, DPH1); 
SFR(0x86, DPSEL);       // DPTR select
SFR(0x8E, CKCON); 
SFR(0x8F, SPC_FNC); 
SFR(0x91, EXIF);        // External interrupt flags
SFR(0x92, MPAGE);       // Page register, used with MOVX @Ri
SFR(0xD8, EICON); 
SFR(0xE8, EIE); 
/*
 * USB data buffers.
 */
#define USB_BUF_SIZE        64
XREG(0x7F00, USB_IN0BUF[USB_BUF_SIZE]);	
XREG(0x7EC0, USB_OUT0BUF[USB_BUF_SIZE]);	
XREG(0x7E80, USB_IN1BUF[USB_BUF_SIZE]);	
XREG(0x7E40, USB_OUT1BUF[USB_BUF_SIZE]);	
XREG(0x7E00, USB_IN2BUF[USB_BUF_SIZE]);	
XREG(0x7DC0, USB_OUT2BUF[USB_BUF_SIZE]);	
XREG(0x7D80, USB_IN3BUF[USB_BUF_SIZE]);	
XREG(0x7D40, USB_OUT3BUF[USB_BUF_SIZE]);	
XREG(0x7D00, USB_IN4BUF[USB_BUF_SIZE]);	
XREG(0x7CC0, USB_OUT4BUF[USB_BUF_SIZE]);	
XREG(0x7C80, USB_IN5BUF[USB_BUF_SIZE]);	
XREG(0x7C40, USB_OUT5BUF[USB_BUF_SIZE]);	
XREG(0x7C00, USB_IN6BUF[USB_BUF_SIZE]);	
XREG(0x7BC0, USB_OUT6BUF[USB_BUF_SIZE]);	
XREG(0x7B80, USB_IN7BUF[USB_BUF_SIZE]);	
XREG(0x7B40, USB_OUT7BUF[USB_BUF_SIZE]);	
/*
 * Isochronous data FIFOs.
 */
XREG(0x7F60, ISO_OUT8DATA);
XREG(0x7F61, ISO_OUT9DATA);
XREG(0x7F62, ISO_OUT10DATA);
XREG(0x7F63, ISO_OUT11DATA);
XREG(0x7F64, ISO_OUT12DATA);
XREG(0x7F65, ISO_OUT13DATA);
XREG(0x7F66, ISO_OUT14DATA);
XREG(0x7F67, ISO_OUT15DATA);
XREG(0x7F68, ISO_IN8DATA);
XREG(0x7F69, ISO_IN9DATA);
XREG(0x7F6A, ISO_IN10DATA);
XREG(0x7F6B, ISO_IN11DATA);
XREG(0x7F6C, ISO_IN12DATA);
XREG(0x7F6D, ISO_IN13DATA);
XREG(0x7F6E, ISO_IN14DATA);
XREG(0x7F6F, ISO_IN15DATA);
XREG(0x7F70, ISO_OUT8BC);
XREG(0x7F71, ISO_OUT9BC);
XREG(0x7F72, ISO_OUT10BC);
XREG(0x7F73, ISO_OUT11BC);
XREG(0x7F74, ISO_OUT12BC);
XREG(0x7F75, ISO_OUT13BC);
XREG(0x7F76, ISO_OUT14BC);
XREG(0x7F77, ISO_OUT15BC);
/*
 * CPU control and status.
 */
#define CPUCS_CLK24OE       0x02
#define CPUCS_RESET         0x01
XREG(0x7F92, CPUCS);
/*
 * I/O port control.
 */
#define PORT_CFG_IO         0x00
XREG(0x7F93, PORTA_CFG);
XREG(0x7F94, PORTB_CFG);
XREG(0x7F95, PORTC_CFG);
XREG(0x7F96, PORTA_OUT);
XREG(0x7F97, PORTB_OUT);
XREG(0x7F98, PORTC_OUT);
XREG(0x7F99, PORTA_PINS);
XREG(0x7F9A, PORTB_PINS);
XREG(0x7F9B, PORTC_PINS);
XREG(0x7F9C, PORTA_OE);
XREG(0x7F9D, PORTB_OE);
XREG(0x7F9E, PORTC_OE);
/*
 * UART control.
 */
XREG(0x7F9F, UART230);
/*
 * Isochronous control and status.
 */
XREG(0x7FA0, ISO_ERR);
XREG(0x7FA1, ISO_CTL);
XREG(0x7FA2, ISO_ZBCOUT);
/*
 * I2C control, status and data.
 */
XREG(0x7FA5, I2C_CS);
XREG(0x7FA6, I2C_DAT);
XREG(0x7FA7, I2C_MODE);
/*
 * USB IRQ control.
 */
XREG(0x7FA8, USB_IVEC);
XREG(0x7FA9, USB_INIRQ);
XREG(0x7FAA, USB_OUTIRQ);
XREG(0x7FAB, USB_IRQ);
XREG(0x7FAC, USB_INIEN);
XREG(0x7FAD, USB_OUTIEN);
XREG(0x7FAE, USB_IEN);
XREG(0x7FAF, USB_BAV);
XREG(0x7FB0, USB_IBNIRQ);
XREG(0x7FB1, USB_IBNIEN);
/*
 * Breakpoint address.
 */
XREG(0x7FB2, BP_ADDRH);
XREG(0x7FB3, BP_ADDRL);
/*
 * Endpoint 0-7 control, status and byte counts.
 */
#define USB_CS0OUTBSY       0x08
#define USB_CS0INBSY        0x04
#define USB_CS0HSNAK        0x02
#define USB_CS0STALL        0x01
#define USB_CSINBSY         0x02
#define USB_CSINSTL         0x01
#define USB_CSOUTBSY        0x02
#define USB_CSOUTSTL        0x01
XREG(0x7FB4, USB_EP0CS);
XREG(0x7FB5, USB_IN0BC);
XREG(0x7FB6, USB_IN1CS);
XREG(0x7FB7, USB_IN1BC);
XREG(0x7FB8, USB_IN2CS);
XREG(0x7FB9, USB_IN2BC);
XREG(0x7FBA, USB_IN3CS);
XREG(0x7FBB, USB_IN3BC);
XREG(0x7FBC, USB_IN4CS);
XREG(0x7FBD, USB_IN4BC);
XREG(0x7FBE, USB_IN5CS);
XREG(0x7FBF, USB_IN5BC);
XREG(0x7FC0, USB_IN6CS);
XREG(0x7FC1, USB_IN6BC);
XREG(0x7FC2, USB_IN7CS);
XREG(0x7FC3, USB_IN7BC);
XREG(0x7FC5, USB_OUT0BC);
XREG(0x7FC6, USB_OUT1CS);
XREG(0x7FC7, USB_OUT1BC);
XREG(0x7FC8, USB_OUT2CS);
XREG(0x7FC9, USB_OUT2BC);
XREG(0x7FCA, USB_OUT3CS);
XREG(0x7FCB, USB_OUT3BC);
XREG(0x7FCC, USB_OUT4CS);
XREG(0x7FCD, USB_OUT4BC);
XREG(0x7FCE, USB_OUT5CS);
XREG(0x7FCF, USB_OUT5BC);
XREG(0x7FD0, USB_OUT6CS);
XREG(0x7FD1, USB_OUT6BC);
XREG(0x7FD2, USB_OUT7CS);
XREG(0x7FD3, USB_OUT7BC);
/*
 * Setup data pointer.
 */
XREG(0x7FD4, USB_SUDPTRH);
XREG(0x7FD5, USB_SUDPTRL);
/*
 * USB control and status.
 */
#define USBCS_WAKESRC       0x80
#define USBCS_DISCON        0x08
#define USBCS_DISCOE        0x04
#define USBCS_RENUM         0x02
#define USBCS_SIGRSUME      0x01
XREG(0x7FD6, USBCS);
/*
 * Data toggle control.
 */
#define USB_TOGCTL_Q        0x80
#define USB_TOGCTL_S        0x40
#define USB_TOGCTL_R        0x20
#define USB_TOGCTL_IO       0x10
#define USB_TOGCTL_EP7      0x07
#define USB_TOGCTL_EP6      0x06
#define USB_TOGCTL_EP5      0x05
#define USB_TOGCTL_EP4      0x04
#define USB_TOGCTL_EP3      0x03
#define USB_TOGCTL_EP2      0x02
#define USB_TOGCTL_EP1      0x01
#define USB_TOGCTL_EP0      0x00
XREG(0x7FD7, USB_TOGCTL);
/*
 * Frame count.
 */
XREG(0x7FD8, USB_FRAMEL);
XREG(0x7FD9, USB_FRAMEH);
/*
 * Device address.
 */
XREG(0x7FDB, USB_FNADDR);
/*
 * Endpoint pairing.
 */
#define USB_PAIR_IOSEND0    0x80
#define USB_PAIR_PR6OUT     0x20
#define USB_PAIR_PR4OUT     0x10
#define USB_PAIR_PR2OUT     0x08
#define USB_PAIR_PR6IN      0x04
#define USB_PAIR_PR4IN      0x02
#define USB_PAIR_PR2IN      0x01
XREG(0x7FDD, USB_PAIR);
/*
 * Endpoint valid bits.
 */
#define USB_VAL7            0x80
#define USB_VAL6            0x40
#define USB_VAL5            0x20
#define USB_VAL4            0x10
#define USB_VAL3            0x08
#define USB_VAL2            0x04
#define USB_VAL1            0x02
#define USB_VAL0            0x01
XREG(0x7FDE, USB_INVAL);
XREG(0x7FDF, USB_OUTVAL);
/*
 * Isochronous endpoint valid bits.
 */
#define USB_ISOVAL15        0x80
#define USB_ISOVAL14        0x40
#define USB_ISOVAL13        0x20
#define USB_ISOVAL12        0x10
#define USB_ISOVAL11        0x08
#define USB_ISOVAL10        0x04
#define USB_ISOVAL9         0x02
#define USB_ISOVAL8         0x01
XREG(0x7FE0, USB_ISOINVAL);
XREG(0x7FE1, USB_ISOOUTVAL);
/*
 * Fast transfer control.
 */
#define USB_FASTXFER_ISO    0x80
#define USB_FASTXFER_BLK    0x40
#define USB_FASTXFER_ROPL   0x20
#define USB_FASTXFER_RMOD3  0x18
#define USB_FASTXFER_RMOD2  0x10
#define USB_FASTXFER_RMOD1  0x08
#define USB_FASTXFER_RMOD0  0x00
#define USB_FASTXFER_WPOL   0x04
#define USB_FASTXFER_WMOD3  0x03
#define USB_FASTXFER_WMOD2  0x02
#define USB_FASTXFER_WMOD1  0x01
#define USB_FASTXFER_WMOD0  0x00
XREG(0x7FE2, USB_FASTXFER);
/*
 * Auto pointer.
 */
XREG(0x7FE3, AUTOPTRH);
XREG(0x7FE4, AUTOPTRL);
XREG(0x7FE5, AUTODATA);
/*
 * Setup data buffer. 
 */
XREG(0x7FE8, USB_SETUPDAT[8]);
/*
 * Isochronous endpoint start addresses.
 */
XREG(0x7FF0, ISO_OUT8ADDR);
XREG(0x7FF1, ISO_OUT9ADDR);
XREG(0x7FF2, ISO_OUT10ADDR);
XREG(0x7FF3, ISO_OUT11ADDR);
XREG(0x7FF4, ISO_OUT12ADDR);
XREG(0x7FF5, ISO_OUT13ADDR);
XREG(0x7FF6, ISO_OUT14ADDR);
XREG(0x7FF7, ISO_OUT15ADDR);
XREG(0x7FF8, ISO_IN8ADDR);
XREG(0x7FF9, ISO_IN9ADDR);
XREG(0x7FFA, ISO_IN10ADDR);
XREG(0x7FFB, ISO_IN11ADDR);
XREG(0x7FFC, ISO_IN12ADDR);
XREG(0x7FFD, ISO_IN13ADDR);
XREG(0x7FFE, ISO_IN14ADDR);
XREG(0x7FFF, ISO_IN15ADDR);
                                     
#endif // __EZUSB_H__

