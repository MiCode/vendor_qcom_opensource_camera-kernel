// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include "cam_sensor_io.h"
#include "cam_sensor_i2c.h"
#include "cam_sensor_i3c.h"
#include <linux/pm_runtime.h>

int32_t camera_io_dev_poll(struct camera_io_master *io_master_info,
	uint32_t addr, uint16_t data, uint32_t data_mask,
	enum camera_sensor_i2c_type addr_type,
	enum camera_sensor_i2c_type data_type,
	uint32_t delay_ms)
{
	int16_t mask = data_mask & 0xFF;

	if (!io_master_info) {
		CAM_ERR(CAM_SENSOR, "Invalid Args");
		return -EINVAL;
	}

	switch (io_master_info->master_type) {
	case CCI_MASTER:
		return cam_cci_i2c_poll(io_master_info->cci_client,
			addr, data, mask, data_type, addr_type, delay_ms);
	case I2C_MASTER:
		if (!io_master_info->qup_client) {
			CAM_ERR(CAM_SENSOR, "qup_client is NULL");
			return -EINVAL;
		}
		return cam_qup_i2c_poll(io_master_info->qup_client->i2c_client,
			addr, data, data_mask, addr_type, data_type, delay_ms);
	case I3C_MASTER:
		if (!io_master_info->qup_client) {
			CAM_ERR(CAM_SENSOR, "qup_client is NULL");
			return -EINVAL;
		}
		return cam_qup_i3c_poll(io_master_info->qup_client->i3c_client,
			addr, data, data_mask, addr_type, data_type, delay_ms);
	default:
		CAM_ERR(CAM_SENSOR, "Invalid Master Type: %d", io_master_info->master_type);
	}

	return -EINVAL;
}

int32_t camera_io_dev_erase(struct camera_io_master *io_master_info,
	uint32_t addr, uint32_t size)
{
	if (!io_master_info) {
		CAM_ERR(CAM_SENSOR, "Invalid Args");
		return -EINVAL;
	}

	if (size == 0)
		return 0;

	switch (io_master_info->master_type) {
	case SPI_MASTER:
		CAM_DBG(CAM_SENSOR, "Calling SPI Erase");
		return cam_spi_erase(io_master_info, addr, CAMERA_SENSOR_I2C_TYPE_WORD, size);
	case I2C_MASTER:
	case CCI_MASTER:
	case I3C_MASTER:
		CAM_ERR(CAM_SENSOR, "Erase not supported on Master Type: %d",
			io_master_info->master_type);
		return -EINVAL;
	default:
		CAM_ERR(CAM_SENSOR, "Invalid Master Type: %d", io_master_info->master_type);
	}

	return -EINVAL;
}

int32_t camera_io_dev_read(struct camera_io_master *io_master_info,
	uint32_t addr, uint32_t *data,
	enum camera_sensor_i2c_type addr_type,
	enum camera_sensor_i2c_type data_type,
	bool is_probing)
{
	if (!io_master_info) {
		CAM_ERR(CAM_SENSOR, "Invalid Args");
		return -EINVAL;
	}

	switch (io_master_info->master_type) {
	case SPI_MASTER:
		return cam_spi_read(io_master_info, addr, data, addr_type, data_type);
	case I2C_MASTER:
		if (!io_master_info->qup_client) {
			CAM_ERR(CAM_SENSOR, "Invalid Args: qup_client: NULL");
			return -EINVAL;
		}
		return cam_qup_i2c_read(io_master_info->qup_client->i2c_client,
			addr, data, addr_type, data_type);
	case CCI_MASTER:
		return cam_cci_i2c_read(io_master_info->cci_client,
			addr, data, addr_type, data_type, is_probing);
	case I3C_MASTER:
		if (!io_master_info->qup_client) {
			CAM_ERR(CAM_SENSOR, "Invalid Args: qup_client: NULL");
			return -EINVAL;
		}
		return cam_qup_i3c_read(io_master_info->qup_client->i3c_client,
			addr, data, addr_type, data_type);
	default:
		CAM_ERR(CAM_SENSOR, "Invalid Master Type: %d", io_master_info->master_type);
	}

	return -EINVAL;
}

