// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 */

#include <linux/module.h>
#include "cam_flash_dev.h"
#include "cam_flash_soc.h"
#include "cam_flash_core.h"
#include "cam_common_util.h"
#include "camera_main.h"
#define WT_FLASHLIGHT_DEVNAME		   "factory_test_flash"
int flash_state = 0;

//add for pwm test start
int flash_mode = 0;
int pwm_period = 50000;
int pwm_duty = 80;
//add for pwm test end

//add for breath test start
int dutyBreathStart = 1800;
int dutyBreathEnd = 50000;
int dutyChange = 600;
//add for breath test end

struct platform_device *pdev_factory_test_flash;
#ifdef CONFIG_FLASHLIGHT_PWM
extern int cam_gpio_flash_on(struct cam_flash_ctrl *flash_ctrl,struct pwm_setting * pwm);
extern int cam_gpio_flash_off(struct cam_flash_ctrl *flash_ctrl,struct pwm_setting * pwm);
extern void cam_torch_on(struct cam_flash_ctrl *flash_ctrl, struct pwm_setting *pwm);
extern void cam_torch_off(struct cam_flash_ctrl *flash_ctrl,struct pwm_setting *pwm);

static ssize_t show_flashduty(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", flash_state);
}
static ssize_t store_flashduty(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct cam_flash_ctrl *fctrl = NULL;
	struct pwm_setting pwm;
	int dutyCnt = 0;
	CAM_DBG(CAM_FLASH, "Enter!\n");
	flash_state = simple_strtol(buf, NULL, 10);
	CAM_DBG(CAM_FLASH, "torch:set flash_state= %d\n", flash_state);
	fctrl = platform_get_drvdata(pdev_factory_test_flash);
	switch(flash_state) {
		case 0: //torch mode off
			pwm.period_ns = 50000;
			pwm.duty_ns = 50000;
			cam_torch_off(fctrl,&pwm);
		break;

		case 1: //torch mode on
			pwm.period_ns = 50000;
			pwm.duty_ns = 50000;
			cam_torch_on(fctrl,&pwm);
		break;

		case 2: //flash mode on-off
			pwm.period_ns = 50000;
			pwm.duty_ns = 50000;
			cam_gpio_flash_on(fctrl,&pwm);
			msleep(700);
			cam_gpio_flash_off(fctrl,&pwm);
			msleep(1000);

			pwm.duty_ns = 30000;
			cam_gpio_flash_on(fctrl,&pwm);
			msleep(700);
			cam_gpio_flash_off(fctrl,&pwm);
			msleep(1000);

			pwm.duty_ns = 10000;
			cam_gpio_flash_on(fctrl,&pwm);
			msleep(700);
			cam_gpio_flash_off(fctrl,&pwm);
		break;
		case 3: //torch mode switch pwm on
			pwm.period_ns = 50000;
			pwm.duty_ns = 50000;
			cam_torch_on(fctrl,&pwm);
			msleep(1000);

			pwm.duty_ns = 30000;
			cam_torch_on(fctrl,&pwm);
			msleep(1000);

			pwm.duty_ns = 10000;
			cam_torch_on(fctrl,&pwm);
		break;

//add for pwm test start
		case 4:
			if(!flash_mode) {
				CAM_DBG(CAM_FLASH, "torch mode pwm test!\n");
				pwm.period_ns = pwm_period;
				pwm.duty_ns = pwm_duty;
				cam_torch_on(fctrl,&pwm);
			}
			else {
				CAM_DBG(CAM_FLASH, "flash mode pwm test!\n");
				pwm.period_ns = pwm_period;
				pwm.duty_ns = pwm_duty;
				cam_gpio_flash_on(fctrl,&pwm);
				msleep(700);
				cam_gpio_flash_off(fctrl,&pwm);
			}
		break;
//add for pwm test end

//add for breath test start
		case 5:
			pwm.period_ns = pwm_period;
			pwm.duty_ns = pwm_period;
			cam_torch_on(fctrl,&pwm);
			msleep(5);
			dutyCnt = dutyBreathStart; //1800 50000*3.6%;1400 50000*2.8%
			while(dutyCnt <= dutyBreathEnd) { //50000 50000*100%
				pwm.duty_ns = dutyCnt;
				CAM_ERR(CAM_FLASH, "breath up change duty_ns %d",pwm.duty_ns);
				cam_torch_on(fctrl,&pwm);
				msleep(20);	//wait 20ms in loop
				dutyCnt += dutyChange; //600 50000*1.2%
			}
			dutyCnt = dutyBreathEnd;
			msleep(200); //wait 200ms
			while(dutyCnt >= dutyBreathStart) { //1800 50000*3.6%;200 50000*0.4%
				pwm.duty_ns = dutyCnt;
				CAM_ERR(CAM_FLASH, "breath down change duty_ns %d",pwm.duty_ns);
				cam_torch_on(fctrl,&pwm);
				msleep(20);
				dutyCnt -= dutyChange;
			}
		break;
//add for breath test end

        default:
            cam_torch_off(fctrl,&pwm);
		break;
	}
	CAM_DBG(CAM_FLASH, "Exit!\n");
	return count;
}
static DEVICE_ATTR(rear_flash, 0664, show_flashduty, store_flashduty);

static ssize_t pwm_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", flash_mode);
}
static ssize_t pwm_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	CAM_DBG(CAM_FLASH, "Enter!\n");
	flash_mode = simple_strtol(buf, NULL, 10);
	CAM_DBG(CAM_FLASH, "[pwm_mode_store]set flash_mode= %d\n", flash_mode);
	CAM_DBG(CAM_FLASH, "Exit!\n");
	return count;
}
static DEVICE_ATTR(pwm_mode_flash, 0664, pwm_mode_show, pwm_mode_store);

