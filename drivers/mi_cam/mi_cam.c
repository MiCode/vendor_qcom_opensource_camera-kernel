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
#include <linux/percpu.h>
#include <trace/hooks/cpufreq.h>
#include <trace/hooks/cpuidle.h>
#include "mi_cam.h"
#define CREATE_TRACE_POINTS
#include "mi_cam_trace.h"
#include <linux/cpuidle.h>
#include <linux/cpufreq.h>

static uint cpu_param=0;
module_param(cpu_param, uint, 0644);

static struct micam_dev cam_dev;
static struct class *micam_class;
static unsigned int micam_major;

DEFINE_PER_CPU(struct micam_amu_data, micam_amu_data);
EXPORT_PER_CPU_SYMBOL_GPL(micam_amu_data);

DECLARE_PER_CPU(struct waltgov_cam_cpu, waltgov_cam_cpu);

#define DEFAULT_MEM_STALL_PCT_MAX (60)
#define DEFAULT_STALL_DELTA_MULT (1000)
static u64 mem_stall_pct_max = DEFAULT_MEM_STALL_PCT_MAX; //cutoff stall ratio by percentage
static u64 mem_stall_delta_mult = DEFAULT_STALL_DELTA_MULT;

#ifdef MI_CAM_LOW_DEMAND_IDLE_SUPPORT
#define MAX_CAM_LOW_DEMAND_IDLE_LEVEL 2
#define CAM_HI_LOAD_FREQ_KHZ 1800000
#define CAM_IDLE_WDT_START 0xff
#define CAM_IDLE_WDT_STOP 0xfe
static struct irq_work lowfreq_irq_work;
static struct irq_work restore_freq_irq_work;
static struct cpumask cpu_cores_idle_state_bitmap = CPU_MASK_NONE;
static struct timer_list low_demand_idle_wdog;
static int cam_low_demand_idle = 0;
static int idle_lowfreq_req = 0;
static bool idle_lowfreq_assert = false;
static unsigned int saved_freq = 0;
static int max_cpu_id = 0;
static int cores_threshold = 0;
static struct cpufreq_policy *idle_lowfreq_policy = NULL;
void register_low_demand_idle_to_pm_driver(bool, int, int);
#endif /* MI_CAM_LOW_DEMAND_IDLE_SUPPORT */

#ifdef CONFIG_MI_CAM_WALT

struct waltgov_cam_cpu {
	unsigned long	util;
	unsigned long	cap;
	int	util_thresh;
	int	inflate_factor;
};

/*
 * util - cpu utilization
 * fmax - cpufreq_policy.policy.cpuinfo.max_freq
 * cap  - arch_scale_cpu_capacity by default, maybe adjust in waltgov_walt_adjust()
 * util_thresh - CPU ZONE util threshold
 * inflate_factor - inflate_factor
 */
static unsigned long cam_map_util_freq(unsigned long util, struct cpufreq_policy *policy,
				unsigned long cap, int util_thresh, int inflate_factor)
{
	unsigned long fmax = policy->cpuinfo.max_freq;
	unsigned long util_boost_factor;

	util_boost_factor = (fmax + (fmax >> 2));

	/*
	 * We are updating util_boost_factor to a set value for a specific utilization
	 */
	if (util_thresh == -1) {
		goto out;
	}

	if (util <= util_thresh) {
		util_boost_factor = inflate_factor;
	}
out:
	return (util_boost_factor * util/cap);
}

static unsigned int cam_freq_calibration(unsigned long driving_util, unsigned int driving_cpu,
		struct cpufreq_policy *policy, unsigned long driving_freq)
{
	unsigned int cpu;
	unsigned long target_freq = 0;
	unsigned int fmax_cpu = 0;
	unsigned int fmax_util = 0;
	unsigned long map_freq = 0;
	struct waltgov_cam_cpu *wg_cam_cpu = NULL;
	unsigned long final_freq = driving_freq;

