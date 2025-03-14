#include "cam_isp_context.h"
#include "cam_sensor_dev.h"
#include "cam_actuator_dev.h"
#include "cam_eeprom_dev.h"
#include "cam_ois_dev.h"
#include "cam_dump_util.h"
#include <media/cam_req_mgr.h>

#define INC_HEAD(head, max_entries, ret) \
	div_u64_rem((++head),\
	max_entries, (ret))

struct cam_debug_record_key_message_buffer *cam_key_buffer;
struct cam_io_data_record                  *cam_io_record_buffer;
struct cam_power_info_record               *cam_power_record_buffer;

int get_subdev_id(struct cam_hw_soc_info *soc_info)
{
	if(NULL != strstr(soc_info->dev_name,"sensor"))
	{
		return CAM_SENSOR;
	}
	else if(NULL != strstr(soc_info->dev_name,"eeprom"))
	{
		return CAM_EEPROM;
	}
	else if(NULL != strstr(soc_info->dev_name,"actuator"))
	{
		return CAM_ACTUATOR;
	}
	else if(NULL != strstr(soc_info->dev_name,"ois"))
	{
		return CAM_OIS;
	}
	return CAM_DBG_MOD_MAX;
}

char *get_subdev_name(int subdev_id)
{
	switch(subdev_id)
	{
		case CAM_SENSOR:
			return "sensor";
			break;
		case CAM_EEPROM:
			return "eeprom";
			break;
		case CAM_ACTUATOR:
			return "actuator";
			break;
		case CAM_OIS:
			return "ois";
			break;
		case CAM_ISP:
			return "sensor";
			break;
		default:
			return "unknown subdev";
			break;
	}
	return "";
}

char *get_opcode_name(int opcode)
{
	switch(opcode)
	{
		case EVENT_ISP_EOF:
			return "record_eof_req";
			break;
		case EVENT_ISP_EPOCH:
			return "record_epoch_req";
			break;
		case EVENT_ISP_SOF:
			return "record_sof_req";
			break;
		case EVENT_ADD_REQ:
			return "record_add_req";
			break;
		case EVENT_APPLY_REQ:
			return "record_apply_req";
			break;
		case EVENT_POWER_UP:
			return "record_power_up";
			break;
		case EVENT_POWER_DOWN:
			return "record_power_down";
			break;
		case EVENT_IO:
			return "record_apply_setting_start";
			break;
		case EVENT_IO_END:
			return "record_apply_setting end";
			break;
		default:
			return "unknown subdev";
			break;
	}
	return "";
}

