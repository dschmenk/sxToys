#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include "../sxccd.h"
/*
 * Vendor specific control commands.
 */
#define SXUSB_ECHO                  0
#define SXUSB_CLEAR_PIXELS          1
#define SXUSB_READ_PIXELS_DELAYED   2
#define SXUSB_READ_PIXELS           3
#define SXUSB_SET_TIMER             4
#define SXUSB_GET_TIMER             5
#define SXUSB_RESET                 6
#define SXUSB_SET_CCD               7
#define SXUSB_GET_CCD               8
#define SXUSB_SET_GUIDE_PORT        9
#define SXUSB_WRITE_SERIAL_PORT     10
#define SXUSB_READ_SERIAL_PORT      11
#define SXUSB_SET_SERIAL            12
#define SXUSB_GET_SERIAL            13
#define SXUSB_CAMERA_MODEL          14
#define SXUSB_LOAD_EEPROM           15
#define SXUSB_SET_A2D               16  // Set A2D Configuration registers
#define SXUSB_RED_A2D               17  // Set A2D Red Offest & Gain registers
#define SXUSB_READ_PIXELS_GATED     18  // IOE_7 triggers timed exposure
#define SXUSB_BUILD_NUMBER          19  // Sends firmware build number available from version 1.16
#define SXUSB_SERIAL_NUMBER         20  // Sends camera serial number available from version 1.23
#define SXUSB_STOP_STREAMING        22  // Stops streaming video data from the camera (IMX only)
#define SXUSB_SXIMX_SINGLE_EXP      23  // Single Exposure only (SXIMX only)
#define SXUSB_STOP_SINGLE_EXP       24  // Stops a single exposure if possible (IMX only)
#define SXUSB_STREAM_VIDEO          29  // Sends video image data from camera (IMX only)
// USB commands only useable on cameras with cooler control
#define SXUSB_COOLER_CONTROL        30  // Sets cooler "set Point" & reports current cooler temperature
#define SXUSB_COOLER                30
#define SXUSB_COOLER_TEMPERATURE    31  // Reports cooler temperature
// USB commands only useable on cameras with shutter control
// Check "Caps" bits in t_sxccd_params
#define SXUSB_SHUTTER_CONTROL       32  // Controls shutter & spare cooler MCU port bits
#define SXUSB_SHUTTER               32
#define SXUSB_READ_I2CPORT          33  // Returns shutter status  & spare cooler MCU port bits also sets shutter delay period
// Commands for any recent (2015) camera
#define SXUSB_COOLER_VERSION        34  // Returns the cooler mcu firmware version
#define SXUSB_FAN_CTL               35  // Controls the fan
#define SXUSB_LED_CTL               36  // Controls the led status on IMX
// USB command to provide further extended capabilities
#define SXUSB_EXTENDED_CAPS         40  // Returns a DWORD for a further 32 flags
#define SXUSB_HW_TYPE               41
#define SXUSB_USER_ID               42  // Reads or Writes a 16bit user ID
#define SXUSB_FLOOD_CCD             43  // Flood CCD command currently for H21 only
// USB command for SXIMX camera
#define WRITE_IMX_REG               50  // Writes a byte to the specified register address
#define SX_THS_DELAY                51  // Changes THS delay command
#define SX_MASTER_SLAVE             52  // Switches the camera master/slave mode command
#define CX3_RESET_EP3               54  // Resets & Aborts EP3 (IMX only)
#define READ_IMX_REG                55  // Reads a single IMX register (IMX only)
#define IMX_TEST                    56  // SXIMX Test functions
#define WRITE_IMX_PARAM             57  // Write an IMX parameter (gain, black level etc)
#define LIMIT_IMX_PARAM             58  // Reads the upper limit of an IMX parameter (gain, black level etc)
#define IMX_PATTERN                 59  // Sets the pattern generator pattern for SX294
/*
 * USB bulk block size to read at a time.
 * Must be multiple of bulk transfer buffer size.
 */
