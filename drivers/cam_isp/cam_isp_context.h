/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_ISP_CONTEXT_H_
#define _CAM_ISP_CONTEXT_H_


#include <linux/spinlock_types.h>
#include <media/cam_isp.h>
#include <media/cam_defs.h>
#include <media/cam_tfe.h>

#include "cam_context.h"
#include "cam_isp_hw_mgr_intf.h"
#include "cam_req_mgr_workq.h"

#define CAM_IFE_QTIMER_MUL_FACTOR        10000
#define CAM_IFE_QTIMER_DIV_FACTOR        192

/*
 * Maximum hw resource - This number is based on the maximum
 * output port resource. The current maximum resource number
 * is 24.
 */
#define CAM_ISP_CTX_RES_MAX                     24

/* max requests per ctx for isp */
#define CAM_ISP_CTX_REQ_MAX                     8

/*
 * Maximum entries in state monitoring array for error logging
 */
#define CAM_ISP_CTX_STATE_MONITOR_MAX_ENTRIES   84

/*
 * Threshold response time in us beyond which a request is not expected
 * to be with IFE hw
 */
#define CAM_ISP_CTX_RESPONSE_TIME_THRESHOLD   100000

/* Number of words for dumping isp context */
#define CAM_ISP_CTX_DUMP_NUM_WORDS  5

/* Number of words for dumping isp context events*/
#define CAM_ISP_CTX_DUMP_EVENT_NUM_WORDS  3

/* Number of words for dumping request info*/
#define CAM_ISP_CTX_DUMP_REQUEST_NUM_WORDS  2

/* Maximum entries in event record */
#define CAM_ISP_CTX_EVENT_RECORD_MAX_ENTRIES   8

/* Maximum length of tag while dumping */
#define CAM_ISP_CONTEXT_DUMP_TAG_MAX_LEN 128

/* AEB error count threshold */
#define CAM_ISP_CONTEXT_AEB_ERROR_CNT_MAX 6

/* Debug Buffer length*/
#define CAM_ISP_CONTEXT_DBG_BUF_LEN 1000

/* AFD pipeline delay for FCG configuration */
#define CAM_ISP_AFD_PIPELINE_DELAY 3

/* Maximum entries in frame record */
#define CAM_ISP_CTX_MAX_FRAME_RECORDS  5

/* Congestion count threshold */
#define CAM_ISP_CONTEXT_CONGESTION_CNT_MAX 3

/* forward declaration */
struct cam_isp_context;

/* cam isp context irq handling function type */
typedef int (*cam_isp_hw_event_cb_func)(struct cam_isp_context *ctx_isp,
	void *evt_data);

/**
 * enum cam_isp_ctx_activated_substate - sub states for activated
 *
 */
enum cam_isp_ctx_activated_substate {
	CAM_ISP_CTX_ACTIVATED_SOF,
	CAM_ISP_CTX_ACTIVATED_APPLIED,
	CAM_ISP_CTX_ACTIVATED_EPOCH,
	CAM_ISP_CTX_ACTIVATED_BUBBLE,
	CAM_ISP_CTX_ACTIVATED_BUBBLE_APPLIED,
	CAM_ISP_CTX_ACTIVATED_HW_ERROR,
	CAM_ISP_CTX_ACTIVATED_HALT,
	CAM_ISP_CTX_ACTIVATED_MAX,
};

/**
 * enum cam_isp_ctx_event_type - events for a request
 *
 */
enum cam_isp_ctx_event {
	CAM_ISP_CTX_EVENT_SUBMIT,
	CAM_ISP_CTX_EVENT_APPLY,
	CAM_ISP_CTX_EVENT_EPOCH,
	CAM_ISP_CTX_EVENT_RUP,
	CAM_ISP_CTX_EVENT_BUFDONE,
	CAM_ISP_CTX_EVENT_SHUTTER,
	CAM_ISP_CTX_EVENT_MAX
};

