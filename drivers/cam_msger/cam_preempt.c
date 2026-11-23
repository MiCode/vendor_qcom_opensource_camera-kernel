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
#include "cam_preempt.h"
#include "cam_sched.h"
#include "cam_msger_common.h"
#include <linux/sched/cputime.h>
#include <trace/hooks/sched.h>
#include "../../../kernel/sched/sched.h"

#define LEN_KEY_WORD 3
#define DPM_KEY_WORD "dpm"

#define WRITE_ERROR_CPUMASK_INVALID -1015
#define WRITE_ERROR_CSCHED_MALLOC_FAILED -1016
#define WRITE_ERROR_VAR_GET_FAILED -1020

static struct   cam_sched_data *dpmdata;

void cam_preempt_ioctl(struct file *filp, unsigned int cmd, unsigned long arg, u8 *hit)
{
	int i = 0;
	struct cam_sched_data *dpmdata_bp = NULL;

	if (cmd != IOC_CMD_DMP_DUMP)
		return;

	*hit = 1;
	pr_info("dump cam_premmpt[struct cam_sched_data]\n");

	if (dpmdata == NULL)
		return;

	rcu_read_lock();
	dpmdata_bp = rcu_dereference(dpmdata);
	pr_info("offset:%d\n", dpmdata_bp->offset);
	pr_info("ctl_cpus:%*pbl\n", cpumask_pr_args(&(dpmdata_bp->ctl_cpus)));
	pr_info("CPU-flags:\n");

	for (i = 0; i < CPU_NR; i++)
		pr_info("cpu:%d flag:%d\n", i, dpmdata_bp->flag[i]);

	pr_info("The target process:\n");
	for (i = 0; i < MONITOR_MAX_PID; i++) {
		if (dpmdata_bp->pids[i] > 0)
			pr_info("		  i:%d %d\n", i, dpmdata_bp->pids[i]);
	}
	rcu_read_unlock();
}

