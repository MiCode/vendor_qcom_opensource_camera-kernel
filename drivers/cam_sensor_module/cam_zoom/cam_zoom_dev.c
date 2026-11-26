// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include "cam_zoom_dev.h"
#include "cam_req_mgr_dev.h"
#include "cam_zoom_soc.h"
#include "cam_zoom_core.h"
#include "cam_trace.h"
#include "camera_main.h"
#include "cam_compat.h"
#include "cam_mem_mgr_api.h"
/* xiaomi add for cci debug start */
#include "cam_cci_debug_util.h"
/* xiaomi add for cci debug end */

/* xiaomi dev protection add*/
#include "xm_cam_dev_protection.h"

struct xm_cam_dev_info* get_zoom_xm_cam_dev_info(struct cam_zoom_ctrl_t *z_ctrl)
{
	if (NULL == z_ctrl) {
		return NULL;
	}
	return &(z_ctrl->xm_cam_dev_info_data);
}
/* xiaomi dev protection add*/

static struct cam_i3c_zoom_data {
	struct cam_zoom_ctrl_t                  *z_ctrl;
	struct completion                            probe_complete;
} g_i3c_zoom_data[MAX_CAMERAS];

struct completion *cam_zoom_get_i3c_completion(uint32_t index)
{
	return &g_i3c_zoom_data[index].probe_complete;
}

static int cam_zoom_subdev_close_internal(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	struct cam_zoom_ctrl_t *z_ctrl =
		v4l2_get_subdevdata(sd);

	if (!z_ctrl) {
		CAM_ERR(CAM_ZOOM, "z_ctrl ptr is NULL");
		return -EINVAL;
	}

	mutex_lock(&(z_ctrl->zoom_mutex));
	cam_zoom_shutdown(z_ctrl);
	mutex_unlock(&(z_ctrl->zoom_mutex));

	return 0;
}

static int cam_zoom_subdev_close(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	bool crm_zoomive = cam_req_mgr_is_open();

	if (crm_zoomive) {
		CAM_DBG(CAM_ZOOM,
			"CRM is ZOOMIVE, close should be from CRM");
		return 0;
	}

	return cam_zoom_subdev_close_internal(sd, fh);
}

static long cam_zoom_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	int rc = 0;
	struct cam_zoom_ctrl_t *z_ctrl =
		v4l2_get_subdevdata(sd);

	switch (cmd) {
	case VIDIOC_CAM_CONTROL:
		rc = cam_zoom_driver_cmd(z_ctrl, arg);
		if (rc) {
			if (rc == -EBADR)
				CAM_INFO(CAM_ZOOM,
					"Failed for driver_cmd: %d, it has been flushed",
					rc);
			else
				CAM_ERR(CAM_ZOOM,
					"Failed for driver_cmd: %d", rc);
		}
		break;
	case CAM_SD_SHUTDOWN:
		if (!cam_req_mgr_is_shutdown()) {
			CAM_ERR(CAM_CORE, "SD shouldn't come from user space");
			return 0;
		}

		rc = cam_zoom_subdev_close_internal(sd, NULL);
		break;
	default:
		CAM_ERR(CAM_ZOOM, "Invalid ioctl cmd: %u", cmd);
		rc = -ENOIOCTLCMD;
		break;
	}
	return rc;
}