/**
 * enum cam_isp_state_change_trigger - Different types of ISP events
 *
 */
enum cam_isp_state_change_trigger {
	CAM_ISP_STATE_CHANGE_TRIGGER_ERROR,
	CAM_ISP_STATE_CHANGE_TRIGGER_APPLIED,
	CAM_ISP_STATE_CHANGE_TRIGGER_REG_UPDATE,
	CAM_ISP_STATE_CHANGE_TRIGGER_SOF,
	CAM_ISP_STATE_CHANGE_TRIGGER_EPOCH,
	CAM_ISP_STATE_CHANGE_TRIGGER_DONE,
	CAM_ISP_STATE_CHANGE_TRIGGER_EOF,
	CAM_ISP_STATE_CHANGE_TRIGGER_FLUSH,
	CAM_ISP_STATE_CHANGE_TRIGGER_SEC_EVT_SOF,
	CAM_ISP_STATE_CHANGE_TRIGGER_SEC_EVT_EPOCH,
	CAM_ISP_STATE_CHANGE_TRIGGER_FRAME_DROP,
	CAM_ISP_STATE_CHANGE_TRIGGER_CDM_DONE,
	CAM_ISP_STATE_CHANGE_TRIGGER_MAX
};

#define CAM_ISP_CTX_DISABLE_RECOVERY_AEB           BIT(0)
#define CAM_ISP_CTX_DISABLE_RECOVERY_BUS_OVERFLOW  BIT(1)
#define CAM_ISP_CTX_DISABLE_RECOVERY_BUBBLE        BIT(2)

/**
 * struct cam_isp_ctx_debug -  Contains debug parameters
 *
 * @dentry:                     Debugfs entry
 * @enable_state_monitor_dump:  Enable isp state monitor dump
 * @enable_cdm_cmd_buff_dump:   Enable CDM Command buffer dump
 * @disable_internal_recovery:  Disable internal kernel recovery mask
 *
 */
struct cam_isp_ctx_debug {
	struct dentry  *dentry;
	uint32_t        enable_state_monitor_dump;
	uint8_t         enable_cdm_cmd_buff_dump;
	uint32_t        disable_internal_recovery_mask;
};

/**
 * struct cam_isp_ctx_irq_ops - Function table for handling IRQ callbacks
 *
 * @irq_ops:               Array of handle function pointers.
 *
 */
struct cam_isp_ctx_irq_ops {
	cam_isp_hw_event_cb_func         irq_ops[CAM_ISP_HW_EVENT_MAX];
};

/**
 * struct cam_isp_ctx_req - ISP context request object
 *
 * @base:                      Common request object ponter
 * @cfg:                       ISP hardware configuration array
 * @num_cfg:                   Number of ISP hardware configuration entries
 * @fence_map_out:             Output fence mapping array
 * @num_fence_map_out:         Number of the output fence map
 * @fence_map_in:              Input fence mapping array
 * @num_fence_map_in:          Number of input fence map
 * @num_acked:                 Count to track acked entried for output.
 *                             If count equals the number of fence out, it means
 *                             the request has been completed.
 * @num_deferred_acks:         Number of buf_dones/acks that are deferred to
 *                             handle or signalled in special scenarios.
 *                             Increment this count instead of num_acked and
 *                             handle the events later where eventually
 *                             increment num_acked.
 * @deferred_fence_map_index   Saves the indices of fence_map_out for which
 *                             handling of buf_done is deferred.
 * @bubble_report:             Flag to track if bubble report is active on
 *                             current request
 * @hw_update_data:            HW update data for this request
 * @reapply_type:              Determines type of settings to be re-applied
 * @event_timestamp:           Timestamp for different stage of request
 * @cdm_reset_before_apply:    For bubble re-apply when buf done not coming set
 *                             to True
 *
 */
