// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
 */

#include "cam_eeprom_dev.h"
#include "cam_req_mgr_dev.h"
#include "cam_eeprom_soc.h"
#include "cam_eeprom_core.h"
#include "cam_debug_util.h"
#include "qvga_sc080cs_i.h"
#include "qvga_gc6163b_ii.h"

static int    qvga_state = 0;
static int    qvga_isOpen = 0;
static int    qvga_isCreat = 0;
static int    qvga_sensor = 0;
static struct platform_device* pdev_qvga[2];
static struct platform_device *pdev_qvga_true;

static ssize_t get_qvga_name(struct device *dev, struct device_attribute *attr, char *buf)
{
	if(qvga_sensor == QVGA_GC6163B_I)
	{
		return (qvga_get_name_gc6163b(1, buf));
	} else if(qvga_sensor == QVGA_SC080CS_II)
	{
		return (qvga_get_name_sc080cs(1, buf));
	} else
	{
		CAM_ERR(CAM_EEPROM, "torch:get name fail\n");
		return (qvga_get_name_sc080cs(0, buf));
	}
}

static ssize_t show_qvga_lux_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct cam_eeprom_ctrl_t       *e_ctrl = NULL;
	uint32_t data = 0;
	if (pdev_qvga_true == NULL)
        	return sprintf(buf, "%d\n", -99);
	e_ctrl = platform_get_drvdata(pdev_qvga_true);

    if(qvga_sensor == QVGA_GC6163B_I)
	{
		data = get_qvga_lux_data_gc6163b(e_ctrl);
	} else if(qvga_sensor == QVGA_SC080CS_II)
	{
		data = get_qvga_lux_data_sc0808cs(e_ctrl);
	} else
	{
		CAM_ERR(CAM_EEPROM, "torch:get data fail\n");
	}
	CAM_INFO(CAM_EEPROM, "torch:get data= %d\n", data);
	return sprintf(buf, "%d\n", data);
}

