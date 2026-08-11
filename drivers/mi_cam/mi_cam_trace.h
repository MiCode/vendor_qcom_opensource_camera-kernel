//MIUI ADD:Camera_CamSched
#undef TRACE_SYSTEM
#define TRACE_SYSTEM micamsched

#if !defined(_TRACE_MI_CAM_H_) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_MI_CAM_H_

#include <linux/types.h>
#include <linux/tracepoint.h>

TRACE_EVENT(cam_set_next_freq,
	TP_PROTO(unsigned int cpu_stall, bool pressure_enable,
		unsigned int cpu, unsigned long max_freq, unsigned long util, unsigned long cap,
		unsigned long old_freq, unsigned long next_freq, bool need_update),
	TP_ARGS(cpu_stall, pressure_enable, cpu, max_freq, util, cap,
		old_freq, next_freq, need_update),
	TP_STRUCT__entry(
		__field(unsigned int, cpu_stall)
		__field(bool, pressure_enable)
		__field(unsigned int, cpu)
		__field(unsigned long, max_freq)
		__field(unsigned long, util)
		__field(unsigned long, cap)
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
		__entry->cap				= cap;
		__entry->old_freq			= old_freq;
		__entry->next_freq			= next_freq;
		__entry->need_update			= need_update;
	),

	TP_printk(
		"cam_set_next_freq_event:cpu_stall=%d pressure_enable=%d cpu=%d max_freq=%lu util=%lu max_cap=%lu old_freq=%lu next_freq=%lu need_update=%d",
		__entry->cpu_stall,
		__entry->pressure_enable,
		__entry->cpu,
		__entry->max_freq,
		__entry->util,
		__entry->cap,
		__entry->old_freq,
		__entry->next_freq,
		__entry->need_update)
);


TRACE_EVENT(camsched_calibrate_cpu_cycles,
	TP_PROTO(int stage, int cpu, u64 caled_cycles_delta, u64 raw_cycles_delta, u64 amu_cyc_delta, u64 mem_stall_delta, u64 mem_stall_pct),
	TP_ARGS(stage, cpu, caled_cycles_delta, raw_cycles_delta, amu_cyc_delta, mem_stall_delta, mem_stall_pct),

	TP_STRUCT__entry(
		__field(int,		stage)
		__field(int,		cpu)
		__field(u64,		caled_cycles_delta)
		__field(u64,		raw_cycles_delta)
		__field(u64,		amu_cyc_delta)
		__field(u64,		mem_stall_delta)
		__field(u64,		mem_stall_pct)
	),

	TP_fast_assign(
		__entry->stage					= stage;
		__entry->cpu					= cpu;
		__entry->caled_cycles_delta		= caled_cycles_delta;
		__entry->raw_cycles_delta		= raw_cycles_delta;
		__entry->amu_cyc_delta			= amu_cyc_delta;
		__entry->mem_stall_delta		= mem_stall_delta;
		__entry->mem_stall_pct			= mem_stall_pct;
	),

	TP_printk("stage = %d cpu=%d caled_cycles_delta=%llu raw_cycles_delta=%llu amu_cyc_delta=%llu mem_stall_delta=%llu mem_stall_pct=%llu",
            __entry->stage, __entry->cpu, __entry->caled_cycles_delta, __entry->raw_cycles_delta, __entry->amu_cyc_delta, __entry->mem_stall_delta, __entry->mem_stall_pct)
);

TRACE_EVENT(camsched_read_amu_counter,
	TP_PROTO(int cpu, u64 curr_mem_stall, u64 curr_amu_cyc, u64 mem_stall_delta, u64 amu_cyc_delta, u64 last_update),
	TP_ARGS(cpu, curr_mem_stall, curr_amu_cyc, mem_stall_delta, amu_cyc_delta, last_update),

	TP_STRUCT__entry(
		__field(int,		cpu)
		__field(u64,		curr_mem_stall)
		__field(u64,		curr_amu_cyc)
		__field(u64,		mem_stall_delta)
		__field(u64,		amu_cyc_delta)
		__field(u64,		last_update)
	),

	TP_fast_assign(
		__entry->cpu			= cpu;
		__entry->curr_mem_stall		= curr_mem_stall;
		__entry->curr_amu_cyc		= curr_amu_cyc;
		__entry->mem_stall_delta	= mem_stall_delta;
		__entry->amu_cyc_delta		= amu_cyc_delta;
		__entry->last_update		= last_update;
	),

	TP_printk("cpu = %d curr_mem_stall = %llu curr_amu_cyc=%llu mem_stall_delta=%llu amu_cyc_delta=%llu last_update=%llu",
            __entry->cpu, __entry->curr_mem_stall, __entry->curr_amu_cyc, __entry->mem_stall_delta, __entry->amu_cyc_delta, __entry->last_update)
);

#endif /* _TRACE_MI_CAM_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ./mi_cam
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE mi_cam_trace
#include <trace/define_trace.h>
//END Camera_CamSched
