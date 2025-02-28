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

#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/netdevice.h>
#include <linux/pagemap.h>
#include <linux/ethtool.h>
#include <linux/pci.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
#include <linux/pci-aspm.h>
#endif
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/wait.h>
#include <linux/rtnetlink.h>

#include "pci_common_ipd.h"
#include "ipd.h"
#include "inno_ldh.h"
#include "inno_enet.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
inno_gen_netlink_t* inno_gen_netlink_search(inno_device_t  *idev, char *name);
inno_gen_netlink_t* inno_gen_netlink_search_sflow(inno_device_t  *idev);
static struct genl_multicast_group inno_genl_sflow_mcgrps = {
     .name = "packets",
};
#endif

/*
 *  Callback for device specific calls
 */
void (*inno_populate_inno_hdr) (inno_ldh_t *ldh_hdr,
                                inno_info_header_t *ih,
                                inno_dma_alloc_t  *dma,
                                uint8_t           ext_hdr_type,
                                uint8_t           ext_hdrs_size) = NULL;
int (*inno_unpack_ldh_header) (uint8_t *buf,
                               inno_ldh_t *ldh_hdr) = NULL;
bool (*inno_debug_hdr_present) (inno_ldh_t *ldh_hdr) = NULL;
/**
 * Unpack any inno extension headers in the packet (other than ldh+debug_hdr; ex: ptp shim)
 *
 * param [in]  idev   - inno_device_t pointer
 * param [in]  dma     - pointer to the packet
 * param [in]  ldh_hdr      - pointer to ldh header
 * param [out] ext_hdrs_size - size of extension headers in the packet
 * param [out] ext_info_hdrs - unpacked extension info, that driver sends to NOS
 * param [out] ext_info_hdrs_size - size of ext_info_hdrs
 * param [out] ext_hdr_type - enum of extension header type
 * return status
 */
int (*inno_unpack_ext_hdrs) (inno_device_t *idev,
                             inno_dma_alloc_t  *dma,
                             inno_ldh_t *ldh_hdr,
                             uint32_t *ext_hdrs_size,
                             uint8_t *ext_info_hdrs,
                             uint32_t *ext_info_hdrs_size,
                             uint8_t *ext_hdr_type,
                             uint32_t *sp) = NULL;
uint32_t (*inno_get_ssp) (inno_ldh_t *ldh_hdr) = NULL;
int (*inno_vf2_queue_set) (uint8_t *buf,
                           uint8_t queue) = NULL;

static void
packet_hex_dump(struct sk_buff *skb)
{
    int length, linelength, remaining;
    int next = 0;
    int rowlength = 16;
    int i, j;
    uint8_t *data;

    ipd_cont("Packet hex dump:\n");
    data = (uint8_t *) skb_mac_header(skb);

    if (skb_is_nonlinear(skb)) {
        length = skb->data_len;
    } else {
        length = skb->len;
    }

    remaining = length;
    for (i = 0; i < length; i += rowlength) {
        ipd_cont( "%06x\t", next);

        linelength = min(remaining, rowlength);
        remaining -= rowlength;

        for (j = 0; j < linelength; j++) {
            ipd_cont( "%02X ", (uint32_t) data[j]);
        }

        data += linelength;
        next += rowlength;

        ipd_cont("\n");
    }
}

/*
 * Netdev ops
 */

/** @brief Net device open
 *
 *  @return ERRNO
 */
static int
inno_enet_open(struct net_device *dev)
{
    ipd_trace("Start network device %s\n",dev->name);
    netif_start_queue(dev);
    return 0;
}


/** @brief Net device close
 *
 *  @return ERRNO
 */
static int
inno_enet_close(struct net_device *dev)
{
    ipd_trace("Stop the network device - %s\n", (dev->name?dev->name:"NULL"));
    netif_stop_queue(dev);
    return 0;
}


/** @brief Net device tx
 *
 *  @return ERRNO
 */
static int
inno_enet_tx(struct sk_buff    *skb,
             struct net_device *dev)
{
    unsigned long lock_flags;

    inno_enet_adapter_t       *enet;
    struct page               *page;
    void                      *vmaddr;
    inno_device_t             *idev;
    inno_ring_t               *tx_ring;
    uint16_t                  frag_num, clean_frag_num, pidx, data_len, len;
    dma_addr_t                dma_addr;
    inno_ring_desc_info_t     *info;
    inno_ring_desc_map_type_t map_type;
    inno_tx_desc_t            desc;
    struct net_device         *ndev;
    syshdr_tx_type            syshdr_lookup_type = PIPELINE_BYPASS;
    uint32_t                  hdr_size;
    uint8_t                   abp_mode = 0;
    struct sk_buff            *ldh_skb = NULL;
    uint8_t                   queue = 0;
    uint8_t                   is_tx_queue_valid = 0;

    enet = (inno_enet_adapter_t *) netdev_priv(dev);
    if (enet == NULL) {
        ipd_err("Netdev not initilaized\n");
        return NETDEV_TX_BUSY;
    }
    idev = enet->idev;

    if (idev->inno_netdev.num_interfaces == 0){
        ipd_err("Netdev interface not created\n");
        return NETDEV_TX_BUSY;
    }

    tx_ring = &idev->tx_ring[idev->enet_tx_ring_num];
    if ((tx_ring->flags & INNO_RING_INIT) == 0 ||
            (tx_ring->flags & INNO_RING_NETDEV) == 0) {
        ipd_not("TX ring %d not initialized for netdev. Ring flag is 0x%x\n", idev->enet_tx_ring_num, tx_ring->flags);
        dev_kfree_skb_any(skb);
        return NETDEV_TX_OK;
    }

    spin_lock_irqsave(&idev->lock, lock_flags);

    /* See if there is enough space in the processed descriptors */
    if (RING_SPACE(tx_ring->count, tx_ring->pidx, tx_ring->work_cidx) <
        skb_shinfo(skb)->nr_frags + 3) {
        /* Ring is full */
        ipd_info("Not enough space in TX ring %d. pidx=0x%x cidx=0x%x(0x%x) PIDX=0x%x, CIDX=0x%x\n", idev->enet_tx_ring_num, tx_ring->pidx,
                 tx_ring->work_cidx, tx_ring->cidx, REG32(TXQ_x_DESC_PIDX(tx_ring->num)), REG32(TXQ_x_DESC_CIDX(tx_ring->num)));
        idev->inno_stats.tx_ring_stats[tx_ring->num].ring_full++;
        spin_unlock_irqrestore(&idev->lock, lock_flags);
        return NETDEV_TX_BUSY;
    }

    pidx = tx_ring->pidx;          /* Local copy */

    ipd_debug("TX ring %d pidx: 0x%x cidx: 0x%x(0x%x)\n", tx_ring->num, tx_ring->pidx,
               tx_ring->work_cidx, tx_ring->cidx);

    if((idev->inno_netdev.info_header) && (idev->inno_netdev.single_interface)) {
        inno_info_header_t        *ihp;
        ihp = (inno_info_header_t *)skb->data;
        is_tx_queue_valid = ihp->flag & INNO_TX_QUEUE_NUM_VALID;

        if(ihp->flag & INNO_TX_TYPE_PIPELINE_LOOKUP) {
            enet->sysport =  ihp->ssp;
            syshdr_lookup_type = PIPELINE_LOOKUP;
            if (ihp->flag & INNO_TX_TYPE_ACL_BYPASS_LOOKUP) {
                abp_mode = 1;
                ipd_debug("ACL BYPASS lookup enabled");
            } else {
                ipd_debug("Lookup enabled");
            }
        } else if(ihp->flag & INNO_TX_TYPE_PTP_XCONNECT) {
            enet->sysport =  ihp->dsp;
            syshdr_lookup_type = PIPELINE_PTP;
        } else {
            /* Default option is BYPASS */
            enet->sysport =  ihp->dsp;
            syshdr_lookup_type = PIPELINE_BYPASS;
            if (is_tx_queue_valid) {
                queue = ihp->queue;
            }
        }
        ipd_debug("Sysport received in TX is %d and version is 0x%x, syshdr_lookup_type=%d\n", enet->sysport, ihp->ver, syshdr_lookup_type);
    }

    if (enet->sysport >= NUM_SYSPORTS) {
        spin_unlock_irqrestore(&idev->lock, lock_flags);
        ipd_err("Invalid sysport %d\n",enet->sysport);
        idev->inno_stats.tx_ring_stats[tx_ring->num].drops++;
        if (ipd_loglevel >= IPD_LOGLEVEL_DEBUG){
            packet_hex_dump(skb);
        }
        dev_kfree_skb_any(skb);
        return NETDEV_TX_OK;       /* The only real choice */
    }

    if(idev->inno_netdev.single_interface) {
        ndev = idev->sysport_devs[idev->inno_netdev.ndev_sindex];
    } else {
        ndev = idev->sysport_devs[enet->sysport];
    }

    /* Sysport not initialized */
    if(idev->syshdr1_cnt[enet->sysport] == 0) {
        spin_unlock_irqrestore(&idev->lock, lock_flags);
        ipd_err("Invalid sysport %d\n",enet->sysport);
        idev->inno_stats.tx_ring_stats[tx_ring->num].drops++;
        if (ipd_loglevel >= IPD_LOGLEVEL_DEBUG){
            packet_hex_dump(skb);
        }
        dev_kfree_skb_any(skb);
        return NETDEV_TX_OK;       /* The only real choice */
    }

    /* If sysport is CPU port use LOOKUP */
    if(idev->inno_netdev.cpu_port[enet->sysport] == 1) {
        syshdr_lookup_type = PIPELINE_LOOKUP;
    }

    /* Create the system header descriptor */
    memset(&desc, 0, sizeof(inno_tx_desc_t));
    if (syshdr_lookup_type == PIPELINE_LOOKUP) {
        ipd_debug("TX type lookup\n");
        idev->inno_stats.tx_stats.tx_lookup_packets++;
        if (abp_mode) {
            dma_addr = idev->syshdr1_abp_ba + (SYSHDR_SIZE * 2 * enet->sysport);
            hdr_size = idev->chip_hdr_len * idev->syshdr1_cnt[enet->sysport];
        } else {
            dma_addr = idev->syshdr1_ba + (SYSHDR_SIZE * 2 * enet->sysport);
            hdr_size = idev->chip_hdr_len * idev->syshdr1_cnt[enet->sysport];
        }
    } else if (syshdr_lookup_type == PIPELINE_PTP) {

        ipd_debug("TX type ptp xconnect\n");
        idev->inno_stats.tx_stats.tx_ptp_packets++;
        dma_addr = idev->syshdr1_ptp_ba + (SYSHDR_SIZE * enet->sysport);
        hdr_size = idev->chip_hdr_len;
        ipd_debug("PTP hdr size %d\n", hdr_size);
    } else if (syshdr_lookup_type == PIPELINE_BYPASS) {
        ipd_debug("TX type bypass\n");
        idev->inno_stats.tx_stats.tx_bypass_packets++;

        if (is_tx_queue_valid) {
           // in order to set the egress queue, we clone the pre-formatted
           // per-port ldh header, and mutate the queue number in that copy
           // and use the local packet copy instead
           uint8_t *ldh;

           ipd_verbose("TX queue %d is valid\n", queue);

           hdr_size = idev->chip_hdr_len;
           ldh_skb = alloc_skb( hdr_size, GFP_ATOMIC );
           if (!ldh_skb) {
              spin_unlock_irqrestore(&idev->lock, lock_flags);
              dev_kfree_skb_any(skb);
              ipd_err("Can't allocate ldh skb\n");
              return NETDEV_TX_OK; /* The only real choice */
           }

           skb_reset_mac_header( ldh_skb );
           ldh = skb_put(ldh_skb, hdr_size);
           memcpy(ldh, idev->syshdr2 + (SYSHDR_SIZE * enet->sysport), hdr_size);

           if (inno_vf2_queue_set(ldh, queue) < 0) {
               spin_unlock_irqrestore(&idev->lock, lock_flags);
               dev_kfree_skb_any(skb);
               dev_kfree_skb_any(ldh_skb);
               ipd_err("Failed to set egress queue\n");
               return NETDEV_TX_OK; /* The only real choice */
           }

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
           dma_addr = pci_map_single(idev->pdev, ldh, hdr_size, DMA_TO_DEVICE);
#else
           dma_addr = dma_map_single(&(idev->pdev)->dev, ldh, hdr_size,
                 (enum dma_data_direction)DMA_TO_DEVICE);
#endif
        }
        else {
            dma_addr = idev->syshdr2_ba + (SYSHDR_SIZE * enet->sysport);
            hdr_size = idev->chip_hdr_len;

            ipd_verbose("TX queue %d is invalid\n", queue);
        }
    } else {
        spin_unlock_irqrestore(&idev->lock, lock_flags);
        ipd_debug("Wrong lookup type\n");
        dev_kfree_skb_any(skb);
        return NETDEV_TX_OK; /* The only real choice */
    }