	//Find the max cpu and target freq expect the driving_cpu(0-3)
	for_each_cpu(cpu, policy->cpus) {
		if (driving_cpu == cpu)
			continue;
		wg_cam_cpu = &per_cpu(waltgov_cam_cpu, cpu);

		map_freq = cam_map_util_freq(wg_cam_cpu->util, policy, wg_cam_cpu->cap,
				wg_cam_cpu->util_thresh, wg_cam_cpu->inflate_factor);
		if (map_freq > target_freq) {
			target_freq = map_freq;
			fmax_cpu = cpu;
			fmax_util = wg_cam_cpu->util;
		}
	}

	//Freq decision:Raw freq -> traiblazer_floor --> adaptive_freq --> IPC smart freq --> XM hooks
	if (driving_freq >= target_freq && driving_freq >= cam_cpubg_ceil_freq) {
		target_freq = min_t(unsigned long, target_freq, policy->cpuinfo.max_freq);
		final_freq = max_t(unsigned long, target_freq, cam_cpubg_ceil_freq);
	} else if (target_freq >= driving_freq && target_freq >= cam_cpubg_ceil_freq) {
		driving_freq = min_t(unsigned long, driving_freq, policy->cpuinfo.max_freq);
		final_freq = min_t(unsigned long, driving_freq, cam_cpubg_ceil_freq);
	} else {
		final_freq = driving_freq;
	}

	trace_cam_freq_calibration(driving_cpu, driving_util, driving_freq,
			fmax_cpu, fmax_util, target_freq, final_freq, policy->cpuinfo.max_freq);
	return final_freq;
}

static void cam_set_next_freq(unsigned long driving_util, unsigned int driving_cpu, unsigned long driving_freq,
		unsigned long *next_freq, struct cpufreq_policy *policy, bool *need_freq_update)
{
	struct micam_dev *micam_dev = &cam_dev;
	unsigned int enable_feature = 0;
	unsigned long old_freq = *next_freq;

	if (read_trylock(&micam_dev->f_lock)) {
		enable_feature = micam_dev->enable_feature;
		read_unlock(&micam_dev->f_lock);
	}

	if (!enable_feature && driving_cpu < cam_cpubg_nr_cpus) {
		*next_freq = cam_freq_calibration(driving_util, driving_cpu, policy, old_freq);
		if (*next_freq && *next_freq != old_freq) {
			*need_freq_update = true;
		}
		trace_cam_set_next_freq(enable_feature, driving_cpu, policy->cpuinfo.max_freq, driving_util, 
				old_freq, *next_freq, *need_freq_update);
	}
}

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
#ifdef MI_CAM_LOW_DEMAND_IDLE_SUPPORT
	cam_low_demand_idle = 0;
	register_low_demand_idle_to_pm_driver(false, 0, 0);
#endif/* MI_CAM_LOW_DEMAND_IDLE_SUPPORT */
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

static ssize_t cam_cpubg_ceil_freq_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int rc = 0;
	unsigned long val = 0;
	rc = kstrtoul(buf, 10, &val);
	if (rc)
		return rc;

	if (val <= CAM_CPUBG_DEFAULT_MAX_FREQ)
		cam_cpubg_ceil_freq = val;

	return count;
}

static ssize_t cam_cpubg_ceil_freq_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%lu\n", cam_cpubg_ceil_freq);
}
static DEVICE_ATTR_RW(cam_cpubg_ceil_freq);

static ssize_t cam_cpubg_nr_cpus_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int rc = 0;
	unsigned long val = 0;
	rc = kstrtoul(buf, 10, &val);
	if (rc)
		return rc;

	if (val <= CAM_CPUBG_MAX_NR_CPUS)
		cam_cpubg_nr_cpus = val;

	return count;
}

static ssize_t cam_cpubg_nr_cpus_show(struct device *dev,
				struct device_attribute *atttr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", cam_cpubg_nr_cpus);
}
static DEVICE_ATTR_RW(cam_cpubg_nr_cpus);