#define SX_BULK_READ_SIZE           (1L<<13)
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
#define USB_REQ_DATA                8
/*
 * Minimum time (msec) before letting the camera time the exposure.
 */
#define SX_MIN_EXP_TIME             1000
/*
 * Vendor and product IDs.
 */
#define EZUSB_VENDOR_ID             0x0547
#define EZUSB_PRODUCT_ID            0x2131
#define EZUSB2_VENDOR_ID            0x04B4
#define EZUSB2_PRODUCT_ID           0x8613
#define SX_VENDOR_ID                0x1278
#define ECHO2_PRODUCT_ID            0x0100
#define ECHO3_PRODUCT_ID            0x0200
/*
 * Set and reset 8051 requests.
 */
#define EZUSB_CPUCS_REG             0x7F92
#define EZUSB2_CPUCS_REG            0xE600
#define CPUCS_RESET                 0x01
#define CPUCS_RUN                   0x00
/*
 * Address in 8051 external memory for debug info and CCD parameters.
 * Must use low address alias for EZ-USB
 */
#define SX_EZUSB_DEBUG_BUF          0x1C80
#define SX_EZUSB2_DEBUG_BUF         0xE1F0
/*
 * EZ-USB code download requests.
 */
#define EZUSB_FIRMWARE_LOAD         0xA0
/*
 * EZ-USB download code for each camera. Converted from .HEX file.
 */
struct sx_ezusb_download_record
{
    int           addr;
    int           len;
    unsigned char data[16];
};
static struct sx_ezusb_download_record sx_ezusb_code[] =
{
    #include "sx_ezusb_code.h"
};
static struct sx_ezusb_download_record sx_ezusb2_code[] =
{
    #include "sx_ezusb2_code.h"
};
/*
 * Max number of cameras supported.
 */
#define MAX_CAMS 20
/*
 * SX CCD Camera structure.
 */
static struct sx_cam
{
    libusb_device_handle *handle;
    int snd_endpoint;
    int rcv_endpoint;
    unsigned int model;
} sx_cams[MAX_CAMS];
static unsigned int sx_cnt = 0;
/*
 * Download code to EZ-USB device.
 */
static int sx_ezusb_download(int idVendor, libusb_device *dev)
{
    unsigned int                     i, j, ezusb_cpucs_reg, code_rec_count;
    unsigned char                    setup_data[20];
    struct sx_ezusb_download_record *code_rec;
    libusb_device_handle *handle;

    if (idVendor == EZUSB_VENDOR_ID)
    {
        code_rec        = sx_ezusb_code;
        code_rec_count  = sizeof(sx_ezusb_code)/sizeof(struct sx_ezusb_download_record);
        ezusb_cpucs_reg = EZUSB_CPUCS_REG;
    }
    else //if (idVendor == EZUSB2_VENDOR_ID)
    {
        code_rec        = sx_ezusb2_code;
        code_rec_count  = sizeof(sx_ezusb2_code)/sizeof(struct sx_ezusb_download_record);
        ezusb_cpucs_reg = EZUSB2_CPUCS_REG;
    }
    /*
     * Put 8051 into RESET.
     */
    libusb_open(dev, &handle);
    setup_data[0] = CPUCS_RESET;
    if (libusb_control_transfer(handle,
                                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                                EZUSB_FIRMWARE_LOAD,
                                ezusb_cpucs_reg,
                                0,
                                setup_data,
                                1,
                                1000) < 0)
    {
        printf("sx_ezusb: could not put 8051 into RESET\n");
        libusb_close(handle);
        return SX_ERROR;
    }
    //printf("sx_ezusb: RESET 8051\n");
    //printf("sx_ezusb: Downloading %d code records\n", code_rec_count);
    for (i = 0; i < code_rec_count; i++)
    {
        /*
         * Download code record.
         */
        j = code_rec[i].len;
        memcpy(setup_data, code_rec[i].data, j);
        if (libusb_control_transfer(handle,
                                    LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                                    EZUSB_FIRMWARE_LOAD,
                                    code_rec[i].addr,
                                    0,
                                    setup_data,
                                    j,
                                    1000) < j)
        {
            printf("sx_ezusb: could not download code into 8051\n");
            libusb_close(handle);
            return SX_ERROR;
        }
    }
    /*
     * Take 8051 out of RESET.
     */
    setup_data[0] = CPUCS_RUN;
    if (libusb_control_transfer(handle,
                                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                                EZUSB_FIRMWARE_LOAD,
                                ezusb_cpucs_reg,
                                0,
                                setup_data,
                                1,
                                1000) < 0)
    {
        printf("sz_ezusb: could not take 8051 out of RESET\n");
        libusb_close(handle);
        return SX_ERROR;
    }
    //printf("sx_ezusb: un-RESET 8051\n");
    libusb_close(handle);
    return SX_SUCCESS;
}
/*
 * Get CCD parameters.
 */
