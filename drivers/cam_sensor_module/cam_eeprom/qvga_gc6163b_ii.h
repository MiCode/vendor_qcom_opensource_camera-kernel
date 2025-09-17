#ifndef _QVGA_GC6163B_II_H_
#define _QVGA_GC6163B_II_H_

#include "cam_eeprom_dev.h"
#include "cam_req_mgr_dev.h"
#include "cam_eeprom_soc.h"
#include "cam_eeprom_core.h"
#include "cam_debug_util.h"

#define QVGA_GC6163B_SLAVE_ADDR      0x80
#define QVGA_GC6163B_SLAVE_ADDR_REG  0xf0
#define QVGA_GC6163B_LUX_DATA_REG            0x93
#define QVGA_GC6163B_LUX_DATA_EN_REG         0xfe
#define QVGA_GC6163B_LUX_DATA_EN_REG_VALUE   0x00

struct eeprom_memory_map_init_write_params qvga_gc6163b_hw_on_reset_setting  ={
	.slave_addr = QVGA_GC6163B_SLAVE_ADDR,
	.mem_settings =
	{
		{0xf1, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x03,CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xfc, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x12,CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xfe, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x02,CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x01, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x01,CAMERA_SENSOR_I2C_TYPE_BYTE, 5},
	},
	.memory_map_size = 4,
};

struct eeprom_memory_map_init_write_params qvga_gc6163b_hw_off_reset_setting  ={
	.slave_addr = QVGA_GC6163B_SLAVE_ADDR,
	.mem_settings =
	{
		{0xf1, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00,CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xfc, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x01,CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xfe, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x02,CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x01, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00,CAMERA_SENSOR_I2C_TYPE_BYTE, 5},
	},
	.memory_map_size = 4,
};