int  cam_preempt_write(struct file *fp, char *buff, int length, u8 *hit)
{
	int  i = 0;
	int  j = 0;
	int  idx = 0;
	int  cpu = 0;
	int  cpumask	= 0;
	int  token_len  = 0;
	int  tisot_flag = TISOT_DPM;
	int  iscmdLine = 1;
	char *token	 = NULL;
	char *rest	  = buff;

	u8	offset = 0;
	pid_t pids[MONITOR_MAX_PID] = { 0 };
	int pid_tmp = -1;

	cpumask_t ctl_cpus;

	struct cam_sched_data *dpmdata_new = NULL;
	struct cam_sched_data *dpmdata_bp = NULL;

	if (buff == NULL)
		return WRITE_ERROR_BUF_ADDR_INVALID;

	m_pr(LTYPE_DPM, "[%s] buff:%s len:%d\n", __func__, buff, length);

	*hit = 0;
	if (!(msger_siwtches & DPM_ENABLE))
		return WRITE_ERROR_CAM_DPM_DISABLE;

	if (strncmp(buff, DPM_KEY_WORD, LEN_KEY_WORD) == 0)
		*hit = 1;

	if (!(*hit))
		return -1;

	if (length <= WRITE_CONTENT_MIN_LEN)
		return WRITE_ERROR_LEN_INVALID;

	if (length >= WRITE_CONTENT_MAX_LEN)
		return WRITE_ERROR_LEN_INVALID;

	if (strstr(buff, SPLIT_DELIM) == NULL)
		return WRITE_ERROR_FORMAT_INVALID;

	while ((token = strsep(&rest, SPLIT_DELIM)) != NULL) {
		m_pr(LTYPE_DPM, "[%s] idx:%d token:%s sizeof_token:%lu rest:%s\n", __func__, idx, token
			, strnlen(token, CMDLINE_MAX_LEN), rest);
		if (offset >= MONITOR_MAX_PID)
			break;

		if (idx == 1) {
			if (kstrtoint(token, 10, &cpumask))
				return WRITE_ERROR_CPUMASK_INVALID;

			m_pr(LTYPE_DPM, "[%s] cpumask:%d\n", __func__, cpumask);

			if (cpumask > 1023)
				return WRITE_ERROR_CPUMASK_INVALID;

			if (cpumask == 0)
				break;

			goto __continue__;
		}
		if (idx == 2) {
			if (kstrtoint(token, 10, &iscmdLine)) {
				m_pr(LTYPE_DPM, "[%s] iscmdLine var get failed.\n", __func__);
				return WRITE_ERROR_VAR_GET_FAILED;
			}
		}
		if (idx > 2) {
			if (iscmdLine >= 1) {
				token_len = strnlen(token, CMDLINE_MAX_LEN) - 1;
				m_pr(LTYPE_DPM, "[%s] token:%s\n", __func__, token);
				if (token_len <= 0)
					goto __continue__;

				pid_tmp = find_pid_cmdline(token);
				if (pid_tmp < 0) {
					m_pr(LTYPE_DPM, "[%s] token:%s pid:%d\n", __func__, token, pid_tmp);
					goto __continue__;
				}
				m_pr(LTYPE_DPM, "[%s] find pid:%d\n", __func__, pid_tmp);
				pids[offset++] = pid_tmp;
			} else {
				if (kstrtoint(token, 10, &pid_tmp)) {
					m_pr(LTYPE_DPM, "[%s] pid_tmp var get failed.\n", __func__);
					return WRITE_ERROR_VAR_GET_FAILED;
				}
				m_pr(LTYPE_DPM, "[%s] find pid:%d\n", __func__, pid_tmp);
				pids[offset++] = pid_tmp;
			}
		}
__continue__:
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

	dpmdata_new = kmalloc(sizeof(struct cam_sched_data), GFP_KERNEL);
	if (dpmdata_new == NULL)
		return WRITE_ERROR_CSCHED_MALLOC_FAILED;

	clear_cam_sched_data_entity(dpmdata_new);

	if (cpumask != 0) {
		for_each_possible_cpu(cpu) {
			if (cpumask & (1 << cpu)) {
				cpumask_set_cpu(cpu, &ctl_cpus);
				dpmdata_new->flag[cpu] = tisot_flag;
			}
		}
		memcpy(&(dpmdata_new->pids[0]), &(pids[0]), sizeof(pid_t) * MONITOR_MAX_PID);
		dpmdata_new->offset = offset;
		cpumask_copy(&(dpmdata_new->ctl_cpus), &ctl_cpus);
		m_pr(LTYPE_DPM, "[%s] ctl_cpus:%*pbl\n", __func__, cpumask_pr_args(&ctl_cpus));
	}

	dpmdata_bp = dpmdata;
	rcu_assign_pointer(dpmdata, dpmdata_new);
	synchronize_rcu();
	kfree(dpmdata_bp);
	return length;

}
static bool task_should_be_blocked(struct task_struct *p, int cpu)
{
	bool ret = false;
	int  i = 0;
	int offset = 0;
	pid_t pids_tmp[MONITOR_MAX_PID] = { 0 };

	if (!cpu_active(cpu))
		return ret;

	if (!p || p->mm == NULL)
		return ret;

	if (p->nr_cpus_allowed <= 2)
		return ret;

	rcu_read_lock();
	if (rcu_dereference(dpmdata)->flag[cpu] != TISOT_DPM) {
		rcu_read_unlock();
		return ret;
	}
	offset = rcu_dereference(dpmdata)->offset;
	memcpy(&(pids_tmp[0]), &(rcu_dereference(dpmdata)->pids[0]), sizeof(pid_t) * MONITOR_MAX_PID);
	rcu_read_unlock();

	if (offset == 0)
		goto __func_done__;

	if (p->tgid > pids_tmp[0] || p->tgid < pids_tmp[offset-1])
		goto __func_done__;

	for (i = 0; i < offset; i++) {
		if (p->tgid == pids_tmp[i]) {
			ret = true;
			goto __func_done__;
		}
	}

__func_done__:
	m_pr(LTYPE_DPM, "[%s] %s:%d cpu:%d ret:%d\n", __func__, p->comm, p->pid, cpu, ret);
	return ret;
}
void  cam_preempt_is_cpu_allowed(void *unused, struct task_struct *p, int cpu, bool *allowed)
{
	struct task_struct *rq_curr = NULL;
	bool p_is_target = false;
	bool rq_curr_is_target = false;
	// p -> wakee task

	if (!(msger_siwtches & DPM_ENABLE))
		return;

	rq_curr = cpu_rq(cpu)->curr;
	p_is_target = task_should_be_blocked(p, task_cpu(p));
	rq_curr_is_target = task_should_be_blocked(rq_curr, task_cpu(rq_curr));

	//rq_curr_is_target  p_is_target
	//true			     false	      allowed = false
	//true			     true         allowed = true
	//false			     false        return;
	//false			     true         allowed = true
	if (rq_curr_is_target && !p_is_target) {
		*allowed   = false;
		m_pr(LTYPE_DPM, "[%s] %s:%d is run on cpu(%d) not allowd %s:%d to run.\n", __func__, rq_curr->comm, rq_curr->pid, cpu, p->comm, p->pid);
		return;
	}
	if (rq_curr_is_target && p_is_target) {
		*allowed   = true;
		m_pr(LTYPE_DPM, "[%s] %s:%d is run on cpu(%d) allowd %s:%d to run.\n", __func__, rq_curr->comm, rq_curr->pid, cpu, p->comm, p->pid);
		return;
	}
}
static void xmi_rvh_check_preempt_wakeup_fair(void *unused, struct rq *rq, struct task_struct *p, bool *preempt, bool *nopreempt,
	int wake_flags, struct sched_entity *se, struct sched_entity *pse)
{

	struct task_struct *rq_curr = NULL;
	bool p_is_target = false;
	bool rq_curr_is_target = false;
	// p -> wakee task

	if (!(msger_siwtches & DPM_ENABLE))
		return;

	rq_curr = rq->curr;
	p_is_target = task_should_be_blocked(p, task_cpu(p));
	rq_curr_is_target = task_should_be_blocked(rq_curr, task_cpu(rq_curr));

	//rq_curr_is_target  p_is_target
	//true			     false	      preempt = false, nopreempt = true;
	//true			     true         preempt = false, nopreempt = true;
	//false			     false        return;
	//false			     true         preempt = true
	if (rq_curr_is_target && !p_is_target) {
		*preempt   = false;
		*nopreempt = true;
		m_pr(LTYPE_DPM, "[%s] %s:%d is run on cpu(%d) preempt set to false.\n", __func__, rq_curr->comm, rq_curr->pid, smp_processor_id());
		return;
	}
	if (rq_curr_is_target && p_is_target) {
		*preempt = true;
		*nopreempt = false;
		m_pr(LTYPE_DPM, "[%s] %s:%d wait to run on cpu(%d) preempt set to true.\n", __func__, p->comm, p->pid, smp_processor_id());
	}
}
static void xmi_rvh_check_preempt_wakeup_ignore(void *unused, struct task_struct *p, bool *ignore)
{
	unsigned int cpu_p = -1;
	//p -> current task
	if (!(msger_siwtches & DPM_ENABLE))
		return;
	cpu_p = task_cpu(p);
	if (task_should_be_blocked(p, cpu_p)) {
		*ignore = true;
		m_pr(LTYPE_DPM, "[%s] %s:%d run on cpu(%d) ignore wakeup preempt check.\n", __func__, p->comm, p->pid, smp_processor_id());
	}
}

void cam_preempt_init(void)
{
	dpmdata = kmalloc(sizeof(struct cam_sched_data), GFP_KERNEL);
	if (dpmdata == NULL) {
		m_pr(LTYPE_DPM, "[%s] struct xmi_cam_sched init failed.\n", __func__);
		return;
	}
	clear_cam_sched_data_entity(dpmdata);
	register_trace_android_rvh_check_preempt_wakeup_ignore(xmi_rvh_check_preempt_wakeup_ignore, NULL);
	register_trace_android_rvh_check_preempt_wakeup_fair(xmi_rvh_check_preempt_wakeup_fair, NULL);
	m_pr(LTYPE_DPM, "[%s] finish.\n", __func__);
}
// END Camera_CameraOpt