ULONG sxGetCameraParams(HANDLE sxHandle, USHORT camIndex, t_sxccd_params *params)
{
    unsigned char cam_data[32];
    struct sx_cam *pCam = sxHandle;
    if (libusb_control_transfer(pCam->handle,
                                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
                                SXUSB_GET_CCD,
                                0,
                                camIndex,
                                cam_data,
                                17,
                                1000) < 0)
    {
        printf("Could not get CCD parameters\n");
        return SX_ERROR;
    }
    params->hfront_porch     = cam_data[0];
    params->hback_porch      = cam_data[1];
    params->vfront_porch     = cam_data[4];
    params->vback_porch      = cam_data[5];
    params->width            = cam_data[2]  | (cam_data[3]  << 8);
    params->height           = cam_data[6]  | (cam_data[7]  << 8);
    params->bits_per_pixel   = cam_data[14];
    params->pix_width        =(cam_data[8]  | (cam_data[9]  << 8)) / 256.0;
    params->pix_height       =(cam_data[10] | (cam_data[11] << 8)) / 256.0;
    params->num_serial_ports = cam_data[15];
    params->extra_caps       = cam_data[16];
    //printf("SX camera width:%d height:%d depth:%d caps:%02X\n", sx_cams[sx_cnt].width, sx_cams[sx_cnt].height, sx_cams[sx_cnt].depth, sx_cams[sx_cnt].caps);
    return SX_SUCCESS;
}
/*
 * Open all SX CCD cameras.
 */
