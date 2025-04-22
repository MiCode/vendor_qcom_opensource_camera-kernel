//MIUI ADD:Camera_CamSched
#undef TRACE_SYSTEM
#define TRACE_SYSTEM micamsched

#if !defined(_TRACE_MI_CAM_H_) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_MI_CAM_H_

#include <linux/types.h>
#include <linux/tracepoint.h>

TRACE_EVENT(cam_set_next_freq,
	TP_PROTO(unsigned int cpu_stall, bool pressure_enable,
		unsigned int cpu, unsigned long max_freq, unsigned long util, unsigned long curr_freq,
		unsigned long old_freq, unsigned long next_freq, bool need_update),
	TP_ARGS(cpu_stall, pressure_enable, cpu, max_freq, util, curr_freq,
		old_freq, next_freq, need_update),
	TP_STRUCT__entry(
		__field(unsigned int, cpu_stall)
		__field(bool, pressure_enable)
		__field(unsigned int, cpu)
		__field(unsigned long, max_freq)
		__field(unsigned long, util)
		__field(unsigned long, curr_freq)
		__field(unsigned long, old_freq)
		__field(unsigned long, next_freq)
		__field(bool, need_update)
	),
	TP_fast_assign(
		__entry->cpu_stall			= cpu_stall;
		__entry->pressure_enable		= pressure_enable;
		__entry->cpu				= cpu;
		__entry->max_freq			= max_freq;
		__entry->util				= util;
		__entry->curr_freq			= curr_freq;
		__entry->old_freq			= old_freq;
		__entry->next_freq			= next_freq;
		__entry->need_update			= need_update;
	),

	TP_printk(
		"cam_set_next_freq_event:cpu_stall=%d pressure_enable=%d cpu=%d max_freq=%lu util=%lu curr_freq=%lu old_freq=%lu next_freq=%lu need_update=%d",
		__entry->cpu_stall,
		__entry->pressure_enable,
		__entry->cpu,
		__entry->max_freq,
		__entry->util,
		__entry->curr_freq,
		__entry->old_freq,
		__entry->next_freq,
		__entry->need_update)
);
#endif /* _TRACE_MI_CAM_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ./mi_cam
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE mi_cam_trace
#include <trace/define_trace.h>
//Camera_CamSched END
