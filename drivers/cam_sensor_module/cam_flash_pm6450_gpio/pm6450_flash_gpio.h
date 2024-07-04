#ifndef _PM6450_FLASH_GPIO_H_
#define _PM6450_FLASH_GPIO_H_

#include "../cam_sensor_utils/cam_sensor_cmn_header.h"
#include <linux/component.h>
#include "cam_soc_util.h"
#include "cam_sensor_util.h"
#define PM6450_FLASH_PRINT(fmt,args...) pr_info("[PM6450_FLASHLIGHT] %s line%d " fmt, __func__, __LINE__, ##args)

/* DTS state */
typedef enum {
	PM6450_FLASH_GPIO_STATE_ACTIVE,
	PM6450_FLASH_GPIO_STATE_SUSPEND,
	PM6450_FLASH_GPIO_STATE_MAX,	/* for array size */
} PM6450_FLASH_GPIO_STATE;

/*****************************************************************************
 * Function Prototype
 *****************************************************************************/
extern void pm6450_flash_gpio_select_state(PM6450_FLASH_GPIO_STATE s,struct pwm_setting *pwm);
int pm6450_flash_gpio_init_module(void);
void pm6450_flash_gpio_exit_module(void);
#endif /* _PM6450_FLASH_GPIO_H_*/