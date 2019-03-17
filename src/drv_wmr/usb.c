// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <libusb.h>
#include <memory.h>
#include "wmr.h"

static int open_usb_device(libusb_context* ctx, libusb_device_handle** dev, int vendor_id, int product_id, int idx)
{
    libusb_device** dev_list;
    int res;
    ssize_t cnt;

    cnt = libusb_get_device_list(ctx, &dev_list);
    if(cnt < 0)
    {
        res = (int)cnt;
        goto err_0;
    }

    int wmr_dev_idx = 0;
    ssize_t dev_idx = -1;
    for(ssize_t i = 0; i < cnt; ++i)
    {
        struct libusb_device_descriptor desc;

        res = libusb_get_device_descriptor(dev_list[i], &desc);
        if(res < 0)
        {
            continue;
        }

        if(desc.idVendor == vendor_id && desc.idProduct == product_id)
        {
            if(wmr_dev_idx == idx)
            {
                dev_idx = i;
                break;
            }

            wmr_dev_idx++;
        }
    }

    if(dev_idx < 0)
    {
        res = LIBUSB_ERROR_NOT_FOUND;
        goto err_1;
    }

    res = libusb_open(dev_list[dev_idx], dev);
    if(res < 0)
    {
        goto err_1;
    }

err_1:
    libusb_free_device_list(dev_list, 0);
err_0:
    return res;
}

static bool wmr_camera_start(wmr_usb* usb)
{
    int res;
    int transferred;
    uint8_t start_cmd[sizeof(hololens_camera_start)];

    memcpy(start_cmd, hololens_camera_start, sizeof(hololens_camera_start));

    res = libusb_bulk_transfer(usb->dev, 0x05, start_cmd, sizeof(start_cmd), &transferred, 0);
    if(res < 0)
        goto err;

    if(transferred != sizeof(start_cmd))
    {
        res = LIBUSB_ERROR_IO;
        goto err;
    }

    res = libusb_submit_transfer(usb->img_xfer);
    if(res < 0)
        goto err;

err:
    if(res < 0)
    {
        LOGE("usb error: %s\n", libusb_error_name(res));
    }

    return res >= 0;
}

static bool wmr_camera_stop(wmr_usb* usb)
{
    int res;
    int transferred;
    uint8_t stop_cmd[sizeof(hololens_camera_stop)];

    memcpy(stop_cmd, hololens_camera_stop, sizeof(hololens_camera_stop));

    libusb_cancel_transfer(usb->img_xfer);

    res = libusb_bulk_transfer(usb->dev, 0x05, stop_cmd, sizeof(stop_cmd), &transferred, 0);
    if(res < 0)
        goto err;

    if(transferred != sizeof(stop_cmd))
    {
        res = LIBUSB_ERROR_IO;
        goto err;
    }

err:
    if(res < 0)
    {
        LOGE("usb error: %s\n", libusb_error_name(res));
    }

    return res >= 0;
}

static void wmr_camera_frame_cb(wmr_usb* usb)
{
    uint16_t type = usb->img_frame[6] << 8 | usb->img_frame[7];

    if(type == 300) // long exposure
    {
        // do something with the image

#if 0
        char fname[256];
        snprintf(fname, sizeof(fname), "img_%u.data", time(NULL));
        FILE* f = fopen(fname, "wb");
        fwrite(usb->img_frame, sizeof(usb->img_frame), 1u, f);
        fclose(f);
#endif
    }
}

static void LIBUSB_CALL img_xfer_cb(struct libusb_transfer* xfer)
{
    wmr_usb* usb = (wmr_usb*)xfer->user_data;

    if(xfer->status == LIBUSB_TRANSFER_COMPLETED &&
        xfer->actual_length == xfer->length)
    {
        size_t packet_pos = 0u;
        size_t img_pos = 0u;

        while(img_pos < sizeof(usb->img_frame) && packet_pos < xfer->actual_length)
        {
            packet_pos += 32;

            size_t bytes = sizeof(usb->img_frame) - img_pos;
            if(bytes > 24544)
            {
                bytes = 24544;
            }

            memcpy(&usb->img_frame[img_pos], &xfer->buffer[packet_pos], bytes);

            img_pos += bytes;
            packet_pos += bytes;
        }

        wmr_camera_frame_cb(usb);
    }

    libusb_submit_transfer(xfer);
}

bool wmr_usb_init(wmr_usb* usb, int idx)
{
    int res;

    // Initialize libusb
    res = libusb_init(&usb->ctx);
    if(res < 0)
        goto err_0;

    // Open the USB device
    res = open_usb_device(usb->ctx, &usb->dev, MICROSOFT_VID, HOLOLENS_SENSORS_PID, idx);
    if(res < 0)
        goto err_1;

    // Claim interface 3 for the cameras
    res = libusb_claim_interface(usb->dev, 3);
    if(res < 0)
        goto err_2;

    // Allocate the camera frame transfer
    usb->img_xfer = libusb_alloc_transfer(0);
    if(!usb->img_xfer)
    {
        res = LIBUSB_ERROR_NO_MEM;
        goto err_2;
    }

    libusb_fill_bulk_transfer(usb->img_xfer, usb->dev, 0x85, usb->img_buffer, sizeof(usb->img_buffer), img_xfer_cb, usb, 0);

    wmr_camera_start(usb);

    return true;
err_2:
    libusb_close(usb->dev);
err_1:
    libusb_exit(usb->ctx);
err_0:
    LOGE("usb error: %s\n", libusb_error_name(res));
    return false;
}

void wmr_usb_destroy(wmr_usb* usb)
{
    wmr_camera_stop(usb);

    libusb_close(usb->dev);
    libusb_exit(usb->ctx);
}

void wmr_usb_update(wmr_usb* usb)
{
    libusb_handle_events(usb->ctx);
}
