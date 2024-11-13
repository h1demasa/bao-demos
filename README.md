# Bao Hypervisor Demo Guide on gem5

This tutorial provides a step-by-step guide on how to run different demo 
configurations of the Bao hypervisor featuring multiple guest operating 
systems on gem5.

You can see the automation build guide on [gem5 blog](http://www.gem5.org/2024/11/12/bao-on-gem5.html).

---

**NOTE**

This tutorial assumes you are running a standard Linux distro (e.g. 
Ubuntu) and using bash.

## -1. Install dependencies

```
sudo apt install build-essential bison flex git libssl-dev ninja-build \
    u-boot-tools pandoc libslirp-dev pkg-config libglib2.0-dev libpixman-1-dev \
    gettext-base curl xterm cmake python3-pip xilinx-bootgen device-tree-compiler

pip3 install pykwalify packaging pyelftools
```

## 0. Download and setup the toolchain

Download the latest bare-metal cross-compile toolchain for your target 
architecture:

For Armv8 Aarch64, use the **aarch64-none-elf-** toolchain.

Download it from the [Arm Developer's][arm-toolchains]  website.

Install the toolchain. Then, set the **CROSS_COMPILE** environment variable 
with the reference toolchain prefix path:

```
export CROSS_COMPILE=/path/to/toolchain/install/dir/bin/your-toolchain-prefix-
```

<!-- Links -->

[arm-toolchains]: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

---

# Step-by-step Build Guide

This guide explains how to build the environment from scratch using the official repositories. It provides detailed information about the required modifications and build steps.

First, create directories to store build outputs:

```bash
export WORKDIR=$(pwd)
mkdir -p outputs/config # For build outputs
mkdir -p resources/binaries resources/semihosting # For using from gem5
```

## Clone bao-demos

```bash
git clone https://github.com/bao-project/bao-demos.git
```

The Bao Hypervisor demo repository contains the configuration files for both the operating system and firmware. By default, it does not include settings for running two Linux instances, so we'll need to create these configurations.

The default configuration in "guests/linux/configs/base.config" has shared memory enabled. While this feature requires a Linux kernel patch to work, we won't be using shared memory in this setup, so we'll disable it.

Additionally, X509 certificate processing causes significant delays when running on gem5 in our environment, consistently failing to complete regardless of the allowed time. Therefore, we'll disable these features by adding the following to the base.config file:

```
CONFIG_X509_CERTIFICATE_PARSER=n
CONFIG_SYSTEM_TRUSTED_KEYRING=n
CONFIG_SYSTEM_TRUSTED_KEYS=""
CONFIG_SYSTEM_EXTRA_CERTIFICATE=n
CONFIG_PKCS7_MESSAGE_PARSER=n
CONFIG_ASYMMETRIC_KEY_TYPE=n
CONFIG_ASYMMETRIC_PUBLIC_KEY_SUBTYPE=n
CONFIG_X509_CERTIFICATE_PARSER=n
CONFIG_PKCS7_TEST_KEY=n
CONFIG_SIGNED_PE_FILE_VERIFICATION=n
```

The default Linux configuration for Bao Hypervisor includes Ethernet support. However, since the VExpress_GEM5_Foundation platform we're using doesn't support Ethernet, we'll remove it from the Linux Device Tree (DTB).

We also need to modify the memory range to match the allocation defined by the Bao Hypervisor. Save the following Device Tree configurations as "$WORKDIR/outputs/linux1.dts" and "$WORKDIR/outputs/linux2.dts":

```
/dts-v1/;

/ {
	#size-cells = <0x2>;
	#address-cells = <0x2>;
	interrupt-parent = <&gic>;

	cpus {
		#size-cells = <0x0>;
		#address-cells = <0x1>;

		cpu@0 {
			compatible = "arm,cortex-a53", "arm,armv8";
			device_type = "cpu";
			enable-method = "psci";
			reg = <0x0>;
		};

		cpu@1 {
			compatible = "arm,cortex-a53", "arm,armv8";
			device_type = "cpu";
			enable-method = "psci";
			reg = <0x1>;
		};
	};

	psci {
		compatible = "arm,psci-0.2";
		method = "smc";
	};

	memory@a0000000 {
		reg = <0x0 0xa0000000 0x0 0x40000000>;
		device_type = "memory";
	};

	gic: intc@2F000000 {
		interrupts = <0x01 0x09 0x04>;
		reg = <0x00 0x2F000000 0x00 0x10000 0x00 0x2F100000 0x00 0xf60000>;
		#redistributor-regions = <0x01>;
		compatible = "arm,gic-v3";
		ranges;
		#size-cells = <0x02>;
		#address-cells = <0x02>;
		interrupt-controller;
		#interrupt-cells = <0x03>;
	};

	timer {
		interrupts = <0x1 0xd 0xf04 0x1 0xe 0xf04 0x1 0xb 0xf04 0x1 0xa 0xf04>;
		always-on;
		compatible = "arm,armv8-timer", "arm,armv7-timer";
	};

	pl011@1c0a0000 {
		clock-names = "uartclk", "apb_pclk";
		clocks = <0x8000 0x8000>;
		interrupts = <0x0 0x6 0x4>;
		reg = <0x0 0x1c0a0000 0x0 0x1000>;
		compatible = "arm,pl011", "arm,primecell";
	};

	apb-pclk {
		phandle = <0x8000>;
		clock-output-names = "clk24mhz";
		clock-frequency = <0x16e3600>;
		#clock-cells = <0x0>;
		compatible = "fixed-clock";
	};

	chosen {
        bootargs = "earlycon console=ttyAMA0,115200n8 carrier_timeout=0";
	    stdout-path = "/pl011@1c090000";
	};
};
```

```
/dts-v1/;

/ {
	#size-cells = <0x2>;
	#address-cells = <0x2>;
	interrupt-parent = <&gic>;

	cpus {
		#size-cells = <0x0>;
		#address-cells = <0x1>;

		cpu@0 {
			compatible = "arm,cortex-a53", "arm,armv8";
			device_type = "cpu";
			enable-method = "psci";
			reg = <0x0>;
		};

		cpu@1 {
			compatible = "arm,cortex-a53", "arm,armv8";
			device_type = "cpu";
			enable-method = "psci";
			reg = <0x1>;
		};
	};

	psci {
		compatible = "arm,psci-0.2";
		method = "smc";
	};

	memory@e0000000 {
		reg = <0x0 0xe0000000 0x0 0x40000000>;
		device_type = "memory";
	};

	gic: intc@2F000000 {
		interrupts = <0x01 0x09 0x04>;
		reg = <0x00 0x2F000000 0x00 0x10000 0x00 0x2F100000 0x00 0xf60000>;
		#redistributor-regions = <0x01>;
		compatible = "arm,gic-v3";
		ranges;
		#size-cells = <0x02>;
		#address-cells = <0x02>;
		interrupt-controller;
		#interrupt-cells = <0x03>;
	};

	timer {
		interrupts = <0x1 0xd 0xf04 0x1 0xe 0xf04 0x1 0xb 0xf04 0x1 0xa 0xf04>;
		always-on;
		compatible = "arm,armv8-timer", "arm,armv7-timer";
	};

	pl011@1c0b0000 {
		clock-names = "uartclk", "apb_pclk";
		clocks = <0x8000 0x8000>;
		interrupts = <0x0 0x7 0x4>;
		reg = <0x0 0x1c0b0000 0x0 0x1000>;
		compatible = "arm,pl011", "arm,primecell";
	};

	apb-pclk {
		phandle = <0x8000>;
		clock-output-names = "clk24mhz";
		clock-frequency = <0x16e3600>;
		#clock-cells = <0x0>;
		compatible = "fixed-clock";
	};

	chosen {
        bootargs = "earlycon console=ttyAMA0,115200n8 carrier_timeout=0";
	    stdout-path = "/pl011@1c090000";
	};
};
```

## Clone Linux v6.1

```bash
export LINUX_CFG_FRAG=$WORKDIR/bao-demos/guests/linux/configs/base.config
git clone --depth 1 -b v6.1 https://github.com/torvalds/linux.git
```

The LINUX_CFG_FRAG environment variable stores the path to our modified base.config file, which will be used during the Buildroot build process.

## Build Buildroot 2022.11

```bash
git clone --depth 1 -b 2022.11 https://github.com/buildroot/buildroot.git
make ARCH=aarch64 -C buildroot defconfig BR2_DEFCONFIG=$WORKDIR/bao-demos/guests/linux/buildroot/aarch64.config
```

When building manually, update the "BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE" in buildroot/.config to match the required Linux kernel version (6.1 in this case).

After making these changes, run the following commands to build:

```bash
make -C buildroot linux-reconfigure all
cp buildroot/output/images/*Image $WORKDIR/outputs/Image-fvp-a
```

## Build the device tree and wrap it with the kernel image

```bash
# VM 1
dtc $WORKDIR/outputs/linux1.dts > $WORKDIR/outputs/linux1.dtb
make -C $WORKDIR/bao-demos/guests/linux/lloader ARCH=aarch64 IMAGE=$WORKDIR/outputs/Image-fvp-a DTB=$WORKDIR/outputs/linux1.dtb TARGET=$WORKDIR/outputs/linux1

# VM 2
dtc $WORKDIR/outputs/linux2.dts > $WORKDIR/outputs/linux2.dtb
make -C $WORKDIR/bao-demos/guests/linux/lloader ARCH=aarch64 IMAGE=$WORKDIR/outputs/Image-fvp-a DTB=$WORKDIR/outputs/linux2.dtb TARGET=$WORKDIR/outputs/linux2
```

## Build Bao

```bash
git clone -b demo https://github.com/bao-project/bao-hypervisor.git
```

The default Bao Hypervisor supports up to 2GB of memory. We'll modify it to support 4GB to allocate 1GB to each Linux VM. Apply the following patch:

```diff
diff --git a/src/platform/fvp-a/fvpa_desc.c b/src/platform/fvp-a/fvpa_desc.c
index 7cad46a..0cd091b 100644
--- a/src/platform/fvp-a/fvpa_desc.c
+++ b/src/platform/fvp-a/fvpa_desc.c
@@ -11,9 +11,9 @@ struct platform platform = {
     .region_num = 1,
     .regions =  (struct mem_region[]) {
         {
-            // DRAM, 0GB-2GB
+            // DRAM, 0GB-4GB
             .base = 0x80000000,
-            .size = 0x80000000
+            .size = 0x100000000
         }
     },
```

Create a VM configuration file at "$WORKDIR/outputs/configs/linux+linux.c" with the following content:

```c
#include <config.h>

VM_IMAGE(linux1_image, XSTR(BAO_DEMOS_WRKDIR_IMGS/linux1.bin));
VM_IMAGE(linux2_image, XSTR(BAO_DEMOS_WRKDIR_IMGS/linux2.bin));

struct config config = {
    .vmlist_size = 2,
    .vmlist = {
        { 
            .image = {
                .base_addr = 0xa0000000,
                .load_addr = VM_IMAGE_OFFSET(linux1_image),
                .size = VM_IMAGE_SIZE(linux1_image),
            },

            .entry = 0xa0000000,

            .platform = {
                .cpu_num = 2,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0xa0000000,
                        .size = 0x40000000,
                        .place_phys = true,
                        .phys = 0xa0000000,
                    }
                },

                .dev_num = 2,
                .devs =  (struct vm_dev_region[]) {
                    {   
                        /* UART1, PL011 */
                        .pa = 0x1c0a0000,
                        .va = 0x1c0a0000,
                        .size = 0x10000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {38} 
                    },
                    {   
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {27} 
                    }
                },

                .arch = {
                    .gic = {
                        .gicd_addr = 0x2F000000,
                        .gicr_addr = 0x2F100000,
                    }
                }
            },
        },
        { 
            .image = {
                .base_addr = 0xe0000000,
                .load_addr = VM_IMAGE_OFFSET(linux2_image),
                .size = VM_IMAGE_SIZE(linux2_image),
            },

            .entry = 0xe0000000,

            .platform = {
                .cpu_num = 2,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0xe0000000,
                        .size = 0x40000000,
                        .place_phys = true,
                        .phys = 0xe0000000,
                    }
                },

                .dev_num = 2,
                .devs =  (struct vm_dev_region[]) {
                    {   
                        /* UART2, PL011 */
                        .pa = 0x1c0b0000,
                        .va = 0x1c0b0000,
                        .size = 0x10000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {39} 
                    },
                    {   
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {27} 
                    }
                },

                .arch = {
                    .gic = {
                        .gicd_addr = 0x2F000000,
                        .gicr_addr = 0x2F100000,
                    },
                },
            },
        },
    },
};
```

After applying these changes, build the hypervisor:

```bash
make -C bao-hypervisor \
        PLATFORM=fvp-a \
        CONFIG_REPO=$WORKDIR/outputs/config \
        CONFIG=linux+linux \
        CPPFLAGS=-DBAO_DEMOS_WRKDIR_IMGS=$WORKDIR/outputs
        
cp $WORKDIR/bao-hypervisor/bin/fvp-a/linux+linux/bao.bin $WORKDIR/resources/semihosting/bao.bin
```

## Build U-boot

```bash
git clone --depth 1 -b v2022.10 https://github.com/u-boot/u-boot.git
make -C u-boot vexpress_aemv8a_semi_defconfig
make -C u-boot -j $(nproc)
cp u-boot/u-boot.bin $WORKDIR/outputs/
```

## Build Trusted Firmware-A

```bash
git clone --depth 1 -b v2.9 https://github.com/ARM-software/arm-trusted-firmware.git
make -C arm-trusted-firmware PLAT=fvp bl1 fip ARCH=aarch64 BL33=$WORKDIR/outputs/u-boot.bin

cp $WORKDIR/arm-trusted-firmware/build/fvp/release/fip.bin $WORKDIR/resources/binaries/fip.bin
cp $WORKDIR/arm-trusted-firmware/build/fvp/release/bl1.bin $WORKDIR/resources/binaries/bl1.bin
```

## Run and Connect to the Simulation

Verify that your resources directory has the following structure:

```
resources
    binaries
        bl1.bin
        fip.bin
    semihosting
        bao.bin
```

Start the simulation:

```bash
export M5_PATH=resources/ && \
	gem5/build/ARM/gem5.opt gem5/configs/example/arm/baremetal.py \
	   --workload ArmTrustedFirmware \
	   --num-cores 4 \
	   --mem-size 4GB \
	   --machine-type VExpress_GEM5_Foundation \
	   --semi-enable --semi-path resources/semihosting
```

Connect to the simulation:

```bash
gem5/util/term/m5term 3456 # Bao Hypervisor
gem5/util/term/m5term 3457 # VM 1 (Linux)
gem5/util/term/m5term 3458 # VM 2 (Linux)
```

Monitor the boot process through the terminals. When U-Boot starts, press any key to interrupt the auto-boot process. Then, load and run the Bao binary using semihosting with these commands:

```bash
load hostfs - 0x90000000 bao.bin
go 0x90000000
```

Once the system is up, you can access the Linux shells on ports 3457 and 3458. Log in using "root" as both username and password.