struct cam_isp_ctx_req {
	struct cam_ctx_request               *base;
	struct cam_hw_update_entry           *cfg;
	uint32_t                              num_cfg;
	struct cam_hw_fence_map_entry        *fence_map_out;
	uint32_t                              num_fence_map_out;
	struct cam_hw_fence_map_entry        *fence_map_in;
	uint32_t                              num_fence_map_in;
	uint32_t                              num_acked;
	uint32_t                              num_deferred_acks;
	uint32_t                  deferred_fence_map_index[CAM_ISP_CTX_RES_MAX];
	int32_t                               bubble_report;
	struct cam_isp_prepare_hw_update_data hw_update_data;
	enum cam_hw_config_reapply_type       reapply_type;
	ktime_t                               event_timestamp
		[CAM_ISP_CTX_EVENT_MAX];
	bool                                  bubble_detected;
	bool                                  cdm_reset_before_apply;
};

/**
 * struct cam_isp_context_state_monitor - ISP context state
 *                                        monitoring for
 *                                        debug purposes
 *
 * @curr_state:          Current sub state that received req
 * @trigger:             Event type of incoming req
 * @req_id:              Request id
 * @frame_id:            Frame id based on SOFs
 * @evt_time_stamp       Current time stamp
 *
 */
struct cam_isp_context_state_monitor {
	enum cam_isp_ctx_activated_substate  curr_state;
	enum cam_isp_state_change_trigger    trigger;
	uint64_t                             req_id;
	int64_t                              frame_id;
	struct timespec64                    evt_time_stamp;
};

/**
 * struct cam_isp_context_req_id_info - ISP context request id
 *                     information for bufdone.
 *
 *@last_bufdone_req_id:   Last bufdone request id
 *
 */

struct cam_isp_context_req_id_info {
	int64_t                          last_bufdone_req_id;
};

struct shutter_event {
	uint64_t frame_id;
	uint64_t req_id;
	uint32_t status;
	ktime_t  boot_ts;
	ktime_t  sof_ts;
};

/**
 *
 *
 * struct cam_isp_context_event_record - Information for last 20 Events
 *  for a request; Submit, Apply, EPOCH, RUP, Buf done.
 *
 * @req_id:    Last applied request id
 * @timestamp: Timestamp for the event
 *
 */
struct cam_isp_context_event_record {
	uint64_t req_id;
	ktime_t  timestamp;
	int event_type;
	union event {
		struct shutter_event shutter_event;
	} event;
};

/**
 *
 *
 * struct cam_isp_context_frame_timing_record - Frame timing events
 *
 * @sof_ts:           SOF timestamp
 * @eof_ts:           EOF ts
 * @epoch_ts:         EPOCH ts
 * @secondary_sof_ts: Secondary SOF ts
 *
 */
struct cam_isp_context_frame_timing_record {
	struct timespec64 sof_ts;
	struct timespec64 eof_ts;
	struct timespec64 epoch_ts;
	struct timespec64 secondary_sof_ts;
};


/**
 *
 *
 * struct cam_isp_context_debug_monitors - ISP context debug monitors
 *
 * @state_monitor_head:        State machine monitor head
 * @state_monitor:             State machine monitor info
 * @event_record_head:         Request Event monitor head
 * @event_record:              Request event monitor info
 * @frame_monitor_head:        Frame timing monitor head
 * @frame_monitor:             Frame timing event monitor
 *
 */
struct cam_isp_context_debug_monitors {
	/* State machine monitoring */
	atomic64_t                           state_monitor_head;
	struct cam_isp_context_state_monitor state_monitor[
		CAM_ISP_CTX_STATE_MONITOR_MAX_ENTRIES];

	/* Req event monitor */
	atomic64_t                            event_record_head[
		CAM_ISP_CTX_EVENT_MAX];
	struct cam_isp_context_event_record   event_record[
		CAM_ISP_CTX_EVENT_MAX][CAM_ISP_CTX_EVENT_RECORD_MAX_ENTRIES];

