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
#include "cam_sched.h"
#include <trace/hooks/sched.h>
#include <linux/string_helpers.h>
#include <linux/sched/signal.h>
#include <linux/cpumask.h>
#include <linux/slab.h>
#include "cam_msger_common.h"

#define WRITE_ERR_USR_BUFF_NULL	-2000
#define WRITE_ERR_CONT_2_SHORT	  -2001
#define WRITE_ERR_FORMAT_INVALID  -2002
#define WRITE_ERR_CPUMASK_INVALID -2003
#define WRITE_ERR_CSCHED_MALLOC_FAILED -2004
#define WRITE_ERR_KERL_ALLOC_FAILE	-2005

#define WF_MIGRATED 0x20 /* Internal use, task got migrated */
//WF_MIGRATED sync from /kernel_platform/common/kernel/sched/sched.h

static struct cam_sched_data *csdata;

static void sched_dump(void)
{
	int i = 0;
	struct cam_sched_data *csdata_bp;

	pr_info("struct cam_sched_data[csdata]\n");
	if (csdata == NULL)
		return;
	rcu_read_lock();
	csdata_bp = rcu_dereference(csdata);
	pr_info("offset:%d\n", csdata_bp->offset);
	pr_info("ctl_cpus:%*pbl\n", cpumask_pr_args(&(csdata_bp->ctl_cpus)));
	pr_info("CPU-flags:\n");
	for (i = 0; i < CPU_NR; i++)
		pr_info("cpu:%d flag:%d\n", i, csdata_bp->flag[i]);

	pr_info("The PID of the managed thread:\n");
	for (i = 0; i < MONITOR_MAX_PID; i++) {
		if (csdata_bp->pids[i] > 0)
			pr_info("i:%d %d\n", i, csdata_bp->pids[i]);
	}
	rcu_read_unlock();
}
void clear_cam_sched_data_entity(struct cam_sched_data *item)
{
	if (item == NULL)
		return;

	item->offset = 0;
	cpumask_clear(&(item->ctl_cpus));
	memset(&(item->pids), 0, sizeof(pid_t) * MONITOR_MAX_PID);
	memset(&(item->flag[0]), TISOT_DEFAULT, sizeof(u8) * CPU_NR);
}

