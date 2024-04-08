#ifndef _CAM_RUMNAS4H_H_
#define _CAM_RUMNAS4H_H_
#define APP_RUMBAS4HFW_SIZE    (28*1024)
#define RUMBAS4H_REG_APP_VER    0x00FC
#define RUMBAS4H_REG_OIS_STS    0x0001
#define STATE_IDLE              0x01
#define RUMBAS4H_REG_FWUP_CTRL  0x000C
#define RUMBAS4H_REG_DATA_BUF   0x100
#define RUMBAS4H_REG_FWUP_ERR   0x0006
#define RUMBAS4H_SUCCESS_STATE  0x0000
#define RUMBAS4H_REG_FWUP_CHKSUM 0x0008

#define TX_SIZE_32_BYTE  32
#define TX_SIZE_64_BYTE  64
#define TX_SIZE_128_BYTE  128
#define TX_SIZE_256_BYTE  256
#define TX_BUFFER_SIZE  TX_SIZE_256_BYTE
#define RX_BUFFER_SIZE  4
#define C1 6

struct cam_sensor_i2c_reg_array SETTING_0100[256] = {};

struct cam_sensor_i2c_reg_setting write_0100 =
{
	SETTING_0100,
	256,
	CAMERA_SENSOR_I2C_TYPE_WORD,
	CAMERA_SENSOR_I2C_TYPE_BYTE,
	13,
	NULL,
	0,
};

struct cam_sensor_i2c_reg_array SETTING_0008[] =
{
	//reg_addr,                 reg_data,   delayms,    data_mask
	{RUMBAS4H_REG_FWUP_CHKSUM,    0,          0,        0},
	{RUMBAS4H_REG_FWUP_CHKSUM,    0,          0,        0},
	{RUMBAS4H_REG_FWUP_CHKSUM,    0x0,        0,        0},
	{RUMBAS4H_REG_FWUP_CHKSUM,    0x80,       0,        0},
};

struct cam_sensor_i2c_reg_setting write_0008 =
{
	SETTING_0008,
	4,
	CAMERA_SENSOR_I2C_TYPE_WORD,
	CAMERA_SENSOR_I2C_TYPE_BYTE,
	210,
	NULL,
	0,
};

struct cam_sensor_i2c_reg_array SETTING_000C[] =
{
	//reg_addr, reg_data,   delayms,    data_mask
	{0x000C,    0x75,        65,        0},
};

struct cam_sensor_i2c_reg_setting write_000C =
{
	SETTING_000C,
	1,
	CAMERA_SENSOR_I2C_TYPE_WORD,
	CAMERA_SENSOR_I2C_TYPE_BYTE,
	65,
	NULL,
	0,
};

struct cam_sensor_i2c_reg_array SETTING_0036[] =
{
	//reg_addr, reg_data,   delayms,    data_mask
	{0x0036,    0x41,        190,        0},
};

struct cam_sensor_i2c_reg_setting write_0036 =
{
	SETTING_0036,
	1,
	CAMERA_SENSOR_I2C_TYPE_WORD,
	CAMERA_SENSOR_I2C_TYPE_BYTE,
	190,
	NULL,
	0,
};

struct cam_sensor_i2c_reg_array SETTING_000D[] =
{
	//reg_addr, reg_data,   delayms,    data_mask
	{0x000D,    0x01,        20,        0},
};

struct cam_sensor_i2c_reg_setting write_000D =
{
	SETTING_000D,
	1,
	CAMERA_SENSOR_I2C_TYPE_WORD,
	CAMERA_SENSOR_I2C_TYPE_BYTE,
	20,
	NULL,
	0,
};

struct cam_sensor_i2c_reg_array SETTING_000E[] =
{
	//reg_addr, reg_data,   delayms,    data_mask
	{0x000E,    0x06,        10,        0},
};

struct cam_sensor_i2c_reg_setting  write_000E =
{
	SETTING_000E,
	1,
	CAMERA_SENSOR_I2C_TYPE_WORD,
	CAMERA_SENSOR_I2C_TYPE_BYTE,
	10,
	NULL,
	0,
};

int32_t rumbas4h_load_fw_buff(
	struct cam_ois_ctrl_t *o_ctrl,
	char* firmware_name,
	uint8_t *read_data,
	uint32_t read_length);

int32_t rumbas4h_i2c_read_data(
	struct cam_ois_ctrl_t *o_ctrl,
	uint32_t addr,
	uint32_t length,
	uint8_t *data);

int rumbas4h_ois_apply_settings(
	struct cam_ois_ctrl_t *o_ctrl,
	struct i2c_settings_array *i2c_set);

int rumbas4h_ois_pkt_download(
	struct cam_ois_ctrl_t *o_ctrl);

#endif
