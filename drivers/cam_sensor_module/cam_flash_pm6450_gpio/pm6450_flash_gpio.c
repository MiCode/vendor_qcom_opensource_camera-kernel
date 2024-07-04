/*
 * Copyright (C) 2021 HUAQIN Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/pwm.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <media/rc-core.h>
#include <linux/platform_device.h>
#include "pm6450_flash_gpio.h"



/*****************************************************************************
 * Data Structure
 *****************************************************************************/

struct pwm_device *pwm = NULL;
struct pinctrl  *pinctrl_info = NULL;
struct pinctrl_state *flash_pwm_pinctrl = NULL;
/*****************************************************************************
 * Function
 *****************************************************************************/

void pm6450_flash_gpio_select_state(PM6450_FLASH_GPIO_STATE s,  struct pwm_setting *flash_pwm){
    struct pwm_state pstate;
    int rc = 0;
    pwm_get_state(pwm, &pstate);
    pstate.period = flash_pwm->period_ns;//50000;
    pstate.duty_cycle = flash_pwm->duty_ns;//50000;
    PM6450_FLASH_PRINT("period:%d,duty:%d",
            pstate.period, pstate.duty_cycle);
    if(flash_pwm_pinctrl) {
        rc = pinctrl_select_state(pinctrl_info,flash_pwm_pinctrl);
        if(rc) {
            PM6450_FLASH_PRINT("failed to select state flash_pwm_pinctrl");
            return;
        }
    }
    switch(s){
    case PM6450_FLASH_GPIO_STATE_ACTIVE:
        pstate.enabled = true;
        rc = pwm_apply_state(pwm, &pstate);
        break;
    case PM6450_FLASH_GPIO_STATE_SUSPEND:
        pstate.enabled = false;
        rc = pwm_apply_state(pwm, &pstate);
        break;
    default:
        PM6450_FLASH_PRINT("Failed to control PWM use a err state!\n");
    }

    if(rc < 0) {
        PM6450_FLASH_PRINT("Apply PWM state fail, rc = %d", rc);
    }
}

static int pm6450_flash_gpio_component_bind(struct device *dev,
	struct device *master_dev, void *data)
{
	int32_t rc = 0;

    PM6450_FLASH_PRINT("+");
    pinctrl_info = devm_pinctrl_get(dev);
    if (IS_ERR_OR_NULL(pinctrl_info)) {
		PM6450_FLASH_PRINT("Getting pinctrl handle failed");
		return -EINVAL;
	}
    flash_pwm_pinctrl = pinctrl_lookup_state(pinctrl_info,
        "camera_flash_pwm_default");
    if (IS_ERR_OR_NULL(flash_pwm_pinctrl)) {
        flash_pwm_pinctrl = NULL;
        devm_pinctrl_put(pinctrl_info);
        PM6450_FLASH_PRINT("failed to get pinctrl state camera_flash_pwm_default");
        return -EINVAL;
    }
    PM6450_FLASH_PRINT("-");
	return rc;
}

static void pm6450_flash_gpio_component_unbind(struct device *dev,
	struct device *master_dev, void *data)
{
    if(pinctrl_info) {
        PM6450_FLASH_PRINT("release pinctrl info");
        devm_pinctrl_put(pinctrl_info);
        pinctrl_info = NULL;
    }
	PM6450_FLASH_PRINT("Flash Sensor component unbind");
}

const static struct component_ops cam_flash_component_ops = {
	.bind = pm6450_flash_gpio_component_bind,
	.unbind = pm6450_flash_gpio_component_unbind,
};

static int pm6450_flash_pwm_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &cam_flash_component_ops);
	return 0;
}

static int pm6450_flash_pwm_probe(struct platform_device *pdev)
{
	int rc = 0;
    PM6450_FLASH_PRINT("+");
    pwm = devm_pwm_get(&pdev->dev, NULL);
	if (IS_ERR(pwm)) {
        PM6450_FLASH_PRINT("failed to get pwm devm");
		return PTR_ERR(pwm);
    }
    rc = component_add(&pdev->dev, &cam_flash_component_ops);
	if (rc)
		PM6450_FLASH_PRINT("failed to add component rc: %d", rc);
    PM6450_FLASH_PRINT("-");
	return rc;
}

static const struct of_device_id gpio_of_match[] = {
    { .compatible = "qualcomm,pm6450_flash_gpio", },
    {},
};

struct platform_driver pm6450_flash_gpio_platform_driver = {
    .probe = pm6450_flash_pwm_probe,
    .remove = pm6450_flash_pwm_remove,
    .driver = {
        .name = "PM6450_FLASH_GPIO_DTS",
        .of_match_table = gpio_of_match,
    },
};

int pm6450_flash_gpio_init_module(void)
{
    PM6450_FLASH_PRINT("+");
    if (platform_driver_register(&pm6450_flash_gpio_platform_driver)) {
	    PM6450_FLASH_PRINT("Failed to register pm6450_flash_gpio_platform_driver!\n");
	    return -1;
    }
    PM6450_FLASH_PRINT("-");
    return 0;
}

void pm6450_flash_gpio_exit_module(void)
{
    platform_driver_unregister(&pm6450_flash_gpio_platform_driver);
}

MODULE_DESCRIPTION("CONTROL PM6450 FLASH GPIO Driver");
MODULE_LICENSE("GPL");