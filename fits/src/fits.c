#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#include <sys/stat.h>
#define creat(f,m) _open(f,O_BINARY|O_WRONLY|O_CREAT,_S_IWRITE)
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
#define FITS_CARD_COMMENT   31
#define BZERO               32768
static int             fits_fd, fits_card, image_width, image_height;
static char            fits_record[FITS_CARD_COUNT+10][FITS_CARD_SIZE]; // Add a little buffer space
static unsigned short *image_pixels;

/*
 * Convert unsigned LE pixels to signed BE pixels.
 */
static void convert_pixels(unsigned short *src, unsigned short *dst, int offset, int count)
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
int fits_write_key_int(const char *key, int value, const char *comment)
{
    sprintf(fits_record[fits_card], "%-8s= %20d", key, value);
    if (comment)
        sprintf(fits_record[fits_card] + FITS_CARD_COMMENT, "/ %s", comment);
    return ++fits_card >= FITS_CARD_COUNT;
}
int fits_write_key_float(const char *key, float value, const char *comment)
{
    sprintf(fits_record[fits_card], "%-8s= %20f", key, value);
    if (comment)
        sprintf(fits_record[fits_card] + FITS_CARD_COMMENT, "/ %s", comment);
    return ++fits_card >= FITS_CARD_COUNT;
}
int fits_write_key_string(const char *key, const char * value, const char *comment)
{
    sprintf(fits_record[fits_card], "%-8s= '%s'", key, value);
    if (comment)
        sprintf(fits_record[fits_card] + FITS_CARD_COMMENT + (strlen(value) > 18 ? strlen(value) - 18 : 0), "/ %s", comment);
    return ++fits_card >= FITS_CARD_COUNT;
}
int fits_write_image(unsigned short *pixels, int width, int height)
{
    /*
     * Fill out image header values.
     */
    sprintf(fits_record[fits_card++], "BITPIX  = %20d", 16);
    sprintf(fits_record[fits_card++], "NAXIS   = %20d", 2);
    sprintf(fits_record[fits_card++], "NAXIS1  = %20d", width);
    sprintf(fits_record[fits_card++], "NAXIS2  = %20d", height);
    sprintf(fits_record[fits_card++], "BZERO   = %20f", (float)BZERO);
    sprintf(fits_record[fits_card++], "BSCALE  = %20f", 1.0);
    /*
     * Save image values.
     */
    image_width  = width;
    image_height = height;
    image_pixels = (unsigned short *)pixels;
    return fits_card >= FITS_CARD_COUNT;
}
/*
 * Init FITS header and image array.
 */
int fits_open(const char *filename)
{
    /*
     * Create file.
     */
    if ((fits_fd = creat(filename, 0666)) < 0)
        return fits_fd;
    /*
     * Init header and pixel pointers
     */
    memset(fits_record, ' ', FITS_RECORD_SIZE);
    sprintf(fits_record[0], "SIMPLE  = %20c", 'T');
    fits_card = 1;
    return 0;
}
/*
 * Write out header and image array then close file.
 */
int fits_close(void)
{
    int i, image_end, image_pitch;
    unsigned short *fits_pixels;
    /*
     * End header and convert NULLS to spaces.
     */
    sprintf(fits_record[fits_card], "END");
    for (i = 0; i < FITS_RECORD_SIZE; i++)
        if (((char *)fits_record)[i] < ' ')
            ((char *)fits_record)[i] = ' ';
    if (write(fits_fd, fits_record, FITS_RECORD_SIZE) != FITS_RECORD_SIZE)
        return -1;
    /*
     * Convert and write image data.
     */
    image_end  = image_width * (image_height - 1);
    image_pitch = image_width * 2;
    fits_pixels = (unsigned short *)malloc(image_pitch);
    for (i = 0; i < image_height; i++)
    {
        convert_pixels(image_pixels + image_end - i * image_width, fits_pixels, BZERO, image_width);
        if (write(fits_fd, fits_pixels, image_pitch) != image_pitch)
        {
            free(fits_pixels);
            return -1;
        }
    }
    free(fits_pixels);
    return close(fits_fd);
}
int fits_cleanup(void)
{
    if (fits_fd > 0)
        close(fits_fd);
    return (fits_fd = 0);
}
#if 0
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
#endif
