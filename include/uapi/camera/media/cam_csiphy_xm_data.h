/*XM add the file
* maintain the spec phy rules in here
*/

#ifndef _CAM_CSIPHY_XM_DATA_HHH_
#define _CAM_CSIPHY_XM_DATA_HHH_

#include <media/cam_sensor.h>

#define MIPI_CSIPHY_COM_PATH				"/sys/csiphy_umd_paras/csiphy_umd_paras"

#define XM_CSIPHY_CRC_THRESHOLD_DIVISOR			0x1000

#define XM_CSIPHY_CRC_FORCE_FULL_RECOVERY		0x1001
#define XM_CSIPHY_CRC_FORCE_FULL_RECOVERY_ENABLE	0x0001
#define XM_CSIPHY_CRC_FORCE_FULL_RECOVERY_DIABLE	0x0000

#define XM_CSIPHY_ENABLE_AUTO_EQ				0x1002
#define XM_CSIPHY_ENABLE_AUTO_EQ_ENABLE			0x0001
#define XM_CSIPHY_ENABLE_AUTO_EQ_DISABLE		0x0000

#define XM_CSIPHY_ENABLE_BACKUP_EQ				0x1004

#define XM_CSIPHY_RESET_PHY_PARA			0x1003

#define XM_CSIPHY_UPDATE_CSID_ERR_TYPE			0x1005

struct cam_xm_csid_error_types {
	bool	is_valid;
	uint32_t	non_recoverable_csid_errors;
	uint32_t	recoverable_csid_errors;
};

struct cam_csiphy_xm_data_t {
	uint32_t	crc_threshold_divisor;
	uint32_t	crc_force_full_recovery;
	uint32_t	crc_occurred;
	uint32_t	auto_eq_enable;
	uint32_t	backup_eq_enable;
	struct cam_xm_csid_error_types	xm_csid_error_types;
};

#endif
