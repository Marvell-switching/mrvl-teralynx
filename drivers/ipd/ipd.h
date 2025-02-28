/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
*******************************************************************************/

#ifndef __IPD_H__
#define __IPD_H__

#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/genetlink.h>

/** @file ipd.h
 *
 * @brief Header file for innovium PCIe kernel module
 */

#include "inno_ioctl.h"
#include "inno_ring.h"
#include "inno_debug.h"

#define BAR_0    0

#define PAGE(addr)      (((uint64_t)(addr)) & ~(PAGE_SIZE - 1))
#define OFFSET(addr)    (((uint64_t)(addr)) & (PAGE_SIZE - 1))
#define CACHE_ALIGN(x)               \
    (((x) + idev->cache_align - 1) & \
     ~(idev->cache_align - 1))

/* Macros to find the H/W registers and VM addrs of H/W features */
#define REG32(reg)    (*((volatile uint32_t *)(idev->bar0 + (reg))))
#define REG64(reg)    (*((volatile uint64_t *)(idev->bar0 + (reg))))
#define UPPER32(val)  ((uint32_t) ((val) >> 32))
#define LOWER32(val)  ((uint32_t) ((val) & 0x00000000ffffffff))

/* Length allocated in RX SKB headers.  Should fit most small packets
 * but not wasste too much space */
#define INNO_RX_SKB_HDR_LEN    256

#define INNO_DMA_BLOCK_SIZE    4096

#define MIN_PACKET_SIZE        64

#define INNO_PCI_VENDOR_ID             0x1d98
#define MRVL_PCI_VENDOR_ID             0x11ab
#define INNO_TERALYNX_PCI_DEVICE_ID    0x1b58
#define INNO_K2_PCI_DEVICE_ID          0x1b59
#define MRVL_TL10_PCI_DEVICE_ID        0x6000

#define INNO_DMA_MASK                  64
#define INNO_TERALYNX_PCI_DEVICE_REV_ID_A0    0x0
#define INNO_TERALYNX_PCI_DEVICE_REV_ID_B0    0x10

#define IPD_BOOTTYPE_COLD     0   /* cold reset */
#define IPD_BOOTTYPE_FAST     1   /* fast reset */

extern uint16_t rupt_mask_words;
struct inno_device;

typedef enum {
    INNO_FLG_MSI_WA_ENABLED = 1
}inno_dev_flags_t;

typedef enum {
    INNO_IRQ_TYPE_BEST_EFFORT = 1,
    INNO_IRQ_TYPE_MSIX_ONLY,
    INNO_IRQ_TYPE_MSI_ONLY,
    INNO_IRQ_TYPE_INTX_ONLY
}inno_intr_type_t;

#define	INNO_INTR_MSIX_ENABLED	1
#define	INNO_INTR_MSI_ENABLED	2
#define	INNO_INTR_INTX_ENABLED	4

/** @brief Main ring data structure
 *
 */
/** @brief Info about each descriptor entry
 *
 */
typedef enum {
    DESC_MAP_SINGLE,
    DESC_MAP_PAGE,
    DESC_MAP_NONE
} inno_ring_desc_map_type_t;

typedef struct inno_ring_desc_info {
    struct sk_buff            *skb;
    uint32_t                  len;      /** Len of ring data */
    dma_addr_t                dma_addr; /** DMA addr to unmap */
    inno_ring_desc_map_type_t map_type; /** Type of mapping */
} inno_ring_desc_info_t;

typedef struct inno_ring {
    struct inno_device    *idev;        /** Backpointer to dev */

    uint8_t               num;          /** Ring number */
    inno_ring_flags_t     flags;        /** Ring flags */

    union {
        inno_rx_desc_t *rx_desc;        /** Addr of desc ring */
        inno_tx_desc_t *tx_desc;        /** Addr of desc ring */
    };
    union {
        inno_rx_wb_t *rx_wb;            /** Addr of wb ring */
        inno_tx_wb_t *tx_wb;            /** Addr of wb ring */
    };
    union {
        volatile uint32_t *cidx_addr;            /** Kaddr of CIDX WB */
        volatile uint32_t *pidx_addr;            /** Kaddr of PIDX WB */
    };
    union {
        uint32_t work_cidx;             /** working CIDX value */
        uint32_t work_pidx;             /** working PIDX value */
    };
    uint32_t              cidx;         /** Local CIDX */
    uint32_t              pidx;         /** Local PIDX */

    uint32_t              count;        /** Number of descriptors */
    uint32_t              idx_offset;   /** Pool offset of IDX in */
    dma_addr_t            desc_ba;      /** PCIe bus addr of desc ring */
    dma_addr_t            wb_ba;        /** PCIe bus addr of wb ring */
    uint8_t               *tx_cksum;    /** TX checksum */
    dma_addr_t            tx_cksum_ba;  /** PCIe bus addr of tx checksum */

    inno_ring_desc_info_t *desc_info;   /** Descriptor info array (Netdev) */

    inno_dma_alloc_t      *pages;       /** Array of ring pages */

} inno_ring_t;

