#include <stdio.h>
#include <string.h>

/*
 * Addressing modes.
 */
#define ADDR_IMP    0 // Implicit in opcode
#define ADDR_REG    1 // Encoded in opcode
#define ADDR_MEM    2 // Encoded in opcode
#define ADDR_DB     3 // Encoded in opcode
#define ADDR_DIRECT 4 // Encoded in next byte
#define ADDR_REL    5 // Encoded in next byte
#define ADDR_BIT    6 // Encoded in next byte
#define ADDR_DATA8  7 // Encoded in next byte
#define ADDR_ADDR8  8 // Encoded in next byte
#define ADDR_ADDR11 9 // Encoded in opcode and next byte
#define ADDR_DATA16 10 // Encoded in next 2 bytes
#define ADDR_ADDR16 11 // Encoded in next 2 bytes
#define ADDR_MODES  12
unsigned int addr_mode_mask[] = {0xFF, 0xF8, 0xFE, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF};
/*
 * Addressing mode masks.
 */
struct opcode_rec
{
    int  opcode;
    int  addr_mode;
    char format[64];
};
struct opcode_rec opcode_recs[] =
{
/* ADD */
{0x28, ADDR_REG,    "ADD   A, R%d"},
{0x25, ADDR_DIRECT, "ADD   A, %03XH"},
{0x26, ADDR_MEM,    "ADD   A, @R%d"},
{0x24, ADDR_DATA8,  "ADD   A, #%03XH"},
/* ADDC */
{0x38, ADDR_REG,    "ADDC  A, R%d"},
{0x35, ADDR_DIRECT, "ADDC  A, %03XH"},
{0x36, ADDR_MEM,    "ADDC  A, @R%d"},
{0x34, ADDR_DATA8,  "ADDC  A, #0%03XH"},
/* SUBB */
{0x98, ADDR_REG,    "SUBB  A, R%d"},
{0x95, ADDR_DIRECT, "SUBB  A, %03XH"},
{0x96, ADDR_MEM,    "SUBB  A, @R%d"},
{0x94, ADDR_DATA8,  "SUBB  A, #0%03XH"},
/* INC */
{0x04, ADDR_IMP,    "INC   A"},
{0x08, ADDR_REG,    "INC   R%d"},
{0x05, ADDR_DIRECT, "INC   %03XH"},
{0x06, ADDR_MEM,    "INC   @R%d"},
{0xA3, ADDR_IMP,    "INC   DPTR"},
/* DEC */
{0x14, ADDR_IMP,    "DEC   A"},
{0x18, ADDR_REG,    "DEC   R%d"},
{0x15, ADDR_DIRECT, "DEC   %03XH"},
{0x16, ADDR_MEM,    "DEC   @R%d"},
/* MUL */
{0xA4, ADDR_IMP,    "MUL   AB"},
/* DIV */
{0x84, ADDR_IMP,    "DIV   AB"},
/* DA */
{0xD4, ADDR_IMP,    "DA    A"},
/* ANL */
{0x58, ADDR_REG,    "ANL   A, R%d"},
{0x55, ADDR_DIRECT, "ANL   A, %03XH"},
{0x56, ADDR_MEM,    "ANL   A, @R%d"},
{0x54, ADDR_DATA8,  "ANL   A, #%03XH"},
{0x52, ADDR_DIRECT, "ANL   %03XH, A"},
{0x53, ADDR_DIRECT | (ADDR_DATA8 << 8), "ANL   %03XH, #%03XH"},
/* ORL */
{0x48, ADDR_REG,    "ORL   A, R%d"},
{0x45, ADDR_DIRECT, "ORL   A, %03XH"},
{0x46, ADDR_MEM,    "ORL   A, @R%d"},
{0x44, ADDR_DATA8,  "ORL   A, #%03XH"},
{0x42, ADDR_DIRECT, "ORL   %03XH, A"},
{0x43, ADDR_DIRECT | (ADDR_DATA8 << 8), "ORL   %03XH, #%03XH"},
/* XORL */
{0x68, ADDR_REG,    "XORL  A, R%d"},
{0x65, ADDR_DIRECT, "XORL  A, %03XH"},
{0x66, ADDR_MEM,    "XORL  A, @R%d"},
{0x64, ADDR_DATA8,  "XORL  A, #%03XH"},
{0x62, ADDR_DIRECT, "XORL  %03XH, A"},
{0x63, ADDR_DIRECT | (ADDR_DATA8 << 8), "XORL  %03XH, #%03XH"},
/* CLR */
{0xE4, ADDR_IMP,    "CLR   A"},
/* CPL */
{0xF4, ADDR_IMP,    "CPL   A"},
/* SWAP */
{0xC4, ADDR_IMP,    "SWAP  A"},
/* RL */
{0x23, ADDR_IMP,    "RL    A"},
/* RLC */
{0x33, ADDR_IMP,    "RLC   A"},
/* RR */
{0x03, ADDR_IMP,    "RR    A"},
/* RRC */
{0x13, ADDR_IMP,    "RRC   A"},
/* MOV */
{0xE8, ADDR_REG,    "MOV   A, R%d"},
{0xE5, ADDR_DIRECT, "MOV   A, %03XH"},
{0xE6, ADDR_MEM,    "MOV   A, @R%d"},
{0x74, ADDR_DATA8,  "MOV   A, #%03XH"},
{0xF8, ADDR_REG,    "MOV   R%d, A"},
{0xA8, ADDR_REG    | (ADDR_DIRECT << 8), "MOV   R%d, %03XH"},
{0x78, ADDR_REG    | (ADDR_DATA8  << 8), "MOV   R%d, #%03XH"},
{0xF5, ADDR_DIRECT, "MOV   %03XH, A"},
{0x88, ADDR_REG    | (ADDR_DIRECT << 8), "MOV   %03XH, R%d"},
{0x85, ADDR_DIRECT | (ADDR_DIRECT << 8), "MOV   %03XH, %03XH"},
{0x86, ADDR_MEM    | (ADDR_DIRECT << 8), "MOV   %03XH, @R%d"},
{0x75, ADDR_DIRECT | (ADDR_DATA8  << 8), "MOV   %03XH, #%03XH"},
{0xF6, ADDR_MEM,    "MOV   @R%d, A"},
{0xA6, ADDR_MEM | (ADDR_DIRECT << 8), "MOV   @R%d, %03XH"},
{0x76, ADDR_MEM | (ADDR_DATA8  << 8), "MOV   @R%d, #%03XH"},
{0x90, ADDR_DATA16, "MOV   DPTR, #%05XH"},
/* MOVC */
{0x93, ADDR_IMP,    "MOVC  A, @A+DPTR"},
{0x83, ADDR_IMP,    "MOVC  A, @A+PC"},
/* MOVX */
{0xE2, ADDR_MEM,    "MOVX  A, @R%d"},
{0xE0, ADDR_IMP,    "MOVX  A, @DPTR"},
{0xF2, ADDR_MEM,    "MOVX  @R%d, A"},
{0xF0, ADDR_IMP,    "MOVX  @DPTR, A"},
/* PUSH */
{0xC0, ADDR_DIRECT, "PUSH  %03XH"},
/* POP */
{0xD0, ADDR_DIRECT, "POP   %03XH"},
/* XCH */
{0xC8, ADDR_REG,    "XCH   A, R%d"},
{0xC5, ADDR_DIRECT, "XCH   A, %03XH"},
{0xC6, ADDR_MEM,    "XCH   A, @R%d"},
{0xD6, ADDR_MEM,    "XCHD  A, @R%d"},
/* CLR */
{0xC3, ADDR_IMP,    "CLR   C"},
{0xC2, ADDR_BIT,    "CLR   %03XH.%d"},
/* SETB */
{0xD3, ADDR_IMP,    "SETB  C"},
{0xD2, ADDR_BIT,    "SETB  %03XH.%d"},
/* CPL */
{0xB3, ADDR_IMP,    "CPL   C"},
{0xB2, ADDR_BIT,    "CPL   %03XH.%d"},
/* ANL */
{0x82, ADDR_BIT,    "ANL   C, %03XH.%d"},
{0xB0, ADDR_BIT,    "ANL   C, /%03XH.%d"},
/* ORL */
{0x72, ADDR_BIT,    "ORL   C, %03XH.%d"},
{0xA0, ADDR_BIT,    "ORL   C, /%03XH.%d"},
/* MOV */
{0xA2, ADDR_BIT,    "MOV   C, %03XH.%d"},
{0x92, ADDR_BIT,    "MOV   %03XH.%d, C"},
/* CALL */
{0x11, ADDR_ADDR11, "ACALL %05XH"},
{0x12, ADDR_ADDR16, "LCALL %05XH"},
/* RET */
{0x22, ADDR_IMP,    "RET"},
{0x32, ADDR_IMP,    "RETI"},
/* JMP */
{0x01, ADDR_ADDR11, "AJMP  %05XH"},
{0x02, ADDR_ADDR16, "LJMP  %05XH"},
{0x80, ADDR_REL,    "SJMP  %05XH"},
{0x40, ADDR_REL,    "JC    %05XH"},
{0x50, ADDR_REL,    "JNC   %05XH"},
{0x20, ADDR_BIT   | (ADDR_REL << 8), "JB    %03XH.%d, %05XH"},
{0x30, ADDR_BIT   | (ADDR_REL << 8), "JNB   %03XH.%d, %05XH"},
{0x10, ADDR_BIT   | (ADDR_REL << 8), "JBC   %03XH.%d, %05XH"},
{0x73, ADDR_IMP,    "JMP   @A+DPTR"},
{0x60, ADDR_REL,    "JZ    %05XH"},
{0x70, ADDR_REL,    "JNZ   %05XH"},
{0xB5, ADDR_DIRECT| (ADDR_REL << 8), "CJNE  A, %03XH, %05XH"},
{0xB4, ADDR_DATA8 | (ADDR_REL << 8), "CJNE  A, #%03XH, %05XH"},
{0xB8, ADDR_REG   | (ADDR_DATA8 << 8) | (ADDR_REL << 16), "CJNE  R%d, #%03XH, %05XH"},
{0xB6, ADDR_MEM   | (ADDR_DATA8 << 8) | (ADDR_REL << 16), "CJNE  @R%d, #%03XH, %05XH"},
{0xD8, ADDR_REG   | (ADDR_REL << 8), "DJNZ  R%d, %05XH"},
{0xD5, ADDR_DIRECT| (ADDR_REL << 8), "DJNZ  %03XH, %05XH"},
/* NOP */
{0x00, ADDR_IMP,    "NOP"},
/* .DB */
{0x00, ADDR_DB,     ".DB  %03XH ('%c')"} // Catch all - define byte data
};

