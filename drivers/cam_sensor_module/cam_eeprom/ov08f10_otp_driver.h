#include "cam_eeprom_dev.h"
#include "cam_req_mgr_dev.h"
#include "cam_eeprom_soc.h"
#include "cam_eeprom_core.h"
#include "cam_debug_util.h"

#define OTP_OV08F10_SLAVE_ADDR      0x20

#define OTP_OV08F10_SENSORID_REG   0x00

struct eeprom_memory_map_init_write_params otp_ov08f10_read_byte_setting  ={
	.slave_addr = OTP_OV08F10_SLAVE_ADDR,
	.mem_settings =
    {
		{0xfd, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x20, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x0e, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0xe7, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x03, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0xe7, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE,  5},		
		{0xfd, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x1d, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x1c, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x19, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x20, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x0f, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0xe7, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x03, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0xe7, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE,  3}
	},
	.memory_map_size = 10,
};


struct eeprom_memory_map_init_write_params otp_ov08f10_set_page_setting  ={
	.slave_addr = OTP_OV08F10_SLAVE_ADDR,
	.mem_settings =
        {
		{0xfd, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x03, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x9F, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x20, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x9D, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x10, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0xA9, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xFF, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0xFD, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x09, CAMERA_SENSOR_I2C_TYPE_BYTE,  0}
	},
	.memory_map_size = 5,
};



void init_otp_read_byte_ov08f10(struct cam_eeprom_ctrl_t *e_ctrl){

	struct eeprom_memory_map_init_write_params *pWriteParams = NULL;
	struct cam_sensor_i2c_reg_array    i2c_reg_array = {0};
	struct cam_sensor_i2c_reg_setting  i2c_reg_settings = {0};
	uint32_t count_write;
	int rc = 0;
	pWriteParams = &otp_ov08f10_read_byte_setting;
	for(count_write=0;count_write < pWriteParams->memory_map_size; count_write++) {
		i2c_reg_settings.addr_type = pWriteParams->mem_settings[count_write].addr_type;
		i2c_reg_settings.data_type = pWriteParams->mem_settings[count_write].data_type;
		i2c_reg_settings.size = 1;
		i2c_reg_array.reg_addr = pWriteParams->mem_settings[count_write].reg_addr;
		i2c_reg_array.reg_data = pWriteParams->mem_settings[count_write].reg_data;
		i2c_reg_array.delay = pWriteParams->mem_settings[count_write].delay;
		i2c_reg_settings.reg_setting = &i2c_reg_array;

		CAM_DBG(CAM_EEPROM, "count_write %d,%d %d", count_write,i2c_reg_settings.addr_type,i2c_reg_settings.data_type);
		CAM_DBG(CAM_EEPROM, "ov08f10 count_write %d,0x%x 0x%x", count_write,i2c_reg_array.reg_addr,i2c_reg_array.reg_data);
		
		rc = camera_io_dev_write(&(e_ctrl->io_master_info), &i2c_reg_settings);
		if (rc) {
			CAM_ERR(CAM_EEPROM, "ov08f10 write init params failed rc %d", rc);
			return ;
		}
	}
	return ;
}

void otp_set_page_ov08f10(struct cam_eeprom_ctrl_t *e_ctrl, int page){

	struct eeprom_memory_map_init_write_params *pWriteParams = NULL;
	struct cam_sensor_i2c_reg_array    i2c_reg_array = {0};
	struct cam_sensor_i2c_reg_setting  i2c_reg_settings = {0};
	uint32_t count_write;
	int rc = 0;
	pWriteParams = &otp_ov08f10_set_page_setting;
	for(count_write=0;count_write < pWriteParams->memory_map_size; count_write++) {
		i2c_reg_settings.addr_type = pWriteParams->mem_settings[count_write].addr_type;
		i2c_reg_settings.data_type = pWriteParams->mem_settings[count_write].data_type;
		i2c_reg_settings.size = 1;
		i2c_reg_array.reg_addr = pWriteParams->mem_settings[count_write].reg_addr;
		i2c_reg_array.reg_data = pWriteParams->mem_settings[count_write].reg_data;
		if(count_write == 3)
			i2c_reg_array.reg_data = page;
		i2c_reg_array.delay = pWriteParams->mem_settings[count_write].delay;
		i2c_reg_settings.reg_setting = &i2c_reg_array;

		CAM_DBG(CAM_EEPROM, "count_write %d,%d %d", count_write,i2c_reg_settings.addr_type,i2c_reg_settings.data_type);
		CAM_DBG(CAM_EEPROM, "ov08f10 count_write %d,0x%x 0x%x", count_write,i2c_reg_array.reg_addr,i2c_reg_array.reg_data);
		rc = camera_io_dev_write(&(e_ctrl->io_master_info), &i2c_reg_settings);
		if (rc) {
			CAM_ERR(CAM_EEPROM, "ov08f10 write init params failed rc %d", rc);
			return ;
		}
	}
	return ;
}