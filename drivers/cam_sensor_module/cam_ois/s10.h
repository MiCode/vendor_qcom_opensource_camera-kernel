#ifndef _CAM_S10_H_
#define _CAM_S10_H_

#define FIRMWARE_UPDATE_RETRY_TIMES     1
#define FIRMWARE_UPDATE_EVERY_TIMES     1
#define FIRMWARE_FLASH_DUMP_SWITCH      1
#define START_DUMP_ADDR 0x5000
#define END_DUMP_ADDR   0x6300
#define DL_BYTES        4
#define DL_FLCDATA_SIZE 64
#define DL_DELAY_MS     3
#define FLCDATA_ADDR    0x7000
#define FLCST_ADDR      0x4050
#define GYRO_X          0x5814
#define GYRO_Y          0x5818

/**
 * @o_ctrl: OIS ctrl structure
 *
 * This API for s10 ois fw download.
 *
 * @return return success or failure.
 */
int s10_ois_pkt_download(struct cam_ois_ctrl_t *o_ctrl);

#endif /* _CAM_S10_H_ */