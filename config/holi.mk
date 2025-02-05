# SPDX-License-Identifier: GPL-2.0-only

# Localized KCONFIG settings
CONFIG_SPECTRA_ISP := y
CONFIG_SPECTRA_OPE := y
CONFIG_SPECTRA_TFE := y
CONFIG_SPECTRA_SENSOR := y

# Flags to pass into C preprocessor
ccflags-y += -DCONFIG_SPECTRA_ISP=1
ccflags-y += -DCONFIG_SPECTRA_TFE=1
ccflags-y += -DCONFIG_SPECTRA_OPE=1
ccflags-y += -DCONFIG_SPECTRA_SENSOR=1

# External Dependencies
ifeq ($(CONFIG_QCOM_VA_MINIDUMP), y)
KBUILD_CPPFLAGS += -DCONFIG_QCOM_VA_MINIDUMP=1
endif