    desc.hsn_upper = (uint32_t)(dma_addr >> 32);
    desc.hsn_lower = (uint32_t)(dma_addr & 0x00000000ffffffff);
    desc.length    = hdr_size;
    desc.sop = 1;

    ipd_debug("TX: SOP Address Virtual - %p Physical - 0x%x 0x%x hdr_size - %d sysport - %d\n", idev->syshdr2, (unsigned int)desc.hsn_upper, (unsigned int)desc.hsn_lower, hdr_size, enet->sysport);

    /* Copy the cached copy of the descriptor to the actual ring */
    memcpy(&tx_ring->tx_desc[RING_IDX_MASKED(pidx)], &desc,
           sizeof(inno_tx_desc_t));

    info           = &tx_ring->desc_info[RING_IDX_MASKED(pidx)];
    info->len      = hdr_size;
    info->map_type = !ldh_skb ? DESC_MAP_NONE : DESC_MAP_SINGLE;
    info->skb      = ldh_skb;
    if (info->skb) {
        ipd_debug("Created LDH skb pointer is %p\n", info->skb);
    }

    idev->inno_stats.tx_ring_stats[tx_ring->num].descs++;
    idev->inno_stats.tx_ring_stats[tx_ring->num].bytes+=hdr_size;
    idev->inno_stats.tx_ring_stats[tx_ring->num].packets++;

    RING_IDX_INCR(tx_ring->count, pidx);

    /* Prime with the header info for the first time in the loop */
    len      = skb_headlen(skb);
    ndev->stats.tx_bytes += len;
    if(idev->inno_netdev.info_header) {
        vmaddr = skb->data + NETDEV_INFO_HEADER_SIZE;
        len -= NETDEV_INFO_HEADER_SIZE;
    }else{
        vmaddr   = skb->data;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
        dma_addr = pci_map_single(idev->pdev, vmaddr, len, DMA_TO_DEVICE);
#else
        dma_addr = dma_map_single(&(idev->pdev)->dev, vmaddr, len,
                        (enum dma_data_direction)DMA_TO_DEVICE);
#endif
    page     = 0;
    map_type = DESC_MAP_SINGLE;

    data_len = skb->data_len;
    ndev->stats.tx_packets++;
    /* Map each fragment */
    for (frag_num = 0; ; frag_num++) {
        skb_frag_t            *frag;

        if (frag_num > 100) {
            break;
        }

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
        if (pci_dma_mapping_error(idev->pdev, dma_addr)) {
#else
        if (dma_mapping_error(&idev->pdev->dev, dma_addr)) {
#endif
            ipd_err("net_tx err mapping\n");
            goto err_cleanup;
        }

        /* Set the descriptor in the ring */
        memset(&desc, 0, sizeof(inno_tx_desc_t));
        desc.hsn_upper = (uint32_t)(dma_addr >> 32);
        desc.hsn_lower = (uint32_t)(dma_addr & 0x00000000ffffffff);
        desc.length    = len;
        ipd_debug("TX: desc len = %d, data_len=%d, len=%d, skb_headlen=%d\n",
                  desc.length, data_len, len, skb_headlen(skb));
        ipd_debug("TX: PKT Address Virtual - %p Physical - 0x%x 0x%x size - %d\n", vmaddr, (unsigned int)desc.hsn_upper, (unsigned int)desc.hsn_lower, len);

        /* Copy the cached copy of the descriptor to the actual ring */
        memcpy(&tx_ring->tx_desc[RING_IDX_MASKED(pidx)], &desc,
               sizeof(inno_tx_desc_t));

        info           = &tx_ring->desc_info[RING_IDX_MASKED(pidx)];
        info->dma_addr = dma_addr;
        info->len      = len;
        info->map_type = map_type;
        info->skb      = NULL;

        idev->inno_stats.tx_ring_stats[tx_ring->num].descs++;
        idev->inno_stats.tx_ring_stats[tx_ring->num].bytes+=len;

        RING_IDX_INCR(tx_ring->count, pidx);

        if (frag_num == skb_shinfo(skb)->nr_frags) {
            if (data_len != 0) {
                ipd_err("Num frags and total len mismatch");
            }

            break;
        }

        /* Re-prime for the next time around the loop */
        frag     = &skb_shinfo(skb)->frags[frag_num];
        len      = skb_frag_size(frag);
        data_len -= len;
        page     = skb_frag_page(frag);
        vmaddr   = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
        dma_addr = pci_map_page(idev->pdev, page, frag->page_offset,
                                len, DMA_TO_DEVICE);
#else
        dma_addr = dma_map_page(&(idev->pdev)->dev, page, frag->page_offset,
                                len, DMA_TO_DEVICE);
#endif
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
        dma_addr = pci_map_page(idev->pdev, page, skb_frag_off(frag),
                                len, DMA_TO_DEVICE);
#else
        dma_addr = dma_map_page(&(idev->pdev)->dev, page, skb_frag_off(frag),
                                len, DMA_TO_DEVICE);
#endif
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
        if (pci_dma_mapping_error(idev->pdev, dma_addr)) {
#else
        if (dma_mapping_error(&(idev->pdev)->dev, dma_addr)) {
#endif
            ipd_err("pci dma mapping error\n");
            goto err_cleanup;
        }
        map_type = DESC_MAP_PAGE;
        ndev->stats.tx_bytes += len;
    }

    info->skb      = skb;            /* SKB gets recorded in last one */

    /* Create the checksum descriptor */
    memset(&desc, 0, sizeof(inno_tx_desc_t));
    dma_addr = idev->tx_netdev_cksum_ba;
    desc.hsn_upper = (uint32_t)(dma_addr >> 32);
    desc.hsn_lower = (uint32_t)(dma_addr & 0x00000000ffffffff);
    desc.length    = ETH_FCS_LEN;
    if(idev->inno_netdev.info_header) {
        if((skb->len - NETDEV_INFO_HEADER_SIZE  + ETH_FCS_LEN) < MIN_PACKET_SIZE )
            desc.length    = MIN_PACKET_SIZE - (skb->len - NETDEV_INFO_HEADER_SIZE);
    }else{
        if((skb->len  + ETH_FCS_LEN) < MIN_PACKET_SIZE )
            desc.length    = MIN_PACKET_SIZE - skb->len;
    }
    memset(idev->tx_netdev_cksum, 0, MIN_PACKET_SIZE);
    desc.eop = 1;
    ipd_debug("TX: EOP Address Virtual - %p Physical - 0x%x 0x%x size - %d\n", idev->tx_netdev_cksum, (unsigned int)desc.hsn_upper, (unsigned int)desc.hsn_lower, desc.length);

    idev->inno_stats.tx_ring_stats[tx_ring->num].descs++;
    idev->inno_stats.tx_ring_stats[tx_ring->num].bytes+=desc.length;

    /* Copy the cached copy of the descriptor to the actual ring */
    memcpy(&tx_ring->tx_desc[RING_IDX_MASKED(pidx)], &desc,
           sizeof(inno_tx_desc_t));

    info           = &tx_ring->desc_info[RING_IDX_MASKED(pidx)];
    info->len      = desc.length;
    info->map_type = DESC_MAP_NONE;
    info->skb      = NULL;

    RING_IDX_INCR(tx_ring->count, pidx);
    tx_ring->pidx = pidx;

    /* Write the new PIDX to kickstart the DMA engine */
    REG32(TXQ_x_DESC_PIDX(tx_ring->num)) = tx_ring->pidx;

    spin_unlock_irqrestore(&idev->lock, lock_flags);

    return NETDEV_TX_OK;

err_cleanup:
    pidx = tx_ring->pidx;
    clean_frag_num = 0;
    do {
        info = &tx_ring->desc_info[RING_IDX_MASKED(pidx)];

        if (info->map_type == DESC_MAP_SINGLE) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
            pci_unmap_single(idev->pdev, info->dma_addr, info->len,
                             DMA_TO_DEVICE);
#else
            dma_unmap_single(&(idev->pdev)->dev, info->dma_addr, info->len,
                             DMA_TO_DEVICE);
#endif
        } else if (info->map_type == DESC_MAP_PAGE) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
            pci_unmap_page(idev->pdev, info->dma_addr, info->len,
                           DMA_TO_DEVICE);
#else
            dma_unmap_page(&(idev->pdev)->dev, info->dma_addr, info->len,
                           DMA_TO_DEVICE);
#endif
        }
        // free up any allocated skbs, except the actual
        // input skb
        if (info->skb && info->skb != skb ) {
           dev_kfree_skb_any(info->skb);
        }
        RING_IDX_INCR(tx_ring->count, pidx);
    } while (clean_frag_num++ < frag_num);

    spin_unlock_irqrestore(&idev->lock, lock_flags);

    dev_kfree_skb_any(skb);
    return NETDEV_TX_OK;        
}

/** @brief Net device tx timeout
 *
 *  @return ERRNO
 */
static void
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
inno_enet_tx_timeout(struct net_device *dev)
#else
inno_enet_tx_timeout(struct net_device *dev, unsigned int txqueue)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
    ipd_warn("TX timeout on interface %s\n", dev->name);
#else
    ipd_warn("TX timeout on interface %s queue %d\n", dev->name, txqueue);
#endif
    dev->stats.tx_errors++;
}

/** @brief Get Net device Statistics
 *
 *  @return netdev stats structure
 */
static struct net_device_stats *
inno_enet_get_stats(struct net_device *dev)
{
    inno_enet_adapter_t       *enet;

    enet = (inno_enet_adapter_t *) netdev_priv(dev);
    if (enet == NULL) {
        ipd_err("Netdev not initilaized\n");
        return NULL;
    }
    return &dev->stats;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
/** @brief Get Net device Statistics
 *
 *  @return netdev stats structure
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
static struct rtnl_link_stats64 *
inno_enet_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
#else
void
inno_enet_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
#endif
{
    inno_enet_adapter_t       *enet;

    enet                      = (inno_enet_adapter_t *) netdev_priv(dev);
    if (enet == NULL) {
        ipd_err("Netdev not initilaized\n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
        return NULL;
#else
        return;
#endif
    }
    stats->rx_packets         = dev->stats.rx_packets;
    stats->rx_bytes           = dev->stats.rx_bytes;
    stats->tx_packets	      = dev->stats.tx_packets;
    stats->tx_bytes           = dev->stats.tx_bytes;
    stats->multicast	      = dev->stats.multicast;
    stats->tx_errors	      = dev->stats.tx_errors;
    stats->rx_errors	      = dev->stats.rx_errors;
    stats->rx_dropped	      = dev->stats.rx_dropped;
    stats->rx_length_errors   = dev->stats.rx_length_errors;
    stats->rx_crc_errors      = dev->stats.rx_crc_errors;
    stats->rx_missed_errors   = dev->stats.rx_missed_errors;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
    return stats;
#endif
}
#endif

/** @brief Net device MAC address set
 *
 *  @return ERRNO
 */
static int
inno_enet_set_mac(struct net_device *dev, void *addr)
{
    inno_enet_adapter_t       *enet;
    struct sockaddr *addr_new = addr;
    struct sockaddr addr_old;

    enet = (inno_enet_adapter_t *) netdev_priv(dev);
    if (enet == NULL) {
        ipd_err("Netdev not initilaized\n");
        return -ENOMEM;
    }

    memcpy(&(addr_old.sa_data), dev->dev_addr, ETH_ALEN);

    if (!is_valid_ether_addr(addr_new->sa_data))
        return -EADDRNOTAVAIL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0)
    memcpy(dev->dev_addr, addr_new->sa_data, dev->addr_len);
#else
    dev_addr_mod(dev, 0, addr_new->sa_data, dev->addr_len);
#endif

    ipd_debug("MAC address changed for inteface %s from %pM to %pM\n", dev->name, &addr_old, dev->dev_addr);
    return 0;
}