int sxOpen(HANDLE *sxHandles)
{
    int devc, renum, i;
    unsigned char cam_data[32];
    libusb_device *sx_dev[MAX_CAMS];
    libusb_device **devv;
    struct libusb_device_descriptor desc;
    struct libusb_config_descriptor *config;

    if (sx_cnt)
    {
        while (sx_cnt--) libusb_close(sx_cams[sx_cnt].handle);
        sx_cnt = 0;
    }
    else
        libusb_init(NULL);
    devc  = libusb_get_device_list(NULL, &devv);
    renum = 0;
    for (i = 0; i < devc; i++)
    {
        /*
         * Look for any unconfigured EZUSB(2) devices.
         */
        libusb_get_device_descriptor(devv[i], &desc);
        if ((desc.idVendor == EZUSB_VENDOR_ID  && desc.idProduct == EZUSB_PRODUCT_ID)
         || (desc.idVendor == EZUSB2_VENDOR_ID && desc.idProduct == EZUSB2_PRODUCT_ID))
            renum += sx_ezusb_download(desc.idVendor, devv[i]);
    }
    if (renum)
    {
        /*
         * Free device list, wait for device renumeration, and get updated list.
         */
        libusb_free_device_list(devv, 1);
        sleep(3);
        devc = libusb_get_device_list(NULL, &devv);
    }
    while (devc-- > 0)
    {
        /*
         * Look for SX cameras.
         */
        libusb_get_device_descriptor(devv[devc], &desc);
        if (desc.idVendor == SX_VENDOR_ID)
        {
            sx_dev[sx_cnt] = devv[devc];
            //printf("Found Starlight Xpress camera (PID=%04X)\n", desc.idProduct);
            if (libusb_open(sx_dev[sx_cnt], &sx_cams[sx_cnt].handle) < 0)
            {
                printf("Can't open camera.\n");
                libusb_free_device_list(devv, 1);
                return 0;
            }
            libusb_get_config_descriptor(sx_dev[sx_cnt], 0, &config);
            libusb_set_configuration(sx_cams[sx_cnt].handle, config->bConfigurationValue);
            libusb_claim_interface(sx_cams[sx_cnt].handle, config->interface[0].altsetting[0].bInterfaceNumber);
            libusb_set_interface_alt_setting(sx_cams[sx_cnt].handle, config->interface[0].altsetting[0].bInterfaceNumber, config->interface[0].altsetting[0].bAlternateSetting);
            for (i = 0; i < config->interface[0].altsetting[0].bNumEndpoints; i++)
            {
                if (config->interface[0].altsetting[0].endpoint[i].bEndpointAddress & LIBUSB_ENDPOINT_IN)
                    sx_cams[sx_cnt].rcv_endpoint = config->interface[0].altsetting[0].endpoint[i].bEndpointAddress;
                else
                    sx_cams[sx_cnt].snd_endpoint = config->interface[0].altsetting[0].endpoint[i].bEndpointAddress;
            }
            /*
             * Reset camera first.
             */
            if (libusb_control_transfer(sx_cams[sx_cnt].handle,
                                        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                                        SXUSB_RESET,
                                        0,
                                        0,
                                        NULL,
                                        0,
                                        1000) < 0)
            {
                printf("Error reseting camera.\n");
                break;
            }
            /*
             * Read model.
             */
            if (libusb_control_transfer(sx_cams[sx_cnt].handle,
                                        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
                                        SXUSB_CAMERA_MODEL,
                                        0,
                                        0,
                                        cam_data,
                                        2,
                                        1000) < 0)
            {
                printf("Error reading camera model.\n");
                break;
            }
            else
            {
                sx_cams[sx_cnt].model = cam_data[0] | (cam_data[1] << 8);
                //printf("SX camera model: %04X\n", sx_cams[sx_cnt].model);
                sxHandles[sx_cnt] = &sx_cams[sx_cnt];
                sx_cnt++;
            }
            libusb_free_config_descriptor(config);
        }
    }
    libusb_free_device_list(devv, 1);
    return sx_cnt;
}
/*
 * Close all SX CCD cameras.
 */
void sxClose(HANDLE sxHandle)
{
    struct sx_cam *pCam = sxHandle;
    libusb_close(pCam->handle);
    if (--sx_cnt == 0)
        libusb_exit(NULL);
}
/*
 * Get camera parameters.
 */
USHORT sxGetCameraModel(HANDLE sxHandle)
{
    struct sx_cam *pCam = sxHandle;
    return pCam->model;
}
ULONG sxSetCameraModel(HANDLE sxHandle, USHORT newmodel)
{
    unsigned char cam_data[32];
    struct sx_cam *pCam = sxHandle;
    //printf("Setting camera model to %02X\n", newmodel);
    if (libusb_control_transfer(pCam->handle,
                                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                                SXUSB_CAMERA_MODEL,
                                newmodel,
                                0,
                                NULL,
                                0,
                                1000) < 0)
    {
        printf("Error setting camera model.\n");
        return SX_ERROR;
    }
    if (libusb_control_transfer(pCam->handle,
                                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
                                SXUSB_CAMERA_MODEL,
                                0,
                                0,
                                cam_data,
                                2,
                                1000) < 0)
    {
          printf("Error reading camera model.\n");
    }
    pCam->model = cam_data[0];
    //printf("SX camera model: %02X\n", sx_cams[sx_cnt].model);
    return pCam->model;
}
/*
 * Reset camera.
 */