struct eeprom_memory_map_init_write_params qvga_gc6163b_setting  ={
	.slave_addr = QVGA_GC6163B_SLAVE_ADDR,
	.mem_settings =
	{
		{0xfe, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xa0, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xfe, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xa0, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xfe, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xa0, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xf6, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xfa, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x11, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xfc, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x12, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xfe, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x03, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x04, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xfa, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x01, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x41, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x02, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x12, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x0f, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x01, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x0d, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x30, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x12, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xc8, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x14, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x54, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x15, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x32, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x16, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x04, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x17, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x19, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x1d, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xb9, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x1f, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x15, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x7a, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x7b, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x14, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x7d, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x36, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xfe, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x10, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x20, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x7e, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x22, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x38, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x24, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x54, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x26, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x87, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x2a, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x2f, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x37, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x46, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x3f, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x18, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x49, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x40, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x4a, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x40, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x4b, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x40, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x50, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x3c, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x52, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x4f, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x53, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x81, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x54, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x43, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x56, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x78, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x57, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xaa, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},//20160901
		{0x58, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xff, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},//20160901
		{0x5b, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x60, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //dd&ee th
		{0x5c, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x80, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //60/OT_th
		{0xab, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x28, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xac, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xb5, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x60, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x45, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x62, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x68, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //20160901
		{0x63, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x13, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //edge effect
		{0x64, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x43, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x65, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x13, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //Y
		{0x66, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x26, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x67, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x07, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x68, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xf5, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //Cb
		{0x69, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xea, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x6a, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x21, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x6b, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x21, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //Cr
		{0x6c, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xe4, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x6d, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xfb, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x81, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x30, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //cb
		{0x82, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x30, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //cr
		{0x83, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x4a, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //luma contrast
		{0x85, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x06, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},  //luma offset
		{0x8d, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x78, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //edge dec sa
		{0x8e, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x25, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //autogray
		{0x90, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x38, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},//20160901
		{0x92, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x50, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //target
		{0x9d, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x32, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},//STEP
		{0x9e, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x61, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},//[7:4]margin  10fps
		{0x9f, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xf4, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xa3, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x28, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //pregain
		{0xa4, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x01, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xb1, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x1e, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //Y_to_C_diff
		{0xb3, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x20, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //C_max
		{0xbd, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x70, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //R_limit
		{0xbe, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x58, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //G_limit
		{0xbf, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xa0, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //B_limit
		{0x43, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x80, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xb0, CAMERA_SENSOR_I2C_TYPE_BYTE, 0xf2, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xb5, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x40, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xb8, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x05, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xba, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x60, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xfe, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x02, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x01, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x01, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //spi enable
		{0x02, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x80, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},   //LSB & Falling edge sample; ddr disable
		{0x03, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x20, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},	//1-wire
		{0x04, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x20, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},   //[4] master_outformat
		{0x0a, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},   //Data ID, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00-YUV422, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x01-RGB565
		{0x13, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x10, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0x24, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x02, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //[1]sck_always [0]BT656
		{0x28, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x03, CAMERA_SENSOR_I2C_TYPE_BYTE, 0}, //clock_div_spi
		{0xfe, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x00, CAMERA_SENSOR_I2C_TYPE_BYTE, 0},
		{0xf1, CAMERA_SENSOR_I2C_TYPE_BYTE, 0x03, CAMERA_SENSOR_I2C_TYPE_BYTE, 5}, //output enable
	},
	.memory_map_size = 91,
};

static int get_qvga_lux_data_gc6163b(struct cam_eeprom_ctrl_t *e_ctrl){
	struct cam_sensor_i2c_reg_array    i2c_reg_array = {0};
	struct cam_sensor_i2c_reg_setting  i2c_reg_settings = {0};
	uint32_t lux = 0;
	int rc = 0;
	i2c_reg_settings.addr_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
	i2c_reg_settings.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
	i2c_reg_settings.size = 1;
	i2c_reg_array.reg_addr = QVGA_GC6163B_LUX_DATA_EN_REG;
	i2c_reg_array.reg_data = QVGA_GC6163B_LUX_DATA_EN_REG_VALUE;
	i2c_reg_array.delay = 5;
	i2c_reg_settings.reg_setting = &i2c_reg_array;
	rc = camera_io_dev_write(&(e_ctrl->io_master_info), &i2c_reg_settings);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "write init params failed rc = %d", rc);
		return rc;
	}

	rc = camera_io_dev_read( &(e_ctrl->io_master_info),
				QVGA_GC6163B_LUX_DATA_REG, &lux, CAMERA_SENSOR_I2C_TYPE_BYTE,
				CAMERA_SENSOR_I2C_TYPE_BYTE);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "read qvga gc6163b lux fail. rc = %d", rc);
		return -1;
	}

	if (lux > 0)
		lux -= 1;
	CAM_INFO(CAM_EEPROM, "lux: 0x%x", lux);
	return lux;
}

static void qvga_hw_off_reset_gc6163b(struct cam_eeprom_ctrl_t *e_ctrl){

	struct eeprom_memory_map_init_write_params *pWriteParams = NULL;
	struct cam_sensor_i2c_reg_array    i2c_reg_array = {0};
	struct cam_sensor_i2c_reg_setting  i2c_reg_settings = {0};
	uint32_t count_write;
	int rc = 0;
	pWriteParams = &qvga_gc6163b_hw_off_reset_setting;
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

static void qvga_hw_on_reset_gc6163b(struct cam_eeprom_ctrl_t *e_ctrl){

	struct eeprom_memory_map_init_write_params *pWriteParams = NULL;
	struct cam_sensor_i2c_reg_array    i2c_reg_array = {0};
	struct cam_sensor_i2c_reg_setting  i2c_reg_settings = {0};
	uint32_t count_write;
	int rc = 0;
	pWriteParams = &qvga_gc6163b_hw_on_reset_setting;
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

static void init_qvga_setting_gc6163b(struct cam_eeprom_ctrl_t *e_ctrl){

	struct eeprom_memory_map_init_write_params *pWriteParams = NULL;
	struct cam_sensor_i2c_reg_array    i2c_reg_array = {0};
	struct cam_sensor_i2c_reg_setting  i2c_reg_settings = {0};
	uint32_t count_write;
	int rc = 0;
	pWriteParams = &qvga_gc6163b_setting;
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

static ssize_t qvga_get_name_gc6163b(int driver_flag, char *buf)
{
	ssize_t len = 0;
	if (driver_flag) {
		len += snprintf(buf + len, PAGE_SIZE - len, "%s\n",
				"gc6163b_i");
	} else {
		len += snprintf(buf + len, PAGE_SIZE - len, "%s\n",
				"none");
	}
	return len;
}
#endif
