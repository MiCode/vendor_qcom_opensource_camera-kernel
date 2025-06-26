// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2014-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/of_address.h>
#include <linux/slab.h>

#include <soc/qcom/socinfo.h>
#include "cam_compat.h"
#include "cam_debug_util.h"
#include "cam_cpas_api.h"
#include "camera_main.h"
#include "cam_eeprom_dev.h"
#include "cam_eeprom_core.h"

#if IS_ENABLED(CONFIG_SPECTRA_USE_RPMH_DRV_API)
#include <soc/qcom/rpmh.h>

#define CAM_RSC_DRV_IDENTIFIER "cam_rsc"

const struct device *cam_cpas_get_rsc_dev_for_drv(uint32_t index)
{
	const struct device *rsc_dev;

	rsc_dev = rpmh_get_device(CAM_RSC_DRV_IDENTIFIER, index);
	if (!rsc_dev) {
		CAM_ERR(CAM_CPAS, "Invalid dev for index: %u", index);
		return NULL;
	}

	return rsc_dev;
}

int cam_cpas_start_drv_for_dev(const struct device *dev)
{
	int rc = 0;

	if (!dev) {
		CAM_ERR(CAM_CPAS, "Invalid dev for DRV enable");
		return -EINVAL;
	}

	rc = rpmh_drv_start(dev);
	if (rc) {
		CAM_ERR(CAM_CPAS, "[%s] Failed in DRV start", dev_name(dev));
		return rc;
	}

	return rc;
}

int cam_cpas_stop_drv_for_dev(const struct device *dev)
{
	int rc = 0;

	if (!dev) {
		CAM_ERR(CAM_CPAS, "Invalid dev for DRV disable");
		return -EINVAL;
	}

	rc = rpmh_drv_stop(dev);
	if (rc) {
		CAM_ERR(CAM_CPAS, "[%s] Failed in DRV stop", dev_name(dev));
		return rc;
	}

	return rc;
}

int cam_cpas_drv_channel_switch_for_dev(const struct device *dev)
{
	int rc = 0;

	if (!dev) {
		CAM_ERR(CAM_CPAS, "Invalid dev for DRV channel switch");
		return -EINVAL;
	}

	rc = rpmh_write_sleep_and_wake_no_child(dev);
	if (rc) {
		CAM_ERR(CAM_CPAS, "[%s] Failed in DRV channel switch", dev_name(dev));
		return rc;
	}

	return rc;
}

#else
const struct device *cam_cpas_get_rsc_dev_for_drv(uint32_t index)
{
	return NULL;
}

int cam_cpas_start_drv_for_dev(const struct device *dev)

{
	return 0;
}

int cam_cpas_stop_drv_for_dev(const struct device *dev)
{
	return 0;
}

int cam_cpas_drv_channel_switch_for_dev(const struct device *dev)
{
	return 0;
}
#endif

int cam_smmu_fetch_csf_version(struct cam_csf_version *csf_version)
{
#ifdef CONFIG_ARCH_QTI_VM
	csf_version->arch_ver = 3;
	csf_version->max_ver = 0;
	csf_version->min_ver = 0;
#elif defined CONFIG_SECURE_CAMERA_25
	struct csf_version csf_ver;
	int rc;

	/* Fetch CSF version from SMMU proxy driver */
	rc = smmu_proxy_get_csf_version(&csf_ver);
	if (rc) {
		CAM_ERR(CAM_SMMU,
			"Failed to get CSF version from SMMU proxy: %d", rc);
		return rc;
	}

	csf_version->arch_ver = csf_ver.arch_ver;
	csf_version->max_ver = csf_ver.max_ver;
	csf_version->min_ver = csf_ver.min_ver;
#else
	/* This defaults to the legacy version */
	csf_version->arch_ver = 2;
	csf_version->max_ver = 0;
	csf_version->min_ver = 0;
#endif
	return 0;
}

unsigned long cam_update_dma_map_attributes(unsigned long attrs)
{
#ifdef CONFIG_SECURE_CAMERA_25
	attrs |= DMA_ATTR_QTI_SMMU_PROXY_MAP;
#endif
	return attrs;
}