LONG sxReset(HANDLE sxHandle)
{
    struct sx_cam *pCam = sxHandle;
    return libusb_control_transfer(pCam->handle,
                                   LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                                   SXUSB_RESET,
                                   0,
                                   0,
                                   NULL,
                                   0,
                                   1000) < 0 ? SX_ERROR : SX_SUCCESS;
}
LONG sxClearPixels(HANDLE sxHandle, USHORT flags, USHORT camIndex)
{
    struct sx_cam *pCam = sxHandle;
    return libusb_control_transfer(pCam->handle,
                                   LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                                   SXUSB_CLEAR_PIXELS,
                                   flags,
                                   camIndex,
                                   NULL,
                                   0,
                                   1000) < 0 ? SX_ERROR : SX_SUCCESS;
}
/*
 * Short exposure.
 */
LONG sxExposePixelsGated(HANDLE sxHandle, USHORT flags, USHORT camIndex, USHORT xoffset, USHORT yoffset, USHORT width, USHORT height, USHORT xbin, USHORT ybin, ULONG msec)
{
    unsigned char cam_data[22];
    struct sx_cam *pCam = sxHandle;
    cam_data[USB_REQ_DATA + 0]  = xoffset;
    cam_data[USB_REQ_DATA + 1]  = xoffset >> 8;
    cam_data[USB_REQ_DATA + 2]  = yoffset;
    cam_data[USB_REQ_DATA + 3]  = yoffset >> 8;
    cam_data[USB_REQ_DATA + 4]  = width;
    cam_data[USB_REQ_DATA + 5]  = width >> 8;
    cam_data[USB_REQ_DATA + 6]  = height;
    cam_data[USB_REQ_DATA + 7]  = height >> 8;
    cam_data[USB_REQ_DATA + 8]  = xbin;
    cam_data[USB_REQ_DATA + 9]  = ybin;
    cam_data[USB_REQ_DATA + 10] = msec;
    cam_data[USB_REQ_DATA + 11] = msec >> 8;
    cam_data[USB_REQ_DATA + 12] = msec >> 16;
    cam_data[USB_REQ_DATA + 13] = msec >> 24;
    return libusb_control_transfer(pCam->handle,
                                   LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                                   SXUSB_READ_PIXELS_GATED,
                                   flags,
                                   camIndex,
                                   cam_data+USB_REQ_DATA,
                                   14,
                                   1000) < 0 ? SX_ERROR : SX_SUCCESS;
}
LONG sxExposePixels(HANDLE sxHandle, USHORT flags, USHORT camIndex, USHORT xoffset, USHORT yoffset, USHORT width, USHORT height, USHORT xbin, USHORT ybin, ULONG msec)
{
    unsigned char cam_data[22];
    struct sx_cam *pCam = sxHandle;
    cam_data[USB_REQ_DATA + 0]  = xoffset;
    cam_data[USB_REQ_DATA + 1]  = xoffset >> 8;
    cam_data[USB_REQ_DATA + 2]  = yoffset;
    cam_data[USB_REQ_DATA + 3]  = yoffset >> 8;
    cam_data[USB_REQ_DATA + 4]  = width;
    cam_data[USB_REQ_DATA + 5]  = width >> 8;
    cam_data[USB_REQ_DATA + 6]  = height;
    cam_data[USB_REQ_DATA + 7]  = height >> 8;
    cam_data[USB_REQ_DATA + 8]  = xbin;
    cam_data[USB_REQ_DATA + 9]  = ybin;
    cam_data[USB_REQ_DATA + 10] = msec;
    cam_data[USB_REQ_DATA + 11] = msec >> 8;
    cam_data[USB_REQ_DATA + 12] = msec >> 16;
    cam_data[USB_REQ_DATA + 13] = msec >> 24;
    return libusb_control_transfer(pCam->handle,
                                   LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                                   SXUSB_READ_PIXELS_DELAYED,
                                   flags,
                                   camIndex,
                                   cam_data+USB_REQ_DATA,
                                   14,
                                   1000) < 0 ? SX_ERROR : SX_SUCCESS;
}
/*
 * Read image.
 */
