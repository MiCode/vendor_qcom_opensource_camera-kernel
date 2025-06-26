CAMERA_DLKM_ENABLED := true
ifeq ($(TARGET_KERNEL_DLKM_DISABLE), true)
	ifeq ($(TARGET_KERNEL_DLKM_CAMERA_OVERRIDE), false)
		CAMERA_DLKM_ENABLED := false;
	endif
endif

ifeq ($(CAMERA_DLKM_ENABLED),true)
PRODUCT_PACKAGES += camera.ko
PRODUCT_PACKAGES += cameralog.ko
# MIUI ADD: Camera_CameraOpt
PRODUCT_PACKAGES += cameramsger.ko
# END Camera_CameraOpt
# MIUI ADD: Camera_CamSched
PRODUCT_PACKAGES += mi_cam.ko
# END Camera_CamSched
endif