static ssize_t pwm_duty_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", pwm_duty);
}
static ssize_t pwm_duty_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	CAM_DBG(CAM_FLASH, "Enter!\n");
	pwm_duty = simple_strtol(buf, NULL, 10);
	CAM_DBG(CAM_FLASH, "[pwm_duty_store]set pwm_duty= %d\n", pwm_duty);
	CAM_DBG(CAM_FLASH, "Exit!\n");
	return count;
}
static DEVICE_ATTR(pwm_duty_flash, 0664, pwm_duty_show, pwm_duty_store);

static ssize_t pwm_period_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", pwm_period);
}
static ssize_t pwm_period_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	CAM_DBG(CAM_FLASH, "Enter!\n");
	pwm_period = simple_strtol(buf, NULL, 10);
	CAM_DBG(CAM_FLASH, "[pwm_period_store]set pwm_period= %d\n", pwm_period);
	CAM_DBG(CAM_FLASH, "Exit!\n");
	return count;
}
static DEVICE_ATTR(pwm_period_flash, 0664, pwm_period_show, pwm_period_store);


static ssize_t breath_start_duty_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", dutyBreathStart);
}
static ssize_t breath_start_duty_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	CAM_DBG(CAM_FLASH, "Enter!\n");
	dutyBreathStart = simple_strtol(buf, NULL, 10);
	CAM_DBG(CAM_FLASH, "[breath_start_duty_store]set dutyBreathStart= %d\n", dutyBreathStart);
	CAM_DBG(CAM_FLASH, "Exit!\n");
	return count;
}
static DEVICE_ATTR(breath_start_duty, 0664, breath_start_duty_show, breath_start_duty_store);

static ssize_t breath_end_duty_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", dutyBreathEnd);
}
static ssize_t breath_end_duty_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	CAM_DBG(CAM_FLASH, "Enter!\n");
	dutyBreathEnd = simple_strtol(buf, NULL, 10);
	CAM_DBG(CAM_FLASH, "[breath_end_duty_store]set dutyBreathEnd= %d\n", dutyBreathEnd);
	CAM_DBG(CAM_FLASH, "Exit!\n");
	return count;
}
static DEVICE_ATTR(breath_end_duty, 0664, breath_end_duty_show, breath_end_duty_store);

static ssize_t breath_climb_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", dutyChange);
}
static ssize_t breath_climb_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	CAM_DBG(CAM_FLASH, "Enter!\n");
	dutyChange = simple_strtol(buf, NULL, 10);
	CAM_DBG(CAM_FLASH, "[breath_climb_store]set dutyChange= %d\n", dutyChange);
	CAM_DBG(CAM_FLASH, "Exit!\n");
	return count;
}
static DEVICE_ATTR(breath_climb, 0664, breath_climb_show, breath_climb_store);