LONG sxLatchPixels(HANDLE sxHandle, USHORT flags, USHORT camIndex, USHORT xoffset, USHORT yoffset, USHORT width, USHORT height, USHORT xbin, USHORT ybin)
{
    unsigned char cam_data[18];
    struct sx_cam *pCam = sxHandle;
    //if (!pCam->opened)
    //    return SX_ERROR;
    //flags |= SXCCD_EXP_FLAGS_NOWIPE_FRAME|SXCCD_EXP_FLAGS_NOCLEAR_FRAME;
    cam_data[USB_REQ_DATA + 0]  = xoffset;
    cam_data[USB_REQ_DATA + 1]  = xoffset >> 8;
    cam_data[USB_REQ_DATA + 2]  = yoffset;
    cam_data[USB_REQ_DATA + 3]  = yoffset >> 8;
    cam_data[USB_REQ_DATA + 4]  = width;
    cam_data[USB_REQ_DATA + 5]  = width >> 8;
    cam_data[USB_REQ_DATA + 6]  = height;
    cam_data[USB_REQ_DATA + 7]  = height >> 8;
    cam_data[USB_REQ_DATA + 8]  = xbin;
    cam_data[USB_REQ_DATA + 9]  = ybin;
    return libusb_control_transfer(pCam->handle,
                                   LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                                   SXUSB_READ_PIXELS,
                                   flags,
                                   camIndex,
                                   cam_data+USB_REQ_DATA,
                                   10,
                                   1000) < 0 ? 0 : 1;
}
LONG sxReadPixels(HANDLE sxHandle, USHORT *pixels, ULONG count)
{
    int xfer = 0;
    struct sx_cam *pCam = sxHandle;
    libusb_bulk_transfer(pCam->handle,
                         pCam->rcv_endpoint,
                         (BYTE *)pixels,
                         count*2,
                         &xfer,
                         10000);
    return xfer;
}
LONG sxSetShutter(HANDLE sxHandle, USHORT state)
{
    struct sx_cam *pCam = sxHandle;
    return SX_SUCCESS;
}
ULONG sxSetTimer(HANDLE sxHandle, ULONG msec)
{
    struct sx_cam *pCam = sxHandle;
    return SX_SUCCESS;
}
ULONG sxGetTimer(HANDLE sxHandle)
{
    struct sx_cam *pCam = sxHandle;
    return SX_SUCCESS;
}
ULONG sxSetSTAR2000(HANDLE sxHandle, BYTE star2k)
{
    struct sx_cam *pCam = sxHandle;
    return SX_SUCCESS;
}
ULONG sxSetSerialPort(HANDLE sxHandle, USHORT portIndex, USHORT property, ULONG value)
{
    struct sx_cam *pCam = sxHandle;
    return SX_SUCCESS;
}
USHORT sxGetSerialPort(HANDLE sxHandle, USHORT portIndex, USHORT property)
{
    struct sx_cam *pCam = sxHandle;
    return SX_SUCCESS;
}
ULONG sxWriteSerialPort(HANDLE sxHandle, USHORT portIndex, USHORT flush, USHORT count, BYTE *data)
{
    struct sx_cam *pCam = sxHandle;
    return SX_SUCCESS;
}
ULONG sxReadSerialPort(HANDLE sxHandle, USHORT portIndex, USHORT count, BYTE *data)
{
    struct sx_cam *pCam = sxHandle;
    return SX_SUCCESS;
}
ULONG sxGetFirmwareVersion(HANDLE sxHandle)
{
    struct sx_cam *pCam = sxHandle;
    return SX_SUCCESS;
}
USHORT sxGetBuildNumber(HANDLE sxHandle)
{
    struct sx_cam *pCam = sxHandle;
    return SX_SUCCESS;
}
ULONG sxSetCooler(HANDLE sxHandle, UCHAR SetStatus, USHORT SetTemp, UCHAR *RetStatus, USHORT *RetTemp )
{
    struct sx_cam *pCam = sxHandle;
    return SX_SUCCESS;
}
ULONG sxGetCoolerTemp(HANDLE sxHandle, UCHAR *RetStatus, USHORT *RetTemp )
{
    struct sx_cam *pCam = sxHandle;
    return SX_SUCCESS;
}
ULONG sxGetDLLVersion()
{
    return 0x140;
}