int cam_debug_record_key_message(int subdev_id,void *ctrl,void *info,uint64_t request_id,int32_t idx,int opcode)
{
	int                                 rc = 0;
	long                                microsec = 0;
	uint64_t                            req_id = request_id;
	uint32_t                            min_volt,max_volt;
	uint32_t                            index,IO_index;
	struct timespec64                   ts,debug_ts;
	struct cam_sensor_power_ctrl_t      *power_ctrl = NULL;
	struct cam_isp_context              *ctx_isp = NULL;
	struct cam_context                  *ctx = NULL;
	struct cam_req_mgr_add_request      *add_request = NULL;
	struct i2c_settings_list            *i2c_set = NULL;
	struct cam_sensor_power_setting     *power_setting = NULL;
	struct cam_hw_soc_info              *soc_info = NULL;
	struct cam_req_mgr_apply_request    *apply = NULL;
	struct cam_req_mgr_core_link        *link = NULL;
	struct cam_req_mgr_connected_device *dev = NULL;
	struct cam_sensor_ctrl_t            *s_ctrl = NULL;

	CAM_DBG(CAM_UTIL,"device: %s operation: %s,seq_id: %llu",get_subdev_name(subdev_id),get_opcode_name(opcode),request_id);
	if(cam_key_buffer == NULL||cam_io_record_buffer == NULL||cam_power_record_buffer == NULL)
	{
		return 0;
	}
	CAM_GET_TIMESTAMP(ts);
	if(subdev_id == CAM_ISP)
	{
		ctx = (struct cam_context*)ctrl;
		if(NULL == ctx)
		{
			CAM_ERR(CAM_UTIL,"ctx is NULL");
			return 0;
		}
		if(opcode == EVENT_ISP_SOF)
		{
			ctx_isp = (struct cam_isp_context*)ctx->ctx_priv;
			if(ctx_isp == NULL)
			{
				CAM_ERR(CAM_UTIL,"ctx_isp is NULL");
				return 0;
			}
			link = cam_get_link_priv(ctx->link_hdl);
			if(link == NULL)
			{
				CAM_ERR(CAM_UTIL,"link_handle is NULL");
				return 0;
			}
			for(int i = 0;i < link->num_devs;++i)
			{
				dev = &link->l_dev[i];
				if(dev->dev_info.dev_id == CAM_REQ_MGR_DEVICE_SENSOR) break;
			}
			if(dev == NULL || dev->dev_info.dev_id != CAM_REQ_MGR_DEVICE_SENSOR)
			{
				CAM_ERR(CAM_UTIL,"dev info Not valid");
				return 0;
			}
			s_ctrl = (struct cam_sensor_ctrl_t*)cam_get_device_priv(dev->dev_hdl);
			soc_info = &s_ctrl->soc_info;
			INC_HEAD(cam_key_buffer->event_record_head[EVENT_ISP_SOF],MAX_RECORD_REQ,&index);
			cam_key_buffer->event_id[EVENT_ISP_SOF][index].opcode = EVENT_ISP_SOF;
			if(soc_info)
			{
				strlcpy(cam_key_buffer->event_id[EVENT_ISP_SOF][index].u.frame_info.dev_name,soc_info->dev_name,CAM_SENSOR_NAME_MAX_LENGTH);
			}
			cam_key_buffer->event_id[EVENT_ISP_SOF][index].u.frame_info.frame_id = ctx_isp->frame_id;
			CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_key_buffer->event_id[EVENT_ISP_SOF][index].ts_h,
											cam_key_buffer->event_id[EVENT_ISP_SOF][index].ts_m,
											cam_key_buffer->event_id[EVENT_ISP_SOF][index].ts_s,
											cam_key_buffer->event_id[EVENT_ISP_SOF][index].ts_ms);
			cam_key_buffer->event_id[EVENT_ISP_SOF][index].req_id = req_id;
		}
		else if(opcode == EVENT_ISP_EOF)
		{
			INC_HEAD(cam_key_buffer->event_record_head[EVENT_ISP_EOF],MAX_RECORD_REQ,&index);
			cam_key_buffer->event_id[EVENT_ISP_EOF][index].opcode = EVENT_ISP_EOF;
			CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_key_buffer->event_id[EVENT_ISP_EOF][index].ts_h,
											cam_key_buffer->event_id[EVENT_ISP_EOF][index].ts_m,
											cam_key_buffer->event_id[EVENT_ISP_EOF][index].ts_s,
											cam_key_buffer->event_id[EVENT_ISP_EOF][index].ts_ms);
			ctx_isp = (struct cam_isp_context*)ctx->ctx_priv;
			if(ctx_isp == NULL)
			{
				CAM_ERR(CAM_UTIL,"ctx_isp is NULL");
				return 0;
			}
			link = cam_get_link_priv(ctx->link_hdl);
			if(link == NULL)
			{
				CAM_ERR(CAM_UTIL,"link_handle is NULL");
				return 0;
			}
			for(int i = 0;i < link->num_devs;++i)
			{
				dev = &link->l_dev[i];
				if(dev->dev_info.dev_id == CAM_REQ_MGR_DEVICE_SENSOR) break;
			}
			if(dev == NULL || dev->dev_info.dev_id != CAM_REQ_MGR_DEVICE_SENSOR)
			{
				CAM_ERR(CAM_UTIL,"dev info Not valid");
				return 0;
			}
			s_ctrl = (struct cam_sensor_ctrl_t*)cam_get_device_priv(dev->dev_hdl);
			soc_info = &s_ctrl->soc_info;
			if(soc_info)
			{
				strlcpy(cam_key_buffer->event_id[EVENT_ISP_EOF][index].u.frame_info.dev_name,soc_info->dev_name,CAM_SENSOR_NAME_MAX_LENGTH);
			}
			cam_key_buffer->event_id[EVENT_ISP_EOF][index].u.frame_info.frame_id = ctx_isp->frame_id;
		}
		else if(opcode == EVENT_ISP_EPOCH)
		{
			INC_HEAD(cam_key_buffer->event_record_head[EVENT_ISP_EPOCH],MAX_RECORD_REQ,&index);
			cam_key_buffer->event_id[EVENT_ISP_EPOCH][index].opcode = EVENT_ISP_EPOCH;
			CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_key_buffer->event_id[EVENT_ISP_EPOCH][index].ts_h,
											cam_key_buffer->event_id[EVENT_ISP_EPOCH][index].ts_m,
											cam_key_buffer->event_id[EVENT_ISP_EPOCH][index].ts_s,
											cam_key_buffer->event_id[EVENT_ISP_EPOCH][index].ts_ms);
			ctx_isp = (struct cam_isp_context*)ctx->ctx_priv;
			if(ctx_isp == NULL)
			{
				CAM_ERR(CAM_UTIL,"ctx_isp is NULL");
				return 0;
			}
			link = cam_get_link_priv(ctx->link_hdl);
			if(link == NULL)
			{
				CAM_ERR(CAM_UTIL,"link_handle is NULL");
				return 0;
			}
			for(int i = 0;i < link->num_devs;++i)
			{
				dev = &link->l_dev[i];
				if(dev->dev_info.dev_id == CAM_REQ_MGR_DEVICE_SENSOR) break;
			}
			if(dev == NULL || dev->dev_info.dev_id != CAM_REQ_MGR_DEVICE_SENSOR)
			{
				CAM_ERR(CAM_UTIL,"dev info Not valid");
				return 0;
			}
			s_ctrl = (struct cam_sensor_ctrl_t*)cam_get_device_priv(dev->dev_hdl);
			soc_info = &s_ctrl->soc_info;
			if(soc_info)
			{
				strlcpy(cam_key_buffer->event_id[EVENT_ISP_EPOCH][index].u.frame_info.dev_name,soc_info->dev_name,CAM_SENSOR_NAME_MAX_LENGTH);
			}
			cam_key_buffer->event_id[EVENT_ISP_EPOCH][index].u.frame_info.frame_id = ctx_isp->frame_id;
		}
		else if(opcode == EVENT_ADD_REQ)
		{
			link = cam_get_link_priv(ctx->link_hdl);
			for(int i = 0;i < link->num_devs;++i)
			{
				dev = &link->l_dev[i];
				if(dev->dev_info.dev_id == CAM_REQ_MGR_DEVICE_SENSOR) break;
			}
			s_ctrl = (struct cam_sensor_ctrl_t*)cam_get_device_priv(dev->dev_hdl);
			soc_info = &s_ctrl->soc_info;
			INC_HEAD(cam_key_buffer->event_record_head[EVENT_ADD_REQ],MAX_RECORD_REQ,&index);
			cam_key_buffer->event_id[EVENT_ADD_REQ][index].opcode = EVENT_ADD_REQ;
			CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_key_buffer->event_id[EVENT_ADD_REQ][index].ts_h,
											cam_key_buffer->event_id[EVENT_ADD_REQ][index].ts_m,
											cam_key_buffer->event_id[EVENT_ADD_REQ][index].ts_s,
											cam_key_buffer->event_id[EVENT_ADD_REQ][index].ts_ms);
			add_request = (struct cam_req_mgr_add_request *)info;
			cam_key_buffer->event_id[EVENT_ADD_REQ][index].req_id = add_request->req_id;
			cam_key_buffer->event_id[EVENT_ADD_REQ][index].u.add_req.module_id = soc_info->index;
			cam_key_buffer->event_id[EVENT_ADD_REQ][index].u.add_req.subdev_id = subdev_id;
			cam_key_buffer->event_id[EVENT_ADD_REQ][index].u.add_req.dev_hdl   = add_request->dev_hdl;
			cam_key_buffer->event_id[EVENT_ADD_REQ][index].u.add_req.link_hdl  = add_request->link_hdl;
		}
		else if(opcode == EVENT_APPLY_REQ)
		{
			link = cam_get_link_priv(ctx->link_hdl);
			for(int i = 0;i < link->num_devs;++i)
			{
				dev = &link->l_dev[i];
				if(dev->dev_info.dev_id == CAM_REQ_MGR_DEVICE_SENSOR) break;
			}
			s_ctrl = (struct cam_sensor_ctrl_t*)cam_get_device_priv(dev->dev_hdl);
			soc_info = &s_ctrl->soc_info;
			INC_HEAD(cam_key_buffer->event_record_head[EVENT_APPLY_REQ],MAX_RECORD_REQ,&index);
			cam_key_buffer->event_id[EVENT_APPLY_REQ][index].opcode = EVENT_APPLY_REQ;
			CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_key_buffer->event_id[EVENT_APPLY_REQ][index].ts_h,
											cam_key_buffer->event_id[EVENT_APPLY_REQ][index].ts_m,
											cam_key_buffer->event_id[EVENT_APPLY_REQ][index].ts_s,
											cam_key_buffer->event_id[EVENT_APPLY_REQ][index].ts_ms);
			apply		= (struct cam_req_mgr_apply_request*)info;
			cam_key_buffer->event_id[EVENT_APPLY_REQ][index].req_id = request_id;
			cam_key_buffer->event_id[EVENT_APPLY_REQ][index].u.app_req.link_hdl = apply->link_hdl;
			if(soc_info) cam_key_buffer->event_id[EVENT_APPLY_REQ][index].u.app_req.module_id = soc_info->index;
			cam_key_buffer->event_id[EVENT_APPLY_REQ][index].u.app_req.subdev_id = subdev_id;
		}
	}
	else if(opcode == EVENT_POWER_UP)
	{
		INC_HEAD(cam_power_record_buffer->event_record_head[0],MAX_RECORD_POWER,&index);
		CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_power_record_buffer->power_seq[0][index].ts_h,
											cam_power_record_buffer->power_seq[0][index].ts_m,
											cam_power_record_buffer->power_seq[0][index].ts_s,
											cam_power_record_buffer->power_seq[0][index].ts_ms);
		power_ctrl = (struct cam_sensor_power_ctrl_t *)ctrl;
		power_setting = &power_ctrl->power_setting[request_id];
		soc_info = (struct cam_hw_soc_info*)info;
		cam_power_record_buffer->power_seq[0][index].req_id = -1;
		cam_power_record_buffer->power_seq[0][index].module_id = soc_info->index;
		cam_power_record_buffer->power_seq[0][index].subdev_id = get_subdev_id(soc_info);
		switch(power_setting->seq_type)
		{
			case SENSOR_MCLK:
					if(power_setting->config_val)
					cam_power_record_buffer->power_seq[0][index].clk_rate = power_setting->config_val;
					cam_power_record_buffer->power_seq[0][index].power_seq_msg = power_setting->seq_type;
					cam_power_record_buffer->power_seq[0][index].gpio_val = 0;
					min_volt = soc_info->rgltr_min_volt[idx];
					max_volt = soc_info->rgltr_max_volt[idx];
					cam_power_record_buffer->power_seq[0][index].voltage[0] = min_volt;
					cam_power_record_buffer->power_seq[0][index].voltage[1] = max_volt;
					break;
			case SENSOR_RESET:
			case SENSOR_STANDBY:
			case SENSOR_CUSTOM_GPIO1:
			case SENSOR_CUSTOM_GPIO2:
					cam_power_record_buffer->power_seq[0][index].clk_rate = 0;
					cam_power_record_buffer->power_seq[0][index].power_seq_msg = power_setting->seq_type;
					cam_power_record_buffer->power_seq[0][index].gpio_val = power_setting->config_val;
					cam_power_record_buffer->power_seq[0][index].voltage[0] = 0;
					cam_power_record_buffer->power_seq[0][index].voltage[1] = 0;
					break;
			case SENSOR_VANA:
			case SENSOR_VANA1:
			case SENSOR_VDIG:
			case SENSOR_VIO:
			case SENSOR_VAF:
			case SENSOR_VAF_PWDM:
			case SENSOR_CUSTOM_REG1:
			case SENSOR_CUSTOM_REG2:
			case SENSOR_BOB:
			case SENSOR_BOB2:
					cam_power_record_buffer->power_seq[0][index].clk_rate = 0;
					cam_power_record_buffer->power_seq[0][index].power_seq_msg = power_setting->seq_type;
					if(power_setting->valid_config)
					{
						min_volt = power_setting->config_val;
						max_volt = power_setting->config_val;
					}
					else
					{
						min_volt = soc_info->rgltr_min_volt[idx];
						max_volt = soc_info->rgltr_max_volt[idx];
					}
					cam_power_record_buffer->power_seq[0][index].gpio_val = 1;
					cam_power_record_buffer->power_seq[0][index].voltage[0] = min_volt;
					cam_power_record_buffer->power_seq[0][index].voltage[1] = max_volt;
					break;
			default:
					break;
		}
	}
	else if(opcode == EVENT_POWER_DOWN)
	{
		INC_HEAD(cam_power_record_buffer->event_record_head[1],MAX_RECORD_POWER,&index);
		CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_power_record_buffer->power_seq[1][index].ts_h,
											cam_power_record_buffer->power_seq[1][index].ts_m,
											cam_power_record_buffer->power_seq[1][index].ts_s,
											cam_power_record_buffer->power_seq[1][index].ts_ms);
		power_ctrl = (struct cam_sensor_power_ctrl_t *)ctrl;
		power_setting = &power_ctrl->power_down_setting[request_id];
		soc_info = (struct cam_hw_soc_info*)info;
		cam_power_record_buffer->power_seq[1][index].req_id = -1;
		cam_power_record_buffer->power_seq[1][index].module_id = soc_info->index;
		cam_power_record_buffer->power_seq[1][index].subdev_id = get_subdev_id(soc_info);
		switch(power_setting->seq_type)
		{
			case SENSOR_MCLK:
					if(power_setting->config_val)
					cam_power_record_buffer->power_seq[1][index].clk_rate = power_setting->config_val;
					cam_power_record_buffer->power_seq[1][index].power_seq_msg = power_setting->seq_type;
					cam_power_record_buffer->power_seq[1][index].gpio_val = 0;
					min_volt = soc_info->rgltr_min_volt[idx];
					max_volt = soc_info->rgltr_max_volt[idx];
					cam_power_record_buffer->power_seq[1][index].voltage[0] = min_volt;
					cam_power_record_buffer->power_seq[1][index].voltage[1] = max_volt;
					break;
			case SENSOR_RESET:
			case SENSOR_STANDBY:
			case SENSOR_CUSTOM_GPIO1:
			case SENSOR_CUSTOM_GPIO2:
					cam_power_record_buffer->power_seq[1][index].clk_rate = 0;
					cam_power_record_buffer->power_seq[1][index].power_seq_msg = power_setting->seq_type;
					cam_power_record_buffer->power_seq[1][index].gpio_val = power_setting->config_val;
					cam_power_record_buffer->power_seq[1][index].voltage[0] = 0;
					cam_power_record_buffer->power_seq[1][index].voltage[1] = 0;
					break;
			case SENSOR_VANA:
			case SENSOR_VANA1:
			case SENSOR_VDIG:
			case SENSOR_VIO:
			case SENSOR_VAF:
			case SENSOR_VAF_PWDM:
			case SENSOR_CUSTOM_REG1:
			case SENSOR_CUSTOM_REG2:
			case SENSOR_BOB:
			case SENSOR_BOB2:
					cam_power_record_buffer->power_seq[1][index].clk_rate = 0;
					cam_power_record_buffer->power_seq[1][index].power_seq_msg = power_setting->seq_type;
					if(power_setting->valid_config)
					{
						min_volt = power_setting->config_val;
						max_volt = power_setting->config_val;
					}
					else
					{
						min_volt = soc_info->rgltr_min_volt[idx];
						max_volt = soc_info->rgltr_max_volt[idx];
					}
					cam_power_record_buffer->power_seq[1][index].gpio_val = 0;
					cam_power_record_buffer->power_seq[1][index].voltage[0] = min_volt;
					cam_power_record_buffer->power_seq[1][index].voltage[1] = max_volt;
					break;
			default:
					break;
		}
	}
	else if(opcode == EVENT_IO)
	{
		if(CAM_SENSOR == subdev_id)
		{
			INC_HEAD(cam_io_record_buffer->event_record_head[0],APPLY_SETTING_REQ,&index);
			CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_io_record_buffer->sensor_setting_data[index].ts_h,
											cam_io_record_buffer->sensor_setting_data[index].ts_m,
											cam_io_record_buffer->sensor_setting_data[index].ts_s,
											cam_io_record_buffer->sensor_setting_data[index].ts_ms);
			cam_io_record_buffer->sensor_setting_data[index].req_id = request_id;

			soc_info  = (struct cam_hw_soc_info *)	ctrl;
			i2c_set   = (struct i2c_settings_list *)info;

			cam_io_record_buffer->sensor_setting_data[index].module_id = soc_info->index;
			cam_io_record_buffer->sensor_setting_data[index].subdev_id = subdev_id;
			cam_io_record_buffer->sensor_setting_data[index].opcode = i2c_set->op_code;
			cam_io_record_buffer->sensor_setting_data[index].apply_record_size = idx;
			cam_io_record_buffer->sensor_setting_data[index].regAddrType = i2c_set->i2c_settings.addr_type;
			cam_io_record_buffer->sensor_setting_data[index].regDataType = i2c_set->i2c_settings.data_type;
			for(IO_index = 0;IO_index < idx && IO_index < MAX_RECORD_SENSOR_SETTING;++IO_index)
			{
				cam_io_record_buffer->sensor_setting_data[index].ad[IO_index].address = i2c_set->i2c_settings.reg_setting[IO_index].reg_addr;
				cam_io_record_buffer->sensor_setting_data[index].ad[IO_index].data    = i2c_set->i2c_settings.reg_setting[IO_index].reg_data;
			}
		}
		else if(CAM_OIS == subdev_id)
		{
			INC_HEAD(cam_io_record_buffer->event_record_head[1],APPLY_SETTING_REQ,&index);
			CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_io_record_buffer->ois_setting[index].ts_h,
											cam_io_record_buffer->ois_setting[index].ts_m,
											cam_io_record_buffer->ois_setting[index].ts_s,
											cam_io_record_buffer->ois_setting[index].ts_ms);
			cam_io_record_buffer->ois_setting[index].req_id = request_id;

			soc_info  = (struct cam_hw_soc_info *)	ctrl;
			i2c_set   = (struct i2c_settings_list *)info;

			cam_io_record_buffer->ois_setting[index].module_id = soc_info->index;
			cam_io_record_buffer->ois_setting[index].subdev_id = subdev_id;
			cam_io_record_buffer->ois_setting[index].opcode = i2c_set->op_code;
			cam_io_record_buffer->ois_setting[index].apply_record_size = idx;

			for(IO_index = 0;IO_index < idx && IO_index < MAX_RECORD_OTHER_SETTING;++IO_index)
			{
				cam_io_record_buffer->ois_setting[index].ad[IO_index].address = i2c_set->i2c_settings.reg_setting[IO_index].reg_addr;
				cam_io_record_buffer->ois_setting[index].ad[IO_index].data    = i2c_set->i2c_settings.reg_setting[IO_index].reg_data;
			}
		}
	}
	else if(opcode == EVENT_IO_END)
	{
		if(CAM_SENSOR == subdev_id)
		{
			div_u64_rem(cam_io_record_buffer->event_record_head[0],APPLY_SETTING_REQ,&index);
			CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_io_record_buffer->sensor_setting_data[index].end_ts_h,
											cam_io_record_buffer->sensor_setting_data[index].end_ts_m,
											cam_io_record_buffer->sensor_setting_data[index].end_ts_s,
											cam_io_record_buffer->sensor_setting_data[index].end_ts_ms);
		}
		else if(CAM_OIS == subdev_id)
		{
			div_u64_rem(cam_io_record_buffer->event_record_head[1],APPLY_SETTING_REQ,&index);
			CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_io_record_buffer->ois_setting[index].end_ts_h,
											cam_io_record_buffer->ois_setting[index].end_ts_m,
											cam_io_record_buffer->ois_setting[index].end_ts_s,
											cam_io_record_buffer->ois_setting[index].end_ts_ms);
		}
	}
	else if(opcode == EVENT_ADD_REQ)
	{
		INC_HEAD(cam_key_buffer->event_record_head[EVENT_ADD_REQ],MAX_RECORD_REQ,&index);
		cam_key_buffer->event_id[EVENT_ADD_REQ][index].opcode = EVENT_ADD_REQ;
		CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_key_buffer->event_id[EVENT_ADD_REQ][index].ts_h,
											cam_key_buffer->event_id[EVENT_ADD_REQ][index].ts_m,
											cam_key_buffer->event_id[EVENT_ADD_REQ][index].ts_s,
											cam_key_buffer->event_id[EVENT_ADD_REQ][index].ts_ms);
		soc_info 	= (struct cam_hw_soc_info *)ctrl;
		add_request = (struct cam_req_mgr_add_request *)info;
		cam_key_buffer->event_id[EVENT_ADD_REQ][index].req_id = add_request->req_id;
		cam_key_buffer->event_id[EVENT_ADD_REQ][index].u.add_req.module_id = soc_info->index;
		cam_key_buffer->event_id[EVENT_ADD_REQ][index].u.add_req.subdev_id = subdev_id;
		cam_key_buffer->event_id[EVENT_ADD_REQ][index].u.add_req.dev_hdl   = add_request->dev_hdl;
		cam_key_buffer->event_id[EVENT_ADD_REQ][index].u.add_req.link_hdl  = add_request->link_hdl;
	}
	else if(opcode == EVENT_APPLY_REQ)
	{
		INC_HEAD(cam_key_buffer->event_record_head[EVENT_APPLY_REQ],MAX_RECORD_REQ,&index);
		cam_key_buffer->event_id[EVENT_APPLY_REQ][index].opcode = EVENT_APPLY_REQ;
		CAM_CONVERT_TIMESTAMP_FORMAT(ts,cam_key_buffer->event_id[EVENT_APPLY_REQ][index].ts_h,
											cam_key_buffer->event_id[EVENT_APPLY_REQ][index].ts_m,
											cam_key_buffer->event_id[EVENT_APPLY_REQ][index].ts_s,
											cam_key_buffer->event_id[EVENT_APPLY_REQ][index].ts_ms);
		soc_info 	= (struct cam_hw_soc_info *)ctrl;
		apply		= (struct cam_req_mgr_apply_request*)info;
		cam_key_buffer->event_id[EVENT_APPLY_REQ][index].req_id = request_id;
		cam_key_buffer->event_id[EVENT_APPLY_REQ][index].u.app_req.link_hdl = apply->link_hdl;
		cam_key_buffer->event_id[EVENT_APPLY_REQ][index].u.app_req.module_id = soc_info->index;
		cam_key_buffer->event_id[EVENT_APPLY_REQ][index].u.app_req.subdev_id = subdev_id;
	}
	CAM_GET_TIMESTAMP(debug_ts);
	CAM_GET_TIMESTAMP_DIFF_IN_MICRO(ts, debug_ts, microsec);
	CAM_DBG(CAM_UTIL,"%s end %s, occupy time is: %ld ms",get_subdev_name(subdev_id),get_opcode_name(opcode),microsec/1000);
	return rc;
}