#ifdef CONFIG_COMPAT
static long cam_zoom_init_subdev_do_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, unsigned long arg)
{
	struct cam_control cmd_data;
	int32_t rc = 0;

	if (copy_from_user(&cmd_data, (void __user *)arg,
		sizeof(cmd_data))) {
		CAM_ERR(CAM_ZOOM,
			"Failed to copy from user_ptr=%pK size=%zu",
			(void __user *)arg, sizeof(cmd_data));
		return -EFAULT;
	}

	switch (cmd) {
	case VIDIOC_CAM_CONTROL:
		cmd = VIDIOC_CAM_CONTROL;
		rc = cam_zoom_subdev_ioctl(sd, cmd, &cmd_data);
		if (rc) {
			CAM_ERR(CAM_ZOOM,
				"Failed in zoom subdev handling rc: %d",
				rc);
			return rc;
		}
		break;
	default:
		CAM_ERR(CAM_ZOOM, "Invalid compat ioctl: %d", cmd);
		rc = -ENOIOCTLCMD;
		break;
	}

	if (!rc) {
		if (copy_to_user((void __user *)arg, &cmd_data,
			sizeof(cmd_data))) {
			CAM_ERR(CAM_ZOOM,
				"Failed to copy to user_ptr=%pK size=%zu",
				(void __user *)arg, sizeof(cmd_data));
			rc = -EFAULT;
		}
	}
	return rc;
}
#endif

static struct v4l2_subdev_core_ops cam_zoom_subdev_core_ops = {
	.ioctl = cam_zoom_subdev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = cam_zoom_init_subdev_do_ioctl,
#endif
};

static struct v4l2_subdev_ops cam_zoom_subdev_ops = {
	.core = &cam_zoom_subdev_core_ops,
};

static const struct v4l2_subdev_internal_ops cam_zoom_internal_ops = {
	.close = cam_zoom_subdev_close,
};

static int cam_zoom_init_subdev(struct cam_zoom_ctrl_t *z_ctrl)
{
	int rc = 0;

	z_ctrl->v4l2_dev_str.internal_ops =
		&cam_zoom_internal_ops;
	z_ctrl->v4l2_dev_str.ops =
		&cam_zoom_subdev_ops;
	strscpy(z_ctrl->device_name, CAMX_ZOOM_DEV_NAME,
		sizeof(z_ctrl->device_name));
	z_ctrl->v4l2_dev_str.name =
		z_ctrl->device_name;
	z_ctrl->v4l2_dev_str.sd_flags =
		(V4L2_SUBDEV_FL_HAS_DEVNODE | V4L2_SUBDEV_FL_HAS_EVENTS);
	z_ctrl->v4l2_dev_str.ent_function =
		CAM_ZOOM_DEVICE_TYPE;
	z_ctrl->v4l2_dev_str.token = z_ctrl;
	z_ctrl->v4l2_dev_str.close_seq_prior =
		 CAM_SD_CLOSE_MEDIUM_PRIORITY;

	rc = cam_register_subdev(&(z_ctrl->v4l2_dev_str));
	if (rc)
		CAM_ERR(CAM_ZOOM,
			"Fail with cam_register_subdev rc: %d", rc);

	return rc;
}

static int cam_zoom_i2c_component_bind(struct device *dev,
	struct device *master_dev, void *data)
{
	int32_t                          rc = 0;
	int32_t                          i = 0;
	struct i2c_client               *client;
	struct cam_zoom_ctrl_t      *z_ctrl;
	struct cam_hw_soc_info          *soc_info = NULL;
	struct cam_zoom_soc_private *soc_private = NULL;
	struct device_node              *np = NULL;
	const char                      *drv_name;

	client = container_of(dev, struct i2c_client, dev);
	if (!client) {
		CAM_ERR(CAM_ZOOM,
			"Failed to get i2c client");
		return -EFAULT;
	}

	/* Create sensor control structure */
	z_ctrl = CAM_MEM_ZALLOC(sizeof(*z_ctrl), GFP_KERNEL);
	if (!z_ctrl)
		return -ENOMEM;

	z_ctrl->io_master_info.qup_client = CAM_MEM_ZALLOC(sizeof(
		struct cam_sensor_qup_client), GFP_KERNEL);
	if (!(z_ctrl->io_master_info.qup_client)) {
		rc = -ENOMEM;
		goto free_ctrl;
	}

	i2c_set_clientdata(client, z_ctrl);

	soc_private = CAM_MEM_ZALLOC(sizeof(struct cam_zoom_soc_private),
		GFP_KERNEL);
	if (!soc_private) {
		rc = -ENOMEM;
		goto free_qup;
	}
	z_ctrl->soc_info.soc_private = soc_private;

