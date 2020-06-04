#include <math.h>
#include "aip.h"
#ifdef __cplusplus
extern "C" {
#endif
/*
 * 16 bit image sample to RGB pixel LUT
 */
unsigned char redLUT[LUT_SIZE];
unsigned char blugrnLUT[LUT_SIZE];
void calcRamp(int black, int white, float gamma, int filter)
{
    int pix, offset;
    float scale, recipg, pixClamp;

    offset = LUT_INDEX(black) - 1;
    scale  = (float)MAX_PIX/(white - black);
    recipg = 1.0/gamma;
    for (pix = 0; pix < LUT_SIZE; pix++)
    {
        pixClamp = ((float)(pix - offset)/(LUT_SIZE-1)) * scale;
        if (pixClamp > 1.0) pixClamp = 1.0;
        else if (pixClamp < 0.0) pixClamp = 0.0;
        redLUT[pix]    = 255.0 * pow(pixClamp, recipg);
        blugrnLUT[pix] = filter ? 0 : redLUT[pix];
    }
}
/*
 * Star registration
 */
void calcCentroid(int width, int height, unsigned short *pixels, int x, int y, int x_radius, int y_radius, float *x_centroid, float *y_centroid, int min)
{
    int   i, j;
    float sum;

    *x_centroid = 0.0;
    *y_centroid = 0.0;
    sum         = 0.0;

    for (j = y - y_radius; j <= y + y_radius; j++)
        for (i = x - x_radius; i <= x + x_radius; i++)
            if (pixels[j * width + i] > min)
            {
                *x_centroid += i * pixels[j * width + i];
                *y_centroid += j * pixels[j * width + i];
                sum         +=     pixels[j * width + i];
            }
    if (sum != 0.0)
    {
        *x_centroid /= sum;
        *y_centroid /= sum;
    }
}
int findBestCentroid(int width, int height, unsigned short *pixels, float *x_centroid, float *y_centroid, int x_range, int y_range, int *x_max_radius, int *y_max_radius, float sigs)
{
    int   i, j, x, y, x_min, x_max, y_min, y_max, x_radius, y_radius;
    int   pixel, pixel_min, pixel_max;
    float pixel_ave, pixel_sig;

    x           = (int)*x_centroid;
    y           = (int)*y_centroid;
    x_min       = x - x_range;
    x_max       = x + x_range;
    y_min       = y - y_range;
    y_max       = y + y_range;
    x           = -1;
    y           = -1;
    x_radius    = 0;
    y_radius    = 0;
    pixel_ave   = 0.0;
    pixel_sig   = 0.0;
    if (x_min < *x_max_radius)        x_min = *x_max_radius;
    if (x_max > width-*x_max_radius)  x_max = width-*x_max_radius;
    if (y_min < *y_max_radius)        y_min = *y_max_radius;
    if (y_max > height-*y_max_radius) y_max = height-*y_max_radius;
    for (j = y_min; j < y_max; j++)
        for (i = x_min; i < x_max; i++)
            pixel_ave += pixels[j * width + i];
    pixel_ave /= (y_max - y_min) * (x_max - x_min);
    for (j = y_min; j < y_max; j++)
        for (i = x_min; i < x_max; i++)
            pixel_sig += (pixel_ave - pixels[j * width + i]) * (pixel_ave - pixels[j * width + i]);
    pixel_sig = sqrt(pixel_sig / ((y_max - y_min) * (x_max - x_min) - 1));
    pixel_max = pixel_min = pixel_ave + pixel_sig * sigs;
    for (j = y_min; j < y_max; j++)
    {
        for (i = x_min; i < x_max; i++)
        {
            if (pixels[j * width + i] > pixel_max)
            {
                pixel = pixels[j * width + i];
                /*
                 * Local maxima
                 */
                if ((pixel >= pixels[j * width + i + 1])
                 && (pixel >= pixels[j * width + i - 1])
                 && (pixel >= pixels[(j + 1) * width + i])
                 && (pixel >= pixels[(j - 1) * width + i]))
                {
                    /*
                     * Avoid hot pixels
                     */
                     if ((pixel_min < pixels[j * width + i + 1])
                      && (pixel_min < pixels[j * width + i - 1])
                      && (pixel_min < pixels[(j + 1) * width + i])
                      && (pixel_min < pixels[(j - 1) * width + i]))
                    {
                        /*
                         * Find radius of highlight
                         */
                        for (y_radius = 1; (y_radius <= *y_max_radius)
                                        && ((pixels[(j + y_radius) * width + i] > pixel_min)
                                         || (pixels[(j - y_radius) * width + i] > pixel_min)); y_radius++);
                        for (x_radius = 1; (x_radius <= *x_max_radius)
                                        && ((pixels[j * width + i + x_radius] > pixel_min)
                                         || (pixels[j * width + i - x_radius] > pixel_min)); x_radius++);
                        /*
                         * If its really big, skip it.  Something like the moon
                         */
                        if (x_radius < *x_max_radius && y_radius < *y_max_radius)
                        {
                            pixel_max = pixel;
                            calcCentroid(width, height, pixels, i, j, x_radius, y_radius, x_centroid, y_centroid, pixel_min);
                            x = (int)(*x_centroid + 0.5);
                            y = (int)(*y_centroid + 0.5);
                            calcCentroid(width, height, pixels, x, y, x_radius, y_radius, x_centroid, y_centroid, pixel_min);
                        }
                    }
                }
            }
        }
    }
    if  (x >= 0 && y >= 0)
    {
        *x_max_radius = x_radius;
        *y_max_radius = y_radius;
    }
    return x >= 0 && y >= 0;
}
#ifdef __cplusplus
}
#endif
