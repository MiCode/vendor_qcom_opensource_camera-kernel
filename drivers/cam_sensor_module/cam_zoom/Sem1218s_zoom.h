#ifndef _CAM_SEM1218S_H_
#define _CAM_SEM1218S_H_

/* Default set 256 byte but you can change TX_BUFFER_SIZE between 32 to 256 */
#define TX_SIZE_32_BYTE  32
#define TX_SIZE_64_BYTE  64
#define TX_SIZE_128_BYTE  128
#define TX_SIZE_256_BYTE  256
#define TX_BUFFER_SIZE  TX_SIZE_256_BYTE
#define RX_BUFFER_SIZE  4

#define REG_AFZM_CTRL  0x0200
#define REG_AFZM_STS   0x0201

#define REG_FWUP_CTRL  0x1000
#define REG_FWUP_ERR   0x1001

#define REG_APP_VER    0x1008
#define REG_FWUP_CRC   0x1010
#define REG_DATA_BUF   0x1100

#define AFZM_OFF     0x00
#define STATE_READY  0x01
#define NO_ERROR     0x00
#define SECURE_BOOT  0x40
#define RESET_REQ    0x80

#define FWUP_CTRL_32_SET 0x01
#define FWUP_CTRL_64_SET 0x03
#define FWUP_CTRL_128_SET 0x05
#define FWUP_CTRL_256_SET 0x07
#define APP_FW_SIZE  (48*1024)
#define POLYNOMIAL  0x04C11DB7

uint32_t CalculateCRC32(uint8_t *data, uint32_t size);
uint8_t load_fw_buff(
	struct cam_zoom_ctrl_t *z_ctrl,
	char* firmware_name,
	uint8_t *read_data,
	uint32_t read_length);

#endif

