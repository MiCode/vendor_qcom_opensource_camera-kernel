// MIUI ADD: Camera_CamSched
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/input.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>
#include <linux/timer.h>
#include <linux/notifier.h>
#include <linux/cpumask.h>
#include <linux/cpufreq.h>
#include <linux/fb.h>
#include <linux/poll.h>
#include <linux/kthread.h>
#include "mi_cam.h"
#define CREATE_TRACE_POINTS
#include "mi_cam_trace.h"

static uint cpu_param=0;
module_param(cpu_param, uint, 0644);

static struct micam_dev cam_dev;
static struct class *micam_class;
static unsigned int micam_major;

DEFINE_PER_CPU(struct micam_amu_data, micam_amu_data);
EXPORT_PER_CPU_SYMBOL_GPL(micam_amu_data);

#define DEFAULT_MEM_STALL_PCT_MAX (60)
#define DEFAULT_STALL_DELTA_MULT (1000)
static u64 mem_stall_pct_max = DEFAULT_MEM_STALL_PCT_MAX; //cutoff stall ratio by percentage
static u64 mem_stall_delta_mult = DEFAULT_STALL_DELTA_MULT;

#ifdef CONFIG_MI_CAM_WALT
static void micam_update_amu_cnt(struct micam_amu_data *arch_data)
{
	if (!arch_data)
		return;

	arch_data->mem_stall = read_sysreg_s(SYS_AMEVCNTR0_MEM_STALL);
	arch_data->amu_cyc = read_sysreg_s(SYS_AMEVCNTR0_CORE_EL0);
}

static bool micam_read_amu_counter_delta(struct micam_amu_data *arch_data)
{
	u64 mem_stall = 0;
	u64 amu_cyc = 0;

	if (!arch_data)
		return false;

	mem_stall = read_sysreg_s(SYS_AMEVCNTR0_MEM_STALL);
	amu_cyc = read_sysreg_s(SYS_AMEVCNTR0_CORE_EL0);
	if (unlikely(!(mem_stall && amu_cyc))) {
		pr_err("%s failed: mem_stall=%llu amu_cyc=%llu\n", __func__, mem_stall, amu_cyc);
		return false;
	} else {
		if (unlikely(mem_stall < arch_data->mem_stall)) { //MEM_STALL counter wrap
			arch_data->mem_stall_delta = U64_MAX - arch_data->mem_stall + mem_stall;
		} else {
			arch_data->mem_stall_delta = mem_stall - arch_data->mem_stall;
		}

		if (unlikely(amu_cyc < arch_data->amu_cyc)) { //CPU cycles counter wrap
			arch_data->amu_cyc_delta = U64_MAX - arch_data->amu_cyc + amu_cyc;
		} else {
			arch_data->amu_cyc_delta = amu_cyc - arch_data->amu_cyc;
		}

		arch_data->mem_stall = mem_stall;
		arch_data->amu_cyc = amu_cyc;
		trace_camsched_read_amu_counter(arch_data->cpu, mem_stall, amu_cyc, arch_data->mem_stall_delta, arch_data->amu_cyc_delta, arch_data->last_update);
		return true;
	}
}

u64 micam_calibrate_cpu_cycles(u64 wallclock, int cpu, u64 raw_cycles_delta)
{
	int stage = 0;
	u64 calc_data1 = 0;
	u64 caled_cycles_delta = 0;
	u64 mem_stall_delta = 0;
	u64 amu_cyc_delta = 0;
	u64 mem_stall_pct = 0;
	unsigned int enable_feature = 0;
	struct micam_dev *micam_dev = &cam_dev;
	struct micam_amu_data *arch_data = &per_cpu(micam_amu_data, raw_smp_processor_id());

	enable_feature = micam_dev->enable_feature;

	//Not Camera scene, update counters and return raw cycles
	if (!enable_feature) {
		micam_update_amu_cnt(arch_data);
		return raw_cycles_delta;
	}

	//Feature enabled, Update counter delta
	if ((arch_data->last_update != wallclock) &&
	    (micam_read_amu_counter_delta(arch_data))) {
		arch_data->last_update = wallclock;
	}

	//start to calibrate target cpu cycles
	arch_data = &per_cpu(micam_amu_data, cpu);
	mem_stall_delta = arch_data->mem_stall_delta;
	amu_cyc_delta = arch_data->amu_cyc_delta;

	//10x scaled mem_stall per cpu cycles percentage
	mem_stall_pct = mult_frac(mem_stall_delta, mem_stall_delta_mult, amu_cyc_delta);

	// maximum stall ratio percentage cutoff, calc_data1 is not allowed to exceed mem_stall_pct_max
	// ie, minimum of caled_cycles_delta is not allowed to lower than (100% - mem_stall_pct_max) compare to raw_cycles_delta
	calc_data1 = min(mem_stall_pct, mem_stall_pct_max);

	//CPU cycles calibration
	if (likely(amu_cyc_delta > mem_stall_delta)) {
		caled_cycles_delta = raw_cycles_delta - mult_frac(calc_data1, raw_cycles_delta, 100);
		if (likely(caled_cycles_delta < raw_cycles_delta)) {
			stage = 1;
		} else { //return raw_cycles_delta
			stage = 2;
			caled_cycles_delta = raw_cycles_delta;
		}
	} else { //return raw_cycles_delta
		stage = 3;
		caled_cycles_delta = raw_cycles_delta;
	}

	trace_camsched_calibrate_cpu_cycles(stage, cpu, caled_cycles_delta, raw_cycles_delta,
			amu_cyc_delta, mem_stall_delta, calc_data1);

	return caled_cycles_delta;
}
#endif

