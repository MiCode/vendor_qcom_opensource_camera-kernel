load("//build/kernel/kleaf:kernel.bzl", "ddk_module")
load("//build/bazel_common_rules/dist:dist.bzl", "copy_to_dist_dir")
load(":target_variants.bzl", "get_all_variants")
load(":project_defconfig.bzl", "get_project_defconfig")
load("//build/kernel/kleaf:kernel.bzl", "ddk_headers")

def _define_mi_cam_module(target, variant):
    tv = "{}_{}".format(target, variant)
    base_deps = []
    deps = []
    base_deps = select({
        "//build/kernel/kleaf:socrepo_true": [
            "//soc-repo:all_headers",
            ":mi_cam_inc_headers",
            ":camera_headers",
            "//soc-repo:{}/kernel/sched/walt/sched-walt".format(tv),
        ],
        "//build/kernel/kleaf:socrepo_false": [
            "//msm-kernel:all_headers",
        ],
    })

    kernel_build = select({
        "//build/kernel/kleaf:socrepo_true": "//soc-repo:{}_base_kernel".format(tv),
        "//build/kernel/kleaf:socrepo_false": "//msm-kernel:{}".format(tv),
    })

    ddk_module(
        name = "{}_mi_cam".format(tv),
        out = "mi_cam.ko",
        srcs = [
            "drivers/mi_cam/mi_cam.c",
        ],
        deps = base_deps + deps,
        kernel_build = kernel_build,
    )
    copy_to_dist_dir(
        name = "{}_mi_cam_dist".format(tv),
        data = [
            ":{}_mi_cam".format(tv),
        ],
        dist_dir = "out/target/product/{}/dlkm/lib/modules/".format(target),
        flat = True,
        wipe_dist_dir = False,
        allow_duplicate_filenames = False,
        mode_overrides = {"**/*": "644"},
        log = "info",
    )

def define_mi_cam_module():
    print("******************************mi_cam******************************")
    for (t, v) in get_all_variants():
        _define_mi_cam_module(t, v)