/** @brief Net device MTU set
 *
 *  @return ERRNO
 */
static int
inno_enet_set_mtu(struct net_device *dev, int new_mtu)
{
    inno_enet_adapter_t  *enet;
    inno_device_t        *idev;
    int mtu_size          = new_mtu;
    enet = (inno_enet_adapter_t *) netdev_priv(dev);
    if (enet == NULL) {
        ipd_err("Netdev not initilaized\n");
        return -ENOMEM;
    }

    idev = enet->idev;
    if(idev->inno_netdev.single_interface) {
        mtu_size+=NETDEV_INFO_HEADER_SIZE;
    }
    dev->mtu = mtu_size;
    ipd_info("MTU changed for inteface %s to %d\n",
             dev->name, mtu_size);
    return 0;
}

/** @brief Net device set rx_mode
 *
 *  @return ERRNO
 */
static void
inno_enet_set_rx_mode(struct net_device *dev)
{
    inno_enet_adapter_t       *enet;
    enet = (inno_enet_adapter_t *) netdev_priv(dev);
    if (enet == NULL) {
        ipd_err("Netdev not initilaized\n");
        return;
    }

    ipd_debug(" Set RX mode called\n");
}

static const struct net_device_ops inno_enet_netdev_ops =
{
    .ndo_open            = inno_enet_open,
    .ndo_stop            = inno_enet_close,
    .ndo_start_xmit      = inno_enet_tx,
    .ndo_tx_timeout      = inno_enet_tx_timeout,
    .ndo_get_stats       = inno_enet_get_stats,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
    .ndo_get_stats64     = inno_enet_get_stats64,
#endif
    .ndo_set_mac_address = inno_enet_set_mac,
    .ndo_change_mtu      = inno_enet_set_mtu,
    .ndo_set_rx_mode     = inno_enet_set_rx_mode,
};

/*
 * End of netdev ops
 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
void send_sflow_packet(inno_device_t *idev, struct sk_buff *skb,
                       inno_gen_netlink_t *genl,
                       uint16_t in_sp, uint16_t out_sp,
                       uint32_t sample_rate, struct net *inet,
                       uint8_t sflow_dir)
{
    struct sk_buff *nl_skb;
    int data_len, meta_len, ret;
    uint16_t in_ifindex = 0, out_ifindex = 0, group_id = 0;
    struct net_device *ndev;
    void *data;

    if(in_sp) {
        if(idev->inno_netdev.single_interface) {
            ndev = idev->sysport_devs[idev->inno_netdev.ndev_sindex];
        } else {
            ndev = idev->sysport_devs[in_sp];
        }
        if(ndev) {
            in_ifindex = ndev->ifindex;
        } else {
            ipd_err("Netdev for sp %d not found\n", in_sp);
        }
    }
    if(out_sp) {
        if(idev->inno_netdev.single_interface) {
            ndev = idev->sysport_devs[idev->inno_netdev.ndev_sindex];
        } else {
            ndev = idev->sysport_devs[out_sp];
        }
        if(ndev) {
            out_ifindex = ndev->ifindex;
        } else {
            ipd_err("Netdev for sp %d not found\n", out_sp);
        }
    }
    meta_len = (in_ifindex ? nla_total_size(sizeof(uint16_t)) : 0) +
        (out_ifindex ? nla_total_size(sizeof(uint16_t)) : 0) +
        nla_total_size(sizeof(uint32_t)) +    /* sample_rate */
        nla_total_size(sizeof(uint32_t)) +    /* orig_size */
        nla_total_size(sizeof(uint32_t)) +    /* group_num */
        nla_total_size(sizeof(uint32_t));     /* seq */

    data_len = skb->len;

    nl_skb = genlmsg_new(meta_len + nla_total_size(data_len), GFP_ATOMIC);
    if (unlikely(!nl_skb)){
        ipd_err("genlmsg_new failed\n");
        return;
    }

    ipd_debug("Sflow meta_len=%d data_len=%d\n", meta_len, nla_total_size(data_len));

    data = genlmsg_put(nl_skb, 0, 0, &(genl->inno_genl_family), 0, 0);
    if (unlikely(!data)){
        ipd_err("genlmsg_put failed\n");
        goto error;
    }

    if (in_ifindex) {
        ret = nla_put_u16(nl_skb, INNO_SFLOW_GENL_ATTR_IIFINDEX, in_ifindex);
        if (unlikely(ret < 0)){
            ipd_err("INNO_SFLOW_GENL_ATTR_IIFINDEX nla_put failed\n");
            goto error;
        }
    }

    if (out_ifindex) {
        ret = nla_put_u16(nl_skb, INNO_SFLOW_GENL_ATTR_OIFINDEX, out_ifindex);
        if (unlikely(ret < 0)){
            ipd_err("INNO_SFLOW_GENL_ATTR_OIFINDEX nla_put failed\n");
            goto error;
        }
    }

    ret = nla_put_u32(nl_skb, INNO_SFLOW_GENL_ATTR_SAMPLE_RATE, sample_rate);
    if (unlikely(ret < 0)){
            ipd_err("INNO_SFLOW_GENL_ATTR_SAMPLE_RATE nla_put failed\n");
        goto error;
    }

    ret = nla_put_u32(nl_skb, INNO_SFLOW_GENL_ATTR_ORIGSIZE, skb->len);
    if (unlikely(ret < 0)){
            ipd_err("INNO_SFLOW_GENL_ATTR_ORIGSIZE nla_put failed\n");
        goto error;
    }

    if (sflow_dir) {
        /* Egress */
        group_id = 2;
    } else {
        group_id = 1;
    }
    ret = nla_put_u32(nl_skb, INNO_SFLOW_GENL_ATTR_SAMPLE_GROUP, group_id);
    if (unlikely(ret < 0)){
            ipd_err("INNO_SFLOW_GENL_ATTR_SAMPLE_GROUP nla_put %d failed\n", group_id);
        goto error;
    }

    ret = nla_put_u32(nl_skb, INNO_SFLOW_GENL_ATTR_GROUP_SEQ, ++(genl->seq));
    if (unlikely(ret < 0)) {
            ipd_err("INNO_SFLOW_GENL_ATTR_GROUP_SEQ nla_put failed\n");
        goto error;
    }

    if (data_len) {
        int nla_len = nla_total_size(data_len);
        struct nlattr *nla;
        nla = (struct nlattr *)skb_put(nl_skb, nla_len);
        nla->nla_type = INNO_SFLOW_GENL_ATTR_DATA;
        nla->nla_len = nla_attr_size(data_len);

        if (skb_copy_bits(skb, 0, nla_data(nla), data_len)) {
            ipd_err("skb_copy_bits failed\n");
            goto error;
        }
    }

    genlmsg_end(nl_skb, data);
    genlmsg_multicast_netns(&genl->inno_genl_family, inet, nl_skb, 0,
                            0, GFP_ATOMIC);
    kfree_skb(skb);
    return;
error:
    kfree_skb(skb);
    nlmsg_free(nl_skb);
}

static void
inno_receive_sflow(inno_device_t *idev, struct sk_buff *skb, struct net *inet)
{
    struct   sk_buff *sflow_skb;
    struct   ethhdr  *ehdr;
    uint32_t *pkt_ptr, offset=0;
    uint8_t sflow_dir;
    uint16_t i_sp, e_sp;
    uint32_t sample_rate, sp;
    inno_gen_netlink_t *genl=NULL;

    genl = idev->inno_netlink.sflow_genl;
    if(genl == NULL) {
        ipd_err("Cannot find sflow gen netlink\n");
        kfree_skb(skb);
        return;
    }

    /* Set skb->data to point to eth header */
    if (idev->inno_netdev.single_interface) {
        skb_pull(skb, NETDEV_INFO_HEADER_SIZE-ETH_HLEN);
    } else {
        skb_push(skb, ETH_HLEN);
    }
    ehdr = (struct ethhdr*)(skb->data);
    if(ehdr->h_proto == htons(ETH_P_8021Q)) {
        offset += (VLAN_ETH_HLEN + 20);
    } else {
        offset += (VLAN_ETH_HLEN - VLAN_HLEN + 20);
    }

    /* GRE header(4) + Base header(4) */
    offset += 8;
    pkt_ptr = (uint32_t*)(skb->data + offset);
    sflow_dir = (htonl(*pkt_ptr) >> 15)&0x1;
    if(sflow_dir) {
        /* Egress */
        i_sp = 0;
        sp = e_sp = htonl(*pkt_ptr)&0x7fff;
        sample_rate = idev->inno_netdev.esflow_sample_rate[sp];
    } else {
        /* Ingress */
        sp = i_sp = htonl(*pkt_ptr)&0x7fff;
        pkt_ptr = (uint32_t*)(skb->data + offset + 4);
        e_sp = htonl(*pkt_ptr)&0x7fff;
        sample_rate = idev->inno_netdev.isflow_sample_rate[sp];
    }


    /* Extension header for sflow(16) */
    offset+=16;
    skb_pull(skb,offset);

    sflow_skb = skb_copy(skb, GFP_ATOMIC);
    kfree_skb(skb);
    if(sflow_skb == NULL) {
        ipd_err("skb_copy failed\n");
        return;
    }

    ipd_info("Sflow dir=%d i_sp=%d e_sp=%d len=%d sample_rate=%d seq=%d\n",
	          sflow_dir, i_sp, e_sp, sflow_skb->len, sample_rate, genl->seq);
    send_sflow_packet(idev, sflow_skb, genl, i_sp, e_sp, sample_rate, inet, sflow_dir);
    return;
}

static inno_gen_netlink_tid_t*
netlink_filter_trap(inno_device_t *idev, uint8_t trapid)
{
    inno_gen_netlink_tid_t *inno_genl_trapid = NULL;
    struct list_head *l = NULL ;

    if(list_empty(&idev->trapid_list) == 1) {
        return NULL;
    }

    list_for_each(l, &idev->trapid_list){
        inno_genl_trapid = list_entry(l, struct inno_gen_netlink_tid_s, list);
        if(inno_genl_trapid->trapid == trapid){
            return inno_genl_trapid;
        }
    }
    return NULL;
}

static int
send_pgenl_packet(inno_device_t *idev, struct sk_buff *skb, struct net *inet, inno_gen_netlink_t *inno_genl, uint16_t ssp, uint16_t dsp)
{
    struct sk_buff *nl_skb;
    int data_len, meta_len, ret;
    uint16_t in_ifindex=0, out_ifindex = 0;
    struct net_device *ndev;
    void *data;

    /* Set skb->data to point to eth header */
    if (idev->inno_netdev.single_interface) {
        skb_pull(skb, NETDEV_INFO_HEADER_SIZE-ETH_HLEN);
    } else {
        skb_push(skb, ETH_HLEN);
    }

    if (ssp) {
        if (idev->inno_netdev.single_interface) {
            ndev = idev->sysport_devs[idev->inno_netdev.ndev_sindex];
        } else {
            ndev = idev->sysport_devs[ssp];
        }
        if (ndev) {
            in_ifindex = ndev->ifindex;
        } else {
            ipd_err("Netdev for sp %d not found\n", ssp);
        }
    }
    if (dsp)
    {
        if (idev->inno_netdev.single_interface) {
            ndev = idev->sysport_devs[idev->inno_netdev.ndev_sindex];
        } else {
            ndev = idev->sysport_devs[dsp];
        }
        if (ndev) {
            out_ifindex = ndev->ifindex;
        } else {
            ipd_err("Netdev for sp %d not found\n", dsp);
        }
    }

    meta_len =
        nla_total_size(sizeof(uint16_t)) +    /* ingress port */
        nla_total_size(sizeof(uint16_t)) +    /* egress port */
        nla_total_size(sizeof(uint32_t));     /* context */

    data_len = skb->len;

    nl_skb = genlmsg_new(meta_len + nla_total_size(data_len), GFP_ATOMIC);
    if (unlikely(!nl_skb)){
        ipd_err("genlmsg_new failed\n");
        return -1;
    }
    ipd_info("Netlink packet meta_len=%d data_len=%d in_ifindex=0x%x(%d) outifindex=0x%x(%d)\n", meta_len, nla_total_size(data_len), in_ifindex, in_ifindex, out_ifindex, out_ifindex);

    data = genlmsg_put(nl_skb, 0, 0, &(inno_genl->inno_genl_family), 0, 0);
    if (unlikely(!data))
        goto error;

    ret = nla_put_u16(nl_skb, INNO_PGENL_ATTR_SOURCE_IFINDEX, in_ifindex);
    if (unlikely(ret < 0))
        goto error;

    ret = nla_put_u16(nl_skb, INNO_PGENL_ATTR_DEST_IFINDEX, out_ifindex);
    if (unlikely(ret < 0))
        goto error;

    ret = nla_put_u32(nl_skb, INNO_PGENL_ATTR_CONTEXT, 0);
    if (unlikely(ret < 0))
        goto error;

    if (data_len) {
        int nla_len = nla_total_size(data_len);
        struct nlattr *nla;
        nla = (struct nlattr *)skb_put(nl_skb, nla_len);
        nla->nla_type = INNO_PGENL_ATTR_PAYLOAD;
        nla->nla_len = nla_attr_size(data_len);

        if (skb_copy_bits(skb, 0, nla_data(nla), data_len))
            goto error;
    }

    genlmsg_end(nl_skb, data);
    genlmsg_multicast_netns(&inno_genl->inno_genl_family, inet, nl_skb, 0,
                            0, GFP_ATOMIC);

    kfree_skb(skb);
    return 0;
error:
    kfree_skb(skb);
    nlmsg_free(nl_skb);
    return -1;
}