	/* Frame timing monitor */
	atomic64_t                            frame_monitor_head;
	struct cam_isp_context_frame_timing_record frame_monitor[
		CAM_ISP_CTX_MAX_FRAME_RECORDS];
};

/**
 * struct cam_isp_skip_frame_info - FIFO Queue for number of skipped frames for
 *                                  the decision of FCG prediction
 * @num_frame_skipped:              Keep track of the number of skipped frames in between
 *                                  of the normal frames
 * @list:                           List member used to append this node to a linked list
 */
struct cam_isp_skip_frame_info {
	uint32_t                         num_frame_skipped;
	struct list_head                 list;
};

/**
 * struct cam_isp_fcg_prediction_tracker - Track the number of skipped frames before and
 *                                         indicate which FCG prediction should be applied
 *
 * @num_skipped:               Number of skipped frames from previous normally applied frame
 *                             to this normally applied frame
 * @sum_skipped:               Sum of the number of frames from req generation to req apply
 * @skipped_list:              Keep track of the number of skipped frames in between from two
 *                             normal frames
 */
struct cam_isp_fcg_prediction_tracker {
	struct cam_isp_fcg_caps              *fcg_caps;
	uint32_t                              num_skipped;
	uint32_t                              sum_skipped;
	struct list_head                      skipped_list;
};

/**
 * struct cam_isp_context   -  ISP context object
 *
 * @base:                      Common context object pointer
 * @frame_id:                  Frame id tracking for the isp context
 * @frame_id_meta:             Frame id read every epoch for the ctx
 *                             meta from the sensor
 * @substate_actiavted:        Current substate for the activated state.
 * @process_bubble:            Atomic variable to check if ctx is still
 *                             processing bubble.
 * @substate_machine:          ISP substate machine for external interface
 * @substate_machine_irq:      ISP substate machine for irq handling
 * @req_base:                  Common request object storage
 * @req_isp:                   ISP private request object storage
 * @hw_ctx:                    HW object returned by the acquire device command
 * @sof_timestamp_val:         Captured time stamp value at sof hw event
 * @boot_timestamp:            Boot time stamp for a given req_id
 * @active_req_cnt:            Counter for the active request
 * @reported_req_id:           Last reported request id
 * @subscribe_event:           The irq event mask that CRM subscribes to, IFE
 *                             will invoke CRM cb at those event.
 * @last_applied_req_id:       Last applied request id
 * @recovery_req_id:           Req ID flagged for internal recovery
 * @last_sof_timestamp:        SOF timestamp of the last frame
 * @bubble_frame_cnt:          Count of the frame after bubble
 * @aeb_error_cnt:             Count number of times a specific AEB error scenario is
 *                             enountered
 * @out_of_sync_cnt:           Out of sync error count for AEB
 * @congestion_cnt:            Count number of times congestion was encountered
 *                             consecutively
 * @state_monitor_head:        Write index to the state monitoring array
 * @req_info                   Request id information about last buf done
 * @dbg_monitors:              Debug monitors for ISP context
 * @apply_in_progress          Whether request apply is in progress
 * @init_timestamp:            Timestamp at which this context is initialized
 * @isp_device_type:           ISP device type
 * @rxd_epoch:                 Indicate whether epoch has been received. Used to
 *                             decide whether to apply request in offline ctx
 * @workq:                     Worker thread for offline ife
 * @trigger_id:                ID provided by CRM for each ctx on the link
 * @last_bufdone_err_apply_req_id:  last bufdone error apply request id
 * @v4l2_event_sub_ids         contains individual bits representing subscribed v4l2 ids
 * @evt_inject_params:         event injection parameters
 * @last_sof_jiffies:          Record the jiffies of last sof
 * @last_applied_jiffies:      Record the jiffiest of last applied req
 * @vfe_bus_comp_grp:          Vfe bus comp group record
 * @sfe_bus_comp_grp:          Sfe bus comp group record
 * @mswitch_default_apply_delay_max_cnt: Max mode switch delay among all devices connected
 *                                       on the same link as this ISP context
 * @mswitch_default_apply_delay_ref_cnt: Ref cnt for this context to decide when to apply
 *                                       mode switch settings
 * @hw_idx:                    Hardware ID
 * @fcg_tracker:               FCG prediction tracker containing number of previously skipped
 *                             frames and indicates which prediction should be used
 * @rdi_only_context:          Get context type information.
 *                             true, if context is rdi only context
 * @offline_context:           Indicate whether context is for offline IFE
 * @vfps_aux_context:          Indicate whether context is for VFPS aux link
 * @resume_hw_in_flushed:      Indicate whether resume hw in flushed state in vfps case
 * @hw_acquired:               Indicate whether HW resources are acquired
 * @init_received:             Indicate whether init config packet is received
 * @split_acquire:             Indicate whether a separate acquire is expected
 * @custom_enabled:            Custom HW enabled for this ctx
 * @use_frame_header_ts:       Use frame header for qtimer ts
 * @support_consumed_addr:     Indicate whether HW has last consumed addr reg
 * @sof_dbg_irq_en:            Indicates whether ISP context has enabled debug irqs
 * @use_default_apply:         Use default settings in case of frame skip
 * @aeb_enabled:               Indicate if stream is for AEB
 * @bubble_recover_dis:        Bubble recovery disabled
 * @handle_mswitch:            Indicates if IFE needs to explicitly handle mode switch
 *                             on frame skip callback from request manager.
 *                             This is decided based on the max mode switch delay published
 *                             by other devices on the link as part of link setup
 * @mode_switch_en:            Indicates if mode switch is enabled
 * @sfe_en:                    Indicates if SFE is being used
 *
 */
