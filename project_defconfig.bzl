load("@bazel_skylib//rules:write_file.bzl", "write_file")

common_configs = [
	"CONFIG_SPECTRA_ISP=y",
	"CONFIG_SPECTRA_ICP=y",
	"CONFIG_SPECTRA_JPEG=y",
	"CONFIG_SPECTRA_SENSOR=y",
	"CONFIG_SPECTRA_USE_CLK_CRM_API=y",
	"CONFIG_SPECTRA_USE_RPMH_DRV_API=y",
	"CONFIG_SPECTRA_LLCC_STALING=y",
]

dependency_config = [
	"CONFIG_TARGET_SYNX_ENABLE=y",
	"CONFIG_INTERCONNECT_QCOM=y",
	"CONFIG_DOMAIN_ID_SECURE_CAMERA=y",
	"CONFIG_DYNAMIC_FD_PORT_CONFIG=y",
	"CONFIG_SECURE_CAMERA_25=y",
	"CONFIG_MSM_MMRM=y",
]

project_configs = select({
    # Project-specific configs
    ":no_project": [],
    ":pineapple": dependency_config + [
        "CONFIG_SPECTRA_SECURE_CAMNOC_REG_UPDATE=y",
    ],
    ":sun": dependency_config + [
        "CONFIG_SPECTRA_SECURE_DYN_PORT_CFG=y",
        "CONFIG_SPECTRA_SECURE_CAMNOC_REG_UPDATE=y",
    ],
    ":canoe": [],
})

"""
Return a label which defines a project-specific defconfig snippet to be
applied on top of the platform defconfig.
"""

def get_project_defconfig(target, variant):
    rule_name = "{}_{}_project_defconfig".format(target, variant)

    write_file(
        name = rule_name,
        out = "{}.generated".format(rule_name),
        content = common_configs + project_configs + [""],
    )

    return rule_name
