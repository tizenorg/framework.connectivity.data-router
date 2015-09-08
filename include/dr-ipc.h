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



#ifndef _DR_IPC_H
#define  _DR_IPC_H

#include <glib.h>

/* This enum value should be matching with  dr_serial_event_t */
enum {
	SERIAL_CLOSED,
	SERIAL_OPENED,
};

gboolean _init_dbus_signal(void);
void _send_serial_status_signal(int event);

void _init_serial_server(void);
gboolean _deinit_serial_server(void);
gboolean _is_exist_serial_session(void);
gboolean _wait_serial_session(void);
int _write_to_serial_client(char *buf, int buf_len);
#endif
