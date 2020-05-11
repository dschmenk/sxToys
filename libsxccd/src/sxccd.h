/***************************************************************************\

    Copyright (c) 2001-2013 David Schmenk

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

#ifndef _SXCCD_H_
#define _SXCCD_H_
/*
 * CCD model parameters.
 */
#define SXCCD_GUIDER			0x8000
#define SXCCD_INTERLEAVE		0x0040
#define SXCCD_COLOR			0x0080
#define SXCCD_HX5                       0x0005
#define SXCCD_HX9                       0x0009
#define SXCCD_MX5                       0x0045
#define SXCCD_MX5C                      0x00C5
#define SXCCD_MX7                       0x0047
#define SXCCD_MX7C                      0x00C7
#define SXCCD_MX9                       0x0049
#define SXCCD_HX_IMAGE_FIELDS           1
#define SXCCD_MX_IMAGE_FIELDS           2
/*
 * CCD color representation.
 *  Packed colors allow individual sizes up to 16 bits.
 *  2X2 matrix bits are represented as:
 *      0 1
 *      2 3
 */
#define SXCCD_COLOR_PACKED_RGB          0x8000
#define SXCCD_COLOR_PACKED_BGR          0x4000
#define SXCCD_COLOR_PACKED_RED_SIZE     0x0F00
#define SXCCD_COLOR_PACKED_GREEN_SIZE   0x00F0
#define SXCCD_COLOR_PACKED_BLUE_SIZE    0x000F
#define SXCCD_COLOR_MATRIX_ALT_EVEN     0x2000
#define SXCCD_COLOR_MATRIX_ALT_ODD      0x1000
#define SXCCD_COLOR_MATRIX_2X2          0x0000
#define SXCCD_COLOR_MATRIX_RED_MASK     0x0F00
#define SXCCD_COLOR_MATRIX_GREEN_MASK   0x00F0
#define SXCCD_COLOR_MATRIX_BLUE_MASK    0x000F
#define SXCCD_COLOR_MONOCHROME          0x0FFF
/*
 * Caps bit definitions.
 */
#define SXCCD_CAPS_STAR2K               0x01
#define SXCCD_CAPS_COMPRESS             0x02
#define SXCCD_CAPS_EEPROM               0x04
#define SXCCD_CAPS_GUIDER               0x08
/*
 * CCD command options.
 */
#define SXCCD_EXP_FLAGS_FIELD_ODD     	1
#define SXCCD_EXP_FLAGS_FIELD_EVEN    	2
#define SXCCD_EXP_FLAGS_FIELD_BOTH    	(SXCCD_EXP_FLAGS_FIELD_EVEN|SXCCD_EXP_FLAGS_FIELD_ODD)
#define SXCCD_EXP_FLAGS_FIELD_MASK    	SXCCD_EXP_FLAGS_FIELD_BOTH
#define SXCCD_EXP_FLAGS_NOBIN_ACCUM   	4
#define SXCCD_EXP_FLAGS_NOWIPE_FRAME  	8
#define SXCCD_EXP_FLAGS_TDI_SCAN        32
#define SXCCD_EXP_FLAGS_NOCLEAR_FRAME 	64
/*
 * Buffer size given frame parameters.
#define FRAMEBUF_SIZE(w,h,d,x,y) (((((w)+(x)-1)/(x))*(((h)+(y)-1)/(y)))*((d)/8))
 */
#define FRAMEBUF_SIZE(w,h,d,x,y) ((((w)/(x))*((h)/(y)))*((d)/8))
/*
 * Functions.
 */
int sxccd_open(int defmodel);
void sxccd_close(void);
int sxccd_get_model(unsigned int cam_idx);
int sxccd_set_model(unsigned int cam_idx, int model);
int sxccd_get_frame_dimensions(unsigned int cam_idx, unsigned int *width, unsigned int *height, unsigned int *depth);
int sxccd_get_pixel_dimensions(unsigned int cam_idx, unsigned int *pixwidth, unsigned int *pixheight);
int sxccd_get_caps(unsigned int cam_idx, unsigned int *caps, unsigned int *ports);
int sxcd_reset(unsigned int cam_idx);
int sxccd_clear_frame(unsigned int cam_idx, unsigned int options);
int sxccd_read_pixels_delayed(unsigned int cam_idx, unsigned int options, unsigned int xoffset, unsigned int yoffset, unsigned int width, unsigned int height, unsigned int xbin, unsigned int ybin, unsigned int msec, unsigned char *pixbuf);
int sxccd_read_pixels(unsigned int cam_idx, unsigned int options, unsigned int xoffset, unsigned int yoffset, unsigned int width, unsigned int height, unsigned int xbin, unsigned int ybin, unsigned char *pixbuf);
/*
 * Pretty names
 */
#define sxOpen                  sxccd_open
#define sxClose                 sxccd_close
#define sxGetModel              sxccd_get_model
#define sxSetModel              sxccd_set_model
#define sxGetFrameDimensions    sxccd_get_frame_dimensions
#define sxGetPixelDimensions    sxccd_get_pixel_dimensions
#define sxGetCaps               sxccd_get_caps
#define sxReset                 sxccd_reset
#define sxClearFrame            sxccd_clear_frame
#define sxReadPixelsDelayed     sxccd_read_pixels_delayed
#define sxReadPixels            sxccd_read_pixels
#endif /* _SXCCD_H_ */