	z_ctrl->io_master_info.qup_client->i2c_client = client;
	soc_info = &z_ctrl->soc_info;
	soc_info->dev = &client->dev;
	soc_info->dev_name = client->name;
	z_ctrl->io_master_info.master_type = I2C_MASTER;

	np = of_node_get(client->dev.of_node);
	drv_name = of_node_full_name(np);

	rc = cam_zoom_parse_dt(z_ctrl, &client->dev);
	if (rc < 0) {
		CAM_ERR(CAM_ZOOM, "failed: cam_sensor_parse_dt rc %d", rc);
		goto free_soc;
	}

	rc = cam_zoom_init_subdev(z_ctrl);
	if (rc)
		goto free_soc;

	if (soc_private->i2c_info.slave_addr != 0)
		z_ctrl->io_master_info.qup_client->i2c_client->addr =
			soc_private->i2c_info.slave_addr;

	z_ctrl->i2c_data.per_frame =
		CAM_MEM_ZALLOC(sizeof(struct i2c_settings_array) *
		MAX_PER_FRAME_ARRAY, GFP_KERNEL);
	if (z_ctrl->i2c_data.per_frame == NULL) {
		rc = -ENOMEM;
		goto unreg_subdev;
	}

	cam_sensor_module_add_i2c_device((void *) z_ctrl, CAM_SENSOR_ZOOM);
	INIT_LIST_HEAD(&(z_ctrl->i2c_data.init_settings.list_head));
	INIT_LIST_HEAD(&(z_ctrl->i2c_data.config_settings.list_head));

	for (i = 0; i < MAX_PER_FRAME_ARRAY; i++)
		INIT_LIST_HEAD(&(z_ctrl->i2c_data.per_frame[i].list_head));

	z_ctrl->bridge_intf.device_hdl = -1;
	z_ctrl->bridge_intf.link_hdl = -1;
	z_ctrl->bridge_intf.ops.get_dev_info =
		cam_zoom_publish_dev_info;
	z_ctrl->bridge_intf.ops.link_setup =
		cam_zoom_establish_link;
	z_ctrl->bridge_intf.ops.apply_req =
		cam_zoom_apply_request;
	z_ctrl->last_flush_req = 0;
	z_ctrl->cam_zoom_state = CAM_ZOOM_INIT;
	of_node_put(np);

	/* xiaomi dev protection add*/
	xm_cam_dev_init_dev_info(get_zoom_xm_cam_dev_info(z_ctrl), XM_CAM_DEV_TYPE_ZOOM);
	/* xiaomi dev protection add*/

	return rc;

unreg_subdev:
	cam_unregister_subdev(&(z_ctrl->v4l2_dev_str));
free_soc:
	CAM_MEM_FREE(soc_private);
free_qup:
	CAM_MEM_FREE(z_ctrl->io_master_info.qup_client);
free_ctrl:
	CAM_MEM_FREE(z_ctrl);
	return rc;
}

static void cam_zoom_i2c_component_unbind(struct device *dev,
	struct device *master_dev, void *data)
{
	struct i2c_client               *client = NULL;
	struct cam_zoom_ctrl_t      *z_ctrl = NULL;

	client = container_of(dev, struct i2c_client, dev);
	if (!client) {
		CAM_ERR(CAM_ZOOM,
			"Failed to get i2c client");
		return;
	}

	z_ctrl = i2c_get_clientdata(client);
	/* Handle I2C Devices */
	if (!z_ctrl) {
		CAM_ERR(CAM_ZOOM, "Zoom device is NULL");
		return;
	}

	CAM_INFO(CAM_ZOOM, "i2c remove invoked");
	mutex_lock(&(z_ctrl->zoom_mutex));
	cam_zoom_shutdown(z_ctrl);
	mutex_unlock(&(z_ctrl->zoom_mutex));
	cam_unregister_subdev(&(z_ctrl->v4l2_dev_str));

