#include "cam_eeprom_dev.h"
#include "cam_req_mgr_dev.h"
#include "cam_eeprom_soc.h"
#include "cam_eeprom_core.h"
#include "cam_debug_util.h"
#define OTP_GC08A8_SLAVE_ADDR      0x20
#define OTP_GC08A8_SENSORID_REG    0x03f0
#define OTP_GC08A8_DATA_ADDR_REG   0x0a6c
struct eeprom_memory_map_init_write_params otp_gc08a8_read_byte_setting  ={
	.slave_addr = OTP_GC08A8_SLAVE_ADDR,
	.mem_settings =
        {
		{0x031c, CAMERA_SENSOR_I2C_TYPE_WORD, 0x60, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0315, CAMERA_SENSOR_I2C_TYPE_WORD, 0x80, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0324, CAMERA_SENSOR_I2C_TYPE_WORD, 0x42, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0316, CAMERA_SENSOR_I2C_TYPE_WORD, 0x09, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0a67, CAMERA_SENSOR_I2C_TYPE_WORD, 0x80, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0313, CAMERA_SENSOR_I2C_TYPE_WORD, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0a53, CAMERA_SENSOR_I2C_TYPE_WORD, 0x0e, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0a65, CAMERA_SENSOR_I2C_TYPE_WORD, 0x17, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0a47, CAMERA_SENSOR_I2C_TYPE_WORD, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0a58, CAMERA_SENSOR_I2C_TYPE_WORD, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0ace, CAMERA_SENSOR_I2C_TYPE_WORD, 0x0c, CAMERA_SENSOR_I2C_TYPE_BYTE, 10}
	},
	.memory_map_size = 11,
};
struct eeprom_memory_map_init_write_params otp_gc08a8_read_continue_setting  ={
	.slave_addr = OTP_GC08A8_SLAVE_ADDR,
	.mem_settings =
        {
		{0x031c, CAMERA_SENSOR_I2C_TYPE_WORD, 0x60, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0315, CAMERA_SENSOR_I2C_TYPE_WORD, 0x80, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0324, CAMERA_SENSOR_I2C_TYPE_WORD, 0x42, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0316, CAMERA_SENSOR_I2C_TYPE_WORD, 0x09, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0a67, CAMERA_SENSOR_I2C_TYPE_WORD, 0x80, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0313, CAMERA_SENSOR_I2C_TYPE_WORD, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0a53, CAMERA_SENSOR_I2C_TYPE_WORD, 0x0e, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0a65, CAMERA_SENSOR_I2C_TYPE_WORD, 0x17, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0a68, CAMERA_SENSOR_I2C_TYPE_WORD, 0xa1, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0a47, CAMERA_SENSOR_I2C_TYPE_WORD, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0a58, CAMERA_SENSOR_I2C_TYPE_WORD, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE,  0},
		{0x0ace, CAMERA_SENSOR_I2C_TYPE_WORD, 0x0c, CAMERA_SENSOR_I2C_TYPE_BYTE, 10}
	},
	.memory_map_size = 12,
};
static void init_otp_read_continue_gc08a8(struct cam_eeprom_ctrl_t *e_ctrl){
	struct eeprom_memory_map_init_write_params *pWriteParams = NULL;
	struct cam_sensor_i2c_reg_array    i2c_reg_array = {0};
	struct cam_sensor_i2c_reg_setting  i2c_reg_settings = {0};
	uint32_t count_write;
	int rc = 0;
	pWriteParams = &otp_gc08a8_read_continue_setting;
	for(count_write=0;count_write < pWriteParams->memory_map_size; count_write++) {
		i2c_reg_settings.addr_type = pWriteParams->mem_settings[count_write].addr_type;
		i2c_reg_settings.data_type = pWriteParams->mem_settings[count_write].data_type;
		i2c_reg_settings.size = 1;
		i2c_reg_array.reg_addr = pWriteParams->mem_settings[count_write].reg_addr;
		i2c_reg_array.reg_data = pWriteParams->mem_settings[count_write].reg_data;
		i2c_reg_array.delay = pWriteParams->mem_settings[count_write].delay;
		i2c_reg_settings.reg_setting = &i2c_reg_array;
		CAM_DBG(CAM_EEPROM, "count_write %d,%d %d", count_write,i2c_reg_settings.addr_type,i2c_reg_settings.data_type);
		CAM_DBG(CAM_EEPROM, "count_write %d,0x%x 0x%x", count_write,i2c_reg_array.reg_addr,i2c_reg_array.reg_data);
		rc = camera_io_dev_write(&(e_ctrl->io_master_info), &i2c_reg_settings);
		if (rc) {
			CAM_ERR(CAM_EEPROM, "write init params failed rc %d", rc);
			return ;
		}
	}
	return ;
}
void init_otp_read_byte_gc08a8(struct cam_eeprom_ctrl_t *e_ctrl){
	struct eeprom_memory_map_init_write_params *pWriteParams = NULL;
	struct cam_sensor_i2c_reg_array    i2c_reg_array = {0};
	struct cam_sensor_i2c_reg_setting  i2c_reg_settings = {0};
	uint32_t count_write;
	int rc = 0;
	pWriteParams = &otp_gc08a8_read_byte_setting;
	for(count_write=0;count_write < pWriteParams->memory_map_size; count_write++) {
		i2c_reg_settings.addr_type = pWriteParams->mem_settings[count_write].addr_type;
		i2c_reg_settings.data_type = pWriteParams->mem_settings[count_write].data_type;
		i2c_reg_settings.size = 1;
		i2c_reg_array.reg_addr = pWriteParams->mem_settings[count_write].reg_addr;
		i2c_reg_array.reg_data = pWriteParams->mem_settings[count_write].reg_data;
		i2c_reg_array.delay = pWriteParams->mem_settings[count_write].delay;
		i2c_reg_settings.reg_setting = &i2c_reg_array;
		CAM_DBG(CAM_EEPROM, "count_write %d,%d %d", count_write,i2c_reg_settings.addr_type,i2c_reg_settings.data_type);
		CAM_DBG(CAM_EEPROM, "count_write %d,0x%x 0x%x", count_write,i2c_reg_array.reg_addr,i2c_reg_array.reg_data);
		rc = camera_io_dev_write(&(e_ctrl->io_master_info), &i2c_reg_settings);
		if (rc) {
			CAM_ERR(CAM_EEPROM, "write init params failed rc %d", rc);
			return ;
		}
	}
	return ;
}
static void otp_set_addr_gc08a8(struct cam_eeprom_ctrl_t *e_ctrl, int addr){
	struct cam_sensor_i2c_reg_array    i2c_reg_array = {0};
	struct cam_sensor_i2c_reg_setting  i2c_reg_settings = {0};
	int rc = 0;
	int addr_write;
	i2c_reg_settings.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
	i2c_reg_settings.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
	i2c_reg_settings.size = 1;
	i2c_reg_array.delay = 0;
	for (addr_write = 0; addr_write < 2; addr_write++){
		i2c_reg_array.reg_addr = 0x0a69 + addr_write;
		i2c_reg_array.reg_data = (addr >> 8*(1 - addr_write) & 0xff);
         	i2c_reg_settings.reg_setting = &i2c_reg_array;
	        rc = camera_io_dev_write(&(e_ctrl->io_master_info), &i2c_reg_settings);
	        if (rc) {
			CAM_ERR(CAM_EEPROM, "write init params failed rc %d", rc);
		        return ;
		}
	}
}
void otp_byte_flag_gc08a8(struct cam_eeprom_ctrl_t *e_ctrl){
	struct cam_sensor_i2c_reg_array    i2c_reg_array = {0};
	struct cam_sensor_i2c_reg_setting  i2c_reg_settings = {0};
	int rc = 0;
	i2c_reg_settings.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
	i2c_reg_settings.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
	i2c_reg_settings.size = 1;
	i2c_reg_array.reg_addr = 0x0313;
	i2c_reg_array.reg_data = 0x20;
	i2c_reg_array.delay = 0;
	i2c_reg_settings.reg_setting = &i2c_reg_array;
	rc = camera_io_dev_write(&(e_ctrl->io_master_info), &i2c_reg_settings);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "write init params failed rc %d", rc);
		return ;
	}
}
void otp_continue_flag_gc08a8(struct cam_eeprom_ctrl_t *e_ctrl){
	struct cam_sensor_i2c_reg_array    i2c_reg_array = {0};
	struct cam_sensor_i2c_reg_setting  i2c_reg_settings = {0};
	int rc = 0;
	i2c_reg_settings.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
	i2c_reg_settings.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
	i2c_reg_settings.size = 1;
	i2c_reg_array.reg_addr = 0x0313;
	i2c_reg_array.reg_data = 0x20;
	i2c_reg_array.delay = 0;
	i2c_reg_settings.reg_setting = &i2c_reg_array;
	rc = camera_io_dev_write(&(e_ctrl->io_master_info), &i2c_reg_settings);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "write init params failed rc %d", rc);
		return ;
	}
	i2c_reg_array.reg_data = 0x12;
	i2c_reg_settings.reg_setting = &i2c_reg_array;
	rc = camera_io_dev_write(&(e_ctrl->io_master_info), &i2c_reg_settings);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "write init params failed rc %d", rc);
		return ;
	}
}
static int otp_read_reg_gc08a8(struct cam_eeprom_ctrl_t *e_ctrl){
	int otp_data;
	camera_io_dev_read(&e_ctrl->io_master_info, OTP_GC08A8_DATA_ADDR_REG,&otp_data, CAMERA_SENSOR_I2C_TYPE_WORD,CAMERA_SENSOR_I2C_TYPE_BYTE);
	return otp_data;
}