static int qvga_probe(void)
{
	struct cam_eeprom_ctrl_t       *e_ctrl = NULL;
	struct cam_eeprom_soc_private  *soc_private;
	struct cam_sensor_power_ctrl_t *power_info;
	uint32_t                       chipid;
	uint32_t                       i;

	for (i = 1; i < QVGA_NUM; i++)
	{
		if (QVGA_GC6163B_I == i)
		{
			e_ctrl = platform_get_drvdata(pdev_qvga[i - 1]);
			soc_private = (struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
			power_info = &soc_private->power_info;
			cam_eeprom_power_up(e_ctrl, &soc_private->power_info);
			camera_io_dev_read(&e_ctrl->io_master_info, QVGA_GC6163B_SLAVE_ADDR_REG, &chipid, CAMERA_SENSOR_I2C_TYPE_BYTE, CAMERA_SENSOR_I2C_TYPE_BYTE);
			cam_eeprom_power_down(e_ctrl);
			if(chipid == 0xba){
				qvga_sensor = QVGA_GC6163B_I;
				pdev_qvga_true = pdev_qvga[i - 1];
				CAM_INFO(CAM_EEPROM, "QVGA gc6163b probesuccessfully 0x%x",chipid);
				break;
			}
			else
			{
				CAM_ERR(CAM_EEPROM, "invaild qvga gc6163b sensor id: 0x%x", chipid);
			}
		} else if (QVGA_SC080CS_II == i)
		{
			e_ctrl = platform_get_drvdata(pdev_qvga[i - 1]);
			soc_private = (struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
			power_info = &soc_private->power_info;
			cam_eeprom_power_up(e_ctrl, &soc_private->power_info);
			camera_io_dev_read(&e_ctrl->io_master_info, QVGA_SC080CS_SLAVE_ADDR_REG, &chipid, CAMERA_SENSOR_I2C_TYPE_WORD, CAMERA_SENSOR_I2C_TYPE_WORD);
			cam_eeprom_power_down(e_ctrl);
			if(chipid == 0x3a6c){
				qvga_sensor = QVGA_SC080CS_II;
				pdev_qvga_true = pdev_qvga[i - 1];
				CAM_INFO(CAM_EEPROM, "QVGA sc080cs probesuccessfully 0x%x",chipid);
				break;
			}
			else
			{
				CAM_ERR(CAM_EEPROM, "invaild qvga sc080cs sensor id: 0x%x", chipid);
			}
		}
	}

	if (qvga_sensor < QVGA_GC6163B_I || qvga_sensor > QVGA_NUM)
	{
		CAM_ERR(CAM_EEPROM, "invaild qvga index: %d", qvga_sensor);
		return  -1;
	}

	return 0;
}

static ssize_t store_qvga_opt(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct cam_eeprom_ctrl_t       *e_ctrl = NULL;
	struct cam_eeprom_soc_private  *soc_private;
	struct cam_sensor_power_ctrl_t *power_info;
	uint32_t                       chipid;

	qvga_state = simple_strtol(buf, NULL, 10);
	CAM_DBG(CAM_EEPROM, "torch:set qvga_state= %d, qvga_isOpen= %d\n", qvga_state, qvga_isOpen);
	if (QVGA_PROBE == qvga_state)
	{
		if (qvga_probe())
		{
			CAM_ERR(CAM_EEPROM, "qvga probe fail");
		}
		return count;
	}
	
	if (pdev_qvga_true == NULL)
        	return -1;
	e_ctrl = platform_get_drvdata(pdev_qvga_true);

	soc_private = (struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
	power_info = &soc_private->power_info;
	if(qvga_sensor == QVGA_GC6163B_I)
	{
//GC6163B
#if 1
		switch(qvga_state){
			case QVGA_OPEN:
				if(0 == qvga_isOpen){
					qvga_isOpen = 1;
					cam_eeprom_power_up(e_ctrl, power_info);
					camera_io_dev_read(&e_ctrl->io_master_info,QVGA_GC6163B_SLAVE_ADDR_REG,&chipid, CAMERA_SENSOR_I2C_TYPE_BYTE,CAMERA_SENSOR_I2C_TYPE_BYTE);
					qvga_hw_on_reset_gc6163b(e_ctrl);
					init_qvga_setting_gc6163b(e_ctrl);
					CAM_INFO(CAM_EEPROM, "read chipid 0x%x",chipid);
				} else {
					CAM_WARN(CAM_EEPROM, "QVGA power on repeatedly!");
				}
				break;
			case QVGA_GET_LUX:
				get_qvga_lux_data_gc6163b(e_ctrl);
				break;
			case QVGA_CLOSE:
			default:
				if(1 == qvga_isOpen){
					qvga_isOpen = 0;
					qvga_hw_off_reset_gc6163b(e_ctrl);
					cam_eeprom_power_down(e_ctrl);
				} else {
					CAM_WARN(CAM_EEPROM, "QVGA power down repeatedly!");
				}
				break;
		}
#endif
	} else if(qvga_sensor == QVGA_SC080CS_II)
	{
//SC0808CS
#if 1
		switch(qvga_state){
			case QVGA_OPEN:
				if(0 == qvga_isOpen){
					qvga_isOpen = 1;
					cam_eeprom_power_up(e_ctrl, power_info);
					camera_io_dev_read(&e_ctrl->io_master_info,QVGA_SC080CS_SLAVE_ADDR_REG,&chipid, CAMERA_SENSOR_I2C_TYPE_WORD,CAMERA_SENSOR_I2C_TYPE_WORD);
					//qvga_hw_on_reset(e_ctrl);
					init_qvga_settinit_qvga_sc080cs(e_ctrl);
					CAM_INFO(CAM_EEPROM, "read chipid 0x%x",chipid);
				} else {
					CAM_WARN(CAM_EEPROM, "QVGA power on repeatedly!");
				}
				break;
			case QVGA_GET_LUX:
				get_qvga_lux_data_sc0808cs(e_ctrl);
				break;
			case QVGA_CLOSE:
			default:
				if(1 == qvga_isOpen){
					qvga_isOpen = 0;
					//qvga_hw_off_reset(e_ctrl);
					cam_eeprom_power_down(e_ctrl);
				} else {
					CAM_WARN(CAM_EEPROM, "QVGA power down repeatedly!");
				}
				break;
		}
#endif
	} else
	{
		CAM_ERR(CAM_EEPROM, "qvga error\n");
	}
	return count;
}

static DEVICE_ATTR(rear_qvga, 0664, show_qvga_lux_data, store_qvga_opt);
static DEVICE_ATTR(cam_name, 0444, get_qvga_name, NULL);

static void cam_qvga_creat(void)
{
	static struct class *qvga_class;
	static struct device *qvga_device;

	qvga_class = class_create(THIS_MODULE, "qvga");   ///sys/class/qvga
	if (IS_ERR(qvga_class)) {
		CAM_ERR(CAM_EEPROM, "qvga Unable to create class, err = %d\n",
			(int)PTR_ERR(qvga_class));
		return ;
	}
	qvga_device =
		device_create(qvga_class, NULL, MKDEV(0,3), NULL, QVGA_DEVNAME);  ///sys/class/qvga/qvga/
	if (NULL == qvga_device) {
		CAM_ERR(CAM_EEPROM, "qvga device_create fail ~");
	}
	if (device_create_file(qvga_device,&dev_attr_rear_qvga)) { ///sys/class/qvga/qvga/rear_qvga
		CAM_ERR(CAM_EEPROM, "qvga device_create_file fail!\n");
	}
	if (device_create_file(qvga_device,&dev_attr_cam_name)) { ///sys/class/qvga/qvga/cam_name
		CAM_ERR(CAM_EEPROM, "qvga device_create_file fail!\n");
	}
	return;
}

static long cam_eeprom_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	int                       rc     = 0;
	struct cam_eeprom_ctrl_t *e_ctrl = v4l2_get_subdevdata(sd);

	switch (cmd) {
	case VIDIOC_CAM_CONTROL:
		rc = cam_eeprom_driver_cmd(e_ctrl, arg);
		break;
	default:
		rc = -ENOIOCTLCMD;
		break;
	}

	return rc;
}

static int cam_eeprom_subdev_close(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	struct cam_eeprom_ctrl_t *e_ctrl =
		v4l2_get_subdevdata(sd);

	if (!e_ctrl) {
		CAM_ERR(CAM_EEPROM, "e_ctrl ptr is NULL");
			return -EINVAL;
	}

	mutex_lock(&(e_ctrl->eeprom_mutex));
	cam_eeprom_shutdown(e_ctrl);
	mutex_unlock(&(e_ctrl->eeprom_mutex));

	return 0;
}

int32_t cam_eeprom_update_i2c_info(struct cam_eeprom_ctrl_t *e_ctrl,
	struct cam_eeprom_i2c_info_t *i2c_info)
{
	struct cam_sensor_cci_client        *cci_client = NULL;

	if (e_ctrl->io_master_info.master_type == CCI_MASTER) {
		cci_client = e_ctrl->io_master_info.cci_client;
		if (!cci_client) {
			CAM_ERR(CAM_EEPROM, "failed: cci_client %pK",
				cci_client);
			return -EINVAL;
		}
		cci_client->cci_i2c_master = e_ctrl->cci_i2c_master;
		cci_client->sid = (i2c_info->slave_addr) >> 1;
		cci_client->retries = 3;
		cci_client->id_map = 0;
		cci_client->i2c_freq_mode = i2c_info->i2c_freq_mode;
	} else if (e_ctrl->io_master_info.master_type == I2C_MASTER) {
		e_ctrl->io_master_info.client->addr = i2c_info->slave_addr;
		CAM_DBG(CAM_EEPROM, "Slave addr: 0x%x", i2c_info->slave_addr);
	} else if (e_ctrl->io_master_info.master_type == SPI_MASTER) {
		CAM_ERR(CAM_EEPROM, "Slave addr: 0x%x Freq Mode: %d",
		i2c_info->slave_addr, i2c_info->i2c_freq_mode);
	}
	return 0;
}

#ifdef CONFIG_COMPAT
static long cam_eeprom_init_subdev_do_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, unsigned long arg)
{
	struct cam_control cmd_data;
	int32_t rc = 0;

	if (copy_from_user(&cmd_data, (void __user *)arg,
		sizeof(cmd_data))) {
		CAM_ERR(CAM_EEPROM,
			"Failed to copy from user_ptr=%pK size=%zu",
			(void __user *)arg, sizeof(cmd_data));
		return -EFAULT;
	}

	switch (cmd) {
	case VIDIOC_CAM_CONTROL:
		rc = cam_eeprom_subdev_ioctl(sd, cmd, &cmd_data);
		if (rc < 0) {
			CAM_ERR(CAM_EEPROM,
				"Failed in eeprom suddev handling rc %d",
				rc);
			return rc;
		}
		break;
	default:
		CAM_ERR(CAM_EEPROM, "Invalid compat ioctl: %d", cmd);
		rc = -EINVAL;
	}

	if (!rc) {
		if (copy_to_user((void __user *)arg, &cmd_data,
			sizeof(cmd_data))) {
			CAM_ERR(CAM_EEPROM,
				"Failed to copy from user_ptr=%pK size=%zu",
				(void __user *)arg, sizeof(cmd_data));
			rc = -EFAULT;
		}
	}
	return rc;
}
#endif

static const struct v4l2_subdev_internal_ops cam_eeprom_internal_ops = {
	.close = cam_eeprom_subdev_close,
};

static struct v4l2_subdev_core_ops cam_eeprom_subdev_core_ops = {
	.ioctl = cam_eeprom_subdev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = cam_eeprom_init_subdev_do_ioctl,
#endif
};

static struct v4l2_subdev_ops cam_eeprom_subdev_ops = {
	.core = &cam_eeprom_subdev_core_ops,
};

static int cam_eeprom_init_subdev(struct cam_eeprom_ctrl_t *e_ctrl)
{
	int rc = 0;

	e_ctrl->v4l2_dev_str.internal_ops = &cam_eeprom_internal_ops;
	e_ctrl->v4l2_dev_str.ops = &cam_eeprom_subdev_ops;
	strlcpy(e_ctrl->device_name, CAM_EEPROM_NAME,
		sizeof(e_ctrl->device_name));
	e_ctrl->v4l2_dev_str.name = e_ctrl->device_name;
	e_ctrl->v4l2_dev_str.sd_flags =
		(V4L2_SUBDEV_FL_HAS_DEVNODE | V4L2_SUBDEV_FL_HAS_EVENTS);
	e_ctrl->v4l2_dev_str.ent_function = CAM_EEPROM_DEVICE_TYPE;
	e_ctrl->v4l2_dev_str.token = e_ctrl;

	rc = cam_register_subdev(&(e_ctrl->v4l2_dev_str));
	if (rc)
		CAM_ERR(CAM_SENSOR, "Fail with cam_register_subdev");

	return rc;
}

static int cam_eeprom_i2c_driver_probe(struct i2c_client *client,
	 const struct i2c_device_id *id)
{
	int                             rc = 0;
	struct cam_eeprom_ctrl_t       *e_ctrl = NULL;
	struct cam_eeprom_soc_private  *soc_private = NULL;
	struct cam_hw_soc_info         *soc_info = NULL;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		CAM_ERR(CAM_EEPROM, "i2c_check_functionality failed");
		goto probe_failure;
	}

	e_ctrl = kzalloc(sizeof(*e_ctrl), GFP_KERNEL);
	if (!e_ctrl) {
		CAM_ERR(CAM_EEPROM, "kzalloc failed");
		rc = -ENOMEM;
		goto probe_failure;
	}

	soc_private = kzalloc(sizeof(*soc_private), GFP_KERNEL);
	if (!soc_private)
		goto ectrl_free;

	e_ctrl->soc_info.soc_private = soc_private;

	i2c_set_clientdata(client, e_ctrl);

	mutex_init(&(e_ctrl->eeprom_mutex));

	INIT_LIST_HEAD(&(e_ctrl->wr_settings.list_head));
	soc_info = &e_ctrl->soc_info;
	soc_info->dev = &client->dev;
	soc_info->dev_name = client->name;
	e_ctrl->io_master_info.master_type = I2C_MASTER;
	e_ctrl->io_master_info.client = client;
	e_ctrl->eeprom_device_type = MSM_CAMERA_I2C_DEVICE;
	e_ctrl->cal_data.mapdata = NULL;
	e_ctrl->cal_data.map = NULL;
	e_ctrl->userspace_probe = false;

	rc = cam_eeprom_parse_dt(e_ctrl);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "failed: soc init rc %d", rc);
		goto free_soc;
	}

	rc = cam_eeprom_update_i2c_info(e_ctrl, &soc_private->i2c_info);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "failed: to update i2c info rc %d", rc);
		goto free_soc;
	}

	rc = cam_eeprom_init_subdev(e_ctrl);
	if (rc)
		goto free_soc;

	if (soc_private->i2c_info.slave_addr != 0)
		e_ctrl->io_master_info.client->addr =
			soc_private->i2c_info.slave_addr;

	e_ctrl->bridge_intf.device_hdl = -1;
	e_ctrl->bridge_intf.ops.get_dev_info = NULL;
	e_ctrl->bridge_intf.ops.link_setup = NULL;
	e_ctrl->bridge_intf.ops.apply_req = NULL;
	e_ctrl->cam_eeprom_state = CAM_EEPROM_INIT;

	return rc;
