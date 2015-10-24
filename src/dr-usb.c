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


#include <poll.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <glib.h>
#include <sys/reboot.h>

#include "dr-main.h"
#include "dr-modem.h"
#include "dr-usb.h"
#include "dr-parser.h"
#include "dr-common.h"
#include "dr-ipc.h"
#include "dr-util.h"


#define SOCK_WAIT_TIME 500000
#define USB_TTY_NODE	"/dev/ttyGS0"
#define USB_CTL_NODE	"/dev/dun"


extern dr_info_t dr_info;
extern volatile gboolean dsr_status;

struct pollfd poll_events[2];

static void *__usb_monitor_thread(void *arg);


static int __open_usb_node(void)
{
	int usb_fd = 0;
	int ctl_fd = 0;
	struct termios tty_termios;
	char err_buf[ERRMSG_SIZE] = {0, };

	INFO("Open usb device");

	usb_fd = open(USB_TTY_NODE, O_RDWR);
	if (usb_fd < 0) {
		strerror_r(errno, err_buf, ERRMSG_SIZE);
		ERR("%s open failed. %s (%d)", USB_TTY_NODE, err_buf, errno);
		goto failed;
	}

	ctl_fd = open(USB_CTL_NODE, O_RDWR);
	if (ctl_fd < 0) {
		strerror_r(errno, err_buf, ERRMSG_SIZE);
		ERR("%s open failed. %s (%d)", USB_CTL_NODE, err_buf, errno);
		goto failed;
	}

	memset((char *)&tty_termios, 0, sizeof(struct termios));

	if (tcgetattr(usb_fd, &tty_termios) < 0) {
		strerror_r(errno, err_buf, ERRMSG_SIZE);
		ERR("tcgetattr error : %s (%d)\n", err_buf, errno);
		goto failed;
	}

	cfmakeraw(&tty_termios);

	tty_termios.c_iflag &= ~ICRNL;
	tty_termios.c_iflag &= ~INLCR;
	tty_termios.c_oflag &= ~OCRNL;
	tty_termios.c_oflag &= ~ONLCR;
	tty_termios.c_lflag &= ~ICANON;
	tty_termios.c_lflag &= ~ECHO;

	if (tcsetattr(usb_fd, TCSANOW, &tty_termios) < 0) {
		strerror_r(errno, err_buf, ERRMSG_SIZE);
		ERR("tcsetattr error : %s (%d)\n", err_buf, errno);
		goto failed;
	}

	poll_events[0].fd = usb_fd;
	poll_events[0].events = POLLIN | POLLERR | POLLHUP;
	poll_events[0].revents = 0;

	poll_events[1].fd = ctl_fd;
	poll_events[1].events = POLLIN | POLLERR | POLLHUP;
	poll_events[1].revents = 0;

	dr_info.line.output_line_state.state = 0;
	dr_info.line.input_line_state.state = 0;
	dr_info.usb.usb_fd = usb_fd;
	dr_info.usb.usb_ctrl_fd = ctl_fd;

	INFO("usb_fd= 0x%x\n", usb_fd);
	INFO("usb_ctrl_fd= 0x%x\n", ctl_fd);
	return 0;

failed:
	if(usb_fd >= 0) {
		close(usb_fd);
	}
	if(ctl_fd >= 0) {
		close(ctl_fd);
	}
	return -1;
}

static int __close_usb_node(void)
{
	INFO("Close usb device");

	DBG("usb_fd = 0x%x, ctrl_fd = 0x%x\n",
			dr_info.usb.usb_fd, dr_info.usb.usb_ctrl_fd);
	if (dr_info.usb.usb_fd > 0) {
		if (close(dr_info.usb.usb_fd) != 0) {
			ERR("ttyGS0 Close error");
			return -1;
		}
		DBG("ttyGS0 Close success");
		dr_info.usb.usb_fd = 0;
	}

	if (dr_info.usb.usb_ctrl_fd > 0) {
		if (close(dr_info.usb.usb_ctrl_fd) != 0) {
			ERR("ACM Close error");
			return -1;
		}
		DBG("ACM Close success");
		dr_info.usb.usb_ctrl_fd = 0;
	}
	return 0;
}



int _init_usb(void)
{
	int ret = 0;

	INFO("Initialize usb interface");

	ret = __open_usb_node();
	if (ret < 0) {
		ERR("USB node open failed");
		return -1;
	}

	if (0 != pthread_create(&dr_info.usb.thread_id, NULL,
				__usb_monitor_thread, &dr_info.usb.usb_fd)) {
		ERR("USB  thread launch failed");
		__close_usb_node();
		return -1;
	}

//	_send_dtr_ctrl_signal(DTR_ON);

	DBG("- ");
	return 0;
}