#ifdef MI_CAM_LOW_DEMAND_IDLE_SUPPORT
static ssize_t cam_low_demand_idle_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int rc = 0;
	int val = 0;

	if (buf[0] == CAM_IDLE_WDT_START) {//watchdog trigger
		mod_timer(&low_demand_idle_wdog, jiffies+msecs_to_jiffies(5000));  //5s
		return count;
	} else if (buf[0] == CAM_IDLE_WDT_STOP) {//stop watchdog
		del_timer(&low_demand_idle_wdog);
		return count;
	}

	rc = kstrtoint(buf, 10, &val);
	if (rc)
		return rc;

	if (val > MAX_CAM_LOW_DEMAND_IDLE_LEVEL)
		val = MAX_CAM_LOW_DEMAND_IDLE_LEVEL;

	if (val && cpumask_equal(idle_lowfreq_policy->cpus, idle_lowfreq_policy->related_cpus)) {
		cam_low_demand_idle = val;
		register_low_demand_idle_to_pm_driver(true, cam_low_demand_idle, cpumask_weight(idle_lowfreq_policy->cpus));
	} else {
		cam_low_demand_idle = 0;
		register_low_demand_idle_to_pm_driver(false, 0, 0);
	}

	return count;
}

static ssize_t cam_low_demand_idle_show(struct device *dev,
				struct device_attribute *atttr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", cam_low_demand_idle);
}
static DEVICE_ATTR_RW(cam_low_demand_idle);

static void idle_exception_process(struct timer_list *timer)
{
	cam_low_demand_idle = 0;
	register_low_demand_idle_to_pm_driver(false, 0, 0);
}
#endif/* MI_CAM_LOW_DEMAND_IDLE_SUPPORT */

static struct attribute *mi_cam_attrs[] = {
	&dev_attr_cam_cpubg_ceil_freq.attr,
	&dev_attr_cam_cpubg_nr_cpus.attr,
	&dev_attr_cam_memstall_max_pct.attr,
	&dev_attr_cam_memstall_mult.attr,
#ifdef MI_CAM_LOW_DEMAND_IDLE_SUPPORT
	&dev_attr_cam_low_demand_idle.attr,
#endif/* MI_CAM_LOW_DEMAND_IDLE_SUPPORT */
	NULL
};

