// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include "cam_csiphy_soc.h"
#include "cam_csiphy_core.h"
#include "include/cam_csiphy_2_1_0_hwreg.h"
#include "include/cam_csiphy_2_1_1_hwreg.h"
#include "include/cam_csiphy_2_1_2_hwreg.h"
#include "include/cam_csiphy_2_1_3_hwreg.h"
#include "include/cam_csiphy_2_2_0_hwreg.h"

/* Clock divide factor for CPHY spec v1.0 */
#define CSIPHY_DIVISOR_16                    16
/* Clock divide factor for CPHY spec v1.2 and up */
#define CSIPHY_DIVISOR_32                    32
/* Clock divide factor for DPHY */
#define CSIPHY_DIVISOR_8                     8
#define CSIPHY_LOG_BUFFER_SIZE_IN_BYTES      250
#define ONE_LOG_LINE_MAX_SIZE                20

/* xiaomi add for mipi phy backup setting begin*/
extern uint64_t xm_mipi_kmd_setting;
static struct kobject csiphy_umd_paras_kobject;
static void csiphy_umd_paras_kobj_init(void);
/* xiaomi add for mipi phy backup setting end*/

static int cam_csiphy_io_dump(void __iomem *base_addr, uint16_t num_regs, int csiphy_idx)
{
	char                                    *buffer;
	uint8_t                                  buffer_offset = 0;
	uint8_t                                  rem_buffer_size = CSIPHY_LOG_BUFFER_SIZE_IN_BYTES;
	uint16_t                                 i;
	uint32_t                                 reg_offset;

	if (!base_addr || !num_regs) {
		CAM_ERR(CAM_CSIPHY, "Invalid params. base_addr: 0x%p num_regs: %u",
			base_addr, num_regs);
		return -EINVAL;
	}

	buffer = kzalloc(CSIPHY_LOG_BUFFER_SIZE_IN_BYTES, GFP_KERNEL);
	if (!buffer) {
		CAM_ERR(CAM_CSIPHY, "Could not allocate the memory for buffer");
		return -ENOMEM;
	}

	CAM_INFO(CAM_CSIPHY, "Base: 0x%pK num_regs: %u", base_addr, num_regs);
	CAM_INFO(CAM_CSIPHY, "CSIPHY:%d Dump", csiphy_idx);
	for (i = 0; i < num_regs; i++) {
		reg_offset = i << 2;
		buffer_offset += scnprintf(buffer + buffer_offset, rem_buffer_size, "0x%x=0x%x\n",
			reg_offset, cam_io_r_mb(base_addr + reg_offset));

		rem_buffer_size = CSIPHY_LOG_BUFFER_SIZE_IN_BYTES - buffer_offset;

		if (rem_buffer_size <= ONE_LOG_LINE_MAX_SIZE) {
			buffer[buffer_offset - 1] = '\0';
			pr_info("%s\n", buffer);
			buffer_offset = 0;
			rem_buffer_size = CSIPHY_LOG_BUFFER_SIZE_IN_BYTES;
		}
	}

	if (buffer_offset) {
		buffer[buffer_offset - 1] = '\0';
		pr_info("%s\n", buffer);
	}

	kfree(buffer);

	return 0;
}

int32_t cam_csiphy_reg_dump(struct cam_hw_soc_info *soc_info)
{
	int32_t rc = 0;
	resource_size_t size = 0;
	void __iomem *addr = NULL;

	if (!soc_info) {
		rc = -EINVAL;
		CAM_ERR(CAM_CSIPHY, "invalid input %d", rc);
		return rc;
	}
	addr = soc_info->reg_map[0].mem_base;
	size = resource_size(soc_info->mem_block[0]);
	rc = cam_csiphy_io_dump(addr, (size >> 2), soc_info->index);
	if (rc < 0) {
		CAM_ERR(CAM_CSIPHY, "generating dump failed %d", rc);
		return rc;
	}
	return rc;
}

