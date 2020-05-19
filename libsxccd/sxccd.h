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

#define VERSION_MSD 1
#define VERSION_LSD 4

#ifdef _MSC_VER_
#ifdef __cplusplus
#define DLL_EXPORT                    extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT                    __declspec(dllexport)
#endif
#else
#ifdef __cplusplus
#define DLL_EXPORT                    extern "C"
#else
#define DLL_EXPORT
#endif
typedef void *         HANDLE;
typedef          long  LONG;
typedef unsigned long  ULONG;
typedef          short SHORT;
typedef unsigned short USHORT;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;
#endif
/*
Version 1.4 - 13/03/2015
Added commands
sxGetCoolerVer(HANDLE sxHandle)							// Reads cooler firmware version.
sxSetSerialNumber(HANDLE sxHandle, BYTE *serialNumber)	// Writes 32 byte serial number to eeprom.
*/
/*
 * CCD model parameters.
 */
#define SXCCD_GUIDER					0x8000
#define SXCCD_HX_IMAGE_FIELDS           1
#define SXCCD_MX_IMAGE_FIELDS           2
#define SXCCD_INTERLEAVE				0x0040
#define SXCCD_COLOR						0x0080
#define SXCCD_HX5                       0x0005
#define SXCCD_HX9                       0x0009
#define SXCCD_MX5                       0x0045
#define SXCCD_MX5C                      0x00C5
#define SXCCD_MX7                       0x0047
#define SXCCD_MX7C                      0x00C7
#define SXCCD_MX9                       0x0049
#define SXCCD_MX8C                      0xC8
#define SXCCD_M25C		 				0x59
#define SXCCD_M26C		 				0x5A
#define SXCCD_H5		 				0x05
#define SXCCD_H5C		 				0x85
#define SXCCD_H9		 				0x09
#define SXCCD_H9C		 				0x89
#define SXCCD_H16		 				0x10
#define SXCCD_H18		 				0x12
#define SXCCD_H35		 				0x23
#define SXCCD_H36		 				0x24
#define SXCCD_H290		 				0x58
#define SXCCD_H694		 				0x57
#define SXCCD_BR694		 				0x20
#define SXCCD_H674		 				0x56
#define SXCCD_H694C		 				0xB7
#define SXCCD_H674C		 				0xB6
#define SXCCD_SuperStarC 				0x3A
#define SXCCD_SuperStarM 				0x19
#define SXCCD_LodeStar   				0x46
#define SXCCD_CoStar     				0x17
#define SXCCD_H814		 				0x28
#define SXCCD_H814C		 				0xA8
#define SXCCD_H825		 				0x3B
#define SXCCD_H825C		 				0xBB
#define SXCCD_UltraStarC 				0xBC
#define SXCCD_UltraStarM 				0x3C
#define SXCCD_H834		 				0x29
#define SXCCD_H834C		 				0xA9
#define SXCCD_H17		 				0x11
#define SXCCD_H17C		 				0x91
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
* Capabilities (Caps) bit definitions.
 */
#define SXCCD_CAPS_STAR2K               0x01
#define SXCCD_CAPS_COMPRESS             0x02
#define SXCCD_CAPS_EEPROM               0x04
#define SXCCD_CAPS_GUIDER               0x08
#define SXCCD_CAPS_COOLER				0x10
#define SXCCD_CAPS_SHUTTER				0x20
/*
 * CCD camera index.
 */
#define SXCCD_IMAGE_HEAD                0
#define SXCCD_GUIDE_HEAD                1
/*
 * CCD command flags bit definitions.
 */
#define SXCCD_EXP_FLAGS_FIELD_ODD     	0x01
#define SXCCD_EXP_FLAGS_FIELD_EVEN    	0x02
#define SXCCD_EXP_FLAGS_FIELD_BOTH    	(SXCCD_EXP_FLAGS_FIELD_EVEN|SXCCD_EXP_FLAGS_FIELD_ODD)
#define SXCCD_EXP_FLAGS_FIELD_MASK    	SXCCD_EXP_FLAGS_FIELD_BOTH
#define SXCCD_EXP_FLAGS_SPARE2		   	0x04
#define SXCCD_EXP_FLAGS_NOWIPE_FRAME  	0x08
#define SXCCD_EXP_FLAGS_SPARE4		   	0x10
#define SXCCD_EXP_FLAGS_TDI           	0x20
#define SXCCD_EXP_FLAGS_NOCLEAR_FRAME 	0x40
#define SXCCD_EXP_FLAGS_NOCLEAR_REGISTER 0x80
// Upper bits in byte of CCD_EXP_FLAGS word
#define SXCCD_EXP_FLAGS_SPARE8			0x01
#define SXCCD_EXP_FLAGS_SPARE9		   	0x02
#define SXCCD_EXP_FLAGS_SPARE10		   	0x04
#define SXCCD_EXP_FLAGS_SPARE11		   	0x08
#define SXCCD_EXP_FLAGS_SPARE12		   	0x10
#define SXCCD_EXP_FLAGS_SHUTTER_MANUAL  0x20
#define SXCCD_EXP_FLAGS_SHUTTER_OPEN    0x40
#define SXCCD_EXP_FLAGS_SHUTTER_CLOSE   0x80
/*
 * Serial port queries.
 */
#define	SXCCD_SERIAL_PORT_AVAIL_OUTPUT	0
#define	SXCCD_SERIAL_PORT_AVAIL_INPUT	1
/*
 * Limits.
 */
