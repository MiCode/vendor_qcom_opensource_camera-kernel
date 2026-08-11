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
#include "cam_mm.h"
#include <trace/hooks/vmscan.h>
#include "cam_msger_common.h"
#include <linux/string_helpers.h>
#include <linux/mmzone.h>
#include <linux/mm.h>
#include <linux/fdtable.h>
#include <linux/pid.h>
#include "cam_pid.h"
#include <linux/fs.h>

#define WRITE_ERR_CMD_LIENE_INVALID  -5001
#define WRITE_ERR_FORMAT_INVALID	 -5002
#define WRITE_ERR_KERL_ALLOC_FAILE   -5004
#define WRITE_ERR_WATERMARK_TYPE_INVALID     -5005
#define WRITE_ERR_WATERMARK_DISTANCE_INVALID -5006

// static void sched_dump(void){
// }
void cam_mm_ioctl(struct file *filp, unsigned int cmd,  unsigned long arg, u8 *hit)
{
	if (cmd != IOC_CMD_MM_DUMP)
		return;
    //sched_dump();
	*hit = 1;
}
void try_to_free_file_cache(int target_pid) {
	struct pid     *pid;
	struct file    *file;
	struct inode   *inode;
	struct fdtable *fdt;
	struct task_struct  *task;
	struct files_struct * files;
	struct inode_entry *entry;
	int i, count = 0;
	LIST_HEAD(inode_list);

	if (target_pid <= 0) {
		m_pr(LTYPE_MM, "[%s] target_pid(%d) value is valid.", __func__, target_pid);
		return;
	}
	pid = find_get_pid(target_pid);
	if ( !pid ) {
		m_pr(LTYPE_MM, "[%s] target_pid(%d) can not find pid_struct.", __func__, target_pid);
		return;
	}
	task = get_pid_task(pid, PIDTYPE_PID);
	if ( !task ) {
		m_pr(LTYPE_MM, "[%s] target_pid(%d) can not find task_struct.", __func__, target_pid);
		return;
	}
	task_lock(task);
	files = task->files;
	if ( !files ) {
		task_unlock(task);
		m_pr(LTYPE_MM, "[%s] target_pid(%d) can not find task_struct.", __func__, target_pid);
		goto out;
	}

	rcu_read_lock();
	fdt = files_fdtable(files);
	for(i = 0; i < fdt->max_fds; i++) {
		file = rcu_dereference(fdt->fd[i]);
		if ( !file )
			continue;
		inode = file_inode(file);
		if ( !inode )
			continue;
		spin_lock(&inode->i_lock);
		if ((inode->i_state & (I_FREEING|I_WILL_FREE|I_NEW)) ||
		    (mapping_empty(inode->i_mapping) && !need_resched())) {
			spin_unlock(&inode->i_lock);
			continue;
		}
		atomic_inc(&inode->i_count);
		spin_unlock(&inode->i_lock);

		entry = kmalloc(sizeof(struct inode_entry), GFP_ATOMIC);
		if(!entry) {
			iput(inode);
			continue;
		}
		entry->inode = inode;
		list_add_tail(&entry->list, &inode_list);
		count++;
	}
	rcu_read_unlock();
	task_unlock(task);

	while(!list_empty(&inode_list)){
		entry = list_first_entry(&inode_list, struct inode_entry, list);
		list_del(&entry->list);
		invalidate_mapping_pages(entry->inode->i_mapping, 0, -1);
		iput(entry->inode);
		kfree(entry);
		cond_resched();
	}

	m_pr(LTYPE_MM,"[%s] Invalidated %d file mappings for PID %d \n.", __func__, count, target_pid);

out:
	put_task_struct(task);
	put_pid(pid);
}
int cam_mm_rw(struct file *fp, char *buff, int length, u8 *hit)
{
	int  idx = 0;
	int  node = 0;
	int  zoneid = 0;
	int  watermark_type = -1;
	int  watermark_distance = -1;
	char *rest	= buff;
	char *token   = NULL;
	struct zone *zone;
	unsigned long flags;
	pg_data_t *pgdat = NULL;

	if (buff == NULL || length <= 3)
		return WRITE_ERR_CMD_LIENE_INVALID;

	*hit = 0;
	if (!(msger_siwtches & MM_ENABLE))
		return -1;

	if (strncmp(buff, "mm", 2) == 0)
		*hit = 1;

	if (!(*hit))
		return -1;

	if (strstr(buff, SPLIT_DELIM) == NULL)
		return WRITE_ERR_FORMAT_INVALID;

	m_pr(LTYPE_MM, "[%s] buff:%s len:%d\n", __func__, buff, length);

	while ((token = strsep(&rest, SPLIT_DELIM)) != NULL) {
		if (idx > 2)
			break;

		if (idx == 1) {
			if (kstrtoint(token, 10, &watermark_type))
				return WRITE_ERR_WATERMARK_TYPE_INVALID;
			if (watermark_type < WATHERMARK_TYPE_MIN || watermark_type > MEMORY_OPERATION_TYPE_DROP_FILE_CACHE)
				return WRITE_ERR_WATERMARK_TYPE_INVALID;

			m_pr(LTYPE_MM, " %s watermark_type:%d.\n", __func__, watermark_type);
		}
		if (idx == 2) {
			if (kstrtoint(token, 10, &watermark_distance))
				return WRITE_ERR_WATERMARK_TYPE_INVALID;
			if (watermark_distance < 0)
				return WRITE_ERR_WATERMARK_TYPE_INVALID;

			m_pr(LTYPE_MM, " %s watermark_distance:%d.\n", __func__, watermark_distance);
		}
		idx++;
	}

	if(watermark_type == MEMORY_OPERATION_TYPE_DROP_FILE_CACHE) {
		try_to_free_file_cache(watermark_distance);
	} else {
		for_each_online_node(node) {
			pgdat = NODE_DATA(node);
			for (zoneid = 0; zoneid < MAX_NR_ZONES; zoneid++) {
				zone = &pgdat->node_zones[zoneid];
				spin_lock_irqsave(&zone->lock, flags);
				if (watermark_type == WATHERMARK_TYPE_MIN)
					zone->_watermark[WMARK_MIN]   = min_wmark_pages(zone) + watermark_distance;

				if (watermark_type == WATHERMARK_TYPE_LOW)
					zone->_watermark[WMARK_LOW]   = low_wmark_pages(zone) + watermark_distance;

				if (watermark_type == WATHERMARK_TYPE_HIGH)
					zone->_watermark[WMARK_HIGH]  = high_wmark_pages(zone) + watermark_distance;

				spin_unlock_irqrestore(&zone->lock, flags);
			}
		}
	}

	return length;
}

//static void xmi_vh_tune_swappiness(void *unused, int *swappiness)
//{
//	m_pr(LTYPE_MM, " %s swappiness:%d.\n", __func__, *swappiness);
//}
//static void xmi_rvh_set_balance_anon_file_reclaim(void *unused, bool *balance_anon_file_reclaim)
//{
//	m_pr(LTYPE_MM, " %s balance_anon_file_reclaim:%d.\n", __func__, *balance_anon_file_reclaim);
//}
//static void xmi_trace_android_vh_tune_scan_type(void *unused, enum scan_balance *scan_type)
//{
//}
void cam_mm_init(void)
{
	m_pr(LTYPE_MM, " %s finish.\n", __func__);
	//register_trace_android_vh_tune_swappiness(xmi_vh_tune_swappiness, NULL);
	//register_trace_android_rvh_set_balance_anon_file_reclaim(xmi_rvh_set_balance_anon_file_reclaim, NULL);
	//register_trace_android_vh_tune_scan_type(xmi_trace_android_vh_tune_scan_type, NULL);
}
// END Camera_CameraOpt