	/*Free Allocated Mem */
	CAM_MEM_FREE(z_ctrl->i2c_data.per_frame);
	z_ctrl->i2c_data.per_frame = NULL;
	/* xiaomi dev protection add*/
	xm_cam_dev_destroy_dev_info(get_zoom_xm_cam_dev_info(z_ctrl));
	/* xiaomi dev protection add*/
	v4l2_set_subdevdata(&z_ctrl->v4l2_dev_str.sd, NULL);
	CAM_MEM_FREE(z_ctrl);
}

const static struct component_ops cam_zoom_i2c_component_ops = {
	.bind = cam_zoom_i2c_component_bind,
	.unbind = cam_zoom_i2c_component_unbind,
};

#if KERNEL_VERSION(6, 2, 0) <= LINUX_VERSION_CODE
static int cam_zoom_driver_i2c_probe(struct i2c_client *client)
{
	int rc = 0;

	if (client == NULL) {
		CAM_ERR(CAM_ZOOM, "Invalid Args client: %pK",
			client);
		return -EINVAL;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		CAM_ERR(CAM_ZOOM, "%s :: i2c_check_functionality failed",
			 client->name);
		return -EFAULT;
	}

	CAM_DBG(CAM_ZOOM, "Adding sensor zoom component");

	cam_soc_util_initialize_power_domain(&client->dev);

	rc = component_add(&client->dev, &cam_zoom_i2c_component_ops);
	if (rc)
		CAM_ERR(CAM_ZOOM, "failed to add component rc: %d", rc);

	return rc;
}
#else
static int32_t cam_zoom_driver_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;

	if (client == NULL || id == NULL) {
		CAM_ERR(CAM_ZOOM, "Invalid Args client: %pK id: %pK",
			client, id);
		return -EINVAL;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		CAM_ERR(CAM_ZOOM, "%s :: i2c_check_functionality failed",
			 client->name);
		return -EFAULT;
	}

	CAM_DBG(CAM_ZOOM, "Adding sensor zoom component");

	cam_soc_util_initialize_power_domain(&client->dev);

	rc = component_add(&client->dev, &cam_zoom_i2c_component_ops);
	if (rc)
		CAM_ERR(CAM_ZOOM, "failed to add component rc: %d", rc);

	return rc;
}
#endif

#if KERNEL_VERSION(6, 1, 0) <= LINUX_VERSION_CODE
void cam_zoom_driver_i2c_remove(
	struct i2c_client *client)
{
	component_del(&client->dev, &cam_zoom_i2c_component_ops);

	cam_soc_util_uninitialize_power_domain(&client->dev);
}
#else
static int32_t cam_zoom_driver_i2c_remove(
	struct i2c_client *client)
{
	component_del(&client->dev, &cam_zoom_i2c_component_ops);

	cam_soc_util_uninitialize_power_domain(&client->dev);

	return 0;
}
#endif

static int cam_zoom_platform_component_bind(struct device *dev,
	struct device *master_dev, void *data)
{
	int32_t                          rc = 0;
	int32_t                          i = 0;
	struct cam_zoom_ctrl_t       *z_ctrl = NULL;
	struct cam_zoom_soc_private  *soc_private = NULL;
	struct platform_device *pdev = to_platform_device(dev);

	/* Create zoom control structure */
	z_ctrl = devm_kzalloc(&pdev->dev,
		sizeof(struct cam_zoom_ctrl_t), GFP_KERNEL);
	if (!z_ctrl)
		return -ENOMEM;

	/*fill in platform device*/
	z_ctrl->pdev = pdev;
	z_ctrl->v4l2_dev_str.pdev = pdev;
	z_ctrl->soc_info.pdev = pdev;
	z_ctrl->soc_info.dev = &pdev->dev;
	z_ctrl->soc_info.dev_name = pdev->name;
	z_ctrl->io_master_info.master_type = CCI_MASTER;

	z_ctrl->io_master_info.cci_client = CAM_MEM_ZALLOC(sizeof(
		struct cam_sensor_cci_client), GFP_KERNEL);
	if (!(z_ctrl->io_master_info.cci_client)) {
		rc = -ENOMEM;
		goto free_ctrl;
	}