void cam_init_message_buffer(void)
{
	if(cam_key_buffer != NULL||cam_io_record_buffer != NULL||cam_power_record_buffer != NULL)
	{
		if(cam_key_buffer != NULL)
			CAM_DBG(CAM_UTIL,"cam_key_buffer has been created");
		if(cam_io_record_buffer != NULL)
			CAM_DBG(CAM_UTIL,"cam_io_record_buffer has been created");
		return;
	}

	CAM_ERR(CAM_UTIL,"io_record_size:%d",sizeof(struct cam_io_data_record));
	cam_key_buffer = vmalloc(sizeof(struct cam_debug_record_key_message_buffer));
	cam_io_record_buffer = vmalloc(sizeof(struct cam_io_data_record));
	cam_power_record_buffer = vmalloc(sizeof(struct cam_power_info_record));
	CAM_ERR(CAM_UTIL,"cam_debug_record_key_message_buffer:%d",sizeof(struct cam_debug_record_key_message_buffer));

	if(!cam_key_buffer||!cam_io_record_buffer||!cam_power_record_buffer)
	{
		if(!cam_key_buffer)
		{
			CAM_ERR(CAM_UTIL,"vmalloc cam_key_buffer failure");
		}
		if(!cam_io_record_buffer)
		{
			CAM_ERR(CAM_UTIL,"vmalloc cam_io_record_buffer failure");
		}
		if(!cam_power_record_buffer)
		{
			CAM_ERR(CAM_UTIL,"vmalloc cam_power_record_buffer failure");
		}
		vfree(cam_key_buffer);
		vfree(cam_io_record_buffer);
		vfree(cam_power_record_buffer);
		return;
	}
	for(int i = 0;i < EVENT_MAX;++i)
	{
		if(i < 2)
		{
			cam_io_record_buffer->event_record_head[i] = -1;
			cam_power_record_buffer->event_record_head[i] = -1;
		}
		cam_key_buffer->event_record_head[i] = -1;
	}
}

