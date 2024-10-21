ARCH?=aarch64
ARCH_PROFILE:=armv8-a
ARCH_SUB:=$(ARCH)

include $(bao_demos)/platforms/atf.mk
include $(bao_demos)/platforms/uboot.mk

uboot_defconfig:=vexpress_aemv8a_semi_defconfig
uboot_image:=$(wrkdir_plat_imgs)/u-boot.bin
$(eval $(call build-uboot, $(uboot_image), $(uboot_defconfig)))

atf_bl33:=$(uboot_image)

atf_plat:=fvp
atf_targets:=bl1 fip
atf_flags+=QEMU_USE_GIC_DRIVER=QEMU_GICV3 ARCH=$(ARCH) BL33:=$(atf_bl33)
atf_targets_path:=$(atf_src)/build/$(atf_plat)/release
atf_fip:=$(wrkdir_plat_imgs)/fip.bin
atf_bl1:=$(wrkdir_plat_imgs)/bl1.bin

$(atf_fip) $(atf_bl1): $(atf_bl33) $(atf_src)
	$(MAKE) -C $(atf_src) PLAT=$(atf_plat) $(atf_targets) $(atf_flags)
	$(foreach target, $(atf_targets), \
		cp $(atf_targets_path)/$(target).bin $(wrkdir_plat_imgs)/$(target).bin;)

platform: $(bao_image) $(atf_fip) $(atf_bl1)

instructions:=$(bao_demos)/platforms/$(PLATFORM)/README.md