int32_t cam_csiphy_common_status_reg_dump(struct csiphy_device *csiphy_dev)
{
	struct csiphy_reg_parms_t *csiphy_reg = NULL;
	int32_t                    rc = 0;
	resource_size_t            size = 0;
	void __iomem              *phy_base = NULL;
	int                        reg_id = 0;
	uint32_t                   val, status_reg, clear_reg;

	if (!csiphy_dev) {
		rc = -EINVAL;
		CAM_ERR(CAM_CSIPHY, "invalid input %d", rc);
		return rc;
	}

	csiphy_reg = csiphy_dev->ctrl_reg->csiphy_reg;
	phy_base = csiphy_dev->soc_info.reg_map[0].mem_base;
	status_reg = csiphy_reg->mipi_csiphy_interrupt_status0_addr;
	clear_reg = csiphy_reg->mipi_csiphy_interrupt_clear0_addr;
	size = csiphy_reg->csiphy_num_common_status_regs;

	CAM_INFO(CAM_CSIPHY, "PHY base addr=%pK offset=0x%x size=%d",
		phy_base, status_reg, size);

	if (phy_base != NULL) {
		for (reg_id = 0; reg_id < size; reg_id++) {
			val = cam_io_r(phy_base + status_reg + (0x4 * reg_id));

			if (reg_id < csiphy_reg->csiphy_interrupt_status_size)
				cam_io_w_mb(val, phy_base + clear_reg + (0x4 * reg_id));

			CAM_INFO(CAM_CSIPHY, "CSIPHY%d_COMMON_STATUS%u = 0x%x",
				csiphy_dev->soc_info.index, reg_id, val);
		}
	} else {
		rc = -EINVAL;
		CAM_ERR(CAM_CSIPHY, "phy base is NULL  %d", rc);
		return rc;
	}
	return rc;
}

enum cam_vote_level get_clk_voting_dynamic(
	struct csiphy_device *csiphy_dev, int32_t index)
{
	uint32_t cam_vote_level = 0;
	uint32_t last_valid_vote = 0;
	struct cam_hw_soc_info *soc_info;
	uint64_t phy_data_rate = csiphy_dev->csiphy_info[index].data_rate;

	soc_info = &csiphy_dev->soc_info;
	phy_data_rate = max(phy_data_rate, csiphy_dev->current_data_rate);

	if (csiphy_dev->csiphy_info[index].csiphy_3phase) {
		if (csiphy_dev->is_divisor_32_comp)
			do_div(phy_data_rate, CSIPHY_DIVISOR_32);
		else
			do_div(phy_data_rate, CSIPHY_DIVISOR_16);
	} else {
		do_div(phy_data_rate, CSIPHY_DIVISOR_8);
	}

	 /* round off to next integer */
	phy_data_rate += 1;
	csiphy_dev->current_data_rate = phy_data_rate;

	for (cam_vote_level = 0; cam_vote_level < CAM_MAX_VOTE; cam_vote_level++) {
		if (soc_info->clk_level_valid[cam_vote_level] != true)
			continue;

		if (soc_info->clk_rate[cam_vote_level]
			[csiphy_dev->rx_clk_src_idx] > phy_data_rate) {
			CAM_DBG(CAM_CSIPHY,
				"match detected %s : %llu:%d level : %d",
				soc_info->clk_name[csiphy_dev->rx_clk_src_idx],
				phy_data_rate,
				soc_info->clk_rate[cam_vote_level]
				[csiphy_dev->rx_clk_src_idx],
				cam_vote_level);
			return cam_vote_level;
		}
		last_valid_vote = cam_vote_level;
	}
	return last_valid_vote;
}