	soc_private = CAM_MEM_ZALLOC(sizeof(struct cam_zoom_soc_private),
		GFP_KERNEL);
	if (!soc_private) {
		rc = -ENOMEM;
		goto free_cci_client;
	}
	z_ctrl->soc_info.soc_private = soc_private;
	soc_private->power_info.dev = &pdev->dev;

	z_ctrl->i2c_data.per_frame =
		CAM_MEM_ZALLOC(sizeof(struct i2c_settings_array) *
		MAX_PER_FRAME_ARRAY, GFP_KERNEL);
	if (z_ctrl->i2c_data.per_frame == NULL) {
		rc = -ENOMEM;
		goto free_soc;
	}

	cam_sensor_module_add_i2c_device((void *) z_ctrl, CAM_SENSOR_ZOOM);
	INIT_LIST_HEAD(&(z_ctrl->i2c_data.init_settings.list_head));
	INIT_LIST_HEAD(&(z_ctrl->i2c_data.config_settings.list_head));

	for (i = 0; i < MAX_PER_FRAME_ARRAY; i++)
		INIT_LIST_HEAD(&(z_ctrl->i2c_data.per_frame[i].list_head));

	rc = cam_zoom_parse_dt(z_ctrl, &(pdev->dev));
	if (rc < 0) {
		CAM_ERR(CAM_ZOOM, "Paring zoom dt failed rc %d", rc);
		goto free_mem;
	}

	/* Fill platform device id*/
	pdev->id = z_ctrl->soc_info.index;

	rc = cam_zoom_init_subdev(z_ctrl);
	if (rc)
		goto free_mem;

	z_ctrl->bridge_intf.device_hdl = -1;
	z_ctrl->bridge_intf.link_hdl = -1;
	z_ctrl->bridge_intf.ops.get_dev_info =
		cam_zoom_publish_dev_info;
	z_ctrl->bridge_intf.ops.link_setup =
		cam_zoom_establish_link;
	z_ctrl->bridge_intf.ops.apply_req =
		cam_zoom_apply_request;
	z_ctrl->bridge_intf.ops.flush_req =
		cam_zoom_flush_request;
	z_ctrl->last_flush_req = 0;

	platform_set_drvdata(pdev, z_ctrl);
	z_ctrl->cam_zoom_state = CAM_ZOOM_INIT;

	CAM_DBG(CAM_ZOOM, "Component bound successfully %d",
		z_ctrl->soc_info.index);

	g_i3c_zoom_data[z_ctrl->soc_info.index].z_ctrl = z_ctrl;
	init_completion(&g_i3c_zoom_data[z_ctrl->soc_info.index].probe_complete);

	/* xiaomi add for cci debug start */
	rc = cam_cci_dev_create_debugfs_entry(z_ctrl->device_name,
		z_ctrl->soc_info.index, CAM_ZOOM_NAME,
		&z_ctrl->io_master_info, z_ctrl->io_master_info.cci_client->cci_i2c_master,
		&z_ctrl->cci_debug);
	if (rc) {
		CAM_WARN(CAM_ZOOM, "debugfs creation failed");
		rc = 0;
	}
	/* xiaomi add for cci debug end */
	/* xiaomi dev protection add*/
	xm_cam_dev_init_dev_info(get_zoom_xm_cam_dev_info(z_ctrl), XM_CAM_DEV_TYPE_ZOOM);
	/* xiaomi dev protection add*/

	return rc;

free_mem:
	CAM_MEM_FREE(z_ctrl->i2c_data.per_frame);
free_soc:
	CAM_MEM_FREE(soc_private);
free_cci_client:
	CAM_MEM_FREE(z_ctrl->io_master_info.cci_client);
free_ctrl:
	devm_kfree(&pdev->dev, z_ctrl);
	return rc;
}

