/*
* serial-client
*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
* PROPRIETARY/CONFIDENTIAL
*
* Contact: Hocheol Seo <hocheol.seo@samsung.com>,
*          Injun Yang <injun.yang@samsung.com>,
*          Seungyoun Ju <sy39.ju@samsung.com>
*
* This software is the confidential and proprietary information of
* SAMSUNG ELECTRONICS ("Confidential Information"). You agree and acknowledge
* that this software is owned by Samsung and you shall not disclose
* such Confidential Information and shall use it only in accordance with
* the terms of the license agreement you entered into with SAMSUNG ELECTRONICS.
* SAMSUNG make no representations or warranties about the suitability of
* the software, either express or implied, including but not limited to
* the implied warranties of merchantability, fitness for a particular purpose,
* or non-infringement. SAMSUNG shall not be liable for any damages suffered by
* licensee arising out of or related to this software.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <glib.h>
#include <vconf.h>
#include <serial.h>

#include "serial_client.h"

//#define OSP_TEST

serial_h serial = NULL;
static GMainLoop	*g_mainloop;

static void __state_cb(serial_error_e result, serial_state_e state, void *user_data)
{
	switch (state) {
	case SERIAL_STATE_OPENED:
		DBG("SERIAL_OPENED : %d\n", result);
		serial_write(serial, "Connected", 9);
		break;

	case SERIAL_STATE_CLOSED:
		DBG("SERIAL_CLOSED : %d\n", result);

		serial_unset_state_changed_cb(serial);
		serial_unset_data_received_cb(serial);
		serial_destroy(serial);

#ifndef OSP_TEST
		g_main_loop_quit(g_mainloop);
		g_main_loop_unref(g_mainloop);
#endif
		break;

	default:
		ERR("Invalid state : %d", state);
		break;
	}

}

static bool __data_cb(const char *data, int data_length, void *user_data)
{
	DBG("Received : %s\n", data);

	if (strncmp(data, "close", 5) == 0) {
		serial_write(serial, "Closed", 6);
		serial_close(serial);
	} else {
		DBG("written size: %d\n", serial_write(serial, "OK", 2));
	}

	return true;
}

static void __serial_init(void)
{
	int ret;

	ret = serial_create(&serial);
	if (ret != SERIAL_ERROR_NONE)
		ERR("Serial_create failed\n");

	ret = serial_set_state_changed_cb(serial, __state_cb, NULL);
	if (ret != SERIAL_ERROR_NONE)
		ERR("Serial_set_status_cb failed");

	ret = serial_set_data_received_cb(serial, __data_cb, NULL);
	if (ret != SERIAL_ERROR_NONE)
		ERR("Serial_set_data_cb failed");

	DBG("Request serial open");
	ret = serial_open(serial);
	if (ret != SERIAL_ERROR_NONE)
		ERR("Serial_open failed\n");
}

#ifdef OSP_TEST
static void __vconf_key_handler(void *data)
{
	int val;
	vconf_get_int(VCONFKEY_DR_OSP_SERIAL_OPEN_INT, &val);
	DBG("Key changed : %d\n", val);

	if (val == 1) {
		__serial_init();
	}
}
#endif

int main(int argc, char *argv[])
{
	DBG("Start Serial-client");

#ifndef OSP_TEST
	__serial_init();
#else
	/* OSP is always daemon */
	/* To avoide getting signal from OSP, use vconf key */
	vconf_notify_key_changed(VCONFKEY_DR_OSP_SERIAL_OPEN_INT,
				(vconf_callback_fn) __vconf_key_handler, NULL);
#endif

	g_mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(g_mainloop);

	DBG("Terminated");

	return 0;
}