int32_t cam_csiphy_enable_hw(struct csiphy_device *csiphy_dev, int32_t index)
{
	int32_t rc = 0;
	struct cam_hw_soc_info   *soc_info;
	enum cam_vote_level vote_level = CAM_SVS_VOTE;

	soc_info = &csiphy_dev->soc_info;

	if (csiphy_dev->ref_count++) {
		CAM_ERR(CAM_CSIPHY, "csiphy refcount = %d",
			csiphy_dev->ref_count);
		return rc;
	}

	vote_level = csiphy_dev->ctrl_reg->getclockvoting(csiphy_dev, index);
	rc = cam_soc_util_enable_platform_resource(soc_info, true,
		vote_level, true);
	if (rc < 0) {
		CAM_ERR(CAM_CSIPHY, "failed to enable platform resources %d",
			rc);
		return rc;
	}

	rc = cam_soc_util_set_src_clk_rate(soc_info,
		soc_info->clk_rate[0][soc_info->src_clk_idx]);

	if (rc < 0) {
		CAM_ERR(CAM_CSIPHY, "csiphy_clk_set_rate failed rc: %d", rc);
		goto csiphy_disable_platform_resource;
	}

	cam_csiphy_reset(csiphy_dev);

	return rc;


csiphy_disable_platform_resource:
	cam_soc_util_disable_platform_resource(soc_info, true, true);

	return rc;
}

int32_t cam_csiphy_disable_hw(struct csiphy_device *csiphy_dev)
{
	struct cam_hw_soc_info   *soc_info;

	if (!csiphy_dev || !csiphy_dev->ref_count) {
		CAM_ERR(CAM_CSIPHY, "csiphy dev NULL / ref_count ZERO");
		return 0;
	}
	soc_info = &csiphy_dev->soc_info;

	if (--csiphy_dev->ref_count) {
		CAM_ERR(CAM_CSIPHY, "csiphy refcount = %d",
			csiphy_dev->ref_count);
		return 0;
	}

	cam_csiphy_reset(csiphy_dev);

	cam_soc_util_disable_platform_resource(soc_info, true, true);

	return 0;
}

int32_t cam_csiphy_parse_dt_info(struct platform_device *pdev,
	struct csiphy_device *csiphy_dev)
{
	int32_t   rc = 0, i = 0;
	uint32_t  clk_cnt = 0;
	uint32_t   is_regulator_enable_sync;
	char      *csi_3p_clk_name = "csi_phy_3p_clk";
	char      *csi_3p_clk_src_name = "csiphy_3p_clk_src";
	struct cam_hw_soc_info   *soc_info;
	/* xiaomi add for mipi phy backup setting begin*/
	static uint8_t init_once;
	/* xiaomi add for mipi phy backup setting end*/

	soc_info = &csiphy_dev->soc_info;

	rc = cam_soc_util_get_dt_properties(soc_info);
	if (rc < 0) {
		CAM_ERR(CAM_CSIPHY, "parsing common soc dt(rc %d)", rc);
		return  rc;
	}

	rc = of_property_read_u32(soc_info->dev->of_node, "rgltr-enable-sync",
		&is_regulator_enable_sync);
	if (rc) {
		rc = 0;
		is_regulator_enable_sync = 0;
	}

	csiphy_dev->prgm_cmn_reg_across_csiphy = (bool) is_regulator_enable_sync;

