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



#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <glib.h>

#include "dr-main.h"
#include "dr-util.h"
#include "dr-common.h"



int _system_cmd(const char *command)
{
	int pid = 0, status = 0;
	char *const environ[] = { NULL };

	if (NULL == command) {
		ERR("Invalid param !!!\n");
		return -1;
	}

	pid = fork();

	if (-1 == pid) {
		ERR("::: fork failed to create a process :::\n");
		return -1;
	}

	if (pid == 0) {
		char *argv[4];

		argv[0] = "sh";
		argv[1] = "-c";
		argv[2] = (char *)command;
		argv[3] = 0;

		execve("/bin/sh", argv, environ);
		abort();
	}

	do {
		if (waitpid(pid, &status, 0) == -1) {
			if (errno != EINTR)
				return -1;
		} else {
			return status;
		}

	} while (1);
}

int _system_cmd_ext(const char *cmd, char *const arg_list[])
{
	int pid, pid2;
	int status = 0;

	pid = fork();
	switch (pid) {
	case -1:
		perror("fork failed");
		return -1;

	case 0:
		pid2 = fork();
		if(pid2 == 0) {
			execv(cmd, arg_list);
			exit(256);
		} else
			exit(0);
		break;

	default:
		waitpid(pid, &status, 0);
		break;
	}

	return 0;
}