int hexbyte(char *hexchars)
{
    return (((hexchars[0] - '0' - ((hexchars[0] > '9') ? ('A' - '9' - 1) : 0)) << 4) | (hexchars[1] - '0' - ((hexchars[1] > '9') ? ('A' - '9' - 1) : 0)));
}
         char hexline[80];
unsigned char hexdata[18];
         int  hexaddr = 0;
         int  hexlen  = 0;
int get_byte(unsigned int *addr, unsigned char *byte)
{
    int  hextype, i, j;

    if (hexlen == 0)
    {
        if (gets(hexline))
        {
            if ((hexline[0] == ':') && strlen(hexline) >= 11)
            {
                hexlen  = hexbyte(&hexline[1]);
                hexaddr = (hexbyte(&hexline[3]) << 8) | hexbyte(&hexline[5]);
                hextype = hexbyte(&hexline[7]);
                if (hextype == 0)
                    for (i = 9, j = 0; hexline[i] >= '0' && hexline[i] <= 'F'; i += 2, j++)
                        hexdata[j] = hexbyte(&hexline[i]);
                else
                    return (0);
            }
            else
                return (0);
        }
        else
            return (0);
    }
    *addr = hexaddr++;
    *byte = hexdata[0];
    memcpy(&(hexdata[0]), &(hexdata[1]), --hexlen);
    return (1);
}
int unget_byte(unsigned char byte)
{
    memcpy(&(hexdata[1]), &(hexdata[0]), hexlen++);
    hexaddr--;
}
int get_opcode(unsigned int *addr, unsigned char *opcode, unsigned int *operand, struct opcode_rec **opcode_rec, int *op_count)
{
    unsigned int next_addr, ad_count, i, j;

    if (get_byte(addr, &(opcode[0])))
    {
        for (i = 0; i < sizeof(opcode_recs)/sizeof(struct opcode_rec); i++)
        {
            if (opcode_recs[i].opcode == (opcode[0] & addr_mode_mask[(opcode_recs[i].addr_mode & 0xFF)]))
            {
                if ((opcode_recs[i].addr_mode & 0xFF) >= ADDR_DIRECT) 
                {
                    if (get_byte(&next_addr, &(opcode[1])))
                    {
                        if (next_addr != *addr + 1)
                        {
                            unget_byte(opcode[1]);
                            opcode[1] = 0;
                        }
                    }
                    else
                        return (0);
                }
                switch (opcode_recs[i].addr_mode & 0xFF)
                {
                    case ADDR_REG:
                        operand[0] = opcode[0] & ~addr_mode_mask[ADDR_REG];
                        *op_count  = 1;
                        ad_count   = 1;
                        break;
                    case ADDR_MEM:
                        operand[0] = opcode[0] & ~addr_mode_mask[ADDR_MEM];
                        *op_count  = 1;
                        ad_count   = 1;
                        break;
                    case ADDR_DB:
                        operand[1] = (opcode[0] >= ' ' && opcode[0] <= 'Z') ? opcode[0] : '.';
                        operand[0] = opcode[0];
                        *op_count  = 1;
                        ad_count   = 2;
                        break;
                    case ADDR_DIRECT:
                        operand[0] = opcode[1];
                        *op_count  = 2;
                        ad_count   = 1;
                        break;
                    case ADDR_REL:
                        operand[0] = *addr + 2 + (signed char)opcode[1];
                        *op_count  = 2;
                        ad_count   = 1;
                        break;
                    case ADDR_BIT:
                        if (opcode[1] >= 0x80)
                        {
                            operand[0] = opcode[1] & 0xF8;
                            operand[1] = opcode[1] & 0x07;
                        }
                        else
                        {
                            operand[0] = 0x20 + (opcode[1] >> 3);
                            operand[1] = opcode[1] & 7;
                        }
                        *op_count  = 2;
                        ad_count   = 2;
                        break;
                    case ADDR_DATA8:
                    case ADDR_ADDR8:
                        operand[0] = opcode[1];
                        *op_count  = 2;
                        ad_count   = 1;
                        break;
                    case ADDR_ADDR11:
                        operand[0] = (((int)opcode[0] & 0xE0) << 3) | opcode[1] | (*addr & 0xF800);
                        *op_count  = 2;
                        ad_count   = 1;
                        break;
                    case ADDR_DATA16:
                    case ADDR_ADDR16:
                        if (get_byte(&next_addr, &(opcode[2])))
                        {
                            if (next_addr != *addr + 2)
                            {
                                unget_byte(opcode[2]);
                                opcode[2] = 0;
                            }
                        }
                        else
                            return (0);
                        operand[0] = opcode[2] | (opcode[1] << 8);
                        *op_count  = 3;
                        ad_count   = 1;
                        break;
                    default:
                        operand[0] = opcode[0];
                        *op_count  = 1;
                        ad_count   = 1;
                        break;
                }
                /*
                 * Only a few opcodes have a second addressing mode.
                 */
                if (((opcode_recs[i].addr_mode >> 8) & 0xFF) >= ADDR_DIRECT) 
                {
                    if (get_byte(&next_addr, &(opcode[*op_count])))
                    {
                        if (next_addr != *addr + *op_count)
                        {
                            unget_byte(opcode[*op_count]);
                            opcode[*op_count] = 0;
                        }
                    }
                    else
                        return (0);
                }
                switch ((opcode_recs[i].addr_mode >> 8) & 0xFF)
                {
                    case ADDR_DIRECT:
                    case ADDR_DATA8:
                    case ADDR_ADDR8:
                        operand[ad_count++] = opcode[(*op_count)++];
                        break;
                    case ADDR_REL:
                        operand[ad_count++] = *addr + *op_count + 1 + (signed char)opcode[(*op_count)++];
                        break;
                    default:
                        break;
                }
                /*
                 * Only a very few opcodes have a third addressing mode.
                 */
                switch ((opcode_recs[i].addr_mode >> 16) & 0xFF)
                {
                    case ADDR_REL:
                        if (get_byte(&next_addr, &(opcode[*op_count])))
                        {
                            if (next_addr != *addr + *op_count)
                            {
                                unget_byte(opcode[*op_count]);
                                opcode[*op_count] = 0;
                            }
                        }
                        else
                            return (0);
                        operand[ad_count] = *addr + *op_count + 1 + (signed char)opcode[(*op_count)++];
                        break;
                    default:
                        break;
                }
                *opcode_rec = &(opcode_recs[i]);
                return (1);
            }
        }
    }
    return (0);
}
int main(int argc, char **argv)
{
    int                op_count, i;
    unsigned int       addr, operand[3];
    unsigned char      opcode[3];
    struct opcode_rec *opcode_rec;

    while (get_opcode(&addr, opcode, operand, &opcode_rec, &op_count))
    {
        printf("%05XH: ", addr);
        for (i = 0; i < 3; i++)
            if (i < op_count)
                printf("%03XH ", opcode[i]);
            else
                printf("     ");
            printf("; ");
        printf(opcode_rec->format, operand[0], operand[1], operand[2]);
        printf("\n");
    }
    printf("\n");
    return (0);
}