	if (of_device_is_compatible(soc_info->dev->of_node, "qcom,csiphy-v2.1.0")) {
		csiphy_dev->ctrl_reg = &ctrl_reg_2_1_0;
		csiphy_dev->hw_version = CSIPHY_VERSION_V210;
		csiphy_dev->is_divisor_32_comp = true;
		csiphy_dev->clk_lane = 0;
	} else if (of_device_is_compatible(soc_info->dev->of_node, "qcom,csiphy-v2.1.1")) {
		csiphy_dev->ctrl_reg = &ctrl_reg_2_1_1;
		csiphy_dev->hw_version = CSIPHY_VERSION_V211;
		csiphy_dev->is_divisor_32_comp = true;
		csiphy_dev->clk_lane = 0;
	} else if (of_device_is_compatible(soc_info->dev->of_node, "qcom,csiphy-v2.1.2")) {
		csiphy_dev->ctrl_reg = &ctrl_reg_2_1_2;
		csiphy_dev->hw_version = CSIPHY_VERSION_V212;
		csiphy_dev->is_divisor_32_comp = true;
		csiphy_dev->clk_lane = 0;
	} else if (of_device_is_compatible(soc_info->dev->of_node, "qcom,csiphy-v2.1.3")) {
		csiphy_dev->ctrl_reg = &ctrl_reg_2_1_3;
		csiphy_dev->hw_version = CSIPHY_VERSION_V213;
		csiphy_dev->is_divisor_32_comp = true;
		csiphy_dev->clk_lane = 0;
	} else if (of_device_is_compatible(soc_info->dev->of_node, "qcom,csiphy-v2.2.0")) {
		csiphy_dev->ctrl_reg = &ctrl_reg_2_2_0;
		csiphy_dev->hw_version = CSIPHY_VERSION_V220;
		csiphy_dev->is_divisor_32_comp = true;
		csiphy_dev->clk_lane = 0;
		/* xiaomi add cphy reg - begin */
	} else if (of_device_is_compatible(soc_info->dev->of_node, "qcom,csiphy-v2.1.2-m11")) {
		csiphy_dev->ctrl_reg = &ctrl_reg_2_1_2_m11;
		csiphy_dev->hw_version = CSIPHY_VERSION_V212;
		csiphy_dev->is_divisor_32_comp = true;
		csiphy_dev->clk_lane = 0;
	} else if (of_device_is_compatible(soc_info->dev->of_node, "qcom,csiphy-v2.1.2-m1")) {
		csiphy_dev->ctrl_reg = &ctrl_reg_2_1_2_m1;
		csiphy_dev->hw_version = CSIPHY_VERSION_V212;
		csiphy_dev->is_divisor_32_comp = true;
		csiphy_dev->clk_lane = 0;
	} else if (of_device_is_compatible(soc_info->dev->of_node, "qcom,csiphy-v2.1.2-n11")) {
		csiphy_dev->ctrl_reg = &ctrl_reg_2_1_2_n11;
		csiphy_dev->hw_version = CSIPHY_VERSION_V212;
		csiphy_dev->is_divisor_32_comp = true;
		csiphy_dev->clk_lane = 0;
	} else if (of_device_is_compatible(soc_info->dev->of_node, "qcom,csiphy-v2.1.2-m18")) {
		if (init_once == 0) {
			csiphy_umd_paras_kobj_init();
			init_once = 1;
		}
		csiphy_dev->device_has_customized = 1;
		strncpy(csiphy_dev->phy_dts_name, "qcom,csiphy-v2.1.2-m18", strlen("qcom,csiphy-v2.1.2-m18"));
		csiphy_dev->ctrl_reg = &ctrl_reg_2_1_2_m18;
		csiphy_dev->hw_version = CSIPHY_VERSION_V212;
		csiphy_dev->is_divisor_32_comp = true;
		csiphy_dev->clk_lane = 0;
		/* xiaomi add hw trigger - end   */
	}else {
		CAM_ERR(CAM_CSIPHY, "invalid hw version : 0x%x",
			csiphy_dev->hw_version);
		rc =  -EINVAL;
		return rc;
	}

	if (soc_info->num_clk > CSIPHY_NUM_CLK_MAX) {
		CAM_ERR(CAM_CSIPHY, "invalid clk count=%d, max is %d",
			soc_info->num_clk, CSIPHY_NUM_CLK_MAX);
		return -EINVAL;
	}

	for (i = 0; i < soc_info->num_clk; i++) {
		if (!strcmp(soc_info->clk_name[i],
			csi_3p_clk_src_name)) {
			csiphy_dev->csiphy_3p_clk_info[0].clk_name =
				soc_info->clk_name[i];
			csiphy_dev->csiphy_3p_clk_info[0].clk_rate =
				soc_info->clk_rate[0][i];
			csiphy_dev->csiphy_3p_clk[0] =
				soc_info->clk[i];
			continue;
		} else if (!strcmp(soc_info->clk_name[i],
				csi_3p_clk_name)) {
			csiphy_dev->csiphy_3p_clk_info[1].clk_name =
				soc_info->clk_name[i];
			csiphy_dev->csiphy_3p_clk_info[1].clk_rate =
				soc_info->clk_rate[0][i];
			csiphy_dev->csiphy_3p_clk[1] =
				soc_info->clk[i];
			continue;
		} else if (!strcmp(soc_info->clk_name[i],
				CAM_CSIPHY_RX_CLK_SRC)) {
			csiphy_dev->rx_clk_src_idx = i;
			continue;
		}

		CAM_DBG(CAM_CSIPHY, "clk_rate[%d] = %d", clk_cnt,
			soc_info->clk_rate[0][clk_cnt]);
		clk_cnt++;
	}

