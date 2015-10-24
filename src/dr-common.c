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
#include <gio/gio.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "dr-main.h"
#include "dr-common.h"
#include "dr-util.h"

#define DEVICED_BUS_NAME          "org.tizen.system.deviced"
#define DEVICED_OBJECT_PATH       "/Org/Tizen/System/DeviceD/Power"
#define DEVICED_INTERFACE    "org.tizen.system.deviced.power"

extern GDBusConnection *dbus_connection;

static void __request_reboot_cb(GDBusConnection *conn,
				GAsyncResult *res, gpointer user_data)
{
	DBG("reboot");
}

int _request_reboot(char *parameter)
{
	GError *error = NULL;

	DBG("+");

	if (parameter == NULL)
		return -1;

	if (dbus_connection == NULL)
		dbus_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);

	if (!dbus_connection) {
		if (error) {
			ERR("Unable to connect to gdbus: %s", error->message);
			g_clear_error(&error);
		}
		return -1;
	}

	g_dbus_connection_call(dbus_connection, DEVICED_BUS_NAME,
			DEVICED_OBJECT_PATH,
			DEVICED_INTERFACE,
			"reboot",
			 g_variant_new("(si)", parameter, 0), NULL,
			G_DBUS_CALL_FLAGS_NONE, -1, NULL,
			(GAsyncReadyCallback)__request_reboot_cb, NULL);
	DBG("-");

	return 0;
}

int _get_usb_state(int *usb_state)
{
	if (-1 == vconf_get_int(VCONFKEY_SYSMAN_USB_STATUS, (void *)usb_state)) {
		ERR("Vconf get failed\n");
		return -1;
	}
	DBG("USB state : %d\n", *usb_state);
	return 0;
}