#define	SXCCD_MAX_CAMS                 	20
#define SXCCD_SERIAL_NUMBER_SIZE		32		/* total number of bytes for serial number */
#define SXCCD_SNO_WRITE_BLOCK			4		/* no of bytes of serial number to write */
/*
 * Errors.
 */
#define SX_ERROR                        0
#define SX_SUCCESS                      1
/*
 * Buffer size & pixel count given frame parameters.
 */
#define FRAMEBUF_SIZE(w,h,d,x,y)        ((((w)/(x))*((h)/(y)))*((d)/8))
#define FRAMEBUF_COUNT(w,h,x,y)         ((((w)/(x))*((h)/(y))))
/*
 * Structure to hold camera information.
 */
struct sxccd_params
{
    USHORT hfront_porch;
    USHORT hback_porch;
    USHORT width;
    USHORT vfront_porch;
    USHORT vback_porch;
    USHORT height;
    float  pix_width;
    float  pix_height;
    USHORT color_matrix;
    BYTE   bits_per_pixel;
    BYTE   num_serial_ports;
    BYTE   extra_caps;
    BYTE   vclk_delay;
};
typedef struct sxccd_params t_sxccd_params;
/*
 * Prototypes.
 */
DLL_EXPORT LONG   sxReset(HANDLE sxHandle);
DLL_EXPORT LONG   sxClearPixels(HANDLE sxHandle, USHORT flags, USHORT camIndex);
DLL_EXPORT LONG   sxLatchPixels(HANDLE sxHandle, USHORT flags, USHORT camIndex, USHORT xoffset, USHORT yoffset, USHORT width, USHORT height, USHORT xbin, USHORT ybin);
DLL_EXPORT LONG   sxExposePixels(HANDLE sxHandle, USHORT flags, USHORT camIndex, USHORT xoffset, USHORT yoffset, USHORT width, USHORT height, USHORT xbin, USHORT ybin, ULONG msec);
DLL_EXPORT LONG   sxExposePixelsGated(HANDLE sxHandle, USHORT flags, USHORT camIndex, USHORT xoffset, USHORT yoffset, USHORT width, USHORT height, USHORT xbin, USHORT ybin, ULONG msec);
DLL_EXPORT LONG   sxReadPixels(HANDLE sxHandle, USHORT *pixels, ULONG count);
DLL_EXPORT LONG   sxSetShutter(HANDLE sxHandle, USHORT state);
DLL_EXPORT ULONG  sxSetTimer(HANDLE sxHandle, ULONG msec);
DLL_EXPORT ULONG  sxGetTimer(HANDLE sxHandle);
DLL_EXPORT ULONG  sxGetCameraParams(HANDLE sxHandle, USHORT camIndex, t_sxccd_params *params);
DLL_EXPORT ULONG  sxSetSTAR2000(HANDLE sxHandle, BYTE star2k);
DLL_EXPORT ULONG  sxSetSerialPort(HANDLE sxHandle, USHORT portIndex, USHORT property, ULONG value);
DLL_EXPORT USHORT sxGetSerialPort(HANDLE sxHandle, USHORT portIndex, USHORT property);
DLL_EXPORT ULONG  sxWriteSerialPort(HANDLE sxHandle, USHORT portIndex, USHORT flush, USHORT count, BYTE *data);
DLL_EXPORT ULONG  sxReadSerialPort(HANDLE sxHandle, USHORT portIndex, USHORT count, BYTE *data);
DLL_EXPORT USHORT sxGetCameraModel(HANDLE sxHandle);
DLL_EXPORT ULONG  sxSetCameraModel(HANDLE sxHandle, USHORT model);
DLL_EXPORT ULONG  sxGetFirmwareVersion(HANDLE sxHandle);
DLL_EXPORT USHORT sxGetBuildNumber(HANDLE sxHandle);
DLL_EXPORT int    sxOpen(HANDLE *sxHandles);
DLL_EXPORT void   sxClose(HANDLE sxHandle);
DLL_EXPORT ULONG  sxSetCooler(HANDLE sxHandle, UCHAR SetStatus, USHORT SetTemp, UCHAR *RetStatus, USHORT *RetTemp );
DLL_EXPORT ULONG  sxGetCoolerTemp(HANDLE sxHandle, UCHAR *RetStatus, USHORT *RetTemp );
DLL_EXPORT ULONG  sxGetDLLVersion();
#ifdef SX_CUSTOM
DLL_EXPORT ULONG  sxGetSerialNumber(HANDLE sxHandle, BYTE *serialNumber);
DLL_EXPORT LONG   sxSetSerialNumber(HANDLE sxHandle, BYTE *serialNumber);
DLL_EXPORT LONG   sxReadPixels2(HANDLE sxHandle, USHORT *pixels, USHORT width, USHORT height, USHORT border);
DLL_EXPORT ULONG  sxReadEEPROM(HANDLE sxHandle, USHORT address, USHORT count, BYTE *data);
DLL_EXPORT USHORT sxGetCoolerVer(HANDLE sxHandle);
DLL_EXPORT ULONG  sxSetCameraParams(HANDLE sxHandle, USHORT camIndex, t_sxccd_params *params);
DLL_EXPORT ULONG  sxSetCameraModel(HANDLE sxHandle, USHORT model);
DLL_EXPORT ULONG  sxWriteEEPROM(HANDLE sxHandle, USHORT address, USHORT count, BYTE *data, USHORT admin_code);
#endif
#endif /* _SXCCD_H_ */