static int cam_flash_factory_test_creat()
{
	static struct class *wt_flashlight_class;
	static struct device *wt_flashlight_device;

	wt_flashlight_class = class_create(THIS_MODULE, "camera");   //  /sys/class/camera
	if (IS_ERR(wt_flashlight_class)) {
		CAM_ERR(CAM_FLASH, "[flashlight_probe] Unable to create class, err = %d ~",
			(int)PTR_ERR(wt_flashlight_class));
		return -1 ;
	}
	wt_flashlight_device =
		device_create(wt_flashlight_class, NULL, MKDEV(0,1), NULL, WT_FLASHLIGHT_DEVNAME);  //   /sys/class/camera/factory_test_flash
	if (NULL == wt_flashlight_device) {
		CAM_ERR(CAM_FLASH, "[flashlight_probe] device_create fail ~");
	}
	if (device_create_file(wt_flashlight_device,&dev_attr_rear_flash)) { // /sys/class/camera/factory_test_flash/rear_flash
		CAM_ERR(CAM_FLASH, "[flashlight_probe]device_create_file rear_flash fail!\n");
	}
	if (device_create_file(wt_flashlight_device,&dev_attr_pwm_mode_flash)) { // /sys/class/camera/factory_test_flash/pwm_mode_flash
		CAM_ERR(CAM_FLASH, "[flashlight_probe]device_create_file pwm_mode_flash fail!\n");
	}
	if (device_create_file(wt_flashlight_device,&dev_attr_pwm_period_flash)) { // /sys/class/camera/factory_test_flash/pwm_period_flash
		CAM_ERR(CAM_FLASH, "[flashlight_probe]device_create_file pwm_period_flash fail!\n");
	}
	if (device_create_file(wt_flashlight_device,&dev_attr_pwm_duty_flash)) { // /sys/class/camera/factory_test_flash/pwm_duty_flash
		CAM_ERR(CAM_FLASH, "[flashlight_probe]device_create_file pwm_duty_flash fail!\n");
	}
	if (device_create_file(wt_flashlight_device,&dev_attr_breath_start_duty)) { // /sys/class/camera/factory_test_flash/breath_start_duty
		CAM_ERR(CAM_FLASH, "[flashlight_probe]device_create_file breath_start_duty fail!\n");
	}
	if (device_create_file(wt_flashlight_device,&dev_attr_breath_end_duty)) { // /sys/class/camera/factory_test_flash/breath_end_duty
		CAM_ERR(CAM_FLASH, "[flashlight_probe]device_create_file breath_end_duty fail!\n");
	}
	if (device_create_file(wt_flashlight_device,&dev_attr_breath_climb)) { // /sys/class/camera/factory_test_flash/breath_climb
		CAM_ERR(CAM_FLASH, "[flashlight_probe]device_create_file breath_climb fail!\n");
	}
	return 0;
}
#endif
static int32_t cam_flash_driver_cmd(struct cam_flash_ctrl *fctrl,
		void *arg, struct cam_flash_private_soc *soc_private)
{
	int rc = 0;
	int i = 0;
	struct cam_control *cmd = (struct cam_control *)arg;

	if (!fctrl || !arg) {
		CAM_ERR(CAM_FLASH, "fctrl/arg is NULL with arg:%pK fctrl%pK",
			fctrl, arg);
		return -EINVAL;
	}

	if (cmd->handle_type != CAM_HANDLE_USER_POINTER) {
		CAM_ERR(CAM_FLASH, "Invalid handle type: %d",
			cmd->handle_type);
		return -EINVAL;
	}

	mutex_lock(&(fctrl->flash_mutex));
	switch (cmd->op_code) {
	case CAM_ACQUIRE_DEV: {
		struct cam_sensor_acquire_dev flash_acq_dev;
		struct cam_create_dev_hdl bridge_params;

		if (fctrl->flash_state != CAM_FLASH_STATE_INIT) {
			CAM_ERR(CAM_FLASH,
				"Cannot apply Acquire dev: Prev state: %d",
				fctrl->flash_state);
			rc = -EINVAL;
			goto release_mutex;
		}

		if (fctrl->bridge_intf.device_hdl != -1) {
			CAM_ERR(CAM_FLASH, "Device is already acquired");
			rc = -EINVAL;
			goto release_mutex;
		}

		rc = copy_from_user(&flash_acq_dev,
			u64_to_user_ptr(cmd->handle),
			sizeof(flash_acq_dev));
		if (rc) {
			CAM_ERR(CAM_FLASH, "Failed Copying from User");
			goto release_mutex;
		}

		bridge_params.session_hdl = flash_acq_dev.session_handle;
		bridge_params.ops = &fctrl->bridge_intf.ops;
		bridge_params.v4l2_sub_dev_flag = 0;
		bridge_params.media_entity_flag = 0;
		bridge_params.priv = fctrl;
		bridge_params.dev_id = CAM_FLASH;

		flash_acq_dev.device_handle =
			cam_create_device_hdl(&bridge_params);
		if (flash_acq_dev.device_handle <= 0) {
			rc = -EFAULT;
			CAM_ERR(CAM_FLASH, "Can not create device handle");
			goto release_mutex;
		}
		fctrl->bridge_intf.device_hdl =
			flash_acq_dev.device_handle;
		fctrl->bridge_intf.session_hdl =
			flash_acq_dev.session_handle;
		fctrl->apply_streamoff = false;

		rc = copy_to_user(u64_to_user_ptr(cmd->handle),
			&flash_acq_dev,
			sizeof(struct cam_sensor_acquire_dev));
		if (rc) {
			CAM_ERR(CAM_FLASH, "Failed Copy to User with rc = %d",
				rc);
			rc = -EFAULT;
			goto release_mutex;
		}
		if (fctrl->func_tbl.power_ops == cam_flash_pmic_power_ops) {
			if (fctrl->func_tbl.power_ops(fctrl, true))
				CAM_WARN(CAM_FLASH, "Power up Failed");
		}
		fctrl->flash_state = CAM_FLASH_STATE_ACQUIRE;

		CAM_INFO(CAM_FLASH, "CAM_ACQUIRE_DEV for dev_hdl: 0x%x",
			fctrl->bridge_intf.device_hdl);
		break;
	}
	case CAM_RELEASE_DEV: {
		CAM_INFO(CAM_FLASH, "CAM_RELEASE_DEV for dev_hdl: 0x%x",
			fctrl->bridge_intf.device_hdl);
		if ((fctrl->flash_state == CAM_FLASH_STATE_INIT) ||
			(fctrl->flash_state == CAM_FLASH_STATE_START)) {
			CAM_WARN(CAM_FLASH,
				"Wrong state for Release dev: Prev state:%d",
				fctrl->flash_state);
		}

		if (fctrl->bridge_intf.device_hdl == -1 &&
			fctrl->flash_state == CAM_FLASH_STATE_ACQUIRE) {
			CAM_ERR(CAM_FLASH,
				"Invalid Handle: Link Hdl: %d device hdl: %d",
				fctrl->bridge_intf.device_hdl,
				fctrl->bridge_intf.link_hdl);
			rc = -EINVAL;
			goto release_mutex;
		}

		if (fctrl->bridge_intf.link_hdl != -1) {
			CAM_ERR(CAM_FLASH,
				"Device [%d] still active on link 0x%x",
				fctrl->flash_state,
				fctrl->bridge_intf.link_hdl);
			rc = -EAGAIN;
			goto release_mutex;
		}

		if ((fctrl->flash_state == CAM_FLASH_STATE_CONFIG) ||
			(fctrl->flash_state == CAM_FLASH_STATE_START))
			fctrl->func_tbl.flush_req(fctrl, FLUSH_ALL, 0);

		if (cam_flash_release_dev(fctrl))
			CAM_WARN(CAM_FLASH,
				"Failed in destroying the device Handle");

		if (fctrl->func_tbl.power_ops) {
			if (fctrl->func_tbl.power_ops(fctrl, false))
				CAM_WARN(CAM_FLASH, "Power Down Failed");
		}

		fctrl->streamoff_count = 0;
		fctrl->flash_state = CAM_FLASH_STATE_INIT;
		break;
	}
	case CAM_QUERY_CAP: {
		struct cam_flash_query_cap_info flash_cap = {0};

		CAM_DBG(CAM_FLASH, "CAM_QUERY_CAP_V1");
		flash_cap.slot_info  = fctrl->soc_info.index;
		for (i = 0; i < fctrl->flash_num_sources; i++) {
			flash_cap.max_current_flash[i] =
				soc_private->flash_max_current[i];
			flash_cap.max_duration_flash[i] =
				soc_private->flash_max_duration[i];
		}

		for (i = 0; i < fctrl->torch_num_sources; i++)
			flash_cap.max_current_torch[i] =
				soc_private->torch_max_current[i];

		if (copy_to_user(u64_to_user_ptr(cmd->handle),
			&flash_cap, sizeof(struct cam_flash_query_cap_info))) {
			CAM_ERR(CAM_FLASH, "Failed Copy to User");
			rc = -EFAULT;
			goto release_mutex;
		}
		fctrl->apply_streamoff = false;
		fctrl->flash_state = CAM_FLASH_STATE_START;
		break;
	}

	case CAM_QUERY_CAP_V2: {
		struct cam_flash_query_cap_info_v2 flash_cap = {0};

		CAM_DBG(CAM_FLASH, "CAM_QUERY_CAP_V2");
		flash_cap.slot_info  = fctrl->soc_info.index;
		flash_cap.flash_type = soc_private->flash_type;
		flash_cap.version = 1;
		flash_cap.num_valid_params = 0;
		CAM_DBG(CAM_FLASH, "ts1218 dts flash_type is %d, it should same with camx&chi flash_type",
		 soc_private->flash_type);
		CAM_DBG(CAM_FLASH, "ts1218 get flash_enabel_gpio %d", soc_private->flash_gpio_enable);
		for (i = 0; i < fctrl->flash_num_sources; i++) {
			flash_cap.max_current_flash[i] =
				soc_private->flash_max_current[i];
			flash_cap.max_duration_flash[i] =
				soc_private->flash_max_duration[i];
		}

		for (i = 0; i < fctrl->torch_num_sources; i++)
			flash_cap.max_current_torch[i] =
				soc_private->torch_max_current[i];

		if (copy_to_user(u64_to_user_ptr(cmd->handle),
			&flash_cap, sizeof(struct cam_flash_query_cap_info_v2))) {
			CAM_ERR(CAM_FLASH, "Failed Copy to User");
			rc = -EFAULT;
			goto release_mutex;
		}
		break;
	}


	case CAM_START_DEV: {
		CAM_INFO(CAM_FLASH, "CAM_START_DEV for dev_hdl: 0x%x",
			fctrl->bridge_intf.device_hdl);
		if ((fctrl->flash_state == CAM_FLASH_STATE_INIT) ||
			(fctrl->flash_state == CAM_FLASH_STATE_START)) {
			CAM_WARN(CAM_FLASH,
				"Cannot apply Start Dev: Prev state: %d",
				fctrl->flash_state);
			rc = -EINVAL;
			goto release_mutex;
		}

		fctrl->apply_streamoff = false;
		fctrl->flash_state = CAM_FLASH_STATE_START;
		break;
	}
	case CAM_STOP_DEV: {
		CAM_INFO(CAM_FLASH, "CAM_STOP_DEV ENTER for dev_hdl: 0x%x",
			fctrl->bridge_intf.device_hdl);
		if (fctrl->flash_state != CAM_FLASH_STATE_START) {
			CAM_WARN(CAM_FLASH,
				"Cannot apply Stop dev: Prev state is: %d",
				fctrl->flash_state);
			rc = -EINVAL;
			goto release_mutex;
		}

		fctrl->func_tbl.flush_req(fctrl, FLUSH_ALL, 0);
		fctrl->last_flush_req = 0;
		cam_flash_off(fctrl);
		fctrl->flash_state = CAM_FLASH_STATE_ACQUIRE;
		break;
	}
	case CAM_CONFIG_DEV: {
		CAM_DBG(CAM_FLASH, "CAM_CONFIG_DEV");
		rc = fctrl->func_tbl.parser(fctrl, arg);
		if (rc) {
			CAM_ERR(CAM_FLASH, "Failed Flash Config: rc=%d\n", rc);
			goto release_mutex;
		}
		break;
	}
	default:
		CAM_ERR(CAM_FLASH, "Invalid Opcode: %d", cmd->op_code);
		rc = -EINVAL;
	}

release_mutex:
	mutex_unlock(&(fctrl->flash_mutex));
	return rc;
}