int32_t camera_io_dev_read_seq(struct camera_io_master *io_master_info,
	uint32_t addr, uint8_t *data,
	enum camera_sensor_i2c_type addr_type,
	enum camera_sensor_i2c_type data_type, int32_t num_bytes)
{
	switch (io_master_info->master_type) {
	case CCI_MASTER:
		return cam_camera_cci_i2c_read_seq(io_master_info->cci_client,
			addr, data, addr_type, data_type, num_bytes);
	case I2C_MASTER:
		if (!io_master_info->qup_client) {
			CAM_ERR(CAM_SENSOR, "Invalid Args: qup_client: NULL");
			return -EINVAL;
		}
		return cam_qup_i2c_read_seq(io_master_info->qup_client->i2c_client,
			addr, data, addr_type, num_bytes);
	case SPI_MASTER:
		return cam_spi_read_seq(io_master_info, addr, data, addr_type, num_bytes);
	case I3C_MASTER:
		if (!io_master_info->qup_client) {
			CAM_ERR(CAM_SENSOR, "Invalid Args: qup_client: NULL");
			return -EINVAL;
		}
		return cam_qup_i3c_read_seq(io_master_info->qup_client->i3c_client,
			addr, data, addr_type, num_bytes);
	default:
		CAM_ERR(CAM_SENSOR, "Invalid Master Type: %d", io_master_info->master_type);
	}

	return -EINVAL;
}

int32_t camera_io_dev_write(struct camera_io_master *io_master_info,
	struct cam_sensor_i2c_reg_setting *write_setting)
{
	if (!write_setting || !io_master_info) {
		CAM_ERR(CAM_SENSOR,
			"Input parameters not valid ws: %pK ioinfo: %pK",
			write_setting, io_master_info);
		return -EINVAL;
	}

	if (!write_setting->reg_setting) {
		CAM_ERR(CAM_SENSOR, "Invalid Register Settings");
		return -EINVAL;
	}

	switch (io_master_info->master_type) {
	case CCI_MASTER:
		return cam_cci_i2c_write_table(io_master_info, write_setting);
	case I2C_MASTER:
		return cam_qup_i2c_write_table(io_master_info, write_setting);
	case SPI_MASTER:
		return cam_spi_write_table(io_master_info, write_setting);
	case I3C_MASTER:
		return cam_qup_i3c_write_table(io_master_info, write_setting);
	default:
		CAM_ERR(CAM_SENSOR, "Invalid Master Type:%d", io_master_info->master_type);
	}

	return -EINVAL;
}

int32_t camera_io_dev_write_continuous(struct camera_io_master *io_master_info,
	struct cam_sensor_i2c_reg_setting *write_setting,
	uint8_t cam_sensor_i2c_write_flag)
{
	if (!write_setting || !io_master_info) {
		CAM_ERR(CAM_SENSOR,
			"Input parameters not valid ws: %pK ioinfo: %pK",
			write_setting, io_master_info);
		return -EINVAL;
	}

	if (!write_setting->reg_setting) {
		CAM_ERR(CAM_SENSOR, "Invalid Register Settings");
		return -EINVAL;
	}

	switch (io_master_info->master_type) {
	case CCI_MASTER:
		return cam_cci_i2c_write_continuous_table(io_master_info,
			write_setting, cam_sensor_i2c_write_flag);
	case I2C_MASTER:
		return cam_qup_i2c_write_continuous_table(io_master_info,
			write_setting, cam_sensor_i2c_write_flag);
	case SPI_MASTER:
		return cam_spi_write_table(io_master_info, write_setting);
	case I3C_MASTER:
		return cam_qup_i3c_write_continuous_table(io_master_info,
			write_setting, cam_sensor_i2c_write_flag);
	default:
		CAM_ERR(CAM_SENSOR, "Invalid Master Type:%d", io_master_info->master_type);
	}

	return -EINVAL;
}

