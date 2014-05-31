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



#ifndef _DATA_ROUTER_H_
#define _DATA_ROUTER_H_

#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <net/if.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <vconf.h>
#include <vconf-keys.h>




/** 
  * @def         USB_BUFFER_SIZE
  * Defines the maximum buffer size for the USB
  */
#define USB_BUFFER_SIZE		1500
#define ERRMSG_SIZE				100


/** @internal
 *   This structure has information about the Modem
 */

typedef union {
	struct {
		volatile unsigned int dtr:1;
		volatile unsigned int rts:1;
		volatile unsigned int reserved:6;
	} bits;
	volatile unsigned char state;
} dr_modem_input_lines;

typedef union {
	struct {
		volatile unsigned int dcd:1;
		volatile unsigned int dsr:1;
		volatile unsigned int brk:1;
		volatile unsigned int ri:1;
		volatile unsigned int reserved:4;
	} bits;
	volatile unsigned char state;
} dr_modem_output_lines;


/** @internal
 *   This structure has information about modem connection lines
 */
typedef struct {
	dr_modem_output_lines output_line_state;
	dr_modem_input_lines input_line_state;
} dr_line_info_t;


/** @internal
 *   This structure has information about the USB connection
 */
typedef struct {
	gboolean connected;				/**< To check whether USB is connected or not */
	pthread_t thread_id;				/**< USB thread id */
	int usb_fd;					/**< USB file descriptor */
	int usb_ctrl_fd;
	int data_path;				/**< Data path */
	char data_buffer[USB_BUFFER_SIZE+1];	/**< Data buffer */
} dr_usb_info_t;


/** @internal
 *   This structure has information about the DR
 */
typedef struct {
	dr_usb_info_t usb;						/**< Information about USB */
	dr_line_info_t line;						/**< Information about modem status lines */
} dr_info_t;



gboolean _deinit_dr(void);


#endif				//_DATA_ROUTER_H_