static int32_t cam_flash_init_default_params(struct cam_flash_ctrl *fctrl)
{
	/* Validate input parameters */
	if (!fctrl) {
		CAM_ERR(CAM_FLASH, "failed: invalid params fctrl %pK",
			fctrl);
		return -EINVAL;
	}

	CAM_DBG(CAM_FLASH,
		"master_type: %d", fctrl->io_master_info.master_type);
	/* Initialize cci_client */
	if (fctrl->io_master_info.master_type == CCI_MASTER) {
		fctrl->io_master_info.cci_client = kzalloc(sizeof(
			struct cam_sensor_cci_client), GFP_KERNEL);
		if (!(fctrl->io_master_info.cci_client))
			return -ENOMEM;
	} else if (fctrl->io_master_info.master_type == I2C_MASTER) {
		if (!(fctrl->io_master_info.client))
			return -EINVAL;
	} else {
		CAM_ERR(CAM_FLASH,
			"Invalid master / Master type Not supported");
		return -EINVAL;
	}

	return 0;
}

static const struct of_device_id cam_flash_dt_match[] = {
	{.compatible = "qcom,camera-flash", .data = NULL},
	{}
};

static int cam_flash_subdev_close_internal(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	struct cam_flash_ctrl *fctrl =
		v4l2_get_subdevdata(sd);

	if (!fctrl) {
		CAM_ERR(CAM_FLASH, "Flash ctrl ptr is NULL");
		return -EINVAL;
	}

	mutex_lock(&fctrl->flash_mutex);
	cam_flash_shutdown(fctrl);
	mutex_unlock(&fctrl->flash_mutex);

	return 0;
}