void micam_exception_process(struct timer_list *timer)
{
#ifdef CONFIG_MI_CAM_WALT
	struct micam_dev *micam_dev = &cam_dev;

	//Disable cpu-stall feature
	write_lock(&micam_dev->f_lock);
	micam_dev->enable_feature = 0;
	write_unlock(&micam_dev->f_lock);
	pr_err("%s:[MICAM_SCHED] trigger watchdog to exit micam_sched!\n", __func__);
#endif
}

static ssize_t cam_memstall_max_pct_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int rc = 0;
	int val = 0;
	rc = kstrtoint(buf, 10, &val);
	if (rc)
		return rc;
	mem_stall_pct_max = val;

	return count;
}

static ssize_t cam_memstall_max_pct_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", (int)mem_stall_pct_max);
}
static DEVICE_ATTR_RW(cam_memstall_max_pct);

static ssize_t cam_memstall_mult_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int rc = 0;
	int val = 0;
	rc = kstrtoint(buf, 10, &val);
	if (rc)
		return rc;
	mem_stall_delta_mult = val;

	return count;
}

static ssize_t cam_memstall_mult_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", (int)mem_stall_delta_mult);
}
static DEVICE_ATTR_RW(cam_memstall_mult);

static ssize_t cam_cpubg_pressure_enable_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int rc = 0;
	bool val = false;
	rc = kstrtobool(buf, &val);
	if (rc)
		return rc;

	cam_cpubg_pressure_enable = val;
	return count;
}

static ssize_t cam_cpubg_pressure_enable_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", cam_cpubg_pressure_enable);
}
static DEVICE_ATTR_RW(cam_cpubg_pressure_enable);

static ssize_t cam_cpubg_target_load_thresh_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int rc = 0;
	int val = 0;
	rc = kstrtoint(buf, 10, &val);
	if (rc)
		return rc;

	if (val <= CAM_CPUBG_DEFAULT_TL_THRESH)
		cam_cpubg_target_load_thresh = val;
	return count;
}

static ssize_t cam_cpubg_target_load_thresh_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", cam_cpubg_target_load_thresh);
}
static DEVICE_ATTR_RW(cam_cpubg_target_load_thresh);

static ssize_t cam_cpubg_max_freq_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int rc = 0;
	unsigned long val = 0;
	rc = kstrtoul(buf, 10, &val);
	if (rc)
		return rc;

	if (val <= CAM_CPUBG_DEFAULT_MAX_FREQ)
		cam_cpubg_max_freq = val;

	return count;
}

static ssize_t cam_cpubg_max_freq_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%lu\n", cam_cpubg_max_freq);
}
static DEVICE_ATTR_RW(cam_cpubg_max_freq);



static struct attribute *cam_cpubg_pressure_attrs[] = {
	&dev_attr_cam_cpubg_pressure_enable.attr,
	&dev_attr_cam_cpubg_target_load_thresh.attr,
	&dev_attr_cam_cpubg_max_freq.attr,
	&dev_attr_cam_memstall_max_pct.attr,
	&dev_attr_cam_memstall_mult.attr,
	NULL
};

static struct attribute_group cam_cpubg_pressure_attr_group = {
	.attrs = cam_cpubg_pressure_attrs,
};

static int micam_open(struct inode *inode, struct file *filp)
{
	pr_info("%s:[MICAM_SCHED] open success!\n",__func__);
	return 0;
}

static int micam_release(struct inode *inode, struct file *filp)
{
	pr_info("%s:[MICAM_SCHED] release success!\n",__func__);
	return 0;
}