static int
inno_receive_pkt(inno_device_t *idev, struct sk_buff *skb, struct net *inet, inno_info_header_t *ih, inno_gen_netlink_t *inno_genl)
{
    int rc = 0;
    if(inno_genl->nl_type == INNO_NL_PGENL) {
        rc = send_pgenl_packet(idev, skb, inet, inno_genl, ih->ssp, ih->dsp);
    } else {
        ipd_err("Unknown netlink filter %d netlink %s\n", inno_genl->nl_type, inno_genl->inno_genl_family.name);
        rc = -1;
    }
    return rc;
}
#endif /*LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)*/

/** @brief NAPI poll routine for RX rings
 *
 *  @param [in] rx_ring - Pointer to inno_ring
 *  @param [in] budget  - Max number of packets to process
 *  @return Remainder in budget
 */
static int
inno_rx_ring_poll(struct napi_struct *napi,
                  inno_ring_t *rx,
                  int         budget)
{
    uint16_t              posted_cidx;
    uint16_t              posted_cidx_latest;
    struct sk_buff        *skb = NULL;
    rxq_0_desc_cidx_t     rxq_cidx;
    struct net_device     *ndev;
    struct pci_dev        *pdev;
    unsigned char         *data;
    uint32_t              sysport;
    vlan_tag_action       vtag;
    struct ethhdr         *ehdr;
    bool                  tagged;
    bool                  pkt_tag=false;
    struct vlan_ethhdr    vethhdr;
    uint8_t               payload_offset = 0;
    uint32_t              chip_header_size = 0;
    inno_info_header_t    ih;
    bool                  dbg_hdr;
    uint32_t              ssp, sflow_sp=0;
    uint8_t               pkt_sflow=0;
    uint32_t              ext_hdrs_size, ext_info_hdrs_size;
    bool                  need_release = 0;
    uint32_t              last_cidx;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
    uint8_t                pkt_trap=0;
    inno_gen_netlink_tid_t *inno_genl_trap;
#endif

    inno_device_t *idev = rx->idev;

    pdev = idev->pdev;

    spin_lock(&idev->napi_lock);


    /*
     * Ideal way to get the CIDX is to read from writeback area as this stays in sync with device writing the data,
     * the descriptor and the cidx into write-back memory in that order.
     * However, for A0, due to Bugz 7525 (last packet issue), read the CIDX from register
     *
     * Note that in A0 case, reading the cidx from register and the descriptor from write-back area might lead
     * to a case where SW is reading stale descriptor content from memory ( reason: device issues a dma write into host memory
     * and then updates the CIDX register; device doesn't wait until the write actually made into host memory
     * Bugz 16448 documents a work-around for this issue
     * SW team(Guru, Tony T, Purna) have decided not to take the workaround (add a retry logic) at this stage as all production
     * systems will have only B0 chips
     */
    if(idev->device_id == INNO_TERALYNX_PCI_DEVICE_ID &&
            idev->rev_id == INNO_TERALYNX_PCI_DEVICE_REV_ID_A0) {
        posted_cidx = REG32(RXQ_x_DESC_CIDX(rx->num));
    } else {
       posted_cidx = *rx->cidx_addr;
    }

   if (!( (posted_cidx <= rx->count - 1) ||
        ((posted_cidx >= 0x8000) && (posted_cidx <= (0x8000 + rx->count - 1))) )) {

       posted_cidx_latest = REG32(RXQ_x_DESC_CIDX(rx->num));
       ipd_debug("Invalid CPU RX ring %u CIDX read from WB: 0x%x Reg CIDX: 0x%x ",
                 rx->num, posted_cidx, posted_cidx_latest);

       /* Do not process CPU RX packets for this ring
        * until the next interrupt. Do not write to
        * * CIDX intr reigster as well
        */
       spin_unlock(&idev->napi_lock);
       return 0;
   }

    while ((budget) && (rx->cidx != posted_cidx)) {
        inno_rx_wb_t      wb;
        uint32_t          end_cidx;
        inno_dma_alloc_t  *dma      = NULL;
        int               pull_len  = 0;
        int               total_len = 0;
        int               copy_len  = 0;
        int               block_len  = 0;
        inno_ldh_t        ldh;
        uint64_t          *wb_ptr;
        uint8_t           ext_info_hdrs[EXT_HDRS_MAX_LEN];
        uint8_t           ext_hdr_type = 0;

        if(idev->napi_init == 0) {
            spin_unlock(&idev->napi_lock);
            return 0;
        }

        memset(&ldh,0,LDH_HEADER_MAX_SIZE);

        wb_ptr = (uint64_t*)&wb;
        /* Find the SOP bit (should be the first one!) */
        do {
            memcpy(&wb, &rx->rx_wb[RING_IDX_MASKED(rx->cidx)],
                   sizeof(inno_rx_wb_t));
            if (wb.ecc) {
                /* Descriptor ecc - discard */
                ipd_err("RX WB ecc indication - index %x\n", rx->cidx);
            } else if (wb.err) {
                /* Descriptor error - discard */
                ipd_err("RX WB error indication - index %x\n", rx->cidx);
            } else if (wb.sop) {
                break;
            }
            ipd_err("RX WB w/o expected SOP - index %x wb-%llx\n", rx->cidx, *wb_ptr);

            RING_IDX_INCR(rx->count, rx->cidx);
            idev->inno_stats.rx_ring_stats[rx->num].descs++;
        } while (rx->cidx != posted_cidx);

        if (!wb.sop) {           /* If we did not find an SOP */
            break;
        }

        /* Find the EOP bit */
        end_cidx = rx->cidx;
        do {
            memcpy(&wb, &rx->rx_wb[RING_IDX_MASKED(end_cidx)],
                   sizeof(inno_rx_wb_t));
            if ((total_len != 0) && (wb.sop)) {
                ipd_err("RX WB got unexpected SOP - index %x\n", end_cidx);
                last_cidx = rx->cidx;
                need_release = 1;
                rx->cidx = end_cidx;
                RING_IDX_INCR(rx->count, rx->cidx);
                idev->inno_stats.rx_ring_stats[rx->num].descs++;
                goto next_sop;                /* Skip to next SOP */
            }

            total_len += wb.length;
            ipd_debug("wb length %d index 0x%x\n", wb.length, rx->cidx);
            if ((wb.ecc) || (wb.err)) {
                /* Descriptor WB - discard */
                ipd_err("RX descriptor error indication - IDX %x", end_cidx);

                last_cidx = rx->cidx;
                need_release = 1;
                rx->cidx = end_cidx;
                RING_IDX_INCR(rx->count, rx->cidx);
                idev->inno_stats.rx_ring_stats[rx->num].descs++;
                goto next_sop;                /* Skip to next SOP */
            }

            if (wb.eop) {
                break;
            }
            RING_IDX_INCR(rx->count, end_cidx);
        } while (end_cidx != posted_cidx);

        if (!wb.eop) {               /* EOP buffer not written yet */
            break;
        }

        if ((total_len < idev->chip_hdr_len) ||
            (total_len > DEFAULT_INNO_CPU_MTU_SIZE)) {

            ipd_debug("Packet length %d less than min %d or exceeded the max limit - %d\n", total_len, idev->chip_hdr_len, DEFAULT_INNO_CPU_MTU_SIZE);
            idev->inno_stats.rx_ring_stats[rx->num].drops++;
            RING_IDX_INCR(rx->count, end_cidx);
            idev->inno_stats.rx_ring_stats[rx->num].descs++;
            last_cidx = rx->cidx;
            need_release = 1;
            rx->cidx = end_cidx;      /* Skip up to here */
            goto next_sop;           /* Go back to SOP scan */
        }

        /* We have a complete packet now */
        dma = &rx->pages[RING_IDX_MASKED(rx->cidx)];
        if(inno_unpack_ldh_header(dma->vmaddr, &ldh) < 0){
            ipd_err("Invalid LDH\n");
            idev->inno_stats.rx_ring_stats[rx->num].drops++;
            RING_IDX_INCR(rx->count, end_cidx);
            idev->inno_stats.rx_ring_stats[rx->num].descs++;
            last_cidx = rx->cidx;
            need_release = 1;
            rx->cidx = end_cidx;      /* Skip up to here */
            goto next_sop;           /* Go back to SOP scan */
        }

        /* calculate chip header size based on dbg hdr presence */
        dbg_hdr = inno_debug_hdr_present(&ldh);
        if (dbg_hdr) {
            chip_header_size = CHIP_HEADER_SIZE;
        } else {
            chip_header_size = (CHIP_HEADER_SIZE - idev->chip_dbg_hdr_len);
        }

        /* check if any extension headers present (ex: ptp shim header) */
        ext_hdrs_size = 0;
        ext_info_hdrs_size = 0;
        if(inno_unpack_ext_hdrs(idev, dma, &ldh, &ext_hdrs_size,
                                ext_info_hdrs, &ext_info_hdrs_size,
                                &ext_hdr_type, &sflow_sp) < 0) {
            ipd_err("Invalid extension header\n");
            idev->inno_stats.rx_ring_stats[rx->num].drops++;
            RING_IDX_INCR(rx->count, end_cidx);
            idev->inno_stats.rx_ring_stats[rx->num].descs++;
            last_cidx = rx->cidx;
            need_release = 1;
            rx->cidx = end_cidx;      /* Skip up to here */
            goto next_sop;           /* Go back to SOP scan */
        }
        ipd_debug("ext_hdrs_size %d, ext_info_hdrs_size %d, ext_hdr_type %d\n",
                ext_hdrs_size, ext_info_hdrs_size, ext_hdr_type);

		if((ext_hdr_type == INNO_HEADER_EXTENSION_TYPE_SFLOW) && (idev->inno_netlink.sflow)) {
			pkt_sflow = 1;
		}

        chip_header_size += ext_hdrs_size;

        total_len     -= chip_header_size;
        total_len     -= ETH_FCS_LEN;

        ssp           =  inno_get_ssp(&ldh);
        /* Egress sflow case ssp in LDH will be 0 *
         * Get ssp from SFLOW header              */
        if ((pkt_sflow == 1) && (ssp == 0)) {
            ssp = sflow_sp;
        }

        if (idev->inno_netdev.single_interface)
            sysport        = idev->inno_netdev.ndev_sindex;
        else
            sysport        = ssp;

        if ((ssp > NUM_SYSPORTS) ||
            (idev->sysport_devs[sysport] == NULL)) {
            if(ssp > NUM_SYSPORTS) {
                ipd_err("Invalid sysport in packet: %x(%x) total_len: %d cidx: 0x%x-0x%x-0x%x\n",
                        ssp, sysport, total_len, end_cidx, rx->cidx, posted_cidx);
            } else {
                ipd_info("Invalid sysport in packet(netdev not found): %x(%x) total_len: %d cidx: 0x%x-0x%x-0x%x\n",
                        ssp, sysport, total_len, end_cidx, rx->cidx, posted_cidx);
            }
            idev->inno_stats.rx_ring_stats[rx->num].drops++;
            RING_IDX_INCR(rx->count, end_cidx);
            idev->inno_stats.rx_ring_stats[rx->num].descs++;
            last_cidx = rx->cidx;
            need_release = 1;
            rx->cidx = end_cidx;      /* Skip up to here */
            goto next_sop;           /* Go back to SOP scan */
        }

        ndev = idev->sysport_devs[sysport];
        vtag = idev->inno_netdev.vlan_action[sysport];

        if (!netif_running(ndev)) {
            ipd_verbose("Netdev %s down\n",ndev->name);
            idev->inno_stats.rx_ring_stats[rx->num].drops++;
            RING_IDX_INCR(rx->count, end_cidx);
            idev->inno_stats.rx_ring_stats[rx->num].descs++;
            last_cidx = rx->cidx;
            need_release = 1;
            rx->cidx = end_cidx;      /* Skip up to here */
            budget=1;
            goto next_sop;           /* Go back to SOP scan */
        }

        ehdr = (struct ethhdr*)(dma->vmaddr + chip_header_size);
        ipd_debug("RX packet proto is 0x%x\n", htons(ehdr->h_proto));

        if (ehdr->h_proto == htons(ETH_P_8021Q)) {
            tagged = true;
        } else {
            tagged = false;
        }

        /* First block but the whole thing won't fit in the SKB
         * header.  So we have to pull the ETH header in and then
         * make the rest a frag */

        pull_len = INNO_RX_SKB_HDR_LEN;
        if (pull_len > total_len) {
            pull_len = total_len;
        }
        if (idev->inno_netdev.info_header) {
            pull_len += NETDEV_INFO_HEADER_SIZE;
        }

        pull_len += ext_info_hdrs_size;

        memset(&ih, 0, sizeof(ih));
        inno_populate_inno_hdr(&ldh, &ih, dma, ext_hdr_type, ext_info_hdrs_size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
        if ((inno_genl_trap = netlink_filter_trap(idev, ih.trap)) != NULL){
            pkt_trap = 1;
        }
#endif
        memcpy(&vethhdr, dma->vmaddr+chip_header_size, VLAN_ETH_HLEN);

        /* Decide the VLAN tag action to take */
        switch(vtag) {
            case VLAN_TAG_ORIGINAL:
                /* No change */
                if (tagged) {
                    pkt_tag = true;
                } else {
                    pkt_tag = false;
                }
                ipd_debug("VLAN tag action - none\n");
                break;
            case VLAN_TAG_STRIP:
                /* Strip vlan tag */
                if (tagged) {
                    pull_len -= VLAN_HLEN;
                    total_len -= VLAN_HLEN;
                }
                pkt_tag = false;
                ipd_debug("VLAN tag action - strip\n");
                break;
            case VLAN_TAG_KEEP:
                /* Add vlan tag */
                if (!tagged) {
                    pull_len += VLAN_HLEN;
                    total_len += VLAN_HLEN;
                    vethhdr.h_vlan_proto = htons(ETH_P_8021Q);
                    vethhdr.h_vlan_TCI = htons(ih.l2vni & 0xfff);
                }
                pkt_tag = true;
                ipd_debug("VLAN tag action - keep\n");
                break;
            default:
                /* No change */
                if (tagged) {
                    pkt_tag = true;
                } else {
                    pkt_tag = false;
                }
                ipd_err("No vlan tag action");
                break;
        }

        skb = alloc_skb(pull_len+INNO_RX_SKB_HDR_LEN, GFP_ATOMIC);
        if (!skb) {
            ndev->stats.rx_dropped++;
            ipd_err("Unable allocate Rx skb memory\n");
            break;
        }

        skb_reserve(skb, INNO_RX_SKB_HDR_LEN);
        data = skb_put(skb, pull_len);
        ipd_debug("pull_len = %d, skb len = %d, data = %p, hdr = %p, tail = %p\n",
                   pull_len, skb->len, skb->data, skb->head, skb_tail_pointer(skb));

        if(idev->inno_netdev.info_header) {
            memcpy(data, &ih, NETDEV_INFO_HEADER_SIZE);
            pull_len -= NETDEV_INFO_HEADER_SIZE;
            data += NETDEV_INFO_HEADER_SIZE;
            total_len += NETDEV_INFO_HEADER_SIZE;

            /* copy the ext_info_hdrs */
            if(ext_info_hdrs_size != 0) {
                memcpy(data, ext_info_hdrs, ext_info_hdrs_size);
                pull_len -= ext_info_hdrs_size;
                data += ext_info_hdrs_size;
                total_len += ext_info_hdrs_size;
            }
        }

        ipd_debug("RX packet: sysport %d cidx: 0x%x-0x%x-0x%x\n", ssp, end_cidx, rx->cidx, posted_cidx);

        switch(vtag) {
            case VLAN_TAG_ORIGINAL:
                /* Copy the ethernet header, with VLAN tag if needed */
                if(pkt_tag) {
                    memcpy(data, &vethhdr, VLAN_ETH_HLEN);
                    data += VLAN_ETH_HLEN;
                    copy_len = VLAN_ETH_HLEN;
                    payload_offset = VLAN_ETH_HLEN;
                } else {
                    memcpy(data, &vethhdr, ETH_HLEN);
                    data += ETH_HLEN;
                    copy_len = ETH_HLEN;
                    payload_offset = ETH_HLEN;
                }
                break;
            case VLAN_TAG_STRIP:
                /* Copy the ethernet header, with VLAN tag if needed */
                memcpy(data, &vethhdr, (ETH_HLEN-2));
                data += (ETH_HLEN-2);
                copy_len = (ETH_HLEN-2);
                /* Copy rest of the packet */
                if(tagged) {
                    payload_offset = (VLAN_ETH_HLEN - 2);
                }else {
                    payload_offset = (ETH_HLEN - 2);
                }
                break;
            case VLAN_TAG_KEEP:
                /* Copy the ethernet header, with VLAN tag if needed */
                memcpy(data, &vethhdr, (VLAN_ETH_HLEN-2));
                data += (VLAN_ETH_HLEN-2);
                copy_len = (VLAN_ETH_HLEN-2);
                /* Copy rest of the packet */
                if(tagged) {
                    payload_offset = (VLAN_ETH_HLEN - 2);
                }else {
                    payload_offset = (ETH_HLEN - 2);
                }
                break;
            default:
                /* Copy the ethernet header, with VLAN tag if needed */
                if(pkt_tag) {
                    memcpy(data, &vethhdr, VLAN_ETH_HLEN);
                    data += VLAN_ETH_HLEN;
                    copy_len = VLAN_ETH_HLEN;
                    payload_offset = VLAN_ETH_HLEN;
                } else {
                    memcpy(data, &vethhdr, ETH_HLEN);
                    data += ETH_HLEN;
                    copy_len = ETH_HLEN;
                    payload_offset = ETH_HLEN;
                }
                break;
        }

        block_len = chip_header_size+payload_offset+(pull_len-copy_len);
        ipd_debug("payload_offset = %d, pull_len = %d, copy_len = %d\n", payload_offset,pull_len, copy_len);
        memcpy(data, dma->vmaddr+chip_header_size+payload_offset, pull_len-copy_len);

        /* Update the wb */
        memcpy(&wb, &rx->rx_wb[RING_IDX_MASKED(rx->cidx)],
               sizeof(inno_rx_wb_t));

        skb->dev = ndev;
        /* eth_type_trans will reduce the skb->len */
        skb->protocol = eth_type_trans(skb, ndev);
        ipd_debug("pull_len = %d, skb len = %d, data = %p, hdr = %p, tail = %p\n",
                   pull_len, skb->len, skb->data, skb->head, skb_tail_pointer(skb));

        ipd_debug("block_len=%d, wb.length=%d \n", block_len, wb.length);

        /* First block */
        if (block_len >= (wb.length-ETH_FCS_LEN)) {
            /* Nothing left - release the page */
            dma_unmap_page(&pdev->dev, dma->dma_addr, PAGE_SIZE,
                           DMA_FROM_DEVICE);
            __free_page(dma->page);
        } else {
            int fcs_len = 0;
            if(wb.eop) fcs_len = ETH_FCS_LEN;
            /* Add the page to the skb as a new frag */
            skb_add_rx_frag(skb, skb_shinfo(skb)->nr_frags, dma->page,
                            block_len,
                            wb.length - block_len - fcs_len, PAGE_SIZE);
            dma_unmap_page(&pdev->dev, dma->dma_addr, PAGE_SIZE,
                           DMA_FROM_DEVICE);
        }

        memset(dma, 0, sizeof(inno_dma_alloc_t));

        while (1) {
            int fcs_len = 0;
            ipd_debug("RX: desc len %d for cidx 0x%x\n",wb.length, rx->cidx);
            idev->inno_stats.rx_ring_stats[rx->num].descs++;
            ipd_debug("Received packet with trapid %d\n", ih.trap);
            if (wb.eop) {
                int ret;
                ipd_debug("RX: skb len %d data_len %d\n", skb->len, skb->data_len);
                if((pkt_sflow) && (idev->inno_netlink.sflow == 1)) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
                    inno_receive_sflow(idev, skb, dev_net(ndev));
                } else if(pkt_trap == 1) {
                    pkt_trap = 0;
                    ret = inno_receive_pkt(idev, skb, dev_net(ndev), &ih, inno_genl_trap->inno_genl);
                    if(ret == 0) {
                        inno_genl_trap->stats.rx_packets++;
                        inno_genl_trap->stats.rx_bytes += total_len;
                        inno_genl_trap->inno_genl->stats.rx_packets++;
                        inno_genl_trap->inno_genl->stats.rx_bytes += total_len;
                    }else{
                        inno_genl_trap->stats.rx_drops++;
                        inno_genl_trap->inno_genl->stats.rx_drops++;
                    }
#endif
                }
                else {
                    ret = netif_receive_skb(skb); /* Send the pkt out the xface */
                    if (ret != NET_RX_SUCCESS){
                        ndev->stats.rx_dropped++;
                        ipd_debug("netif_receive_skb failed wth error %d\n",ret);
                    }
                    ndev->stats.rx_packets++;
                    ndev->stats.rx_bytes += total_len;
                }
                idev->inno_stats.rx_ring_stats[rx->num].bytes+=total_len;
                idev->inno_stats.rx_ring_stats[rx->num].packets++;
                pkt_sflow = 0;
                skb = NULL;
                break;
            }

            RING_IDX_INCR(rx->count, rx->cidx);

            memcpy(&wb, &rx->rx_wb[RING_IDX_MASKED(rx->cidx)],
                   sizeof(inno_rx_wb_t));
            dma = &rx->pages[RING_IDX_MASKED(rx->cidx)];

            if(wb.eop) {
                fcs_len = ETH_FCS_LEN;
            } else {
                /* inspect if the next descriptor has EOP
                 * If that is the case and the length of the EOP
                 * is smaller than FCS, remove FCS related bytes
                 * in the current descriptor as well
                 */
                inno_rx_wb_t wbn;
                uint32_t     next_cidx = rx->cidx;
                RING_IDX_INCR(rx->count, next_cidx);
                memcpy(&wbn, &rx->rx_wb[RING_IDX_MASKED(next_cidx)],
                       sizeof(inno_rx_wb_t));
                if (wbn.eop && (wbn.length < ETH_FCS_LEN)) {
                    fcs_len = ETH_FCS_LEN - wbn.length;
                }
            }

            if (wb.length > fcs_len) {
                /* Add the page to the skb as a new frag */
                skb_add_rx_frag(skb, skb_shinfo(skb)->nr_frags, dma->page,
                                0, wb.length - fcs_len, PAGE_SIZE);
            }
            dma_unmap_page(&pdev->dev, dma->dma_addr, PAGE_SIZE,
                           DMA_FROM_DEVICE);

            memset(dma, 0, sizeof(inno_dma_alloc_t));
        }

        RING_IDX_INCR(rx->count, rx->cidx);

    next_sop:
        budget--;
        if (1 == need_release)
        {
            if (rx->pages[RING_IDX_MASKED(last_cidx)].page != NULL)
            {
                /* Temporarily commented out log, needs investigation on why control reaches here */
                /* ipd_info("last_cidx %d page: %p\n", last_cidx, rx->pages[RING_IDX_MASKED(last_cidx)].page); */
                dma_unmap_page(&pdev->dev, rx->pages[RING_IDX_MASKED(last_cidx)].dma_addr, PAGE_SIZE,
                               DMA_FROM_DEVICE);
                __free_page(rx->pages[RING_IDX_MASKED(last_cidx)].page);
            }
            need_release = 0;
        }
        /* Update the posted_cidx */
        posted_cidx = REG32(RXQ_x_DESC_CIDX(rx->num));
    } /* while current cidx != posted cidx */

    rxq_cidx.flds.num_f = rx->cidx;
    REG32(RXQ_CIDX_INTR_x(rx->num)) = rxq_cidx.data;

    spin_unlock(&idev->napi_lock);

    return budget;
}


