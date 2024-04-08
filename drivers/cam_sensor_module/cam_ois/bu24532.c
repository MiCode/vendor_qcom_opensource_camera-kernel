#include <linux/module.h>
#include <linux/firmware.h>
#include "cam_ois_core.h"
#include "cam_ois_soc.h"
#include "cam_sensor_util.h"
#include "cam_debug_util.h"
#include "cam_res_mgr_api.h"
#include "cam_common_util.h"
#include "cam_packet_util.h"
#include "bu24532.h"

static int bu24532_ois_fw_update_op = 0;
module_param(bu24532_ois_fw_update_op, int, 0644);

static int bu24532_ois_enable_read_eeprom = 0;
module_param(bu24532_ois_enable_read_eeprom, int, 0644);

// read program ID
struct cam_sensor_i2c_reg_array SETTING_PROGRAM_ID =
{
    //reg_addr, reg_data,   delayms,    data_mask
    0x6010,    0,           0,          0
};

// OIS status Check
struct cam_sensor_i2c_reg_array SETTING_6024 =
{
    //reg_addr, reg_data,   delayms,    data_mask
    0x6024,    0x01,        100,        0
};

//OIS Start Boot
struct cam_sensor_i2c_reg_array SETTING_F011[] =
{
    //reg_addr, reg_data,   delayms,    data_mask
    {0xF011,    0x00,       0,          0},
};
struct cam_sensor_i2c_reg_setting  write_F011 =
{
    SETTING_F011,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_BYTE,
    0,
    NULL,
    0,
};

//start DL
struct cam_sensor_i2c_reg_array SETTING_F013[] =
{
    //reg_addr, reg_data,   delayms,    data_mask
    {0xF013,    0x00,       0,          0},
};
struct cam_sensor_i2c_reg_setting  write_F013 =
{
    SETTING_F013,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_BYTE,
    0,
    NULL,
    0,
};

//SPI master I/O setting
struct cam_sensor_i2c_reg_array SETTING_6018[] =
{
    //reg_addr, reg_data,   delayms,    data_mask
    {0x6018,    0x10,       0,          0},
};
struct cam_sensor_i2c_reg_setting  write_6018 =
{
    SETTING_6018,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_BYTE,
    0,
    NULL,
    0,
};

//SPI master pull up/down setting
struct cam_sensor_i2c_reg_array SETTING_6019[] =
{
    //reg_addr, reg_data,   delayms,    data_mask
    {0x6019,    0x87,       0,          0},
};
struct cam_sensor_i2c_reg_setting  write_6019 =
{
    SETTING_6019,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_BYTE,
    0,
    NULL,
    0,
};

//OIS standby mode
struct cam_sensor_i2c_reg_array SETTING_6020[] =
{
    //reg_addr, reg_data,   delayms,    data_mask
    {0x6020,    0x00,       0,          0},
};
struct cam_sensor_i2c_reg_setting  write_6020 =
{
    SETTING_6020,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_BYTE,
    0,
    NULL,
    0,
};

//OIS stop gyro
struct cam_sensor_i2c_reg_array SETTING_6023[] =
{
    //reg_addr, reg_data,   delayms,    data_mask
    {0x6023,    0x02,       0,          0},
};
struct cam_sensor_i2c_reg_setting  write_6023 =
{
    SETTING_6023,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_BYTE,
    0,
    NULL,
    0,
};

//OSC Clock Setting1 -> for DL FW
struct cam_sensor_i2c_reg_array OSCCLK1_6080[] =
{
    //reg_addr, reg_data,       delayms,    data_mask
    {0x6080,    0x10868CE1,     0,          0},
};
struct cam_sensor_i2c_reg_setting  write_OSCCLK1_6080 =
{
    OSCCLK1_6080,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_DWORD,
    0,
    NULL,
    0,
};

//OSC Clock Setting1 -> for DL FW
struct cam_sensor_i2c_reg_array OSCCLK1_6084[] =
{
    //reg_addr, reg_data,       delayms,    data_mask
    {0x6084,    0x08068180,     0,          0},
};
struct cam_sensor_i2c_reg_setting  write_OSCCLK1_6084 =
{
    OSCCLK1_6084,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_DWORD,
    0,
    NULL,
    0,
};

//OSC Clock Setting2 -> for DL FW
struct cam_sensor_i2c_reg_array OSCCLK2_6080[] =
{
    //reg_addr, reg_data,       delayms,    data_mask
    {0x6080,    0x108690E1,     0,          0},
};
struct cam_sensor_i2c_reg_setting  write_OSCCLK2_6080 =
{
    OSCCLK2_6080,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_DWORD,
    0,
    NULL,
    0,
};

//OSC Clock Setting2 -> for DL FW
struct cam_sensor_i2c_reg_array OSCCLK2_6084[] =
{
    //reg_addr, reg_data,       delayms,    data_mask
    {0x6084,    0x08068100,     0,          0},
};
struct cam_sensor_i2c_reg_setting  write_OSCCLK2_6084 =
{
    OSCCLK2_6084,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_DWORD,
    0,
    NULL,
    0,
};

//OSC Clock Setting3 -> for auto boot
struct cam_sensor_i2c_reg_array OSCCLK3_6080[] =
{
    //reg_addr, reg_data,   delayms,    data_mask
    {0x6080,    0xC800B4E1,     0,          0},
};
struct cam_sensor_i2c_reg_setting  write_OSCCLK3_6080 =
{
    OSCCLK3_6080,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_DWORD,
    0,
    NULL,
    0,
};