/** @brief Interrupt vector definition */
typedef struct inno_rupt_vector_s {
    uint16_t          vector;                    /** Vector number */
    uint32_t          mask[RUPT_MASK_WORDS_MAX]; /** Rupts in this vector */
    uint32_t          pend[RUPT_MASK_WORDS_MAX]; /** Rupts pending */
    volatile int      flag;                      /** Wait flag */
    wait_queue_head_t wait_q;                    /** Rupt ioctl waitQ */
} inno_rupt_vector_t;

/** @brief Interrupt registers */
typedef struct inno_intr_regs_s{
    uint32_t         intr_incr;
    uint32_t         intr_inms;
    uint32_t         intr_inmc;
    uint32_t         intr_immr;
    uint32_t         intr_iac_cause;
    uint32_t         intr_iac_pci_mask;
} inno_intr_regs_t;

/** @brief HRR definition */
typedef struct inno_hrr_s {
    void                    *vmaddr;        /** Kaddr of async HRR */
    uint32_t                count;          /** Number of ring entries */
    uint32_t                size;           /** Size of each ring entry  */
    uint32_t                descriptors;    /** Number of HRR descriptor */
    dma_addr_t              ba;             /** PCIe bus addr of async HRR */
    inno_dma_alloc_t        *pages;         /** Async FIFO page buffers */
} inno_hrr_t;

/** @brief HRR definition */
typedef struct inno_learn_s {
    void                      *vmaddr;        /** vaddr of LEARN */
    void                      *wb_vmaddr;     /** vaddr of LEARN WB */
    inno_learn_flags_t        flags;          /** Learn flags */
    uint32_t                  count;          /** Number of LEARN descriptors */
    uint32_t                  size;           /** Size of each LEARN descriptor */
    dma_addr_t                ba;             /** PCIe bus addr of LEARN */
    dma_addr_t                orig_ba;        /** Original PCIe bus addr of LEARN */
    uint32_t                  pidx_offset;    /** Offset of producer idx */

    dma_addr_t                wb_ba;          /** PCIe bus addr of wb */
    uint32_t                  wb_pidx_offset; /** Offset of producer idx */

    rxe_dma_msg_data_pidx_0_t pidx;           /** Local PIDX */
    rxe_dma_msg_data_cidx_0_t cidx;           /** Local CIDX */
} inno_learn_t;

typedef struct inno_pic_st_s {
    volatile void           *wb_vmaddr;       /** vaddr of PIC Status chain WB */
    uint32_t                count;            /** Number of entries */
    dma_addr_t              wb_ba;            /** Bus address of PIC status chain wb */
} inno_pic_st_t;

#define POOL_SIZE       (2 * PAGE_SIZE)

#define NUM_SYSPORTS    (1 << 12)

typedef struct inno_netdev_s {
    uint8_t     single_interface;
    uint8_t     info_header;
    uint16_t    ndev_sindex;
    uint16_t    num_interfaces;
    uint8_t     vlan_action[NUM_SYSPORTS];
    uint16_t    ptp_event_queue_id;
    bool        ptp_event_queue_valid;
    uint32_t    isflow_sample_rate[NUM_SYSPORTS];
    uint32_t    esflow_sample_rate[NUM_SYSPORTS];
    uint32_t    cpu_port[NUM_SYSPORTS];
} inno_netdev_t;

typedef struct inno_gen_netlink_stats_s {
    uint64_t rx_packets;
    uint64_t rx_bytes;
    uint64_t rx_drops;
} inno_gen_netlink_stats_t;

typedef struct inno_gen_netlink_tid_s {
    struct list_head          list;
    uint32_t                  trapid;
    inno_gen_netlink_stats_t  stats;
    struct inno_gen_netlink_s *inno_genl;
}inno_gen_netlink_tid_t;

typedef struct inno_gen_netlink_s {
    uint32_t                  seq;                // Sequence number
    netLinkAppType            nl_type;            // Netlink type
    struct genl_family        inno_genl_family;
    inno_gen_netlink_stats_t  stats;
    struct inno_gen_netlink_s *next;
} inno_gen_netlink_t;

typedef struct inno_netlink_s {
    uint32_t            sflow;
    inno_gen_netlink_t  *sflow_genl;
    inno_gen_netlink_t  *gnetlink;
} inno_netlink_t;

/** @brief Innovium device master struct
 *
 */