static void cam_zoom_platform_component_unbind(struct device *dev,
	struct device *master_dev, void *data)
{
	struct cam_zoom_ctrl_t      *z_ctrl;
	struct platform_device *pdev = to_platform_device(dev);

	z_ctrl = platform_get_drvdata(pdev);
	if (!z_ctrl) {
		CAM_ERR(CAM_ZOOM, "Zoom device is NULL");
		return;
	}

	mutex_lock(&(z_ctrl->zoom_mutex));
	cam_zoom_shutdown(z_ctrl);
	mutex_unlock(&(z_ctrl->zoom_mutex));
	cam_unregister_subdev(&(z_ctrl->v4l2_dev_str));
	/* xiaomi add for cci debug start */
	if (z_ctrl->io_master_info.master_type == CCI_MASTER) {
		cam_cci_dev_remove_debugfs_entry((void *)z_ctrl->cci_debug);
	}
	/* xiaomi add for cci debug end */

	CAM_MEM_FREE(z_ctrl->i2c_data.per_frame);
	z_ctrl->i2c_data.per_frame = NULL;
	/* xiaomi dev protection add*/
	xm_cam_dev_destroy_dev_info(get_zoom_xm_cam_dev_info(z_ctrl));
	/* xiaomi dev protection add*/
	v4l2_set_subdevdata(&z_ctrl->v4l2_dev_str.sd, NULL);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, z_ctrl);
	CAM_INFO(CAM_ZOOM, "Zoom component unbinded");
}

const static struct component_ops cam_zoom_platform_component_ops = {
	.bind = cam_zoom_platform_component_bind,
	.unbind = cam_zoom_platform_component_unbind,
};

#if KERNEL_VERSION(6, 1, 0) <= LINUX_VERSION_CODE
void cam_zoom_platform_remove(
	struct platform_device *pdev)
{
	component_del(&pdev->dev, &cam_zoom_platform_component_ops);

	cam_soc_util_uninitialize_power_domain(&pdev->dev);
}
#else
static int32_t cam_zoom_platform_remove(
	struct platform_device *pdev)
{
	component_del(&pdev->dev, &cam_zoom_platform_component_ops);

	cam_soc_util_uninitialize_power_domain(&pdev->dev);

	return 0;
}
#endif

static const struct of_device_id cam_zoom_driver_dt_match[] = {
	{.compatible = "qcom,zoom"},
	{}
};

static int32_t cam_zoom_driver_platform_probe(
	struct platform_device *pdev)
{
	int rc = 0;

	CAM_DBG(CAM_ZOOM, "Adding sensor zoom component");

	cam_soc_util_initialize_power_domain(&pdev->dev);

	rc = component_add(&pdev->dev, &cam_zoom_platform_component_ops);
	if (rc)
		CAM_ERR(CAM_ZOOM, "failed to add component rc: %d", rc);

	return rc;
}

MODULE_DEVICE_TABLE(of, cam_zoom_driver_dt_match);

struct platform_driver cam_zoom_platform_driver = {
	.probe = cam_zoom_driver_platform_probe,
	.driver = {
		.name = "qcom,zoom",
		.owner = THIS_MODULE,
		.of_match_table = cam_zoom_driver_dt_match,
		.suppress_bind_attrs = true,
	},
	.remove = cam_zoom_platform_remove,
};

static const struct i2c_device_id i2c_id[] = {
	{ZOOM_DRIVER_I2C, (kernel_ulong_t)NULL},
	{ }
};

static const struct of_device_id cam_zoom_i2c_driver_dt_match[] = {
	{.compatible = "qcom,cam-i2c-zoom"},
	{}
};
MODULE_DEVICE_TABLE(of, cam_zoom_i2c_driver_dt_match);

struct i2c_driver cam_zoom_i2c_driver = {
	.id_table = i2c_id,
	.probe  = cam_zoom_driver_i2c_probe,
	.remove = cam_zoom_driver_i2c_remove,
	.driver = {
		.of_match_table = cam_zoom_i2c_driver_dt_match,
		.owner = THIS_MODULE,
		.name = ZOOM_DRIVER_I2C,
		.suppress_bind_attrs = true,
	},
};