void _deinit_usb(void)
{
	int status;

	INFO("Deinitialize usb interface");

	if(dr_info.usb.thread_id <= 0) {
		ERR("Invalid USB interface  !!!");
		return;
	}
	INFO("USB disconnected!!!!!!!!!!");

	_send_dtr_ctrl_signal(DTR_OFF);

	/*This should be called in main thread */
	pthread_cancel(dr_info.usb.thread_id);
	pthread_join(dr_info.usb.thread_id, (void **)&status);
	DBG("Thread status : %d \n", status);

	_send_serial_status_signal(SERIAL_CLOSED);
	__close_usb_node();
	DBG("-\n");
	return;
}



int _write_to_usb(char *buf, int buf_len)
{
	int write_len;

	write_len = 0;
	do {
		int len;
		len = write(dr_info.usb.usb_fd, buf + write_len,
				buf_len - write_len);
		if (len == -1) {
			char err_buf[ERRMSG_SIZE] = {0, };
			strerror_r(errno, err_buf, ERRMSG_SIZE);
			ERR("USB write failed : %s [%d]", err_buf, errno);
			break;
		}
		write_len += len;
	} while (write_len < buf_len);

	return write_len;
}


int _send_usb_line_state(int ctrl)
{
	int ret;

	INFO("Set ioctl [%x]", ctrl);

	ret = ioctl(dr_info.usb.usb_ctrl_fd, GS_CDC_NOTIFY_SERIAL_STATE, ctrl);
	if (ret < 0) {
		int err = errno;
		char err_buf[ERRMSG_SIZE] = { 0, };
		strerror_r(err, err_buf, ERRMSG_SIZE);
		ERR("ioctl error : %s (%d)", err_buf, err);
		return -EIO;
	}
	return 0;
}

static void __process_at_cmd(char *buffer, int nread)
{
	if(buffer == NULL || nread == 0)
		return;

	int type;
	int check_cnt = 0;
	char *info = NULL;
	static char *tmp = NULL;
	static gboolean wait_cr = FALSE;

	DBG("Received: %s [%d byte]\n",buffer, nread);
	if(TRUE == _is_exist_serial_session()) {
		_write_to_serial_client(buffer, nread);
		return;
	}

	/* Handling at cmd comes from hyperterminal
		 Default format is "AT<command...><CR>"
		 But several application use different format */
	if (buffer[nread -1] != '\r' && buffer[nread -1] != '\n' && buffer[nread -1] != '\0') {
		if (tmp == NULL)
			tmp = g_malloc0(USB_BUFFER_SIZE);

		g_strlcat(tmp, buffer, USB_BUFFER_SIZE);
		wait_cr = TRUE;
		DBG("Waiting <CR>");
		return;
	} else if (wait_cr && buffer[nread -1] == '\r') {
		DBG("Founded <CR>");
		if (tmp != NULL) {
			g_strlcat(tmp, buffer, USB_BUFFER_SIZE);
			g_strlcpy(buffer, tmp, USB_BUFFER_SIZE);
			nread = strlen(buffer);
			g_free(tmp);
			tmp = NULL;
		}
		wait_cr = FALSE;
	}

	type = _get_at_cmd_type(buffer, nread);
	switch (type) {
	case ATZ_TOKEN:
		info = OK;
		_write_to_usb(info, strlen(info));
		return;
	case AT_OSP_TOKEN:
	case AT_TIZEN_OSP_TOKEN:
		_init_serial_server();

		if (vconf_set_int(VCONFKEY_DR_OSP_SERIAL_OPEN_INT,
				VCONFKEY_DR_OSP_SERIAL_OPEN) != 0) {
			ERR("vconf set failed\n");

			if (type == AT_OSP_TOKEN)
				info = ERROR;
			else
				info = "\r\ntizen.response='tizen.failure'\r\n";

			_write_to_usb(info, strlen(info));
			return;
		}

		if (_wait_serial_session()) {
			if (type == AT_OSP_TOKEN)
				info = OK;
			else
				info = "\r\ntizen.response='tizen.success'\r\n";
		} else {
			if (type == AT_OSP_TOKEN)
				info = ERROR;
			else
				info = "\r\ntizen.response='tizen.failure'\r\n";
		}

		_write_to_usb(info, strlen(info));
		return;
	case AT_SERIAL_TEST:
		_init_serial_server();
		_system_cmd_ext("/usr/bin/serial-client-test", NULL);
		return;
	default:
		break;
	}

	while (dsr_status == FALSE) {
		sleep(1);
		if (check_cnt > 3) {
			info = "\r\nERROR\r\n";
			_write_to_usb(info, strlen(info));
			return;
		}
		check_cnt++;
	}
	check_cnt = 0;

	return;
}