/** @brief NAPI poll routine for TX rings
 *
 *  @param [in] tx - Pointer to inno_ring
 *  @return None
 */
static void
inno_tx_ring_poll(inno_ring_t *tx)
{
    inno_device_t         *idev = tx->idev;
    uint16_t              cur_cidx;
    inno_tx_wb_t          wb;

    spin_lock(&idev->napi_lock);

    if(idev->napi_init == 0) {
        spin_unlock(&idev->napi_lock);
        return;
    }

    if(idev->device_id == INNO_TERALYNX_PCI_DEVICE_ID &&
            idev->rev_id == INNO_TERALYNX_PCI_DEVICE_REV_ID_A0) {
        tx->cidx = REG32(TXQ_x_DESC_CIDX(tx->num));
    } else {
        tx->cidx = *tx->cidx_addr;
    }
    cur_cidx = tx->work_cidx;

    /* For each completed descriptor */
    while (cur_cidx != tx->cidx) {
        inno_ring_desc_info_t *info;

        info = &tx->desc_info[RING_IDX_MASKED(cur_cidx)];

        /* Copy the WB data out to minimize cache conflicts */
        memcpy(&wb, &tx->tx_wb[RING_IDX_MASKED(cur_cidx)],
               sizeof(inno_tx_wb_t));
        memset(&tx->tx_wb[RING_IDX_MASKED(cur_cidx)], 0,
               sizeof(inno_tx_wb_t));

        if (wb.dd == 0) {
            //ipd_debug("wb.dd is 0\n");
            /* Supposed to be done but still pending !?!! */
            //break;
        }

        if (wb.crc) {
            /* CRC error - increment counters */
            idev->inno_stats.wb_stats.wb_crc++;
            ipd_err("Writeback area CRC count: %llu\tcur_cidx: %d",
                    idev->inno_stats.wb_stats.wb_crc, cur_cidx);
        }

        if (wb.err) {
            /* Generic error - increment counters */
            idev->inno_stats.wb_stats.wb_err++;
            ipd_err("Writeback area ERR count: %llu\tcur_cidx: %d",
                    idev->inno_stats.wb_stats.wb_err, cur_cidx);
        }

        if (info->map_type == DESC_MAP_SINGLE) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
            pci_unmap_single(idev->pdev, info->dma_addr, info->len,
                             DMA_TO_DEVICE);
#else
            dma_unmap_single(&(idev->pdev)->dev, info->dma_addr, info->len,
                             DMA_TO_DEVICE);
#endif
        } else if (info->map_type == DESC_MAP_PAGE) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
            pci_unmap_page(idev->pdev, info->dma_addr, info->len,
                           DMA_TO_DEVICE);
#else
            dma_unmap_page(&(idev->pdev)->dev, info->dma_addr, info->len,
                             DMA_TO_DEVICE);
#endif
        }
        if (info->skb != NULL) {
            ipd_debug("Free skb pointer is %p\n", info->skb);
            dev_kfree_skb_any(info->skb);
        }

        /* Increment the cidx pointer */
        RING_IDX_INCR(tx->count, cur_cidx);
    }

    tx->work_cidx = cur_cidx;

    spin_unlock(&idev->napi_lock);
}


