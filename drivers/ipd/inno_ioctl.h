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

#ifndef __IPD_IOCTL_H__
#define __IPD_IOCTL_H__

#include <linux/types.h>

#ifndef __KERNEL__
#include <net/if.h>
#include <linux/genetlink.h>
#endif

#define IFCS_IPD_MAGIC             'I'
#define IFCS_PCIE_IOCTL_VERSION    1

/* Track Innovium nodes */
#define MAX_INNO_DEVICES    8

#ifndef LDH_HEADER_MAX_NUM
/* We support 2 LDH headers on a packet */
#define LDH_HEADER_MAX_NUM  2

/* 20 Bytes of LDH */
#ifndef LDH_HEADER_MAX_SIZE
#define LDH_HEADER_MAX_SIZE 20
#endif
#define LDH_MAX_HEADERS_SIZE (LDH_HEADER_MAX_SIZE*LDH_HEADER_MAX_NUM)

#endif

#define VERSION_SIZE        32

#define MAX_FLUSH_PKT_LEN   255

#ifndef MODULE
typedef uint64_t dma_addr_t;
#endif

/** @brief Struct to hold date needed to free a dma coherent alloc
 *
 */
typedef struct {
    dma_addr_t   dma_addr;              /** Physical addr */
    struct page  *page;                 /** Page pointer (TX - zcopy) */
    void         *vmaddr;               /** VMaddr */
} inno_dma_alloc_t;

/* Structs for various IOCTL calls */
typedef struct {
    uint16_t          version;              /** IOCTL version */
    uint8_t           instance;             /** PCIe device instance */
    uint8_t           unused1;              /** Padding for 32 bit compat */
    uint32_t          unused2;
} inno_ioctl_hdr_t;

typedef struct {
    uint16_t vendor_id;                 /** PCI vendor ID */
    uint16_t device_id;                 /** PCI device ID */
    uint8_t rev_id;                     /** PCI revision ID */
    uint8_t bus_num;                    /** PCI bus number */
    uint8_t dev_num;                    /** PCI device number */
    uint8_t fn_num;                     /** PCI function number */
} inno_dev_info_t;

typedef struct {
    inno_ioctl_hdr_t hdr;
    uint8_t          num_nodes;
    inno_dev_info_t  node_info[MAX_INNO_DEVICES];
} inno_ioctl_nodes_t;

typedef struct {
    inno_ioctl_hdr_t hdr;
    uint64_t serial_num;                /** Device serial number */
    uint64_t bar0;                      /** Bus addr of mem bar0 */
    uint64_t bar0_size;                 /** Mapped size of bar0*/
    uint64_t pool_baddr;                /** Bus addr of pool */
    uint64_t pool_paddr;                /** Physical addr of pool */
    uint16_t vendor_id;                 /** PCI vendor ID */
    uint16_t device_id;                 /** PCI device ID */
    uint8_t rev_id;                     /** PCI revision ID */
    uint8_t bus_num;                    /** PCI bus number */
    uint8_t dev_num;                    /** PCI device number */
    uint8_t fn_num;                     /** PCI function number */
    uint32_t pool_size;                 /** Size of pool in bytes */
    uint32_t rupt_wb_size;              /** Size of rupt WB */
    uint32_t rupt_wb_offset;            /** Offset of rupt WB in pool */
    uint32_t sync_blk_size;             /** Size of sync blk dma area */
    uint32_t sync_blk_offset;           /** Offset of sync blk dma area in pool */
    uint32_t sync_wb_size;              /** Size of sync wb area */
    uint32_t sync_wb_offset;            /** Offset of sync wb area in pool */
    uint32_t dma_cache_align;           /** Required cache alignment */
    uint8_t  version[VERSION_SIZE];     /** Driver version */
} inno_ioctl_query_t;

/* The num MSIX intrs could be less than INNO_MSIX_NUM_VECTORS */
#define RUPT_MASK_WORDS_MAX   10