typedef struct inno_device {
    struct pci_dev          *pdev;                 /** PCI device */
    struct net_device       *ndev;                 /** Network device */

    struct net_device       *sysport_devs[NUM_SYSPORTS]; /** Network devices */

    int                     instance;              /** Instance num of this dev */
    uint16_t                vendor_id;             /** PCI vendor id */
    uint16_t                device_id;             /** PCI device id */
    uint8_t                 rev_id;                /** PCI revision id */
    uint64_t                serial_num;            /** Dev serial num */
    uint8_t                 bus_num;               /** PCI Bus number */
    uint8_t                 dev_num;               /** PCI Device number */
    uint8_t                 fn_num;                /** PCI Function number */

    uint32_t                cache_align;           /** Cache alignment */

    void __iomem            *bar0;                 /** BAR 0 kaddr */
    void __iomem            *bar0_ba;              /** BAR0 address */
    uint32_t                bar0_size;             /** Size of BAR0 */

    /* Rings */
    inno_ring_t             tx_ring[NUM_TX_RINGS]; /** TX rings */
    inno_ring_t             rx_ring[NUM_RX_RINGS]; /** RX rings */

    int                     last_ring_num;         /** Last round-robin ring */

    /* Interrupts */
    int                     num_vectors;           /** Actual number obtained */
    inno_rupt_vector_t      rupts[NUM_MSIX_VECS];  /** Interrupt vectors */

    struct msix_entry       msix[NUM_MSIX_VECS];   /** Memory for MSIX vectors */
    char                    msix_names[NUM_MSIX_VECS][IFNAMSIZ + 10]; /** MSIX names */
    uint32_t                immr[256/32*5];       /** IMMR shadow */


    uint8_t                 *pool;                 /** Kaddr of pool */
    dma_addr_t              pool_ba;               /** Bus addr of pool */

    spinlock_t              rupt_lock;             /** Spinlock for rupt info */
    spinlock_t              lock;                  /** Generic spinlock */
    spinlock_t              napi_lock;             /** lock for NAPI poll spinlock */

    struct napi_struct      napi;                  /** Struct for NAPI rupts */

    uint32_t                napi_mask[RUPT_MASK_WORDS_MAX]; /** NAPI interrupts */
    int                     napi_init;             /** 1 == NAPI initialized */

    uint8_t                 enet_tx_ring_num;      /** Ring num of enet tx */
    uint8_t                 enet_rx_ring_num;      /** Ring num of enet rx */

    uint8_t                 intr_type;
    /* System headers for TX netdev */
    dma_addr_t              syshdr1_ba;                /** DMA of syshdr1 */
    char                    *syshdr1;                  /** VMaddr of syshdrs */
    uint8_t                 syshdr1_cnt[NUM_SYSPORTS]; /** syshdr1 count */
    dma_addr_t              syshdr1_abp_ba;            /** DMA of syshdr1_abp */
    char                    *syshdr1_abp;              /** VMaddr of syshdrs */
    dma_addr_t              syshdr1_ptp_ba;            /** DMA of syshdr1_ptp */
    char                    *syshdr1_ptp;              /** VMaddr of syshdrs ptp */
    dma_addr_t              syshdr2_ba;                /** DMA of syshdr2 */
    char                    *syshdr2;                  /** VMaddr of syshdrs */
    /* Checksum desc for TX netdev */
    dma_addr_t              tx_netdev_cksum_ba;    /** DMA of tx cksum */
    char                    *tx_netdev_cksum;      /** VMaddr of tx cksum */

    /* Sync response area */
    dma_addr_t              sync_resp_ba;          /** Sync response bus addr */
    void                    *sync_resp;            /** Kaddr of sync response */

    inno_hrr_t              hrr;                   /** Host response ring */
    inno_dma_alloc_t        sync_page;             /** Sync page buffer */
    inno_learn_t            learn;                 /** Learn ring */

    inno_pic_st_t           pic_st;                /** PIC Status chain */

    uint32_t                msi_wa_vector;
    uint32_t                flags; /** Any flags to maintain; enum: inno_dev_flags_t */

    inno_stats_t            inno_stats;            /** Stats */

    inno_netdev_t           inno_netdev;           /** Netdev related info */
    inno_netlink_t          inno_netlink;          /** Netlink related info */
    struct list_head        trapid_list;;          /** Netlink trapid list */

    uint8_t                 chip_hdr_len;          /** Device specific CPU hdr len */
    uint8_t                 chip_dbg_hdr_len;      /** Device specific CPU DBG hdr len */

    inno_intr_regs_t        inno_intr_regs;        /** Struct for INTR_* registers */

    inno_params_info_t      inno_params_info;      /** Struct for inno device parameters */
} inno_device_t;

#endif /* __IPD_H__ */