free_soc:
	kfree(soc_private);
ectrl_free:
	kfree(e_ctrl);
probe_failure:
	return rc;
}

static int cam_eeprom_i2c_driver_remove(struct i2c_client *client)
{
	int                             i;
	struct v4l2_subdev             *sd = i2c_get_clientdata(client);
	struct cam_eeprom_ctrl_t       *e_ctrl;
	struct cam_eeprom_soc_private  *soc_private;
	struct cam_hw_soc_info         *soc_info;

	if (!sd) {
		CAM_ERR(CAM_EEPROM, "Subdevice is NULL");
		return -EINVAL;
	}

	e_ctrl = (struct cam_eeprom_ctrl_t *)v4l2_get_subdevdata(sd);
	if (!e_ctrl) {
		CAM_ERR(CAM_EEPROM, "eeprom device is NULL");
		return -EINVAL;
	}

	soc_private =
		(struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
	if (!soc_private) {
		CAM_ERR(CAM_EEPROM, "soc_info.soc_private is NULL");
		return -EINVAL;
	}

	CAM_INFO(CAM_EEPROM, "i2c driver remove invoked");
	soc_info = &e_ctrl->soc_info;
	for (i = 0; i < soc_info->num_clk; i++)
		devm_clk_put(soc_info->dev, soc_info->clk[i]);

	mutex_lock(&(e_ctrl->eeprom_mutex));
	cam_eeprom_shutdown(e_ctrl);
	mutex_unlock(&(e_ctrl->eeprom_mutex));
	mutex_destroy(&(e_ctrl->eeprom_mutex));
	cam_unregister_subdev(&(e_ctrl->v4l2_dev_str));
	kfree(soc_private);
	v4l2_set_subdevdata(&e_ctrl->v4l2_dev_str.sd, NULL);
	kfree(e_ctrl);

	return 0;
}

static int cam_eeprom_spi_setup(struct spi_device *spi)
{
	struct cam_eeprom_ctrl_t       *e_ctrl = NULL;
	struct cam_hw_soc_info         *soc_info = NULL;
	struct cam_sensor_spi_client   *spi_client;
	struct cam_eeprom_soc_private  *eb_info;
	struct cam_sensor_power_ctrl_t *power_info = NULL;
	int                             rc = 0;

	e_ctrl = kzalloc(sizeof(*e_ctrl), GFP_KERNEL);
	if (!e_ctrl)
		return -ENOMEM;

	soc_info = &e_ctrl->soc_info;
	soc_info->dev = &spi->dev;
	soc_info->dev_name = spi->modalias;

	e_ctrl->v4l2_dev_str.ops = &cam_eeprom_subdev_ops;
	e_ctrl->userspace_probe = false;
	e_ctrl->cal_data.mapdata = NULL;
	e_ctrl->cal_data.map = NULL;

	spi_client = kzalloc(sizeof(*spi_client), GFP_KERNEL);
	if (!spi_client) {
		kfree(e_ctrl);
		return -ENOMEM;
	}

	eb_info = kzalloc(sizeof(*eb_info), GFP_KERNEL);
	if (!eb_info)
		goto spi_free;
	e_ctrl->soc_info.soc_private = eb_info;

	e_ctrl->eeprom_device_type = MSM_CAMERA_SPI_DEVICE;
	e_ctrl->io_master_info.spi_client = spi_client;
	e_ctrl->io_master_info.master_type = SPI_MASTER;
	spi_client->spi_master = spi;
	INIT_LIST_HEAD(&(e_ctrl->wr_settings.list_head));
	power_info = &eb_info->power_info;
	power_info->dev = &spi->dev;

	/* set spi instruction info */
	spi_client->retry_delay = 1;
	spi_client->retries = 0;

	/* Initialize mutex */
	mutex_init(&(e_ctrl->eeprom_mutex));

	e_ctrl->bridge_intf.device_hdl = -1;
	rc = cam_eeprom_parse_dt(e_ctrl);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "failed: spi soc init rc %d", rc);
		goto board_free;
	}

	rc = cam_eeprom_spi_parse_of(spi_client);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "Device tree parsing error");
		goto board_free;
	}

	rc = cam_eeprom_init_subdev(e_ctrl);
	if (rc)
		goto board_free;

	e_ctrl->bridge_intf.ops.get_dev_info = NULL;
	e_ctrl->bridge_intf.ops.link_setup = NULL;
	e_ctrl->bridge_intf.ops.apply_req = NULL;

	v4l2_set_subdevdata(&e_ctrl->v4l2_dev_str.sd, e_ctrl);
	return rc;