	csiphy_dev->csiphy_max_clk =
		soc_info->clk_rate[0][soc_info->src_clk_idx];

	rc = cam_soc_util_request_platform_resource(&csiphy_dev->soc_info,
		cam_csiphy_irq, csiphy_dev);

	return rc;
}

int32_t cam_csiphy_soc_release(struct csiphy_device *csiphy_dev)
{
	if (!csiphy_dev) {
		CAM_ERR(CAM_CSIPHY, "csiphy dev NULL");
		return 0;
	}

	cam_soc_util_release_platform_resource(&csiphy_dev->soc_info);

	return 0;
}


/* xiaomi add for mipi phy backup setting begin*/
static ssize_t csiphy_umd_paras_show(struct kobject *kobj, struct kobj_attribute *attr,
	char *buf)
{
	CAM_INFO(CAM_CSIPHY, "xm_mipi_kmd_setting[0x%lx]", xm_mipi_kmd_setting);

	return snprintf(buf, 64, "%lx", xm_mipi_kmd_setting);
}

static ssize_t csiphy_umd_paras_store(struct kobject *kobj, struct kobj_attribute *attr,
	const char *buf, size_t count)
{
	int rc;
	const char *p = NULL;

	p = buf;
	rc = kstrtou64(p, 16, &xm_mipi_kmd_setting);
	if (rc < 0) {
		CAM_ERR(CAM_CSIPHY, "error converting buf:[%s]", buf);
	} else {
		CAM_INFO(CAM_CSIPHY, "xm_mipi_kmd_setting:[0x%lx]", xm_mipi_kmd_setting);
	}

	return count;
}


static ssize_t csiphy_umd_paras_object_show(struct kobject *k, struct attribute *attr, char *buf)
{
	struct kobj_attribute *kobj_attr;
	int ret = -EIO;

	kobj_attr = container_of(attr, struct kobj_attribute, attr);

	if (kobj_attr->show)
		ret = kobj_attr->show(k, kobj_attr, buf);

	return ret;
}

static ssize_t csiphy_umd_paras_object_store(struct kobject *k, struct attribute *attr, const char *buf, size_t size)
{
	struct kobj_attribute *kobj_attr;
	int ret = -EIO;

	kobj_attr = container_of(attr, struct kobj_attribute, attr);

	if (kobj_attr->store)
		ret = kobj_attr->store(k, kobj_attr, buf, sizeof(buf));

	return ret;
}

static struct kobj_attribute csiphy_umd_paras_attribute =
__ATTR(csiphy_umd_paras, 0644, csiphy_umd_paras_show, csiphy_umd_paras_store);

static const struct sysfs_ops csiphy_umd_paras_sysfs_ops = {
	.show = csiphy_umd_paras_object_show,
	.store = csiphy_umd_paras_object_store,
};

static struct kobj_type csiphy_umd_paras_object_type = {
	.sysfs_ops = &csiphy_umd_paras_sysfs_ops,
	.release	= NULL,
};

static void csiphy_umd_paras_kobj_init(void)
{
	int rc = 0;
	memset(&csiphy_umd_paras_kobject, 0x00, sizeof(csiphy_umd_paras_kobject));

	if (kobject_init_and_add(&csiphy_umd_paras_kobject, &csiphy_umd_paras_object_type, NULL, "csiphy_umd_paras")) {
		kobject_put(&csiphy_umd_paras_kobject);
		return;
	}

	kobject_uevent(&csiphy_umd_paras_kobject, KOBJ_ADD);
	rc = sysfs_create_file(&csiphy_umd_paras_kobject, &csiphy_umd_paras_attribute.attr);
	if (rc < 0) {
		CAM_ERR(CAM_CPAS,
			"Failed to create debug attribute, rc=%d\n", rc);
		sysfs_remove_file(&csiphy_umd_paras_kobject, &csiphy_umd_paras_attribute.attr);
	}

	return ;
}
/* xiaomi add for mipi phy backup setting end*/


