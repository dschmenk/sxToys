#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include "sxccd.h"
/*
 * Vendor specific control commands.
 */
#define SX_USB_ECHO                 0
#define SX_USB_CLEAR_PIXELS         1
#define SX_USB_READ_PIXELS_DELAYED  2
#define SX_USB_READ_PIXELS          3
#define SX_USB_SET_TIMER            4
#define SX_USB_GET_TIMER            5
#define SX_USB_RESET                6
#define SX_USB_SET_CCD              7
#define SX_USB_GET_CCD              8
#define SX_USB_SET_GUIDE_PORT       9
#define SX_USB_WRITE_SERIAL_PORT    10
#define SX_USB_READ_SERIAL_PORT     11
#define SX_USB_SET_SERIAL           12
#define SX_USB_GET_SERIAL           13
#define SX_USB_CAMERA_MODEL         14
#define SX_USB_LOAD_EEPROM          15
/*
 * Caps bit definitions.
 */
#define SX_USB_CAPS_STAR2K          0x01
#define SX_USB_CAPS_COMPRESS        0x02
#define SX_USB_CAPS_EEPROM          0x04
#define SX_USB_CAPS_GUIDER          0x08
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
#define EZUSB_VENDOR_ID     0x0547
#define EZUSB_PRODUCT_ID    0x2131
#define EZUSB2_VENDOR_ID    0x04B4
#define EZUSB2_PRODUCT_ID   0x8613
#define SX_VENDOR_ID        0x1278
#define ECHO2_PRODUCT_ID    0x0100
#define ECHO3_PRODUCT_ID    0x0200
/*
 * Set and reset 8051 requests.
 */
#define EZUSB_CPUCS_REG     0x7F92
#define EZUSB2_CPUCS_REG    0xE600
#define CPUCS_RESET         0x01
#define CPUCS_RUN           0x00
/*
 * Address in 8051 external memory for debug info and CCD parameters.
 * Must use low address alias for EZ-USB
 */
#define SX_EZUSB_DEBUG_BUF      0x1C80
#define SX_EZUSB2_DEBUG_BUF     0xE1F0
/*
 * EZ-USB code download requests.
 */
#define EZUSB_FIRMWARE_LOAD 0xA0
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
#define MAX_CAMS 4
/*
 * SX CCD Camera structure.
 */
static struct sx_cam
{
    libusb_device_handle *handle;
    int snd_endpoint;
    int rcv_endpoint;
    unsigned int num;
    unsigned int model;
    unsigned int guider;
    unsigned int hfront_porch;
    unsigned int hback_porch;
    unsigned int vfront_porch;
    unsigned int vback_porch;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    unsigned int pix_width;
    unsigned int pix_height;
    unsigned int ser_ports;
    unsigned int caps;
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
        return 0;
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
            return 0;
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
        return 0;
    }
    //printf("sx_ezusb: un-RESET 8051\n");
    libusb_close(handle);
    return 1;
}
/*
 * Open all SX CCD cameras.
 */
int sxccd_open(int defmodel)
{
    int devc, renum, i;
    unsigned char cam_data[32];
    libusb_device *sx_dev[MAX_CAMS];
    libusb_device **devv;
    struct libusb_device_descriptor desc;
    struct libusb_config_descriptor *config;

    if (sx_cnt)
        while (sx_cnt--) libusb_close(sx_cams[sx_cnt].handle);
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
         * Look SX cameras.
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
                        SX_USB_RESET,
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
                        SX_USB_CAMERA_MODEL,
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
                if (cam_data[0] == 0 && cam_data[1] == 0)
                {
                    if (!defmodel)
                        defmodel = SXCCD_MX5;
                    //printf("Setting camera model to %02X\n", defmodel);
                    if (libusb_control_transfer(sx_cams[sx_cnt].handle,
                                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                                SX_USB_CAMERA_MODEL,
                                defmodel,
                                0,
                                NULL,
                                0,
                                1000) < 0)
                    {
                        printf("Error setting camera model.\n");
                    }
                    if (libusb_control_transfer(sx_cams[sx_cnt].handle,
                                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
                                SX_USB_CAMERA_MODEL,
                                0,
                                0,
                                cam_data,
                                2,
                                1000) < 0)
                    {
                          printf("Error reading camera model.\n");
                    }
                }
                sx_cams[sx_cnt].model = cam_data[0];
                //printf("SX camera model: %02X\n", sx_cams[sx_cnt].model);
                /*
                 * Get CCD parameters.
                 */
                if (libusb_control_transfer(sx_cams[sx_cnt].handle,
                                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
                                SX_USB_GET_CCD,
                                0,
                                0,
                                cam_data,
                                17,
                                1000) < 0)
                {
                    //printf("Could not get CCD parameters\n");
                    break;
                }
                sx_cams[sx_cnt].num          = 0;
                sx_cams[sx_cnt].guider       = 0;
                sx_cams[sx_cnt].hfront_porch = cam_data[0];
                sx_cams[sx_cnt].hback_porch  = cam_data[1];
                sx_cams[sx_cnt].vfront_porch = cam_data[4];
                sx_cams[sx_cnt].vback_porch  = cam_data[5];
                sx_cams[sx_cnt].width        = cam_data[2]  | (cam_data[3]  << 8);
                sx_cams[sx_cnt].height       = cam_data[6]  | (cam_data[7]  << 8);
                sx_cams[sx_cnt].depth        = cam_data[14];
                sx_cams[sx_cnt].pix_width    = cam_data[8]  | (cam_data[9]  << 8);
                sx_cams[sx_cnt].pix_height   = cam_data[10] | (cam_data[11] << 8);
                sx_cams[sx_cnt].ser_ports    = cam_data[15];
                sx_cams[sx_cnt].caps         = cam_data[16];
                //printf("SX camera width:%d height:%d depth:%d caps:%02X\n", sx_cams[sx_cnt].width, sx_cams[sx_cnt].height, sx_cams[sx_cnt].depth, sx_cams[sx_cnt].caps);
            }
            libusb_free_config_descriptor(config);
            sx_cnt++;
        }
    }
    libusb_free_device_list(devv, 1);
    return sx_cnt;
}
/*
 * Close all SX CCD cameras.
 */
