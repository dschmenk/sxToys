#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#define open _open
#define creat _creat
#define write _write
#define close _close
#else
#include <unistd.h>
#endif
#include "fits.h"
/*
 * World's worst FITS file write routine. Has the advantage of being very small.
 */
#define FITS_CARD_COUNT     36
#define FITS_CARD_SIZE      80
#define FITS_RECORD_SIZE    (FITS_CARD_COUNT*FITS_CARD_SIZE)
#define BZERO               32768
/*
 * Convert unsigned LE pixels to signed BE pixels.
 */
static void convert_pixels(unsigned short *src, unsigned short *dst, unsigned short offset, int count)
{
    unsigned short pixel;

    while (count--)
    {
	pixel = *src++ - offset;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	pixel = ((pixel & 0xFF00) >> 8) | ((pixel & 0x00FF) << 8);
#endif
	*dst++ = pixel;
    }
}
/*
 * Save image to FITS file.
 */
int fits_write(char *filename, unsigned char *pixels, int width, int height, int exposure, char *creator, char *camera)
{
    char      record[FITS_CARD_COUNT][FITS_CARD_SIZE];
    unsigned short *fits_pixels;
    int       i, image_size, image_pitch, fd;

    /*
     * Create file.
     */
    if ((fd = creat(filename, 0666)) < 0)
        return fd;
    /*
     * Calculate FITS buffer size
     */
    image_pitch = width  * 2;
    image_size  = height * image_pitch;
    fits_pixels = (unsigned short *)malloc(image_pitch);
    /*
     * Fill header records.
     */
    memset(record, ' ', FITS_RECORD_SIZE);
    i = 0;
    sprintf(record[i++], "SIMPLE  = %20c", 'T');
    sprintf(record[i++], "BITPIX  = %20d", 16);
    sprintf(record[i++], "NAXIS   = %20d", 2);
    sprintf(record[i++], "NAXIS1  = %20d", width);
    sprintf(record[i++], "NAXIS2  = %20d", height);
    sprintf(record[i++], "BZERO   = %20f", (float)BZERO);
    sprintf(record[i++], "BSCALE  = %20f", 1.0);
    //sprintf(record[i++], "DATAMIN = %20u", image->datamin);
    //sprintf(record[i++], "DATAMAX = %20u", image->datamax);
    //sprintf(record[i++], "XPIXSZ  = %20f", image->pixel_width);
    //sprintf(record[i++], "YPIXSZ  = %20f", image->pixel_height);
    sprintf(record[i++], "CREATOR = '%s'", creator);
    //sprintf(record[i++], "DATE-OBS= '%sT%s'", image->date, image->time);
    sprintf(record[i++], "EXPOSURE= %20f", (float)exposure / 1000.0);
    //sprintf(record[i++], "OBJECT  = '%s'", image->object);
    sprintf(record[i++], "INSTRUME= '%s'", camera);
    //sprintf(record[i++], "OBSERVER= '%s'", image->observer);
    //sprintf(record[i++], "TELESCOP= '%s'", image->telescope);
    //sprintf(record[i++], "LOCATION= '%s'", image->location);
    sprintf(record[i++], "END");
    /*
     * Convert NULLS to spaces.
     */
    for (i = 0; i < FITS_RECORD_SIZE; i++)
        if (((char *)record)[i] == '\0')
            ((char *)record)[i] = ' ';
    write(fd, record, FITS_RECORD_SIZE);
    /*
     * Convert and write image data.
     */
    for (i = 0; i < height; i++)
    {
        convert_pixels((unsigned short *)(pixels + image_size - (i+1) * image_pitch), fits_pixels, BZERO, width);
        write(fd, fits_pixels, image_pitch);
    }
    free(fits_pixels);
    /*
     * Pad remaining record size with zeros and close.
     */
    if (image_size % FITS_RECORD_SIZE)
    {
        memset(record, 0, FITS_RECORD_SIZE);
        write(fd, record, FITS_RECORD_SIZE - (image_size % FITS_RECORD_SIZE));
    }
    return close(fd);
}