static int cam_flash_subdev_close(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	bool crm_active = cam_req_mgr_is_open();

	if (crm_active) {
		CAM_DBG(CAM_FLASH, "CRM is ACTIVE, close should be from CRM");
		return 0;
	}

	return cam_flash_subdev_close_internal(sd, fh);
}

static long cam_flash_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	int rc = 0;
	struct cam_flash_ctrl *fctrl = NULL;
	struct cam_flash_private_soc *soc_private = NULL;

	CAM_DBG(CAM_FLASH, "Enter");

	fctrl = v4l2_get_subdevdata(sd);
	soc_private = fctrl->soc_info.soc_private;

	switch (cmd) {
	case VIDIOC_CAM_CONTROL: {
		rc = cam_flash_driver_cmd(fctrl, arg,
			soc_private);
		if (rc)
			CAM_ERR(CAM_FLASH,
				"Failed in driver cmd: %d", rc);
		break;
	}
	case CAM_SD_SHUTDOWN:
		if (!cam_req_mgr_is_shutdown()) {
			CAM_ERR(CAM_CORE, "SD shouldn't come from user space");
			return 0;
		}

		rc = cam_flash_subdev_close_internal(sd, NULL);
		break;
	default:
		CAM_ERR(CAM_FLASH, "Invalid ioctl cmd type");
		rc = -ENOIOCTLCMD;
		break;
	}

	CAM_DBG(CAM_FLASH, "Exit");
	return rc;
}

#ifdef CONFIG_COMPAT
static long cam_flash_subdev_do_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, unsigned long arg)
{
	struct cam_control cmd_data;
	int32_t rc = 0;

	if (copy_from_user(&cmd_data, (void __user *)arg,
		sizeof(cmd_data))) {
		CAM_ERR(CAM_FLASH,
			"Failed to copy from user_ptr=%pK size=%zu",
			(void __user *)arg, sizeof(cmd_data));
		return -EFAULT;
	}

	switch (cmd) {
	case VIDIOC_CAM_CONTROL: {
		rc = cam_flash_subdev_ioctl(sd, cmd, &cmd_data);
		if (rc)
			CAM_ERR(CAM_FLASH, "cam_flash_ioctl failed");
		break;
	}
	default:
		CAM_ERR(CAM_FLASH, "Invalid compat ioctl cmd_type:%d",
			cmd);
		rc = -ENOIOCTLCMD;
		break;
	}

	if (!rc) {
		if (copy_to_user((void __user *)arg, &cmd_data,
			sizeof(cmd_data))) {
			CAM_ERR(CAM_FLASH,
				"Failed to copy to user_ptr=%pK size=%zu",
				(void __user *)arg, sizeof(cmd_data));
			rc = -EFAULT;
		}
	}

	return rc;
}
#endif

static struct v4l2_subdev_core_ops cam_flash_subdev_core_ops = {
	.ioctl = cam_flash_subdev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = cam_flash_subdev_do_ioctl
#endif
};

static struct v4l2_subdev_ops cam_flash_subdev_ops = {
	.core = &cam_flash_subdev_core_ops,
};

static const struct v4l2_subdev_internal_ops cam_flash_internal_ops = {
	.close = cam_flash_subdev_close,
};

static int cam_flash_init_subdev(struct cam_flash_ctrl *fctrl)
{
	int rc = 0;

	strlcpy(fctrl->device_name, CAM_FLASH_NAME,
		sizeof(fctrl->device_name));
	fctrl->v4l2_dev_str.internal_ops =
		&cam_flash_internal_ops;
	fctrl->v4l2_dev_str.ops = &cam_flash_subdev_ops;
	fctrl->v4l2_dev_str.name = CAMX_FLASH_DEV_NAME;
	fctrl->v4l2_dev_str.sd_flags =
		V4L2_SUBDEV_FL_HAS_DEVNODE | V4L2_SUBDEV_FL_HAS_EVENTS;
	fctrl->v4l2_dev_str.ent_function = CAM_FLASH_DEVICE_TYPE;
	fctrl->v4l2_dev_str.token = fctrl;
	fctrl->v4l2_dev_str.close_seq_prior = CAM_SD_CLOSE_MEDIUM_PRIORITY;

	rc = cam_register_subdev(&(fctrl->v4l2_dev_str));
	if (rc)
		CAM_ERR(CAM_FLASH, "Fail to create subdev with %d", rc);

	return rc;
}

static int cam_flash_component_bind(struct device *dev,
	struct device *master_dev, void *data)
{
	int32_t rc = 0, i = 0;
	struct cam_flash_ctrl *fctrl = NULL;
	struct device_node *of_parent = NULL;
	struct platform_device *pdev = to_platform_device(dev);
	struct cam_hw_soc_info *soc_info = NULL;

	CAM_DBG(CAM_FLASH, "Binding flash component");
	if (!pdev->dev.of_node) {
		CAM_ERR(CAM_FLASH, "of_node NULL");
		return -EINVAL;
	}

	fctrl = kzalloc(sizeof(struct cam_flash_ctrl), GFP_KERNEL);
	if (!fctrl)
		return -ENOMEM;