int32_t camera_io_init(struct camera_io_master *io_master_info)
{
	int rc = 0;

	if (!io_master_info) {
		CAM_ERR(CAM_SENSOR, "Invalid Args");
		return -EINVAL;
	}

	switch (io_master_info->master_type) {
	case CCI_MASTER:
		io_master_info->cci_client->cci_subdev = cam_cci_get_subdev(
						io_master_info->cci_client->cci_device);
		return cam_sensor_cci_i2c_util(io_master_info->cci_client, MSM_CCI_INIT);
	case I3C_MASTER:
		if ((io_master_info->qup_client != NULL) &&
			(io_master_info->qup_client->i3c_client != NULL)) {
			struct device *parent_dev =
				io_master_info->qup_client->i3c_client->dev.parent;

			if (parent_dev != NULL) {
				/* I3C master driver: Wait for HOT JOIN only during ACQUIRE*/
				if ((parent_dev->of_node != NULL) &&
					(parent_dev->of_node->data != NULL) &&
					(io_master_info->qup_client->i3c_wait_for_hotjoin) &&
					(io_master_info->qup_client->pm_ctrl_client_enable)) {
					*(uint32_t *)(parent_dev->of_node->data) = 1;
					CAM_DBG(CAM_SENSOR, "%s:%d: %s: SET of_node->data: %d",
						__func__, __LINE__, io_master_info->sensor_name,
						*(uint32_t *)(parent_dev->of_node->data));
				}
				if (io_master_info->qup_client->pm_ctrl_client_enable) {
					CAM_DBG(CAM_SENSOR,
						"%s:%d: %s: wait_for_hotjoin: %d I3C_MASTER: Calling get_sync",
						__func__, __LINE__, io_master_info->sensor_name,
						io_master_info->qup_client->i3c_wait_for_hotjoin);
					rc = pm_runtime_get_sync(parent_dev);
					if (rc < 0) {
						CAM_WARN(CAM_SENSOR,
							"Fail I3C getsync rc: %d for parent: %s",
							rc,	dev_name(parent_dev));
					}
				}
				/* I3C master driver: Dont Wait for HOT JOIN Further-on */
				if ((parent_dev->of_node != NULL) &&
					(parent_dev->of_node->data != NULL) &&
					(io_master_info->qup_client->i3c_wait_for_hotjoin) &&
					(io_master_info->qup_client->pm_ctrl_client_enable)) {
					*(uint32_t *)(parent_dev->of_node->data) = 0;
					CAM_DBG(CAM_SENSOR, "%s:%d: %s: SET of_node->data: %d",
						__func__, __LINE__, io_master_info->sensor_name,
						*(uint32_t *)(parent_dev->of_node->data));
				}
			}
		}
		return 0;
	case I2C_MASTER:
		if ((io_master_info->qup_client != NULL) &&
			(io_master_info->qup_client->i2c_client != NULL) &&
			(io_master_info->qup_client->i2c_client->adapter != NULL) &&
			(io_master_info->qup_client->pm_ctrl_client_enable)) {
			CAM_DBG(CAM_SENSOR, "%s:%d: %s: I2C_MASTER: Calling get_sync",
				__func__, __LINE__, io_master_info->sensor_name);
			rc = pm_runtime_get_sync(
				io_master_info->qup_client->i2c_client->adapter->dev.parent);
			if (rc < 0) {
				CAM_ERR(CAM_SENSOR, "Failed to get sync rc: %d", rc);
				return -EINVAL;
			}
		}
		return 0;
	case SPI_MASTER: return 0;
	default:
		CAM_ERR(CAM_SENSOR, "Invalid Master Type:%d", io_master_info->master_type);
	}

	return -EINVAL;
}

int32_t camera_io_release(struct camera_io_master *io_master_info)
{
	int rc = 0;

	if (!io_master_info) {
		CAM_ERR(CAM_SENSOR, "Invalid Args");
		return -EINVAL;
	}

	switch (io_master_info->master_type) {
	case CCI_MASTER:
		return cam_sensor_cci_i2c_util(io_master_info->cci_client, MSM_CCI_RELEASE);
	case I3C_MASTER:
		if ((io_master_info->qup_client != NULL) &&
			(io_master_info->qup_client->i3c_client != NULL) &&
			(io_master_info->qup_client->pm_ctrl_client_enable)) {
			CAM_DBG(CAM_SENSOR, "%s:%d: %s: I3C_MASTER: Calling put_sync",
				__func__, __LINE__, io_master_info->sensor_name);
			rc = pm_runtime_put_sync(
				io_master_info->qup_client->i3c_client->dev.parent);
			if (rc < 0) {
				CAM_WARN(CAM_SENSOR,
					"Failed to I3C PUT_SYNC rc: %d parent: %s",
					rc, dev_name(
					io_master_info->qup_client->i3c_client->dev.parent));
			}
		}
		return 0;
	case I2C_MASTER:
		if ((io_master_info->qup_client != NULL) &&
			(io_master_info->qup_client->i2c_client != NULL) &&
			(io_master_info->qup_client->i2c_client->adapter != NULL) &&
			(io_master_info->qup_client->pm_ctrl_client_enable)) {
			CAM_DBG(CAM_SENSOR, "%s:%d: %s: I2C_MASTER: Calling put_sync",
				__func__, __LINE__, io_master_info->sensor_name);
			pm_runtime_put_sync(
				io_master_info->qup_client->i2c_client->adapter->dev.parent);
		}
		return 0;
	case SPI_MASTER: return 0;
	default:
		CAM_ERR(CAM_SENSOR, "Invalid Master Type:%d", io_master_info->master_type);
	}

	return -EINVAL;
}
