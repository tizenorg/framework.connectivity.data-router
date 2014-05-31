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
#include <dbus/dbus-glib.h>
#include <dirent.h>
#include <dbus/dbus.h>


#include "dr-modem.h"
#include "dr-main.h"
#include "dr-usb.h"
#include "dr-noti-handler.h"
#include "dr-common.h"
#include "dr-parser.h"
#include "dr-ipc.h"


dr_info_t dr_info;
GMainLoop *mainloop;


static void __signal_handler(int signo)
{
	DBG("+\n");
	DBG("SIGNAL Number [%d]  !!! \n", signo);
	_send_dtr_ctrl_signal(DTR_OFF);
	DBG("-\n");
	exit(0);
}

gboolean _init_dr(void)
{
	DBG("+\n");
	int usb_state = -1;

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
	DBG("+\n");

	_unregister_vconf_notification();
	_unregister_telephony_event();

	DBG("-\n");

	g_main_loop_quit(mainloop);

	return TRUE;
}


int main(int argc, char *argv[])
{
	DBG("+\n");

	g_type_init();
	_init_dbus_signal();


	if (!g_thread_supported())
		g_thread_init(NULL);

	dbus_g_thread_init();

	_register_vconf_notification();
	_register_telephony_event();

	_init_dr();

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	DBG("-\n");
	return 0;
}
