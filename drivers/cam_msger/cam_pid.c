// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
// MIUI ADD: Camera_CameraOpt
#include "cam_pid.h"
#include "cam_msger_common.h"

#define WRITE_ERR_CMD_LIENE_INVALID  -3001
#define WRITE_ERR_FORMAT_INVALID	 -3002
#define DEFAULT_VALUE_TGID_NOT_FOUND -3003
#define WRITE_ERR_KERL_ALLOC_FAILE   -3004
#define WRITE_PID_NOT_FOUND		  -3005

int get_tgid_by_cmdline(struct file *fp, char *buff, int length, u8 *hit)
{
	//struct task_struct *task = NULL;
	int  idx = 0;
	int  token_len = 0;
	int  pid = DEFAULT_VALUE_TGID_NOT_FOUND;
	char *rest	= buff;
	char *token   = NULL;

	if (buff == NULL || length <= 4)
		return WRITE_ERR_CMD_LIENE_INVALID;

	*hit = 0;
	if (!(msger_siwtches & PID_ENABLE))
		return -1;
	if (strncmp(buff, "ctp", 3) == 0)
		*hit = 1;

	if (!(*hit))
		return -1;

	if (strstr(buff, SPLIT_DELIM) == NULL)
		return WRITE_ERR_FORMAT_INVALID;

	while ((token = strsep(&rest, SPLIT_DELIM)) != NULL) {
		if (idx > 1)
			break;

		if (idx == 1) {
			token_len = strnlen(token, CMDLINE_MAX_LEN)-1;
			if (token_len <= 0) {
				pid = WRITE_ERR_CMD_LIENE_INVALID;
				return pid;
			}
			pid = find_pid_cmdline(token);
			if (pid < 0) {
				pid = WRITE_PID_NOT_FOUND;
				break;
			}
		}
		idx++;
	}

	return pid;
}
// END Camera_CameraOpt