struct cam_isp_context {
	struct cam_context              *base;

	uint64_t                         frame_id;
	uint32_t                         frame_id_meta;
	uint32_t                         substate_activated;
	atomic_t                         process_bubble;
	struct cam_ctx_ops              *substate_machine;
	struct cam_isp_ctx_irq_ops      *substate_machine_irq;

	struct cam_ctx_request           req_base[CAM_ISP_CTX_REQ_MAX];
	struct cam_isp_ctx_req           req_isp[CAM_ISP_CTX_REQ_MAX];

	void                            *hw_ctx;
	uint64_t                         sof_timestamp_val;
	uint64_t                         boot_timestamp;
	int32_t                          active_req_cnt;
	int64_t                          reported_req_id;
	uint64_t                         reported_frame_id;
	uint32_t                         subscribe_event;
	int64_t                          last_applied_req_id;
	uint64_t                         recovery_req_id;
	uint64_t                         last_sof_timestamp;
	uint32_t                         bubble_frame_cnt;
	uint32_t                         aeb_error_cnt;
	uint32_t                         out_of_sync_cnt;
	uint32_t                         congestion_cnt;
	struct cam_isp_context_req_id_info    req_info;
	struct cam_isp_context_debug_monitors dbg_monitors;
	atomic_t                              apply_in_progress;
	atomic_t                              internal_recovery_set;
	unsigned int                          init_timestamp;
	uint32_t                              isp_device_type;
	atomic_t                              rxd_epoch;
	struct cam_req_mgr_core_workq        *workq;
	int32_t                               trigger_id;
	int64_t                               last_bufdone_err_apply_req_id;
	uint32_t                              v4l2_event_sub_ids;
	struct cam_hw_inject_evt_param        evt_inject_params;
	uint64_t                              last_sof_jiffies;
	uint64_t                              last_applied_jiffies;
	struct cam_isp_context_comp_record   *vfe_bus_comp_grp;
	struct cam_isp_context_comp_record   *sfe_bus_comp_grp;
	int32_t                               mswitch_default_apply_delay_max_cnt;
	atomic_t                              mswitch_default_apply_delay_ref_cnt;
	uint32_t                              hw_idx;
	struct cam_isp_fcg_prediction_tracker fcg_tracker;
	bool                                  rdi_only_context;
	bool                                  offline_context;
	bool                                  vfps_aux_context;
	bool                                  resume_hw_in_flushed;
	bool                                  hw_acquired;
	bool                                  init_received;
	bool                                  split_acquire;
	bool                                  custom_enabled;
	bool                                  use_frame_header_ts;
	bool                                  support_consumed_addr;
	bool                                  sof_dbg_irq_en;
	bool                                  use_default_apply;
	bool                                  aeb_enabled;
	bool                                  bubble_recover_dis;
	bool                                  handle_mswitch;
	bool                                  mode_switch_en;
	bool                                  sfe_en;
};

