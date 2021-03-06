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



#ifndef _DR_COMMON_H_
#define _DR_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <dlog.h>



#define DR_MID "DR"
#define DBG(fmt, args...) SLOG(LOG_DEBUG, DR_MID, "[%s()][Ln:%d] "fmt, \
					__func__, __LINE__, ##args)
#define ERR(fmt, args...) SLOG(LOG_ERROR, DR_MID, "[%s()][Ln:%d] "fmt, \
						__func__, __LINE__, ##args)


int _get_usb_state(int *usb_state);

#endif
