#ifndef  _CAM_XIAOMI_FLASH_OIS_H_
#define  _CAM_XIAOMI_FLASH_OIS_H_
#include "bu24721.c"
#include "bu24532.c"
#include "rumbas4h.c"

typedef int (*ois_flash_pkt_download)(struct cam_ois_ctrl_t *o_ctrl);

struct flash_ois_function {
    uint8_t                flag;                  //interface for different vendor
    ois_flash_pkt_download mi_ois_pkt_download;   //jump function
};

const struct flash_ois_function pflash_ois[ ] =
{
    {A1, bu24721_ois_pkt_download},
    {A2, bu24721_ois_pkt_download},
    {A3, bu24721_ois_pkt_download},
    {B1, bu24721_ois_pkt_download},
    {B2, bu24532_ois_pkt_download},
    {C1, rumbas4h_ois_pkt_download},
    {C2, bu24721_ois_pkt_download}
};
#endif