//OSC Clock Setting3 -> for auto boot
struct cam_sensor_i2c_reg_array OSCCLK3_6084[] =
{
    //reg_addr, reg_data,       delayms,    data_mask
    {0x6084,    0x00000000,     0,          0},
};
struct cam_sensor_i2c_reg_setting  write_OSCCLK3_6084 =
{
    OSCCLK3_6084,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_DWORD,
    0,
    NULL,
    0,
};

//OSC Clock Setting4 -> for auto boot
struct cam_sensor_i2c_reg_array OSCCLK4_6080[] =
{
    //reg_addr, reg_data,   delayms,    data_mask
    {0x6080,    0xC800B0E1,     0,          0},
};
struct cam_sensor_i2c_reg_setting  write_OSCCLK4_6080 =
{
    OSCCLK4_6080,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_DWORD,
    0,
    NULL,
    0,
};

//OSC Clock Setting4 -> for auto boot
struct cam_sensor_i2c_reg_array OSCCLK4_6084[] =
{
    //reg_addr, reg_data,   delayms,    data_mask
    {0x6084,    0x00000001,     0,          0},
};
struct cam_sensor_i2c_reg_setting  write_OSCCLK4_6084 =
{
    OSCCLK4_6084,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_DWORD,
    0,
    NULL,
    0,
};

// OIS servo off
struct cam_sensor_i2c_reg_array SETTING_60D6[] =
{
    //reg_addr, reg_data,   delayms,    data_mask
    {0x60D6,    0x00,       0,          0},
};
struct cam_sensor_i2c_reg_setting  write_60D6 =
{
    SETTING_60D6,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_BYTE,
    0,
    NULL,
    0,
};

//OIS reset
struct cam_sensor_i2c_reg_array SETTING_7010[] =
{
    //reg_addr, reg_data,   delayms,    data_mask
    {0x7010,    0x00,       0,          0},
};
struct cam_sensor_i2c_reg_setting  write_7010 =
{
    SETTING_7010,
    1,
    CAMERA_SENSOR_I2C_TYPE_WORD,
    CAMERA_SENSOR_I2C_TYPE_BYTE,
    0,
    NULL,
    0,
};

static int bu24532_ois_apply_settings(struct cam_ois_ctrl_t *o_ctrl,
	struct i2c_settings_array *i2c_set)
{
	struct i2c_settings_list *i2c_list=NULL;
	int32_t rc = 0;
	uint32_t i, size;
	int32_t j = 0; // xiaomi add

	if (o_ctrl == NULL || i2c_set == NULL) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return -EINVAL;
	}

	if (i2c_set->is_settings_valid != 1) {
		CAM_ERR(CAM_OIS, " Invalid settings");
		return -EINVAL;
	}

	list_for_each_entry(i2c_list,
		&(i2c_set->list_head), list) {
		/* xiaomi add I2C trace begin */
		switch (i2c_list->op_code) {
		case CAM_SENSOR_I2C_WRITE_RANDOM:
		case CAM_SENSOR_I2C_WRITE_BURST:
		case CAM_SENSOR_I2C_WRITE_SEQ: {
			for (j = 0;j < i2c_list->i2c_settings.size;j++) {
				trace_cam_i2c_write_log_event("[OISSETTINGS]", o_ctrl->ois_name,
					i2c_set->request_id, j, "WRITE", i2c_list->i2c_settings.reg_setting[j].reg_addr,
					i2c_list->i2c_settings.reg_setting[j].reg_data);
			}
			break;
		}
		case CAM_SENSOR_I2C_READ_RANDOM:
		case CAM_SENSOR_I2C_READ_SEQ: {
			for (j = 0;j < i2c_list->i2c_settings.size;j++) {
				trace_cam_i2c_write_log_event("[OISSETTINGS]", o_ctrl->ois_name,
					i2c_set->request_id, j, "READ", i2c_list->i2c_settings.reg_setting[j].reg_addr,
					i2c_list->i2c_settings.reg_setting[j].reg_data);
			}
			break;
		}
		default:
			break;
		} /* xiaomi add I2C trace end */
		if (i2c_list->op_code ==  CAM_SENSOR_I2C_WRITE_RANDOM) {
			rc = camera_io_dev_write(&(o_ctrl->io_master_info),
				&(i2c_list->i2c_settings));
			if (rc < 0) {
				CAM_ERR(CAM_OIS,
					"Failed in Applying i2c wrt settings");
				return rc;
			}
		} else if (i2c_list->op_code == CAM_SENSOR_I2C_WRITE_SEQ) {
			rc = camera_io_dev_write_continuous(
				&(o_ctrl->io_master_info),
				&(i2c_list->i2c_settings),
				CAM_SENSOR_I2C_WRITE_SEQ);
			if (rc < 0) {
				CAM_ERR(CAM_OIS,
					"Failed to seq write I2C settings: %d",
					rc);
				return rc;
			}
		} else if (i2c_list->op_code == CAM_SENSOR_I2C_POLL) {
			size = i2c_list->i2c_settings.size;
			for (i = 0; i < size; i++) {
				rc = camera_io_dev_poll(
				&(o_ctrl->io_master_info),
				i2c_list->i2c_settings.reg_setting[i].reg_addr,
				i2c_list->i2c_settings.reg_setting[i].reg_data,
				i2c_list->i2c_settings.reg_setting[i].data_mask,
				i2c_list->i2c_settings.addr_type,
				i2c_list->i2c_settings.data_type,
				i2c_list->i2c_settings.reg_setting[i].delay);
				if (rc < 0) {
					CAM_ERR(CAM_OIS,
						"i2c poll apply setting Fail");
					return rc;
				}
			}
		}
	}

	return rc;
}

