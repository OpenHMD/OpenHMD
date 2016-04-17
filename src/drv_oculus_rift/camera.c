#include "libuvc/libuvc.h"
#include "rift.h"

static uvc_context_t *ctx = 0;
static uvc_device_t *dev = 0;
static uvc_device_handle_t *devh = 0;
static uvc_stream_ctrl_t ctrl;

uvc_error_t oc_uvc_ensure_frame_size(uvc_frame_t *frame, size_t need_bytes)
{
	if (frame->library_owns_data) {
		if (!frame->data || frame->data_bytes != need_bytes) {
			frame->data_bytes = need_bytes;
			frame->data = realloc(frame->data, frame->data_bytes);
		}
		if (!frame->data) {
			return UVC_ERROR_NO_MEM;
		}
		return UVC_SUCCESS;
	} else {
		if (!frame->data || frame->data_bytes < need_bytes) {
			return UVC_ERROR_NO_MEM;
		}
		return UVC_SUCCESS;
	}
}


uvc_error_t oc_uvc_dk2_to_rgb(uvc_frame_t *in, uvc_frame_t *out)
{
	if (in->frame_format != UVC_FRAME_FORMAT_YUYV) {
		return UVC_ERROR_INVALID_PARAM;
	}

	if (oc_uvc_ensure_frame_size(out, in->width * 2 * in->height * 3) < 0) {
		return UVC_ERROR_NO_MEM;
	}

	out->width = in->width * 2;
	out->height = in->height;
	out->frame_format = UVC_FRAME_FORMAT_RGB;
	out->step = in->width * 2 * 3;
	out->sequence = in->sequence;
	out->capture_time = in->capture_time;
	out->source = in->source;


	uint8_t *pyuv = (uint8_t*)in->data;
	uint8_t *prgb = (uint8_t*)out->data;
	uint8_t *prgb_end = prgb + out->data_bytes;

	while (prgb < prgb_end) {
		prgb[0] = prgb[1] = prgb[2] = pyuv[0];
		prgb += 3;
		pyuv += 1;
	}

	return UVC_SUCCESS;
}

/* This callback function runs once per frame. Use it to perform any
* quick processing you need, or have it put the frame into your application's
* input queue. If this function takes too long, you'll start losing frames. */
void oc_callback(uvc_frame_t *frame, void *ptr) 
{
	uvc_frame_t *bgr;
	uvc_error_t ret;

	/* We'll convert the image from YUV/JPEG to BGR, so allocate space */
	bgr = uvc_allocate_frame(frame->width*2 * frame->height*3);
	if (!bgr) {
		return;
	}

	/* Do the BGR conversion */
	ret = oc_uvc_dk2_to_rgb(frame, bgr);
	if (ret) {
		uvc_free_frame(bgr);
		return;
	}
	uvc_free_frame(bgr);
}

int oc_init()
{
	uvc_error_t res;
	/* Initialize a UVC service context. Libuvc will set up its own libusb
	* context. Replace NULL with a libusb_context pointer to run libuvc
	* from an existing libusb context. */
	res = uvc_init(&ctx, NULL);
	if (res < 0) 
	{
		return res;
	}

	/* Locates the first attached UVC device, stores in dev */
	///TODO: CV1 implementation
	res = uvc_find_device(ctx, &dev, 0x2833, 0x0201, NULL); /* filter devices: vendor_id, product_id, "serial_num", look for DK2 camera */
	if (res < 0)
		return -1;
	else 
	{
		/* Try to open the device: requires exclusive access */
		res = uvc_open(dev, &devh); //realsense fork
		if (res < 0)
			return -1;
		else
		{
			res = uvc_get_stream_ctrl_format_size(
				devh, &ctrl, /* result stored in ctrl */
				UVC_FRAME_FORMAT_YUYV, /* YUV 422, aka YUV 4:2:2. try _COMPRESSED */
				376, 480, 60 /* width, height, fps */
			);
		}

		if (res < 0)
			return -1;
		else 
		{
			/* Start the video stream. The library will call user function cb:*/
			res = uvc_start_streaming(devh, &ctrl, oc_callback, 0, 0);
			if (res < 0) 
				return -1;
		}
	}
}

int oc_exit()
{
	/* End the stream. Blocks until last callback is serviced */
	uvc_stop_streaming(devh);
	/* Release our handle on the device */
	uvc_close(devh);	
	/* Release the device descriptor */
	uvc_unref_device(dev);
	/* Close the UVC context. This closes and cleans up any existing device handles,
	* and it closes the libusb context if one was not provided. */
	uvc_exit(ctx);
}