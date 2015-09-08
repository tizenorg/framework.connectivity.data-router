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


#include <time.h>
#include "dr-main.h"
#include "dr-parser.h"
#include "dr-common.h"
#include "dr-usb.h"


typedef struct {
	const char *at_cmd;
	int token_type;
} dr_at_cmd_t;

const dr_at_cmd_t dr_at_cmds[] = {
	{"ATZ", ATZ_TOKEN},
	{"AT+OSPSERIALOPEN", AT_OSP_TOKEN},
	{"tizen.request='tizen.initialize'", AT_TIZEN_OSP_TOKEN},
	{"", OTHER_TOKEN},
};


int _get_at_cmd_type(char *buf)
{
	if (buf == NULL) {
		return TOKEN_ERROR;
	}

	int idx;

	for (idx = 0; dr_at_cmds[idx].at_cmd[0] != '\0'; idx++) {
		if (!g_ascii_strncasecmp
		    ((const gchar *)buf, (const gchar *)dr_at_cmds[idx].at_cmd,
		     strlen(dr_at_cmds[idx].at_cmd))) {
			DBG("Found %s\n", dr_at_cmds[idx].at_cmd);
			buf = buf + strlen(dr_at_cmds[idx].at_cmd);
			return dr_at_cmds[idx].token_type;
		}
	}

	return OTHER_TOKEN;
}