static void *__usb_monitor_thread(void *arg)
{
	int ret = 0;
	int nread = 0;
	int usb_fd = 0;
	int poll_state = 0;
	char *buffer;

	usb_fd = *((int *)arg);
	DBG("USB monitor thread launched, usb_fd= 0x%x", usb_fd);
	while (usb_fd) {
		poll_state = poll((struct pollfd *)&poll_events, 2, -1);
		if (poll_state > 0) {
			if (poll_events[0].revents & POLLIN) {
				buffer = g_malloc0(USB_BUFFER_SIZE + 1);
				if (buffer == NULL) {
					ERR("Fail to allocate memory");
					__close_usb_node();
					pthread_exit(NULL);
				}
				nread = read(usb_fd, buffer, USB_BUFFER_SIZE);
				if (nread < 0) {
					g_free(buffer);
					if (errno == EINTR) {
						DBG("EINTR...");
						continue;
					}
					char err_buf[ERRMSG_SIZE] = {0, };
					strerror_r(errno, err_buf, ERRMSG_SIZE);
					ERR("Read %s failed : %s [%d]", USB_TTY_NODE, err_buf, errno);
					__close_usb_node();
					pthread_exit(NULL);
				}

				*(buffer + nread) = 0;
				__process_at_cmd(buffer, nread);
				g_free(buffer);
			}

			if (poll_events[0].revents & POLLHUP ||
				poll_events[0].revents & POLLERR) {
				ERR("%s occurred !!! [%s]",
						(poll_events[0].revents & POLLHUP)
						? "POLLHUP" : "POLLERR",
						USB_TTY_NODE);
				__close_usb_node();
				pthread_exit(NULL);
			}

			if (poll_events[1].revents & POLLIN) {
				unsigned int line_state;

				ret = read(dr_info.usb.usb_ctrl_fd,
						&line_state, sizeof(unsigned int));
				if (ret < 0) {
					ERR("read length is less then zero");
					if (errno == EINTR) {
						continue;
					}
					__close_usb_node();
					pthread_exit(NULL);
				}

				INFO("line state %x", line_state);

				DBG("line state: rts%c, dtr%c",
					     line_state & ACM_CTRL_RTS ? '+' : '-',
					     line_state & ACM_CTRL_DTR ? '+' : '-');
				DBG("(!(line_state & ACM_CTRL_DTR)): %d",
						(!(line_state & ACM_CTRL_DTR)));

				if ((line_state & ACM_CTRL_DTR) &&
						(dr_info.line.input_line_state.bits.dtr == FALSE)) {
					INFO("ACM_CTRL_DTR+ received");
					if(dr_info.line.output_line_state.bits.dsr == FALSE) {
					/*
					* When USB initialized, DTR On is set to CP.
					* If DR set DTR ON twice, Modem will be reponse as DSR Off
					*/
						_send_dtr_ctrl_signal(DTR_ON);
					}
					dr_info.line.input_line_state.bits.dtr = TRUE;
				} else if ((!(line_state & ACM_CTRL_DTR)) &&
						(dr_info.line.input_line_state.bits.dtr == TRUE)) {
					INFO("ACM_CTRL_DTR- received");
					if (_deinit_serial_server() == TRUE) {
						if(TRUE == _is_exist_serial_session())
							_send_serial_status_signal(SERIAL_CLOSED);
					}

					if (dsr_status == TRUE) {
						_send_dtr_ctrl_signal(DTR_OFF);
						dr_info.line.input_line_state.bits.dtr = FALSE;
					} else {
						INFO("DSR status is OFF, Skip the DTR-");
					}
				}
			}

			if (poll_events[1].revents & POLLHUP ||
				poll_events[1].revents & POLLERR) {
				ERR("%s occurred !!! [%s]",
						(poll_events[1].revents & POLLHUP)
						? "POLLHUP" : "POLLERR",
						USB_CTL_NODE);
				__close_usb_node();
				pthread_exit(NULL);
			}

		}
		else if (poll_state == 0)
			ERR("poll timeout");
		else if (poll_state < 0)
			ERR("poll error");
	}
	return NULL;
}