#define MSIX_VECTOR_NAPI   0               /* NAPI interrupts (driver) */
#define MSIX_VECTOR_ASYNC  1               /* Async rupts */
#define MSIX_VECTOR_TX     2               /* TX ring rupts */
#define MSIX_VECTOR_RX     3               /* RX ring rupts */
#define MSIX_VECTOR_ERROR  4               /* Error rupts */
#define MSIX_VECTOR_LEARN  5               /* Learn rupts */
#define MSIX_VECTOR_PIC    6               /* pic rupts */
#define MSIX_VECTOR_RX_MCU0 7              /* RX ring MCU0 */
#define NUM_MSIX_VECS      8

typedef struct {
    inno_ioctl_hdr_t hdr;
    uint16_t vector;                        /** Vector number */
    uint32_t mask[RUPT_MASK_WORDS_MAX];     /** Rupts to wait/fired */
} inno_ioctl_rupt_mask_t;

typedef struct {
    inno_ioctl_hdr_t hdr;
    uint64_t timeout;                       /** Timeout for wait */
    uint64_t remaining_time;                /** Timeout remaining */
    uint32_t vector;                        /** Vector to wait */
    uint32_t rupts[RUPT_MASK_WORDS_MAX];    /** Rupts fired */
} inno_ioctl_rupt_wait_t;

typedef enum {
    INNO_RING_INIT   = 1,              /** Common flag - ignore as param */
    INNO_RING_ENABLE = 2,              /** Common flag - ignore as param */
    INNO_RING_NETDEV = 4,              /** Ring uses netdev xface */
    INNO_RING_TX     = 8,              /** TX/RX ring */
} inno_ring_flags_t;

typedef enum {
    INNO_LEARN_INIT   = 1,             /** Common flag - ignore as param */
    INNO_LEARN_ENABLE = 2,             /** Common flag - ignore as param */
} inno_learn_flags_t;

typedef struct {
    inno_ioctl_hdr_t hdr;
    uint64_t          ring;               /** bus address of descr ring */
    uint64_t          wb;                 /** bus address of writeback */
    uint64_t          ring_pa;            /** physical address of descr ring */
    uint64_t          wb_pa;              /** physical address of writeback */
    uint16_t          count;              /** Number of descriptors */
    uint16_t          vector;             /** Interrupt vector num */
    inno_ring_flags_t flags;              /** Type flags */
    uint8_t           num;                /** Ring number */
    uint32_t          idx_offset;         /** Offset of IDX in pool */
    uint16_t          cidx;               /** Current CIDX */
    uint16_t          pidx;               /** Current PIDX */
} inno_ioctl_ring_t;

typedef struct {
    inno_ioctl_hdr_t hdr;
    inno_ring_flags_t flags;              /** Type flags */
    uint8_t           num;                /** Ring number */
    uint16_t          last_pidx;          /* PIDX where we stop filling */
} inno_ioctl_desc_t;

typedef struct {
    inno_ioctl_hdr_t hdr;
    uint64_t hrr;                           /** Bus addr of ring */
    uint64_t hrr_pa;                        /** Physical addr of ring */
    uint16_t size;                          /** Size of each entry */
    uint16_t count;                         /** Number of H/W slots */
    uint16_t descriptors;                   /** Number of S/W descriptors */
    uint32_t hrr_pidx_offset;               /** Pool offset of HRR PIDX */
} inno_ioctl_hrr_t;

typedef struct {
    inno_ioctl_hdr_t hdr;
    uint64_t learn_ba;                      /** Bus address of ring */
    uint64_t learn_pa;                      /** Physical address of ring */
    uint64_t wb_ba;                         /** Bus address of learn wb */
    uint64_t wb_pa;                         /** Physical address of learn wb */
    uint16_t count;                         /** Number of descriptors */
    uint32_t pidx_offset;                   /** Pool offset of LEARN PIDX */
    uint16_t cidx;                          /** Current CIDX, IFCS is the consumer */
    uint16_t pidx;                          /** Current PIDX, device is the producer */
} inno_ioctl_learn_t;

typedef struct {
    inno_ioctl_hdr_t hdr;
    dma_addr_t wb_ba;                       /** Bus address of PIC status chain wb */
    uint64_t   wb_pa;                       /** Physical address of PIC status chain wb */
    uint32_t count;                         /** Number of entries */
} inno_ioctl_pic_st_t;

