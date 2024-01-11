#ifndef _CAM_BU24532_H_
#define _CAM_BU24532_H_

#define EEPROM_ADDR 0x50 // AO A1

#define DL_BYTES     4
#define DL_6200_SIZE 64
#define DL_2800_SIZE 64
#define DL_3000_SIZE 64
#define DL_BF00_SIZE 64

#define DL_DELAY_MS  10

#define CALI_ADDR       0x1400
#define CALI_SIZE_BYTE  132

#define FIRMWARE_READ_EEPROM_DEBUG     1

#define AUTO_BOOT_RETRY_TIME 3

/**
 * @o_ctrl: OIS ctrl structure
 *
 * This API for bu24532 ois fw download.
 *
 * @return Returns success or failure.
 */
int bu24532_ois_pkt_download(
    struct cam_ois_ctrl_t *o_ctrl);

#endif
/*_CAM_BU24532_H_*/