	fctrl->pdev = pdev;
	fctrl->of_node = pdev->dev.of_node;
	fctrl->soc_info.pdev = pdev;
	fctrl->soc_info.dev = &pdev->dev;
	fctrl->soc_info.dev_name = pdev->name;

	platform_set_drvdata(pdev, fctrl);

	rc = cam_flash_get_dt_data(fctrl, &fctrl->soc_info);
	if (rc) {
		CAM_ERR(CAM_FLASH, "cam_flash_get_dt_data failed with %d", rc);
		kfree(fctrl);
		return -EINVAL;
	}

	if (of_find_property(pdev->dev.of_node, "cci-master", NULL)) {
		/* Get CCI master */
		rc = of_property_read_u32(pdev->dev.of_node, "cci-master",
			&fctrl->cci_i2c_master);
		CAM_DBG(CAM_FLASH, "cci-master %d, rc %d",
			fctrl->cci_i2c_master, rc);
		if (rc < 0) {
			/* Set default master 0 */
			fctrl->cci_i2c_master = MASTER_0;
			rc = 0;
		}

		fctrl->io_master_info.master_type = CCI_MASTER;
		rc = cam_flash_init_default_params(fctrl);
		if (rc) {
			CAM_ERR(CAM_FLASH,
				"failed: cam_flash_init_default_params rc %d",
				rc);
			return rc;
		}

		of_parent = of_get_parent(pdev->dev.of_node);
		if (of_property_read_u32(of_parent, "cell-index",
				&fctrl->cci_num) < 0)
		/* Set default master 0 */
			fctrl->cci_num = CCI_DEVICE_0;

		fctrl->io_master_info.cci_client->cci_device = fctrl->cci_num;
		CAM_DBG(CAM_FLASH, "cci-index %d", fctrl->cci_num, rc);

		soc_info = &fctrl->soc_info;
		if (!soc_info->gpio_data) {
			CAM_INFO(CAM_FLASH, "No GPIO found");
			rc = 0;
			return rc;
		}

		if (!soc_info->gpio_data->cam_gpio_common_tbl_size) {
			CAM_INFO(CAM_FLASH, "No GPIO found");
			return -EINVAL;
		}

		rc = cam_sensor_util_init_gpio_pin_tbl(soc_info,
				&fctrl->power_info.gpio_num_info);
		if ((rc < 0) || (!fctrl->power_info.gpio_num_info)) {
			CAM_ERR(CAM_FLASH, "No/Error Flash GPIOs");
			return -EINVAL;
		}

		fctrl->i2c_data.per_frame =
			kzalloc(sizeof(struct i2c_settings_array) *
			MAX_PER_FRAME_ARRAY, GFP_KERNEL);
		if (fctrl->i2c_data.per_frame == NULL) {
			CAM_ERR(CAM_FLASH, "No Memory");
			rc = -ENOMEM;
			goto free_cci_resource;
		}

		INIT_LIST_HEAD(&(fctrl->i2c_data.init_settings.list_head));
		INIT_LIST_HEAD(&(fctrl->i2c_data.config_settings.list_head));
		INIT_LIST_HEAD(&(fctrl->i2c_data.streamoff_settings.list_head));
		for (i = 0; i < MAX_PER_FRAME_ARRAY; i++)
			INIT_LIST_HEAD(
				&(fctrl->i2c_data.per_frame[i].list_head));

		fctrl->func_tbl.parser = cam_flash_i2c_pkt_parser;
		fctrl->func_tbl.apply_setting = cam_flash_i2c_apply_setting;
		fctrl->func_tbl.power_ops = cam_flash_i2c_power_ops;
		fctrl->func_tbl.flush_req = cam_flash_i2c_flush_request;

			/* Initialize regulators to default parameters */
		for (i = 0; i < soc_info->num_rgltr; i++) {
			soc_info->rgltr[i] = devm_regulator_get(soc_info->dev,
				soc_info->rgltr_name[i]);
			if (IS_ERR_OR_NULL(soc_info->rgltr[i])) {
				rc = PTR_ERR(soc_info->rgltr[i]);
				rc  = rc ? rc : -EINVAL;
				CAM_ERR(CAM_FLASH, "Get failed for regulator %s %d",
					soc_info->rgltr_name[i], rc);
				goto free_cci_resource;
			}
			CAM_DBG(CAM_FLASH, "Get for regulator %s",
				soc_info->rgltr_name[i]);
		}
	} else {
		/* PMIC Flash */
		fctrl->func_tbl.parser = cam_flash_pmic_pkt_parser;
		fctrl->func_tbl.apply_setting = cam_flash_pmic_apply_setting;
		fctrl->func_tbl.power_ops = NULL;
#if IS_REACHABLE(CONFIG_LEDS_QPNP_FLASH_V2)
		CAM_DBG(CAM_FLASH, "PMIC power_ops");
		fctrl->is_regulator_enabled = false;
		fctrl->func_tbl.power_ops = cam_flash_pmic_power_ops;
#endif
		fctrl->func_tbl.flush_req = cam_flash_pmic_flush_request;
	}

	rc = cam_flash_init_subdev(fctrl);
	if (rc) {
		if (fctrl->io_master_info.cci_client != NULL)
			goto free_cci_resource;
		else
			goto free_resource;
	}

	fctrl->bridge_intf.device_hdl = -1;
	fctrl->bridge_intf.link_hdl = -1;
	fctrl->bridge_intf.ops.get_dev_info = cam_flash_publish_dev_info;
	fctrl->bridge_intf.ops.link_setup = cam_flash_establish_link;
	fctrl->bridge_intf.ops.apply_req = cam_flash_apply_request;
	fctrl->bridge_intf.ops.flush_req = cam_flash_flush_request;
	fctrl->last_flush_req = 0;
	#ifdef CONFIG_FLASHLIGHT_PWM
	cam_flash_factory_test_creat();
	#endif
	pdev_factory_test_flash = pdev;