static int apply_fw_setting(struct cam_ois_ctrl_t *o_ctrl)
{
	struct cam_sensor_cci_client      *ois_cci_client = NULL;
	int32_t                            rc = 0;

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "[BU24532]Invalid Args");
		return -EINVAL;
	}

	ois_cci_client = o_ctrl->io_master_info.cci_client;

	CAM_INFO(CAM_OIS, "[BU24532]apply kernel firmware setting.");
	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_7010);
	cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
		SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
		CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_6019);
	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_6018);

	cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
		SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
		CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_OSCCLK3_6080);
	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_OSCCLK3_6084);

	cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
		SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
		CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_OSCCLK4_6080);
	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_OSCCLK4_6084);

	cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
		SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
		CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_F011);

	msleep(40); // You must wait for 40ms for boot to start successfully

	return rc;
}

static int cam_bu24532_download(struct cam_ois_ctrl_t *o_ctrl)
{
	uint16_t                           total_dwords = 0;
	uint8_t                           *ptr = NULL;
	int32_t                            rc = 0, cnt, i, j;
	uint32_t                           fw_size;
	const struct firmware             *fw = NULL;
	const char                        *fw_name_prog = NULL;
	const char                        *fw_name_coeff = NULL;
	const char                        *fw_name_mem = NULL;
	const char                        *fw_name_pher = NULL;
	uint32_t                           curr_fw_version = 0;
	char                               name_prog[32] = {0};
	char                               name_coeff[32] = {0};
	char                               name_mem[32] = {0};
	char                               name_pher[32] = {0};
	struct device                     *dev = &(o_ctrl->pdev->dev);
	struct cam_sensor_i2c_reg_setting  i2c_reg_setting;
	void                              *vaddr = NULL;
	struct camera_io_master            eeprom_io_master_info;
	struct cam_sensor_cci_client      *ois_cci_client = NULL;
	uint16_t                           sid_ois = 0;

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "[BU24532]Invalid Args");
		return -EINVAL;
	}

	eeprom_io_master_info = o_ctrl->io_master_info;
	ois_cci_client = o_ctrl->io_master_info.cci_client;
	sid_ois =  o_ctrl->io_master_info.cci_client->sid;

	snprintf(name_coeff, 32, "%s.coeff", o_ctrl->ois_name);

	snprintf(name_prog, 32, "%s.prog", o_ctrl->ois_name);

	snprintf(name_mem, 32, "%s.mem", o_ctrl->ois_name);

	snprintf(name_pher, 32, "%s.pher", o_ctrl->ois_name);

	/* cast pointer as const pointer*/
	fw_name_prog = name_prog;
	fw_name_coeff = name_coeff;
	fw_name_mem = name_mem;
	fw_name_pher = name_pher;

	/* 1. Check if the firmware needs to be updated */
	if( 0 == bu24532_ois_fw_update_op &&
		false == o_ctrl->is_second_init) {
		apply_fw_setting(o_ctrl);
	}

	if (0 == bu24532_ois_fw_update_op){
		cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
			SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

		cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
			SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

		cam_cci_i2c_read(ois_cci_client, SETTING_PROGRAM_ID.reg_addr,
			&curr_fw_version, CAMERA_SENSOR_I2C_TYPE_WORD,
			CAMERA_SENSOR_I2C_TYPE_DWORD, TRUE);

		CAM_INFO(CAM_OIS, "[BU24532]read program ID:0x%x, current ID:0x%x",
				curr_fw_version,
				o_ctrl->opcode.fw_version);

		if (true == o_ctrl->is_second_init &&
			curr_fw_version != o_ctrl->opcode.fw_version)
		{
			for (i=0; i< AUTO_BOOT_RETRY_TIME; i++)
			{
				CAM_ERR(CAM_OIS, "[BU24532]retry[%d:max%d] auto boot when open camera!",
						i,AUTO_BOOT_RETRY_TIME);
				apply_fw_setting(o_ctrl);

				cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
					SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
					CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

				cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
					SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
					CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

				cam_cci_i2c_read(ois_cci_client, SETTING_PROGRAM_ID.reg_addr,
					&curr_fw_version, CAMERA_SENSOR_I2C_TYPE_WORD,
					CAMERA_SENSOR_I2C_TYPE_DWORD, TRUE);

				CAM_INFO(CAM_OIS, "[BU24532]read program ID:0x%x, current ID:0x%x",
						curr_fw_version,
						o_ctrl->opcode.fw_version);
				if (curr_fw_version == o_ctrl->opcode.fw_version)
				{
					break;
				}
			}
			if (curr_fw_version != o_ctrl->opcode.fw_version)
			{
				CAM_ERR(CAM_OIS, "[BU24532]read program ID:0x%x, current ID:0x%x, AUTO BOOT failed",
						curr_fw_version,
						o_ctrl->opcode.fw_version);
				return -EINVAL;
			}
		}
	}

	if ((curr_fw_version != o_ctrl->opcode.fw_version
			&& 0 == bu24532_ois_fw_update_op) ||
		(curr_fw_version != o_ctrl->opcode.fw_version
			&& FIRMWARE_UPDATE_FORCED == bu24532_ois_fw_update_op) ||
		 FIRMWARE_UPDATE_EVERY_TIMES == bu24532_ois_fw_update_op ) {
		/* 2. Preparing to download firmware*/
		cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_7010);
		cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
			SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

		cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_6019);

		cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_6018);

		cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
			SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

		cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_OSCCLK1_6080);
		cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_OSCCLK1_6084);

		cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
			SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

		cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_OSCCLK2_6080);
		cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_OSCCLK2_6084);

		cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
			SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

		cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_F013);

		usleep_range(2000,2010); // You must wait for 2ms boot before downloading the firmware

		/*3. Load FW and DL*/
		rc = request_firmware(&fw, fw_name_prog, dev);
		if (rc) {
			CAM_ERR(CAM_OIS, "[BU24532]Failed to locate %s", fw_name_prog);
			return rc;
		}

		total_dwords = fw->size/DL_BYTES; // 4 bytes to write eeprom
		i2c_reg_setting.addr_type = o_ctrl->opcode.fw_addr_type;
		i2c_reg_setting.data_type = CAMERA_SENSOR_I2C_TYPE_DWORD;
		i2c_reg_setting.size = total_dwords;
		i2c_reg_setting.delay = DL_DELAY_MS;
		fw_size = (sizeof(struct cam_sensor_i2c_reg_array) * total_dwords);
		vaddr = vmalloc(fw_size);
		if (!vaddr) {
			CAM_ERR(CAM_OIS,
				"Failed in allocating i2c_array: fw_size: %u", fw_size);
			release_firmware(fw);
			return -ENOMEM;
		}

		CAM_DBG(CAM_OIS, "[BU24532]FW prog size(dword):%d.", total_dwords);

		i2c_reg_setting.reg_setting = (struct cam_sensor_i2c_reg_array *) (
			vaddr);

		for (i = 0, ptr = (uint8_t *)fw->data, j = 0; i < total_dwords;) {
			for (cnt = 0; cnt < DL_6200_SIZE/DL_BYTES && i < total_dwords;
				cnt++, i++) {
					i2c_reg_setting.reg_setting[cnt].reg_addr =
						o_ctrl->opcode.prog + j * DL_6200_SIZE;
					i2c_reg_setting.reg_setting[cnt].reg_data = ptr[0] << 24 |
						ptr[1] << 16 | ptr[2] << 8 | ptr[3];
					i2c_reg_setting.reg_setting[cnt].delay = 0;
					i2c_reg_setting.reg_setting[cnt].data_mask = 0;
					CAM_DBG(CAM_OIS, "[BU24532]addr:0x%04x data: 0x%02x 0x%02x 0x%02x 0x%02x",
							o_ctrl->opcode.prog + j * DL_6200_SIZE + cnt*4,
							ptr[0], ptr[1], ptr[2], ptr[3]);
					ptr+=4;
			}
			i2c_reg_setting.size = cnt;

			if (o_ctrl->opcode.is_addr_increase) {
				j++;
			}

			eeprom_io_master_info.cci_client->sid = EEPROM_ADDR;
			rc = camera_io_dev_write_continuous(&(eeprom_io_master_info),
				&i2c_reg_setting, CAM_SENSOR_I2C_WRITE_BURST);
			eeprom_io_master_info.cci_client->sid = sid_ois;
			if (rc < 0) {
				CAM_ERR(CAM_OIS, "[BU24532]OIS FW(prog) size(%d) download failed. %d",
					total_dwords, rc);
				goto release_firmware;
			}
		}
		vfree(vaddr);
		vaddr = NULL;
		fw_size = 0;
		release_firmware(fw);

		rc = request_firmware(&fw, fw_name_coeff, dev);
		if (rc) {
			CAM_ERR(CAM_OIS, "[BU24532]Failed to locate %s", fw_name_coeff);
			return rc;
		}

		total_dwords = fw->size/DL_BYTES; // 4 bytes to write eeprom
		i2c_reg_setting.addr_type = o_ctrl->opcode.fw_addr_type;
		i2c_reg_setting.data_type = CAMERA_SENSOR_I2C_TYPE_DWORD;
		i2c_reg_setting.size = total_dwords;
		i2c_reg_setting.delay = DL_DELAY_MS;
		fw_size = (sizeof(struct cam_sensor_i2c_reg_array) * total_dwords);
		vaddr = vmalloc(fw_size);
		if (!vaddr) {
			CAM_ERR(CAM_OIS,
				"Failed in allocating i2c_array: fw_size: %u", fw_size);
			release_firmware(fw);
			return -ENOMEM;
		}

		CAM_DBG(CAM_OIS, "[BU24532]FW coeff size(dword):%d", total_dwords);

		i2c_reg_setting.reg_setting = (struct cam_sensor_i2c_reg_array *) (
			vaddr);

		for (i = 0, ptr = (uint8_t *)fw->data, j = 0; i < total_dwords;) {
			for (cnt = 0; cnt < DL_2800_SIZE/DL_BYTES && i < total_dwords;
				cnt++, i++) {
					i2c_reg_setting.reg_setting[cnt].reg_addr =
						o_ctrl->opcode.coeff + j * DL_2800_SIZE;
					i2c_reg_setting.reg_setting[cnt].reg_data = ptr[0] << 24 |
						ptr[1] << 16 | ptr[2] << 8 | ptr[3];
					i2c_reg_setting.reg_setting[cnt].delay = 0;
					i2c_reg_setting.reg_setting[cnt].data_mask = 0;
					CAM_DBG(CAM_OIS, "[BU24532]addr:0x%04x data: 0x%02x 0x%02x 0x%02x 0x%02x",
							o_ctrl->opcode.coeff + j * DL_2800_SIZE + cnt*4,
							ptr[0], ptr[1], ptr[2], ptr[3]);
					ptr+=4;
			}
			i2c_reg_setting.size = cnt;

			if (o_ctrl->opcode.is_addr_increase) {
				j++;
			}

			eeprom_io_master_info.cci_client->sid = EEPROM_ADDR;
			rc = camera_io_dev_write_continuous(&(eeprom_io_master_info),
				&i2c_reg_setting, CAM_SENSOR_I2C_WRITE_BURST);
			eeprom_io_master_info.cci_client->sid = sid_ois;
			if (rc < 0) {
				CAM_ERR(CAM_OIS, "[BU24532]OIS FW(coeff) size(%d) download failed rc: %d",
					total_dwords, rc);
				goto release_firmware;
			}
		}
		vfree(vaddr);
		vaddr = NULL;
		fw_size = 0;
		release_firmware(fw);

		rc = request_firmware(&fw, fw_name_mem, dev);
		if (rc) {
			CAM_ERR(CAM_OIS, "[BU24532]Failed to locate %s", fw_name_mem);
			return rc;
		}

		total_dwords = fw->size/DL_BYTES; // 4 bytes to write eeprom
		i2c_reg_setting.addr_type = o_ctrl->opcode.fw_addr_type;
		i2c_reg_setting.data_type = CAMERA_SENSOR_I2C_TYPE_DWORD;
		i2c_reg_setting.size = total_dwords;
		i2c_reg_setting.delay = DL_DELAY_MS;
		fw_size = (sizeof(struct cam_sensor_i2c_reg_array) * total_dwords);
		vaddr = vmalloc(fw_size);
		if (!vaddr) {
			CAM_ERR(CAM_OIS,
				"Failed in allocating i2c_array: fw_size: %u", fw_size);
			release_firmware(fw);
			return -ENOMEM;
		}

		CAM_DBG(CAM_OIS, "[BU24532]FW mem size(dword):%d", total_dwords);

		i2c_reg_setting.reg_setting = (struct cam_sensor_i2c_reg_array *) (
			vaddr);

		for (i = 0, ptr = (uint8_t *)fw->data, j = 0; i < total_dwords;) {
			for (cnt = 0; cnt < DL_3000_SIZE/DL_BYTES && i < total_dwords;
				cnt++, i++) {
					i2c_reg_setting.reg_setting[cnt].reg_addr =
						o_ctrl->opcode.memory + j * DL_3000_SIZE;
					i2c_reg_setting.reg_setting[cnt].reg_data = ptr[0] << 24 |
						ptr[1] << 16 | ptr[2] << 8 | ptr[3];
					i2c_reg_setting.reg_setting[cnt].delay = 0;
					i2c_reg_setting.reg_setting[cnt].data_mask = 0;
					CAM_DBG(CAM_OIS, "[BU24532]addr:0x%04x data: 0x%02x 0x%02x 0x%02x 0x%02x",
							o_ctrl->opcode.memory + j * DL_3000_SIZE + cnt*4,
							ptr[0], ptr[1], ptr[2], ptr[3]);
					ptr+=4;
			}
			i2c_reg_setting.size = cnt;

			if (o_ctrl->opcode.is_addr_increase) {
				j++;
			}

			eeprom_io_master_info.cci_client->sid = EEPROM_ADDR;
			rc = camera_io_dev_write_continuous(&(eeprom_io_master_info),
				&i2c_reg_setting, CAM_SENSOR_I2C_WRITE_BURST);
			eeprom_io_master_info.cci_client->sid = sid_ois;
			if (rc < 0) {
				CAM_ERR(CAM_OIS, "[BU24532]OIS FW(mem) size(%d) download failed rc: %d",
					total_dwords, rc);
				goto release_firmware;
			}
		}
		vfree(vaddr);
		vaddr = NULL;
		fw_size = 0;
		release_firmware(fw);

		rc = request_firmware(&fw, fw_name_pher, dev);
		if (rc) {
			CAM_ERR(CAM_OIS, "[BU24532]Failed to locate %s", fw_name_pher);
			return rc;
		}

		total_dwords = fw->size/DL_BYTES; // 4 bytes to write eeprom
		i2c_reg_setting.addr_type = o_ctrl->opcode.fw_addr_type;
		i2c_reg_setting.data_type = CAMERA_SENSOR_I2C_TYPE_DWORD;
		i2c_reg_setting.size = total_dwords;
		i2c_reg_setting.delay = DL_DELAY_MS;
		fw_size = (sizeof(struct cam_sensor_i2c_reg_array) * total_dwords);
		vaddr = vmalloc(fw_size);
		if (!vaddr) {
			CAM_ERR(CAM_OIS,
				"Failed in allocating i2c_array: fw_size: %u", fw_size);
			release_firmware(fw);
			return -ENOMEM;
		}

		CAM_DBG(CAM_OIS, "[BU24532]FW(pheripheral) size(dword):%d", total_dwords);

		i2c_reg_setting.reg_setting = (struct cam_sensor_i2c_reg_array *) (
			vaddr);

		for (i = 0, ptr = (uint8_t *)fw->data, j = 0; i < total_dwords;) {
			for (cnt = 0; cnt < DL_BF00_SIZE/DL_BYTES && i < total_dwords;
				cnt++, i++) {
					i2c_reg_setting.reg_setting[cnt].reg_addr =
						o_ctrl->opcode.pheripheral + j * DL_BF00_SIZE;
					i2c_reg_setting.reg_setting[cnt].reg_data = ptr[0] << 24 |
						ptr[1] << 16 | ptr[2] << 8 | ptr[3];
					i2c_reg_setting.reg_setting[cnt].delay = 0;
					i2c_reg_setting.reg_setting[cnt].data_mask = 0;
					CAM_DBG(CAM_OIS, "[BU24532]addr:0x%04x data: 0x%02x 0x%02x 0x%02x 0x%02x",
							o_ctrl->opcode.pheripheral + j * DL_BF00_SIZE + cnt*4,
							ptr[0], ptr[1], ptr[2], ptr[3]);
					ptr+=4;
			}
			i2c_reg_setting.size = cnt;

			if (o_ctrl->opcode.is_addr_increase) {
				j++;
			}

			eeprom_io_master_info.cci_client->sid = EEPROM_ADDR;
			rc = camera_io_dev_write_continuous(&(eeprom_io_master_info),
				&i2c_reg_setting, CAM_SENSOR_I2C_WRITE_BURST);
			eeprom_io_master_info.cci_client->sid = sid_ois;
			if (rc < 0) {
				CAM_ERR(CAM_OIS, "[BU24532]OIS FW(pher) size(%d) download failed rc: %d",
					total_dwords, rc);
				goto release_firmware;
			}
		}

		/* 4. Check whether the download is successful*/
		apply_fw_setting(o_ctrl);

		cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
			SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

		cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
			SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
			CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

		cam_cci_i2c_read(ois_cci_client, SETTING_PROGRAM_ID.reg_addr,
			&curr_fw_version, CAMERA_SENSOR_I2C_TYPE_WORD,
			CAMERA_SENSOR_I2C_TYPE_DWORD, TRUE);

		CAM_INFO(CAM_OIS, "[BU24532]after DL, read program ID:0x%x, current ID:0x%x",
				curr_fw_version,
				o_ctrl->opcode.fw_version);

		if (curr_fw_version != o_ctrl->opcode.fw_version &&
			0 == bu24532_ois_fw_update_op){
			rc = -EINVAL;
		}
	}

