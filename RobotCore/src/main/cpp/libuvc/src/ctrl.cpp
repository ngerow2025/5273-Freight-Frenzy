/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (C) 2010-2012 Ken Tossell
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the author nor other contributors may be
*     used to endorse or promote products derived from this software
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/
/**
 * @defgroup ctrl Video capture and processing controls
 * @brief Functions for manipulating device settings and stream parameters
 *
 * The `uvc_get_*` and `uvc_set_*` functions are used to read and write the settings associated
 * with the device's input, processing and output units.
 */

#include "libuvc.h"
#include "libuvc/libuvc_internal.h"

#define CTRL_TIMEOUT_MILLIS 0


/***** GENERIC CONTROLS *****/
/**
 * @brief Get the length of a control on a terminal or unit.
 * 
 * @param devh UVC device handle
 * @param unit Unit or Terminal ID; obtain this from the uvc_extension_unit_t describing the extension unit
 * @param ctrl Vendor-specific control number to query
 * @return On success, the length of the control as reported by the device. Otherwise,
 *   a uvc_error error describing the error encountered.
 * @ingroup ctrl
 */
int uvc_get_ctrl_len(uvc_device_handle_t *devh, uint8_t unit, uint8_t ctrl) {
  unsigned char buf[2];

  int ret = (uvc_error)libusb_control_transfer(
    devh->usb_devh,
    REQ_TYPE_GET, UVC_GET_LEN,
    ctrl << 8,
    unit << 8 | devh->info->ctrl_if.bInterfaceNumber,		// XXX saki
    buf,
    2,
    CTRL_TIMEOUT_MILLIS);

  if (ret < 0)
    return ret;
  else
    return (unsigned short)SW_TO_SHORT(buf);
}

/**
 * @brief Perform a GET_* request from an extension unit.
 * 
 * @param devh UVC device handle
 * @param unit Unit ID; obtain this from the uvc_extension_unit_t describing the extension unit
 * @param ctrl Control number to query
 * @param data Data buffer to be filled by the device
 * @param len Size of data buffer
 * @param req_code GET_* request to execute
 * @return On success, the number of bytes actually transferred. Otherwise,
 *   a uvc_error error describing the error encountered.
 * @ingroup ctrl
 */
int uvc_get_ctrl(uvc_device_handle_t *devh, uint8_t unit, uint8_t ctrl, void *data, int len, enum uvc_req_code req_code) {
  return libusb_control_transfer(
    devh->usb_devh,
    REQ_TYPE_GET, req_code,
    ctrl << 8,
    unit << 8 | devh->info->ctrl_if.bInterfaceNumber,		// XXX saki
    (unsigned char *)data,
    len,
    CTRL_TIMEOUT_MILLIS);
}

/**
 * @brief Perform a SET_CUR request to a terminal or unit.
 * 
 * @param devh UVC device handle
 * @param unit Unit or Terminal ID
 * @param ctrl Control number to set
 * @param data Data buffer to be sent to the device
 * @param len Size of data buffer
 * @return On success, the number of bytes actually transferred. Otherwise,
 *   a uvc_error error describing the error encountered.
 * @ingroup ctrl
 */
int uvc_set_ctrl(uvc_device_handle_t *devh, uint8_t unit, uint8_t ctrl, void *data, int len) {
  return libusb_control_transfer(
    devh->usb_devh,
    REQ_TYPE_SET, UVC_SET_CUR,
    ctrl << 8,
    unit << 8 | devh->info->ctrl_if.bInterfaceNumber,		// XXX saki
    (unsigned char *)data,
    len,
    CTRL_TIMEOUT_MILLIS);
}

/***** INTERFACE CONTROLS *****/
/** VC Request Error Code Control (UVC 4.2.1.2) */ // XXX added saki
uvc_error uvc_vc_get_error_code(uvc_device_handle_t *devh, uvc_vc_error_code_control *error_code, enum uvc_req_code req_code) {
	uint8_t error_char = 0;
	uvc_error ret = UVC_SUCCESS;

	ret = (uvc_error)libusb_control_transfer(
            devh->usb_devh,
            REQ_TYPE_GET, req_code,
			UVC_VC_REQUEST_ERROR_CODE_CONTROL << 8,
			devh->info->ctrl_if.bInterfaceNumber,	// XXX saki
			&error_char,
            sizeof(error_char),
            CTRL_TIMEOUT_MILLIS);

	if (ret == 1) {
		*error_code = (uvc_vc_error_code_control)error_char;
		return UVC_SUCCESS;
	} else {
		return ret;
	}
}

/** VS Request Error Code Control */ // XXX added saki
uvc_error uvc_vs_get_error_code(uvc_device_handle_t *devh, uvc_vs_error_code_control *error_code, enum uvc_req_code req_code) {
	uint8_t error_char = 0;
	uvc_error ret = UVC_SUCCESS;

#if 0 // This code may cause hang-up on some combinations of device and camera and temporary disabled.
	ret = (uvc_error)libusb_control_transfer(devh->usb_devh, REQ_TYPE_GET, req_code,
			UVC_VS_STREAM_ERROR_CODE_CONTROL << 8,
			devh->info->stream_ifs->bInterfaceNumber,	// XXX is this OK?
			&error_char, sizeof(error_char), CTRL_TIMEOUT_MILLIS);

	if (LIKELY(ret == 1)) {
		*error_code = error_char;
		return UVC_SUCCESS;
	} else {
		return ret;
	}
#else
    ret = UVC_ERROR_NOT_SUPPORTED;
	return ret;
#endif
}

uvc_error uvc_get_power_mode(uvc_device_handle_t *devh, enum uvc_device_power_mode *mode, enum uvc_req_code req_code) {
  uint8_t mode_char;
  uvc_error ret;

  ret = uvc_error(libusb_control_transfer(
    devh->usb_devh,
    REQ_TYPE_GET, req_code,
    UVC_VC_VIDEO_POWER_MODE_CONTROL << 8,
    devh->info->ctrl_if.bInterfaceNumber,	// XXX saki
    &mode_char,
    sizeof(mode_char),
    CTRL_TIMEOUT_MILLIS));

  if (ret == 1) {
    *mode = (uvc_device_power_mode)mode_char;
    return UVC_SUCCESS;
  } else {
    return ret;
  }
}

uvc_error uvc_set_power_mode(uvc_device_handle_t *devh, enum uvc_device_power_mode mode) {
  uint8_t mode_char = mode;
  uvc_error ret;

  ret = uvc_error(libusb_control_transfer(
    devh->usb_devh,
    REQ_TYPE_SET, UVC_SET_CUR,
    UVC_VC_VIDEO_POWER_MODE_CONTROL << 8,
    devh->info->ctrl_if.bInterfaceNumber,	// XXX saki
    &mode_char,
    sizeof(mode_char),
    CTRL_TIMEOUT_MILLIS));

  if (ret == 1)
    return UVC_SUCCESS;
  else
    return ret;
}

/** @todo Request Error Code Control (UVC 1.5, 4.2.1.2) */