	mutex_init(&(fctrl->flash_mutex));

	fctrl->flash_state = CAM_FLASH_STATE_INIT;
	CAM_DBG(CAM_FLASH, "Component bound successfully");
	return rc;

free_cci_resource:
	kfree(fctrl->io_master_info.cci_client);
	fctrl->io_master_info.cci_client = NULL;
free_resource:
	kfree(fctrl->i2c_data.per_frame);
	kfree(fctrl->soc_info.soc_private);
	cam_soc_util_release_platform_resource(&fctrl->soc_info);
	fctrl->i2c_data.per_frame = NULL;
	fctrl->soc_info.soc_private = NULL;
	kfree(fctrl);
	fctrl = NULL;
	return rc;
}

static void cam_flash_component_unbind(struct device *dev,
	struct device *master_dev, void *data)
{
	struct cam_flash_ctrl *fctrl;
	struct platform_device *pdev = to_platform_device(dev);

	fctrl = platform_get_drvdata(pdev);
	if (!fctrl) {
		CAM_ERR(CAM_FLASH, "Flash device is NULL");
		return;
	}

	mutex_lock(&fctrl->flash_mutex);
	cam_flash_shutdown(fctrl);
	mutex_unlock(&fctrl->flash_mutex);
	cam_unregister_subdev(&(fctrl->v4l2_dev_str));
	cam_flash_put_source_node_data(fctrl);
	platform_set_drvdata(pdev, NULL);
	v4l2_set_subdevdata(&fctrl->v4l2_dev_str.sd, NULL);
	kfree(fctrl);
	CAM_INFO(CAM_FLASH, "Flash Sensor component unbind");
}

const static struct component_ops cam_flash_component_ops = {
	.bind = cam_flash_component_bind,
	.unbind = cam_flash_component_unbind,
};

static int cam_flash_platform_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &cam_flash_component_ops);
	return 0;
}

static int32_t cam_flash_platform_probe(struct platform_device *pdev)
{
	int rc = 0;

	CAM_DBG(CAM_FLASH, "Adding Flash Sensor component");
	rc = component_add(&pdev->dev, &cam_flash_component_ops);
	if (rc)
		CAM_ERR(CAM_FLASH, "failed to add component rc: %d", rc);

	return rc;
}

static int cam_flash_i2c_component_bind(struct device *dev,
	struct device *master_dev, void *data)
{
	int32_t rc = 0, i = 0;
	struct i2c_client      *client = NULL;
	struct cam_flash_ctrl  *fctrl = NULL;
	struct cam_hw_soc_info *soc_info = NULL;

	client = container_of(dev, struct i2c_client, dev);
	if (client == NULL) {
		CAM_ERR(CAM_FLASH, "Invalid Args client: %pK",
			client);
		return -EINVAL;
	}

	/* Create sensor control structure */
	fctrl = kzalloc(sizeof(*fctrl), GFP_KERNEL);
	if (!fctrl)
		return -ENOMEM;

	client->dev.driver_data = fctrl;
	fctrl->io_master_info.client = client;
	fctrl->of_node = client->dev.of_node;
	fctrl->soc_info.dev = &client->dev;
	fctrl->soc_info.dev_name = client->name;
	fctrl->io_master_info.master_type = I2C_MASTER;

	rc = cam_flash_get_dt_data(fctrl, &fctrl->soc_info);
	if (rc) {
		CAM_ERR(CAM_FLASH, "failed: cam_sensor_parse_dt rc %d", rc);
		goto free_ctrl;
	}

	rc = cam_flash_init_default_params(fctrl);
	if (rc) {
		CAM_ERR(CAM_FLASH,
				"failed: cam_flash_init_default_params rc %d",
				rc);
		goto free_ctrl;
	}

	soc_info = &fctrl->soc_info;

	/* Initalize regulators to default parameters */
	for (i = 0; i < soc_info->num_rgltr; i++) {
		soc_info->rgltr[i] = devm_regulator_get(soc_info->dev,
			soc_info->rgltr_name[i]);
		if (IS_ERR_OR_NULL(soc_info->rgltr[i])) {
			rc = PTR_ERR(soc_info->rgltr[i]);
			rc  = rc ? rc : -EINVAL;
			CAM_ERR(CAM_FLASH, "get failed for regulator %s %d",
				soc_info->rgltr_name[i], rc);
			goto free_ctrl;
		}
		CAM_DBG(CAM_FLASH, "get for regulator %s",
			soc_info->rgltr_name[i]);
	}

	if (!soc_info->gpio_data) {
		CAM_DBG(CAM_FLASH, "No GPIO found");
	} else {
		if (!soc_info->gpio_data->cam_gpio_common_tbl_size) {
			CAM_DBG(CAM_FLASH, "No GPIO found");
			rc = -EINVAL;
			goto free_ctrl;
		}

		rc = cam_sensor_util_init_gpio_pin_tbl(soc_info,
			&fctrl->power_info.gpio_num_info);
		if ((rc < 0) || (!fctrl->power_info.gpio_num_info)) {
			CAM_ERR(CAM_FLASH, "No/Error Flash GPIOs");
			rc = -EINVAL;
			goto free_ctrl;
		}
	}

	rc = cam_flash_init_subdev(fctrl);
	if (rc)
		goto free_ctrl;