/** @brief NAPI poll routine
 *
 */
static int
inno_ring_poll(struct napi_struct *napi,
               int                budget)
{
    int         ring_num;
    int         start_budget, last_budget, work_done;
    int         i;
    int         pcie_read_done = 0;

    inno_device_t *idev =
        container_of(napi, inno_device_t, napi);

    work_done = 0;

    last_budget = start_budget = budget;
    ring_num    = idev->last_ring_num;
    while (work_done < budget) {
        inno_ring_t *rx = &idev->rx_ring[ring_num];

        if ((rx->flags & INNO_RING_INIT) &&
            (rx->flags & INNO_RING_NETDEV)) {
            int next_budget;
            next_budget = inno_rx_ring_poll(napi, rx, budget);
            work_done  += budget - next_budget;
            budget      = next_budget;
        }

        /* Increment the last ring */
        if (++ring_num >= NUM_RX_RINGS) {
            ring_num = 0;
        }

        /* See if we just finished checking them all */
        if (ring_num == idev->last_ring_num) {
            if (last_budget == budget) {
                /* Once around w/o any RX packets found */

                if (pcie_read_done == 1) {
                    break;
                }

                /* There is a race condition between the S/W write to
                   clear the interrupt and the H/W writeback of the ring
                   data and PIDX.  We fix this by doing a read on the
                   PCIe bus that forces a round trip and guarantees
                   that all of the preceeding traffic in both
                   directions is complete.  We do this after
                   processing everything once in order to avoid
                   delaying those packets that have already posted. */

                (void) REG32(SYNC_MODE);    /* Any reg will do */
                pcie_read_done = 1;         /* Only do this once */
            }

            last_budget = budget;       /* Next loop */
        }
    }

    idev->last_ring_num = ring_num;

    /* Now process the TX rings once */
    while (1) {
        for (ring_num = 0; ring_num < NUM_TX_RINGS; ring_num++) {
            inno_ring_t *tx = &idev->tx_ring[ring_num];
            if ((tx->flags & INNO_RING_INIT) &&
                (tx->flags & INNO_RING_NETDEV)) {
                inno_tx_ring_poll(tx);
            }
        }

        if (pcie_read_done) {
            /* Already forced the flush */
            break;
        }

        /* See the above note on the race condition */
        (void) REG32(SYNC_MODE);    /* Any reg will do */
        pcie_read_done = 1;         /* Only do this once */
    }

    /* Now allocate new pages for the next packets */
    for (ring_num = 0; ring_num < NUM_RX_RINGS; ring_num++) {
        inno_ring_t *rx = &idev->rx_ring[ring_num];
        if ((rx->flags & INNO_RING_INIT) &&
            (rx->flags & INNO_RING_NETDEV)) {
            while (rx->pidx != RING_IDX_COMPL(rx->cidx)) {
                rxq_0_desc_pidx_t rxq_pidx;
                inno_dma_alloc_t  *dma = &rx->pages[RING_IDX_MASKED(rx->pidx)];
                dma->page = alloc_page(__GFP_HIGHMEM);
                if (dma->page == NULL) {
                    /* We might run out or we will get more on the next rupt */
                    break;
                }
                dma->dma_addr = dma_map_page(&idev->pdev->dev, dma->page, 0,
                                             PAGE_SIZE, DMA_FROM_DEVICE);
                if (dma_mapping_error(&idev->pdev->dev, dma->dma_addr)) {
                    ipd_err("%s dma_map_page err ring_num:%d offset:%u len:%u \n", __func__,
                            ring_num, 0, (unsigned)PAGE_SIZE);
                    return -1;
                }

                dma->vmaddr = page_address(dma->page);
                smp_mb();
                rx->rx_desc[RING_IDX_MASKED(rx->pidx)].hsn_upper =
                    (uint32_t)(dma->dma_addr >> 32);
                rx->rx_desc[RING_IDX_MASKED(rx->pidx)].hsn_lower =
                    (uint32_t)(dma->dma_addr & 0x00000000ffffffff);
                RING_IDX_INCR(rx->count, rx->pidx);
                rxq_pidx.flds.num_f = rx->pidx;
                REG32(RXQ_x_DESC_PIDX(ring_num)) = rxq_pidx.data;
            }
        }
    }

    if (work_done < start_budget) {

        napi_complete(napi);

        /* Re-enable interrupts */
        for (i = 0; i < rupt_mask_words; i++) {
            if (idev->napi_mask[i] != 0) {
                REG32(idev->inno_intr_regs.intr_inms + i * 4) = idev->napi_mask[i];
            }
        }
    }

    return work_done;
}


/** @brief Unregister the NAPI netdev
 *
 *  @param [in] idev    - innovium device
 *  @return ERRNO
 */
int
inno_napi_deinit(inno_device_t  *idev)
{
    inno_enet_adapter_t *enet = (inno_enet_adapter_t *) netdev_priv(idev->ndev);

    ipd_trace("inno napi deinit\n");

    if (!idev->ndev) {
        return -ENOMEM;
    }

    if (!enet->idev) {
        return -ENOMEM;
    }

    idev->napi_init = 0;
    napi_disable(&idev->napi);
    netif_napi_del(&idev->napi);

    enet->idev = NULL;
    enet->sysport = 0;
    free_netdev(idev->ndev);

    return 0;
}

/** @brief Initialize the NAPI netdev
 *
 *  @param [in] idev    - innovium device
 *  @return ERRNO
 */
int
inno_napi_init(inno_device_t  *idev)
{
    char name[IFNAMSIZ];
    inno_enet_adapter_t *enet;

    ipd_trace("inno napi init\n");

    sprintf(name, "inno%d", idev->instance);

    spin_lock_init(&idev->napi_lock);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
    idev->ndev = alloc_netdev(sizeof(inno_enet_adapter_t), name, ether_setup);
#else
    idev->ndev = alloc_netdev(sizeof(inno_enet_adapter_t), name,
                              NET_NAME_PREDICTABLE, ether_setup);
#endif

    if (!idev->ndev) {
        return -ENOMEM;
    }

    SET_NETDEV_DEV(idev->ndev, &idev->pdev->dev);
    idev->ndev->netdev_ops = &inno_enet_netdev_ops;

    enet = (inno_enet_adapter_t *) netdev_priv(idev->ndev);
    enet->idev = idev;
    enet->sysport = 0;

    /* Set up the NAPI polling */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0)
    netif_napi_add(idev->ndev, &idev->napi, inno_ring_poll, 8);
#else
    netif_napi_add_weight(idev->ndev, &idev->napi, inno_ring_poll, 8);
#endif

    napi_enable(&idev->napi);

    memset(&(idev->inno_netdev),0,sizeof(inno_netdev_t));

    idev->napi_init = 1;

    if(idev->device_id == INNO_TERALYNX_PCI_DEVICE_ID) {
        inno_unpack_ldh_header = inno_unpack_ldh_header_v0;
        inno_debug_hdr_present = inno_debug_hdr_present_v0;
        inno_unpack_ext_hdrs   = inno_unpack_ext_hdrs_v0;
        inno_populate_inno_hdr = inno_populate_inno_hdr_v0;
        inno_get_ssp           = inno_get_ssp_v0;
        inno_vf2_queue_set     = inno_vf2_queue_set_v0;
     } else if(idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
        inno_unpack_ldh_header = inno_unpack_ldh_header_v2;
        inno_debug_hdr_present = inno_debug_hdr_present_v2;
        inno_unpack_ext_hdrs   = inno_unpack_ext_hdrs_v2;
        inno_populate_inno_hdr = inno_populate_inno_hdr_v2;
        inno_get_ssp           = inno_get_ssp_v2;
        inno_vf2_queue_set     = inno_vf2_queue_set_v2;
    } else {
        ipd_err("Unknown innovium device\n");
        return -ENODEV;
    }

    return 0;
}


/** @brief Create netdev for a port
 *
 *  @param [in] idev  - innovium device
 *  @param [in] ioctl - ioctl area from user
 *  @return ERRNO
 */