board_free:
	kfree(e_ctrl->soc_info.soc_private);
spi_free:
	kfree(spi_client);
	kfree(e_ctrl);
	return rc;
}

static int cam_eeprom_spi_driver_probe(struct spi_device *spi)
{
	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_0;
	spi_setup(spi);

	CAM_DBG(CAM_EEPROM, "irq[%d] cs[%x] CPHA[%x] CPOL[%x] CS_HIGH[%x]",
		spi->irq, spi->chip_select, (spi->mode & SPI_CPHA) ? 1 : 0,
		(spi->mode & SPI_CPOL) ? 1 : 0,
		(spi->mode & SPI_CS_HIGH) ? 1 : 0);
	CAM_DBG(CAM_EEPROM, "max_speed[%u]", spi->max_speed_hz);

	return cam_eeprom_spi_setup(spi);
}

static int cam_eeprom_spi_driver_remove(struct spi_device *sdev)
{
	int                             i;
	struct v4l2_subdev             *sd = spi_get_drvdata(sdev);
	struct cam_eeprom_ctrl_t       *e_ctrl;
	struct cam_eeprom_soc_private  *soc_private;
	struct cam_hw_soc_info         *soc_info;

	if (!sd) {
		CAM_ERR(CAM_EEPROM, "Subdevice is NULL");
		return -EINVAL;
	}

	e_ctrl = (struct cam_eeprom_ctrl_t *)v4l2_get_subdevdata(sd);
	if (!e_ctrl) {
		CAM_ERR(CAM_EEPROM, "eeprom device is NULL");
		return -EINVAL;
	}

	soc_info = &e_ctrl->soc_info;
	for (i = 0; i < soc_info->num_clk; i++)
		devm_clk_put(soc_info->dev, soc_info->clk[i]);

	mutex_lock(&(e_ctrl->eeprom_mutex));
	cam_eeprom_shutdown(e_ctrl);
	mutex_unlock(&(e_ctrl->eeprom_mutex));
	mutex_destroy(&(e_ctrl->eeprom_mutex));
	cam_unregister_subdev(&(e_ctrl->v4l2_dev_str));
	kfree(e_ctrl->io_master_info.spi_client);
	e_ctrl->io_master_info.spi_client = NULL;
	soc_private =
		(struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
	if (soc_private) {
		kfree(soc_private->power_info.gpio_num_info);
		soc_private->power_info.gpio_num_info = NULL;
		kfree(soc_private);
		soc_private = NULL;
	}
	v4l2_set_subdevdata(&e_ctrl->v4l2_dev_str.sd, NULL);
	kfree(e_ctrl);

	return 0;
}

static int32_t cam_eeprom_platform_driver_probe(
	struct platform_device *pdev)
{
	int32_t                         rc = 0;
	struct cam_eeprom_ctrl_t       *e_ctrl = NULL;
	struct cam_eeprom_soc_private  *soc_private = NULL;

	e_ctrl = kzalloc(sizeof(struct cam_eeprom_ctrl_t), GFP_KERNEL);
	if (!e_ctrl)
		return -ENOMEM;

	e_ctrl->soc_info.pdev = pdev;
	e_ctrl->soc_info.dev = &pdev->dev;
	e_ctrl->soc_info.dev_name = pdev->name;
	e_ctrl->eeprom_device_type = MSM_CAMERA_PLATFORM_DEVICE;
	e_ctrl->cal_data.mapdata = NULL;
	e_ctrl->cal_data.map = NULL;
	e_ctrl->userspace_probe = false;

	e_ctrl->io_master_info.master_type = CCI_MASTER;
	e_ctrl->io_master_info.cci_client = kzalloc(
		sizeof(struct cam_sensor_cci_client), GFP_KERNEL);
	if (!e_ctrl->io_master_info.cci_client) {
		rc = -ENOMEM;
		goto free_e_ctrl;
	}

	soc_private = kzalloc(sizeof(struct cam_eeprom_soc_private),
		GFP_KERNEL);
	if (!soc_private) {
		rc = -ENOMEM;
		goto free_cci_client;
	}
	e_ctrl->soc_info.soc_private = soc_private;
	soc_private->power_info.dev = &pdev->dev;

	/* Initialize mutex */
	mutex_init(&(e_ctrl->eeprom_mutex));
	rc = cam_eeprom_parse_dt(e_ctrl);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "failed: soc init rc %d", rc);
		goto free_soc;
	}
	rc = cam_eeprom_update_i2c_info(e_ctrl, &soc_private->i2c_info);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "failed: to update i2c info rc %d", rc);
		goto free_soc;
	}

	INIT_LIST_HEAD(&(e_ctrl->wr_settings.list_head));
	rc = cam_eeprom_init_subdev(e_ctrl);
	if (rc)
		goto free_soc;

	e_ctrl->bridge_intf.device_hdl = -1;
	e_ctrl->bridge_intf.ops.get_dev_info = NULL;
	e_ctrl->bridge_intf.ops.link_setup = NULL;
	e_ctrl->bridge_intf.ops.apply_req = NULL;
	platform_set_drvdata(pdev, e_ctrl);
	e_ctrl->cam_eeprom_state = CAM_EEPROM_INIT;
	CAM_DBG(CAM_EEPROM, "Component bound successfully");