static struct i3c_device_id zoom_i3c_id[MAX_I3C_DEVICE_ID_ENTRIES + 1];

static int cam_zoom_i3c_driver_probe(struct i3c_device *client)
{
	int32_t rc = 0;
	struct cam_zoom_ctrl_t       *z_ctrl = NULL;
	uint32_t                          index;
	struct device                    *dev;

	if (!client) {
		CAM_INFO(CAM_ZOOM, "Null Client pointer");
		return -EINVAL;
	}

	dev = &client->dev;

	CAM_DBG(CAM_ZOOM, "Probe for I3C Slave %s", dev_name(dev));

	rc = of_property_read_u32(dev->of_node, "cell-index", &index);
	if (rc) {
		CAM_ERR(CAM_ZOOM, "device %s failed to read cell-index", dev_name(dev));
		return rc;
	}

	if (index >= MAX_CAMERAS) {
		CAM_ERR(CAM_ZOOM, "Invalid Cell-Index: %u for %s", index, dev_name(dev));
		return -EINVAL;
	}

	z_ctrl = g_i3c_zoom_data[index].z_ctrl;
	if (!z_ctrl) {
		CAM_ERR(CAM_ZOOM,
			"z_ctrl is null. I3C Probe before platfom driver probe for %s",
			dev_name(dev));
		return -EINVAL;
	}

	cam_sensor_utils_parse_pm_ctrl_flag(dev->of_node, &(z_ctrl->io_master_info));

	CAM_INFO(CAM_SENSOR,
		"master: %d (1-CCI, 2-I2C, 3-SPI, 4-I3C) pm_ctrl_client_enable: %d",
		z_ctrl->io_master_info.master_type,
		z_ctrl->io_master_info.qup_client->pm_ctrl_client_enable);

	z_ctrl->io_master_info.qup_client->i3c_client = client;
	z_ctrl->io_master_info.qup_client->i3c_wait_for_hotjoin = false;

	complete_all(&g_i3c_zoom_data[index].probe_complete);

	CAM_DBG(CAM_ZOOM, "I3C Probe Finished for %s", dev_name(dev));
	return rc;
}

#if (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
static void cam_i3c_driver_remove(struct i3c_device *client)
{
	int32_t                        rc = 0;
	struct cam_zoom_ctrl_t     *z_ctrl = NULL;
	struct device                  *dev;
	uint32_t                       index;

	if (!client) {
		CAM_ERR(CAM_SENSOR, "I3C Driver Remove: Invalid input args");
		return;
	}

	dev = &client->dev;

	CAM_DBG(CAM_SENSOR, "driver remove for I3C Slave %s", dev_name(dev));

	rc = of_property_read_u32(dev->of_node, "cell-index", &index);
	if (rc) {
		CAM_ERR(CAM_UTIL, "device %s failed to read cell-index", dev_name(dev));
		return;
	}

	if (index >= MAX_CAMERAS) {
		CAM_ERR(CAM_SENSOR, "Invalid Cell-Index: %u for %s", index, dev_name(dev));
		return;
	}

	z_ctrl = g_i3c_zoom_data[index].z_ctrl;
	if (!z_ctrl) {
		CAM_ERR(CAM_SENSOR, "z_ctrl is null. I3C Probe before platfom driver probe for %s",
			dev_name(dev));
		return;
	}

	CAM_DBG(CAM_SENSOR, "I3C remove invoked for %s",
		(client ? dev_name(&client->dev) : "none"));
	CAM_MEM_FREE(z_ctrl->io_master_info.qup_client);
	z_ctrl->io_master_info.qup_client = NULL;
}