ssize_t micam_read(struct file *file, char __user *buff, size_t length, loff_t *ppos)
{
	pr_info("%s:[MICAM_SCHED] read success!\n",__func__);
	return 0;
}

ssize_t micam_write(struct file *file, const char __user *buf, size_t length, loff_t *ppos)
{
	pr_info("%s:[MICAM_SCHED] write success!\n",__func__);
	return 0;
}

static long micam_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct micam_dev *micam_dev = &cam_dev;

	switch (cmd) {
		case MI_CAM_SCHED_ENABLE:
			write_lock(&micam_dev->f_lock);
			micam_dev->enable_feature = 1;
			write_unlock(&micam_dev->f_lock);
			mod_timer(&micam_dev->crm_wdog, jiffies +msecs_to_jiffies(5000));  //5s
			pr_info("%s : [MICAM_SCHED] Start adjust walt!\n", __func__);
			break;
		case MI_CAM_SCHED_DISABLE:
			write_lock(&micam_dev->f_lock);
			micam_dev->enable_feature = 0;
			write_unlock(&micam_dev->f_lock);
			del_timer(&micam_dev->crm_wdog);
			pr_info("%s: [MICAM_SCHED] Stop adjust walt!\n", __func__);
			break;
		case MI_CAM_SCHED_SET_WDOG:
			mod_timer(&micam_dev->crm_wdog, jiffies +msecs_to_jiffies(5000));  //5s
			break;
		case MI_CAM_SCHED_WRITE:
			break;
		case MI_CAM_SCHED_READ:
			break;
		default:
            ret = -EINVAL;
	}
	return ret;
}

static const struct file_operations micam_fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = micam_ioctl,
	.open = micam_open,
	.release = micam_release,
	//.poll = micam_poll,
	.read = micam_read,
	.write= micam_write,
};

static int __init mi_camera_init(void)
{
	int rc = 0;
	int cpu = 0;
	struct device *device;
	struct micam_dev *micam_dev =&cam_dev;
#ifdef CONFIG_MI_CAM_WALT
    struct micam_amu_data *amu_data = NULL;
#endif

	micam_major = register_chrdev(0, MI_CAM_DEV_NAME, &micam_fops);
	micam_class = class_create(MI_CAM_DEV_NAME);
	if (IS_ERR(micam_class)) {
		unregister_chrdev(micam_major,  MI_CAM_DEV_NAME);
		pr_warn("Failed to create class.\n");
		return PTR_ERR(micam_class);
	}

	micam_dev->enable_feature = 0;
	micam_dev->dev = MKDEV(micam_major, 0);
	device = device_create(micam_class, NULL, micam_dev->dev,
                NULL, MI_CAM_SCHED);
	if (IS_ERR(device)) {
		pr_err("error while trying to create %s\n", MI_CAM_SCHED);
		return -EINVAL;
	}

	micam_dev->micam_device = device;
	rwlock_init(&micam_dev->f_lock);

	timer_setup(&micam_dev->crm_wdog, micam_exception_process, 0);
	micam_dev->crm_wdog.expires = jiffies +msecs_to_jiffies(5000);   //5s

	if (device != NULL) {
		rc = sysfs_create_group(&device->kobj, &cam_cpubg_pressure_attr_group);
		if (rc) {
			pr_warn("Failed to create cam_cpubg_pressure attributes!, rc = %d\n", rc);
			return -EINVAL;
		}
    }

#ifdef CONFIG_MI_CAM_WALT
	for_each_possible_cpu(cpu) {
		amu_data = &per_cpu(micam_amu_data, cpu);
		memset(amu_data, 0, sizeof(micam_amu_data));
		amu_data->cpu = cpu;
	}

	register_micam_cpu_cycles_calibration_hook(micam_calibrate_cpu_cycles);
	pr_info("%s: [MICAM_SCHED] open success!\n", __func__);
#endif

	return 0;
}

static void __exit mi_cam_exit(void)
{
	struct micam_dev *micam_dev =&cam_dev;

#ifdef CONFIG_MI_CAM_WALT
	unregister_micam_cpu_cycles_calibration_hook();
	pr_info("%s: [MICAM_SCHED] Close success!\n", __func__);
#endif

	sysfs_remove_group(&micam_dev->micam_device->kobj, &cam_cpubg_pressure_attr_group);
	device_destroy(micam_class, micam_dev->dev);
	class_destroy(micam_class);
}

module_init(mi_camera_init);
module_exit(mi_cam_exit);
MODULE_DESCRIPTION("mi cam sched driver");
MODULE_LICENSE("GPL v2");
// END Camera_CamSched
