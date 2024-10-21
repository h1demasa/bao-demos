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