static struct attribute_group mi_cam_attr_group = {
	.attrs = mi_cam_attrs,
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

#ifdef MI_CAM_LOW_DEMAND_IDLE_SUPPORT
static void low_demand_idle_cpu_freq(void *unused,
			struct cpufreq_policy *policy,
			unsigned int *target_freq,
			unsigned int old_target_freq)
{
	if (policy->cpu > 0) //only for cluster 0
		return;

	if (idle_lowfreq_req && !idle_lowfreq_assert) //request from myself
		idle_lowfreq_assert = true;
	else //from other user, override my request
		idle_lowfreq_req = 0;
}

static void lowfreq_request(struct irq_work *irq_work)
{
	struct cpufreq_policy *policy = idle_lowfreq_policy;

	if(policy->cur != policy->min) {
		saved_freq = policy->cur;
		idle_lowfreq_assert = false; //wait for cpufreq to set freq
		cpufreq_driver_fast_switch(policy, policy->min);
	}
}

static void lowfreq_release(struct irq_work *irq_work)
{
	struct cpufreq_policy *policy = idle_lowfreq_policy;

	if(cpumask_weight(&cpu_cores_idle_state_bitmap) == 0) {
		idle_lowfreq_req = 0;
		if(policy->cur == policy->min)
			cpufreq_driver_fast_switch(policy, saved_freq);
	}
}

static void low_demand_idle_enter_idle(void *unused, int *state,
				     struct cpuidle_device *dev)
{
	if (dev->cpu > max_cpu_id)
		return;
	cpumask_set_cpu(dev->cpu, &cpu_cores_idle_state_bitmap);
	if (!idle_lowfreq_req && cpumask_weight(&cpu_cores_idle_state_bitmap) == cores_threshold) {/*self core running only, other cores of cluster 0 are in idle*/
		if (idle_lowfreq_policy->cur < CAM_HI_LOAD_FREQ_KHZ) { //not allowed when heavy load reach high freq
			idle_lowfreq_req = 1;
			irq_work_queue_on(&lowfreq_irq_work, dev->cpu^1);/*wake another core to set freq*/
		}
	}
}

static void low_demand_idle_exit_idle(void *unused, int state,
				     struct cpuidle_device *dev)
{
	if (dev->cpu > max_cpu_id)
		return;
	cpumask_clear_cpu(dev->cpu, &cpu_cores_idle_state_bitmap);
	if(idle_lowfreq_req && cpumask_weight(&cpu_cores_idle_state_bitmap) == 0) {
		irq_work_queue(&restore_freq_irq_work);/*queue to restore freq*/
	}
}

void register_low_demand_idle_to_pm_driver(bool enable, int level, int cores_num)
{
	if (enable && idle_lowfreq_policy) {
		cores_threshold = cores_num + 1 - level;
		max_cpu_id = cores_num - 1;
		register_trace_android_vh_cpufreq_fast_switch(low_demand_idle_cpu_freq, NULL);
		register_trace_android_vh_cpu_idle_enter(low_demand_idle_enter_idle, NULL);
		register_trace_android_vh_cpu_idle_exit(low_demand_idle_exit_idle, NULL);
	} else {
		unregister_trace_android_vh_cpu_idle_enter(low_demand_idle_enter_idle, NULL);
		unregister_trace_android_vh_cpu_idle_exit(low_demand_idle_exit_idle, NULL);
		unregister_trace_android_vh_cpufreq_fast_switch(low_demand_idle_cpu_freq, NULL);
		idle_lowfreq_req = 0;
	}
	pr_info("%s: [MICAM-SCHED-LDI] enable: %d, level: %d, cores_num: %d\n", __func__, enable, level, cores_num);
}
#endif/* MI_CAM_LOW_DEMAND_IDLE_SUPPORT */

static int __init mi_camera_init(void)
{
	int rc = 0;
	struct device *device;
	struct micam_dev *micam_dev =&cam_dev;
#ifdef CONFIG_MI_CAM_WALT
	int cpu = 0;
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
		rc = sysfs_create_group(&device->kobj, &mi_cam_attr_group);
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
	register_cam_freq_hook(cam_set_next_freq);
	pr_info("%s: [MICAM_SCHED] open success!\n", __func__);
#endif
#ifdef MI_CAM_LOW_DEMAND_IDLE_SUPPORT
	init_irq_work(&lowfreq_irq_work, lowfreq_request);
	init_irq_work(&restore_freq_irq_work, lowfreq_release);
	timer_setup(&low_demand_idle_wdog, idle_exception_process, 0);
	idle_lowfreq_policy = cpufreq_cpu_get(0);
	if (!idle_lowfreq_policy) {
		pr_err("%s: cpufreq policy not found for cpu0\n",
						__func__);
	}
#endif/* MI_CAM_LOW_DEMAND_IDLE_SUPPORT */

	return 0;
}

static void __exit mi_cam_exit(void)
{
	struct micam_dev *micam_dev =&cam_dev;

#ifdef CONFIG_MI_CAM_WALT
	unregister_micam_cpu_cycles_calibration_hook();
	unregister_cam_freq_hook();
	pr_info("%s: [MICAM_SCHED] Close success!\n", __func__);
#endif
#ifdef MI_CAM_LOW_DEMAND_IDLE_SUPPORT
	if (idle_lowfreq_policy)
		cpufreq_cpu_put(idle_lowfreq_policy);
#endif/* MI_CAM_LOW_DEMAND_IDLE_SUPPORT */

	sysfs_remove_group(&micam_dev->micam_device->kobj, &mi_cam_attr_group);
	device_destroy(micam_class, micam_dev->dev);
	class_destroy(micam_class);
}

module_init(mi_camera_init);
module_exit(mi_cam_exit);
MODULE_DESCRIPTION("mi cam sched driver");
MODULE_LICENSE("GPL v2");
// END Camera_CamSched
