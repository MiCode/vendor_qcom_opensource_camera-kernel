/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2011-2014, 2017-2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_IO_UTIL_H_
#define _CAM_IO_UTIL_H_

#include <linux/types.h>
/**
 * struct cam_io_print_data - Place holder for printing the registers
 *
 * @soc_info:        SOC Info of the calling hw block
 * @start_offset:    Start offset to print
 * @num_reg:         Number of registers
 * @mod_id:          Module ID of calling module
 */

struct cam_io_print_data {
	struct cam_hw_soc_info *soc_info;
	char                   *token;
	uint32_t                start_offset;
	uint32_t                num_reg;
	uint32_t                mod_id;
	uint32_t                blk_id;
};

#define CAM_INFO_IF(__module, is_error_case, fmt, args...)  \
({ \
	if (is_error_case) \
		CAM_INFO(__module, fmt, ##args); \
	else \
		CAM_DBG(__module, fmt, ##args); \
})

/**
 * cam_io_w()
 *
 * @brief:              Camera IO util for register write
 *
 * @data:               Value to be written
 * @addr:               Address used to write the value
 *
 * @return:             Success or Failure
 */
int cam_io_w(uint32_t data, void __iomem *addr);

/**
 * cam_io_w_mb()
 *
 * @brief:              Camera IO util for register write with memory barrier.
 *                      Memory Barrier is only before the write to ensure the
 *                      order. If need to ensure this write is also flushed
 *                      call wmb() independently in the caller.
 *
 * @data:               Value to be written
 * @addr:               Address used to write the value
 *
 * @return:             Success or Failure
 */
int cam_io_w_mb(uint32_t data, void __iomem *addr);

/**
 * cam_io_r()
 *
 * @brief:              Camera IO util for register read
 *
 * @addr:               Address of register to be read
 *
 * @return:             Value read from the register address
 */
uint32_t cam_io_r(void __iomem *addr);

/**
 * cam_io_r_mb()
 *
 * @brief:              Camera IO util for register read with memory barrier.
 *                      Memory Barrier is only before the write to ensure the
 *                      order. If need to ensure this write is also flushed
 *                      call rmb() independently in the caller.
 *
 * @addr:               Address of register to be read
 *
 * @return:             Value read from the register address
 */
uint32_t cam_io_r_mb(void __iomem *addr);

/**
 * cam_io_memcpy()
 *
 * @brief:              Camera IO util for memory to register copy
 *
 * @dest_addr:          Destination register address
 * @src_addr:           Source regiser address
 * @len:                Range to be copied
 *
 * @return:             Success or Failure
 */
int cam_io_memcpy(void __iomem *dest_addr,
		void __iomem *src_addr, uint32_t len);

/**
 * cam_io_memcpy_mb()
 *
 * @brief:              Camera IO util for memory to register copy
 *                      with barrier.
 *                      Memory Barrier is only before the write to ensure the
 *                      order. If need to ensure this write is also flushed
 *                      call wmb() independently in the caller.
 *
 * @dest_addr:          Destination register address
 * @src_addr:           Source regiser address
 * @len:                Range to be copied
 *
 * @return:             Success or Failure
 */
int cam_io_memcpy_mb(void __iomem *dest_addr,
	void __iomem *src_addr, uint32_t len);

/**
 * cam_io_poll_value_wmask()
 *
 * @brief:              Poll register value with bitmask.
 *
 * @addr:               Register address to be polled
 * @wait_data:          Wait until @bmask read from @addr matches this data
 * @bmask:              Bit mask
 * @retry:              Number of retry
 * @min_usecs:          Minimum time to wait for retry
 * @max_usecs:          Maximum time to wait for retry
 *
 * @return:             Success or Failure
 *
 * This function can sleep so it should not be called from interrupt
 * handler, spin_lock etc.
 */
int cam_io_poll_value_wmask(void __iomem *addr, uint32_t wait_data,
	uint32_t bmask, uint32_t retry, unsigned long min_usecs,
	unsigned long max_usecs);

/**
 * cam_io_poll_value()
 *
 * @brief:              Poll register value
 *
 * @addr:               Register address to be polled
 * @wait_data:          Wait until value read from @addr matches this data
 * @retry:              Number of retry
 * @min_usecs:          Minimum time to wait for retry
 * @max_usecs:          Maximum time to wait for retry
 *
 * @return:             Success or Failure
 *
 * This function can sleep so it should not be called from interrupt
 * handler, spin_lock etc.
 */
int cam_io_poll_value(void __iomem *addr, uint32_t wait_data, uint32_t retry,
	unsigned long min_usecs, unsigned long max_usecs);

/**
 * cam_io_w_same_offset_block()
 *
 * @brief:              Write a block of data to same address
 *
 * @data:               Block data to be written
 * @addr:               Register offset to be written.
 * @len:                Number of the data to be written
 *
 * @return:             Success or Failure
 */
int cam_io_w_same_offset_block(const uint32_t *data, void __iomem *addr,
	uint32_t len);

/**
 * cam_io_w_mb_same_offset_block()
 *
 * @brief:              Write a block of data to same address with barrier.
 *                      Memory Barrier is only before the write to ensure the
 *                      order. If need to ensure this write is also flushed
 *                      call wmb() independently in the caller.
 *
 * @data:               Block data to be written
 * @addr:               Register offset to be written.
 * @len:                Number of the data to be written
 *
 * @return:             Success or Failure
 */
int cam_io_w_mb_same_offset_block(const uint32_t *data, void __iomem *addr,
	uint32_t len);

/**
 * cam_io_w_offset_val_block()
 *
 * @brief:              This API is to write a block of registers
 *                      represented by a 2 dimensional array table with
 *                      register offset and value pair
 *
 *  offset0, value0,
 *  offset1, value1,
 *  offset2, value2,
 *  and so on...
 *
 * @data:               Pointer to 2-dimensional offset-value array
 * @addr_base:          Base address to which offset will be added to
 *                      get the register address
 * @len:                Length of offset-value pair array to be written in
 *                      number of uin32_t
 *
 * @return:             Success or Failure
 *
 */
int32_t cam_io_w_offset_val_block(const uint32_t data[][2],
	void __iomem *addr_base, uint32_t len);

/**
 * cam_io_w_mb_offset_val_block()
 *
 * @brief:              This API is to write a block of registers
 *                      represented by a 2 dimensional array table with
 *                      register offset and value pair with memory barrier.
 *                      Memory Barrier is only before the write to ensure the
 *                      order. If need to ensure this write is also flushed
 *                      call wmb() independently in the caller.
 *                      The OFFSETS NEED to be different because of the way
 *                      barrier is used here.
 *
 *  offset0, value0,
 *  offset1, value1,
 *  offset2, value2,
 *  and so on...
 *
 * @data:               Pointer to 2-dimensional offset-value array
 * @addr_base:          Base address to which offset will be added to
 *                      get the register address
 * @len:                Length of offset-value pair array to be written in
 *                      number of uin32_t
 *
 * @return:             Success or Failure
 *
 */
int32_t cam_io_w_mb_offset_val_block(const uint32_t data[][2],
	void __iomem *addr_base, uint32_t len);

/**
 * cam_io_dump()
 *
 * @brief:              Camera IO util for dumping a range of register
 *
 * @base_addr:          Start register address for the dumping
 * @start_offset:       Start register offset for the dump
 * @size:               Size specifying the range for dumping
 * @client_mask:        Module id for passing respective dump mask from client
 * @is_error_case:      Flag to indicate whether the dump is triggered by an error condition
 *
 * @return:             Success or Failure
 */
int cam_io_dump(void __iomem *base_addr, uint32_t start_offset, int size,
	uint64_t client_mask, bool is_error_case);

/**
 * cam_io_print_info()
 *
 * @brief:              Camera IO util for dumping a range of register
 *
 * @base_addr:          Start register address for the dumping
 * @cam_base:           Cam_base of IP
 * @start_offset:       Start register offset for the dump
 * @size:               Size specifying the range for dumping
 * @mod_id:             Driver module dumping the reg info
 * @token:              Token by Client to filter while debug
 *
 * @return:             Success or Failure
 */
int cam_io_print_info(struct cam_io_print_data *io_data);
#endif /* _CAM_IO_UTIL_H_ */
