// SPDX-License-Identifier: GPL-2.0+
// MIUI ADD: Camera_CameraOpt
#include "cam_dump.h"

static unsigned int top_n = 8;
static struct kmem_cache *process_cache;
static DEFINE_MUTEX(dump_mutex);
static struct list_head dump_list;
static struct proc_dir_entry *cameramind_dir;

MODULE_IMPORT_NS(DMA_BUF);

static struct dma_buf *file_to_dma_buf(struct file *file)
{
	return file->private_data;
}

static struct proc_meminfo *process_info_add(struct task_struct *task, int rss_size,
					int ion_size)
{
	struct proc_meminfo *dump_info = NULL;
	const struct cred *cred;

	if (!task)
		return NULL;

	dump_info = kmem_cache_zalloc(process_cache, GFP_ATOMIC);
	if (!dump_info)
		return NULL;

	memcpy(dump_info->task_name, task->comm, TASK_COMM_LEN);
	dump_info->rss_size = rss_size;
	dump_info->ion_size = ion_size;
	dump_info->pid = task->pid;
	cred = get_task_cred(task);
	dump_info->uid = cred->uid.val;
	put_cred(cred);

	return dump_info;
}

static struct task_struct *find_lock_task_mm(struct task_struct *p)
{
	struct task_struct *t;

	rcu_read_lock();

	for_each_thread(p, t) {
		task_lock(t);
		if (likely(t->mm))
			goto found;
		task_unlock(t);
	}
	t = NULL;
found:
	rcu_read_unlock();

	return t;
}

static unsigned long process_get_ion_size(struct task_struct *task)
{
	int n = 0;
	size_t ion_size = 0;
	struct files_struct *files = NULL;
	struct fdtable *fdt = NULL;

	if (task->flags & PF_KTHREAD)
		return ion_size;

	files = task->files;
	if (!files)
		return ion_size;

	spin_lock(&files->file_lock);
	for (fdt = files_fdtable(files); n < fdt->max_fds; n++) {
		struct dma_buf *dbuf = NULL;
		struct file *f = rcu_dereference_check_fdtable(files, fdt->fd[n]);

		if (!f)
			continue;
		if (!is_dma_buf_file(f))
			continue;
		dbuf = file_to_dma_buf(f);
		if (!dbuf)
			continue;

		if (!dbuf->priv)
			continue;
		ion_size += dbuf->size;
	}
	spin_unlock(&files->file_lock);

	return (unsigned long)TO_KB(ion_size);
}

int process_info_scan(void)
{
	int count;
	struct task_struct *p;
	struct task_struct *task;
	unsigned long rss_size;
	unsigned long ion_size;
	struct proc_meminfo *dump_info = NULL;
	struct proc_meminfo *temp_info = NULL;
	ktime_t old_ns = ktime_get();

	while (!list_empty(&dump_list)) {
		temp_info = list_entry(dump_list.next, struct proc_meminfo, node_task);
		list_del(&temp_info->node_task);
		kmem_cache_free(process_cache, temp_info);
		temp_info = NULL;
	}

	rcu_read_lock();
	for_each_process(p) {
		if (!p)
			continue;

		task = find_lock_task_mm(p);
		if (!task)
			continue;

		rss_size = get_mm_rss(task->mm) + get_mm_counter(task->mm, MM_SWAPENTS);
		rss_size = PAGES_TO_KB(rss_size);
		ion_size = process_get_ion_size(task);

		if (list_empty(&dump_list)) {
			dump_info = process_info_add(task, rss_size, ion_size);
			if (dump_info)
				list_add(&dump_info->node_task, &dump_list);
			goto t_unlock;
		}

		count = 0;
		list_for_each_entry(temp_info, &dump_list, node_task) {
			count++;
		if (count >= top_n)
			break;
		if ((rss_size + ion_size) > (temp_info->rss_size + temp_info->ion_size)) {
			dump_info = process_info_add(task, rss_size, ion_size);
			if (dump_info)
				list_add_tail(&dump_info->node_task, &temp_info->node_task);
			break;
		} else if (list_is_last(&temp_info->node_task, &dump_list)) {
			dump_info = process_info_add(task, rss_size, ion_size);
			if (dump_info)
				list_add(&dump_info->node_task, &temp_info->node_task);
			break;
		}
	}
t_unlock:
	task_unlock(task);
	}
	rcu_read_unlock();
	pr_info("take %llu ns", ktime_get() - old_ns);

	return 0;
}

int dump_process_info(struct seq_file *m, void *v)
{
	int cnt = 0;
	struct proc_meminfo *temp_info = NULL;

	mutex_lock(&dump_mutex);
	process_info_scan();

	if (list_empty(&dump_list)) {
		pr_err("dump_list is empty\n");
		mutex_unlock(&dump_mutex);
		return -1;
	}

	list_for_each_entry(temp_info, &dump_list, node_task) {
		if (!temp_info)
			continue;
		if (cnt++ < top_n) {
			if (m) {
				seq_printf(m, "%-5d %-5d %-16s %dKB %dKB %dKB\n",
							temp_info->pid, temp_info->uid,
							temp_info->task_name,
							temp_info->rss_size + temp_info->ion_size,
							temp_info->rss_size, temp_info->ion_size);
			}
		}
	}
	mutex_unlock(&dump_mutex);
	return 0;
}

ssize_t	processinfo_dump_contrl(struct file *file, const char __user *buffer,
							size_t count, loff_t *ppos)
{
	int rv;

	rv = kstrtouint_from_user(buffer, count, 10, &top_n);
	if (rv < 0)
		return rv;
	if (top_n > 100 || top_n < 0)
		top_n = 8;
	return count;
}


static int processinfo_dump_open(struct inode *inode, struct file *file)
{
	return single_open(file, dump_process_info, NULL);
}

static const struct proc_ops process_info_fops = {
	.proc_open	= processinfo_dump_open,
	.proc_write	= processinfo_dump_contrl,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
};


void dump_init(void)
{
	struct proc_dir_entry *proc_entry;

	INIT_LIST_HEAD(&dump_list);
	process_cache = kmem_cache_create("process_info_cache",
		sizeof(struct proc_meminfo), 0, 0, NULL);
	if (!process_cache) {
		pr_err("create process_info_cache failed\n");
		return;
	}

	cameramind_dir = proc_mkdir("cameramind", NULL);
	if (!cameramind_dir) {
		pr_err("Failed to create /proc/cameramind directory\n");
		goto cleanup_cache;
	}

	proc_entry = proc_create("topn_process_info", 0664, cameramind_dir, &process_info_fops);
	if (!proc_entry) {
		pr_err("Failed to create /proc/cameramind/topn_process_info proc entry\n");
		proc_remove(cameramind_dir);
		cameramind_dir = NULL;
		goto cleanup_cache;
	} else {
		pr_info("/proc/cameramind/topn_process_info proc entry created successfully\n");
	}

	return;

cleanup_cache:
	kmem_cache_destroy(process_cache);
	process_cache = NULL;
}

// END Camera_CameraOpt