void cam_sched_ioctl(struct file *filp, unsigned int cmd,  unsigned long arg, u8 *hit)
{
	if (cmd != IOC_CMD_SCHED_DUMP)
		return;

	sched_dump();
	*hit = 1;
}
int cam_sched_w_pids(struct file *fp, char *buff, int length, u8 *hit)
{
	int  i = 0;
	int  j = 0;
	int  idx = 0;
	int  cpu = 0;
	int  cpumask	= 0;
	int  token_len  = 0;
	int  tisot_flag = TISOT_NEVERALLOW;
	char *token	 = NULL;
	char *rest	  = buff;

	u8	offset = 0;
	pid_t pids[MONITOR_MAX_PID] = { 0 };
	int pid_tmp = -1;

	cpumask_t ctl_cpus;

	struct cam_sched_data *csdata_new = NULL;
	struct cam_sched_data *csdata_bp  = NULL;

	if (buff == NULL)
		return WRITE_ERR_USR_BUFF_NULL;

	m_pr(LTYPE_SCHED, "[%s] buff:%s len:%d\n", __func__, buff, length);

	*hit = 0;
	if (!(msger_siwtches & SCD_ENABLE))
		return -1;

	if (strncmp(buff, "ctl", 3) == 0)
		*hit = 1;

	if (!(*hit))
		return -1;

	if (length <= WRITE_CONTENT_MIN_LEN)
		return WRITE_ERR_CONT_2_SHORT;

	if (length >= WRITE_CONTENT_MAX_LEN)
		return WRITE_ERR_FORMAT_INVALID;

	if (strstr(buff, SPLIT_DELIM) == NULL)
		return WRITE_ERR_FORMAT_INVALID;

	while ((token = strsep(&rest, SPLIT_DELIM)) != NULL) {
		m_pr(LTYPE_SCHED, "[%s] idx:%d token:%s sizeof_token:%lu rest:%s\n", __func__, idx,
			token, strnlen(token, CMDLINE_MAX_LEN), rest);

		if (offset >= MONITOR_MAX_PID)
			break;

		if (idx == 1) {
			if (kstrtoint(token, 10, &cpumask))
				return WRITE_ERR_CPUMASK_INVALID;

			m_pr(LTYPE_SCHED, "[%s] cpumask:%d\n", __func__, cpumask);
			if (cpumask > 255)
				return WRITE_ERR_CPUMASK_INVALID;

			if (cpumask == 0)
				break;
			idx++;
			continue;
		}

		if (idx == 2) {
			m_pr(LTYPE_SCHED, "[%s] token_len:%lu, %lu, %lu\n", __func__, strlen(token), strlen(TISOT_ALLOW_STR), strlen(TISOT_NRALLOW_STR));
			if (strnlen(token, TISOT_STR_LEN_MAX) >= strlen(TISOT_ALLOW_STR)
				&& strncmp(token, TISOT_ALLOW_STR, strlen(TISOT_ALLOW_STR)) == 0) {
				tisot_flag = TISOT_ALLOW;
				idx++;
				continue;
			}
			if (strnlen(token, TISOT_STR_LEN_MAX) >= strlen(TISOT_NRALLOW_STR)
				&& strncmp(token, TISOT_NRALLOW_STR, strlen(TISOT_NRALLOW_STR)) == 0) {
				tisot_flag = TISOT_NEVERALLOW;
				idx++;
				continue;
			}
			m_pr(LTYPE_SCHED, "[%s] tisot_flag:%X\n", __func__, tisot_flag);
			break;
		}
		if (idx > 2) {
			token_len = strnlen(token, CMDLINE_MAX_LEN) - 1;
			if (token_len <= 0) {
				idx++;
				continue;
			}
			pid_tmp = find_pid_cmdline(token);
			if (pid_tmp < 0) {
				m_pr(LTYPE_SCHED, "[%s] token:%s pid:%d\n", __func__, token, pid_tmp);
				idx++;
				continue;
			}
			m_pr(LTYPE_SCHED, "[%s] token:%s pid:%d\n", __func__, token, pid_tmp);
			pids[offset++] = pid_tmp;
		}
		idx++;
	}

	if (offset != 0) {
		for (i = 0; i < offset-1; i++) {
			for (j = i+1; j < offset; j++) {
				if (pids[i] < pids[j]) {
					pids[i] ^= pids[j];
					pids[j] ^= pids[i];
					pids[i] ^= pids[j];
				}
			}
		}
	}

	csdata_new = kmalloc(sizeof(struct cam_sched_data), GFP_KERNEL);
	if (csdata_new == NULL)
		return WRITE_ERR_CSCHED_MALLOC_FAILED;

	clear_cam_sched_data_entity(csdata_new);

	if (cpumask != 0) {
		for_each_possible_cpu(cpu) {
			if (cpumask & (1 << cpu)) {
				cpumask_set_cpu(cpu, &ctl_cpus);
				csdata_new->flag[cpu] = tisot_flag;
			}
		}
		memcpy(&(csdata_new->pids[0]), &(pids[0]), sizeof(pid_t) * MONITOR_MAX_PID);
		csdata_new->offset = offset;
		cpumask_copy(&(csdata_new->ctl_cpus), &ctl_cpus);
		m_pr(LTYPE_SCHED, "[%s] ctl_cpus:%*pbl\n", __func__, cpumask_pr_args(&ctl_cpus));
	}

	csdata_bp = csdata;
	rcu_assign_pointer(csdata, csdata_new);
	synchronize_rcu();
	kfree(csdata_bp);
	return length;
}
static bool effect_cpu_action(struct task_struct *p, int cpu, u8 *flag, bool *ctl_proc, int *offset)
{
	bool ret = false;
	int  i = 0;
	pid_t pids_tmp[MONITOR_MAX_PID] = { 0 };

	if (!cpu_active(cpu))
		return ret;

	if (!p || p->mm == NULL)
		return ret;

	if (p->nr_cpus_allowed <= 2)
		return ret;

	rcu_read_lock();
	if (rcu_dereference(csdata)->flag[cpu] == TISOT_DEFAULT) {
		rcu_read_unlock();
		return ret;
	}
	*flag	= rcu_dereference(csdata)->flag[cpu];
	*offset = rcu_dereference(csdata)->offset;
	memcpy(&(pids_tmp[0]), &(rcu_dereference(csdata)->pids[0]), sizeof(pid_t) * MONITOR_MAX_PID);
	rcu_read_unlock();

	if (*offset == 0) {
		*ctl_proc = true;
		ret = true;
		goto __func_done__;
	}

	if ((*flag & TISOT_NEVERALLOW)) {
		if (p->tgid > pids_tmp[0] || p->tgid < pids_tmp[*offset-1])
			goto __func_done__;

		for (i = 0; i < *offset; i++) {
			if (p->tgid == pids_tmp[i]) {
				*ctl_proc = true;
				ret = true;
				goto __func_done__;
			}
		}
	}

	if ((*flag & TISOT_ALLOW)) {
		ret = true;
		if (p->tgid > pids_tmp[0] || p->tgid < pids_tmp[*offset-1])
			goto __func_done__;

		for (i = 0; i < *offset; i++) {
			if (p->tgid == pids_tmp[i]) {
				*ctl_proc = true;
				ret = false;
				goto __func_done__;
			}
		}
	}

__func_done__:
	m_pr(LTYPE_SCHED, "[%s] %s:%d cpu:%d ret:%d\n", __func__, p->comm, p->pid, cpu, ret);
	return ret;
}
static void filter_cpu(struct task_struct *p, int prev_cpu, int *cpu, bool ctl_proc, int flag, int offset)
{
	int  cpuidx  = -1;
	bool is_ctl = false;
	cpumask_t allowed_cpus;

	if (p == NULL)
		return;
	cpumask_clear(&(allowed_cpus));

	m_pr(LTYPE_SCHED, "[%s] %s-%d cpu:%d ctl_proc:%d flag:%d offset:%d\n", __func__
		, p->comm, p->pid, *cpu, ctl_proc, flag, offset);

	rcu_read_lock();
	is_ctl = cpumask_test_cpu(prev_cpu, &(rcu_dereference(csdata)->ctl_cpus));
	cpumask_andnot(&(allowed_cpus), p->cpus_ptr, &(rcu_dereference(csdata)->ctl_cpus));
	rcu_read_unlock();

	m_pr(LTYPE_SCHED, "[%s] %s-%d cpu:%d allowed_cpus:%*pbl\n", __func__, p->comm, p->pid, *cpu, cpumask_pr_args(&(allowed_cpus)));

	if (cpumask_empty(&(allowed_cpus)))
		return;

	cpuidx = prev_cpu;
	if (is_ctl)
		cpuidx = cpumask_any(&(allowed_cpus));

	if (offset == 0
		|| (flag == TISOT_ALLOW && !ctl_proc)
		|| (flag == TISOT_NEVERALLOW && ctl_proc)) {
		*cpu = cpuidx;
	}
}
static void xmi_rvh_is_cpu_allowed(void *unused, struct task_struct *p, int cpu, bool *allowed)
{
	u8	flag	 = TISOT_DEFAULT;
	int  offset	= 0;
	bool ctl_proc = false;

	if (effect_cpu_action(p, cpu, &flag, &ctl_proc, &offset)) {
		if (offset == 0) {
			*allowed = false;
			return;
		}
		*allowed = true;
		if ((flag == TISOT_ALLOW && !ctl_proc) || (flag == TISOT_NEVERALLOW && ctl_proc))
			*allowed = false;

		m_pr(LTYPE_SCHED, "[%s] %s-%d cpu:%d allowed:%d\n", __func__, p->comm, p->pid, cpu, *allowed);
	}
}
static void xmi_rvh_select_task_rq_fair(void *unused, struct task_struct *p, int prev_cpu, int sd_flag, int wake_flags, int *new_cpu)
{
	u8	flag	  = TISOT_DEFAULT;
	int  offset	= 0;
	bool ctl_proc = false;

	if (!(msger_siwtches & SCD_ENABLE))
		return;

	if (!(wake_flags & WF_MIGRATED))
		return;

	if (effect_cpu_action(p, *new_cpu, &flag, &ctl_proc, &offset)) {
		m_pr(LTYPE_SCHED, "[%s-begin] %s-%d prev_cpu:%d new_cpu:%d\n", __func__, p->comm, p->pid, prev_cpu, *new_cpu);
		filter_cpu(p, prev_cpu, new_cpu, ctl_proc, flag, offset);
		if (*new_cpu == -1)
			*new_cpu = prev_cpu;

		m_pr(LTYPE_SCHED, "[%s-finish] %s-%d prev_cpu:%d new_cpu:%d\n", __func__, p->comm, p->pid, prev_cpu, *new_cpu);
	}
}
static void xmi_can_migrate_task(void *unused, struct task_struct *p, int dst_cpu, int *can_migrate)
{
	u8	flag	 = TISOT_DEFAULT;
	int  offset	= 0;
	bool ctl_proc = false;

	if (!(msger_siwtches & SCD_ENABLE))
		return;

	if (effect_cpu_action(p, dst_cpu, &flag, &ctl_proc, &offset)) {
		m_pr(LTYPE_SCHED, "[%s-begin] %s-%d dest_cpu:%d cam_migrate:%d ctl_proc:%d flag:%d\n", __func__, p->comm, p->pid, dst_cpu, *can_migrate, ctl_proc, flag);
		if (offset == 0) {
			*can_migrate = 0;
			goto __func_done_;
		}
		*can_migrate = 1;
		if (flag == TISOT_ALLOW && !ctl_proc) {
			*can_migrate = 0;
			goto __func_done_;
		}
		if (flag == TISOT_NEVERALLOW && ctl_proc)
			*can_migrate = 0;

__func_done_:
		m_pr(LTYPE_SCHED, "[%s-finish] %s-%d dest_cpu:%d cam_migrate:%d\n", __func__, p->comm, p->pid, dst_cpu, *can_migrate);
	}
}
static void xmi_sched_newidle_balance(void *unused, struct rq *this_rq, struct rq_flags *rf, int *pulled_task, int *done)
{
	unsigned int this_cpu = get_cpu();

	put_cpu();

	if (!(msger_siwtches & SCD_ENABLE))
		return;

	if (*pulled_task == 1)
		return;//qcom already balance and pull the task.

	rcu_read_lock();
	if (rcu_dereference(csdata)->flag[this_cpu] == TISOT_DEFAULT) {
		rcu_read_unlock();
		return;
	}
	*done = 1;
	rcu_read_unlock();
	m_pr(LTYPE_SCHED, "[%s] this-cpu:%d done:%d pulled_task:%d\n", __func__, this_cpu, *done, *pulled_task);
}

void cam_sched_init(void)
{
	csdata = kmalloc(sizeof(struct cam_sched_data), GFP_KERNEL);
	if (csdata == NULL) {
		m_pr(LTYPE_SCHED, "[%s] struct xmi_cam_sched init failed.\n", __func__);
		return;
	}
	clear_cam_sched_data_entity(csdata);

	register_trace_android_rvh_is_cpu_allowed(xmi_rvh_is_cpu_allowed, NULL);
	register_trace_android_rvh_can_migrate_task(xmi_can_migrate_task, NULL);
	register_trace_android_rvh_select_task_rq_fair(xmi_rvh_select_task_rq_fair, NULL);
	register_trace_android_rvh_sched_newidle_balance(xmi_sched_newidle_balance, NULL);
	m_pr(LTYPE_SCHED, " %s finish.\n", __func__);
}
// END Camera_CameraOpt