typedef enum {
    IOCTL_SEND_ZCOPY_FLAG = 1,                /** Zero copy */
    IOCTL_SEND_LDH_FLAG = 2                   /** LDH field present */
} ioctl_send_flag_t;

typedef struct {
    inno_ioctl_hdr_t hdr;
    uint64_t buf;                          /** VM of buffer */
    uint8_t  ring_num;                      /** Ring number to use */
    uint32_t buf_len;                       /** Buffer len */
    uint16_t start_pidx;                    /** PIDX of first desc */
    uint16_t last_pidx;                     /** PIDX of last desc */
    uint8_t  ldh[LDH_MAX_HEADERS_SIZE];      /** LDH to send */
    uint8_t  num_ldh;                       /** Number of LDH headers */
    ioctl_send_flag_t flags;                /** Control flags */
} inno_ioctl_tx_send_t;

typedef struct {
    inno_ioctl_hdr_t hdr;
    uint8_t  hrr;                           /** Unpin from HRR ring */
    uint8_t  ring_num;                      /** Ring number to use */
    uint16_t idx;                           /** First IDX */
} inno_ioctl_unpin_pages_t;

#define SYSHDR_SIZE 20
#define INNO_NETDEV_SINGLE_INTERFACE 1
#define INNO_NETDEV_CPU_PORT 2

typedef struct {
    inno_ioctl_hdr_t hdr;
    uint16_t sysport;                       /** System port for netdev */
    struct   sockaddr ethaddr;              /** Ethernet mac address */
    uint32_t flag;                          /** Netdev flags */
    char     intf_name[IFNAMSIZ+1];           /** Netdev interface name */
} inno_ioctl_netdev_t;

typedef enum {
    INNO_ND_INFOHDR,
    INNO_ND_VLANTAG,
    INNO_ND_SYSHDR,
    INNO_ND_IFSTATE,
    INNO_ND_LINKSTATE,
    INNO_ND_MTU,
    INNO_ND_RING,
    INNO_ND_PTP_EVENT_INFO,
    INNO_ND_INGRESS_SFLOW_SAMPLE_RATE,
    INNO_ND_EGRESS_SFLOW_SAMPLE_RATE
}inno_netdev_info_id_t;

typedef enum {
    INNO_RING_TYPE_TX    = 1,               /** TX ring */
    INNO_RING_TYPE_RX    = 2,               /** RX ring */
    INNO_RING_TYPE_HRR   = 3,               /** Async Host response ring */
    INNO_RING_TYPE_LEARN = 4                /** Learn ring */
} inno_ring_type_t;

typedef struct __attribute__((__packed__)) {
    inno_ioctl_hdr_t hdr;
    uint16_t id;
    uint16_t sysport;                                /** System port for netdev */
    union {
        struct {
            uint8_t  syshdr1[SYSHDR_SIZE*2];         /** Syshdr1 for this port */
            uint8_t  syshdr1_cnt;                    /** Count of syshdr per port */
            uint8_t  syshdr1_abp[SYSHDR_SIZE*2];     /** Syshdr1_abp for this port */
            uint8_t  syshdr1_ptp[SYSHDR_SIZE];     /** Syshdr1_ptp for this port */
            uint8_t  syshdr2[SYSHDR_SIZE];           /** Syshdr2 for this port */
        }ldh;
        struct {
            inno_ring_type_t    type;                /** Ring type TX/RX */
            uint32_t            num;                 /** Ring number */
            uint64_t            packets;             /** Packets sent/recvd */
            uint64_t            bytes;               /** Total bytes sent/recvd */
        }ring;
        struct {
            uint32_t    ptp_event_queue_id;     /* PTP event queue id */
            bool        ptp_event_queue_valid;  /* is PTP event queue valid */
        }ptp_info;
        struct sockaddr ethaddr;
        char intf_name[IFNAMSIZ+1];                /** Netdev interface name */
        uint16_t vtag;                           /** VLAN tag action */
        uint16_t ifstate;                        /** Interface state */
        uint16_t linkstate;                      /** Link state */
        uint16_t mtu;                            /** MTU */
        uint32_t sflow_sample_rate;              /** Sflow sample rate */
    }u;
}inno_ioctl_netdev_info_t;