/**
 * struct cam_isp_context_dump_header - ISP context dump header
 * @tag:       Tag name for the header
 * @word_size: Size of word
 * @size:      Size of data
 *
 */
struct cam_isp_context_dump_header {
	uint8_t   tag[CAM_ISP_CONTEXT_DUMP_TAG_MAX_LEN];
	uint64_t  size;
	uint32_t  word_size;
};

/** * struct cam_isp_ctx_req_mini_dump - ISP mini dumprequest
 *
 * @map_out:                   Output fence mapping
 * @map_in:                    Input fence mapping
 * @io_cfg:                    IO buffer configuration
 * @reapply_type:              Determines type of settings to be re-applied
 * @request_id:                Request ID
 * @num_fence_map_out:         Number of the output fence map
 * @num_fence_map_in:          Number of input fence map
 * @num_io_cfg:                Number of ISP hardware configuration entries
 * @num_acked:                 Count to track acked entried for output.
 * @num_deferred_acks:         Number of buf_dones/acks that are deferred to
 *                             handle or signalled in special scenarios.
 *                             Increment this count instead of num_acked and
 *                             handle the events later where eventually
 *                             increment num_acked.
 * @bubble_report:             Flag to track if bubble report is active on
 *                             current request
 * @bubble_detected:           Flag to track if bubble is detected
 * @cdm_reset_before_apply:    For bubble re-apply when buf done not coming set
 *                             to True
 *
 */
struct cam_isp_ctx_req_mini_dump {
	struct cam_hw_fence_map_entry   *map_out;
	struct cam_hw_fence_map_entry   *map_in;
	struct cam_buf_io_cfg           *io_cfg;
	enum cam_hw_config_reapply_type  reapply_type;
	uint64_t                         request_id;
	uint8_t                          num_fence_map_in;
	uint8_t                          num_fence_map_out;
	uint8_t                          num_io_cfg;
	uint8_t                          num_acked;
	uint8_t                          num_deferred_acks;
	bool                             bubble_report;
	bool                             bubble_detected;
	bool                             cdm_reset_before_apply;
};