//qvga
	if(soc_private->i2c_info.slave_addr == QVGA_SC080CS_SLAVE_ADDR){
		if (!qvga_isCreat) {
			cam_qvga_creat();
			qvga_isCreat = 1;
			CAM_INFO(CAM_EEPROM, "QVGA sc080cs Component bound successfully %x",soc_private->i2c_info.slave_addr);
		}
		pdev_qvga[QVGA_SC080CS_II - 1] = pdev;
	}
	if(soc_private->i2c_info.slave_addr == QVGA_GC6163B_SLAVE_ADDR){
		if (!qvga_isCreat) {
			cam_qvga_creat();
			qvga_isCreat = 1;
			CAM_INFO(CAM_EEPROM, "QVGA gc6163b Component bound successfully %x",soc_private->i2c_info.slave_addr);
		}
		pdev_qvga[QVGA_GC6163B_I - 1] = pdev;
	}

	return rc;
free_soc:
	kfree(soc_private);
free_cci_client:
	kfree(e_ctrl->io_master_info.cci_client);
free_e_ctrl:
	kfree(e_ctrl);

	return rc;
}

static int cam_eeprom_platform_driver_remove(struct platform_device *pdev)
{
	int                        i;
	struct cam_eeprom_ctrl_t  *e_ctrl;
	struct cam_hw_soc_info    *soc_info;

	e_ctrl = platform_get_drvdata(pdev);
	if (!e_ctrl) {
		CAM_ERR(CAM_EEPROM, "eeprom device is NULL");
		return -EINVAL;
	}

	CAM_INFO(CAM_EEPROM, "Platform driver remove invoked");
	soc_info = &e_ctrl->soc_info;

	for (i = 0; i < soc_info->num_clk; i++)
		devm_clk_put(soc_info->dev, soc_info->clk[i]);

	mutex_lock(&(e_ctrl->eeprom_mutex));
	cam_eeprom_shutdown(e_ctrl);
	mutex_unlock(&(e_ctrl->eeprom_mutex));
	mutex_destroy(&(e_ctrl->eeprom_mutex));
	cam_unregister_subdev(&(e_ctrl->v4l2_dev_str));
	kfree(soc_info->soc_private);
	kfree(e_ctrl->io_master_info.cci_client);
	platform_set_drvdata(pdev, NULL);
	v4l2_set_subdevdata(&e_ctrl->v4l2_dev_str.sd, NULL);
	kfree(e_ctrl);

	return 0;
}