	fctrl->i2c_data.per_frame =
		kzalloc(sizeof(struct i2c_settings_array) *
		MAX_PER_FRAME_ARRAY, GFP_KERNEL);
	if (fctrl->i2c_data.per_frame == NULL) {
		rc = -ENOMEM;
		goto unreg_subdev;
	}

	INIT_LIST_HEAD(&(fctrl->i2c_data.init_settings.list_head));
	INIT_LIST_HEAD(&(fctrl->i2c_data.config_settings.list_head));
	INIT_LIST_HEAD(&(fctrl->i2c_data.streamoff_settings.list_head));
	for (i = 0; i < MAX_PER_FRAME_ARRAY; i++)
		INIT_LIST_HEAD(&(fctrl->i2c_data.per_frame[i].list_head));

	fctrl->func_tbl.parser = cam_flash_i2c_pkt_parser;
	fctrl->func_tbl.apply_setting = cam_flash_i2c_apply_setting;
	fctrl->is_regulator_enabled = false;
	fctrl->func_tbl.power_ops = cam_flash_i2c_power_ops;
	fctrl->func_tbl.flush_req = cam_flash_i2c_flush_request;

	fctrl->bridge_intf.device_hdl = -1;
	fctrl->bridge_intf.link_hdl = -1;
	fctrl->bridge_intf.ops.get_dev_info = cam_flash_publish_dev_info;
	fctrl->bridge_intf.ops.link_setup = cam_flash_establish_link;
	fctrl->bridge_intf.ops.apply_req = cam_flash_apply_request;
	fctrl->bridge_intf.ops.flush_req = cam_flash_flush_request;
	fctrl->last_flush_req = 0;

	mutex_init(&(fctrl->flash_mutex));
	fctrl->flash_state = CAM_FLASH_STATE_INIT;

	return rc;

unreg_subdev:
	cam_unregister_subdev(&(fctrl->v4l2_dev_str));
free_ctrl:
	kfree(fctrl);
	fctrl = NULL;
	return rc;
}

static void cam_flash_i2c_component_unbind(struct device *dev,
	struct device *master_dev, void *data)
{
	struct i2c_client     *client = NULL;
	struct cam_flash_ctrl *fctrl = NULL;

	client = container_of(dev, struct i2c_client, dev);
	if (!client) {
		CAM_ERR(CAM_FLASH,
			"Failed to get i2c client");
		return;
	}

	fctrl = i2c_get_clientdata(client);
	/* Handle I2C Devices */
	if (!fctrl) {
		CAM_ERR(CAM_FLASH, "Flash device is NULL");
		return;
	}

	CAM_INFO(CAM_FLASH, "i2c driver remove invoked");
	/*Free Allocated Mem */
	kfree(fctrl->i2c_data.per_frame);
	fctrl->i2c_data.per_frame = NULL;
	kfree(fctrl);
}

const static struct component_ops cam_flash_i2c_component_ops = {
	.bind = cam_flash_i2c_component_bind,
	.unbind = cam_flash_i2c_component_unbind,
};

static int32_t cam_flash_i2c_driver_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;

	if (client == NULL || id == NULL) {
		CAM_ERR(CAM_FLASH, "Invalid Args client: %pK id: %pK",
			client, id);
		return -EINVAL;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		CAM_ERR(CAM_FLASH, "%s :: i2c_check_functionality failed",
			client->name);
		return -EFAULT;
	}

	CAM_DBG(CAM_FLASH, "Adding sensor flash component");
	rc = component_add(&client->dev, &cam_flash_i2c_component_ops);
	if (rc)
		CAM_ERR(CAM_FLASH, "failed to add component rc: %d", rc);

	return rc;
}

static int32_t cam_flash_i2c_driver_remove(struct i2c_client *client)
{
	component_del(&client->dev, &cam_flash_i2c_component_ops);

	return 0;
}

MODULE_DEVICE_TABLE(of, cam_flash_dt_match);

struct platform_driver cam_flash_platform_driver = {
	.probe = cam_flash_platform_probe,
	.remove = cam_flash_platform_remove,
	.driver = {
		.name = "CAM-FLASH-DRIVER",
		.owner = THIS_MODULE,
		.of_match_table = cam_flash_dt_match,
		.suppress_bind_attrs = true,
	},
};

static const struct of_device_id cam_flash_i2c_dt_match[] = {
	{.compatible = "qcom,cam-i2c-flash", .data = NULL},
	{}
};
MODULE_DEVICE_TABLE(of, cam_flash_i2c_dt_match);

static const struct i2c_device_id i2c_id[] = {
	{FLASH_DRIVER_I2C, (kernel_ulong_t)NULL},
	{ }
};

struct i2c_driver cam_flash_i2c_driver = {
	.id_table = i2c_id,
	.probe  = cam_flash_i2c_driver_probe,
	.remove = cam_flash_i2c_driver_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = FLASH_DRIVER_I2C,
		.of_match_table = cam_flash_i2c_dt_match,
		.suppress_bind_attrs = true,
	},
};

int32_t cam_flash_init_module(void)
{
	int32_t rc = 0;

	rc = platform_driver_register(&cam_flash_platform_driver);
	if (rc < 0) {
		CAM_ERR(CAM_FLASH, "platform probe failed rc: %d", rc);
		return rc;
	}

	rc = i2c_add_driver(&cam_flash_i2c_driver);
	if (rc < 0)
		CAM_ERR(CAM_FLASH, "i2c_add_driver failed rc: %d", rc);

	return rc;
}

void cam_flash_exit_module(void)
{
	platform_driver_unregister(&cam_flash_platform_driver);
	i2c_del_driver(&cam_flash_i2c_driver);
}

MODULE_DESCRIPTION("CAM FLASH");
MODULE_LICENSE("GPL v2");