size_t cam_align_dma_buf_size(size_t len)
{
#ifdef CONFIG_SECURE_CAMERA_25
	len = ALIGN(len, SMMU_PROXY_MEM_ALIGNMENT);
#endif
	return len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
int cam_reserve_icp_fw(struct cam_fw_alloc_info *icp_fw, size_t fw_length)
{
	int rc = 0;
	struct device_node *of_node;
	struct device_node *mem_node;
	struct resource     res;

	of_node = (icp_fw->fw_dev)->of_node;
	mem_node = of_parse_phandle(of_node, "memory-region", icp_fw->fw_id);
	if (!mem_node) {
		rc = -ENOMEM;
		CAM_ERR(CAM_SMMU, "FW memory carveout of ICP%d not found", icp_fw->fw_id);
		goto end;
	}

	rc = of_address_to_resource(mem_node, 0, &res);
	of_node_put(mem_node);
	if (rc < 0) {
		CAM_ERR(CAM_SMMU, "Unable to get start of FW mem carveout of ICP%u", icp_fw->fw_id);
		goto end;
	}

	icp_fw->fw_hdl = res.start;
	icp_fw->fw_kva = ioremap_wc(icp_fw->fw_hdl, fw_length);
	if (!icp_fw->fw_kva) {
		CAM_ERR(CAM_SMMU, "Failed to map the FW of ICP%d", icp_fw->fw_id);
		rc = -ENOMEM;
		goto end;
	}

	memset_io(icp_fw->fw_kva, 0, fw_length);
end:
	return rc;
}

void cam_unreserve_icp_fw(struct cam_fw_alloc_info *icp_fw, size_t fw_length)
{
	iounmap(icp_fw->fw_kva);
}

int cam_ife_notify_safe_lut_scm(bool safe_trigger)
{
	const uint32_t smmu_se_ife = 0;
	uint32_t camera_hw_version, rc = 0;

	rc = cam_cpas_get_cpas_hw_version(&camera_hw_version);
	if (!rc) {
		switch (camera_hw_version) {
		case CAM_CPAS_TITAN_170_V100:
		case CAM_CPAS_TITAN_170_V110:
		case CAM_CPAS_TITAN_175_V100:
			if (qcom_scm_smmu_notify_secure_lut(smmu_se_ife, safe_trigger)) {
				CAM_ERR(CAM_ISP, "scm call to enable safe failed");
				rc = -EINVAL;
			}
			break;
		default:
			break;
		}
	}

	return rc;
}

void cam_cpastop_scm_write(struct cam_cpas_hw_errata_wa *errata_wa)
{
	int reg_val;

	qcom_scm_io_readl(errata_wa->data.reg_info.offset, &reg_val);
	reg_val |= errata_wa->data.reg_info.value;
	qcom_scm_io_writel(errata_wa->data.reg_info.offset, reg_val);
}

static int camera_platform_compare_dev(struct device *dev, const void *data)
{
	return platform_bus_type.match(dev, (struct device_driver *) data);
}

static int camera_i2c_compare_dev(struct device *dev, const void *data)
{
	return i2c_bus_type.match(dev, (struct device_driver *) data);
}
#else
int cam_reserve_icp_fw(struct cam_fw_alloc_info *icp_fw, size_t fw_length)
{
	int rc = 0;

	icp_fw->fw_kva = dma_alloc_coherent(icp_fw->fw_dev, fw_length,
		&icp_fw->fw_hdl, GFP_KERNEL);

	if (!icp_fw->fw_kva) {
		CAM_ERR(CAM_SMMU, "FW memory of ICP%u alloc failed", icp_fw->fw_id);
		rc = -ENOMEM;
	}

	return rc;
}

void cam_unreserve_icp_fw(struct cam_fw_alloc_info *icp_fw, size_t fw_length)
{
	dma_free_coherent(icp_fw->fw_dev, fw_length, icp_fw->fw_kva,
		icp_fw->fw_hdl);
}

int cam_ife_notify_safe_lut_scm(bool safe_trigger)
{
	const uint32_t smmu_se_ife = 0;
	uint32_t camera_hw_version, rc = 0;
	struct scm_desc description = {
		.arginfo = SCM_ARGS(2, SCM_VAL, SCM_VAL),
		.args[0] = smmu_se_ife,
		.args[1] = safe_trigger,
	};

	rc = cam_cpas_get_cpas_hw_version(&camera_hw_version);
	if (!rc) {
		switch (camera_hw_version) {
		case CAM_CPAS_TITAN_170_V100:
		case CAM_CPAS_TITAN_170_V110:
		case CAM_CPAS_TITAN_175_V100:
			if (scm_call2(SCM_SIP_FNID(0x15, 0x3), &description)) {
				CAM_ERR(CAM_ISP, "scm call to enable safe failed");
				rc = -EINVAL;
			}
			break;
		default:
			break;
		}
	}

	return rc;
}

void cam_cpastop_scm_write(struct cam_cpas_hw_errata_wa *errata_wa)
{
	int reg_val;

	reg_val = scm_io_read(errata_wa->data.reg_info.offset);
	reg_val |= errata_wa->data.reg_info.value;
	scm_io_write(errata_wa->data.reg_info.offset, reg_val);
}

static int camera_platform_compare_dev(struct device *dev, void *data)
{
	return platform_bus_type.match(dev, (struct device_driver *) data);
}

static int camera_i2c_compare_dev(struct device *dev, void *data)
{
	return i2c_bus_type.match(dev, (struct device_driver *) data);
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
void cam_free_clear(const void * ptr)
{
	kfree_sensitive(ptr);
}
#else
void cam_free_clear(const void * ptr)
{
	kzfree(ptr);
}
#endif

#ifdef CONFIG_DOMAIN_ID_SECURE_CAMERA
int cam_csiphy_notify_secure_mode(struct csiphy_device *csiphy_dev,
	bool protect, int32_t offset, bool __maybe_unused is_shutdown)
{
	int rc = 0;
	struct Object client_env, sc_object;
	ITCDriverSensorInfo params = {0};
	struct cam_csiphy_secure_info *secure_info;

	if (offset >= CSIPHY_MAX_INSTANCES_PER_PHY) {
		CAM_ERR(CAM_CSIPHY, "Invalid CSIPHY offset");
		return -EINVAL;
	}

#if !IS_ENABLED(CONFIG_QCOM_SI_CORE)
	if (!is_shutdown) {
#endif
		rc = get_client_env_object(&client_env);
		if (rc) {
			CAM_ERR(CAM_CSIPHY, "Failed getting mink env object, rc: %d", rc);
			return rc;
		}

		rc = IClientEnv_open(client_env, CTrustedCameraDriver_UID, &sc_object);
		if (rc) {
			CAM_ERR(CAM_CSIPHY, "Failed getting mink sc_object, rc: %d", rc);
			return rc;
		}

		secure_info = &csiphy_dev->csiphy_info[offset].secure_info;
		params.csid_hw_idx_mask = secure_info->csid_hw_idx_mask;
		params.cdm_hw_idx_mask = secure_info->cdm_hw_idx_mask;
		params.vc_mask = secure_info->vc_mask;
		params.phy_lane_sel_mask =
			csiphy_dev->csiphy_info[offset].csiphy_phy_lane_sel_mask;
		params.protect = protect ? 1 : 0;

		rc = ITrustedCameraDriver_dynamicProtectSensor(sc_object, &params);
		if (rc) {
			CAM_ERR(CAM_CSIPHY, "Mink secure call failed, rc: %d", rc);
			return rc;
		}

		rc = Object_release(sc_object);
		if (rc) {
			CAM_ERR(CAM_CSIPHY, "Failed releasing secure camera object, rc: %d", rc);
			return rc;
		}

		rc = Object_release(client_env);
		if (rc) {
			CAM_ERR(CAM_CSIPHY, "Failed releasing mink env object, rc: %d", rc);
			return rc;
		}
#if !IS_ENABLED(CONFIG_QCOM_SI_CORE)
	} else {
		/* This is a temporary work around until the SMC Invoke driver is
		 * refactored to avoid the dependency on FDs, which was causing issues
		 * during process shutdown.
		 */
		rc = qcom_scm_camera_protect_phy_lanes(protect, 0);
		if (rc) {
			CAM_ERR(CAM_CSIPHY, "SCM call to hypervisor failed");
			return rc;
		}
	}
#endif

	return 0;
}
#elif KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
int cam_csiphy_notify_secure_mode(struct csiphy_device *csiphy_dev,
	bool protect, int32_t offset, bool __always_unused is_shutdown)
{
	int rc = 0;

	/**
	 * A check here is made if the target is using
	 * an older version of the kernel driver (< 6.0)
	 * with domain id feature present. In this case,
	 * we are to fail this call, as the new mink call
	 * is only supported on kernel driver versions 6.0
	 * and above, and the new domain id scheme is not
	 * backwards compatible with the older scheme.
	 */
	if (csiphy_dev->domain_id_security) {
		CAM_ERR(CAM_CSIPHY,
			"Domain id support not present on current kernel driver: %d",
			LINUX_VERSION_CODE);
		return -EINVAL;
	}

	if (offset >= CSIPHY_MAX_INSTANCES_PER_PHY) {
		CAM_ERR(CAM_CSIPHY, "Invalid CSIPHY offset");
		rc = -EINVAL;
	} else if (qcom_scm_camera_protect_phy_lanes(protect,
			csiphy_dev->csiphy_info[offset].csiphy_cpas_cp_reg_mask)) {
		CAM_ERR(CAM_CSIPHY, "SCM call to hypervisor failed");
		rc = -EINVAL;
	}

	return rc;
}
#else
int cam_csiphy_notify_secure_mode(struct csiphy_device *csiphy_dev,
	bool protect, int32_t offset, bool __always_unused is_shutdown)
{
	int rc = 0;
	struct scm_desc description = {
		.arginfo = SCM_ARGS(2, SCM_VAL, SCM_VAL),
		.args[0] = protect,
		.args[1] = csiphy_dev->csiphy_info[offset]
			.csiphy_cpas_cp_reg_mask,
	};

	if (offset >= CSIPHY_MAX_INSTANCES_PER_PHY) {
		CAM_ERR(CAM_CSIPHY, "Invalid CSIPHY offset");
		rc = -EINVAL;
	} else if (scm_call2(SCM_SIP_FNID(0x18, 0x7), &description)) {
		CAM_ERR(CAM_CSIPHY, "SCM call to hypervisor failed");
		rc = -EINVAL;
	}

	return rc;
}
#endif

#ifdef CONFIG_SPECTRA_SECURE_CAMNOC_REG_UPDATE
int cam_update_camnoc_qos_settings(uint32_t use_case_id,
	uint32_t qos_cnt, struct qcom_scm_camera_qos *scm_buf)
{
	int rc = 0;

	rc = qcom_scm_camera_update_camnoc_qos(use_case_id, qos_cnt, scm_buf);
	if (rc)
		CAM_ERR(CAM_CPAS, "scm call to update QoS failed: %d, use_case_id: %d",
			rc, use_case_id);

	return rc;
}
#else
int cam_update_camnoc_qos_settings(uint32_t use_case_id,
	uint32_t qos_cnt, struct qcom_scm_camera_qos *scm_buf)
{
	CAM_ERR(CAM_CPAS, "scm call to update QoS is not supported under this kernel");
	return -EOPNOTSUPP;
}
#endif

/* Callback to compare device from match list before adding as component */
static inline int camera_component_compare_dev(struct device *dev, void *data)
{
	return dev == data;
}

/* Add component matches to list for master of aggregate driver */
int camera_component_match_add_drivers(struct device *master_dev,
	struct component_match **match_list)
{
	int i, rc = 0;
	struct platform_device *pdev = NULL;
	struct i2c_client *client = NULL;
	struct device *start_dev = NULL, *match_dev = NULL;

	if (!master_dev || !match_list) {
		CAM_ERR(CAM_UTIL, "Invalid parameters for component match add");
		rc = -EINVAL;
		goto end;
	}

	for (i = 0; i < ARRAY_SIZE(cam_component_platform_drivers); i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
		struct device_driver const *drv =
			&cam_component_platform_drivers[i]->driver;
		const void *drv_ptr = (const void *)drv;
#else
		struct device_driver *drv = &cam_component_platform_drivers[i]->driver;
		void *drv_ptr = (void *)drv;
#endif
		start_dev = NULL;
		while ((match_dev = bus_find_device(&platform_bus_type,
			start_dev, drv_ptr, &camera_platform_compare_dev))) {
			put_device(start_dev);
			pdev = to_platform_device(match_dev);
			CAM_DBG(CAM_UTIL, "Adding matched component:%s", pdev->name);
			component_match_add(master_dev, match_list,
				camera_component_compare_dev, match_dev);
			start_dev = match_dev;
		}
		put_device(start_dev);
	}

	for (i = 0; i < ARRAY_SIZE(cam_component_i2c_drivers); i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
		struct device_driver const *drv =
			&cam_component_i2c_drivers[i]->driver;
		const void *drv_ptr = (const void *)drv;
#else
		struct device_driver *drv = &cam_component_i2c_drivers[i]->driver;
		void *drv_ptr = (void *)drv;
#endif
		start_dev = NULL;
		while ((match_dev = bus_find_device(&i2c_bus_type,
			start_dev, drv_ptr, &camera_i2c_compare_dev))) {
			put_device(start_dev);
			client = to_i2c_client(match_dev);
			CAM_DBG(CAM_UTIL, "Adding matched component:%s", client->name);
			component_match_add(master_dev, match_list,
				camera_component_compare_dev, match_dev);
			start_dev = match_dev;
		}
		put_device(start_dev);
	}

end:
	return rc;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include <linux/qcom-iommu-util.h>
void cam_check_iommu_faults(struct iommu_domain *domain,
	struct cam_smmu_pf_info *pf_info)
{
	struct qcom_iommu_fault_ids fault_ids = {0, 0, 0};

	if (qcom_iommu_get_fault_ids(domain, &fault_ids))
		CAM_ERR(CAM_SMMU, "Cannot get smmu fault ids");
	else
		CAM_ERR(CAM_SMMU, "smmu fault ids bid:%d pid:%d mid:%d",
			fault_ids.bid, fault_ids.pid, fault_ids.mid);

	pf_info->bid = fault_ids.bid;
	pf_info->pid = fault_ids.pid;
	pf_info->mid = fault_ids.mid;
}
#else
void cam_check_iommu_faults(struct iommu_domain *domain,
	struct cam_smmu_pf_info *pf_info)
{
	struct iommu_fault_ids fault_ids = {0, 0, 0};

	if (iommu_get_fault_ids(domain, &fault_ids))
		CAM_ERR(CAM_SMMU, "Error: Can not get smmu fault ids");

	CAM_ERR(CAM_SMMU, "smmu fault ids bid:%d pid:%d mid:%d",
		fault_ids.bid, fault_ids.pid, fault_ids.mid);

	pf_info->bid = fault_ids.bid;
	pf_info->pid = fault_ids.pid;
	pf_info->mid = fault_ids.mid;
}
#endif

static int inline cam_subdev_list_cmp(struct cam_subdev *entry_1, struct cam_subdev *entry_2)
{
	if (entry_1->close_seq_prior > entry_2->close_seq_prior)
		return 1;
	else if (entry_1->close_seq_prior < entry_2->close_seq_prior)
		return -1;
	else
		return 0;
}

#if (KERNEL_VERSION(5, 18, 0) <= LINUX_VERSION_CODE)
int cam_compat_util_get_dmabuf_va(struct dma_buf *dmabuf, uintptr_t *vaddr)
{
	struct iosys_map mapping = {0};
#if (KERNEL_VERSION(6, 2, 0) <= LINUX_VERSION_CODE)
	int error_code = dma_buf_vmap_unlocked(dmabuf, &mapping);
#else
	int error_code = dma_buf_vmap(dmabuf, &mapping);
#endif

	if (error_code) {
		*vaddr = 0;
	} else {
		*vaddr = (mapping.is_iomem) ?
			(uintptr_t)mapping.vaddr_iomem : (uintptr_t)mapping.vaddr;
		CAM_DBG(CAM_MEM,
			"dmabuf=%p, *vaddr=%p, is_iomem=%d, vaddr_iomem=%p, vaddr=%p",
			dmabuf, *vaddr, mapping.is_iomem, mapping.vaddr_iomem, mapping.vaddr);
	}

	return error_code;
}

void cam_compat_util_put_dmabuf_va(struct dma_buf *dmabuf, void *vaddr)
{
	struct iosys_map mapping = IOSYS_MAP_INIT_VADDR(vaddr);

#if (KERNEL_VERSION(6, 2, 0) <= LINUX_VERSION_CODE)
	dma_buf_vunmap_unlocked(dmabuf, &mapping);
#else
	dma_buf_vunmap(dmabuf, &mapping);
#endif
}

#elif (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
int cam_compat_util_get_dmabuf_va(struct dma_buf *dmabuf, uintptr_t *vaddr)
{
	struct dma_buf_map mapping;
	int error_code = dma_buf_vmap(dmabuf, &mapping);

	if (error_code) {
		*vaddr = 0;
	} else {
		*vaddr = (mapping.is_iomem) ?
			(uintptr_t)mapping.vaddr_iomem : (uintptr_t)mapping.vaddr;
		CAM_DBG(CAM_MEM,
			"dmabuf=%p, *vaddr=%p, is_iomem=%d, vaddr_iomem=%p, vaddr=%p",
			dmabuf, *vaddr, mapping.is_iomem, mapping.vaddr_iomem, mapping.vaddr);
	}

	return error_code;
}

void cam_compat_util_put_dmabuf_va(struct dma_buf *dmabuf, void *vaddr)
{
	struct dma_buf_map mapping = DMA_BUF_MAP_INIT_VADDR(vaddr);

	dma_buf_vunmap(dmabuf, &mapping);
}

#else
int cam_compat_util_get_dmabuf_va(struct dma_buf *dmabuf, uintptr_t *vaddr)
{
	int error_code = 0;
	void *addr = dma_buf_vmap(dmabuf);

	if (!addr) {
		*vaddr = 0;
		error_code = -ENOSPC;
	} else {
		*vaddr = (uintptr_t)addr;
	}

	return error_code;
}

void cam_compat_util_put_dmabuf_va(struct dma_buf *dmabuf, void *vaddr)
{
	dma_buf_vunmap(dmabuf, vaddr);
}
#endif

struct sg_table *cam_compat_dmabuf_map_attach(struct dma_buf_attachment *attach,
	enum dma_data_direction dma_dir)
{
#if (KERNEL_VERSION(6, 2, 0) <= LINUX_VERSION_CODE)
	return dma_buf_map_attachment_unlocked(attach, dma_dir);
#else
	return dma_buf_map_attachment(attach, dma_dir);
#endif
}

void cam_compat_dmabuf_unmap_attach(struct dma_buf_attachment *attach,
	struct sg_table *table, enum dma_data_direction dma_dir)
{
#if (KERNEL_VERSION(6, 2, 0) <= LINUX_VERSION_CODE)
	dma_buf_unmap_attachment_unlocked(attach, table, dma_dir);
#else
	dma_buf_unmap_attachment(attach, table, dma_dir);
#endif
}

#if (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
void cam_smmu_util_iommu_custom(struct device *dev,
	dma_addr_t discard_start, size_t discard_length)
{

}

int cam_req_mgr_ordered_list_cmp(void *priv,
	const struct list_head *head_1, const struct list_head *head_2)
{
	return cam_subdev_list_cmp(list_entry(head_1, struct cam_subdev, list),
		list_entry(head_2, struct cam_subdev, list));
}
#else
void cam_smmu_util_iommu_custom(struct device *dev,
	dma_addr_t discard_start, size_t discard_length)
{
	iommu_dma_enable_best_fit_algo(dev);

	if (discard_start)
		iommu_dma_reserve_iova(dev, discard_start, discard_length);

	return;
}

int cam_req_mgr_ordered_list_cmp(void *priv,
	struct list_head *head_1, struct list_head *head_2)
{
	return cam_subdev_list_cmp(list_entry(head_1, struct cam_subdev, list),
		list_entry(head_2, struct cam_subdev, list));
}
#endif

#if (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
long cam_dma_buf_set_name(struct dma_buf *dmabuf, const char *name)
{
	long ret = 0;

	ret = dma_buf_set_name(dmabuf, name);

	return ret;
}
#else
long cam_dma_buf_set_name(struct dma_buf *dmabuf, const char *name)
{
	return 0;
}
#endif

#if KERNEL_VERSION(5, 18, 0) <= LINUX_VERSION_CODE
void cam_eeprom_spi_driver_remove(struct spi_device *sdev)
{
	struct v4l2_subdev             *sd = spi_get_drvdata(sdev);
	struct cam_eeprom_ctrl_t       *e_ctrl;
	struct cam_eeprom_soc_private  *soc_private;

	if (!sd) {
		CAM_ERR(CAM_EEPROM, "Subdevice is NULL");
		return;
	}

	e_ctrl = (struct cam_eeprom_ctrl_t *)v4l2_get_subdevdata(sd);
	if (!e_ctrl) {
		CAM_ERR(CAM_EEPROM, "eeprom device is NULL");
		return;
	}

	mutex_lock(&(e_ctrl->eeprom_mutex));
	cam_eeprom_shutdown(e_ctrl);
	mutex_unlock(&(e_ctrl->eeprom_mutex));
	mutex_destroy(&(e_ctrl->eeprom_mutex));
	cam_unregister_subdev(&(e_ctrl->v4l2_dev_str));
	CAM_MEM_FREE(e_ctrl->io_master_info.spi_client);
	e_ctrl->io_master_info.spi_client = NULL;
	soc_private =
		(struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
	if (soc_private) {
		CAM_MEM_FREE(soc_private->power_info.gpio_num_info);
		soc_private->power_info.gpio_num_info = NULL;
		CAM_MEM_FREE(soc_private);
		soc_private = NULL;
	}
	v4l2_set_subdevdata(&e_ctrl->v4l2_dev_str.sd, NULL);
	CAM_MEM_FREE(e_ctrl);
}

int cam_compat_util_get_irq(struct cam_hw_soc_info *soc_info)
{
	int rc = 0, i;
	struct device_node *of_node = NULL;

	of_node = soc_info->dev->of_node;

	for (i = 0; i < soc_info->irq_count; i++) {
		rc = of_property_read_string_index(of_node, "interrupt-names",
			i, &soc_info->irq_name[i]);
		if (rc) {
			CAM_ERR(CAM_UTIL, "Failed to get irq names at i = %d rc = %d",
				i, rc);
			return -EINVAL;
		}

		soc_info->irq_num[i] = platform_get_irq(soc_info->pdev, i);
		if (soc_info->irq_num[i] < 0) {
			rc = soc_info->irq_num[i];
			CAM_ERR(CAM_UTIL, "Failed to get irq resource at i = %d rc = %d",
				i, rc);
			return rc;
		}
	}

	return rc;
}
#else
int cam_eeprom_spi_driver_remove(struct spi_device *sdev)
{
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
	mutex_lock(&(e_ctrl->eeprom_mutex));
	cam_eeprom_shutdown(e_ctrl);
	mutex_unlock(&(e_ctrl->eeprom_mutex));
	mutex_destroy(&(e_ctrl->eeprom_mutex));
	cam_unregister_subdev(&(e_ctrl->v4l2_dev_str));
	CAM_MEM_FREE(e_ctrl->io_master_info.spi_client);
	e_ctrl->io_master_info.spi_client = NULL;
	soc_private =
		(struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
	if (soc_private) {
		CAM_MEM_FREE(soc_private->power_info.gpio_num_info);
		soc_private->power_info.gpio_num_info = NULL;
		CAM_MEM_FREE(soc_private);
		soc_private = NULL;
	}
	v4l2_set_subdevdata(&e_ctrl->v4l2_dev_str.sd, NULL);
	CAM_MEM_FREE(e_ctrl);

	return 0;
}

int cam_compat_util_get_irq(struct cam_hw_soc_info *soc_info)
{
	int rc = 0, i;
	struct device_node *of_node = NULL;

	of_node = soc_info->dev->of_node;

	for (i = 0; i < soc_info->irq_count; i++) {
		rc = of_property_read_string_index(of_node, "interrupt-names",
			i, &soc_info->irq_name[i]);
		if (rc) {
			CAM_ERR(CAM_UTIL, "Failed to get irq names at i = %d rc = %d",
				i, rc);
			return -EINVAL;
		}

		soc_info->irq_line[i] = platform_get_resource_byname(soc_info->pdev,
			IORESOURCE_IRQ, soc_info->irq_name[i]);
		if (!soc_info->irq_line[i]) {
			CAM_ERR(CAM_UTIL, "Failed to get IRQ line for irq: %s of %s",
				soc_info->irq_name[i], soc_info->dev_name);
			rc = -ENODEV;
			return rc;
		}
		soc_info->irq_num[i] = soc_info->irq_line[i]->start;
	}

	return rc;
}
#endif

bool cam_secure_get_vfe_fd_port_config(void)
{
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
	return false;
#else
	return true;
#endif
}

#if KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE
int cam_get_subpart_info(uint32_t *part_info, uint32_t max_num_cam)
{
	int rc = 0;
	int num_cam;

	num_cam = socinfo_get_part_count(PART_CAMERA);
	if (num_cam != max_num_cam) {
		CAM_ERR(CAM_CPAS, "Unsupported number of parts: %d", num_cam);
		return -EINVAL;
	}

	/*
	 * If bit value in part_info is "0" then HW is available.
	 * If bit value in part_info is "1" then HW is unavailable.
	 */
	rc = socinfo_get_subpart_info(PART_CAMERA, part_info, num_cam);
	if (rc) {
		CAM_ERR(CAM_CPAS, "Failed while getting subpart_info, rc = %d.", rc);
		return rc;
	}

	return 0;
}
#else
int cam_get_subpart_info(uint32_t *part_info, uint32_t max_num_cam)
{
	return 0;
}
#endif

#if KERNEL_VERSION(6, 2, 0) <= LINUX_VERSION_CODE
int cam_iommu_map(struct iommu_domain *domain,
	size_t firmware_start, phys_addr_t fw_hdl,
	size_t firmware_len, int prot)
{
	int rc = 0;

	rc = iommu_map(domain, firmware_start,
			fw_hdl,	firmware_len,
			prot, GFP_ATOMIC);
	return rc;
}
#else
int cam_iommu_map(struct iommu_domain *domain,
	size_t firmware_start, phys_addr_t fw_hdl,
	size_t firmware_len, int prot)
{
	int rc = 0;

	rc = iommu_map(domain, firmware_start,
			fw_hdl,	firmware_len,
			prot);
	return rc;
}
#endif

#if KERNEL_VERSION(6, 2, 0) <= LINUX_VERSION_CODE
ssize_t cam_iommu_map_sg(struct iommu_domain *domain,
	dma_addr_t iova_start, struct scatterlist *sgl,
	uint64_t orig_nents, int prot)
{
	ssize_t size = 0;

	size = iommu_map_sg(domain,
			iova_start,
			sgl, orig_nents,
			prot, GFP_ATOMIC);
	return size;
}
#else
ssize_t cam_iommu_map_sg(struct iommu_domain *domain,
	dma_addr_t iova_start, struct scatterlist *sgl,
	uint64_t orig_nents, int prot)
{
	ssize_t size = 0;

	size = iommu_map_sg(domain, iova_start,
			sgl, orig_nents,
			prot);
	return size;
}
#endif

int16_t cam_get_gpio_counts(struct cam_hw_soc_info *soc_info)
{
	struct device_node *of_node = NULL;
	int16_t gpio_array_size = 0;

	of_node = soc_info->dev->of_node;
#if KERNEL_VERSION(6, 2, 0) <= LINUX_VERSION_CODE
	gpio_array_size = of_count_phandle_with_args(
		of_node, "gpios", "#gpio-cells");
#else
	gpio_array_size = of_gpio_count(of_node);
#endif

	return gpio_array_size;
}

uint16_t cam_get_named_gpio(struct cam_hw_soc_info *soc_info,
	int index)
{
	struct device_node *of_node = NULL;
	uint16_t gpio_pin = 0;

	of_node = soc_info->dev->of_node;
#if KERNEL_VERSION(6, 2, 0) <= LINUX_VERSION_CODE
	gpio_pin = of_get_named_gpio(of_node, "gpios", index);
#else
	gpio_pin = of_get_gpio(of_node, index);
#endif

	return gpio_pin;
}

inline struct icc_path *cam_icc_get_path(struct device *dev,
		const int src_id, const int dst_id, const char *path_name, bool use_path_name)
{
	CAM_DBG(CAM_UTIL, "Get icc path name: %s src_id:%d dst_id:%d use_path_name:%s", path_name,
		src_id, dst_id, CAM_BOOL_TO_YESNO(use_path_name));

#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
	if (!use_path_name) {
		CAM_ERR(CAM_UTIL, "Must use path names to get icc path handle");
		return NULL;
	}

	return of_icc_get(dev, path_name);
#else
	if (use_path_name)
		return of_icc_get(dev, path_name);
	else
		return icc_get(dev, src_id, dst_id);
#endif
}

#if IS_REACHABLE(CONFIG_DMABUF_HEAPS)
#ifdef CONFIG_ARCH_QTI_VM
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
void *cam_mem_heap_add_kernel_pool(struct dma_heap *heap, size_t size)
{
	/* Comment out logic that depends on memory team's change temporary */
	// return qcom_tvm_heap_add_kernel_pool(heap, size);
	return NULL;
}

void cam_mem_heap_remove_kernel_pool(void *handle)
{
	/* Comment out logic that depends on memory team's change temporary */
	// qcom_tvm_heap_remove_kernel_pool(handle);
}
#else
void *cam_mem_heap_add_kernel_pool(struct dma_heap *heap, size_t size)
{
	/* Comment out logic that depends on memory team's change temporary */
	// return qcom_tui_heap_add_kernel_pool(heap, size);
	return NULL;
}

void cam_mem_heap_remove_kernel_pool(void *handle)
{
	/* Comment out logic that depends on memory team's change temporary */
	// qcom_tui_heap_remove_kernel_pool(handle);
}
#endif
#endif
#endif
