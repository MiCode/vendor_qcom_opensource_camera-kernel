/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
 */
// MIUI ADD: Camera_CameraSkyNet
#pragma once
//#ifndef _CAM_LOG_H_
//#define _CAM_LOG_H_
#include <linux/types.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>

//cci_tools cci 0 master write/read slaveaddr regaddr regdata addrtype datatype
//cci_tools qup 8        write/read slaveaddr regaddr regdata  addrtype datatype

/* 定义用于I2C/寄存器操作的结构体 */
typedef struct {
    uint8_t  i2c_type; //0 is cci, 1 is qup
    uint8_t  device_id;
    uint8_t  cci_master;
    uint8_t  opcode;
    uint8_t  slave_addr; // slave address
    uint32_t reg_addr;   /**< Address of the register. */
    uint32_t reg_data;   /**< Data to be written to the register (for write operations). */
    uint8_t  addr_type;
    uint8_t  data_type;
} cam_reg_info;


/* 定义用于I2C/寄存器操作的结构体 */
typedef struct {
    uint32_t slot_id;
    uint8_t  slave_addr;
    uint32_t reg_addr;
    uint32_t reg_data;
    uint8_t  addr_type;
    uint8_t  data_type;
} cam_slot_reg_info;

/**
 * @brief : API to register SENSOR hw to platform framework.
 * @return struct platform_device pointer on on success, or ERR_PTR() on error.
 */
int cam_io_dev_init(void);

/**
 * @brief : API to remove SENSOR Hw from platform framework.
 */
void cam_io_dev_exit(void);

//#endif /* _CAM_LOG_H_ */
// END Camera_CameraSkyNet