void sxccd_close(void)
{
    while (sx_cnt--)
        libusb_close(sx_cams[sx_cnt].handle);
    libusb_exit(NULL);
}
/*
 * Get camera parameters.
 */
int sxccd_get_model(unsigned int cam_idx)
{
    if (cam_idx >= sx_cnt)
        return -1;
    return sx_cams[cam_idx].model;
}
int sxccd_get_frame_dimensions(unsigned int cam_idx, unsigned int *width, unsigned int *height, unsigned int *depth)
{
    if (cam_idx >= sx_cnt)
        return -1;
    *width  = sx_cams[cam_idx].width;
    *height = sx_cams[cam_idx].height;
    *depth  = sx_cams[cam_idx].depth;
    return 0;
}
int sxccd_get_pixel_dimensions(unsigned int cam_idx, unsigned int *pixwidth, unsigned int *pixheight)
{
    if (cam_idx >= sx_cnt)
        return -1;
    *pixwidth  = sx_cams[cam_idx].pix_width;
    *pixheight = sx_cams[cam_idx].pix_height;
    return 0;
}
int sxccd_get_caps(unsigned int cam_idx, unsigned int *caps, unsigned int *ports)
{
    if (cam_idx >= sx_cnt)
        return -1;
    *caps  = sx_cams[cam_idx].caps;
    *ports = sx_cams[cam_idx].ser_ports;
    return 0;
}
/*
 * Reset camera.
 */
int sxcd_reset(unsigned int cam_idx)
{
    if (cam_idx >= sx_cnt)
        return -1;
    return libusb_control_transfer(sx_cams[cam_idx].handle,
                   LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                   SX_USB_RESET,
                   0,
                   0,
                   NULL,
                   0,
                   1000);
}
int sxccd_clear_frame(unsigned int cam_idx, unsigned int options)
{
    if (cam_idx >= sx_cnt)
        return -1;
#if 0
    cam_data[USB_REQ_TYPE    ]  = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT;
    cam_data[USB_REQ         ]  = SX_USB_CLEAR_PIXELS;
    cam_data[USB_REQ_VALUE_L ]  = options;
    cam_data[USB_REQ_VALUE_H ]  = options >> 8;
    cam_data[USB_REQ_INDEX_L ]  = guider;
    cam_data[USB_REQ_INDEX_H ]  = 0;
    cam_data[USB_REQ_LENGTH_L]  = 0;
    cam_data[USB_REQ_LENGTH_H]  = 0;
    libusb_bulk_transfer(sx_cams[cam_idx].handle,
             sx_cams[cam_idx].snd_endpoint,
             cam_data,
             sizeof(cam_data),
             &xfer,
             10000);
    if (xfer != sizeof(cam_data))
#else
    if (libusb_control_transfer(sx_cams[cam_idx].handle,
                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                SX_USB_CLEAR_PIXELS,
                options,
                sx_cams[cam_idx].guider,
                NULL,
                0,
                1000) < 0)
#endif
    {
        //printf("Error writing READ_CLEAR_PIXELS\n");
    }
    return 0;
}
/*
 * Read image with short exposure.
 */