#else
static int cam_i3c_driver_remove(struct i3c_device *client)
{
	struct cam_zoom_ctrl_t     *z_ctrl = NULL;
	struct device                  *dev;
	uint32_t                       index;

	if (!client) {
		CAM_ERR(CAM_SENSOR, "I3C Driver Remove: Invalid input args");
		return -EINVAL;
	}

	dev = &client->dev;

	CAM_DBG(CAM_SENSOR, "driver remove for I3C Slave %s", dev_name(dev));

	rc = of_property_read_u32(dev->of_node, "cell-index", &index);
	if (rc) {
		CAM_ERR(CAM_UTIL, "device %s failed to read cell-index", dev_name(dev));
		return -EINVAL;
	}

	if (index >= MAX_CAMERAS) {
		CAM_ERR(CAM_SENSOR, "Invalid Cell-Index: %u for %s", index, dev_name(dev));
		return -EINVAL;
	}

	z_ctrl = g_i3c_zoom_data[index].z_ctrl;
	if (!z_ctrl) {
		CAM_ERR(CAM_SENSOR, "z_ctrl is null. I3C Probe before platfom driver probe for %s",
			dev_name(dev));
		return -EINVAL;
	}

	CAM_DBG(CAM_SENSOR, "I3C remove invoked for %s",
		(client ? dev_name(&client->dev) : "none"));
	CAM_MEM_FREE(z_ctrl->io_master_info.qup_client);
	z_ctrl->io_master_info.qup_client = NULL;
	return 0;
}
#endif

static struct i3c_driver cam_zoom_i3c_driver = {
	.id_table = zoom_i3c_id,
	.probe = cam_zoom_i3c_driver_probe,
	.remove = cam_i3c_driver_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = ZOOM_DRIVER_I3C,
		.of_match_table = cam_zoom_driver_dt_match,
		.suppress_bind_attrs = true,
	},
};

int cam_zoom_driver_init(void)
{
	int32_t rc = 0;
	struct device_node                      *dev;
	int num_entries = 0;

	rc = platform_driver_register(&cam_zoom_platform_driver);
	if (rc < 0) {
		CAM_ERR(CAM_ZOOM,
			"platform_driver_register failed rc = %d", rc);
		return rc;
	}

	rc = i2c_add_driver(&cam_zoom_i2c_driver);
	if (rc) {
		CAM_ERR(CAM_ZOOM, "i2c_add_driver failed rc = %d", rc);
		goto i2c_register_err;
	}

	memset(zoom_i3c_id, 0, sizeof(struct i3c_device_id) * (MAX_I3C_DEVICE_ID_ENTRIES + 1));

	dev = of_find_node_by_path(I3C_SENSOR_DEV_ID_DT_PATH);
	if (!dev) {
		CAM_DBG(CAM_ZOOM, "Couldnt Find the i3c-id-table dev node");
		return 0;
	}

	rc = cam_sensor_count_elems_i3c_device_id(dev, &num_entries,
		"i3c-zoom-id-table");
	if (rc)
		return 0;

	rc = cam_sensor_fill_i3c_device_id(dev, num_entries,
		"i3c-zoom-id-table", zoom_i3c_id);
	if (rc)
		goto i3c_register_err;

	rc = i3c_driver_register_with_owner(&cam_zoom_i3c_driver, THIS_MODULE);
	if (rc) {
		CAM_ERR(CAM_ZOOM, "i3c_driver registration failed, rc: %d", rc);
		goto i3c_register_err;
	}

	return 0;

i3c_register_err:
	i2c_del_driver(&cam_zoom_i2c_driver);
i2c_register_err:
	platform_driver_unregister(&cam_zoom_platform_driver);

	return rc;
}

void cam_zoom_driver_exit(void)
{
	struct device_node *dev;

	platform_driver_unregister(&cam_zoom_platform_driver);
	i2c_del_driver(&cam_zoom_i2c_driver);

	dev = of_find_node_by_path(I3C_SENSOR_DEV_ID_DT_PATH);
	if (!dev) {
		CAM_DBG(CAM_ZOOM, "Couldnt Find the i3c-id-table dev node");
		return;
	}

	i3c_driver_unregister(&cam_zoom_i3c_driver);
}

MODULE_DESCRIPTION("cam_zoom_driver");
MODULE_LICENSE("GPL v2");
