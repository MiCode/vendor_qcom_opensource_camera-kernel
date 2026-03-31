/*XM add the file
* maintain the spec phy rules in here
*/

#ifndef _CAM_CSIPHY_XM_DATA_HHH_
#define _CAM_CSIPHY_XM_DATA_HHH_

#include <media/cam_sensor.h>

#define MIPI_CSIPHY_COM_PATH				"/sys/csiphy_umd_paras/crc_error_occur"

struct cam_csiphy_xm_data_t {
	int	crc_error_occur;
};

#endif