static const struct of_device_id cam_eeprom_dt_match[] = {
	{ .compatible = "qcom,eeprom" },
	{ }
};


MODULE_DEVICE_TABLE(of, cam_eeprom_dt_match);

static struct platform_driver cam_eeprom_platform_driver = {
	.driver = {
		.name = "qcom,eeprom",
		.owner = THIS_MODULE,
		.of_match_table = cam_eeprom_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = cam_eeprom_platform_driver_probe,
	.remove = cam_eeprom_platform_driver_remove,
};

static const struct i2c_device_id cam_eeprom_i2c_id[] = {
	{ "msm_eeprom", (kernel_ulong_t)NULL},
	{ }
};

static struct i2c_driver cam_eeprom_i2c_driver = {
	.id_table = cam_eeprom_i2c_id,
	.probe  = cam_eeprom_i2c_driver_probe,
	.remove = cam_eeprom_i2c_driver_remove,
	.driver = {
		.name = "msm_eeprom",
	},
};

static struct spi_driver cam_eeprom_spi_driver = {
	.driver = {
		.name = "qcom_eeprom",
		.owner = THIS_MODULE,
		.of_match_table = cam_eeprom_dt_match,
	},
	.probe = cam_eeprom_spi_driver_probe,
	.remove = cam_eeprom_spi_driver_remove,
};
int cam_eeprom_driver_init(void)
{
	int rc = 0;

	rc = platform_driver_register(&cam_eeprom_platform_driver);
	if (rc < 0) {
		CAM_ERR(CAM_EEPROM, "platform_driver_register failed rc = %d",
			rc);
		return rc;
	}

	rc = spi_register_driver(&cam_eeprom_spi_driver);
	if (rc < 0) {
		CAM_ERR(CAM_EEPROM, "spi_register_driver failed rc = %d", rc);
		return rc;
	}

	rc = i2c_add_driver(&cam_eeprom_i2c_driver);
	if (rc < 0) {
		CAM_ERR(CAM_EEPROM, "i2c_add_driver failed rc = %d", rc);
		return rc;
	}

	return rc;
}

void cam_eeprom_driver_exit(void)
{
	platform_driver_unregister(&cam_eeprom_platform_driver);
	spi_unregister_driver(&cam_eeprom_spi_driver);
	i2c_del_driver(&cam_eeprom_i2c_driver);
}

MODULE_DESCRIPTION("CAM EEPROM driver");
MODULE_LICENSE("GPL v2");
