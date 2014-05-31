/*
 * Data-Router
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact:  Hocheol Seo <hocheol.seo@samsung.com>
 *               Injun Yang <injun.yang@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include <glib.h>
#include <sys/wait.h>

#include "dr-main.h"
#include "dr-usb.h"
#include "dr-modem.h"
#include "dr-common.h"



#define ACM_WAIT_TIME 300000

extern dr_info_t dr_info;
volatile gboolean dsr_status = FALSE;

static void __usb_status_noti_handler(void *data)
{
	int usb_state = -1;
	int usb_mode = -1;
	int ret;

	if (_get_usb_state(&usb_state) < 0) {
		ERR(" Unable to get usb status !\n");
		return;
	}

	ret = vconf_get_int(VCONFKEY_SETAPPL_USB_MODE_INT, &usb_mode);
	if (ret != 0) {
		DBG("Vconf get failed\n");
		return;
	}

	DBG("USB noti handler, USB state : %d, Mode : %d\n", usb_state, usb_mode);
	DBG("usb_fd = 0x%x\n", dr_info.usb.usb_fd);


	/*
	 * If USB driver builted in the Kernel, VCONFKEY_SYSMAN_USB_CONNECTED is not used.
	 * If USB driver loaded in DR/MTP then need to check VCONFKEY_SYSMAN_USB_CONNECTED.
	 * After usb driver loaded, MTP will be set vconf value as VCONFKEY_SYSMAN_USB_CONNECTED
	 */
	if (usb_state != VCONFKEY_SYSMAN_USB_DISCONNECTED &&
		usb_mode == SETTING_USB_DEFAULT_MODE) {
		_init_usb();
	} else if (usb_state == VCONFKEY_SYSMAN_USB_DISCONNECTED) {
		_deinit_usb();
		_deinit_dr();
	}
	return;
}

static void __usb_mode_noti_handler(void *data)
{
	int usb_state = -1;
	int usb_mode = -1;
	int ret;

	ret = vconf_get_int(VCONFKEY_SETAPPL_USB_MODE_INT, &usb_mode);
	if (ret != 0) {
		DBG("Vconf get failed\n");
		return;
	}

	DBG("USB Mode noti handler, USB  Mode : %d\n", usb_mode);
	DBG("usb_fd = 0x%x\n", dr_info.usb.usb_fd);

	if (usb_mode != SETTING_USB_DEFAULT_MODE) {
		_deinit_usb();
		_deinit_dr();
	}

	return;
}


gboolean _register_vconf_notification(void)
{
	int ret;

	ret =
	    vconf_notify_key_changed(VCONFKEY_SYSMAN_USB_STATUS, (vconf_callback_fn) __usb_status_noti_handler, NULL);
	if (ret < 0) {
		ERR("Error !!! VCONFKEY reg noti  : %s\n", VCONFKEY_SYSMAN_USB_STATUS);
	}

	ret =
	    vconf_notify_key_changed(VCONFKEY_SETAPPL_USB_MODE_INT,
				     (vconf_callback_fn) __usb_mode_noti_handler, NULL);
	if (ret < 0) {
		ERR("Error !!! VCONFKEY reg noti  : %s\n", VCONFKEY_SETAPPL_USB_MODE_INT);
	}

	return TRUE;
}

void _unregister_vconf_notification(void)
{
	vconf_ignore_key_changed(VCONFKEY_SYSMAN_USB_STATUS, (vconf_callback_fn) __usb_status_noti_handler);
	vconf_ignore_key_changed(VCONFKEY_SETAPPL_USB_MODE_INT, (vconf_callback_fn) __usb_mode_noti_handler);
	return;
}

gboolean _register_telephony_event(void)
{
	return TRUE;
}

void _unregister_telephony_event(void)
{
	return;
}

