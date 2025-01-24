#ifndef CAM_DUMP_UTIL_H
#define CAM_DUMP_UTIL_H

#include <media/cam_req_mgr.h>
//xiaomi add
/**
 *
 */
int cam_debug_record_key_message(int subdev_id,void *ctrl,void *info,uint64_t request_id,int32_t idx,int opcode);

/**
 *
 */
void cam_init_message_buffer(void);

/**
 *
 */
void cam_deinit_message_buffer(void);

/**
 *
 */
void dump_key_msg_buffer(void);

/**
 *
 */
void dump_sensor_setting(void);

struct cam_debug_record_key_message_buffer *cam_get_key_msg_buffer(void);

struct cam_power_info_record *cam_get_power_msg_buffer(void);

struct cam_io_data_record *cam_get_io_msg_buffer(void);
//end
#endif /* CAM_DUMP_UTIL_H */