/**
 * struct cam_isp_ctx_mini_dump_info - Isp context mini dump data
 *
 * @active_list:               Active Req list
 * @pending_list:              Pending req list
 * @wait_list:                 Wait Req List
 * @event_record:              Event record
 * @sof_timestamp_val:         Captured time stamp value at sof hw event
 * @boot_timestamp:            Boot time stamp for a given req_id
 * @last_sof_timestamp:        SOF timestamp of the last frame
 * @init_timestamp:            Timestamp at which this context is initialized
 * @frame_id:                  Frame id read every epoch for the ctx
 * @reported_req_id:           Last reported request id
 * @last_applied_req_id:       Last applied request id
 * @frame_id_meta:             Frame id for meta
 * @ctx_id:                    Context id
 * @subscribe_event:           The irq event mask that CRM subscribes to, IFE
 *                             will invoke CRM cb at those event.
 * @bubble_frame_cnt:          Count of the frame after bubble
 * @isp_device_type:           ISP device type
 * @active_req_cnt:            Counter for the active request
 * @trigger_id:                ID provided by CRM for each ctx on the link
 * @substate_actiavted:        Current substate for the activated state.
 * @rxd_epoch:                 Indicate whether epoch has been received. Used to
 *                             decide whether to apply request in offline ctx
 * @process_bubble:            Atomic variable to check if ctx is still
 *                             processing bubble.
 * @apply_in_progress          Whether request apply is in progress
 * @rdi_only_context:          Get context type information.
 *                             true, if context is rdi only context
 * @offline_context:           Indicate whether context is for offline IFE
 * @hw_acquired:               Indicate whether HW resources are acquired
 * @init_received:             Indicate whether init config packet is received
 *                             meta from the sensor
 * @split_acquire:             Indicate whether a separate acquire is expected
 * @custom_enabled:            Custom HW enabled for this ctx
 * @use_frame_header_ts:       Use frame header for qtimer ts
 * @support_consumed_addr:     Indicate whether HW has last consumed addr reg
 * @use_default_apply:         Use default settings in case of frame skip
 *
 */
struct cam_isp_ctx_mini_dump_info {
	struct cam_isp_ctx_req_mini_dump      *active_list;
	struct cam_isp_ctx_req_mini_dump      *pending_list;
	struct cam_isp_ctx_req_mini_dump      *wait_list;
	struct cam_isp_context_event_record    event_record[
		CAM_ISP_CTX_EVENT_MAX][CAM_ISP_CTX_EVENT_RECORD_MAX_ENTRIES];
	uint64_t                               sof_timestamp_val;
	uint64_t                               boot_timestamp;
	uint64_t                               last_sof_timestamp;
	uint64_t                               init_timestamp;
	int64_t                                frame_id;
	int64_t                                reported_req_id;
	int64_t                                last_applied_req_id;
	int64_t                                last_bufdone_err_apply_req_id;
	uint32_t                               frame_id_meta;
	uint8_t                                ctx_id;
	uint8_t                                subscribe_event;
	uint8_t                                bubble_frame_cnt;
	uint8_t                                isp_device_type;
	uint8_t                                active_req_cnt;
	uint8_t                                trigger_id;
	uint8_t                                substate_activated;
	uint8_t                                rxd_epoch;
	uint8_t                                process_bubble;
	uint8_t                                active_cnt;
	uint8_t                                pending_cnt;
	uint8_t                                wait_cnt;
	bool                                   apply_in_progress;
	bool                                   rdi_only_context;
	bool                                   offline_context;
	bool                                   hw_acquired;
	bool                                   init_received;
	bool                                   split_acquire;
	bool                                   custom_enabled;
	bool                                   use_frame_header_ts;
	bool                                   support_consumed_addr;
	bool                                   use_default_apply;
};

/**
 * cam_isp_context_init()
 *
 * @brief:              Initialization function for the ISP context
 *
 * @ctx:                ISP context obj to be initialized
 * @bridge_ops:         Bridge call back funciton
 * @hw_intf:            ISP hw manager interface
 * @ctx_id:             ID for this context
 * @isp_device_type     Isp device type
 * @img_iommu_hdl       IOMMU HDL for image buffers
 *
 */
int cam_isp_context_init(struct cam_isp_context *ctx,
	struct cam_context *ctx_base,
	struct cam_req_mgr_kmd_ops *bridge_ops,
	struct cam_hw_mgr_intf *hw_intf,
	uint32_t ctx_id,
	uint32_t isp_device_type,
	int img_iommu_hdl);

/**
 * cam_isp_context_deinit()
 *
 * @brief:               Deinitialize function for the ISP context
 *
 * @ctx:                 ISP context obj to be deinitialized
 *
 */
int cam_isp_context_deinit(struct cam_isp_context *ctx);

#endif  /* __CAM_ISP_CONTEXT_H__ */