int
inno_netdev_create(inno_device_t  *idev,
                   inno_ioctl_netdev_t *ioctl)
{
    char name[IFNAMSIZ];
    int  rc;
    inno_enet_adapter_t *enet;

    if (ioctl->sysport >= NUM_SYSPORTS) {
        ipd_err("sysport number invalid - %d\n", ioctl->sysport);
        return -EINVAL;
    }

    if (ioctl->flag & INNO_NETDEV_SINGLE_INTERFACE) {
        if (idev->inno_netdev.single_interface) {
            ipd_err("single interface already created\n");
            return -EINVAL;
        }
        idev->inno_netdev.single_interface = 1;
        /* Use sysport 1 as the single interface */
        idev->inno_netdev.ndev_sindex = 1;
        ioctl->sysport = idev->inno_netdev.ndev_sindex;
    }

    if (idev->sysport_devs[ioctl->sysport] != NULL) {
        ipd_err("sysport dev already allocated - %d\n", ioctl->sysport);
        return -EINVAL;
    }

    if(strlen(ioctl->intf_name) != 0) {
#pragma GCC diagnostic push
#if __GNUC__ >= 8
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
        strncpy(name, ioctl->intf_name, IFNAMSIZ);
#pragma GCC diagnostic pop
    }else{
        if (idev->inno_netdev.single_interface) {
            sprintf(name, "inno%d", idev->instance);
        } else {
            sprintf(name, "inno_%1.1d_%4.4d", idev->instance, ioctl->sysport);
        }
    }

    if (ioctl->flag & INNO_NETDEV_CPU_PORT) {
        idev->inno_netdev.cpu_port[ioctl->sysport] = 1;
        ipd_info("Create %s port as CPU port\n", name);
    } else {
        idev->inno_netdev.cpu_port[ioctl->sysport] = 0;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
    idev->sysport_devs[ioctl->sysport] =
        alloc_netdev(sizeof(inno_enet_adapter_t), name, ether_setup);
#else
    idev->sysport_devs[ioctl->sysport] =
        alloc_netdev(sizeof(inno_enet_adapter_t), name,
                     NET_NAME_PREDICTABLE, ether_setup);
#endif

    if (idev->sysport_devs[ioctl->sysport] == NULL) {
        return -ENOMEM;
    }

    SET_NETDEV_DEV(idev->sysport_devs[ioctl->sysport],
                       &idev->pdev->dev);
    idev->sysport_devs[ioctl->sysport]->netdev_ops =
        &inno_enet_netdev_ops;

    if(is_valid_ether_addr(ioctl->ethaddr.sa_data)){
        eth_mac_addr(idev->sysport_devs[ioctl->sysport], (void*)(&(ioctl->ethaddr)));
    }else{
        /* Set random mac */
        eth_hw_addr_random(idev->sysport_devs[ioctl->sysport]);
    }

    enet = (inno_enet_adapter_t *) netdev_priv(idev->sysport_devs[ioctl->sysport]);
    enet->idev = idev;
    enet->sysport = ioctl->sysport;

    idev->sysport_devs[ioctl->sysport]->watchdog_timeo = (HZ >> 1);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
    idev->sysport_devs[ioctl->sysport]->min_mtu = ETH_MIN_MTU;
    idev->sysport_devs[ioctl->sysport]->max_mtu = ETH_MAX_MTU;
#endif

    rc = register_netdev(idev->sysport_devs[ioctl->sysport]);
    if(rc < 0) {
        ipd_err("Netdev %s for sysport %d failed with rc %d", name, ioctl->sysport, rc);
        free_netdev(idev->sysport_devs[ioctl->sysport]);
        idev->sysport_devs[ioctl->sysport] = NULL;
        return rc;
    }else {
        ipd_debug("Registered netdev %s for sysport %d %p with priv %p\n",
            name, ioctl->sysport,
            idev->sysport_devs[ioctl->sysport], enet);
    }
    if(idev->inno_netdev.single_interface == 0)
        netif_carrier_off(idev->sysport_devs[ioctl->sysport]);
    return 0;
}

/** @brief Delete the netdev for a port
 *
 *  @param [in] idev  - innovium device
 *  @param [in] ioctl - ioctl area from user
 *  @return ERRNO
 */
int
inno_netdev_delete(inno_device_t  *idev,
                   uint16_t       sysport)
{

    if (idev->inno_netdev.single_interface) {
        sysport = idev->inno_netdev.ndev_sindex;
        idev->inno_netdev.single_interface = 0;
        idev->inno_netdev.ndev_sindex = 0;
    }

    if (idev->sysport_devs[sysport] == NULL) {
        return 0;
    }

    ipd_debug("%s: Delete interface %s for sysport %d\n", __FUNCTION__, idev->sysport_devs[sysport]->name, sysport);

    netif_carrier_off(idev->sysport_devs[sysport]);
    netif_tx_disable(idev->sysport_devs[sysport]);
    netif_device_detach(idev->sysport_devs[sysport]);
    unregister_netdev(idev->sysport_devs[sysport]);
    *((inno_device_t **)netdev_priv(idev->sysport_devs[sysport]))
        = NULL;
    free_netdev(idev->sysport_devs[sysport]);
    idev->sysport_devs[sysport] = NULL;

    return 0;
}

/** @brief Set info for a netdev for a port
 *
 *  @param [in] idev  - innovium device
 *  @param [in] ninfo - netdev info from user
 *  @return ERRNO
 */
int
inno_netdev_set(inno_device_t  *idev,
                inno_ioctl_netdev_info_t ninfo)
{
    int rc=0;
    ipd_verbose("%s: set netdev for sysport %x\n", __FUNCTION__, ninfo.sysport);

    switch(ninfo.id) {
        case INNO_ND_INFOHDR:
            idev->inno_netdev.info_header = 1;
            break;
        case INNO_ND_SYSHDR:
            /* Copy the syshdr */
            memcpy(idev->syshdr1 + (SYSHDR_SIZE * 2 * ninfo.sysport),
                    ninfo.u.ldh.syshdr1, SYSHDR_SIZE*2);

            memcpy(idev->syshdr1_abp + (SYSHDR_SIZE * 2 * ninfo.sysport),
                    ninfo.u.ldh.syshdr1_abp, SYSHDR_SIZE*2);

            memcpy(idev->syshdr1_ptp + (SYSHDR_SIZE * ninfo.sysport),
                    ninfo.u.ldh.syshdr1_ptp, SYSHDR_SIZE);

            memcpy(idev->syshdr2 + (SYSHDR_SIZE * ninfo.sysport),
                    ninfo.u.ldh.syshdr2, SYSHDR_SIZE);

            idev->syshdr1_cnt[ninfo.sysport] = ninfo.u.ldh.syshdr1_cnt;
            break;
        case INNO_ND_VLANTAG:
            idev->inno_netdev.vlan_action[ninfo.sysport] = ninfo.u.vtag;
            break;
        case INNO_ND_IFSTATE:
            if((idev->sysport_devs[ninfo.sysport]) && (idev->inno_netdev.single_interface == 0)){
                switch(ninfo.u.ifstate) {
                    case INNO_INTF_UP:
                        rtnl_lock();
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
                        dev_open(idev->sysport_devs[ninfo.sysport]);
#else
                        dev_open(idev->sysport_devs[ninfo.sysport], NULL);
#endif
                        rtnl_unlock();
                        ipd_debug("%s interface up\n", idev->sysport_devs[ninfo.sysport]->name);
                        break;
                    case INNO_INTF_DOWN:
                        rtnl_lock();
                        dev_close(idev->sysport_devs[ninfo.sysport]);
                        rtnl_unlock();
                        ipd_debug("%s interface down\n", idev->sysport_devs[ninfo.sysport]->name);
                        break;
                    default:
                        ipd_err("Unknown state %d for interface %s\n", ninfo.u.ifstate,
                                idev->sysport_devs[ninfo.sysport]->name);
                        rc = -1;
                        break;
                }
            }
            break;
        case INNO_ND_LINKSTATE:
            if((idev->sysport_devs[ninfo.sysport]) && (idev->inno_netdev.single_interface == 0)){
                switch(ninfo.u.linkstate) {
                    case INNO_LINK_UP:
                        netif_carrier_on(idev->sysport_devs[ninfo.sysport]);
                        ipd_debug("%s link up\n", idev->sysport_devs[ninfo.sysport]->name);
                        break;
                    case INNO_LINK_DOWN:
                        netif_carrier_off(idev->sysport_devs[ninfo.sysport]);
                        ipd_debug("%s link down\n", idev->sysport_devs[ninfo.sysport]->name);
                        break;
                    default:
                        ipd_err("Unknown state %d for interface %s\n", ninfo.u.linkstate,
                                idev->sysport_devs[ninfo.sysport]->name);
                        rc = -1;
                        break;
                }
            }
            break;
        case INNO_ND_MTU:
            {
                int mtu_size=ninfo.u.mtu;
                if(idev->inno_netdev.single_interface) {
                    ninfo.sysport = idev->inno_netdev.ndev_sindex;
                    mtu_size+=NETDEV_INFO_HEADER_SIZE;
                }
                idev->sysport_devs[ninfo.sysport]->mtu = mtu_size;
                ipd_debug("MTU for %s set to %d\n", idev->sysport_devs[ninfo.sysport]->name, mtu_size);
            }
            break;
        case INNO_ND_RING:
            if(ninfo.u.ring.type == INNO_RING_TYPE_TX) {
                memset(&(idev->inno_stats.tx_ring_stats[ninfo.u.ring.num]),0,
                       sizeof(idev->inno_stats.tx_ring_stats[ninfo.u.ring.num]));
            } else if (ninfo.u.ring.type == INNO_RING_TYPE_RX) {
                memset(&(idev->inno_stats.rx_ring_stats[ninfo.u.ring.num]),0,
                       sizeof(idev->inno_stats.rx_ring_stats[ninfo.u.ring.num]));
            } else {
                ipd_err("Invalid ring type %d\n", ninfo.u.ring.type);
            }
            break;
        case INNO_ND_PTP_EVENT_INFO:
            idev->inno_netdev.ptp_event_queue_id = ninfo.u.ptp_info.ptp_event_queue_id;
            idev->inno_netdev.ptp_event_queue_valid = ninfo.u.ptp_info.ptp_event_queue_valid;
            break;
        case INNO_ND_INGRESS_SFLOW_SAMPLE_RATE:
            idev->inno_netdev.isflow_sample_rate[ninfo.sysport] = ninfo.u.sflow_sample_rate;
            ipd_info("Sflow sample_rate set for sysport %d is %d\n", ninfo.sysport, ninfo.u.sflow_sample_rate);
            break;
        case INNO_ND_EGRESS_SFLOW_SAMPLE_RATE:
            idev->inno_netdev.esflow_sample_rate[ninfo.sysport] = ninfo.u.sflow_sample_rate;
            ipd_info("Sflow sample_rate set for sysport %d is %d\n", ninfo.sysport, ninfo.u.sflow_sample_rate);
            break;
        default:
            ipd_err("Wrong id %d\n", ninfo.id);
            rc = -1;
            break;
    }

    return rc;
}

/** @brief Get info for a netdev for a port
 *
 *  @param [in] idev  - innovium device
 *  @param [in] ninfo - netdev info from user
 *  @return ERRNO
 */
int
inno_netdev_get(inno_device_t  *idev,
                inno_ioctl_netdev_info_t *ninfo)
{
    int rc=0;
    ipd_verbose("get netdev for sysport %x\n", ninfo->sysport);
    switch(ninfo->id) {
        case INNO_ND_RING:
            if(ninfo->u.ring.type == INNO_RING_TYPE_TX) {
                ninfo->u.ring.packets = idev->inno_stats.tx_ring_stats[ninfo->u.ring.num].packets;
                ninfo->u.ring.bytes = idev->inno_stats.tx_ring_stats[ninfo->u.ring.num].bytes;
            } else if (ninfo->u.ring.type == INNO_RING_TYPE_RX) {
                ninfo->u.ring.packets = idev->inno_stats.rx_ring_stats[ninfo->u.ring.num].packets;
                ninfo->u.ring.bytes = idev->inno_stats.rx_ring_stats[ninfo->u.ring.num].bytes;
            } else {
                ipd_err("Invalid ring type %d\n", ninfo->u.ring.type);
            }
            break;
        case INNO_ND_IFSTATE:
            if((idev->sysport_devs[ninfo->sysport]) && (idev->inno_netdev.single_interface == 0)) {
                if(idev->sysport_devs[ninfo->sysport]->flags & IFF_UP) {
                    ninfo->u.ifstate = INNO_INTF_UP;
                } else {
                    ninfo->u.ifstate = INNO_INTF_DOWN;
                }
            }
            break;
        case INNO_ND_LINKSTATE:
            if((idev->sysport_devs[ninfo->sysport]) && (idev->inno_netdev.single_interface == 0)) {
                if (netif_carrier_ok(idev->sysport_devs[ninfo->sysport])) {
                    ninfo->u.linkstate = INNO_LINK_UP;
                } else {
                    ninfo->u.linkstate = INNO_LINK_DOWN;
                }
            }
            break;
        default:
            ipd_err("Wrong id %d\n", ninfo->id);
            rc = -1;
            break;
    }
    return rc;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
/** @brief Initialize generic netlink
 *
 *  @param [in] idev  - innovium device
 *  @param [in] name  - netlink name
 *  @return ERRNO
 */
int
inno_gen_netlink_create(inno_device_t  *idev, char *name, netLinkAppType nl_type, char *gen_mcgrp_name)
{
    int rc=-1, name_len;
    inno_gen_netlink_t *p_gnetlink;

    p_gnetlink = (inno_gen_netlink_t*) kzalloc(sizeof(inno_gen_netlink_t), GFP_KERNEL);
    if(p_gnetlink == NULL) {
        ipd_err("Generic netlink allocate failed\n");
        return -1;
    }
    name_len = GENL_NAMSIZ-1;
    strncpy(p_gnetlink->inno_genl_family.name, name, name_len);
    p_gnetlink->inno_genl_family.name[name_len] = '\0';
    p_gnetlink->inno_genl_family.version = 1;
    if(nl_type == INNO_NL_SFLOW){
        p_gnetlink->nl_type = INNO_NL_SFLOW;
        p_gnetlink->inno_genl_family.maxattr = INNO_SFLOW_GENL_ATTR_MAX;
    }

    if(nl_type == INNO_NL_PGENL){
        p_gnetlink->nl_type = INNO_NL_PGENL;
        p_gnetlink->inno_genl_family.maxattr = INNO_PGENL_ATTR_MAX;
    }
    p_gnetlink->inno_genl_family.netnsok = true;
    p_gnetlink->inno_genl_family.module = THIS_MODULE;
    strcpy(inno_genl_sflow_mcgrps.name, gen_mcgrp_name);
    p_gnetlink->inno_genl_family.mcgrps = &inno_genl_sflow_mcgrps;
    p_gnetlink->inno_genl_family.n_mcgrps = 1;
    p_gnetlink->seq = 0;
    rc = genl_register_family(&(p_gnetlink->inno_genl_family));
    if(rc) {
        ipd_err("genl_register_family failed, rc - %d\n", rc);
        kfree(p_gnetlink);
        p_gnetlink = NULL;
        return rc;
    }

    if(nl_type == INNO_NL_SFLOW){
        idev->inno_netlink.sflow = 1;
        idev->inno_netlink.sflow_genl = p_gnetlink;
    }

    ipd_info("Generic netlink %s created with mcgrp name %s\n", name, gen_mcgrp_name);

    if(idev->inno_netlink.gnetlink == NULL){
        idev->inno_netlink.gnetlink = p_gnetlink;
    } else {
        inno_gen_netlink_t *p;
        p = idev->inno_netlink.gnetlink;
        while(p->next != NULL) p = p->next;
        p->next = p_gnetlink;
    }
    return rc;
}

/** @brief De-initialize generic netlink
 *
 *  @param [in] idev  - innovium device
 *  @param [in] name  - netlink name
 *  @return ERRNO
 */
int
inno_gen_netlink_delete(inno_device_t  *idev, char *name)
{
    int rc=-1;
    inno_gen_netlink_t *p;
    p = idev->inno_netlink.gnetlink;
    if(p->nl_type == INNO_NL_SFLOW){
        idev->inno_netlink.sflow = 0;
        idev->inno_netlink.sflow_genl = NULL;
        ipd_info("Sflow netlink filter disabled\n");
    }
    if(strcmp(name, p->inno_genl_family.name) == 0) {
        idev->inno_netlink.gnetlink = idev->inno_netlink.gnetlink->next;
    } else {
        inno_gen_netlink_t *q;
        q = p;
        p = p->next;
        while(p != NULL) {
            if(strcmp(p->inno_genl_family.name,name) == 0){
                q->next = p->next;
                break;
            }
            q = q->next;
            p = p->next;
        }

    }
    if(p == NULL) {
        ipd_err("Generic netlink %s not found\n", name);
        return -1;
    }
    rc = genl_unregister_family(&(p->inno_genl_family));
    if(rc) {
        ipd_err("genl_unregister_family failed, rc - %d\n", rc);
    }
    kfree(p);
    ipd_info("Generic netlink %s deleted\n", name);
    return rc;
}

/** @brief Search generic netlink
 *
 *  @param [in] idev  - innovium device
 *  @param [in] name  - netlink name
 *  @return inno_gen_netlink_t*
 */
inno_gen_netlink_t*
inno_gen_netlink_search(inno_device_t  *idev, char *name)
{
	inno_gen_netlink_t *p;
	p = idev->inno_netlink.gnetlink;
	while(p != NULL) {
        if(strcmp(name, p->inno_genl_family.name) == 0) {
			break;
		}
		p = p->next;
	}

	if(p == NULL) {
		return NULL;
	}
	return p;
}

/** @brief Search generic netlink with type sflow
 *
 *  @param [in] idev  - innovium device
 *  @return inno_gen_netlink_t*
 */
inno_gen_netlink_t*
inno_gen_netlink_search_sflow(inno_device_t  *idev)
{
	inno_gen_netlink_t *p;
	p = idev->inno_netlink.gnetlink;
	while(p != NULL) {
		if(p->nl_type == INNO_NL_SFLOW){
			break;
		}
		p = p->next;
	}

	if(p == NULL) {
		return NULL;
	}
	return p;
}

/** @brief Create netlink
 *
 *  @param [in] idev  - innovium device
 *  @param [in] ioctl - ioctl area from user
 *  @return ERRNO
 */
int
inno_netlink_create(inno_device_t  *idev,
                    inno_ioctl_netlink_t *ioctl)
{
    char name[IFNAMSIZ];
    int  rc=-1;

    if(strlen(ioctl->intf_name) != 0) {
#pragma GCC diagnostic push
#if __GNUC__ >= 8
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
        strncpy(name, ioctl->intf_name, IFNAMSIZ);
#pragma GCC diagnostic pop
    }else{
        ipd_err("Netlink name is NULL\n");
        return -EINVAL;
    }

    if(ioctl->type == NETLINK_GENERIC) {
        rc = inno_gen_netlink_create(idev, name, ioctl->nl_app_type, ioctl->gen_mcgrp_name);
    } else {
        ipd_err("Unknow netlink type %d\n", ioctl->type);
    }
    return rc;
}

/** @brief Delete the netlink
 *
 *  @param [in] idev  - innovium device
 *  @param [in] ioctl - ioctl area from user
 *  @return ERRNO
 */
int
inno_netlink_delete(inno_device_t  *idev,
                    inno_ioctl_netlink_t *ioctl)
{
    char name[IFNAMSIZ];
    int  rc=-1;

    if(strlen(ioctl->intf_name) != 0) {
#pragma GCC diagnostic push
#if __GNUC__ >= 8
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
        strncpy(name, ioctl->intf_name, IFNAMSIZ);
#pragma GCC diagnostic pop
    }else{
        ipd_err("Netlink name is NULL\n");
        return -EINVAL;
    }
    if(ioctl->type == NETLINK_GENERIC) {
        rc = inno_gen_netlink_delete(idev, name);
    } else {
        ipd_err("Unknow netlink type %d\n", ioctl->type);
    }
    return rc;
}

/** @brief Deinit netlink
 *
 *  @param [in] idev  - innovium device
 *  @return NONE
 */
void
inno_netlink_deinit(inno_device_t  *idev)
{
    inno_gen_netlink_t *p;
    p = idev->inno_netlink.gnetlink;
    while(p != NULL) {
        char name[IFNAMSIZ];

        if(strlen(p->inno_genl_family.name) != 0) {
            strncpy(name, p->inno_genl_family.name, IFNAMSIZ);
        } else {
            ipd_err("Netlink name is NULL\n");
        }
        inno_gen_netlink_delete(idev, name);
        p = idev->inno_netlink.gnetlink;
    }
}

/** @brief Add trapid to generic netlink
 *
 *  @param [in] idev  - innovium device
 *  @param [in] inno_gen_netlink_t  - Generic netlink socker struct
 *  @param [in] uint32_t            - trapid
 *  @return ERRNO
 */
int
inno_add_trapid_genl(inno_device_t  *idev,
                     inno_gen_netlink_t *inno_genl,
                     uint32_t trapid)
{
    int rc = 0;
    inno_gen_netlink_tid_t *inno_genl_trapid;

    inno_genl_trapid = (inno_gen_netlink_tid_t*) kmalloc(sizeof(inno_gen_netlink_tid_t), GFP_KERNEL);
    if(inno_genl_trapid == NULL) {
        ipd_err("No memory\n");
        return -ENOMEM;
    }
    list_add(&inno_genl_trapid->list, &idev->trapid_list);
    inno_genl_trapid->trapid = trapid;
    inno_genl_trapid->inno_genl = inno_genl;
    memset(&(inno_genl_trapid->stats), 0, sizeof(inno_genl_trapid->stats));
    ipd_info("Added trapid %d to netlink %s nl_type is %d\n", trapid, inno_genl->inno_genl_family.name, inno_genl->nl_type);
    return rc;
}

/** @brief Remove trapid to generic netlink
 *
 *  @param [in] idev  - innovium device
 *  @param [in] inno_gen_netlink_t  - Generic netlink socker struct
 *  @param [in] uint32_t            - trapid
 *  @return ERRNO
 */
int
inno_rem_trapid_genl(inno_device_t  *idev,
                     inno_gen_netlink_t *inno_genl,
                     uint32_t trapid)
{
    int rc = 0;
    inno_gen_netlink_tid_t *inno_genl_trapid;
    struct list_head *l = NULL ;

    list_for_each(l, &idev->trapid_list){
        inno_genl_trapid = list_entry(l, struct inno_gen_netlink_tid_s, list);
        if(inno_genl_trapid->trapid == trapid){
            ipd_info("Removing trapid %d from netlink %s\n", trapid, inno_genl_trapid->inno_genl->inno_genl_family.name);
            list_del(&inno_genl_trapid->list);
            kfree(inno_genl_trapid);
            break;
        }
    }
    return rc;
}

/** @brief Set netlink options
 *
 *  @param [in] idev  - innovium device
 *  @param [in] ioctl - ioctl area from user
 *  @return ERRNO
 */
int
inno_netlink_set(inno_device_t  *idev,
                 inno_ioctl_netlink_info_t ioctl)
{
    inno_gen_netlink_t     *inno_genl;
    char                   name[IFNAMSIZ];
    int                    rc=0;

    ipd_verbose("set netlink for name %s\n", ioctl.intf_name);

    if(strlen(ioctl.intf_name) != 0) {
        strncpy(name, ioctl.intf_name, IFNAMSIZ);
    }else{
        ipd_err("Netlink name is NULL\n");
        return -EINVAL;
    }

    inno_genl = inno_gen_netlink_search(idev, name);
    if(inno_genl == NULL) {
        ipd_err("Generic netlink %s not found\n", name);
        return -ENOENT;
    }

    switch(ioctl.id) {
        case INNO_NL_TRAPID:
            if(ioctl.u.trapid_info.set == 1) {
                rc = inno_add_trapid_genl(idev, inno_genl,ioctl.u.trapid_info.trapid);
            } else {
                rc = inno_rem_trapid_genl(idev, inno_genl,ioctl.u.trapid_info.trapid);
            }
            break;
        default:
            ipd_err("Wrong netlink set id %d\n", ioctl.id);
            rc = -1;
            break;
    }

    return rc;
}


#endif
