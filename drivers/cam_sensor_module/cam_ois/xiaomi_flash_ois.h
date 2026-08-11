#ifndef  _CAM_XIAOMI_FLASH_OIS_H_
#define  _CAM_XIAOMI_FLASH_OIS_H_
#include "sem1217s.h"

#define  A1 1

typedef int (*ois_flash_pkt_download)(struct cam_ois_ctrl_t *o_ctrl);

struct flash_ois_function {
    uint8_t                flag;                  //interface for different vendor
    ois_flash_pkt_download mi_ois_pkt_download;   //jump function
};

const struct flash_ois_function pflash_ois[ ] =
{
    {A1, sem1217s_ois_fw_download}
};
#endif