int sxccd_read_pixels_delayed(unsigned int cam_idx, unsigned int options, unsigned int xoffset, unsigned int yoffset, unsigned int width, unsigned int height, unsigned int xbin, unsigned int ybin, unsigned int msec, unsigned char *pixbuf)
{
    unsigned char cam_data[22];
    int xfer, fb_size;

    if (cam_idx >= sx_cnt)
        return -1;
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
#if 0
    cam_data[USB_REQ_TYPE    ]  = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT;
    cam_data[USB_REQ         ]  = SX_USB_READ_PIXELS_DELAYED;
    cam_data[USB_REQ_VALUE_L ]  = options;
    cam_data[USB_REQ_VALUE_H ]  = options >> 8;
    cam_data[USB_REQ_INDEX_L ]  = sx_cams[cam_idx].guider;
    cam_data[USB_REQ_INDEX_H ]  = 0;
    cam_data[USB_REQ_LENGTH_L]  = 14;
    cam_data[USB_REQ_LENGTH_H]  = 0;
    libusb_bulk_transfer(sx_cams[cam_idx].handle,
             sx_cams[cam_idx].snd_endpoint,
             cam_data,
             sizeof(cam_data),
             &xfer,
             10000);
    if (xfer != sizeof(cam_data))
#else
    if (libusb_control_transfer(sx_cams[cam_idx].handle,
                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                SX_USB_READ_PIXELS_DELAYED,
                options,
                sx_cams[cam_idx].guider,
                cam_data+USB_REQ_DATA,
                14,
                1000) < 0)
#endif
    {
        //printf("Error writing READ_PIXELS_DELAYED\n");
        return 0;
    }
    fb_size = FRAMEBUF_SIZE(width, height, sx_cams[cam_idx].depth, xbin, ybin);
    libusb_bulk_transfer(sx_cams[cam_idx].handle,
             sx_cams[cam_idx].rcv_endpoint,
             pixbuf,
             fb_size,
             &xfer,
             10000);
    if (xfer != fb_size)
    {
        //printf("Error reading %d of %d pixel bytes\n", xfer, fb_size);
    }
    return (xfer);
}
/*
 * Read image.
 */
int sxccd_read_pixels(unsigned int cam_idx, unsigned int options, unsigned int xoffset, unsigned int yoffset, unsigned int width, unsigned int height, unsigned int xbin, unsigned int ybin, unsigned char *pixbuf)
{
    unsigned char cam_data[22];
    int xfer, fb_size, err;

    if (cam_idx >= sx_cnt)
        return -1;
    options |= SXCCD_EXP_FLAGS_NOWIPE_FRAME|SXCCD_EXP_FLAGS_NOCLEAR_FRAME;
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
    cam_data[USB_REQ_DATA + 10] = 1;
    cam_data[USB_REQ_DATA + 11] = 0;
    cam_data[USB_REQ_DATA + 12] = 0;
    cam_data[USB_REQ_DATA + 13] = 0;
#if 0
    cam_data[USB_REQ_TYPE    ]  = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT;
    cam_data[USB_REQ         ]  = SX_USB_READ_PIXELS_DELAYED;
    cam_data[USB_REQ_VALUE_L ]  = options;
    cam_data[USB_REQ_VALUE_H ]  = options >> 8;
    cam_data[USB_REQ_INDEX_L ]  = sx_cams[cam_idx].guider;
    cam_data[USB_REQ_INDEX_H ]  = 0;
    cam_data[USB_REQ_LENGTH_L]  = 14;
    cam_data[USB_REQ_LENGTH_H]  = 0;
    libusb_bulk_transfer(sx_cams[cam_idx].handle,
             sx_cams[cam_idx].snd_endpoint,
             cam_data,
             sizeof(cam_data),
             &xfer,
             10000);
    if (xfer != sizeof(cam_data))
#else
    if ((err=libusb_control_transfer(sx_cams[cam_idx].handle,
                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                SX_USB_READ_PIXELS_DELAYED,
                options,
                sx_cams[cam_idx].guider,
                cam_data+USB_REQ_DATA,
                14,
                1000)) < 0)
#endif
    {
        //printf("Error %d writing READ_PIXELS\n", err);
        return 0;
    }
    fb_size = FRAMEBUF_SIZE(width, height, sx_cams[cam_idx].depth, xbin, ybin);
    libusb_bulk_transfer(sx_cams[cam_idx].handle,
             sx_cams[cam_idx].rcv_endpoint,
             pixbuf,
             fb_size,
             &xfer,
             10000);
    if (xfer != fb_size)
    {
        //printf("Error reading %d of %d pixel bytes\n", xfer, fb_size);
    }
    return (xfer);
}