void cam_deinit_message_buffer(void)
{
	vfree(cam_key_buffer);
	vfree(cam_io_record_buffer);
	vfree(cam_power_record_buffer);
	cam_key_buffer = NULL;
	cam_io_record_buffer = NULL;
	cam_power_record_buffer = NULL;
}

struct cam_debug_record_key_message_buffer *cam_get_key_msg_buffer(void)
{
	return cam_key_buffer;
}

struct cam_power_info_record *cam_get_power_msg_buffer(void)
{
	return cam_power_record_buffer;
}

struct cam_io_data_record *cam_get_io_msg_buffer(void)
{
	return cam_io_record_buffer;
}

void dump_sensor_setting(void)
{
	int64_t record_count = 0;
	if(cam_io_record_buffer == NULL)
	{
		return;
	}
	record_count = cam_io_record_buffer->event_record_head[0];
	if(record_count == -1)
	{
		CAM_DBG(CAM_UTIL,"EVENT %d didn't record",EVENT_IO);
		return;
	}
	else if(record_count >= MAX_RECORD_REQ)
	{
		record_count = MAX_RECORD_REQ - 1;
	}
	for(int i = 0;i < record_count;++i)
	{
		CAM_ERR(CAM_UTIL,"Sensor_Setting_Apply_start:TS:%llu:%llu:%llu:%llu,module_id:%u,subdev_id:%hd,opcode:%hu,apply_record_size:%d,req_id:%llu",
					cam_io_record_buffer->sensor_setting_data[i].ts_h,
					cam_io_record_buffer->sensor_setting_data[i].ts_m,
					cam_io_record_buffer->sensor_setting_data[i].ts_s,
					cam_io_record_buffer->sensor_setting_data[i].ts_ms,
					cam_io_record_buffer->sensor_setting_data[i].module_id,
					cam_io_record_buffer->sensor_setting_data[i].subdev_id,
					cam_io_record_buffer->sensor_setting_data[i].opcode,
					cam_io_record_buffer->sensor_setting_data[i].apply_record_size,
					cam_io_record_buffer->sensor_setting_data[i].req_id);
		for(int k = 0;k < cam_io_record_buffer->sensor_setting_data[i].apply_record_size && k < MAX_RECORD_SENSOR_SETTING;++k)
		{
			CAM_ERR(CAM_UTIL,"addr:%u , data:%u",
					cam_io_record_buffer->sensor_setting_data[i].ad[k].address,
					cam_io_record_buffer->sensor_setting_data[i].ad[k].data);
		}
		CAM_ERR(CAM_UTIL,"Sensor_Setting_Apply_End:TS:%llu:%llu:%llu:%llu,req_id:%llu,subdev_id:%hd",
					cam_io_record_buffer->sensor_setting_data[i].end_ts_h,
					cam_io_record_buffer->sensor_setting_data[i].end_ts_m,
					cam_io_record_buffer->sensor_setting_data[i].end_ts_s,
					cam_io_record_buffer->sensor_setting_data[i].end_ts_ms,
					cam_io_record_buffer->sensor_setting_data[i].req_id
					);
	}
}