typedef enum {
        INNO_NL_SFLOW=1,
        INNO_NL_PGENL,
}netLinkAppType;

typedef struct {
    inno_ioctl_hdr_t hdr;
    uint32_t         type;                         /** Netlink type */
    netLinkAppType   nl_app_type;                  /** Netlink type */
    char             intf_name[IFNAMSIZ+1];          /** Netlink interface name */
    char             gen_mcgrp_name[GENL_NAMSIZ];  /** Generic Netlink mcgrp name */
} inno_ioctl_netlink_t;

typedef enum {
    INNO_NL_FLAG,
    INNO_NL_TRAPID
}inno_netlink_info_id_t;

typedef struct __attribute__((__packed__)) {
    inno_ioctl_hdr_t hdr;
    uint16_t         id;
    char             intf_name[IFNAMSIZ];          /** Netlink interface name */
    union {
        uint32_t         flag;                     /** Flag */
        struct {
            uint32_t     trapid;
            uint32_t     set;
        }trapid_info;
    }u;
}inno_ioctl_netlink_info_t;

typedef struct {
    inno_ioctl_hdr_t  hdr;
    uint64_t          buf_ba;               /** Buffer's bus address returned */
    uint64_t          buf_pa;               /** Buffer's physical address returned */
    inno_ring_type_t  ring_type;            /** If this is a TX/RX/HRR ring */
    uint8_t           ring_num;             /** Ring number */
    uint32_t          idx;                  /** Index inside a ring */
} inno_ioctl_ring_page_alloc_t;

typedef struct inno_params_info_s {
    uint16_t            shim_gre_proto;
    uint16_t            rsvd;
} inno_params_info_t;

typedef struct {
    inno_ioctl_hdr_t  hdr;
    inno_params_info_t info;
} inno_ioctl_params_t;

typedef struct {
    inno_ioctl_hdr_t hdr;
    uint8_t          pkt_len;                /** Packet length */
    uint8_t          pkt[MAX_FLUSH_PKT_LEN]; /** Packet to be sent */
} inno_ioctl_flush_pkt_t;

typedef struct {
   int value;
} inno_ioctl_cfg_init_done_t;

typedef struct {
   int value;
} inno_ioctl_flow_init_t;
/* Probes number of nodes */

#define IPD_INFO_NODES    _IOR(IFCS_IPD_MAGIC, 1, inno_ioctl_nodes_t *)

/* Query Per Node Information */
#define IPD_QUERY_NODE    _IOWR(IFCS_IPD_MAGIC, 2, inno_ioctl_query_t *)

/* Set up for interrupt */
#define IPD_RUPT_MASK    _IOWR(IFCS_IPD_MAGIC, 3, inno_ioctl_rupt_mask_t *)

/* Wait for interrupt */
#define IPD_RUPT_WAIT    _IOWR(IFCS_IPD_MAGIC, 4, inno_ioctl_rupt_wait_t *)

/* Read/Write device memory */
#define IPD_READ_MEM     _IOWR(IFCS_IPD_MAGIC, 5, int32_t *)
#define IPD_WRITE_MEM    _IOWR(IFCS_IPD_MAGIC, 6, int32_t *)

/* Allocate/release a ring */
#define IPD_RING_ALLOC    _IOWR(IFCS_IPD_MAGIC, 7, inno_ioctl_ring_t *)
#define IPD_RING_FREE     _IOWR(IFCS_IPD_MAGIC, 8, inno_ioctl_ring_t *)

/* Allocate/release HRR */
#define IPD_HRR_ALLOC    _IOWR(IFCS_IPD_MAGIC, 9, inno_ioctl_hrr_t *)
#define IPD_HRR_FREE     _IOWR(IFCS_IPD_MAGIC, 10, inno_ioctl_hrr_t *)

/* Allocate/release LEARN */
#define IPD_LEARN_ALLOC    _IOWR(IFCS_IPD_MAGIC, 11, inno_ioctl_learn_t *)
#define IPD_LEARN_FREE     _IOWR(IFCS_IPD_MAGIC, 12, inno_ioctl_learn_t *)

/* Put a TX packet into a ring */
#define IPD_TX_SEND       _IOWR(IFCS_IPD_MAGIC, 13, inno_ioctl_tx_send_t *)
#define IPD_UNPIN_PAGE    _IOWR(IFCS_IPD_MAGIC, 14, inno_ioctl_unpin_pages_t *)