release_firmware:
	vfree(vaddr);
	vaddr = NULL;
	fw_size = 0;
	release_firmware(fw);
	return rc;
}

static int cam_bu24532_read_eeprom(struct cam_ois_ctrl_t *o_ctrl)
{
	int32_t                            rc = 0, i, j;
	const struct firmware             *fw = NULL;
	const char                        *fw_name_prog = NULL;
	const char                        *fw_name_coeff = NULL;
	const char                        *fw_name_mem = NULL;
	const char                        *fw_name_pher = NULL;
	char                               name_prog[32] = {0};
	char                               name_coeff[32] = {0};
	char                               name_mem[32] = {0};
	char                               name_pher[32] = {0};
	struct device                     *dev = &(o_ctrl->pdev->dev);
	struct camera_io_master            eeprom_io_master_info;
	struct cam_sensor_cci_client      *ois_cci_client = NULL;
	uint16_t                           sid_ois = 0;
	char                               *data_tmp = NULL;

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "[BU24532]Invalid Args");
		return -EINVAL;
	}

	eeprom_io_master_info = o_ctrl->io_master_info;
	ois_cci_client = o_ctrl->io_master_info.cci_client;
	sid_ois =  o_ctrl->io_master_info.cci_client->sid;

	snprintf(name_coeff, 32, "%s.coeff", o_ctrl->ois_name);

	snprintf(name_prog, 32, "%s.prog", o_ctrl->ois_name);

	snprintf(name_mem, 32, "%s.mem", o_ctrl->ois_name);

	snprintf(name_pher, 32, "%s.pher", o_ctrl->ois_name);

	/* cast pointer as const pointer*/
	fw_name_prog = name_prog;
	fw_name_coeff = name_coeff;
	fw_name_mem = name_mem;
	fw_name_pher = name_pher;

	/* 2. Preparing to download firmware*/
	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_7010);
	cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
		SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
		CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_6019);

	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_6018);

	cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
		SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
		CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_OSCCLK1_6080);
	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_OSCCLK1_6084);

	cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
		SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
		CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_OSCCLK2_6080);
	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_OSCCLK2_6084);

	cam_cci_i2c_poll(ois_cci_client, SETTING_6024.reg_addr, SETTING_6024.reg_data,
		SETTING_6024.data_mask, CAMERA_SENSOR_I2C_TYPE_BYTE,
		CAMERA_SENSOR_I2C_TYPE_WORD, SETTING_6024.delay);

	cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_F013);

	usleep_range(2000,2010); // You must wait for 2ms boot before downloading the firmware

	/* read calibration data*/
	data_tmp = kzalloc(CALI_SIZE_BYTE, GFP_KERNEL);
	if (!data_tmp)
		return -ENOMEM;

	eeprom_io_master_info.cci_client->sid = EEPROM_ADDR;
	rc = cam_camera_cci_i2c_read_seq(eeprom_io_master_info.cci_client,
		CALI_ADDR,
		data_tmp, CAMERA_SENSOR_I2C_TYPE_WORD,
		CAMERA_SENSOR_I2C_TYPE_BYTE, CALI_SIZE_BYTE);
	eeprom_io_master_info.cci_client->sid = sid_ois;

	for (i=0;i<CALI_SIZE_BYTE;)
	{
		CAM_DBG(CAM_OIS, "[BU24532]addr:0x%04x data: 0x%02x 0x%02x 0x%02x 0x%02x",
				CALI_ADDR+i,
				data_tmp[i], data_tmp[i+1], data_tmp[i+2], data_tmp[i+3]);
		i+=DL_BYTES;
	}
	kfree(data_tmp);
	data_tmp=NULL;

	/* read fw_name_prog*/
	rc = request_firmware(&fw, fw_name_prog, dev);
	if (rc) {
		CAM_ERR(CAM_OIS, "[BU24532]Failed to locate %s", fw_name_prog);
		return rc;
	}

	data_tmp = kzalloc(DL_6200_SIZE, GFP_KERNEL);
	if (!data_tmp) {
		CAM_ERR(CAM_OIS,
			"Failed in allocating data_tmp: size: %d", DL_6200_SIZE);
		release_firmware(fw);
		return -ENOMEM;
	}

	for (j = 0; j < fw->size/DL_6200_SIZE; j++) {
		eeprom_io_master_info.cci_client->sid = EEPROM_ADDR;
		rc = cam_camera_cci_i2c_read_seq(eeprom_io_master_info.cci_client,
			o_ctrl->opcode.prog + j*DL_6200_SIZE,
			data_tmp, CAMERA_SENSOR_I2C_TYPE_WORD,
			CAMERA_SENSOR_I2C_TYPE_BYTE, DL_6200_SIZE);
		eeprom_io_master_info.cci_client->sid = sid_ois;

		for (i=0;i<DL_6200_SIZE;)
		{
			CAM_DBG(CAM_OIS, "[BU24532]addr:0x%04x data: 0x%02x 0x%02x 0x%02x 0x%02x",
					o_ctrl->opcode.prog + j*DL_6200_SIZE + i,
					data_tmp[i], data_tmp[i+1], data_tmp[i+2], data_tmp[i+3]);
			i+=DL_BYTES;
		}
	}

	release_firmware(fw);
	kfree(data_tmp);
	data_tmp=NULL;


	/* read fw_name_coeff*/
	rc = request_firmware(&fw, fw_name_coeff, dev);
	if (rc) {
		CAM_ERR(CAM_OIS, "[BU24532]Failed to locate %s", fw_name_coeff);
		return rc;
	}

	data_tmp = kzalloc(DL_2800_SIZE, GFP_KERNEL);
	if (!data_tmp) {
		CAM_ERR(CAM_OIS,
			"Failed in allocating data_tmp: size: %d", DL_2800_SIZE);
		release_firmware(fw);
		return -ENOMEM;
	}

	for (j = 0; j < fw->size/DL_2800_SIZE; j++) {
		eeprom_io_master_info.cci_client->sid = EEPROM_ADDR;
		rc = cam_camera_cci_i2c_read_seq(eeprom_io_master_info.cci_client,
			o_ctrl->opcode.coeff + j*DL_2800_SIZE,
			data_tmp, CAMERA_SENSOR_I2C_TYPE_WORD,
			CAMERA_SENSOR_I2C_TYPE_BYTE, DL_2800_SIZE);
		eeprom_io_master_info.cci_client->sid = sid_ois;

		for (i=0;i<DL_2800_SIZE;)
		{
			CAM_DBG(CAM_OIS, "[BU24532]addr:0x%04x data: 0x%02x 0x%02x 0x%02x 0x%02x",
					o_ctrl->opcode.coeff + j*DL_2800_SIZE + i,
					data_tmp[i], data_tmp[i+1], data_tmp[i+2], data_tmp[i+3]);
			i+=DL_BYTES;
		}
	}

	release_firmware(fw);
	kfree(data_tmp);
	data_tmp=NULL;


	/* read fw_name_mem*/
	rc = request_firmware(&fw, fw_name_mem, dev);
	if (rc) {
		CAM_ERR(CAM_OIS, "[BU24532]Failed to locate %s", fw_name_mem);
		return rc;
	}

	data_tmp = kzalloc(DL_3000_SIZE, GFP_KERNEL);
	if (!data_tmp) {
		CAM_ERR(CAM_OIS,
			"Failed in allocating data_tmp: size: %d", DL_3000_SIZE);
		release_firmware(fw);
		return -ENOMEM;
	}

	for (j = 0; j < fw->size/DL_3000_SIZE; j++) {
		eeprom_io_master_info.cci_client->sid = EEPROM_ADDR;
		rc = cam_camera_cci_i2c_read_seq(eeprom_io_master_info.cci_client,
			o_ctrl->opcode.memory + j*DL_3000_SIZE,
			data_tmp, CAMERA_SENSOR_I2C_TYPE_WORD,
			CAMERA_SENSOR_I2C_TYPE_BYTE, DL_3000_SIZE);
		eeprom_io_master_info.cci_client->sid = sid_ois;

		for (i=0;i<DL_3000_SIZE;)
		{
			CAM_DBG(CAM_OIS, "[BU24532]addr:0x%04x data: 0x%02x 0x%02x 0x%02x 0x%02x",
					o_ctrl->opcode.memory + j*DL_3000_SIZE + i,
					data_tmp[i], data_tmp[i+1], data_tmp[i+2], data_tmp[i+3]);
			i+=DL_BYTES;
		}
	}

	release_firmware(fw);
	kfree(data_tmp);
	data_tmp=NULL;


	/* read fw_name_pher*/
	rc = request_firmware(&fw, fw_name_pher, dev);
	if (rc) {
		CAM_ERR(CAM_OIS, "[BU24532]Failed to locate %s", fw_name_pher);
		return rc;
	}

	data_tmp = kzalloc(DL_BF00_SIZE, GFP_KERNEL);
	if (!data_tmp) {
		CAM_ERR(CAM_OIS,
			"Failed in allocating data_tmp: size: %d", DL_BF00_SIZE);
		release_firmware(fw);
		return -ENOMEM;
	}

	for (j = 0; j < fw->size/DL_BF00_SIZE; j++) {
		eeprom_io_master_info.cci_client->sid = EEPROM_ADDR;
		rc = cam_camera_cci_i2c_read_seq(eeprom_io_master_info.cci_client,
			o_ctrl->opcode.pheripheral + j*DL_BF00_SIZE,
			data_tmp, CAMERA_SENSOR_I2C_TYPE_WORD,
			CAMERA_SENSOR_I2C_TYPE_BYTE, DL_BF00_SIZE);
		eeprom_io_master_info.cci_client->sid = sid_ois;

		for (i=0;i<DL_BF00_SIZE;)
		{
			CAM_DBG(CAM_OIS, "[BU24532]addr:0x%04x data: 0x%02x 0x%02x 0x%02x 0x%02x",
					o_ctrl->opcode.pheripheral + j*DL_BF00_SIZE + i,
					data_tmp[i], data_tmp[i+1], data_tmp[i+2], data_tmp[i+3]);
			i+=DL_BYTES;
		}
	}

	release_firmware(fw);
	kfree(data_tmp);
	data_tmp=NULL;


	return rc;
}

