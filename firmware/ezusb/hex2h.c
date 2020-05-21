#include <stdio.h>
#include <string.h>

int hexbyte(char *hexchars)
{
    return (((hexchars[0] - '0' - ((hexchars[0] > '9') ? ('A' - '9' - 1) : 0)) << 4) | (hexchars[1] - '0' - ((hexchars[1] > '9') ? ('A' - '9' - 1) : 0)));
}
int main(int argc, char **argv)
{
    char hexline[80];
    int  hexaddr, hexlen, hextype, hexdata[17], i, j;

    while (gets(hexline))
    {
        if ((hexline[0] == ':') && strlen(hexline) >= 11)
        {
            hexlen  = hexbyte(&hexline[1]);
            hexaddr = (hexbyte(&hexline[3]) << 8) | hexbyte(&hexline[5]);
            hextype = hexbyte(&hexline[7]);
            if (hextype == 0)
            {
                for (i = 9, j = 0; hexline[i] >= '0' && hexline[i] <= 'F'; i += 2, j++)
                {
                    hexdata[j] = hexbyte(&hexline[i]);
                }
                if (j == hexlen + 1 && hexlen > 0)
                {
                    printf("{0x%04X,0x%02X,{0x%02X", hexaddr, hexlen, hexdata[0]);
                    for (i = 1; i < hexlen; i++)
                    {
                        printf(",0x%02X", hexdata[i]);
                    }
                    printf("}},\n");
                }
                else
                    return (1);
            }
        }
    }
    return (0);
}
