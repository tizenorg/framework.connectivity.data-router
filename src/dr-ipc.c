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



#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib-bindings.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "dr-common.h"
#include "dr-usb.h"
#include "dr-main.h"
#include "dr-ipc.h"

#define SOCK_WAIT_TIME 10000
#define SOCK_WAIT_CNT 200
#define COM_SOCKET_PATH					"/tmp/.dr_common_stream"
#define BUF_SIZE		65536
#define NETWORK_SERIAL_INTERFACE		"Capi.Network.Serial"

DBusConnection *dbus_connection = NULL;

typedef enum {
	SERIAL_SESSION_DISCONNECTED,
	SERIAL_SESSION_CONNECTED
}dr_session_state_t;

typedef struct {
	int server_socket;
	int client_socket;
	int g_watch_id_server;
	int g_watch_id_client;
	GIOChannel *g_io;
	unsigned char state;
}dr_socket_info_t;

dr_socket_info_t serial_session = {0, };


static DBusHandlerResult __dbus_event_filter(DBusConnection *sys_conn,
							DBusMessage *msg, void *data)
{
	const char *path = dbus_message_get_path(msg);

	if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_SIGNAL)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	if (path == NULL || strcmp(path, "/") == 0)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	if (dbus_message_is_signal(msg, NETWORK_SERIAL_INTERFACE,
						"ready_for_serial")) {
		char *res = NULL;
		dbus_message_get_args(msg, NULL,
					DBUS_TYPE_STRING, &res,
					DBUS_TYPE_INVALID);

		if (g_strcmp0(res, "OK") == 0)
			_send_serial_status_signal(SERIAL_OPENED);
	} else {
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

gboolean _init_dbus_signal(void)
{
	DBG("+\n");
	DBusGConnection *conn;
	GError *err = NULL;
	DBusError dbus_error;

	conn = dbus_g_bus_get(DBUS_BUS_SYSTEM, &err);
	if(!conn) {
		ERR(" DBUS get failed\n");
		g_error_free(err);
		return FALSE;
	}
	dbus_connection = dbus_g_connection_get_connection(conn);

	/* Add the filter for network client functions */
	dbus_error_init(&dbus_error);
	dbus_connection_add_filter(dbus_connection, __dbus_event_filter, NULL, NULL);
	dbus_bus_add_match(dbus_connection,
			   "type=signal,interface=" NETWORK_SERIAL_INTERFACE
			   ",member=ready_for_serial", &dbus_error);
	if (dbus_error_is_set(&dbus_error)) {
		ERR("Fail to add dbus filter signal\n");
		dbus_error_free(&dbus_error);
	}

	DBG("-\n");
	return TRUE;
}

void _send_serial_status_signal(int event)
{
	DBusMessage *msg = NULL;
	if(dbus_connection == NULL) return;

	msg = dbus_message_new_signal("/DataRouter",
					  "User.Data.Router.Introspectable",
					  "serial_status");
	if (!msg) {
		ERR("Unable to allocate D-Bus signal\n");
		return;
	}

	if (!dbus_message_append_args(msg,
			DBUS_TYPE_INT32, &event,
			DBUS_TYPE_INVALID)) {
		ERR("Event sending failed\n");
		dbus_message_unref(msg);
		return;
	}
	DBG("Send dbus signal : %s\n", event ? "SERIAL_OPENED":"SERIAL_CLOSED");
	dbus_connection_send(dbus_connection, msg, NULL);
	dbus_message_unref(msg);
	return;
}


int _write_to_serial_client(char *buf, int buf_len)
{
	int len;
	if (buf == NULL || buf_len == 0) {
		ERR("Invalid param\n");
		return -1;
	}
	len = send(serial_session.client_socket, buf, buf_len, MSG_EOR);
	if (len == -1) {
		char err_buf[ERRMSG_SIZE] = { 0, };
		strerror_r(errno, err_buf, ERRMSG_SIZE);
		ERR("Send failed\n");
		return -1;
	}

	return len;
}


static void __close_client_socket()
{
	int ret;
	DBG("Closing socket\n");
	ret = close(serial_session.client_socket);
	if (ret == -1) {
		perror("close error: ");
	}
	serial_session.state = SERIAL_SESSION_DISCONNECTED;
	return;
}


static gboolean __g_io_server_handler(GIOChannel *io, GIOCondition cond, void *data)
{
	char buffer[BUF_SIZE+1];
	int len = 0;
	int fd;
	fd = g_io_channel_unix_get_fd(io);
	memset(buffer, 0, sizeof(buffer));
	len = recv(fd, buffer, BUF_SIZE, 0);
	if (len <= 0) {
		ERR("Connection closed or Error occured : %d\n", len);
		g_source_remove(serial_session.g_watch_id_server);
		g_source_remove(serial_session.g_watch_id_client);
		__close_client_socket();
		unlink(COM_SOCKET_PATH);
		return FALSE;
	}

	_write_to_usb(buffer, len);
	return TRUE;
}


static gboolean __g_io_accept_handler(GIOChannel *chan, GIOCondition cond, gpointer data)
{
	int serverfd;
	int clientfd;
	GIOChannel *io;
	struct sockaddr_un client_addr;
	socklen_t addrlen;
	addrlen = sizeof(client_addr);

	if ( cond & (G_IO_NVAL | G_IO_HUP | G_IO_ERR) )	{
		DBG("GIOCondition %d \n", cond);

		if(serial_session.server_socket >= 0) {
			close(serial_session.server_socket);
			serial_session.server_socket = 0;
		}
		return FALSE;
	}

	if(serial_session.state == SERIAL_SESSION_CONNECTED) {
		DBG("Connection already exists.....\n");
		return FALSE;
	}

	DBG("Waiting for connection request\n");
	serverfd = g_io_channel_unix_get_fd(chan);
	clientfd = accept(serverfd, (struct sockaddr *)&client_addr, &addrlen);
	if (clientfd > 0)	{
		DBG("serverfd:%d clientfd:%d\n", serverfd, clientfd);

		io = g_io_channel_unix_new(clientfd);
		g_io_channel_set_close_on_unref(io, TRUE);
		serial_session.g_watch_id_client = g_io_add_watch(io, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
			__g_io_server_handler, NULL);
		g_io_channel_unref(io);

		serial_session.client_socket= clientfd;
		serial_session.state = SERIAL_SESSION_CONNECTED;
	} else {
		ERR("Accept failed\n");
		return FALSE;
	}

	return TRUE;
}

static void __create_serial_server()
{
	int server_socket;
	struct sockaddr_un server_addr;
	mode_t sock_mode;

	if ((server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		ERR("sock create error\n");
		exit(1);
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	g_strlcpy(server_addr.sun_path, COM_SOCKET_PATH, sizeof(server_addr.sun_path));
	unlink(COM_SOCKET_PATH);

	/*---Assign a port number to the socket---*/
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
		perror("bind error: ");
		exit(1);
	}

	sock_mode = (S_IRWXU | S_IRWXG | S_IRWXO);	// has 777 permission
	if (chmod(COM_SOCKET_PATH, sock_mode) < 0) {
		perror("chmod error: ");
		close(server_socket);
		exit(1);
	}

	/*---Make it a "listening socket"---*/
	if (listen(server_socket, 1) != 0) {
		perror("listen error: ");
		close(server_socket);
		exit(1);
	}
	serial_session.server_socket = server_socket;
	serial_session.g_io = g_io_channel_unix_new(server_socket);
	g_io_channel_set_close_on_unref(serial_session.g_io, TRUE);
	serial_session.g_watch_id_server = g_io_add_watch(serial_session.g_io,
		G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
		__g_io_accept_handler, NULL);
	g_io_channel_unref(serial_session.g_io);

	return;
}


void _init_serial_server(void)
{
	__create_serial_server();
}

gboolean _deinit_serial_server(void)
{
	__close_client_socket();

	return TRUE;
}

gboolean _is_exist_serial_session(void)
{
	return (serial_session.state == SERIAL_SESSION_CONNECTED) ? TRUE: FALSE;
}

gboolean _wait_serial_session(void)
{
	int cnt = 0;
	while (_is_exist_serial_session() == FALSE) {
		usleep(SOCK_WAIT_TIME);
		if (cnt++ > SOCK_WAIT_CNT)
			return FALSE;
	}

	return TRUE;
}