static int cam_bu24532_fw_read_download(struct cam_ois_ctrl_t *o_ctrl)
{
	int i;
	int rc = 0;

	if (o_ctrl->opcode.fw_version != 0xFF)
	{
		for (i=0; i<FIRMWARE_UPDATE_RETRY_TIMES; i++)
		{
			rc = cam_bu24532_download(o_ctrl);
			if (rc){
				CAM_WARN(CAM_OIS,
						"[BU24532] fw update failed retry:%d,total retry:%d",
						i,
						FIRMWARE_UPDATE_RETRY_TIMES);
			}else{
				CAM_DBG(CAM_OIS, "[BU24532] FW update secuss!");
				break;
			}
		}
	}

	if (FIRMWARE_READ_EEPROM_DEBUG == bu24532_ois_enable_read_eeprom)
	{
		rc = cam_bu24532_read_eeprom(o_ctrl);
		if (rc)
		{
			CAM_WARN(CAM_OIS,
					"[BU24532] read eeprom failed! rc:%d", rc);
		}
		else{
			CAM_DBG(CAM_OIS, "[BU24532] read eeprom secuss!");
		}
	}

	return rc;
}

int bu24532_ois_pkt_download(struct cam_ois_ctrl_t *o_ctrl)
{
	int rc = 0;
	if (o_ctrl->i2c_fwinit_data.is_settings_valid == 1) {
		// apply frist init config
		rc = bu24532_ois_apply_settings(o_ctrl,
			&o_ctrl->i2c_fwinit_data);
		if ((rc == -EAGAIN) &&
			(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
			CAM_WARN(CAM_OIS,
				"CCI HW is restting: Reapplying fwinit settings");
			usleep_range(1000, 1010);
			rc = bu24532_ois_apply_settings(o_ctrl,
				&o_ctrl->i2c_fwinit_data);
		}
		if (rc) {
			CAM_ERR(CAM_OIS,
				"Cannot apply fwinit data %d",
				rc);
			goto pwr_dwn;
		} else {
			CAM_DBG(CAM_OIS, "OIS fwinit settings success");
		}
	}
	else
	{
		// apply secend init or probe
		rc = cam_bu24532_fw_read_download(o_ctrl);

		if (rc)
		{
			CAM_ERR(CAM_OIS, "fw down load failed！");
			goto pwr_dwn;
		}
		else
		{
			if (o_ctrl->i2c_postinit_data.is_settings_valid == 1) {
				rc = bu24532_ois_apply_settings(o_ctrl,
					&o_ctrl->i2c_postinit_data);
				if ((rc == -EAGAIN) &&
					(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
					CAM_WARN(CAM_OIS,
						"CCI HW is restting: Reapplying postinit settings");
					usleep_range(1000, 1010);
					rc = bu24532_ois_apply_settings(o_ctrl,
						&o_ctrl->i2c_postinit_data);
				}
				if (rc) {
					CAM_ERR(CAM_OIS,
						"Cannot apply postinit data %d",
						rc);
					goto pwr_dwn;
				} else {
					CAM_DBG(CAM_OIS, "OIS postinit settings success");
				}
			}

			if (o_ctrl->is_ois_calib) {
				rc = bu24532_ois_apply_settings(o_ctrl,
					&o_ctrl->i2c_calib_data);
				if ((rc == -EAGAIN) &&
					(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
					CAM_WARN(CAM_OIS,
						"CCI HW is restting: Reapplying calib settings");
					usleep_range(1000, 1010);
					rc = bu24532_ois_apply_settings(o_ctrl,
						&o_ctrl->i2c_calib_data);
				}
				if (rc) {
					CAM_ERR(CAM_OIS, "Cannot apply calib data");
					goto pwr_dwn;
				} else {
					CAM_DBG(CAM_OIS, "apply calib data settings success");
				}
			}
		}
	}
	if (!rc)
		return rc;
pwr_dwn:
	// modify by xiaomi for ois init fail will power down twice happen crash.
	// cam_ois_power_down(o_ctrl);
	CAM_ERR(CAM_OIS, "OIS init fail! rc=%d", rc);
	return rc;
}