/* Create/Delete netdev interface */
#define IPD_CREATE_NETDEV    _IOWR(IFCS_IPD_MAGIC, 15, inno_ioctl_netdev_t *)
#define IPD_DELETE_NETDEV    _IOWR(IFCS_IPD_MAGIC, 16, inno_ioctl_netdev_t *)

/* Ring page memory allocation */
#define IPD_RING_PAGE_ALLOC _IOWR(IFCS_IPD_MAGIC, 17, inno_ioctl_ring_page_alloc_t *)
#define IPD_RING_PAGE_FREE  _IOWR(IFCS_IPD_MAGIC, 18, inno_ioctl_ring_page_alloc_t *)

/* Allocate/release PIC status chain */
#define IPD_PIC_STATUS_ALLOC  _IOWR(IFCS_IPD_MAGIC, 19, inno_ioctl_pic_st_t *)
#define IPD_PIC_STATUS_FREE   _IOWR(IFCS_IPD_MAGIC, 20, inno_ioctl_pic_st_t *)
#define IPD_PIC_STATUS_DUMP   _IOWR(IFCS_IPD_MAGIC, 21, inno_ioctl_pic_st_t *)

#define IPD_DEBUG_EVT_THREAD_TIMER   _IOWR(IFCS_IPD_MAGIC, 22, inno_ioctl_pic_st_t *)

/* Enable/Disable a ring */
#define IPD_RING_ENABLE    _IOWR(IFCS_IPD_MAGIC, 23, inno_ioctl_ring_t *)
#define IPD_RING_DISABLE   _IOWR(IFCS_IPD_MAGIC, 24, inno_ioctl_ring_t *)

/* Get/Set netdev interface info */
#define IPD_GET_NETDEV    _IOWR(IFCS_IPD_MAGIC, 25, inno_ioctl_netdev_info_t *)
#define IPD_SET_NETDEV    _IOWR(IFCS_IPD_MAGIC, 26, inno_ioctl_netdev_info_t *)

/* Create/delete netlink interface */
#define IPD_CREATE_NETLINK _IOWR(IFCS_IPD_MAGIC, 27, inno_ioctl_netlink_t *)
#define IPD_DELETE_NETLINK _IOWR(IFCS_IPD_MAGIC, 28, inno_ioctl_netlink_t *)

/* Get/Set netlink interface info */
#define IPD_SET_NETLINK    _IOWR(IFCS_IPD_MAGIC, 29, inno_ioctl_netlink_info_t *)

/* Get/Set IPD params info */
#define IPD_GET_PARAMS    _IOWR(IFCS_IPD_MAGIC, 30, inno_ioctl_params_t *)
#define IPD_SET_PARAMS    _IOWR(IFCS_IPD_MAGIC, 31, inno_ioctl_params_t *)

/* Initialize DMA */
#define IPD_DMA_INIT       _IOW(IFCS_IPD_MAGIC, 32, void *)

/* Send flush packet and recover DMA */
#define IPD_FLUSH_RECOVER_DMA _IOW(IFCS_IPD_MAGIC, 33, inno_ioctl_flush_pkt_t *)

/* Initialize HW */
#define IPD_HW_INIT               _IOW(IFCS_IPD_MAGIC, 34, void *)

/* check hw init state */
#define IPD_IS_USER_CONFIG_DONE   _IOW(IFCS_IPD_MAGIC, 35, int)

/* Set ipd user config done */
#define IPD_USER_CONFIG_DONE      _IOW(IFCS_IPD_MAGIC, 36, int)

/* clear ipd user config done */
#define IPD_USER_CONFIG_CLEARED   _IOW(IFCS_IPD_MAGIC, 37, int)

/* Override ipd flow control */
#define IPD_OVERRIDE_FLOW_CONTROL _IOW(IFCS_IPD_MAGIC, 38, int)

/* Override ipd cleanup resources */
#define IPD_CLEANUP_RESOURCES     _IOW(IFCS_IPD_MAGIC, 39, int)
#endif /* __IPD_IOCTL_H__ */
