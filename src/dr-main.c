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

#include <fcntl.h>
#include <dirent.h>
#include <dbus/dbus.h>
#include <glib-object.h>

#include "dr-modem.h"
#include "dr-main.h"
#include "dr-usb.h"
#include "dr-noti-handler.h"
#include "dr-common.h"
#include "dr-parser.h"
#include "dr-ipc.h"

#define DATA_ROUTER_BUS		"com.samsung.data.router"

dr_info_t dr_info;
GMainLoop *mainloop;


static void __signal_handler(int signo)
{
	ERR("SIGNAL Number [%d]  !!! ", signo);
	_send_dtr_ctrl_signal(DTR_OFF);

	exit(0);
}


static gboolean __dbus_request_name(void)
{
	int ret_code = 0;
	DBusConnection *conn;
	DBusError err;

	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

	if (dbus_error_is_set(&err))
		goto failed;

	ret_code = dbus_bus_request_name(conn,
					DATA_ROUTER_BUS,
					DBUS_NAME_FLAG_DO_NOT_QUEUE,
					&err);
	if (dbus_error_is_set(&err))
		goto failed;

	if(DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER == ret_code) {
		dbus_connection_unref(conn);
		return TRUE;
	}

failed:
	if (dbus_error_is_set(&err)) {
		ERR("D-Bus Error: %s\n", err.message);
		dbus_error_free(&err);
	}

	if (!conn)
		dbus_connection_unref(conn);


	return FALSE;
}


gboolean _init_dr(void)
{
	int usb_state = -1;

	INFO("Initialize data-router");

	signal(SIGINT, __signal_handler);
	signal(SIGQUIT, __signal_handler);
	signal(SIGABRT, __signal_handler);
	signal(SIGSEGV, __signal_handler);
	signal(SIGTERM, __signal_handler);
	signal(SIGPIPE, SIG_IGN);

	memset(&dr_info, 0, sizeof(dr_info));

	/****************   USB    ******************/
	if (_get_usb_state(&usb_state) != -1) {
		if (usb_state != VCONFKEY_SYSMAN_USB_DISCONNECTED ) {
			_init_usb();
		}
	}

	DBG("-\n");

	return TRUE;
}

gboolean _deinit_dr(void)
{
	INFO("Deinitialize data-router");

	_unregister_vconf_notification();
	_unregister_telephony_event();

	DBG("-\n");

	g_main_loop_quit(mainloop);

	return TRUE;
}


int main(int argc, char *argv[])
{
	INFO("<<<<<  Starting data-router daemon >>>>>");

	if (__dbus_request_name() == FALSE) {
		ERR("Aleady dbus instance existed\n");
		exit(0);
	}

	/* We must guarantee dbus thread safety */
	_init_dbus_signal();

	_register_vconf_notification();
	_register_telephony_event();

	_init_dr();

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	INFO("<<<<< Exit data-router >>>>>");
	return 0;
}
