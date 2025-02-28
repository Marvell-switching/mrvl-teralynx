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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/string.h>
#include <linux/swab.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>

#include "pci_common_ipd.h"
#include "ipd.h"
#include "inno_ioctl.h"
#include "inno_ldh.h"
#include "inno_enet.h"
#include "ipd_version.h"

/* Major number for IPD */
#define INNO_MAJOR    101

/* MSIX Vector */
#define INNO_MSIX_NUM_VECTORS    32

#define LEARN_STATUS_LOOP_RETRY_CNT_MAX 10

#define INNO_TERALYNX_CHIP_HDR_LEN     12
#define INNO_TERALYNX_CHIP_DBG_HDR_LEN 32
#define INNO_TL10_CHIP_HDR_LEN          20
#define INNO_TL10_CHIP_DBG_HDR_LEN      40


#define RING_STS_RETRY_CNT 1000

static unsigned int inno_intr = INNO_IRQ_TYPE_BEST_EFFORT;
module_param(inno_intr, uint, 0644);
MODULE_PARM_DESC(inno_intr, "Interrupt mode: 1-BestEffort(default); 2-MSIX_only; 3-MSI_only; 4-INTX_only");

uint32_t ipd_boottype = IPD_BOOTTYPE_COLD;
module_param_named(boottype, ipd_boottype, uint, 0644);
MODULE_PARM_DESC(boottype, "boot type(0-1)");


static unsigned int inno_ipd_opened = 0;
static unsigned int inno_ipd_opened_before = 0;
static int ipd_hw_init_done = 0;
static int ipd_user_cfg_done = 0;

extern void inno_sysfs_init(inno_device_t  *idev, int max_device);
extern void inno_sysfs_deinit(void);
uint16_t rupt_mask_words = 0;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
static int __init
inno_probe(struct pci_dev             *pdev,
           const struct pci_device_id *ent);
static void __devexit
inno_remove(struct pci_dev *pdev);
static int
inno_ioctl(struct inode  *i,
           struct file   *fp,
           unsigned int  cmd,
           unsigned long arg);

#else
static int
inno_probe(struct pci_dev             *pdev,
           const struct pci_device_id *ent);
static void
inno_remove(struct pci_dev *pdev);
static long
inno_ioctl(struct file   *fp,
           unsigned int  cmd,
           unsigned long arg);

#endif
static int inno_intr_init(inno_device_t *idev);
static int inno_intr_deinit(inno_device_t *idev);
static int inno_hw_init (inno_device_t *idev);
static int inno_cleanup_resources(inno_device_t *idev);
static int inno_override_flow_control(inno_device_t *idev, int enable);
/* #define EVT_THREAD 1
*/

#ifdef EVT_THREAD
int exit_evt_thread_hndlr = 0;
int evt_thread_timer = 1000;
struct task_struct *evt_thread;
#endif
static char inno_driver_name[] = "Innovium Switch PCIe Driver";

pci_ers_result_t
inno_error_detected(struct pci_dev  *dev,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
                    enum pci_channel_state state);
#else
                    pci_channel_state_t state);
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
pci_ers_result_t
inno_link_reset(struct pci_dev *dev);
#endif
pci_ers_result_t
inno_slot_reset(struct pci_dev *dev);
void
inno_resume(struct pci_dev *dev);

/* Innovium device ids */
static struct pci_device_id inno_ids[] =
{
    { PCI_DEVICE(INNO_PCI_VENDOR_ID, INNO_TERALYNX_PCI_DEVICE_ID) },
    { PCI_DEVICE(MRVL_PCI_VENDOR_ID, MRVL_TL10_PCI_DEVICE_ID) },
    {                             0,                              },
};

/* PCI IDs */
MODULE_DEVICE_TABLE(pci, inno_ids);

/* Ring interrupt bits */
uint32_t inno_rxq_int[] = {11, 12, 13, 14, 15, 16, 17, 18};
uint32_t inno_txq_int[] = {4, 5, 6, 7, 8, 9, 10};

/* PCI driver for Innovium Nodes*/
const static struct pci_error_handlers inno_pci_error =
{
    .error_detected = inno_error_detected,
    .mmio_enabled   = NULL,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
    .link_reset     = inno_link_reset,
#endif
    .slot_reset     = inno_slot_reset,
    .resume         = inno_resume
};


/* Count of probed nodes - Maximum is MAX_INNO_DEVICES */
static uint32_t inno_num_devices = 0;

/* Per node info struct */
static inno_device_t inno_instances[MAX_INNO_DEVICES];

/* cdev for IPD */
static dev_t       inno_major_dev = MKDEV(INNO_MAJOR, 0);
static struct cdev inno_cdev;
static struct class *inno_cl;

/* Fixed offset macros for POOL allocations */
#define POOL_TX_IDX_OFFSET(num)    (0 + ((num) * CACHE_ALIGN(4)))
#define POOL_RX_IDX_OFFSET(num) \
    (POOL_TX_IDX_OFFSET(RX_RING_OFFSET) + ((num) * CACHE_ALIGN(4)))
#define POOL_IAC_WB_OFFSET \
    POOL_RX_IDX_OFFSET(NUM_RX_RINGS)
#define POOL_IAC_BLK_OFFSET \
    (POOL_IAC_WB_OFFSET + CACHE_ALIGN(256 + 4))
#define POOL_HRR_OFFSET \
    (POOL_IAC_BLK_OFFSET + CACHE_ALIGN(4096))

#define POOL_LEARN_OFFSET \
    (POOL_HRR_OFFSET + CACHE_ALIGN(8))
#define POOL_RUPT_OFFSET \
    (POOL_LEARN_OFFSET + CACHE_ALIGN(8))
#define POOL_END \
    (POOL_RUPT_OFFSET + 8 * CACHE_ALIGN(8))

/** Misc util funtions
 *
 */
static char *intrmode2str(int intr_mode)
{
    switch(intr_mode) {
        case INNO_IRQ_TYPE_BEST_EFFORT:
            return "Best-effort";
        case INNO_IRQ_TYPE_MSIX_ONLY:
            return "MSI-X";
        case INNO_IRQ_TYPE_MSI_ONLY:
            return "MSI";
        case INNO_IRQ_TYPE_INTX_ONLY:
            return "INT-a";
        default:
	    return "Unknown";
    }
}

static char *inno_intrtype_status2str(int status)
{

    switch(status) {
        case INNO_INTR_INTX_ENABLED:
            return "INNO_INTR_INTA_ENABLED";
        case INNO_INTR_MSI_ENABLED:
            return "INNO_INTR_MSI_ENABLED";
        case INNO_INTR_MSIX_ENABLED:
            return "INNO_INTR_MSIX_ENABLED";
        default:
            return "Unknown";
    }
}

void
pen_addr(uint32_t ib, uint32_t addr, uint32_t cmpid,
              uint32_t index, uint32_t *addr_hi, uint32_t *addr_lo)
{
    if (cmpid == 16) {
        /* For PIC pens the IB number is actually the <PIC, IB> encoding */
        *addr_hi = (cmpid | ((ib & 0x0f) << 8));
        *addr_lo = (addr + index + (0x4000 * (ib >> 4)));
    } else {
        *addr_hi = (uint32_t)(cmpid | (uint32_t)(ib << 8));
        *addr_lo = (uint32_t)(addr + index);
    }
}

int
isn_read_pen(inno_device_t  *idev, uint32_t ib, uint32_t addr, uint32_t cmpid,
                 uint32_t index, uint32_t *data, int data_size_bytes)
{
    static uint16_t seqid = 0;
    uint32_t cmdtype = 1;  // read
    int i, iter = 0;
    uint32_t addr_hi, addr_lo;
    uint32_t status;

    // check to see if the SYNC status is not "done".  If so, reset it
    status = REG32(SYNC_STS_0);
    if ((status & 0x6) != 0) {
        REG32(SYNC_STS_0) = 1;  // clear status
    }

    pen_addr(ib, addr, cmpid, index, &addr_hi, &addr_lo);
    REG32(SYNC_ISN_ADR_LO_0) =  addr_lo;
    REG32(SYNC_ISN_ADR_HI_0) = addr_hi;
    REG32(SYNC_CMD_0) =  (++seqid << 20) | (data_size_bytes << 9) | cmdtype;

    do {
        status = REG32(SYNC_STS_0);
        ++iter;
        if (iter == 1000) {
            return -1;
        }
    } while (status & 0x6);

    for (i=0; i < data_size_bytes; i+=4) {
        *data = REG32(SYNC_DATA_0 + i);
        data++;
    }

    return 0;
}

int
isn_write_pen(inno_device_t  *idev, uint32_t ib, uint32_t addr,
                  uint32_t cmpid, uint32_t index, uint32_t *data,
                  int data_size_bytes)
{
    static uint16_t seqid = 0;
    uint32_t cmdtype = 2;  // write
    int i, iter = 0;
    uint32_t addr_hi, addr_lo;
    uint32_t status;

    // check to see if the SYNC status is not "done".  If so, reset it
    status = REG32(SYNC_STS_0);
    if ((status & 0x6) != 0) {
        REG32(SYNC_STS_0) =  1;  // clear status
    }

    pen_addr(ib, addr, cmpid, index, &addr_hi, &addr_lo);
    REG32(SYNC_ISN_ADR_LO_0) = addr_lo;
    REG32(SYNC_ISN_ADR_HI_0) = addr_hi;

    for (i=0; i < data_size_bytes; i+=4) {
        REG32(SYNC_DATA_0 + i) =  *data;
        data++;
    }

    REG32(SYNC_CMD_0) = (++seqid << 20) | (data_size_bytes << 9) | cmdtype;

    do {
        status = REG32(SYNC_STS_0);
        ++iter;
        if (iter == 1000) {
            return -1;
        }
    } while (status & 0x6);

    return 0;
}

/** @brief Rupt mask set IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @param [io] ioctl - ioctl data area
 *  @return ERRNO
 */
static int
inno_rupt_mask(inno_device_t          *idev,
               inno_ioctl_rupt_mask_t *ioctl_mask)
{
    int                i;
    inno_rupt_vector_t *vec = &idev->rupts[ioctl_mask->vector];
    uint16_t           msix_num;

    if (ioctl_mask->vector >= NUM_MSIX_VECS) {
        return -EINVAL;
    }

    if (vec->vector == 0) {
        /* No IRQ allocated */
        ipd_err("vector has no irq: %x %x\n", ioctl_mask->vector, vec->vector);
        return -EINVAL;
    }

    /* Set the H/W registers */
    msix_num = (ioctl_mask->vector < idev->num_vectors) ? ioctl_mask->vector : idev->num_vectors - 1;
    if(idev->flags & INNO_FLG_MSI_WA_ENABLED) {
        msix_num = idev->msi_wa_vector;
    }

    for (i = 0; i < rupt_mask_words; i++) {
        if (vec->mask[i] != 0) {
            /* Disable and clear any existing ones */
            ipd_verbose("Writing INTR_INMC(0x%x) + i*4; reg: 0x%x val: 0x%x\n", idev->inno_intr_regs.intr_inmc, (idev->inno_intr_regs.intr_inmc + i*4), vec->mask[i]);
            ipd_verbose("Writing INTR_INCR(0x%x) + i*4; reg: 0x%x val: 0x%x\n", idev->inno_intr_regs.intr_incr, (idev->inno_intr_regs.intr_incr + i*4), vec->mask[i]);
            REG32(idev->inno_intr_regs.intr_inmc + i * 4) = vec->mask[i];
            REG32(idev->inno_intr_regs.intr_incr + i * 4) = vec->mask[i];
        }
        vec->mask[i] = ioctl_mask->mask[i];
        if (vec->mask[i] != 0) {
            /* Set the vector bitfield for each set bit */
            int j;
            for (j = 0; j < 32; j++) {
                if (vec->mask[i] & (1 << j)) {
                    REG32(idev->inno_intr_regs.intr_immr + (i * 32 + j) * 4) = msix_num;
                    ipd_verbose("Setting INTR_IMMR + %d msix_num: %d\n", (i*32 + j), msix_num);
                }
            }
            ipd_verbose("Writing INTR_INMS + %d; reg: 0x%x val: 0x%x\n",(i*4),  REG32(idev->inno_intr_regs.intr_inms + (i*4)), vec->mask[i]);
            REG32(idev->inno_intr_regs.intr_inms + i * 4) = vec->mask[i];
        }
    }

    /* One wakeup for a changed mask */
    vec->flag = 1;
    wake_up_interruptible(&vec->wait_q);

    return 0;
}


/** @brief Rupt wait IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @param [io] ioctl - ioctl data area
 *  @return ERRNO
 */
static int
inno_rupt_wait(inno_device_t          *idev,
               inno_ioctl_rupt_wait_t *ioctl_wait)
{
    unsigned long      flags;
    int                j;
    int                no_rupts;

    inno_rupt_vector_t *vec = &idev->rupts[ioctl_wait->vector];

    if (ioctl_wait->vector >= NUM_MSIX_VECS) {
        return -EINVAL;
    }

    if (vec->vector == 0) {
        return -EINVAL;                         /* Not allocated */
    }

    ipd_verbose("Waiting for irq %x %x\n", vec->vector, vec->mask[0]);

    /* Enable the interrupts */
    spin_lock_irqsave(&idev->rupt_lock, flags);
    no_rupts = 1;
    for (j = 0; j < rupt_mask_words; j++) {
        if (vec->mask[j] != 0) {
            no_rupts = 0;
            REG32(idev->inno_intr_regs.intr_inms + j * 4) = vec->mask[j];
        }
    }
    spin_unlock_irqrestore(&idev->rupt_lock, flags); /* Release the lock */

    if (no_rupts == 1) {
        return -EPERM;
    }

    if (ioctl_wait->timeout) {
        ioctl_wait->remaining_time =
            wait_event_interruptible_timeout(vec->wait_q,
                vec->flag != 0,
                msecs_to_jiffies(ioctl_wait->timeout));
    } else {
        wait_event_interruptible(vec->wait_q, vec->flag != 0);
    }

    spin_lock_irqsave(&idev->rupt_lock, flags);
    vec->flag = 0;
    memcpy(ioctl_wait->rupts, vec->pend, rupt_mask_words * 4);
    spin_unlock_irqrestore(&idev->rupt_lock, flags); /* Release the lock */

    ipd_verbose("Rupt wait exit %x %x %x\n", vec->vector, vec->mask[0], REG32(idev->inno_intr_regs.intr_inms));

    return 0;
}

/** @brief Tx queue enable function
 *
 *  @param [in] idev - innovium device
 *  @param [in] ringnum - Tx ring number
 *  @return 0 for success and -1 for error
 */
static int
inno_enable_queue_tx(inno_device_t  *idev,
                     int            ringnum)
{
    int rc=-1, i=0, dm_status, status;
    uint32_t reg;
    txq_0_desc_pidx_t txq_pidx;

    /* Step 1 - Check whether TXQ DM or TxE are in disabled state */

    reg        = REG32(DM_STS);
    dm_status  = (reg >> ringnum) & 1;
    reg        = REG32(STS);
    status     = (reg >> ringnum) & 1;
    ipd_debug("TXQ status:%d dm_stats:%d for queue %d\n", status, dm_status,
              ringnum);
    if((dm_status == 1) && (status == 1)){
        ipd_debug("TXQ already in enable state for queue %d\n", ringnum);
        return 0;
    } else {
        ipd_info("TXQ ring %d enable start: TXQ = 0x%x, STS = 0x%x, DISABLE_STS_DONE = 0x%x DM_STS = 0x%x", ringnum, REG32(TXQ), REG32(STS), REG32(DISABLE_STS_DONE), REG32(DM_STS));
    }

    /* Step 2 - Reset DM & TxE */
    
    reg        = REG32(TXQ);
    reg        &= ~(0xc << (ringnum * 4));
    REG32(TXQ) = reg;


    /* Step 3 - Clear pidx */
    txq_pidx.flds.num_f = 0;
    REG32(TXQ_x_DESC_PIDX(ringnum)) = txq_pidx.data;


    /* Step 4 - Remove reset and renable TXQ */
    reg        = REG32(TXQ);
    reg        |= (0xf << (ringnum * 4));
    REG32(TXQ) = reg;


    /* Step 4 - Wait for DM and TxE to become enable */
    for(i = 0; i < RING_STS_RETRY_CNT; i++) {
        uint32_t reg, dm_sts_done, txe_sts_done;

        reg=REG32(DISABLE_STS_DONE);
        
        dm_sts_done  =  (reg >> (ringnum * 2)) & 1;
        txe_sts_done =  (reg >> ((ringnum * 2) + 1)) & 1;

        if ((dm_sts_done == 0) && (txe_sts_done == 0)) {
            rc = 0;
            break;
        }
        udelay(10);
    }

    if(rc == 0) {
        uint32_t dm_sts, txe_sts;
        
        reg    = REG32(DM_STS);
        dm_sts = (reg >> ringnum) & 1;
        
        reg     =  REG32(STS);
        txe_sts =  (reg >> ringnum)&1;
        if ((dm_sts != 1) || (txe_sts != 1)) {
            rc = -1;
        }
    }

    if(rc) {
        ipd_err("TXQ enable failed for queue %d TXQ = 0x%x, STS = 0x%x, DISABLE_STS_DONE = 0x%x DM_STS=0x%x",
                ringnum,  REG32(TXQ), REG32(STS), REG32(DISABLE_STS_DONE), REG32(DM_STS));
    } else {
        ipd_info("TXQ enable succeeded for queue %d\n", ringnum);
    }

    return 0;
}

/** @brief Tx queue disable function
 *
 *  @param [in] idev - innovium device
 *  @param [in] ringnum - Tx ring number
 *  @return 0 for success and -1 for error
 */
static int
inno_disable_queue_tx(inno_device_t  *idev,
                      int            ringnum)
{
    int rc=-1, i=0, dm_status, status;
    uint32_t reg;

    /* Step 1 - Disable TXQ only if either DM or TxE are in enabled state */

    reg  = REG32(DM_STS);
    dm_status = (reg >> ringnum) & 1;
    reg  = REG32(STS);
    status = (reg >> ringnum) & 1;
    ipd_debug("TXQ status:%d dm_stats:%d for queue %d\n", status, dm_status,
              ringnum);
    if((dm_status == 0) && (status == 0)){
        ipd_debug("TXQ already in disable state for queue %d", ringnum);
        return 0;
    } else {
        ipd_info("TXQ ring %d disable start: TXQ = 0x%x, STS = 0x%x, DISABLE_STS_DONE = 0x%x", ringnum, REG32(TXQ), REG32(STS), REG32(DISABLE_STS_DONE));
    }

    /* Step 2 - Disable DM & TxE */

    
    reg        = REG32(TXQ);
    reg        &= ~(0x3 << (ringnum * 4));
    REG32(TXQ) = reg;

    /* Step 3 - Wait for DM and TxE disable status to be done */

    for(i = 0; i < RING_STS_RETRY_CNT; i++) {
        uint32_t reg, dm_sts_done, txe_sts_done;

        reg=REG32(DISABLE_STS_DONE);
        
        dm_sts_done  =  (reg >> (ringnum*2))&1;
        txe_sts_done =  (reg >> ((ringnum*2)+1))&1;

        if ((dm_sts_done == 1) && (txe_sts_done == 1)) {
            rc = 0;
            break;
        }
        udelay(10);
    }

    if(rc == 0) {
        uint32_t dm_sts, txe_sts;
        
        reg = REG32(DM_STS);
        dm_sts  =  (reg >> ringnum) & 1;
        
        reg = REG32(STS);
        txe_sts =  (reg >> ringnum) & 1;
        if ( (dm_sts != 0) || (txe_sts != 0)) {
            rc = -1;
        }
    }

    if(rc) {
        ipd_err("TXQ disable failed for queue %d TXQ = 0x%x, STS = 0x%x, DISABLE_STS_DONE = 0x%x\n",
                ringnum,  REG32(TXQ), REG32(STS), REG32(DISABLE_STS_DONE));
    } else {
        ipd_info("TXQ disable succeeded for queue %d\n", ringnum);
    }

    return 0;
}

/** @brief Rx queue enable function
 *
 *  @param [in] idev - innovium device
 *  @param [in] ringnum - Rx ring number
 *  @return 0 for success and -1 for error
 */
static int
inno_enable_queue_rx(inno_device_t  *idev,
                     int            ringnum)
{
    int rc=-1, i=0;
    int dm_status,status;
    int ring_num;
    uint32_t reg;
    rxq_0_desc_pidx_t rxq_pidx;
    int is_switch_ring = (ringnum < NUM_RX_SWITCH_RINGS);

    /* Step 1 - Check whether RXQ DM or RxE are in disabled state */
    reg  = REG32(DM_STS);
    dm_status = (reg >> (ringnum + 7)) & 1;
    reg  = REG32(STS);
    status = (reg >> (ringnum + 7)) & 1;
    ipd_debug("RXQ status:%d dm_stats:%d for queue %d\n", status, dm_status,
              ringnum);
    if((dm_status == 1)){
        ipd_debug("RXQ already in enable state for queue %d", ringnum);
        return 0;
    } else {
        ipd_info("RXQ ring %d enable start: RXQ = 0x%x, STS = 0x%x, DM_STS = 0x%x DISABLE_STS_DONE = 0x%x",
                 ringnum, REG32(RXQ), REG32(STS), REG32(DM_STS),
                 REG32(DISABLE_STS_DONE));
    }

    if (is_switch_ring) {
        /* instruct the DTM to start sending traffic for this ring */
        reg = REG32(UCM_DTM);

        reg &= ~(0x1 << ringnum);
        reg &= ~(0x1 << (ringnum + 6));
        REG32(UCM_DTM) = reg;
    }

    /* Step 2 - Reset DM & RxE */
    
    reg        = REG32(RXQ);
    reg        &= ~(0x4 << (ringnum * 4));
    REG32(RXQ) = reg;

    /* Step 3 - Clear pidx */
    rxq_pidx.flds.num_f = 0;
    REG32(RXQ_x_DESC_PIDX(ringnum)) = rxq_pidx.data;


    /* Step 4 - Remove reset and renable RXQ */
    reg        = REG32(RXQ);
    reg       |= (0xf << (ringnum * 4));
    REG32(RXQ) = reg;


    /* Step 5 - Wait for DM and RxE to become enable */
    
    ring_num = ringnum + RX_RING_OFFSET;

    for(i=0;i<RING_STS_RETRY_CNT;i++) {
        uint32_t reg, dm_sts_done, rxe_sts_done;

        reg=REG32(DISABLE_STS_DONE);
        
        dm_sts_done  =  (reg >> (ring_num * 2)) & 1;
        rxe_sts_done =  (reg >> ((ring_num * 2) + 1)) & 1;

        if((dm_sts_done == 0) && (rxe_sts_done == 0)) {
            rc = 0;
            break;
        }
        udelay(10);
    }

    if(rc == 0) {
        uint32_t dm_sts, rxe_sts;
        
        reg=REG32(DM_STS);
        dm_sts  =  (reg >> ring_num) & 1;
        
        reg=REG32(STS);
        rxe_sts =  (reg >> ring_num) & 1;
        if((dm_sts != 1)||(rxe_sts != 1)) rc = -1;
    }

    if(rc) {
        ipd_err("RXQ enable failed for queue %d RXQ = 0x%x, STS = 0x%x, DISABLE_STS_DONE = 0x%x\n",
                ringnum,  REG32(RXQ), REG32(STS), REG32(DISABLE_STS_DONE));
    } else {
        ipd_info("RXQ enable succeeded for queue %d\n", ringnum);
    }

    rxq_pidx.flds.num_f = RING_IDX_COMPL(0);
    REG32(RXQ_x_DESC_PIDX(ringnum)) = rxq_pidx.data;

    return 0;
}

/* refill_ring refills all descriptors in a given ring */
static void
refill_ring(inno_device_t  *idev,
            int            ring_num)
{
    uint32_t pidx,cidx;
    inno_ring_t *rx = &idev->rx_ring[ring_num];
    pidx = REG32(RXQ_x_DESC_PIDX(ring_num));
    cidx = REG32(RXQ_x_DESC_CIDX(ring_num));
    /* Now allocate new pages for the next packets */
    if ((rx->flags & INNO_RING_INIT)){
        while (pidx != RING_IDX_COMPL(cidx)) {
            rxq_0_desc_pidx_t rxq_pidx;
            inno_dma_alloc_t  *dma = &rx->pages[RING_IDX_MASKED(pidx)];
            /* RX descriptors are allocated only once. So we should be able to recycle it.
             * For some reason if it is not allocated we allocate it. */
            if (dma->page == NULL) {
                dma->page = alloc_page(__GFP_HIGHMEM);
                if (dma->page == NULL) {
                    /* We might run out or we will get more on the next rupt */
                    return;
                }
                dma->dma_addr = dma_map_page(&idev->pdev->dev, dma->page, 0,
                        PAGE_SIZE, DMA_FROM_DEVICE);

                if (dma_mapping_error(&idev->pdev->dev, dma->dma_addr)) {
                    ipd_err("%s dma_map_page err ring_num:%d offset:%u len:%u \n", __func__,
                            ring_num, 0, (unsigned)PAGE_SIZE);
                    return;
                }
                dma->vmaddr = page_address(dma->page);
                smp_mb();
                rx->rx_desc[RING_IDX_MASKED(pidx)].hsn_upper =
                    (uint32_t)(dma->dma_addr >> 32);
                rx->rx_desc[RING_IDX_MASKED(pidx)].hsn_lower =
                    (uint32_t)(dma->dma_addr & 0x00000000ffffffff);
            }
            RING_IDX_INCR(rx->count, pidx);
            rxq_pidx.flds.num_f = pidx;
            REG32(RXQ_x_DESC_PIDX(ring_num)) = rxq_pidx.data;
        }
    }
}

/** @brief Rx queue disable function
 *
 *  @param [in] idev - innovium device
 *  @param [in] ringnum - Rx ring number
 *  @return 0 for success and -1 for error
 */
static int
inno_disable_queue_rx(inno_device_t  *idev,
                      int            rx_ringnum)
{
    int rc=-1, i=0, dm_status, status;
    int ring_num = rx_ringnum + RX_RING_OFFSET;
    int is_switch_ring = (rx_ringnum < NUM_RX_SWITCH_RINGS);
    uint32_t reg;

    /* Step 1 - Disable RXQ only if DM is in enabled state */
    reg  = REG32(DM_STS);
    dm_status = (reg >> ring_num) & 1;
    reg  = REG32(STS);
    status = (reg >> ring_num) & 1;
    ipd_debug("RXQ status:%d dm_status:%d for queue %d\n", status, dm_status,
              rx_ringnum);
    if(dm_status == 0){
        ipd_debug("RXQ already in disable state for queue %d\n", rx_ringnum);
        return 0;
    } else {
        ipd_info("RXQ ring %d disable start: RXQ = 0x%x, STS = 0x%x, DM_STS = 0x%x, DISABLE_STS_DONE = 0x%x PIDX=0x%x, CIDX=0x%x\n",
                 rx_ringnum, REG32(RXQ), REG32(STS), REG32(DM_STS), REG32(DISABLE_STS_DONE),
                 REG32(RXQ_x_DESC_PIDX(rx_ringnum)), REG32(RXQ_x_DESC_CIDX(rx_ringnum)));
    }

    /* instruct the DTM to stop sending traffic for this ring */
    if (is_switch_ring) {

        reg = REG32(UCM_DTM);
        reg |= (0x1 << rx_ringnum);
        reg |= (0x1 << (rx_ringnum + 6));
        REG32(UCM_DTM) = reg;

        /* Ensure SBUF is empty before ring disable */
        reg = REG32(SBUF_SWITCH_TO_CPU_x_PKT(rx_ringnum));
        if(reg != 0) {
            ipd_info("SBUF_SWITCH_TO_CPU_%d_PKT is non zero(%d). ring %d.\n", rx_ringnum, reg, rx_ringnum);
            ipd_info("Ring %d: PIDX = 0x%x CIDX = 0x%x\n", rx_ringnum, REG32(RXQ_x_DESC_PIDX(rx_ringnum)), REG32(RXQ_x_DESC_CIDX(rx_ringnum)));

            refill_ring(idev, rx_ringnum);

            for(i=0;i<1000;i++) {
                if(REG32(SBUF_SWITCH_TO_CPU_x_PKT(rx_ringnum)) == 0) {
                    break;
                }
                mdelay(1);
            }

            reg = REG32(SBUF_SWITCH_TO_CPU_x_PKT(rx_ringnum));
            if(reg != 0 ) {
                ipd_err("SBUF_SWITCH_TO_CPU_%d_PKT is non zero(%d). ring %d.\n", rx_ringnum, reg, rx_ringnum);
                ipd_info("Ring %d: PIDX = 0x%x CIDX = 0x%x\n", rx_ringnum, REG32(RXQ_x_DESC_PIDX(rx_ringnum)), REG32(RXQ_x_DESC_CIDX(rx_ringnum)));
            }
        }
    }
    /* Step 2 - Disable DM */
    reg  = REG32(RXQ);
    
    reg &= ~(0x1 << (rx_ringnum * 4));
    REG32(RXQ) = reg;

    /* Step 3 - Wait for DM disable status to be done */

    rc=-1;
    for(i = 0; i < RING_STS_RETRY_CNT; i++) {
        uint32_t reg, dm_dis_sts_done;

        reg = REG32(DISABLE_STS_DONE);
        
        dm_dis_sts_done = (reg >> (ring_num*2)) & 1;

        if(dm_dis_sts_done == 1) {
            rc = 0;
            break;
        }
        udelay(10);
    }

    if(rc == 0) {
        uint32_t reg, sts;
        
        reg = REG32(DM_STS);
        sts = (reg >> ring_num) & 1;
        if (sts != 0) {
            rc = -1;
        }
    }

    if(rc) {
        ipd_err("DM disable failed for Rx queue %d RXQ = 0x%x, STS = 0x%x, DISABLE_STS_DONE = 0x%x, DM_STS=0x%x\n",
                rx_ringnum,  REG32(RXQ), REG32(STS), REG32(DISABLE_STS_DONE), REG32(DM_STS));
    } else {
        ipd_info("RXQ disable succeeded for queue %d\n", rx_ringnum);
    }

    if (is_switch_ring) {
        reg = REG32(SBUF_SWITCH_TO_CPU_x_PKT(rx_ringnum));
        if(reg != 0) {
            ipd_err("SBUF_SWITCH_TO_CPU_%d_PKT is non zero(%d). ring %d.\n", rx_ringnum, reg, rx_ringnum);
        }
    }

    return 0;
}

/** @brief Ring allocation IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @param [io] ioctl - ioctl data area
 *  @return ERRNO
 */
static int
inno_alloc_ring(inno_device_t     *idev,
                inno_ioctl_ring_t *ioctl)
{
    inno_ring_t   *ring;
    struct device *dev = NULL;
    int           ring_size;
    int           rc = 0;
    int           msix_vector;
    int           rupt_bit;

    if (ioctl->flags & INNO_RING_TX) {
        if (ioctl->num >= NUM_TX_RINGS) {
            ipd_err("TX Ring number too large");
            return -EINVAL;
        }
        ring      = &idev->tx_ring[ioctl->num];
        ring_size = sizeof(inno_tx_desc_t) * ioctl->count;
    } else {
        if (ioctl->num >= NUM_RX_RINGS) {
            ipd_err("RX Ring number too large");
            return -EINVAL;
        }
        ring      = &idev->rx_ring[ioctl->num];
        ring_size = sizeof(inno_rx_desc_t) * ioctl->count;
    }

    if (ring->flags & INNO_RING_INIT) {
        ipd_err("Ring already intialized");
        goto fill;
    }
   ipd_info("Ring size: %u", ioctl->count);

    memset(ring, 0, sizeof(inno_ring_t));

    ring->num = ioctl->num;

    if (ioctl->flags & INNO_RING_NETDEV) {
        ring->flags |= INNO_RING_NETDEV;
        dev          = &idev->pdev->dev;
        if (ioctl->flags & INNO_RING_TX) {
            idev->enet_tx_ring_num = ring->num;
            idev->napi_mask[0] |= 1 << inno_txq_int[ring->num];
        } else {
            idev->napi_mask[0] |= 1 << inno_rxq_int[ring->num];
        }
        msix_vector = MSIX_VECTOR_NAPI;
    } else {
        msix_vector = ioctl->vector;
    }

    if (ioctl->flags & INNO_RING_TX) {
        ring->flags     |= INNO_RING_TX;
        ring->idx_offset = POOL_TX_IDX_OFFSET(ring->num);
	    ipd_debug("TX_IDX_OFFSET(%d): %d cache_align(4): %d\n", ring->num, POOL_TX_IDX_OFFSET(ring->num), CACHE_ALIGN(4));
    } else {
        ring->idx_offset = POOL_RX_IDX_OFFSET(ring->num);
	    ipd_debug("RX_IDX_OFFSET(%d): %d cache_align(4): %d\n", ring->num, POOL_RX_IDX_OFFSET(ring->num), CACHE_ALIGN(4));
    }

    ring->count = ioctl->count;

    /* Allocate the ring space (TX or RX based on desc_size) */
    ring->rx_desc = (void *)dma_alloc_coherent(&idev->pdev->dev, ring_size,
                                               &ring->desc_ba, GFP_KERNEL);
    if (ring->rx_desc == NULL) {
        ipd_err("Descriptor ring alloc Failed \n");
        return -ENOMEM;
    }

    if (ioctl->flags & INNO_RING_TX) {
        /* Allocate the tx checksum desc */
        ring->tx_cksum = (void *)dma_alloc_coherent(&idev->pdev->dev, ioctl->count * MIN_PACKET_SIZE,
                &ring->tx_cksum_ba, GFP_KERNEL);
        if (ring->tx_cksum == NULL) {
            ipd_err("TX checksum descriptor alloc failed \n");
            dma_free_coherent(dev, ring_size,
                    ring->rx_desc, ring->desc_ba);
            return -ENOMEM;
        }
    }

    /* Allocate the wb space (TX or RX based on desc_size) */
    ring->rx_wb = (void *)dma_alloc_coherent(&idev->pdev->dev, ring_size,
                                             &ring->wb_ba, GFP_KERNEL);
    if (ring->rx_wb == NULL) {
        ipd_err("Writeback ring alloc Failed \n");
        dma_free_coherent(dev, ring_size,
                ring->rx_desc, ring->desc_ba);
        if (ioctl->flags & INNO_RING_TX) {
            dma_free_coherent(dev, ioctl->count * MIN_PACKET_SIZE,
                    ring->tx_cksum, ring->tx_cksum_ba);
        }
        return -ENOMEM;
    }

    memset(ring->rx_wb, 0, ring_size);

    if (ioctl->flags & INNO_RING_NETDEV) {
        /* We allocated the kernel version of the descriptor info */
        if (ring->desc_info != NULL) {
            kfree(ring->desc_info);
        }
        ring->desc_info = kmalloc(ring->count * sizeof(inno_ring_desc_info_t), GFP_KERNEL);
        if (ring->desc_info == NULL) {
            ipd_err("RING desc info alloc Failed \n");
            dma_free_coherent(dev, ring_size,
                    ring->rx_wb, ring->wb_ba);
            dma_free_coherent(dev, ring_size,
                    ring->rx_desc, ring->desc_ba);
            if (ioctl->flags & INNO_RING_TX) {
                dma_free_coherent(dev, ioctl->count * MIN_PACKET_SIZE,
                        ring->tx_cksum, ring->tx_cksum_ba);
            }
            return -ENOMEM;
        }

        ring->cidx_addr = (uint32_t *) (idev->pool + ring->idx_offset);

        memset(ring->desc_info, 0, ring->count * sizeof(inno_ring_desc_info_t));
    }

    if (ring->pages == NULL) {
        ring->pages = (inno_dma_alloc_t *)
                          kmalloc(ring->count * sizeof(inno_dma_alloc_t),
                                  GFP_KERNEL);

        if (ring->pages == NULL) {
            return -ENOMEM;
        }

        memset(ring->pages, 0, ring->count * sizeof(inno_dma_alloc_t));
    }

    ring->idev = idev;

    /* Set up the H/W for the ring */
    if (ring->flags & INNO_RING_TX) {
        txq_0_t                       txq_reg                       = {{0}};
        txq_0_sch_t                   txq_sch                       = {{0}};
        txq_0_cidx_update_t           txq_cidx_update_reg           = {{0}};
        txq_0_cidx_update_cliff_t     txq_cidx_update_cliff_reg     = {{0}};
        txq_0_cidx_update_prescaler_t txq_cidx_update_prescaler_reg = {{0}};
        txq_0_cidx_update_precliff_t  txq_cidx_update_precliff_reg  = {{0}};
        txq_0_cidx_update_tmr_ctl_t   txq_cidx_update_tmr_ctl_reg   = {{0}};

        rupt_bit = inno_txq_int[ring->num];

        /* Disable this function (dm and txe). */
        if(inno_disable_queue_tx(idev, ring->num) < 0) {
            ipd_err("inno_disable_queue_tx failed for Tx ring number %d", ring->num);
            rc = -ETIME;
        }

        REG32(TXQ_x_DESC_RING(ring->num)) = ring->count;

        /* Set up the address pointers for PCIe ops */
        REG32(TXQ_x_BASE_HSN_HI(ring->num)) = ring->desc_ba >> 32;
        REG32(TXQ_x_BASE_HSN_LO(ring->num)) = ring->desc_ba & 0xffffffff;

        REG32(TXQ_x_BASE_HSN_HI_WB(ring->num)) = ring->wb_ba >> 32;
        REG32(TXQ_x_BASE_HSN_LO_WB(ring->num)) = ring->wb_ba & 0xffffffff;

        /* Set the PIDX to zero */
        ring->pidx = 0;

        /* Set the writeback area to zero */
        *((volatile uint32_t *) (idev->pool + ring->idx_offset)) = 0;
        smp_mb();

        ring->cidx = 0;                     /* Reset will set this to zero */

        REG32(TXQ_x_CIDX_WB_HSN_HI(ring->num)) =
            (idev->pool_ba + ring->idx_offset) >> 32;
        REG32(TXQ_x_CIDX_WB_HSN_LO(ring->num)) =
            (idev->pool_ba + ring->idx_offset) & 0xffffffff;

        txq_cidx_update_reg.flds.timer_f    = 0;
        REG32(TXQ_x_CIDX_UPDATE(ring->num)) =
            txq_cidx_update_reg.data;
        txq_cidx_update_cliff_reg.flds.timer_f    = 0;
        REG32(TXQ_x_CIDX_UPDATE_CLIFF(ring->num)) =
            txq_cidx_update_cliff_reg.data;
        txq_cidx_update_prescaler_reg.flds.timer_f    = 0;
        REG32(TXQ_x_CIDX_UPDATE_PRESCALER(ring->num)) =
            txq_cidx_update_prescaler_reg.data;
        txq_cidx_update_precliff_reg.flds.timer_f    = 0;
        REG32(TXQ_x_CIDX_UPDATE_PRECLIFF(ring->num)) =
            txq_cidx_update_precliff_reg.data;
        txq_cidx_update_tmr_ctl_reg.flds.enable_f = 1;

        
        REG32(TXQ_x_CIDX_UPDATE_TMR_CTL(ring->num)) =
            txq_cidx_update_tmr_ctl_reg.data;

        txq_reg.flds.cidx_wb_thres_f           = 1;
        txq_reg.flds.cidx_intr_cnt_coalse_en_f = 1;
        txq_reg.flds.dma_type_f = 1;
        txq_reg.flds.chnl_map_f = 0;
        REG32(TXQ_x(ring->num)) = txq_reg.data;

        /* set the scheduler for this ring */
        txq_sch.flds.weight_f = 5;
        txq_sch.flds.strict_priority_f = 0;
        REG32(TXQ_x_SCH(ring->num)) = txq_sch.data;

        /* Enable TXQ */
        if (ioctl->flags & INNO_RING_NETDEV) {
            /* For rings managed in the user space
             * call to enable happens at a later time
             */
            inno_enable_queue_tx(idev, ring->num);
        }
    } else {
        rxq_0_t                       rxq_reg                       = {{0}};
        rxq_0_sch_t                   rxq_sch                       = {{0}};
        rxq_0_cidx_update_t           rxq_cidx_update_reg           = {{0}};
        rxq_0_cidx_update_cliff_t     rxq_cidx_update_cliff_reg     = {{0}};
        rxq_0_cidx_update_prescaler_t rxq_cidx_update_prescaler_reg = {{0}};
        rxq_0_cidx_update_precliff_t  rxq_cidx_update_precliff_reg  = {{0}};
        rxq_0_cidx_update_tmr_ctl_t   rxq_cidx_update_tmr_ctl_reg   = {{0}};

        rupt_bit = inno_rxq_int[ring->num];

        /* Disable this function (dm and rxe). */
        if(inno_disable_queue_rx(idev, ring->num) < 0) {
            ipd_err("inno_disable_queue_rx failed for Rx ring number %d", ring->num);
            rc = -ETIME;
        }

        rxq_cidx_update_reg.flds.timer_f    = 0;
        REG32(RXQ_x_CIDX_UPDATE(ring->num)) =
            rxq_cidx_update_reg.data;
        rxq_cidx_update_cliff_reg.flds.timer_f    = 0;
        REG32(RXQ_x_CIDX_UPDATE_CLIFF(ring->num)) =
            rxq_cidx_update_cliff_reg.data;
        rxq_cidx_update_prescaler_reg.flds.timer_f    = 0;
        REG32(RXQ_x_CIDX_UPDATE_PRESCALER(ring->num)) =
            rxq_cidx_update_prescaler_reg.data;
        rxq_cidx_update_precliff_reg.flds.timer_f    = 0;
        REG32(RXQ_x_CIDX_UPDATE_PRECLIFF(ring->num)) =
            rxq_cidx_update_precliff_reg.data;
        rxq_cidx_update_tmr_ctl_reg.flds.enable_f = 1;



        REG32(RXQ_x_CIDX_UPDATE_TMR_CTL(ring->num)) =
            rxq_cidx_update_tmr_ctl_reg.data;
        rxq_reg.flds.cidx_wb_thres_f           = 1;
        rxq_reg.flds.cidx_intr_cnt_coalse_en_f = 1;
        rxq_reg.flds.dma_type_f = 1;
        rxq_reg.flds.chnl_map_f = 0;
        REG32(RXQ_x(ring->num)) = rxq_reg.data;

        REG32(RXQ_x_BASE_HSN_HI(ring->num))    = ring->desc_ba >> 32;
        REG32(RXQ_x_BASE_HSN_LO(ring->num))    = ring->desc_ba & 0xffffffff;
        REG32(RXQ_x_BASE_HSN_HI_WB(ring->num)) = ring->wb_ba >> 32;
        REG32(RXQ_x_BASE_HSN_LO_WB(ring->num)) = ring->wb_ba & 0xffffffff;

        /* Allocate pages and program the descriptors */
        if (ioctl->flags & INNO_RING_NETDEV) {
            uint32_t i;
            for (i = 0; i < ioctl->count; i++ ) {
                inno_dma_alloc_t  *dma = &ring->pages[i];
                dma->page = alloc_page(__GFP_HIGHMEM);
                if (dma->page == NULL) {
                    /* We might run out or we will get more on the next rupt */
                    ipd_crit("Unable to allocate RX page memory \
                           ring: %d, index: %d\n", ioctl->num, i);
                    break;
                }
                dma->dma_addr = dma_map_page(&idev->pdev->dev, dma->page, 0,
                                             PAGE_SIZE, DMA_FROM_DEVICE);
                if (dma_mapping_error(&idev->pdev->dev, dma->dma_addr)) {
                    ipd_err("%s dma_map err i:%d offset:%u len:%u \n", __func__,
                            i, 0, (unsigned)PAGE_SIZE);
                    return -1;
                }
                dma->vmaddr = page_address(dma->page);
                smp_mb();
                ring->rx_desc[i].hsn_upper = (uint32_t)(dma->dma_addr >> 32);
                ring->rx_desc[i].hsn_lower = (uint32_t)(dma->dma_addr & 0x00000000ffffffff);
                smp_mb();
                ipd_verbose("Rx pkt addr: upper: 0x%x lower: 0x%x\n", ring->rx_desc[i].hsn_upper, ring->rx_desc[i].hsn_lower);
            }
        }

        /* Set the PIDX to zero */
        ring->pidx = RING_IDX_COMPL(0);

        /* Set the writeback area to zero */
        *((volatile uint32_t *) (idev->pool + ring->idx_offset)) = 0;
        smp_mb();

        ring->cidx = 0;                     /* Reset will set this to zero */

        REG32(RXQ_x_CIDX_WB_HSN_HI(ring->num)) =
            (idev->pool_ba + ring->idx_offset) >> 32;
        REG32(RXQ_x_CIDX_WB_HSN_LO(ring->num)) =
            (idev->pool_ba + ring->idx_offset) & 0xffffffff;
        REG32(RXQ_x_DESC_RING(ring->num)) = ring->count;

        /* Set the scheduler for this ring */
        rxq_sch.flds.weight_f = 5;
        rxq_sch.flds.strict_priority_f = 0;
        REG32(RXQ_x_SCH(ring->num)) = rxq_sch.data;

        /* Enable RXQ */
        if (ioctl->flags & INNO_RING_NETDEV) {
            /* For rings managed in the user space
             * call to enable happens at a later time
             */
            inno_enable_queue_rx(idev, ring->num);
        }
    }

    /* Steer the rupt to this MSIX vector */
    REG32(idev->inno_intr_regs.intr_immr + rupt_bit * 4) = msix_vector;

    /* Enable the interrupts for the updated interrupt mask */
    REG32(idev->inno_intr_regs.intr_inms) = idev->napi_mask[0];

    ring->flags |= INNO_RING_INIT;


fill:
    /* Fill in the return data */
    ioctl->ring       = (off_t)ring->desc_ba;
    ioctl->wb         = (off_t)ring->wb_ba;
    ioctl->ring_pa       = (off_t)virt_to_phys(ring->rx_desc);
    ioctl->wb_pa         = (off_t)virt_to_phys(ring->rx_wb);
    ioctl->idx_offset = ring->idx_offset;
    ioctl->cidx       = ring->cidx;
    ioctl->pidx       = ring->pidx;

    ipd_debug("Ring allocated %s%d ring: %p cidx: %x pidx: %x wb: %p info: %p\n",
            (ring->flags & INNO_RING_TX) ? "TX" : "RX", ring->num,
            (void *)ring->desc_ba, ring->cidx, ring->pidx,
            (void *)ring->wb_ba, ring->desc_info);

    return rc;
}


/** @brief Ring free IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @param [io] ioctl - ioctl data area
 *  @return ERRNO
 */
static int
inno_free_ring(inno_device_t *idev,
               int           tx,
               int           num)
{
    inno_ring_t   *ring;
    struct device *dev = NULL;
    int           desc_size;

    if (tx) {
        if (num >= NUM_TX_RINGS) {
            return -EINVAL;
        }
        ring      = &idev->tx_ring[num];
        desc_size = sizeof(inno_tx_desc_t);
    } else {
        if (num >= NUM_RX_RINGS) {
            return -EINVAL;
        }
        ring      = &idev->rx_ring[num];
        desc_size = sizeof(inno_rx_desc_t);
    }

    if ((ring->flags & INNO_RING_INIT) == 0) {
        return -EINVAL;
    }

    /* Disable this function (dm and txe/rxe). */
    if(tx) {
        if(inno_disable_queue_tx(idev, ring->num) < 0) {
            ipd_err("inno_disable_queue_tx failed for %s ring number %d", (tx)?"Tx":"Rx", ring->num);
        }
    }else{
        if(inno_disable_queue_rx(idev, ring->num) < 0) {
            ipd_err("inno_disable_queue_rx failed for %s ring number %d", (tx)?"Tx":"Rx", ring->num);
        }
        REG32(RXQ_x_DESC_PIDX(ring->num)) = 0;
    }

    if (ring->pages != NULL) {
        if (tx) {
            int i;
            /* Unpin TX pages */
            for (i = 0; i < ring->count; i++) {
                if (ring->pages[i].page != NULL) {
                    put_page(ring->pages[i].page);
                    memset(&ring->pages[i], 0, sizeof(inno_dma_alloc_t));
                }
            }
        } else {
            int i;
            /* Release RX buffers */
            for (i = 0; i < ring->count; i++) {
                if (ring->pages[i].dma_addr != 0) {
                    dma_unmap_page(&idev->pdev->dev, ring->pages[i].dma_addr,
                                   PAGE_SIZE, DMA_FROM_DEVICE);
                    __free_page(ring->pages[i].page);
                    memset(&ring->pages[i], 0, sizeof(inno_dma_alloc_t));
                }
            }
        }

        kfree(ring->pages);
        ring->pages = NULL;
    }
    dev = &idev->pdev->dev;

    if (ring->flags & INNO_RING_NETDEV) {
        if (ring->flags & INNO_RING_TX) {
            idev->napi_mask[0] &= inno_txq_int[ring->num];
        } else {
            idev->napi_mask[0] &= inno_rxq_int[ring->num];
        }
    }

    /* Clear the ring enable */

    /* Free the descriptor info.  It has to be page-chunked for mmap
       to work */
    if (ring->desc_info != NULL) {
        kfree(ring->desc_info);
        ring->desc_info = NULL;
    }

    /* Free the writeback  ring */
    if (ring->rx_wb != NULL) {
        dma_free_coherent(dev, desc_size * ring->count,
                          ring->rx_wb, ring->wb_ba);
        ring->rx_wb = NULL;
    }

    /* Free the descriptor ring  */
    if (ring->rx_desc != NULL) {
        dma_free_coherent(dev, desc_size * ring->count,
                          ring->rx_desc, ring->desc_ba);
        ring->rx_desc = NULL;
    }

    /* Free the tx checksum area  */
    if (ring->tx_cksum != NULL) {
        dma_free_coherent(dev, ring->count * MIN_PACKET_SIZE,
                ring->tx_cksum, ring->tx_cksum_ba);
    }

    memset(ring, 0, sizeof(inno_ring_t));

    return 0;
}


/** @brief Host response ring allocation IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @param [io] ioctl - ioctl data area
 *  @return ERRNO
 */
static int
inno_alloc_hrr(inno_device_t    *idev,
               inno_ioctl_hrr_t *ioctl)
{
    hrr_size_t        hrr_size;
    isn_timeout_cfg_t isn_to_cfg = {{0}};
    axi_timeout_cfg_t axi_to_cfg = {{0}};
    uint32_t          pidx;

    if (idev->hrr.vmaddr == 0) {            /* Check to see if already allocated */
        idev->hrr.count        = ioctl->count;
        idev->hrr.size         = ioctl->size;
        idev->hrr.descriptors  = ioctl->descriptors;

        /* Allocate the ring space */
        idev->hrr.vmaddr = (void *)dma_alloc_coherent(&idev->pdev->dev,
                                               idev->hrr.count * idev->hrr.size,
                                               &idev->hrr.ba, GFP_KERNEL);
    }

    if (idev->hrr.vmaddr == NULL) {
        ipd_err("Host response ring alloc Failed \n");
        return -ENOMEM;
    }

    if (idev->hrr.pages == NULL) {
        /* The number of S/W descriptors could be more than the H/W */
        idev->hrr.pages = (inno_dma_alloc_t *)
                                     kmalloc(ioctl->descriptors *
                                             sizeof(inno_dma_alloc_t),
                                             GFP_KERNEL);

        if (idev->hrr.pages == NULL) {
            return -ENOMEM;
        }

        memset(idev->hrr.pages, 0, ioctl->descriptors *
               sizeof(inno_dma_alloc_t));

    }

    if (idev->hrr.pages == NULL) {
        ipd_err("Async FIFO buffer array allocation failed \n");
        /* free up the memory allocated for HRR */
        dma_free_coherent(&idev->pdev->dev, idev->hrr.count * idev->hrr.size,
                          idev->hrr.vmaddr, idev->hrr.ba);
        return -ENOMEM;
    }

    /* Set up the H/W for the HRR */
    REG32(HRR_ADR_LO)         = LOWER32(idev->hrr.ba);
    REG32(HRR_ADR_HI)         = UPPER32(idev->hrr.ba);
    hrr_size.data             = 0;
    hrr_size.flds.slot_num_f  = idev->hrr.count - 1;
    hrr_size.flds.slot_size_f = idev->hrr.size;
    REG32(HRR_SIZE)           = hrr_size.data;
    REG32(HRR_PIDX_ADR_LO)    = LOWER32(idev->pool_ba + POOL_HRR_OFFSET);
    REG32(HRR_PIDX_ADR_HI)    = UPPER32(idev->pool_ba + POOL_HRR_OFFSET);

    /* Initialize the PIDX */
    pidx = REG32(HRR_PIDX);
    *((volatile uint32_t *) (idev->pool + POOL_HRR_OFFSET)) = pidx;

    isn_to_cfg.flds.en_f   = 1;
    isn_to_cfg.flds.val_f  = 0xa000;   /* In ns */
    REG32(ISN_TIMEOUT_CFG) = isn_to_cfg.data;

    axi_to_cfg.flds.en_f = 0;  /* disable timeout altogether */
    axi_to_cfg.flds.val_f = 0x4000000; /* set proper value for silicon if enabled */
    REG32(AXI_TIMEOUT_CFG) = axi_to_cfg.data;

    /* Fill in the return data */
    ioctl->hrr             = (off_t)idev->hrr.ba;
    ioctl->hrr_pa          = (off_t)virt_to_phys(idev->hrr.vmaddr);
    ioctl->hrr_pidx_offset = POOL_HRR_OFFSET;
    ioctl->size            = idev->hrr.count * idev->hrr.size;

    ipd_debug("HRR alloc: %p size: %x slots: %x pidx: 0x%x HRR_OFFSET: %d\n", (void *) idev->hrr.ba, idev->hrr.size, idev->hrr.count, pidx, POOL_HRR_OFFSET);
    return 0;
}


/** @brief Host response ring free IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @return ERRNO
 */
static int
inno_free_hrr(inno_device_t *idev)
{
    if (idev->hrr.vmaddr == NULL) {
        return -EINVAL;
    }

    if (idev->hrr.pages != NULL) {
        int i;
        for (i = 0; i < idev->hrr.descriptors; i++) {
            if (idev->hrr.pages[i].vmaddr != NULL) {
                dma_unmap_page(&idev->pdev->dev, idev->hrr.pages[i].dma_addr,
                               PAGE_SIZE, DMA_BIDIRECTIONAL);
                __free_page(idev->hrr.pages[i].page);
                memset(&idev->hrr.pages[i], 0, sizeof(inno_dma_alloc_t));

            }
        }
        kfree(idev->hrr.pages);
        idev->hrr.pages = NULL;
    }

    /* Clear the HRR enable */

    /* Free the HRR */
    dma_free_coherent(&idev->pdev->dev, idev->hrr.count * idev->hrr.size,
                      idev->hrr.vmaddr, idev->hrr.ba);

    idev->hrr.vmaddr = NULL;

    return 0;
}

static int
inno_port_st_dump(volatile inno_pic_st_wb_t *pic_st_wb)
{
    ipd_info("%2d %3d %4d %5d %3d %2d %6d %9d "
           "%11d %7d %14d %14d %9d %10d %3d %6d",
           pic_st_wb->ib_id,
           pic_st_wb->pic_id,
           pic_st_wb->lane_id,
           pic_st_wb->link_speed,
           pic_st_wb->fec_type,
           pic_st_wb->enabled,
           pic_st_wb->signal_detected,
           pic_st_wb->autoneg_master,
           pic_st_wb->autoneg_training,
           pic_st_wb->link_up,
           pic_st_wb->rx_link_active,
           pic_st_wb->tx_link_active,
           pic_st_wb->local_fault,
           pic_st_wb->remote_fault,
           pic_st_wb->error,
           pic_st_wb->parity);

    return 0;
}

/** @brief PIC Status chain allocation IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @param [io] ioctl - ioctl data area
 *  @return ERRNO
 */
static int
inno_pic_st_alloc(inno_device_t      *idev,
                  inno_ioctl_pic_st_t *ioctl)
{
    inno_pic_st_t *pic_st = &idev->pic_st;

    if(!pic_st->wb_vmaddr) {
        pic_st->count = ioctl->count;

        /* Allocate the space for PIC status chain */
        pic_st->wb_vmaddr = (void *)dma_alloc_coherent(&idev->pdev->dev,
                                                       pic_st->count *
                                                       sizeof(inno_pic_st_wb_t),
                                                       &pic_st->wb_ba,
                                                       GFP_KERNEL);
    }

    if (pic_st->wb_vmaddr == NULL) {
        ipd_err("Writeback PIC status chain alloc Failed \n");
        return -ENOMEM;
    }

    ipd_debug("PIC writeback bus addr %llx", pic_st->wb_ba);

    /* Set up the H/W for the PIC status chain */
    REG32(INTR_PSC_AHI) = pic_st->wb_ba >> 32;
    REG32(INTR_PSC_ALO) = pic_st->wb_ba & 0xffffffff;

    /* Set the timer for status update (in ns) */
    REG32(INTR_PSC_TMR) = 0xf4240; /* 1,000,000 ns or 1 ms */

    /* Send data back to ioctl caller */
    ioctl->wb_ba = pic_st->wb_ba;
    ioctl->wb_pa = (off_t)virt_to_phys(pic_st->wb_vmaddr);

    ipd_debug("PIC Status chain allocated\n");

    return 0;
}

/** @brief PIC status chain free IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @return ERRNO
 */
static int
inno_pic_st_free(inno_device_t *idev)
{
    inno_pic_st_t *pic_st = &idev->pic_st;

    if (pic_st->wb_vmaddr == NULL) {
        return -EINVAL;
    }



    /* Set the timer to the max value (4 seconds) */
    REG32(INTR_PSC_TMR) = 0xffffffff;

    /* Wait for 3 times the initial PSC_TMR value. This can be
     * 1 ms or 2 ms based on the CPU clock frequency. Waiting for
     * the longest duration possible, just in case
     */
    msleep_interruptible(6);

    /* Turn off the  timer for status update */
    REG32(INTR_PSC_TMR) = 0;

    /* Reset the H/W for the PIC status chain */
    REG32(INTR_PSC_AHI) = 0;
    REG32(INTR_PSC_ALO) = 0;

    /* Free the PIC Status chain WB data area */
    dma_free_coherent(&idev->pdev->dev,
                      pic_st->count *
                      sizeof(inno_pic_st_wb_t),
                      (void*)pic_st->wb_vmaddr,
                      pic_st->wb_ba);

    pic_st->wb_vmaddr = NULL;

    ipd_debug("PIC Status chain wb freed\n");

    return 0;
}


/** @brief PIC status chain dump IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @return ERRNO
 */
static int
inno_pic_st_dump(inno_device_t *idev)
{
    inno_pic_st_t *pic_st = &idev->pic_st;
    int i = 0;
    volatile inno_pic_st_wb_t *pic_st_wb =
                        (inno_pic_st_wb_t*) pic_st->wb_vmaddr;

    if (pic_st->wb_vmaddr == NULL) {
        return -EINVAL;
    }

    ipd_info("%.2s %.3s %.4s %.5s %.3s %.2s %.6s %.9s "
           "%.11s %.7s %14s %.14s %.9s %.10s %.3s %.6s",
           "ib",
           "pic",
           "lane",
           "speed",
           "fec",
           "en",
           "signal",
           "an_master",
           "an_training",
           "link_up",
           "rx_link_active",
           "tx_link_active",
           "local_flt",
           "remote_flt",
           "err",
           "parity");

#define IFCS_MAX_SERDES 256
    for (i = 0; i < IFCS_MAX_SERDES; ++i) {
        inno_port_st_dump(pic_st_wb+i);
    }

    ipd_info("PIC Status chain wb dump done\n");

    return 0;
}

static int
inno_hi_watermark_enable(inno_device_t *idev)
{
    hi_watermark_imsg_0_t hi_wmark_imsg;


    if(idev->device_id == INNO_TERALYNX_PCI_DEVICE_ID ||
       idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
        memset(&hi_wmark_imsg, 0, sizeof(hi_wmark_imsg));
        hi_wmark_imsg.tl_flds.en_f = 1;
        REG32(HI_WATERMARK_IMSG_0) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_1) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_2) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_3) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_4) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_5) = hi_wmark_imsg.data;

        if (idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
            REG32(HI_WATERMARK_IMSG_6) = hi_wmark_imsg.data;
            REG32(HI_WATERMARK_IMSG_7) = hi_wmark_imsg.data;
        }
    } else {
        ipd_err("Unknown innovium device in inno_hi_watermark_clear_reset\n");
        return -ENODEV;
    }

    return 0;
}

static int
inno_hi_watermark_clear_reset(inno_device_t *idev, int log_err)
{
    hi_watermark_imsg_0_t hi_wmark_imsg;
    int                   ret = 0;


    if(idev->device_id == INNO_TERALYNX_PCI_DEVICE_ID ||
       idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
        /* Set the clear */
        memset(&hi_wmark_imsg, 0, sizeof(hi_wmark_imsg));
        /* Keep the enable flag on */
        hi_wmark_imsg.tl_flds.en_f = 1;
        hi_wmark_imsg.tl_flds.clr_f = 1;
        REG32(HI_WATERMARK_IMSG_0) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_1) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_2) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_3) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_4) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_5) = hi_wmark_imsg.data;

        if (idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
            REG32(HI_WATERMARK_IMSG_6) = hi_wmark_imsg.data;
            REG32(HI_WATERMARK_IMSG_7) = hi_wmark_imsg.data;
        }

        /* Reset the clear */
        memset(&hi_wmark_imsg, 0, sizeof(hi_wmark_imsg));
        /* Keep the enable flag on */
        hi_wmark_imsg.tl_flds.en_f = 1;
        hi_wmark_imsg.tl_flds.clr_f = 0;
        REG32(HI_WATERMARK_IMSG_0) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_1) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_2) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_3) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_4) = hi_wmark_imsg.data;
        REG32(HI_WATERMARK_IMSG_5) = hi_wmark_imsg.data;

        if (idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
            REG32(HI_WATERMARK_IMSG_6) = hi_wmark_imsg.data;
            REG32(HI_WATERMARK_IMSG_7) = hi_wmark_imsg.data;
        }

        /* Now read the watermarks */
        hi_wmark_imsg.data = REG32(HI_WATERMARK_IMSG_0);
        if (hi_wmark_imsg.tl_flds.lvl_f != 0) {
            if (log_err) {
                ipd_err("HI_WATERMARK_IMSG_0 is 0x%x, lvl_f: %d\n",
                        hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                ret = -EBUSY;
            } else {
                ipd_info("HI_WATERMARK_IMSG_0 is 0x%x, lvl_f: %d\n",
                         hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                return -EBUSY;
            }
        } else {
            ipd_info("HI_WATERMARK_IMSG_0 is 0x%x, lvl_f: %d\n",
                     hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
        }
        hi_wmark_imsg.data = REG32(HI_WATERMARK_IMSG_1);
        if (hi_wmark_imsg.tl_flds.lvl_f != 0) {
            if (log_err) {
                ipd_err("HI_WATERMARK_IMSG_1 is 0x%x, lvl_f: %d\n",
                        hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                ret = -EBUSY;
            } else {
                ipd_info("HI_WATERMARK_IMSG_1 is 0x%x, lvl_f: %d\n",
                         hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                return -EBUSY;
            }
        } else {
            ipd_info("HI_WATERMARK_IMSG_1 is 0x%x, lvl_f: %d\n",
                     hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
        }
        hi_wmark_imsg.data = REG32(HI_WATERMARK_IMSG_2);
        if (hi_wmark_imsg.tl_flds.lvl_f != 0) {
            if (log_err) {
                ipd_err("HI_WATERMARK_IMSG_2 is 0x%x, lvl_f: %d\n",
                        hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                ret = -EBUSY;
            } else {
                ipd_info("HI_WATERMARK_IMSG_2 is 0x%x, lvl_f: %d\n",
                          hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                return -EBUSY;
            }
        } else {
            ipd_info("HI_WATERMARK_IMSG_2 is 0x%x, lvl_f: %d\n",
                      hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
        }
        hi_wmark_imsg.data = REG32(HI_WATERMARK_IMSG_3);
        if (hi_wmark_imsg.tl_flds.lvl_f != 0) {
            if (log_err) {
                ipd_err("HI_WATERMARK_IMSG_3 is 0x%x, lvl_f: %d\n",
                         hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                ret = -EBUSY;
            } else {
                ipd_info("HI_WATERMARK_IMSG_3 is 0x%x, lvl_f: %d\n",
                         hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                return -EBUSY;
            }
        } else {
            ipd_info("HI_WATERMARK_IMSG_3 is 0x%x, lvl_f: %d\n",
                     hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
        }
        hi_wmark_imsg.data = REG32(HI_WATERMARK_IMSG_4);
        if (hi_wmark_imsg.tl_flds.lvl_f != 0) {
            if (log_err) {
                ipd_err("HI_WATERMARK_IMSG_4 is 0x%x, lvl_f: %d\n",
                         hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                ret = -EBUSY;
            } else {
                ipd_info("HI_WATERMARK_IMSG_4 is 0x%x, lvl_f: %d\n",
                         hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                return -EBUSY;
            }
        } else {
            ipd_info("HI_WATERMARK_IMSG_4 is 0x%x, lvl_f: %d\n",
                     hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
        }
        hi_wmark_imsg.data = REG32(HI_WATERMARK_IMSG_5);
        if (hi_wmark_imsg.tl_flds.lvl_f != 0) {
            if (log_err) {
                ipd_err("HI_WATERMARK_IMSG_5 is 0x%x, lvl_f: %d\n",
                         hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                ret = -EBUSY;
            } else {
                ipd_info("HI_WATERMARK_IMSG_5 is 0x%x, lvl_f: %d\n",
                         hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                return -EBUSY;
            }
        } else {
            ipd_info("HI_WATERMARK_IMSG_5 is 0x%x, lvl_f: %d\n",
                     hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
        }

        if (idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
            hi_wmark_imsg.data = REG32(HI_WATERMARK_IMSG_6);
            if (hi_wmark_imsg.tl_flds.lvl_f != 0) {
                if (log_err) {
                    ipd_err("HI_WATERMARK_IMSG_6 is 0x%x, lvl_f: %d\n",
                            hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                    ret = -EBUSY;
                } else {
                    ipd_info("HI_WATERMARK_IMSG_6 is 0x%x, lvl_f: %d\n",
                            hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                    return -EBUSY;
                }
            } else {
                ipd_info("HI_WATERMARK_IMSG_6 is 0x%x, lvl_f: %d\n",
                        hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
            }
            hi_wmark_imsg.data = REG32(HI_WATERMARK_IMSG_7);
            if (hi_wmark_imsg.tl_flds.lvl_f != 0) {
                if (log_err) {
                    ipd_err("HI_WATERMARK_IMSG_7 is 0x%x, lvl_f: %d\n",
                            hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                    ret = -EBUSY;
                } else {
                    ipd_info("HI_WATERMARK_IMSG_7 is 0x%x, lvl_f: %d\n",
                            hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
                    return -EBUSY;
                }
            } else {
                ipd_info("HI_WATERMARK_IMSG_7 is 0x%x, lvl_f: %d\n",
                        hi_wmark_imsg.data, hi_wmark_imsg.tl_flds.lvl_f);
            }
        }
    } else {
        ipd_err("Unknown innovium device in inno_hi_watermark_clear_reset\n");
        return -ENODEV;
    }

    return ret;
}


/** @brief Learn ring allocation IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @param [io] ioctl - ioctl data area
 *  @return ERRNO
 */
static int
inno_alloc_learn(inno_device_t      *idev,
                 inno_ioctl_learn_t *ioctl)
{
    rxe_dma_msg_size_0_t learn_size;
    inno_learn_t         *learn = &idev->learn;
    blk_and_msg_rxe_t    msg_rxe;
    uint32_t             retry_cnt = 0;
    uint32_t             learn_status = 0;
    int                  ret = 0;
    uint32_t             msg_desc_cidx, msg_desc_pidx;
    uint32_t             msg_data_cidx, msg_data_pidx;

    learn->count = ioctl->count;

    if (learn->flags & INNO_LEARN_INIT) {
        ipd_err("Learn ring already intialized");
        goto fill;
    }


    if (!learn->vmaddr) {          /* Check to see if allocated */
        /* Allocate twice the ring space */
        learn->vmaddr = (void *)dma_alloc_coherent(&idev->pdev->dev,
                                                    learn->count * 2 *
                                                    sizeof(inno_learn_data_t),
                                                    &learn->ba,
                                                    GFP_KERNEL);
        ipd_debug("Learn: allocated %d bytes for learn->ba\n", (unsigned int)(learn->count*sizeof(inno_learn_data_t)));
    }

    if (learn->vmaddr == NULL) {
        ipd_err("learn ring alloc Failed \n");
        return -ENOMEM;
    }

    if (!learn->wb_vmaddr) {    /* Allocate the wb space ( based on desc_size) */
        learn->wb_vmaddr = (void *)dma_alloc_coherent(&idev->pdev->dev,
                                                     learn->count *
                                                     sizeof(inno_learn_wb_t),
                                                     &learn->wb_ba,
                                                     GFP_KERNEL);
    }

    if (learn->wb_vmaddr == NULL) {
        ipd_err("Writeback ring alloc Failed \n");
        dma_free_coherent(&idev->pdev->dev,
                          learn->count * 2 * sizeof(inno_learn_data_t),
                          learn->vmaddr,
                          learn->ba);
        return -ENOMEM;
    }

    /* Get the LEARN PIDX area (after the ring IDX areas) */
    learn->pidx_offset = POOL_LEARN_OFFSET;

    /* Save the base address physical address */
    learn->orig_ba = learn->ba;
    /* Set up the H/W for the LEARN */
    /* Allocated twice the learn size earlier, now check if
     * there is a possiblility of a rollover
     */
    if ( (learn->ba & 0xffffffff) >
         ( (learn->ba + learn->count * sizeof(inno_learn_data_t) ) & 0xffffffff) ) {

        learn->ba = learn->ba + (learn->count * sizeof(inno_learn_data_t));
    }

    REG32(RXE_DMA_MSG_DATA_BASE_HI_0) = learn->ba >> 32;
    REG32(RXE_DMA_MSG_DATA_BASE_LO_0) = learn->ba & 0xffffffff;
    REG32(RXE_DMA_MSG_DESC_BASE_HI_0) = learn->wb_ba >> 32;
    REG32(RXE_DMA_MSG_DESC_BASE_LO_0) = learn->wb_ba & 0xffffffff;
    /* Device requirement: round to 512 bytes
     * rounded to the nearest double word.
     * Ring size is in double words
     */
    learn_size.flds.data_ring_size_f =
        (((learn->count * sizeof(inno_learn_data_t) + 511)/512) * 512) >> 3;
    learn_size.flds.desc_ring_size_f = learn->count;
    ipd_debug("Learn->count: %d learn_size: %d\n",
		      learn_size.flds.desc_ring_size_f, learn_size.flds.data_ring_size_f);
    REG32(RXE_DMA_MSG_SIZE_0) = learn_size.data;
    REG32(RXE_DMA_MSG_PIDX_WB_BASE_HI_0) = UPPER32(idev->pool_ba + POOL_LEARN_OFFSET);
    REG32(RXE_DMA_MSG_PIDX_WB_BASE_LO_0) = LOWER32(idev->pool_ba + POOL_LEARN_OFFSET);

    /* Capture the descriptor values */
    msg_desc_cidx = REG32(RXE_DMA_MSG_DESC_CIDX_0);
    msg_desc_pidx = REG32(RXE_DMA_MSG_DESC_PIDX_0);
    msg_data_cidx = REG32(RXE_DMA_MSG_DATA_CIDX_0);
    msg_data_pidx = REG32(RXE_DMA_MSG_DATA_PIDX_0);

    /* Set the data cidx to data pidx */
    REG32(RXE_DMA_MSG_DATA_CIDX_0) = REG32(RXE_DMA_MSG_DATA_PIDX_0);

    /* read the device's learn PIDX and update WB area */
    learn->pidx.data = REG32(RXE_DMA_MSG_DESC_PIDX_0);
    *((volatile uint32_t *) (idev->pool + POOL_LEARN_OFFSET)) = learn->pidx.data;
    smp_mb();

    learn->cidx.data = learn->pidx.data;
    REG32(RXE_DMA_MSG_DESC_CIDX_0) = learn->cidx.data;
    ipd_debug("Setting Learn MSG CIDX to 0x%.8x PIDX is 0x%.8x\n",
              learn->cidx.data, REG32(RXE_DMA_MSG_DESC_PIDX_0));

    /* Now enable learn */
    msg_rxe.data = REG32(BLK_AND_MSG_RXE);
    /* Disable the reset first */
    msg_rxe.flds.msg_rstn_0_f = 0;
    REG32(BLK_AND_MSG_RXE) = msg_rxe.data;

    /* capture the CPU learn fifo watermarks */
    inno_hi_watermark_enable(idev);
    ret = inno_hi_watermark_clear_reset(idev, 1); /* log an error */
    if (ret < 0) {
        ipd_err("pidx/cidx values before the cleanup attempt: "
                "msg_desc_cidx: 0x%.8x msg_desc_pidx: 0x%.8x "
                "msg_data_cidx: 0x%.8x msg_data_pidx: 0x%.8x\n",
                msg_desc_cidx, msg_desc_pidx, msg_data_cidx, msg_data_pidx);
    }

    /* Perform a reset and enable */
    msg_rxe.flds.msg_rstn_0_f = 1;
    msg_rxe.flds.msg_en_0_f = 1;
    REG32(BLK_AND_MSG_RXE) = msg_rxe.data;

    /* Check for the learn status */
    while (retry_cnt < LEARN_STATUS_LOOP_RETRY_CNT_MAX) {
        learn_status = REG32(RXE_DMA_MSG_STATUS_0);
        if (learn_status) {
            break;
        }
        msleep_interruptible(1);
        retry_cnt++;
    }

    if (learn_status == 0) {
        ipd_err("Learn Status is still 0, but proceeding\n");
    }

    learn->flags |= INNO_LEARN_INIT;

fill:
    /* read the device's learn CIDX */
    learn->cidx.data = REG32(RXE_DMA_MSG_DESC_CIDX_0);
    /* Fill in the return data */
    ioctl->learn_ba          = (off_t)idev->learn.ba;
    ioctl->learn_pa          = (off_t)virt_to_phys(learn->vmaddr);
    ioctl->pidx_offset       = learn->pidx_offset;
    ioctl->wb_ba             = learn->wb_ba;
    ioctl->wb_pa             = (off_t)virt_to_phys(learn->wb_vmaddr);
    ioctl->pidx              = learn->pidx.data;
    ioctl->cidx              = learn->cidx.data;

    ipd_debug("LEARN allocated: %p %p size: 0x%x pidx: 0x%x cidx: 0x%x LEARN_OFFSET: %d pool_ba: 0x%llx\n",
		(void *)ioctl->learn_ba, (void *)ioctl->wb_ba, learn_size.flds.data_ring_size_f, ioctl->pidx, ioctl->cidx, POOL_LEARN_OFFSET, idev->pool_ba);

    return 0;
}

/** @brief Learn ring free IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @return ERRNO
 */
static int
inno_free_learn(inno_device_t *idev)
{
    inno_learn_t      *learn = &idev->learn;
    blk_and_msg_rxe_t msg_rxe;
    uint32_t          retry_cnt = 0;
    uint32_t          learn_status = 0;
    int               ret = 0;
    uint32_t          msg_desc_cidx, msg_desc_pidx;
    uint32_t          msg_data_cidx, msg_data_pidx;

    if (learn->vmaddr == NULL) {
        return -EINVAL;
    }

    /* Instruct the IFC not to send any more learns */
    REG32(LEARN_MSG) = 0x0;
    if (idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
        /* A new register was added in TL10 which
         * controls the flow of learns from IFC to
         * CPU subdomain for IBs 4-7
         */
        REG32(LEARN_MSG_1) = 0x0;
    }

    /* Capture the descriptor values */
    msg_desc_cidx = REG32(RXE_DMA_MSG_DESC_CIDX_0);
    msg_desc_pidx = REG32(RXE_DMA_MSG_DESC_PIDX_0);
    msg_data_cidx = REG32(RXE_DMA_MSG_DATA_CIDX_0);
    msg_data_pidx = REG32(RXE_DMA_MSG_DATA_PIDX_0);

    /* Drain the existing learn DMA entries */
    REG32(RXE_DMA_MSG_DESC_CIDX_0) = REG32(RXE_DMA_MSG_DESC_PIDX_0);
    REG32(RXE_DMA_MSG_DATA_CIDX_0) = REG32(RXE_DMA_MSG_DATA_PIDX_0);

    /* Wait for 10 msec before disabling Learn
     * This is enough time for CPU to drain
     * all the pending Learn messages in Learn FIFO
     */
    msleep_interruptible(10);

    /* Drain the existing learn DMA entries, again */
    REG32(RXE_DMA_MSG_DESC_CIDX_0) = REG32(RXE_DMA_MSG_DESC_PIDX_0);
    REG32(RXE_DMA_MSG_DATA_CIDX_0) = REG32(RXE_DMA_MSG_DATA_PIDX_0);

    ipd_debug("Learn_msg set to 0, DESC CIDX to 0x%.8x DATA CIDX to 0x%.8x\n",
              REG32(RXE_DMA_MSG_DESC_CIDX_0), REG32(RXE_DMA_MSG_DATA_CIDX_0));

    inno_hi_watermark_enable(idev);
    retry_cnt = 0;
    while (retry_cnt < LEARN_STATUS_LOOP_RETRY_CNT_MAX) {
        ret = inno_hi_watermark_clear_reset(idev, 0); /* do not log an error */
        if (ret == 0) {
            break;
        }
        msleep_interruptible(1);
        retry_cnt++;
    }

    ret = inno_hi_watermark_clear_reset(idev, 1); /* log an error */
    if (ret < 0) {
        ipd_err("pidx/cidx values before the cleanup attempt: "
                "msg_desc_cidx: 0x%.8x msg_desc_pidx: 0x%.8x "
                "msg_data_cidx: 0x%.8x msg_data_pidx: 0x%.8x\n",
                msg_desc_cidx, msg_desc_pidx, msg_data_cidx, msg_data_pidx);
    }

    /* Clear the learn enable */
    msg_rxe.data = REG32(BLK_AND_MSG_RXE);
    /* Reset only the Learn enable */
    msg_rxe.flds.msg_en_0_f = 0;
    REG32(BLK_AND_MSG_RXE) = msg_rxe.data;

    /* Check for the learn status */
    retry_cnt = 0;
    while (retry_cnt < LEARN_STATUS_LOOP_RETRY_CNT_MAX) {
        learn_status = REG32(RXE_DMA_MSG_STATUS_0);
        if (learn_status == 0) {
            break;
        }
        msleep_interruptible(1);
        retry_cnt++;
    }

    if (learn_status) {
        ipd_err("Learn Status is still non-zero, but exiting\n");
    }

    /* Drain any learn messages, again */
    REG32(RXE_DMA_MSG_DESC_CIDX_0) = REG32(RXE_DMA_MSG_DESC_PIDX_0);
    ipd_debug("After Learn drain, PIDX: 0x%.8x, CIDX: 0x%.8x\n",
              REG32(RXE_DMA_MSG_DESC_PIDX_0),
              REG32(RXE_DMA_MSG_DESC_CIDX_0));
    /* Also drain the learn data */
    REG32(RXE_DMA_MSG_DATA_CIDX_0) = REG32(RXE_DMA_MSG_DATA_PIDX_0);
    ipd_debug("After Learn data drain, PIDX: 0x%.8x, CIDX: 0x%.8x\n",
              REG32(RXE_DMA_MSG_DATA_PIDX_0),
              REG32(RXE_DMA_MSG_DATA_CIDX_0));

    /* Free the LEARN data area */
    dma_free_coherent(&idev->pdev->dev,
                      learn->count * 2 * sizeof(inno_learn_data_t),
                      learn->vmaddr, learn->orig_ba);

    learn->vmaddr = NULL;

    /* Free the LEARN WB data area */
    dma_free_coherent(&idev->pdev->dev,
                      learn->count * sizeof(inno_learn_wb_t),
                      learn->wb_vmaddr, learn->wb_ba);

    learn->wb_vmaddr = NULL;

    learn->flags &= ~INNO_LEARN_INIT;

    ipd_debug("LEARN freed\n");

    return 0;
}


/** @brief Pin the user pages and place in the ring info
 *
 *  @param [in] idev - innovium device
 *  @param [io] ioctl - ioctl data area
 *  @return ERRNO
 */
static int
inno_tx_send(inno_device_t        *idev,
             inno_ioctl_tx_send_t *ioctl)
{
    int         i, pinned;
    struct page *pages[5];        /* Max for unaligned jumbo + ldh */
    inno_ring_t *tx = &idev->tx_ring[ioctl->ring_num];
    uint32_t    offset;
    uint32_t    rem_len;
    uint16_t    nr_pages;

    if (tx->flags & INNO_RING_NETDEV) {
        return -EINVAL;
    }

    if (ioctl->flags & IOCTL_SEND_ZCOPY_FLAG) {
        uint16_t lpidx;
        if (ioctl->flags & IOCTL_SEND_LDH_FLAG) {
            /* Can't send LDH in its own descriptor for now */
            return -EINVAL;
        }
        offset = (uint32_t) (((uint64_t) ioctl->buf) & (PAGE_SIZE - 1));
        nr_pages = ((offset + ioctl->buf_len - 1) >> PAGE_SHIFT) + 1;

        ipd_debug("tx_send user_pages buf:0x%llx len:%u offset:%u nr_pages:%d\n",
                 ioctl->buf, ioctl->buf_len, offset, nr_pages);
         /*print_hex_dump(KERN_INFO, "", DUMP_PREFIX_OFFSET,
                        16, 1, ioctl->buf, 32, true);*/


        if (nr_pages > 4) {
            return -EINVAL;
        }

        /* No copy - just pin the pages */
        pinned = get_user_pages_fast(((uint64_t)ioctl->buf) & PAGE_MASK,
                                     nr_pages, 0, &pages[0]);
        if (pinned < nr_pages) {
            ipd_err("Could not pin TX buffer pages got:%d needed:%d\n", pinned, nr_pages);
            if (pinned > 0) {
                /* Got some - must unpin them */
                for (i = 0; i < pinned; i++) {
                    put_page(pages[i]);
                }
            }
            return -ENOMEM;
        }

        /* Copy the page info to the ring page struct */
        lpidx = tx->pidx;
        for (i = 0; i < nr_pages; i++) {
            inno_dma_alloc_t *alloc = &tx->pages[RING_IDX_MASKED(lpidx)];
            int rc = 0;

            /* Get the physical addr */
            alloc->dma_addr = dma_map_page(&idev->pdev->dev, pages[i],
                    0, PAGE_SIZE, DMA_TO_DEVICE);
            if (dma_mapping_error(&idev->pdev->dev, alloc->dma_addr)) {
                ipd_err("tx_send dma_map err i:%d offset:%u len:%u \n",
                        i, 0, (unsigned)PAGE_SIZE);
                return -1;
            }

            if(rc < 0) {

                /* Unpin the pages */
                for (i = 0; i < pinned; i++) {
                    put_page(pages[i]);
                }
                return -ENOMEM;
            }

            ipd_debug("tx_send map i:%d p_addr:%lu offset:%u len:%u\n",
                    i, (unsigned long)(alloc->dma_addr),
                    0, (unsigned)PAGE_SIZE);

            alloc->page     = pages[i];
            RING_IDX_INCR(tx->count, lpidx);
        }

    } else {
        /* Not zero copy */
        uint16_t lpidx;
        uint32_t rem_len = ioctl->buf_len;
        uint8_t  *buf = (uint8_t *) ioctl->buf;

        offset = 0;
        if (ioctl->flags & IOCTL_SEND_LDH_FLAG) {
            nr_pages = ((ioctl->buf_len
                        + (idev->chip_hdr_len *ioctl->num_ldh) - 1) >>
                        PAGE_SHIFT) + 1;
        } else {
            nr_pages = ((ioctl->buf_len - 1) >> PAGE_SHIFT) + 1;
        }

        if (nr_pages > 4) {
            return -EINVAL;
        }

        /* Have to allocate pages and copy the LDH+buffer */
        lpidx = tx->pidx;
        for (i = 0; i < nr_pages; i++) {
            inno_dma_alloc_t *alloc = &tx->pages[RING_IDX_MASKED(lpidx)];
            uint8_t *page;
            uint32_t copy_size;

            alloc->page = alloc_page(__GFP_HIGHMEM);
            if (alloc->page == NULL) {
                uint16_t fpidx = tx->pidx;
                ipd_err("Unable to allocate page buffer for tx\n");

                /* Release what we have */
                while (fpidx != lpidx) {
                    alloc = &tx->pages[RING_IDX_MASKED(fpidx)];
                    dma_unmap_page(&idev->pdev->dev, alloc->dma_addr,
                                   PAGE_SIZE, DMA_TO_DEVICE);
                    __free_page(alloc->page);
                    memset(alloc, 0, sizeof(inno_dma_alloc_t));
                    RING_IDX_INCR(tx->count, fpidx);
                }
                return -1;
            }

            alloc->dma_addr = dma_map_page(&idev->pdev->dev,
                                           alloc->page, 0,
                                           PAGE_SIZE, DMA_TO_DEVICE);

            if (dma_mapping_error(&idev->pdev->dev, alloc->dma_addr)) {
                ipd_err("%s dma_map_page err i:%d offset:%u len:%u \n",
                        __func__, i, 0, (unsigned)PAGE_SIZE);
                return -1;
            }
            /* Use the vmaddr as a free flag for  unpin */
            alloc->vmaddr = page_address(alloc->page);
            page = alloc->vmaddr;
            ipd_debug("TX %x %llx %p %p\n",
                     nr_pages, alloc->dma_addr, alloc->page, page);

            if ((i == 0) && (ioctl->flags & IOCTL_SEND_LDH_FLAG)) {
                memcpy(page, &ioctl->ldh, idev->chip_hdr_len*ioctl->num_ldh);
                page += idev->chip_hdr_len*ioctl->num_ldh;
                if (rem_len < (PAGE_SIZE - idev->chip_hdr_len*ioctl->num_ldh)) {
                    copy_size = rem_len;
                } else {
                    copy_size = PAGE_SIZE - idev->chip_hdr_len*ioctl->num_ldh;
                }

            } else if (rem_len >= PAGE_SIZE) {
                copy_size = PAGE_SIZE;
            } else {
                copy_size = rem_len;
            }

            if(copy_from_user(page, buf, copy_size)) {
                uint16_t fpidx = tx->pidx;
                ipd_err("TX: Unable to copy user page buffer\n");

                /* Release what we have */
                while (fpidx != lpidx) {
                    alloc = &tx->pages[RING_IDX_MASKED(fpidx)];
                    dma_unmap_page(&idev->pdev->dev, alloc->dma_addr,
                                   PAGE_SIZE, DMA_TO_DEVICE);
                    __free_page(alloc->page);
                    memset(alloc, 0, sizeof(inno_dma_alloc_t));
                    RING_IDX_INCR(tx->count, fpidx);
                }
                return -1;
            }
            buf += copy_size;
            rem_len -= copy_size;

            RING_IDX_INCR(tx->count, lpidx);
        }
    }

    ioctl->start_pidx = tx->pidx;
    rem_len = ioctl->buf_len;
    if (ioctl->flags & IOCTL_SEND_LDH_FLAG) {
        rem_len += idev->chip_hdr_len*ioctl->num_ldh;
    }

    /* Fill in the descriptor. The last descriptor is used for FCS and also gets marked as EOP */
    for (i = 0; i <= nr_pages; i++) {
        inno_tx_desc_t   tx_desc;  /* Local copy in cache */
        uint32_t         len;
        inno_dma_alloc_t *alloc = &tx->pages[RING_IDX_MASKED(tx->pidx)];

        /* Calculate the actual length of this descriptor */
        if (i == nr_pages) {               /* Last page is for FCS + padding if required */
            /* We expect zero copy buffer to be atleast 64 bytes */
            if(!(ioctl->flags & IOCTL_SEND_ZCOPY_FLAG) && (((ioctl->buf_len + ETH_FCS_LEN))  < MIN_PACKET_SIZE)) {
                len = MIN_PACKET_SIZE - ioctl->buf_len; /* The packet should be atleast 64 bytes */
                memset((tx->tx_cksum + (MIN_PACKET_SIZE*RING_IDX_MASKED(tx->pidx))), 0, len);
            } else {
                len = ETH_FCS_LEN;
            }
            alloc->dma_addr = tx->tx_cksum_ba + (MIN_PACKET_SIZE*RING_IDX_MASKED(tx->pidx));
        } else if (i == nr_pages - 1) {    /* Second last page */
            len = rem_len;
        } else if (i == 0) {               /* If first page (and not also last) */
            len = PAGE_SIZE - offset;
        } else {                           /* Not first or last */
            len = PAGE_SIZE;
        }

        memset(&tx_desc, 0, sizeof(inno_tx_desc_t));
        tx_desc.hsn_upper = (uint32_t)((alloc->dma_addr + offset) >> 32);
        tx_desc.hsn_lower = (uint32_t)((alloc->dma_addr + offset) & 0x00000000ffffffff);
        tx_desc.length = len;

         ipd_debug("hsn_upper: 0x%x hsn_lower: 0x%x len: 0x%x\n",
                  tx_desc.hsn_upper, tx_desc.hsn_lower, tx_desc.length);

        if (i == 0) {
            tx_desc.sop = 1;    /* Start of  packet */
        }

        if (i == nr_pages) {
            tx_desc.eop = 1;    /* End of packet */
        }

        ipd_debug("tx_send mk_desc %d page:%016lx d_addr:0x%08x%08x len:%u offset:%d sop:%d eop:%d\n",
                 nr_pages, (unsigned long)(alloc->dma_addr), tx_desc.hsn_upper, tx_desc.hsn_lower,
                 tx_desc.length, offset, tx_desc.sop, tx_desc.eop);

        /* Copy the cached copy of the descriptor to the actual ring */
        memcpy(&tx->tx_desc[RING_IDX_MASKED(tx->pidx)], &tx_desc,
               sizeof(inno_tx_desc_t));

        RING_IDX_INCR(tx->count, tx->pidx);

        offset = 0;
        rem_len -= len;
        idev->inno_stats.tx_ring_stats[ioctl->ring_num].descs++;
        idev->inno_stats.tx_ring_stats[ioctl->ring_num].bytes+=len;
    }

    ioctl->last_pidx = tx->pidx;
    idev->inno_stats.tx_ring_stats[ioctl->ring_num].packets++;
    return 0;
}


/** @brief Unpin a user page
 *
 *  @param [in] idev - innovium device
 *  @param [io] ioctl - ioctl data area
 *  @return ERRNO
 */
static int
inno_unpin_page(inno_device_t            *idev,
                inno_ioctl_unpin_pages_t *ioctl)
{
    inno_ring_t           *tx   = &idev->tx_ring[ioctl->ring_num];

    /* EOP is FCS */
    if(tx->tx_desc[ioctl->idx].eop) {
        return 0;
    }

    if (tx->pages[ioctl->idx].page != NULL) {
        dma_unmap_page(&idev->pdev->dev, tx->pages[ioctl->idx].dma_addr,
                       PAGE_SIZE, DMA_TO_DEVICE);
        if (tx->pages[ioctl->idx].vmaddr != NULL) {
            /* The vmaddr is used as a "free flag" */
            __free_page(tx->pages[ioctl->idx].page);
            tx->pages[ioctl->idx].vmaddr = NULL;
        } else {
            put_page(tx->pages[ioctl->idx].page);
        }

        tx->pages[ioctl->idx].page = NULL;
    }

    return 0;
}

/** @brief Allocate Ring page memory
 *
 *  @param [in] idev                         - innovium device
 *  @param [in] inno_ioctl_ring_page_alloc_t - ioctl structure
 *  @return ERRNO
 */
static int
inno_ring_page_alloc(inno_device_t *idev,
                     inno_ioctl_ring_page_alloc_t *ioctl_ring_page_alloc)
{
    inno_ring_t      *ring;
    uint32_t         ring_num = ioctl_ring_page_alloc->ring_num;
    uint32_t         idx      = ioctl_ring_page_alloc->idx;
    inno_dma_alloc_t *dma_alloc_p;
    enum dma_data_direction direction;

    switch(ioctl_ring_page_alloc->ring_type) {
    case INNO_RING_TYPE_TX:
        if (ring_num >= NUM_TX_RINGS) {
            ipd_err("Invalid TX ring %u", ring_num);
            return -1;
        }

        ring = &idev->tx_ring[ring_num];
        if ((ring->flags & INNO_RING_INIT) == 0) {
            ipd_err("TX Ring %u uninitialized", ring_num);
            return -1;
        }

        if (idx >= ring->count) {
            ipd_err("Invalid TX ring %u index %u", ring_num, idx);
            return -1;
        }

        if (ring->pages == NULL) {
            ipd_err("Pages array not allocated for TX ring %u",
                             ring_num);
            return -1;
        }

        dma_alloc_p = &ring->pages[idx];
        direction = DMA_TO_DEVICE;

        break;
    case INNO_RING_TYPE_RX:
        if (ioctl_ring_page_alloc->ring_num >= NUM_RX_RINGS) {
            ipd_err("Invalid RX ring %u",
                    ioctl_ring_page_alloc->ring_num);
            return -1;
        }

        ring = &idev->rx_ring[ring_num];
        if ((ring->flags & INNO_RING_INIT) == 0) {
            ipd_err("RX Ring %u uninitialized", ring_num);
            return -1;
        }

        if (idx >= ring->count) {
            ipd_err("Invalid RX ring %u index %u", ring_num, idx);
            return -1;
        }

        if (ring->pages == NULL) {
            ipd_err("Pages array not allocated for RX ring %u",
                             ring_num);
            return -1;
        }

        dma_alloc_p = &ring->pages[idx];
        direction = DMA_FROM_DEVICE;

        dma_alloc_p->page = alloc_page(__GFP_HIGHMEM);
        if (dma_alloc_p->page == NULL) {
            ipd_err("Unable to allocate page buffer buffer for index %u",
                   idx);
            return -1;
        }

        dma_alloc_p->dma_addr = dma_map_page(&idev->pdev->dev, dma_alloc_p->page, 0,
                                             PAGE_SIZE, direction);
        if (dma_mapping_error(&idev->pdev->dev, dma_alloc_p->dma_addr)) {
            ipd_err("%s dma_map_page err idx: %u offset:%u len:%u \n",
                    __func__, idx, 0, (unsigned)PAGE_SIZE);
           return -1;
        }
        dma_alloc_p->vmaddr = page_address(dma_alloc_p->page);

        ioctl_ring_page_alloc->buf_ba = dma_alloc_p->dma_addr;
        ioctl_ring_page_alloc->buf_pa = (off_t)virt_to_phys(dma_alloc_p->vmaddr);
        smp_mb();
        ring->rx_desc[idx].hsn_upper = (uint32_t)(dma_alloc_p->dma_addr >> 32);
        ring->rx_desc[idx].hsn_lower = (uint32_t)(dma_alloc_p->dma_addr & 0x00000000ffffffff);
        smp_mb();

        break;
    case INNO_RING_TYPE_HRR:
        if (idev->hrr.pages == NULL) {
            ipd_err("Async FIFO buffer array unallocated");
            return -1;
        }

        if (idx >= idev->hrr.descriptors) {
            ipd_err("Invalid Async FIFO buffer array index %u", idx);
            return -1;
        }

        dma_alloc_p = &idev->hrr.pages[idx];
        direction = DMA_BIDIRECTIONAL;

        break;
    default:
        return -1;
    }

    /* Allocate the buffer space */
    if(ioctl_ring_page_alloc->ring_type != INNO_RING_TYPE_RX)
    {
        dma_alloc_p->page = alloc_page(__GFP_HIGHMEM);
        if (dma_alloc_p->page == NULL) {
            ipd_err("Unable to allocate page buffer buffer for index %u",
                   idx);
            return -1;
        }

        dma_alloc_p->dma_addr = dma_map_page(&idev->pdev->dev, dma_alloc_p->page, 0,
                                             PAGE_SIZE, direction);
        if (dma_mapping_error(&idev->pdev->dev, dma_alloc_p->dma_addr)) {
            ipd_err("%s dma_map_page err idx: %u offset:%u len:%u \n",
                    __func__, idx, 0, (unsigned)PAGE_SIZE);
            return -1;
        }
        dma_alloc_p->vmaddr = page_address(dma_alloc_p->page);

        ioctl_ring_page_alloc->buf_ba = dma_alloc_p->dma_addr;
        ioctl_ring_page_alloc->buf_pa = (off_t)virt_to_phys(dma_alloc_p->vmaddr);
    }

    return 0;
}

/** @brief Free Ring page memory
 *
 *  @param [in] idev                          - innovium device
 *  @param [in] inno_ioctl_ring_page_alloc_t - ioctl structure
 *  @return ERRNO
 */
static int
inno_ring_page_free(inno_device_t *idev,
                    inno_ioctl_ring_page_alloc_t *ioctl_ring_page_alloc)
{
    inno_ring_t      *ring;
    uint32_t         ring_num = ioctl_ring_page_alloc->ring_num;
    uint32_t         idx      = ioctl_ring_page_alloc->idx;
    inno_dma_alloc_t *dma_alloc_p;
    enum dma_data_direction direction;

    switch(ioctl_ring_page_alloc->ring_type) {
    case INNO_RING_TYPE_TX:
        if (ring_num >= NUM_TX_RINGS) {
            ipd_err("Invalid TX ring %u", ring_num);
            return -1;
        }

        ring = &idev->tx_ring[ring_num];
        if ((ring->flags & INNO_RING_INIT) == 0) {
            ipd_err("TX Ring %u uninitialized", ring_num);
            return -1;
        }

        if (idx >= ring->count) {
            ipd_err("Invalid TX ring %u index %u", ring_num, idx);
            return -1;
        }

        dma_alloc_p = &ring->pages[idx];
        direction = DMA_TO_DEVICE;

        break;
    case INNO_RING_TYPE_RX:
        if (ioctl_ring_page_alloc->ring_num >= NUM_RX_RINGS) {
            ipd_err("Invalid RX ring %u",
                    ioctl_ring_page_alloc->ring_num);
            return -1;
        }

        ring = &idev->rx_ring[ring_num];
        if ((ring->flags & INNO_RING_INIT) == 0) {
            ipd_err("RX Ring %u uninitialized", ring_num);
            return -1;
        }

        if (idx >= ring->count) {
            ipd_err("Invalid TX ring %u index %u", ring_num, idx);
            return -1;
        }

        dma_alloc_p = &ring->pages[idx];
        direction = DMA_FROM_DEVICE;

        break;
    case INNO_RING_TYPE_HRR:
        if (idev->hrr.pages == NULL) {
            ipd_err("Async FIFO buffer array unallocated");
            return -1;
        }

        if (idx >= idev->hrr.descriptors) {
            ipd_err("Invalid Async FIFO buffer array index %u", idx);
            return -1;
        }

        dma_alloc_p = &idev->hrr.pages[idx];
        direction = DMA_BIDIRECTIONAL;

        break;
    default:
        return -1;
    }

    /* Free the buffer space */
    dma_unmap_page(&idev->pdev->dev, dma_alloc_p->dma_addr, PAGE_SIZE, direction);
    __free_page(dma_alloc_p->page);
    memset(dma_alloc_p, 0, sizeof(inno_dma_alloc_t));

    return 0;
}

/** @brief DMA global initialization IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @return ERRNO
 */
static int
inno_dma_init(inno_device_t *idev)
{
    int                            i;
    dma_global_t                   dma_global_reg;
    sbuf_cpu_to_switch_ctrl_t      sbuf_cpu_to_switch_ctrl;
    cpu_to_switchif_src_t          cpu_to_switchif_src;

    /* Perform global DMA controller setup */
    /* Now make sure that the global DMA is enabled */
    memset(&dma_global_reg, 0, sizeof(dma_global_reg));

    dma_global_reg.flds.rst_n_f = 0;    /* Start with a reset */
    REG32(DMA_GLOBAL)           = dma_global_reg.data;
    dma_global_reg.flds.rst_n_f = 1;
    REG32(DMA_GLOBAL)           = dma_global_reg.data;

    dma_global_reg.flds.en_f      = 1;
    dma_global_reg.flds.intr_en_f = 1;
    dma_global_reg.flds.en_f      = 1;
    dma_global_reg.flds.txe_en_f  = 1;
    dma_global_reg.flds.en_txe_cacheline_split_f = 0;
    dma_global_reg.flds.rxe_en_f = 1;
    dma_global_reg.flds.en_rxe_cacheline_split_f = 0;
    dma_global_reg.flds.cacheline_size_f         = idev->cache_align >> 6;
    dma_global_reg.flds.en_dm_cacheline_split_f  = 1;
    REG32(DMA_GLOBAL) = dma_global_reg.data;

    /* Equal weights for CPU and MCUs */
    cpu_to_switchif_src.data          = 0;
    cpu_to_switchif_src.flds.wght_0_f = 1;
    cpu_to_switchif_src.flds.wght_1_f = 1;
    cpu_to_switchif_src.flds.wght_2_f = 1;
    REG32(CPU_TO_SWITCHIF_SRC)        = cpu_to_switchif_src.data;

    /* Reset and enable the SBUF CPU_TO_SWITCH/MCU */
    sbuf_cpu_to_switch_ctrl.data         = 0;
    REG32(SBUF_CPU_TO_SWITCH_CTRL)       = sbuf_cpu_to_switch_ctrl.data;
    REG32(SBUF_MCU_0_CTRL)     = sbuf_cpu_to_switch_ctrl.data;
    REG32(SBUF_MCU_1_CTRL)     = sbuf_cpu_to_switch_ctrl.data;
    sbuf_cpu_to_switch_ctrl.flds.rst_n_f = 1;
    REG32(SBUF_CPU_TO_SWITCH_CTRL)       = sbuf_cpu_to_switch_ctrl.data;
    REG32(SBUF_MCU_0_CTRL)     = sbuf_cpu_to_switch_ctrl.data;
    REG32(SBUF_MCU_1_CTRL)     = sbuf_cpu_to_switch_ctrl.data;
    sbuf_cpu_to_switch_ctrl.flds.en_f    = 1;
    REG32(SBUF_CPU_TO_SWITCH_CTRL)       = sbuf_cpu_to_switch_ctrl.data;
    REG32(SBUF_MCU_0_CTRL)     = sbuf_cpu_to_switch_ctrl.data;
    REG32(SBUF_MCU_1_CTRL)     = sbuf_cpu_to_switch_ctrl.data;

    /* Reset and enable the SBUF SWITCH_TO_CPU */
    for (i = 0; i < 4; i++) {
        sbuf_switch_to_cpu_0_ctrl_t sbuf_switch_to_cpu_ctrl;
        sbuf_switch_to_cpu_0_cnfg2_t  sbuf_cnfg2;
        sbuf_switch_to_cpu_0_cnfg1_t  sbuf_cnfg1;

        sbuf_cnfg1.flds.start_addr_f = 0x800 * i;
        sbuf_cnfg1.flds.size_f       = 0x800;
        REG32(SBUF_SWITCH_TO_CPU_x_CNFG1(i))   = sbuf_cnfg1.data;

        sbuf_cnfg2.flds.xoff_threshold_f = 0x0120;
        sbuf_cnfg2.flds.xon_threshold_f  = 0x00E0;
        REG32(SBUF_SWITCH_TO_CPU_x_CNFG2(i))       = sbuf_cnfg2.data;

        sbuf_switch_to_cpu_ctrl.data         = 0;
        /* REG32(SBUF_SWITCH_TO_CPU_x_CTRL(i))  = sbuf_switch_to_cpu_ctrl.data; */
        sbuf_switch_to_cpu_ctrl.flds.rst_n_f = 1;
        /* REG32(SBUF_SWITCH_TO_CPU_x_CTRL(i))  = sbuf_switch_to_cpu_ctrl.data; */
        sbuf_switch_to_cpu_ctrl.flds.en_f    = 1;
        REG32(SBUF_SWITCH_TO_CPU_x_CTRL(i))  = sbuf_switch_to_cpu_ctrl.data;
    }

    /* Reset and enable the SBUF MCU */
    for (i = 0; i < 2; i++) {
        sbuf_mcu_0_cnfg1_t sbuf_cnfg1;
        sbuf_mcu_0_cnfg2_t sbuf_cnfg2;
        sbuf_mcu_0_ctrl_t  sbuf_mcu_ctrl;

        sbuf_cnfg1.flds.start_addr_f = 0;
        sbuf_cnfg1.flds.size_f  = 0x800;
        REG32(SBUF_MCU_x_CNFG1(i)) = sbuf_cnfg1.data;

        sbuf_cnfg2.flds.xoff_threshold_f = 0xa00;
        sbuf_cnfg2.flds.xon_threshold_f  = 0x500;
        REG32(SBUF_MCU_x_CNFG2(i))       = sbuf_cnfg2.data;

        sbuf_mcu_ctrl.data         = 0;
        REG32(SBUF_MCU_x_CTRL(i))  = sbuf_mcu_ctrl.data;
        sbuf_mcu_ctrl.flds.rst_n_f = 1;
        REG32(SBUF_MCU_x_CTRL(i))  = sbuf_mcu_ctrl.data;
        sbuf_mcu_ctrl.flds.en_f    = 1;
        REG32(SBUF_MCU_x_CTRL(i))  = sbuf_mcu_ctrl.data;
    }
    ipd_info("DMA init done\n");

    return 0;
}

#define RECOVERY_RING_NUM_DESC  8
#define FLUSH_PKT_CNT 5

/** @brief Flush and recover DMA IOCTL function
 *
 *  @param [in] idev - innovium device
 *  @return ERRNO
 */
static int
inno_flush_recover_dma(inno_device_t *idev,
                       inno_ioctl_flush_pkt_t *flush_pkt)
{
    int            ret_val = 0;
    int            rx_ring_num = 0;
    int            rx_ring_size;
    dma_addr_t     rx_desc_ba;
    inno_rx_desc_t *rx_desc;
    inno_rx_wb_t   *rx_wb;
    dma_addr_t     rx_wb_ba;
    uint32_t       i;
    struct page    *rx_page;
    dma_addr_t     rx_dma_addr;
    rxq_0_t        rxq_reg;
    rxq_0_desc_pidx_t             rxq_pidx                       = {{0}};
    rxq_0_cidx_update_t           rxq_cidx_update_reg            = {{0}};
    rxq_0_cidx_update_cliff_t     rxq_cidx_update_cliff_reg      = {{0}};
    rxq_0_cidx_update_prescaler_t rxq_cidx_update_prescaler_reg  = {{0}};
    rxq_0_cidx_update_precliff_t  rxq_cidx_update_precliff_reg   = {{0}};
    rxq_0_cidx_update_tmr_ctl_t   rxq_cidx_update_tmr_ctl_reg    = {{0}};

    int            tx_ring_num = 0;
    int            tx_ring_size;
    dma_addr_t     tx_desc_ba;
    inno_tx_desc_t *tx_desc;
    inno_tx_wb_t   *tx_wb;
    dma_addr_t     tx_wb_ba;
    struct page    *tx_page;
    dma_addr_t     tx_dma_addr;
    uint8_t        *tx_cksum;
    dma_addr_t     tx_cksum_ba;
    txq_0_t        txq_reg;
    void           *tx_page_vmaddr;
    txq_0_desc_pidx_t             txq_pidx                      = {{0}};
    txq_0_cidx_update_t           txq_cidx_update_reg           = {{0}};
    txq_0_cidx_update_cliff_t     txq_cidx_update_cliff_reg     = {{0}};
    txq_0_cidx_update_prescaler_t txq_cidx_update_prescaler_reg = {{0}};
    txq_0_cidx_update_precliff_t  txq_cidx_update_precliff_reg  = {{0}};
    txq_0_cidx_update_tmr_ctl_t   txq_cidx_update_tmr_ctl_reg   = {{0}};

    uint32_t                      rxe_total_pkt_cnt_pre = 0;
    uint32_t                      rxe_total_pkt_cnt_post = 0;

    ipd_debug("Flush recover DMA\n");
    ipd_debug("Packet to be sent: 0x%.8x 0x%.8x 0x%.8x 0x%.8x 0x%.8x 0x%.8x 0x%.8x 0x%.8x\n",
              flush_pkt->pkt[0], flush_pkt->pkt[1], flush_pkt->pkt[2], flush_pkt->pkt[3],
              flush_pkt->pkt[4], flush_pkt->pkt[5], flush_pkt->pkt[6], flush_pkt->pkt[7]);

    rx_ring_size = sizeof(inno_rx_desc_t) * RECOVERY_RING_NUM_DESC;

    /* Allocate the ring space (TX or RX based on desc_size) */
    rx_desc = (void *)dma_alloc_coherent(&idev->pdev->dev, rx_ring_size,
                                         &rx_desc_ba, GFP_KERNEL);
    if (rx_desc == NULL) {
        ipd_err("Descriptor ring alloc Failed for recovery ring \n");
        return -ENOMEM;
    }

    /* Allocate the wb space (TX or RX based on desc_size) */
    rx_wb = (void *)dma_alloc_coherent(&idev->pdev->dev, rx_ring_size,
                                        &rx_wb_ba, GFP_KERNEL);
    if (rx_wb == NULL) {
        ipd_err("Writeback ring alloc Failed \n");
        dma_free_coherent(&idev->pdev->dev, rx_ring_size,
                          rx_desc, rx_desc_ba);
        return -ENOMEM;
    }

    memset(rx_wb, 0, rx_ring_size);

    rxq_cidx_update_reg.flds.timer_f    = 0;
    REG32(RXQ_x_CIDX_UPDATE(rx_ring_num)) =
        rxq_cidx_update_reg.data;
    rxq_cidx_update_cliff_reg.flds.timer_f    = 0;
    REG32(RXQ_x_CIDX_UPDATE_CLIFF(rx_ring_num)) =
        rxq_cidx_update_cliff_reg.data;
    rxq_cidx_update_prescaler_reg.flds.timer_f    = 0;
    REG32(RXQ_x_CIDX_UPDATE_PRESCALER(rx_ring_num)) =
        rxq_cidx_update_prescaler_reg.data;
    rxq_cidx_update_precliff_reg.flds.timer_f    = 0;
    REG32(RXQ_x_CIDX_UPDATE_PRECLIFF(rx_ring_num)) =
        rxq_cidx_update_precliff_reg.data;
    rxq_cidx_update_tmr_ctl_reg.flds.enable_f = 1;
    REG32(RXQ_x_CIDX_UPDATE_TMR_CTL(rx_ring_num)) =
        rxq_cidx_update_tmr_ctl_reg.data;
    rxq_reg.flds.cidx_intr_cnt_coalse_en_f = 1;
    rxq_reg.flds.dma_type_f = 1;
    rxq_reg.flds.chnl_map_f = 0;
    REG32(RXQ_x(rx_ring_num)) = rxq_reg.data;
    REG32(RXQ_x_BASE_HSN_HI(rx_ring_num))    = rx_desc_ba >> 32;
    REG32(RXQ_x_BASE_HSN_LO(rx_ring_num))    = rx_desc_ba & 0xffffffff;
    REG32(RXQ_x_BASE_HSN_HI_WB(rx_ring_num)) = rx_wb_ba >> 32;
    REG32(RXQ_x_BASE_HSN_LO_WB(rx_ring_num)) = rx_wb_ba & 0xffffffff;

    rx_page = alloc_page(__GFP_HIGHMEM);
    if (rx_page == NULL) {
        /* We might run out or we will get more on the next rupt */
        ipd_crit("Unable to allocate RX page memory \
                  ring: %d\n", rx_ring_num);
        dma_free_coherent(&idev->pdev->dev, rx_ring_size,
                          rx_desc, rx_desc_ba);
        dma_free_coherent(&idev->pdev->dev, rx_ring_size,
                          rx_wb, rx_wb_ba);
        return -1;
    }
    rx_dma_addr = dma_map_page(&idev->pdev->dev, rx_page, 0,
                               PAGE_SIZE, DMA_FROM_DEVICE);
    if (dma_mapping_error(&idev->pdev->dev, rx_dma_addr)) {
        ipd_err("%s dma_map len:%u \n", __func__,
                 (unsigned)PAGE_SIZE);
        dma_free_coherent(&idev->pdev->dev, rx_ring_size,
                          rx_desc, rx_desc_ba);
        dma_free_coherent(&idev->pdev->dev, rx_ring_size,
                          rx_wb, rx_wb_ba);
        free_page((unsigned long)rx_page);
        return -1;
    }

    /* Allocate a page of memory and use it for all the descriptors */
    for (i = 0; i < RECOVERY_RING_NUM_DESC; i++ ) {
        smp_mb();
        rx_desc[i].hsn_upper = (uint32_t)(rx_dma_addr >> 32);
        rx_desc[i].hsn_lower = (uint32_t)(rx_dma_addr & 0x00000000ffffffff);
        smp_mb();
    }

    REG32(RXQ_x_DESC_RING(rx_ring_num)) = RECOVERY_RING_NUM_DESC;
    rxq_pidx.flds.num_f = FLUSH_PKT_CNT;
    REG32(RXQ_x_DESC_PIDX(rx_ring_num)) = rxq_pidx.data;
    /* enable RX queue to start receving packets from DTM */
    inno_enable_queue_rx(idev, rx_ring_num);
    ipd_debug("RX queue enabled\n");

    tx_ring_size = sizeof(inno_tx_desc_t) * RECOVERY_RING_NUM_DESC;

    /* Allocate the ring space (TX or RX based on desc_size) */
    tx_desc = (void *)dma_alloc_coherent(&idev->pdev->dev, tx_ring_size,
                                         &tx_desc_ba, GFP_KERNEL);
    if (tx_desc == NULL) {
        ipd_err("Descriptor ring alloc Failed for recovery ring \n");
        return -ENOMEM;
    }

    /* Allocate the tx checksum desc */
    tx_cksum = (void *)dma_alloc_coherent(&idev->pdev->dev,
        RECOVERY_RING_NUM_DESC * MIN_PACKET_SIZE,
        &tx_cksum_ba, GFP_KERNEL);
    if (tx_cksum == NULL) {
        ipd_err("TX checksum descriptor alloc failed \n");
        dma_free_coherent(&idev->pdev->dev, tx_ring_size,
                rx_desc, tx_desc_ba);
        return -ENOMEM;
    }

    /* Allocate the wb space (TX or RX based on desc_size) */
    tx_wb = (void *)dma_alloc_coherent(&idev->pdev->dev, tx_ring_size,
                                       &tx_wb_ba, GFP_KERNEL);
    if (tx_wb == NULL) {
        ipd_err("Writeback ring alloc Failed \n");
        dma_free_coherent(&idev->pdev->dev, tx_ring_size,
                          tx_desc, tx_desc_ba);
        dma_free_coherent(&idev->pdev->dev, RECOVERY_RING_NUM_DESC * MIN_PACKET_SIZE,
                          tx_cksum, tx_cksum_ba);
        return -ENOMEM;
    }
    memset(tx_wb, 0, tx_ring_size);

    tx_page = alloc_page(__GFP_HIGHMEM);
    if (tx_page == NULL) {
        /* We might run out or we will get more on the next rupt */
        ipd_crit("Unable to allocate TX page memory \
                  ring: %d\n", tx_ring_num);
        dma_free_coherent(&idev->pdev->dev, tx_ring_size,
                          tx_desc, tx_desc_ba);
        dma_free_coherent(&idev->pdev->dev, RECOVERY_RING_NUM_DESC * MIN_PACKET_SIZE,
                          tx_cksum, tx_cksum_ba);
        dma_free_coherent(&idev->pdev->dev, tx_ring_size,
                          tx_wb, tx_wb_ba);
        return -1;
    }

    tx_dma_addr = dma_map_page(&idev->pdev->dev, tx_page, 0,
                               PAGE_SIZE, DMA_TO_DEVICE);
    if (dma_mapping_error(&idev->pdev->dev, tx_dma_addr)) {
        ipd_err("%s dma_map len:%u \n", __func__,
                 (unsigned)PAGE_SIZE);
        dma_free_coherent(&idev->pdev->dev, tx_ring_size,
                          tx_desc, tx_desc_ba);
        dma_free_coherent(&idev->pdev->dev, RECOVERY_RING_NUM_DESC * MIN_PACKET_SIZE,
                          tx_cksum, tx_cksum_ba);
        dma_free_coherent(&idev->pdev->dev, tx_ring_size,
                          tx_wb, tx_wb_ba);
        free_page((unsigned long)tx_page);
        return -1;
    }

    txq_reg.flds.cidx_intr_cnt_coalse_en_f = 1;
    txq_reg.flds.dma_type_f = 1;
    txq_reg.flds.chnl_map_f = 0;
    REG32(TXQ_x(tx_ring_num)) = txq_reg.data;
    REG32(TXQ_x_BASE_HSN_HI(tx_ring_num))    = tx_desc_ba >> 32;
    REG32(TXQ_x_BASE_HSN_LO(tx_ring_num))    = tx_desc_ba & 0xffffffff;
    REG32(TXQ_x_BASE_HSN_HI_WB(tx_ring_num)) = tx_wb_ba >> 32;
    REG32(TXQ_x_BASE_HSN_LO_WB(tx_ring_num)) = tx_wb_ba & 0xffffffff;
    REG32(TXQ_x_DESC_RING(tx_ring_num)) = RECOVERY_RING_NUM_DESC;
    txq_pidx.flds.num_f = 1;
    REG32(TXQ_x_DESC_PIDX(tx_ring_num)) = txq_pidx.data;

    txq_cidx_update_reg.flds.timer_f    = 0;
    REG32(TXQ_x_CIDX_UPDATE(tx_ring_num)) =
        txq_cidx_update_reg.data;
    txq_cidx_update_cliff_reg.flds.timer_f    = 0;
    REG32(TXQ_x_CIDX_UPDATE_CLIFF(tx_ring_num)) =
        txq_cidx_update_cliff_reg.data;
    txq_cidx_update_prescaler_reg.flds.timer_f    = 0;
    REG32(TXQ_x_CIDX_UPDATE_PRESCALER(tx_ring_num)) =
        txq_cidx_update_prescaler_reg.data;
    txq_cidx_update_precliff_reg.flds.timer_f    = 0;
    REG32(TXQ_x_CIDX_UPDATE_PRECLIFF(tx_ring_num)) =
        txq_cidx_update_precliff_reg.data;
    txq_cidx_update_tmr_ctl_reg.flds.enable_f = 1;
    REG32(TXQ_x_CIDX_UPDATE_TMR_CTL(tx_ring_num)) =
        txq_cidx_update_tmr_ctl_reg.data;
    /* enable TX queue to start sending packets to BAM */
    inno_enable_queue_tx(idev, tx_ring_num);
    ipd_debug("TX queue enabled\n");

    rxe_total_pkt_cnt_pre = REG32(RXE_TOTAL_PKT_SIZE_CNTR);

    /* Inject the flush packets into ring 0 */
    for (i = 0; i < FLUSH_PKT_CNT; i++) {
        memset(&tx_desc[i], 0, sizeof(inno_tx_desc_t));
        tx_desc[i].hsn_upper = (uint32_t)(tx_dma_addr >> 32);
        tx_desc[i].hsn_lower = (uint32_t)(tx_dma_addr & 0x00000000ffffffff);
        tx_page_vmaddr = page_address(tx_page);
        memcpy(tx_page_vmaddr, flush_pkt->pkt, flush_pkt->pkt_len);
        tx_desc[i].length = flush_pkt->pkt_len + 4;
        tx_desc[i].sop = 1;
        tx_desc[i].eop = 1;
        ipd_debug("Injecting flush packet %d\n", i+1);
    }

    smp_mb();
    REG32(TXQ_x_DESC_PIDX(tx_ring_num)) = FLUSH_PKT_CNT;

    /* Wait for 1 msec */
    udelay(1000);
    /* Wait until the packet is drained by the HW */
    if (REG32(TXQ_x_DESC_CIDX(tx_ring_num)) != FLUSH_PKT_CNT) {
        ipd_err("Flush Packet not drained by HW\n");
        ret_val = -1;
    } else {
        ipd_debug("Packet drained by HW\n");
    }

    /* Wait for 10 milliseconds before checking the RX ring */
    msleep_interruptible(10);
    rxe_total_pkt_cnt_post = REG32(RXE_TOTAL_PKT_SIZE_CNTR);
    /* Check if the packet is received in RX Ring */
    if (REG32(HOST0_SWITCH_TO_CPU_CLEAN_PKT_SIZE_CNTR) < 2) {
        ipd_err("rxe_total_pkt_cnt_pre: %d rxe_total_pkt_cnt_post: %d\n",
                  rxe_total_pkt_cnt_pre, rxe_total_pkt_cnt_post);
        ipd_err("HOST0_SWITCH_TO_CPU_CLEAN_PKT_SIZE_CNTR: %d\n",
                REG32(HOST0_SWITCH_TO_CPU_CLEAN_PKT_SIZE_CNTR));
        ipd_err("DMA not recovered\n");
        ret_val = -1;
    }
    ipd_info("DMA recovered\n");

    /* Free up all the buffers that were allocated */
    dma_free_coherent(&idev->pdev->dev, rx_ring_size,
                      rx_desc, rx_desc_ba);
    dma_free_coherent(&idev->pdev->dev, rx_ring_size,
                      rx_wb, rx_wb_ba);
    dma_unmap_page(&idev->pdev->dev, rx_dma_addr, PAGE_SIZE, DMA_FROM_DEVICE);
    __free_page(rx_page);
    dma_free_coherent(&idev->pdev->dev, tx_ring_size,
                      tx_desc, tx_desc_ba);
    dma_free_coherent(&idev->pdev->dev, RECOVERY_RING_NUM_DESC * MIN_PACKET_SIZE,
                      tx_cksum, tx_cksum_ba);
    dma_free_coherent(&idev->pdev->dev, tx_ring_size,
                      tx_wb, tx_wb_ba);
    dma_unmap_page(&idev->pdev->dev, tx_dma_addr, PAGE_SIZE, DMA_TO_DEVICE);
    __free_page(tx_page);
    return ret_val;;
}

/** @brief Reset/cleanup function
 *
 *  @param [in] idev - innovium device
 */
static void
inno_reset(inno_device_t *idev)
{

    ipd_trace("Inno reset\n");

    if (idev == NULL) {
        return;
    }

    REG32(SYNC_MODE) = 0;                /* Stop sync WB */
    /* cleanup only if hw_init_done is true */
    if (ipd_hw_init_done == 1) {
        inno_cleanup_resources(idev);
        inno_ipd_opened_before = 0;
    }

    /* Release the coherent allocs */
    if (idev->syshdr1) {
        dma_free_coherent(&idev->pdev->dev, SYSHDR_SIZE * 2 * NUM_SYSPORTS,
                          idev->syshdr1, idev->syshdr1_ba);
        idev->syshdr1 = NULL;
    }

    if (idev->syshdr1_abp) {
        dma_free_coherent(&idev->pdev->dev, SYSHDR_SIZE * 2 * NUM_SYSPORTS,
                          idev->syshdr1_abp, idev->syshdr1_abp_ba);
        idev->syshdr1_abp = NULL;
    }

    if (idev->syshdr1_ptp) {
        dma_free_coherent(&idev->pdev->dev, SYSHDR_SIZE * NUM_SYSPORTS,
                          idev->syshdr1_ptp, idev->syshdr1_ptp_ba);
        idev->syshdr1_ptp = NULL;
    }

    if (idev->syshdr2) {
        dma_free_coherent(&idev->pdev->dev, SYSHDR_SIZE * NUM_SYSPORTS,
                          idev->syshdr2, idev->syshdr2_ba);
        idev->syshdr2 = NULL;
    }

    if (idev->tx_netdev_cksum) {
        dma_free_coherent(&idev->pdev->dev, MIN_PACKET_SIZE,
                          idev->tx_netdev_cksum, idev->tx_netdev_cksum_ba);
        idev->tx_netdev_cksum = NULL;
    }

    if (idev->pool) {
        dma_free_coherent(&idev->pdev->dev, POOL_SIZE,
                          idev->pool, idev->pool_ba);
        idev->pool = NULL;
    }

    if (idev->bar0) {
        /* Release base addr mappings */
        iounmap(idev->bar0);
        idev->bar0 = 0;
    }

    pci_disable_device(idev->pdev);
    pci_release_regions(idev->pdev);
    pci_set_drvdata(idev->pdev, NULL);
    idev->pdev = NULL;
}

/** @brief File ioctl function
 *
 *  @return ERRNO
 */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
static int
inno_ioctl(struct inode  *inode,
           struct file   *f,
           unsigned int  cmd,
           unsigned long arg)
#else
static long
inno_ioctl(struct file   *f,
           unsigned int  cmd,
           unsigned long arg)
#endif
{
    inno_device_t                 *idev;
    uint32_t                      reg_ops[2];
    int                           i;
    inno_ioctl_nodes_t            ioctl_nodes;
    inno_ioctl_query_t            ioctl_query;
    inno_ioctl_rupt_wait_t        ioctl_rupt_wait;
    inno_ioctl_rupt_mask_t        ioctl_rupt_mask;
    inno_ioctl_ring_t             ioctl_ring;
    inno_ioctl_hrr_t              ioctl_hrr;
    inno_ioctl_learn_t            ioctl_learn;
    inno_ioctl_pic_st_t           ioctl_pic_st;
    inno_ioctl_tx_send_t          ioctl_tx_send;
    inno_ioctl_unpin_pages_t      ioctl_unpin_pages;
    inno_ioctl_netdev_t           ioctl_netdev;
    inno_ioctl_netdev_info_t      ioctl_netdev_info;
    inno_ioctl_ring_page_alloc_t  ioctl_ring_page_alloc;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
    inno_ioctl_netlink_t          ioctl_netlink;
    inno_ioctl_netlink_info_t     ioctl_netlink_info;
#endif
    inno_ioctl_params_t           ioctl_params;
    inno_ioctl_flush_pkt_t        ioctl_flush_pkt;
    inno_ioctl_cfg_init_done_t    ioctl_cfg_init_done;
    inno_ioctl_flow_init_t        ioctl_flow_init;
    int ret = 0;

    /* Extract device */
    idev = f->private_data;

    ipd_verbose("Ioctl $%p %x %p %p\n", (void *)IPD_INFO_NODES, cmd, (void *) arg, f);

    switch (_IOC_NR(cmd)) {
    case _IOC_NR(IPD_INFO_NODES):
        if (copy_from_user(&ioctl_nodes, (inno_ioctl_nodes_t *)arg,
                           sizeof(inno_ioctl_nodes_t))) {
            return -EACCES;
        }

        if (ioctl_nodes.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }

        ioctl_nodes.num_nodes = inno_num_devices;
        for (i = 0; i < inno_num_devices; i++) {
            ioctl_nodes.node_info[i].vendor_id = inno_instances[i].vendor_id;
            ioctl_nodes.node_info[i].device_id = inno_instances[i].device_id;
            ioctl_nodes.node_info[i].rev_id = inno_instances[i].rev_id;
            ioctl_nodes.node_info[i].bus_num = inno_instances[i].bus_num;
            ioctl_nodes.node_info[i].dev_num = inno_instances[i].dev_num;
            ioctl_nodes.node_info[i].fn_num = inno_instances[i].fn_num;
        }

        if (copy_to_user((inno_ioctl_nodes_t *)arg, &ioctl_nodes,
                         sizeof(inno_ioctl_nodes_t))) {
            return -EACCES;
        }

        break;

    case _IOC_NR(IPD_QUERY_NODE):
        if (copy_from_user(&ioctl_query, (inno_ioctl_query_t *)arg,
                           sizeof(inno_ioctl_query_t))) {
            return -EACCES;
        }
        if (ioctl_query.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_query.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }

        if ((ioctl_query.serial_num == 0) || (ioctl_query.device_id == 0)) {
            /* No serial number or device ID given - just attach in order */
            idev = &inno_instances[ioctl_query.hdr.instance];
        } else {
            /* Scan  for a match */
            int i;
            idev = NULL;
            for (i = 0; i < MAX_INNO_DEVICES; i++) {
                if ((inno_instances[i].serial_num == ioctl_query.serial_num) &&
                    (inno_instances[i].vendor_id == ioctl_query.vendor_id) &&
                    (inno_instances[i].device_id == ioctl_query.device_id)) {
                    idev = &inno_instances[i];
                    break;
                }
            }

            if (idev == NULL) {
                return -EINVAL;
            }

            ioctl_query.hdr.instance = i;
        }

        ioctl_query.vendor_id        = idev->vendor_id;
        ioctl_query.device_id        = idev->device_id;
        ioctl_query.rev_id           = idev->rev_id;
        ioctl_query.serial_num       = idev->serial_num;
        ioctl_query.bus_num          = idev->bus_num;
        ioctl_query.dev_num          = idev->dev_num;
        ioctl_query.fn_num           = idev->fn_num;
        ioctl_query.bar0             = (off_t)idev->bar0_ba;
        ioctl_query.bar0_size        = idev->bar0_size;
        ioctl_query.pool_baddr       = (off_t)idev->pool_ba;
        ioctl_query.pool_paddr       = (off_t)virt_to_phys(idev->pool);
        ioctl_query.pool_size        = POOL_SIZE;
        ioctl_query.sync_blk_offset  = POOL_IAC_BLK_OFFSET;
        ioctl_query.sync_blk_size    = 4096;
        ioctl_query.sync_wb_offset   = POOL_IAC_WB_OFFSET;
        ioctl_query.sync_wb_size     = SYNC_WB_SIZE;
        ioctl_query.rupt_wb_offset   = POOL_RUPT_OFFSET;
        ioctl_query.rupt_wb_size     = 256 >> 3; /* 256 bits */
        ioctl_query.dma_cache_align  = idev->cache_align;
        strncpy(ioctl_query.version, ipd_version, VERSION_SIZE);

        if (copy_to_user((inno_ioctl_query_t *)arg, &ioctl_query,
                         sizeof(inno_ioctl_query_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_RUPT_MASK):
        if (copy_from_user(&ioctl_rupt_mask, (inno_ioctl_rupt_mask_t *)arg,
                           sizeof(inno_ioctl_rupt_mask_t))) {
            return -EACCES;
        }
        if (ioctl_rupt_mask.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_rupt_mask.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }

        ret = inno_rupt_mask(idev, &ioctl_rupt_mask);
        if (ret) {
            return ret;
        }

        break;

    case _IOC_NR(IPD_RUPT_WAIT):
        if (copy_from_user(&ioctl_rupt_wait, (inno_ioctl_rupt_wait_t *)arg,
                           sizeof(inno_ioctl_rupt_wait_t))) {
            return -EACCES;
        }
        if (ioctl_rupt_wait.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_rupt_wait.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }

        ret = inno_rupt_wait(idev, &ioctl_rupt_wait);
        if (ret) {
            return ret;
        }

        if (copy_to_user((inno_ioctl_rupt_wait_t *)arg, &ioctl_rupt_wait,
                         sizeof(inno_ioctl_rupt_wait_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_READ_MEM):
        if (copy_from_user((int *)reg_ops, (int *)arg, sizeof(uint32_t) * 2)) {
            ipd_err("Read Unable to get from user\n");
            return -EACCES;
        }
        //reg_ops[1] = *((uint32_t *)((char *) idev->bar0 + reg_ops[0]));
        reg_ops[1] = *((uint32_t *)(idev->pool + reg_ops[0]));
        if (copy_to_user((int *)arg, reg_ops, sizeof(uint32_t) * 2)) {
            ipd_err("Read Unable to put to user\n");
            return -EACCES;
        }
        break;

#ifdef EVT_THREAD
    case _IOC_NR(IPD_DEBUG_EVT_THREAD_TIMER):
        if (copy_from_user((int *)reg_ops, (int *)arg, sizeof(uint32_t) * 2)) {
            ipd_err("IPD_DEBUG_EVT_THREAD_TIMER: Read Unable to get from user\n");
            return -EACCES;
        }

	evt_thread_timer = reg_ops[0];
	exit_evt_thread_hndlr = reg_ops[1];
	ipd_debug("Evt_thread_timer set to %d msec exit_handler: %d\n", evt_thread_timer, exit_evt_thread_hndlr);
	break;
#endif

    case _IOC_NR(IPD_WRITE_MEM):
        if (copy_from_user((int *)reg_ops, (int *)arg, sizeof(uint32_t) * 2)) {
            ipd_err("write Unable to get from user\n");
            return -EACCES;
        }
        *((uint32_t *)((char *)idev->bar0 + reg_ops[0])) = reg_ops[1];
        break;

    case _IOC_NR(IPD_RING_ALLOC):
        if (copy_from_user(&ioctl_ring, (inno_ioctl_ring_t *)arg,
                           sizeof(inno_ioctl_ring_t))) {
            return -EACCES;
        }
        if (ioctl_ring.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_ring.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_alloc_ring(idev, &ioctl_ring);
        if (ret) {
            return ret;
        }

        if (copy_to_user((inno_ioctl_ring_t *)arg, &ioctl_ring,
                         sizeof(inno_ioctl_ring_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_RING_FREE):
        if (copy_from_user(&ioctl_ring, (inno_ioctl_ring_t *)arg,
                           sizeof(inno_ioctl_ring_t))) {
            return -EACCES;
        }
        if (ioctl_ring.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_ring.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_free_ring(idev, ioctl_ring.flags & INNO_RING_TX,
                             ioctl_ring.num);
        if (ret) {
            return ret;
        }

        if (copy_to_user((inno_ioctl_ring_t *)arg, &ioctl_ring,
                         sizeof(inno_ioctl_ring_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_RING_ENABLE):
        if (copy_from_user(&ioctl_ring, (inno_ioctl_ring_t *)arg,
                           sizeof(inno_ioctl_ring_t))) {
            return -EACCES;
        }
        if (ioctl_ring.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_ring.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }

        if(ioctl_ring.flags & INNO_RING_TX) {
           ret = inno_enable_queue_tx(idev, ioctl_ring.num);
        } else {
           ret = inno_enable_queue_rx(idev, ioctl_ring.num);
       }

       if (ret) {
           return ret;
       }

       if (copy_to_user((inno_ioctl_ring_t *)arg, &ioctl_ring,
                         sizeof(inno_ioctl_ring_t))) {
           return -EACCES;
       }
       break;

    case _IOC_NR(IPD_RING_DISABLE):
        if (copy_from_user(&ioctl_ring, (inno_ioctl_ring_t *)arg,
                           sizeof(inno_ioctl_ring_t))) {
            return -EACCES;
        }
        if (ioctl_ring.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_ring.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }

        if(ioctl_ring.flags & INNO_RING_TX) {
           ret = inno_disable_queue_tx(idev, ioctl_ring.num);
        } else {
           ret = inno_disable_queue_rx(idev, ioctl_ring.num);
       }

       if (ret) {
           return ret;
       }

       if (copy_to_user((inno_ioctl_ring_t *)arg, &ioctl_ring,
                         sizeof(inno_ioctl_ring_t))) {
           return -EACCES;
       }
       break;

    case _IOC_NR(IPD_HRR_ALLOC):
        if (copy_from_user(&ioctl_hrr, (inno_ioctl_hrr_t *)arg,
                           sizeof(inno_ioctl_hrr_t))) {
            return -EACCES;
        }
        if (ioctl_hrr.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_hrr.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        smp_mb();
        ret = inno_alloc_hrr(idev, &ioctl_hrr);
        if (ret) {
            return ret;
        }
        smp_mb();

        if (copy_to_user((inno_ioctl_hrr_t *)arg, &ioctl_hrr,
                         sizeof(inno_ioctl_hrr_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_HRR_FREE):
        if (copy_from_user(&ioctl_hrr, (inno_ioctl_hrr_t *)arg,
                           sizeof(inno_ioctl_hrr_t))) {
            return -EACCES;
        }
        if (ioctl_hrr.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_hrr.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_free_hrr(idev);
        if (ret) {
            return ret;
        }

        if (copy_to_user((inno_ioctl_hrr_t *)arg, &ioctl_hrr,
                         sizeof(inno_ioctl_hrr_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_LEARN_ALLOC):
        if (copy_from_user(&ioctl_learn, (inno_ioctl_learn_t *)arg,
                           sizeof(inno_ioctl_learn_t))) {
            return -EACCES;
        }
        if (ioctl_learn.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_learn.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_alloc_learn(idev, &ioctl_learn);
        if (ret) {
            return ret;
        }

        if (copy_to_user((inno_ioctl_learn_t *)arg, &ioctl_learn,
                         sizeof(inno_ioctl_learn_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_LEARN_FREE):
        if (copy_from_user(&ioctl_learn, (inno_ioctl_learn_t *)arg,
                           sizeof(inno_ioctl_learn_t))) {
            return -EACCES;
        }
        if (ioctl_learn.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_learn.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_free_learn(idev);
        if (ret) {
            return ret;
        }

        if (copy_to_user((inno_ioctl_learn_t *)arg, &ioctl_learn,
                         sizeof(inno_ioctl_learn_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_TX_SEND):
        if (copy_from_user(&ioctl_tx_send, (inno_ioctl_tx_send_t *)arg,
                           sizeof(inno_ioctl_tx_send_t))) {
            return -EACCES;
        }
        if (ioctl_tx_send.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_tx_send.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_tx_send(idev, &ioctl_tx_send);
        if (ret) {
            return ret;
        }

        if (copy_to_user((inno_ioctl_tx_send_t *)arg, &ioctl_tx_send,
                         sizeof(inno_ioctl_tx_send_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_UNPIN_PAGE):
        if (copy_from_user(&ioctl_unpin_pages, (inno_ioctl_unpin_pages_t *)arg,
                           sizeof(inno_ioctl_unpin_pages_t))) {
            return -EACCES;
        }
        if (ioctl_unpin_pages.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_unpin_pages.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_unpin_page(idev, &ioctl_unpin_pages);
        if (ret) {
            return ret;
        }

        if (copy_to_user((inno_ioctl_unpin_pages_t *)arg, &ioctl_unpin_pages,
                         sizeof(inno_ioctl_unpin_pages_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_CREATE_NETDEV):
        if (copy_from_user(&ioctl_netdev, (inno_ioctl_netdev_t *)arg,
                           sizeof(inno_ioctl_netdev_t))) {
            return -EACCES;
        }
        if (ioctl_netdev.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_netdev.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_netdev_create(idev, &ioctl_netdev);
        if (ret) {
            return ret;
        }
        idev->inno_netdev.num_interfaces++;

        break;

    case _IOC_NR(IPD_DELETE_NETDEV):
        if (copy_from_user(&ioctl_netdev, (inno_ioctl_netdev_t *)arg,
                           sizeof(inno_ioctl_netdev_t))) {
            return -EACCES;
        }
        if (ioctl_netdev.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_netdev.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        if (ipd_boottype != IPD_BOOTTYPE_FAST){
            ret = inno_netdev_delete(idev, ioctl_netdev.sysport);
            if (ret) {
                return ret;
            }
            idev->inno_netdev.num_interfaces--;
        }

        break;

    case _IOC_NR(IPD_SET_NETDEV):
        if (copy_from_user(&ioctl_netdev_info, (inno_ioctl_netdev_info_t *)arg,
                           sizeof(inno_ioctl_netdev_info_t))) {
            return -EACCES;
        }
        if (ioctl_netdev_info.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_netdev_info.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_netdev_set(idev, ioctl_netdev_info);
        if (ret) {
            return ret;
        }

        break;

    case _IOC_NR(IPD_GET_NETDEV):
        if (copy_from_user(&ioctl_netdev_info, (inno_ioctl_netdev_info_t *)arg,
                           sizeof(inno_ioctl_netdev_info_t))) {
            return -EACCES;
        }
        if (ioctl_netdev_info.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_netdev_info.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_netdev_get(idev, &ioctl_netdev_info);
        if (ret) {
            return ret;
        }

        /* Copy it back to the user */
        if (copy_to_user((inno_ioctl_netdev_info_t*)arg,
                         &ioctl_netdev_info,
                         sizeof(inno_ioctl_netdev_info_t))) {
            return -EACCES;
        }

        break;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
    case _IOC_NR(IPD_CREATE_NETLINK):
        if (copy_from_user(&ioctl_netlink, (inno_ioctl_netlink_t *)arg,
                           sizeof(inno_ioctl_netlink_t))) {
            return -EACCES;
        }
        if (ioctl_netlink.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_netlink.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_netlink_create(idev, &ioctl_netlink);
        if (ret) {
            return ret;
        }

        break;

    case _IOC_NR(IPD_DELETE_NETLINK):
        if (copy_from_user(&ioctl_netlink, (inno_ioctl_netlink_t *)arg,
                           sizeof(inno_ioctl_netlink_t))) {
            return -EACCES;
        }
        if (ioctl_netlink.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_netlink.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_netlink_delete(idev, &ioctl_netlink);
        if (ret) {
            return ret;
        }

        break;

    case _IOC_NR(IPD_SET_NETLINK):
        if (copy_from_user(&ioctl_netlink_info, (inno_ioctl_netlink_info_t *)arg,
                           sizeof(inno_ioctl_netlink_info_t))) {
            return -EACCES;
        }
        if (ioctl_netlink_info.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_netlink_info.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_netlink_set(idev, ioctl_netlink_info);
        if (ret) {
            return ret;
        }

        break;
#endif /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0) */

    case _IOC_NR(IPD_RING_PAGE_ALLOC):
        if (copy_from_user(&ioctl_ring_page_alloc,
                           (inno_ioctl_ring_page_alloc_t *)arg,
                           sizeof(inno_ioctl_ring_page_alloc_t))) {
            return -EACCES;
        }
        if (ioctl_ring_page_alloc.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_ring_page_alloc.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_ring_page_alloc(idev, &ioctl_ring_page_alloc);
        if (ret) {
            return ret;
        }

        /* Copy it back to the user */
        if (copy_to_user((inno_ioctl_ring_page_alloc_t *)arg,
                         &ioctl_ring_page_alloc,
                         sizeof(inno_ioctl_ring_page_alloc_t))) {
            return -EACCES;
        }

        break;

    case _IOC_NR(IPD_RING_PAGE_FREE):
        if (copy_from_user(&ioctl_ring_page_alloc,
                           (inno_ioctl_ring_page_alloc_t *)arg,
                           sizeof(inno_ioctl_ring_page_alloc_t))) {
            return -EACCES;
        }
        if (ioctl_ring_page_alloc.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_ring_page_alloc.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_ring_page_free(idev, &ioctl_ring_page_alloc);
        if (ret) {
            return ret;
        }

        break;

    case _IOC_NR(IPD_PIC_STATUS_ALLOC):
        if (copy_from_user(&ioctl_pic_st, (inno_ioctl_pic_st_t *)arg,
                           sizeof(inno_ioctl_pic_st_t))) {
            return -EACCES;
        }
        if (ioctl_pic_st.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_pic_st.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_pic_st_alloc(idev, &ioctl_pic_st);
        if (ret) {
            return ret;
        }

        if (copy_to_user((inno_ioctl_pic_st_t *)arg, &ioctl_pic_st,
                         sizeof(inno_ioctl_pic_st_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_PIC_STATUS_FREE):
        if (copy_from_user(&ioctl_pic_st, (inno_ioctl_pic_st_t *)arg,
                           sizeof(inno_ioctl_pic_st_t))) {
            return -EACCES;
        }
        if (ioctl_pic_st.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_pic_st.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_pic_st_free(idev);
        if (ret) {
            return ret;
        }

        if (copy_to_user((inno_ioctl_pic_st_t *)arg, &ioctl_pic_st,
                         sizeof(inno_ioctl_pic_st_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_PIC_STATUS_DUMP):
        if (copy_from_user(&ioctl_pic_st, (inno_ioctl_pic_st_t *)arg,
                           sizeof(inno_ioctl_pic_st_t))) {
            return -EACCES;
        }
        if (ioctl_pic_st.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_pic_st.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ret = inno_pic_st_dump(idev);
        if (ret) {
            return ret;
        }

        if (copy_to_user((inno_ioctl_pic_st_t *)arg, &ioctl_pic_st,
                         sizeof(inno_ioctl_pic_st_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_GET_PARAMS):
        if (copy_from_user(&ioctl_params, (inno_ioctl_params_t *)arg,
                           sizeof(inno_ioctl_params_t))) {
            return -EACCES;
        }
        if (ioctl_params.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_params.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        ioctl_params.info.shim_gre_proto = idev->inno_params_info.shim_gre_proto;
        ioctl_params.info.rsvd = idev->inno_params_info.rsvd;
        if (copy_to_user((inno_ioctl_params_t *)arg, &ioctl_params,
                         sizeof(inno_ioctl_params_t))) {
            return -EACCES;
        }
        /* ipd_err( "%s:TESTING, IPD get params - shim_gre_proto 0x%x rsvd %d\n", __FUNCTION__, ioctl_params.info.shim_gre_proto, ioctl_params.info.rsvd); */
        break;

    case _IOC_NR(IPD_SET_PARAMS):
        if (copy_from_user(&ioctl_params, (inno_ioctl_params_t *)arg,
                           sizeof(inno_ioctl_params_t))) {
            return -EACCES;
        }
        if (ioctl_params.hdr.instance > inno_num_devices) {
            return -EINVAL;
        }
        if (ioctl_params.hdr.version != IFCS_PCIE_IOCTL_VERSION) {
            return -EINVAL;
        }
        idev->inno_params_info.shim_gre_proto = ioctl_params.info.shim_gre_proto;
        idev->inno_params_info.rsvd = ioctl_params.info.rsvd;
        /* ipd_err( "%s:TESTING, IPD set params - shim_gre_proto 0x%x rsvd %d\n", __FUNCTION__, ioctl_params.info.shim_gre_proto, ioctl_params.info.rsvd); */
        break;

    case _IOC_NR(IPD_DMA_INIT):
        ret = inno_dma_init(idev);
        if (ret) {
            return ret;
        }
        break;

    case _IOC_NR(IPD_FLUSH_RECOVER_DMA):
        if (copy_from_user(&ioctl_flush_pkt, (inno_ioctl_flush_pkt_t *)arg,
                           sizeof(inno_ioctl_flush_pkt_t))) {
            return -EACCES;
        }
        ipd_debug("IPD_FLUSH_RECOVER_DMA called\n");

        ret = inno_flush_recover_dma(idev, &ioctl_flush_pkt);
        if (ret) {
            return ret;
        }
        break;

    case _IOC_NR(IPD_HW_INIT):
        ret = inno_hw_init(idev);
        if (ret) {
            return ret;
        }
        break;

    case _IOC_NR(IPD_IS_USER_CONFIG_DONE):
        if (copy_from_user(&ioctl_cfg_init_done, (inno_ioctl_cfg_init_done_t *)arg,
                           sizeof(inno_ioctl_cfg_init_done_t))) {
            return -EACCES;
        }
        ioctl_cfg_init_done.value = ipd_user_cfg_done;

        if (copy_to_user((inno_ioctl_cfg_init_done_t *)arg, &ioctl_cfg_init_done,
                         sizeof(inno_ioctl_cfg_init_done_t))) {
            return -EACCES;
        }
        break;

    case _IOC_NR(IPD_USER_CONFIG_DONE):
        ipd_user_cfg_done = 1;
        break;

    case _IOC_NR(IPD_USER_CONFIG_CLEARED):
        ipd_user_cfg_done = 0;
        break;

    case _IOC_NR(IPD_OVERRIDE_FLOW_CONTROL):
        if (copy_from_user(&ioctl_flow_init, (inno_ioctl_flow_init_t *)arg,
                           sizeof(inno_ioctl_flow_init_t))) {
            return -EACCES;
        }
        ret = inno_override_flow_control(idev, ioctl_flow_init.value);
        if (ret) {
            return ret;
        }
        break;

    case _IOC_NR(IPD_CLEANUP_RESOURCES):
        ret = inno_cleanup_resources(idev);
        if (ret) {
            return ret;
        }
        break;

    default:
        ipd_warn("Bad ioctl. cmd: %d\n", cmd);
        return -EINVAL;
    }

    return 0;
}


#define RUPT_OFFSET(word) INTR_TRIG_OFFSET_ ## word >> 5

#define RUPT_WORD(word) rupt_mask[RUPT_OFFSET(word)]

#define RUPT_GET(word, suffix)             \
    RUPT_WORD(word) & (1 << INTR_TRIG_OFFSET_ ## word ## suffix)

#define RUPT_SET(word, suffix)             \
    RUPT_WORD(word) |=  (1 << INTR_TRIG_OFFSET_ ## word ## suffix)

#define RUPT_CLEAR(word, suffix)             \
    RUPT_WORD(word) &=  ~(1 << INTR_TRIG_OFFSET_ ## word ## suffix)


/** @brief Generic fast interrupt handler
 *
 *  @return ERRNO
 */
static irqreturn_t
inno_rupt_handler(int  irq, void *dev_id)
{
    inno_device_t       *idev = dev_id;
    unsigned long       flags;
    int                 i, j;
    uint32_t            cur_rupts[RUPT_MASK_WORDS_MAX];

    /* Copy the interrupts from the WB */
    memcpy(cur_rupts, idev->pool + POOL_RUPT_OFFSET, rupt_mask_words * 4);

    spin_lock_irqsave(&idev->rupt_lock, flags);
    idev->inno_stats.inno_rupt_stats.num_int[0]++;
    spin_unlock_irqrestore(&idev->rupt_lock, flags);

    /* See which vectors need a kick */
    for (i = 0; i < NUM_MSIX_VECS; i++) {
        spin_lock_irqsave(&idev->rupt_lock, flags);
        for (j = 0; j < rupt_mask_words; j++) {
            REG32(idev->inno_intr_regs.intr_inmc + j * 4) = idev->rupts[i].mask[j];
            if (idev->rupts[i].flag == 0) {
                /* Need to kick this thread */
                idev->rupts[i].flag = 1;
                idev->inno_stats.inno_rupt_stats.num_int[i]++;
                wake_up_interruptible(&idev->rupts[i].wait_q);
            }
        }
        napi_schedule(&idev->napi);
        spin_unlock_irqrestore(&idev->rupt_lock, flags);
    }

    return IRQ_HANDLED;
}

/** @brief Generic fast interrupt handler
 *
 *  @return ERRNO
 */
static irqreturn_t
_inno_rupt_handler(int  irq, void *dev_id)
{
    inno_device_t       *idev = dev_id;
    unsigned long       flags;
    int                 i, j;
    uint32_t            cur_rupts[RUPT_MASK_WORDS_MAX];

    /* Copy the interrupts from the WB */
    memcpy(cur_rupts, idev->pool + POOL_RUPT_OFFSET, rupt_mask_words * 4);

    /* See which vectors need a kick */
    for (i = 0; i < NUM_MSIX_VECS; i++) {
        if (idev->msix[i].vector == irq) {
            spin_lock_irqsave(&idev->rupt_lock, flags);
            idev->inno_stats.inno_rupt_stats.num_int[i]++;
            if (idev->rupts[i].flag == 0) {             /* Skip if pending */
                for (j = 0; j < rupt_mask_words; j++) {
                    idev->rupts[i].pend[j] = cur_rupts[j] & idev->rupts[i].mask[j];
                    if (idev->rupts[i].pend[j] != 0) {
                        REG32(idev->inno_intr_regs.intr_inmc + j * 4) = idev->rupts[i].mask[j];
                        if (idev->rupts[i].flag == 0) {
                            /* Need to kick this thread */
                            idev->rupts[i].flag = 1;
                            idev->inno_stats.inno_rupt_stats.num_int[i]++;
                            wake_up_interruptible(&idev->rupts[i].wait_q);
                        }
                    }
                }
            }
            spin_unlock_irqrestore(&idev->rupt_lock, flags);
        }
    }

    return IRQ_HANDLED;
}

#ifdef EVT_THREAD
int evt_thread_hndlr(void *dev_id)
{
	inno_device_t       *idev = dev_id;
	unsigned long       flags;
	uint32_t            cur_rupts[RUPT_MASK_WORDS_MAX];
	int                 i, j;

	ipd_debug("In %s\n", __FUNCTION__);
	while(exit_evt_thread_hndlr == 0) {
        memcpy(cur_rupts, idev->pool + POOL_RUPT_OFFSET, rupt_mask_words * 4);
		for (i = 0; i < NUM_MSIX_VECS; i++) {
                if (1) /* idev->msix[i].vector == irq) */ {
                    spin_lock_irqsave(&idev->rupt_lock, flags);
                    if(idev->rupts[i].flag != 0)
                        ipd_debug("rupts[%d].flag 0x%x\n", i, idev->rupts[i].flag);
                    if (idev->rupts[i].flag == 0) {             /* Skip if pending */
                        for (j = 0; j < rupt_mask_words; j++) {
                            idev->rupts[i].pend[j] = cur_rupts[j] & idev->rupts[i].mask[j];
                                if(idev->rupts[i].pend[j] != 0)
                                    ipd_debug("rupts[%d].pend[%d] 0x%x\n",i, j, idev->rupts[i].pend[j]);
                            if (idev->rupts[i].pend[j] != 0) {
                                /* Mask off these interrupts */
                                REG32(idev->inno_intr_regs.intr_inmc + j * 4) = idev->rupts[i].mask[j];
                                if (idev->rupts[i].flag == 0) {
                                    /* Need to kick this thread */
                                    idev->rupts[i].flag = 1;
                                    ipd_debug("Waking up rupts[%d].wait_q %s\n", i, idev->msix_names[i]);
                                    wake_up_interruptible(&idev->rupts[i].wait_q);
                                }
                            }
                        }
                    }
                    spin_unlock_irqrestore(&idev->rupt_lock, flags);
                }
            }

        msleep_interruptible(evt_thread_timer);
    }

	ipd_debug("%s: exit_evt_thread_hndlr set to %d; exiting\n", __FUNCTION__, exit_evt_thread_hndlr);
	return 0;
}
#endif

/** @brief Fast interrupt handler for NAPI
 *
 *  @return ERRNO
 */
static irqreturn_t
_inno_napi_handler(int  irq, void *dev_id)
{
    inno_device_t  *idev = dev_id;
    int            i;

    /* Disable and clear the interrupts */
    for (i = 0; i < rupt_mask_words; i++) {
        if (idev->napi_mask[i] != 0) {
            REG32(idev->inno_intr_regs.intr_inmc + i * 4) = idev->napi_mask[i];
            REG32(idev->inno_intr_regs.intr_incr + i * 4) = idev->napi_mask[i];
        }
    }

    idev->inno_stats.inno_rupt_stats.num_int[MSIX_VECTOR_NAPI]++;
    napi_schedule(&idev->napi);

    return IRQ_HANDLED;
}


static int inno_msi_workaround(inno_device_t *idev)
{
	uint32_t msi_mask=0;
	uint32_t msi_data=0, bot_bits = 0;
	int msi_offset, msi_data_offset, msi_mask_offset;

    /* MSI workaround applicable only for Teralynx A0 chip */
    if(idev->device_id == INNO_TERALYNX_PCI_DEVICE_ID) {
        if (idev->rev_id != INNO_TERALYNX_PCI_DEVICE_REV_ID_A0) {
            return 0;
        }
    }

	msi_offset = pci_find_capability(idev->pdev, PCI_CAP_ID_MSI);

    

    msi_data_offset = msi_offset + 12;
    msi_mask_offset = msi_offset + 16;

	ipd_debug("MSI offset: 0x%x\n", msi_offset);
	pci_read_config_dword(idev->pdev, msi_data_offset, &msi_data);
	pci_read_config_dword(idev->pdev, msi_mask_offset, &msi_mask);
	ipd_debug("1. msi_data 0x%x data_offset: %d mask 0x%x mask_offset: %d\n",
             msi_data, msi_data_offset, msi_mask, msi_mask_offset);

    bot_bits = msi_data & 0x1f;

    msi_mask = 0xffffffff & ~(1 << bot_bits);
	pci_write_config_dword(idev->pdev, msi_mask_offset, msi_mask);
    idev->msi_wa_vector = bot_bits;

    ipd_debug("Writing mask 0x%x\n", msi_mask);
    smp_mb();
	pci_read_config_dword(idev->pdev, msi_data_offset, &msi_data);
	pci_read_config_dword(idev->pdev, msi_mask_offset, &msi_mask);
	ipd_debug("2. msi_data 0x%x mask 0x%x bot_bits: 0x%x\n",
             msi_data, msi_mask, bot_bits);
    idev->flags |= INNO_FLG_MSI_WA_ENABLED;
	return 0;
}

static int inno_hw_swaps_clear(inno_device_t *idev)
{
    uint32_t status=0;
    rxe_imsg_t         rxe_imsg;
    rxe_imsg_wrback_t  rxe_imsg_wrback;
    desc_wrback_t      desc_wrback;
    desc_read_t        desc_read;
    desc_cidx_wrback_t desc_cidx_wrback;
    endian_swap_dis_pcie_tar_t endian_swap;

    /* Disable the hw swap */
    rxe_imsg.flds.endianess_f = 0;
    REG32(RXE_IMSG) = rxe_imsg.data;
    rxe_imsg_wrback.flds.endianess_f = 0;
    REG32(RXE_IMSG_WRBACK) = rxe_imsg_wrback.data;
    desc_wrback.flds.endianess_f = 0;
    REG32(DESC_WRBACK) = desc_wrback.data;
    desc_read.flds.endianess_f = 0;
    REG32(DESC_READ) = desc_read.data;
    desc_cidx_wrback.flds.endianess_f = 0;
    REG32(DESC_CIDX_WRBACK) = desc_cidx_wrback.data;
    endian_swap.data = 0xffffffff;
    REG32(ENDIAN_SWAP_DIS_PCIE_TAR) = endian_swap.data;
    REG32(ENDIAN_SWAP_DIS_IAC_CNTG) = endian_swap.data;
    REG32(ENDIAN_SWAP_DIS_IAC_AXIS) = endian_swap.data;
    REG32(ENDIAN_SWAP_DIS_IAC_AXIM) = endian_swap.data;
    REG32(ENDIAN_SWAP_DIS_MSI_WB) = endian_swap.data;
    REG32(ENDIAN_SWAP_DIS_PCIE_MTR) = endian_swap.data;
    smp_mb();
    status = REG32(ENDIAN_SWAP_STS_PCIE_TAR);
    ipd_debug("%s: Swap status: 0x%x\n", __FUNCTION__, status);
    smp_mb();
    return 0;
}

static int inno_hw_swaps_enable(inno_device_t *idev)
{
    uint32_t status=0;

    rxe_imsg_t         rxe_imsg;
    rxe_imsg_wrback_t  rxe_imsg_wrback;
    desc_wrback_t      desc_wrback;
    desc_read_t        desc_read;
    desc_cidx_wrback_t desc_cidx_wrback;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    endian_swap_dis_pcie_tar_t endian_swap;
#else
    endian_swap_ena_pcie_tar_t endian_swap;
#endif

    /* Set the endianess */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    rxe_imsg.flds.endianess_f = 0;
    REG32(RXE_IMSG) = rxe_imsg.data;
    rxe_imsg_wrback.flds.endianess_f = 0;
    REG32(RXE_IMSG_WRBACK) = rxe_imsg_wrback.data;
    desc_wrback.flds.endianess_f = 0;
    REG32(DESC_WRBACK) = desc_wrback.data;
    desc_read.flds.endianess_f = 0;
    REG32(DESC_READ) = desc_read.data;
    desc_cidx_wrback.flds.endianess_f = 0;
    REG32(DESC_CIDX_WRBACK) = desc_cidx_wrback.data;
    endian_swap.data = 0;
    REG32(ENDIAN_SWAP_DIS_PCIE_TAR) = endian_swap.data;
#else
    ipd_debug("In Big Endian\n");
    //rxe_imsg.flds.endianess_f = 0xffffffff;
    rxe_imsg.flds.endianess_f = 0x0;
    REG32(RXE_IMSG) = rxe_imsg.data;
    rxe_imsg_wrback.flds.endianess_f = 0xffffffff;
    REG32(RXE_IMSG_WRBACK) = rxe_imsg_wrback.data;
    desc_wrback.flds.endianess_f = 0xffffffff;
    REG32(DESC_WRBACK) = desc_wrback.data;
    desc_read.flds.endianess_f = 0xffffffff;
    REG32(DESC_READ) = desc_read.data;
    desc_cidx_wrback.flds.endianess_f = 0xffffffff;
    REG32(DESC_CIDX_WRBACK) = desc_cidx_wrback.data;
    endian_swap.data = 0xffffffff;
    REG32(ENDIAN_SWAP_ENA_PCIE_TAR) = endian_swap.data;
    //REG32(ENDIAN_SWAP_DIS_PCIE_TAR) = endian_swap.data;
    //REG32(ENDIAN_SWAP_ENA_PCIE_TAR) = 0x0;
    REG32(ENDIAN_SWAP_ENA_MSI_WB) = endian_swap.data;
    REG32(ENDIAN_SWAP_ENA_IAC_AXIM) = endian_swap.data;
    //REG32(ENDIAN_SWAP_ENA_IAC_AXIS) = endian_swap.data;
    //REG32(ENDIAN_SWAP_ENA_IAC_CNTG) = endian_swap.data;
#endif
    smp_mb();
    status = REG32(ENDIAN_SWAP_STS_PCIE_TAR);
    ipd_debug("%s: Swap status: 0x%x\n", __FUNCTION__, status);
    return 0;
}

static int inno_syncmode_config(inno_device_t *idev)
{
    sync_mode_t        sync_mode;
    int err=0;
    uint32_t status=0;

    memset(&sync_mode, 0, sizeof(sync_mode_t));
    /* Allocate a pool for consumer/producer pointers */
    idev->pool = (void *)dma_alloc_coherent(&idev->pdev->dev, POOL_SIZE,
                                            &idev->pool_ba, GFP_KERNEL);
    if (idev->pool == NULL) {
        ipd_err( "%s:Error, Unable to allocate kernel memory for IDX DMA\n", __FUNCTION__);
        err = -1;
        goto dma_buf_fail;
    }
    ipd_debug("Pool: %p pool_ba: %p pool-size: 0x%x\n", (void *)idev->pool, (void *)idev->pool_ba, (unsigned int)POOL_SIZE);

    /* Set up the syn response writeback area */
    REG32(HSR_ADR_HI) = UPPER32(idev->pool_ba + POOL_IAC_WB_OFFSET);
    REG32(HSR_ADR_LO) = LOWER32(idev->pool_ba + POOL_IAC_WB_OFFSET);
    ipd_debug("Upper: 0x%x lower: 0x%x; POOL_IAC_WB_OFFSET: 0x%x\n",
              UPPER32(idev->pool_ba + POOL_IAC_WB_OFFSET),
              LOWER32(idev->pool_ba + POOL_IAC_WB_OFFSET), POOL_IAC_WB_OFFSET);
    smp_wmb();

    sync_mode.flds.host_rsp_mode_f = 1;
    ipd_debug("Writing into SYNC_MODE 0x%x\n", sync_mode.data);
    REG32(SYNC_MODE)  = sync_mode.data;
    smp_mb();

    status = REG32(SYNC_MODE);
    ipd_debug("Reading back SYNC_MODE 0x%x RUPT_OFFSET: %d POOL_END: %d\n", status, POOL_RUPT_OFFSET, POOL_END);
    smp_mb();

    /* Setup the IAC block read/write buffer */
    REG32(SYNC_HSN_ADR_HI_0) = UPPER32(idev->pool_ba + POOL_IAC_BLK_OFFSET);
    REG32(SYNC_HSN_ADR_LO_0) = LOWER32(idev->pool_ba + POOL_IAC_BLK_OFFSET);
    ipd_debug("Upper: 0x%x lower: 0x%x; POOL_IAC_BLK_OFFSET: 0x%x\n",
              UPPER32(idev->pool_ba + POOL_IAC_BLK_OFFSET),
              LOWER32(idev->pool_ba + POOL_IAC_BLK_OFFSET), POOL_IAC_BLK_OFFSET);
    smp_wmb();

    /* Set up interrupt writeback*/
    REG32(INTR_WB_AHI) = UPPER32(idev->pool_ba + POOL_RUPT_OFFSET);
    REG32(INTR_WB_ALO) = LOWER32(idev->pool_ba + POOL_RUPT_OFFSET);
    REG32(INTR_WB_CTL) = 1;

    /* Clear the PCIe link down bit */
    REG32(PCI_AXI_LINK_DOWN_INDICATOR_BIT_L0) = 0x0;

dma_buf_fail:
    return err;
}

/*****************************************************************
 *
 * File operations
 *
 */


/** @brief Device probe function
 *
 *  @param [in] pdev - PCIe device
 *  @param [in] ent  - PCIe device ID
 *  @return ERRNO
 */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
static int __init
inno_probe(struct pci_dev             *pdev,
           const struct pci_device_id *ent)
#else
static int
inno_probe(struct pci_dev             *pdev,
           const struct pci_device_id *ent)
#endif
{
    int           err, offset;
    inno_device_t *idev;
    uint32_t      jtag_idcode = 0;

    ipd_trace("Inno Probe\n");
    /* Check for Innovium supported devices() */
    if (inno_num_devices == MAX_INNO_DEVICES) {
        return -1;
    }
    ipd_debug("Inno Probe: inno_num_devices: %d\n", inno_num_devices);

    /* Discovered device, start populating the node information */
    idev = &inno_instances[inno_num_devices];
    ipd_debug("pdev: %p\n", pdev);
    ipd_debug("idev: %p\n", idev);

    /* Init the idev area */
    memset(idev, 0, sizeof(inno_device_t));
    idev->instance = inno_num_devices;
    spin_lock_init(&idev->rupt_lock);
    spin_lock_init(&idev->lock);

    idev->cache_align = dma_get_cache_alignment();

    /* Enable DMA Master */
    pci_set_master(pdev);

    /* Get the device identification info from config space */
    pci_read_config_word(pdev, 0, &idev->vendor_id);
    pci_read_config_word(pdev, 2, &idev->device_id);
    pci_read_config_byte(pdev, 8, &idev->rev_id);
    offset = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_DSN);
    if (offset == 0) {
        ipd_err("Missing serial number for device\n");
    } else {
        uint32_t low, high;
        pci_read_config_dword(pdev, offset + 4, &low);
        pci_read_config_dword(pdev, offset + 8, &high);
        idev->serial_num = (((uint64_t)high) << 32) | low;
    }

    if (idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
        rupt_mask_words = 10;
    } else {
        rupt_mask_words = 8;
    }

    idev->bus_num = pdev->bus->number;
    idev->dev_num =  ((pdev->devfn)>>3)&0x7;
    idev->fn_num = (pdev->devfn)&0xf8;

    /* Mark BAR_0 as reserved */
    err = pci_request_region(pdev, BAR_0, inno_driver_name);
    if (err) {
        return err;
    }

    /* Remap BAR0 MMIO region */
    idev->bar0 = pci_ioremap_bar(pdev, BAR_0);
    if (idev->bar0 == NULL) {
        ipd_err("Remap of bar0 failed\n");
        return -ENOMEM;
    }
    ipd_debug("Bar mapped to %p\n", idev->bar0);

    /* Set DMA Mask  - try all possibilities */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
    err = pci_set_dma_mask(pdev, DMA_BIT_MASK(INNO_DMA_MASK));
#else
    err = dma_set_mask(&pdev->dev, DMA_BIT_MASK(INNO_DMA_MASK));
#endif
    if (err) {
        ipd_err("No 64 bit mask");
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
        err = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
#else
        err = dma_set_mask(&pdev->dev, DMA_BIT_MASK(32));
#endif
        if (err) {
            ipd_err("No 32 bit mask");
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
            err = pci_set_dma_mask(pdev, DMA_BIT_MASK(24));
#else
            err = dma_set_mask(&pdev->dev, DMA_BIT_MASK(24));
#endif
            if (err) {
                ipd_err("DMA MASK set error\n");
                return err;
            }
        }
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
    err = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(INNO_DMA_MASK));
#else
    err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(INNO_DMA_MASK));
#endif
    if (err) {
        ipd_err("No 64 bit consistent mask");
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
        err = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
#else
        err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
#endif
        if (err) {
            ipd_err("No 32 bit consistent mask");
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
            err = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(24));
#else
            err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(24));
#endif
            if (err) {
                ipd_err("Consistent DMA MASK set error\n");
                return err;
            }
        }
    }

    /* Set the PCIe stuff in the idev */
    idev->bar0_ba   = (void *)pci_resource_start(pdev, BAR_0);
    idev->bar0_size = pci_resource_len(pdev, BAR_0);
    idev->pdev      = pdev;

    /* Set Innovium device structure */
    pci_set_drvdata(pdev, idev);

    /* Allocate a pool for netdev TX system headers */
    idev->syshdr1 = dma_alloc_coherent(&idev->pdev->dev,
                                      SYSHDR_SIZE * 2 * NUM_SYSPORTS,
                                      &idev->syshdr1_ba, GFP_KERNEL);
    if (idev->syshdr1 == NULL) {
        ipd_err(
            "inno: PROBE:Error, Unable to allocate kernel memory for sys hdrs\n");
        err = -1;
        goto dma_buf_fail;
    }

    idev->syshdr1_abp = dma_alloc_coherent(&idev->pdev->dev,
                                           SYSHDR_SIZE * 2 * NUM_SYSPORTS,
                                           &idev->syshdr1_abp_ba, GFP_KERNEL);
    if (idev->syshdr1_abp == NULL) {
        ipd_err(
            "inno: PROBE:Error, Unable to allocate kernel memory for sys hdrs\n");
        err = -1;
        goto dma_buf_fail;
    }

    idev->syshdr1_ptp = dma_alloc_coherent(&idev->pdev->dev,
                                           SYSHDR_SIZE * NUM_SYSPORTS,
                                           &idev->syshdr1_ptp_ba, GFP_KERNEL);
    if (idev->syshdr1_ptp == NULL) {
        ipd_err(
            "inno: PROBE:Error, Unable to allocate kernel memory for sys hdrs\n");
        err = -1;
        goto dma_buf_fail;
    }

    idev->syshdr2 = dma_alloc_coherent(&idev->pdev->dev,
                                      SYSHDR_SIZE * NUM_SYSPORTS,
                                      &idev->syshdr2_ba, GFP_KERNEL);
    if (idev->syshdr2 == NULL) {
        ipd_err(
            "inno: PROBE:Error, Unable to allocate kernel memory for sys hdrs\n");
        err = -1;
        goto dma_buf_fail;
    }

    /* Allocate a memory for netdev TX checksum */
    idev->tx_netdev_cksum = dma_alloc_coherent(&idev->pdev->dev,
                                      MIN_PACKET_SIZE,
                                      &idev->tx_netdev_cksum_ba, GFP_KERNEL);
    if (idev->tx_netdev_cksum == NULL) {
        ipd_err(
            "inno: PROBE:Error, Unable to allocate kernel memory for tx cksum\n");
        err = -1;
        goto dma_buf_fail;
    }

    /* Set up the inno_intr regs */
    if (idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
        idev->inno_intr_regs.intr_incr = INTR_INCR_1;
        idev->inno_intr_regs.intr_inmc = INTR_INMC_1;
        idev->inno_intr_regs.intr_inms = INTR_INMS_1;
        idev->inno_intr_regs.intr_immr = INTR_IMMR_1;
        idev->inno_intr_regs.intr_iac_cause = INTR_IAC_1_CAUSE;
        idev->inno_intr_regs.intr_iac_pci_mask = INTR_IAC_1_PCI_MASK;
    } else {
        idev->inno_intr_regs.intr_incr = INTR_INCR;
        idev->inno_intr_regs.intr_inmc = INTR_INMC;
        idev->inno_intr_regs.intr_inms = INTR_INMS;
        idev->inno_intr_regs.intr_immr = INTR_IMMR;
        idev->inno_intr_regs.intr_iac_cause = INTR_IAC_CAUSE;
        idev->inno_intr_regs.intr_iac_pci_mask = INTR_IAC_PCI_MASK;
    }

    inno_num_devices++;

    /* Enable the Device */
    err = pci_enable_device(pdev);
    if (err) {
        return err;
    }

    printk("%s probe found PCI device %x:%x revision - %x\n", inno_driver_name, idev->vendor_id, idev->device_id, idev->rev_id);
    printk("%s pci bus:device:function is %d:%d:%d\n", inno_driver_name, pdev->bus->number, ((pdev->devfn)>>3)&0x7, (pdev->devfn)&0xf8);
    printk("%s interrupt mode is %s\n", inno_driver_name, intrmode2str(inno_intr));

    if (idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
        /* In the case of TL10, the device ID is encoded in
         * the JTAG_IDCODE_0 register, extract it.
         */
        jtag_idcode = REG32(JTAG_IDCODE_O);
        idev->rev_id = (jtag_idcode & 0xf0) >> 4;
    }
    return err;

dma_buf_fail:
    return err;
}


/**
 * inno_remove - Device Removal Routine
 * @pdev: PCI device information struct
 **/

/** @brief Device remove function
 *
 *  @param [in] pdev - PCIe device
 *  @return ERRNO
 */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
static void __devexit
inno_remove(struct pci_dev *pdev)
#else
static void
inno_remove(struct pci_dev *pdev)
#endif
{
    inno_device_t *idev;
    ipd_trace("%s: pdev: %p\n", __FUNCTION__, pdev);

    idev = (inno_device_t *)pci_get_drvdata(pdev);
    if (idev == NULL) {
        ipd_trace("%s: idev is null\n", __FUNCTION__);
        return;
    }

    inno_reset(idev);
    inno_num_devices--;
}

static struct pci_driver inno_driver =
{
    .name     = "ipd",
    .id_table = inno_ids,
    .probe    = inno_probe,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
    .remove      = __devexit_p(inno_remove),
#else
    .remove      = inno_remove,
#endif
    .err_handler = &inno_pci_error,
};

/* VMA ops for mmap */
static const struct vm_operations_struct inno_mmap_mem_ops =
{
};

static int inno_intr_enable_intx(inno_device_t *idev)
{
    int err;
    int vector;

    /* Set up the wait queues for now, to keep the other code intact */
    for (vector = 0; vector < NUM_MSIX_VECS; vector++)
    {
        init_waitqueue_head(&idev->rupts[vector].wait_q);
        idev->msix[vector].entry = vector;
        idev->rupts[vector].vector = 1; /* Temporary fix */
    }

    /* Register IRQ Handler */
    idev->num_vectors = 1;
    sprintf(idev->msix_names[0], "inno_irq");
    err = request_irq(idev->pdev->irq, inno_rupt_handler, IRQF_SHARED,
                      "inno_irq", idev);
    if (err) {
        ipd_err("%s:Error,:%d-Unable to register interrupt handler \n",
               __FUNCTION__, err);
        return err;
    }
    idev->intr_type |= INNO_INTR_INTX_ENABLED;
    ipd_info("INT-A Success\n");
    return 0;
}

static int inno_intr_enable_msi(inno_device_t *idev)
{
    int err;
    int vector;

    idev->num_vectors = 0;
    for (vector = 0; vector < NUM_MSIX_VECS; vector++) {
        init_waitqueue_head(&idev->rupts[vector].wait_q);
        idev->msix[vector].entry = vector;
        idev->rupts[vector].vector = 1; /* Temporary fix */
    }

    ipd_trace("MSI enable\n");
    
    err = pci_enable_msi(idev->pdev);
    if (err < 0) {
        ipd_err("Error acquiring MSI vectors err: %d\n", err);
        return err;
    }

    sprintf(idev->msix_names[0], "inno_irq-0");
    err = request_irq(idev->pdev->irq,
                      inno_rupt_handler,
                      0,
                      idev->msix_names[0],
                      idev);

    if (err) {
        ipd_err("%s:Error,:%d-Unable to register vector(%d) handler \n",
               __FUNCTION__, err, vector);
        /* Free the allocated ones */
        return err;
    }

    idev->num_vectors++;
    idev->intr_type |= INNO_INTR_MSI_ENABLED;
    ipd_info("MSI enable success\n");
    return 0;
}

static int inno_intr_enable_msix(inno_device_t *idev)
{
    int err;
    int vector;

    ipd_trace("Enabling MSIX\n");
    idev->num_vectors = 0;
    for (vector = 0; vector < NUM_MSIX_VECS; vector++) {
        init_waitqueue_head(&idev->rupts[vector].wait_q);
        idev->msix[vector].entry = vector;
    }

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 13, 0)
    err = pci_enable_msix(idev->pdev,
                          &idev->msix[0],
                          NUM_MSIX_VECS);
    if (err) {
#else
    err = pci_enable_msix_range(idev->pdev,
                          idev->msix,
                          NUM_MSIX_VECS,
                          NUM_MSIX_VECS);
    if (err < 0) {
#endif
        ipd_err("Couldn't acquire enough MSIX vectors %d\n", err);
        return -1;
    }

    for (vector = 0; vector < NUM_MSIX_VECS; vector++) {
        idev->rupts[vector].flag = 0;
        sprintf(idev->msix_names[vector], "inno_irq-%d-%s", vector, vec2str(vector));
        if (vector == MSIX_VECTOR_NAPI) {
            err = request_irq(idev->msix[vector].vector,
                              &_inno_napi_handler,
                              0,
                              idev->msix_names[vector],
                              idev);
        } else {
            err = request_irq(idev->msix[vector].vector,
                              &_inno_rupt_handler,
                              0,
                              idev->msix_names[vector],
                              idev);
        }
        if (err) {
            ipd_err("Request IRQ error - %x\n", err);
            break;
        }

        idev->num_vectors++;
        idev->rupts[vector].vector = idev->msix[vector].vector;
        ipd_info("IRQ vector %x %x\n", idev->msix[vector].vector, vector);
    }

    if (err) {
        ipd_err("%s:Error,:%d-Unable to register vector(%d) handler \n",
               __FUNCTION__, err, vector);
        /* Free the allocated ones */
        while (vector != 0) {
            vector--;
            free_irq(idev->msix[vector].vector, idev);
        }
        return err;
    }
    idev->intr_type |= INNO_INTR_MSIX_ENABLED;
    ipd_info("MSIX Success\n");
    return 0;
}

static int inno_intr_init_best_effort(inno_device_t *idev)
{
    int err;

    err = inno_intr_enable_msix(idev);
    if(!err) /* msix succeeded */ {
        return 0;
    }

    err = inno_intr_enable_msi(idev);
    if(!err) /* msi succeeded */ {
        return 0;
    }

    err = inno_intr_enable_intx(idev);
    if(!err) /* intx succeeded */ {
        return 0;
    }

    return -1;
}

/*
 * Set up interrupt handler
 */
static int inno_intr_init(inno_device_t *idev)
{
    int i, err = 0;

    /* Clear the interrupt routing - everything to zero */
    for (i = 0; i < 32; i++) {
        REG32(idev->inno_intr_regs.intr_immr + i * 4) = 0;      /* Maps to MSIX 0 */
    }

    /* Disable all of the MSIX interrupts */
    for (i = 0; i < rupt_mask_words; i++) {
        REG32(idev->inno_intr_regs.intr_inmc + i * 4) = 0xffffffff;
    }

    switch(inno_intr) {
        case INNO_IRQ_TYPE_MSIX_ONLY:
		err = inno_intr_enable_msix(idev);
		break;
        case INNO_IRQ_TYPE_MSI_ONLY:
		err = inno_intr_enable_msi(idev);
		break;
        case INNO_IRQ_TYPE_INTX_ONLY:
		err = inno_intr_enable_intx(idev);
		break;
        case INNO_IRQ_TYPE_BEST_EFFORT:
	default:
		err = inno_intr_init_best_effort(idev);
		break;
    }

    return err;
}

static int inno_intr_deinit(inno_device_t *idev)
{
    int i;
    int vector;
    struct pci_dev *pdev;

    pdev = idev->pdev;
    /* Mask off all rupts */
    for (i = 0; i < rupt_mask_words; i++) {
        REG32(idev->inno_intr_regs.intr_inmc + i * 4) = 0xffffffff;
    }

    if(idev->intr_type & INNO_INTR_INTX_ENABLED) {
        ipd_info("Disabling INTx: %x\n", pdev->irq);
        disable_irq(pdev->irq);
        free_irq(pdev->irq, idev);
        idev->rupts[pdev->irq].vector = 0;
        idev->inno_stats.inno_rupt_stats.num_int[0]=0;
    } else if(idev->intr_type & INNO_INTR_MSIX_ENABLED) {
        ipd_info("Disabling MSIX\n");
        for (vector = 0; vector < NUM_MSIX_VECS; vector++) {
            if (idev->msix[vector].vector != 0) {
                unsigned long flags;
                ipd_debug("Close vec %x %x\n", vector, idev->msix[vector].vector);
                disable_irq(idev->msix[vector].vector);
                free_irq(idev->msix[vector].vector, idev);
                idev->msix[vector].vector = 0;
                idev->inno_stats.inno_rupt_stats.num_int[vector]=0;

                /* Kickstart any waiters */
                spin_lock_irqsave(&idev->rupt_lock, flags);
                idev->rupts[vector].vector = 0;
                idev->rupts[vector].flag = 1;
                wake_up_interruptible(&idev->rupts[vector].wait_q);
                spin_unlock_irqrestore(&idev->rupt_lock, flags);
            }
        }
        pci_disable_msix(pdev);
    } else if(idev->intr_type & INNO_INTR_MSI_ENABLED) {
        ipd_info("Disabling MSI: %x\n", pdev->irq);
        disable_irq(pdev->irq);
        free_irq(pdev->irq, idev);
        pci_disable_msi(pdev);
        idev->rupts[pdev->irq].vector = 0;
        idev->inno_stats.inno_rupt_stats.num_int[0]=0;
    }

    return 0;
}


static int
inno_override_flow_control(inno_device_t  *idev , int enable)
{
    uint32_t       cpu_fc;

    /* Overrides flow control settings */

    if(idev->device_id == INNO_TERALYNX_PCI_DEVICE_ID) {
        isn_read_pen(idev, 0, 0x00000540, 5, 33, &cpu_fc, 1);
    } else if(idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
        isn_read_pen(idev, 0, 0x10000a80, 5, 65, &cpu_fc, 1);
    } else {

        ipd_err("Unknown innovium device\n");
    }

    ipd_info("CPU pkt override before write value is %08x\n", cpu_fc);
    if (enable) {
        cpu_fc |= 0x00000003;
    } else if (enable == 0) {
        cpu_fc &= 0xFFFFFFFC;
    }
    ipd_info("CPU pkt override write value is %08x\n", cpu_fc);
    if(idev->device_id == INNO_TERALYNX_PCI_DEVICE_ID) {
        isn_write_pen(idev, 0, 0x00000540, 5, 33, &cpu_fc, 1);
        isn_read_pen(idev, 0, 0x00000540, 5, 33, &cpu_fc, 1);
    } else if(idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
        isn_write_pen(idev, 0, 0x10000a80, 5, 65, &cpu_fc, 1);
        isn_read_pen(idev, 0, 0x10000a80, 5, 65, &cpu_fc, 1);
    } else {

        ipd_err("Unknown innovium device\n");
    }
    ipd_info("CPU pkt override after write value is %08x\n", cpu_fc);

    return 0;
}

static int
inno_cleanup_resources(inno_device_t  *idev )
{
    int vector;
    int            i;

    if (idev == NULL) {
        return 0;
    }
    ipd_debug("In %s", __FUNCTION__);

    /* Overrides flow control settings and stops traffic to CPU*/
    inno_override_flow_control(idev, 1);
    inno_intr_deinit(idev);
    idev->inno_netdev.num_interfaces=0;
    for (i = 0; i < NUM_SYSPORTS; i++) {
        inno_netdev_delete(idev, i);
        idev->syshdr1_cnt[i] = 0;
    }
    memset(&(idev->inno_netdev), 0, sizeof(idev->inno_netdev));

    if (idev->napi_init != 0) {
        inno_napi_deinit(idev);
    }

    (void)inno_free_hrr(idev);

    for (i = 0; i < NUM_TX_RINGS; i++) {
        inno_free_ring(idev, INNO_RING_TX, i);
    }

    for (i = 0; i < NUM_RX_RINGS; i++) {
        inno_free_ring(idev, 0, i);
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
    /* Deinit netlink */
    inno_netlink_deinit(idev);
#endif

    /* Free the learn after Freeing CPU rings */
    (void)inno_free_learn(idev);

    if (idev->pool) {
        dma_free_coherent(&idev->pdev->dev, POOL_SIZE,
        idev->pool, idev->pool_ba);
        idev->pool = NULL;
    }

    for (vector = 0; vector < NUM_MSIX_VECS; vector++) {
        idev->rupts[vector].vector = 0;
    }
    printk("%s success\n", __FUNCTION__);
    ipd_hw_init_done = 0;

    return 0;
}

/** @brief File open function
 *
 *  @return ERRNO
 */
static int
inno_open(struct inode *inode,
          struct file  *fp)
{
    inno_device_t                  *idev;

    ipd_trace("inno open\n");

    /* Extract device, each minor num is index to device */
    idev = &inno_instances[MINOR(inode->i_rdev)];
    if (idev->pdev == NULL) {
        ipd_err("%s: Device does not exist - %d\n", __FUNCTION__, MINOR(inode->i_rdev));
        return -ENODEV;
    }
    fp->private_data = idev;

    if (inno_ipd_opened != 0) {
        ipd_debug("Device opened before, proceeding without initialization");
        return -EEXIST;
    }

    idev->inno_stats.inno_drv_stats.num_open++;

    if ((fp->f_mode & FMODE_WRITE) == 0) {
        /* Opened for read - just for IOCTL probe */
        return 0;
    }

    printk("%s opened successfully\n", inno_driver_name);
    inno_ipd_opened = 1;
    return 0;
}

#define CPU_BLOCK_INIT_DONE_PATTERN 0xAABBCCDD

int
inno_is_cpu_block_init_done(inno_device_t *idev)
{
    uint32_t val;

    val = REG32(SYNC_HSN_ADR_LO_3);

    if(val == CPU_BLOCK_INIT_DONE_PATTERN) {
        return 0;
    }

    return -1;
}

int
inno_cpu_block_init_scratchpad_set(inno_device_t *idev)
{
    uint32_t val;
    REG32(SYNC_HSN_ADR_LO_3) = CPU_BLOCK_INIT_DONE_PATTERN;

    val = REG32(SYNC_HSN_ADR_LO_3);
    ipd_debug("Setting SYNC_HSN_ADR_LO_3; read-back val: 0x%x", val);

    return 0;
}

static int inno_cpu_block_init(inno_device_t *idev)
{
    int rc = 0;
    if (inno_is_cpu_block_init_done(idev) != 0) {
        int                   ring_num;
        txe_chnl_0_t          txe_chnl_reg;
        txe_chnl_3_t          txe_chnl3_reg;
        cnfg_ref_amount_txe_t cnfg_txe;
        cnfg_ref_amount_rxe_t cnfg_rxe;
        iac_mem_init_t        iac_mem_init;
        mcu_mem_init_t        mcu_mem_init;
        dma_global_t           dma_global_reg;
        dma_intf_t             dma_intf_reg;

        ipd_debug("inno_is_cpu_block_init_done performing now");
        /* Chip reset sequence */

        /* Init memories */
        iac_mem_init.flds.bram_f = 1;
        iac_mem_init.flds.asc_cmn_f = 1;
        iac_mem_init.flds.asc_comp0_f = 1;
        iac_mem_init.flds.asc_comp1_f = 1;
        iac_mem_init.flds.asc_comp2_f = 1;
        iac_mem_init.flds.asc_comp3_f = 1;
        iac_mem_init.flds.asc_comp4_f = 1;
        iac_mem_init.flds.asc_comp5_f = 1;
        iac_mem_init.flds.asc_comp6_f = 1;
        iac_mem_init.flds.asc_comp7_f = 1;
        REG32(IAC_MEM_INIT) = iac_mem_init.data;

        mcu_mem_init.flds.mcu1_f = 1;
        mcu_mem_init.flds.mcu2_f = 1;
        REG32(MCU_MEM_INIT) = mcu_mem_init.data;

        /* Initialize the ring attributes that require a global reset */
        memset(&dma_global_reg, 0, sizeof(dma_global_reg));

        dma_global_reg.flds.rst_n_f = 0;    /* Start with a reset */
        REG32(DMA_GLOBAL) = dma_global_reg.data;


        if(idev->device_id == INNO_TERALYNX_PCI_DEVICE_ID) {
            REG32(DMA_RXE_SWITCH_TO_CPU_QUEUE_OFFSET) = 268;
        } else if (idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
            REG32(DMA_RXE_SWITCH_TO_CPU_QUEUE_OFFSET) = 524;
        } else {
            ipd_err("Invalid device: 0x%x\n", idev->device_id);
            return rc;
        }
        for (ring_num = 0; ring_num < NUM_TX_RINGS; ring_num++) {
            txq_0_cache_t txq_cache_reg;
            txq_0_t       txq_reg;

            /* Set the cache size and start addr.  We divide it into 16
             * parts (wastes some) with the TX cache in the first 8 and
             * the RX after that. */
            memset(&txq_cache_reg, 0, sizeof(txq_cache_reg));
            txq_cache_reg.flds.start_addr_f   = 128 * ring_num;
            txq_cache_reg.flds.size_f         = 128;
            txq_cache_reg.flds.lo_threshold_f = 64;
            REG32(TXQ_x_CACHE(ring_num)) = txq_cache_reg.data;

            txq_reg.flds.cidx_wb_thres_f           = 1;
            txq_reg.flds.cidx_intr_cnt_coalse_en_f = 1;
            txq_reg.flds.dma_type_f = 1;
            if (ring_num < NUM_TX_SWITCH_RINGS) {
                txq_reg.flds.chnl_map_f = 0;      /* BAM */
            } else if (ring_num == TX_TO_MCU0_RING) {
                txq_reg.flds.chnl_map_f = 1;      /* MCU0 */
            } else if (ring_num == TX_TO_MCU1_RING) {
                txq_reg.flds.chnl_map_f = 2;      /* MCU1 */
            } else {
                ipd_err("Unmapped Tx ring channel: %d", ring_num);
            }
            REG32(TXQ_x(ring_num)) = txq_reg.data;
        }

        /* Set up the ESB channel params */
        txe_chnl_reg.data = 0;
        txe_chnl_reg.flds.credits_f = 0x300;
        txe_chnl_reg.flds.weight_f = 1;
        txe_chnl_reg.flds.strict_priority_f = 0;
        REG32(TXE_CHNL_x(0)) = txe_chnl_reg.data;

        /* Set up the MCU0 channel params */
        txe_chnl_reg.flds.credits_f = 0x40;
        txe_chnl_reg.flds.weight_f = 1;
        txe_chnl_reg.flds.strict_priority_f = 0;
        REG32(TXE_CHNL_x(1)) = txe_chnl_reg.data;

        /* Set up the MCU1 channel params */
        txe_chnl_reg.flds.credits_f = 0x40;
        txe_chnl_reg.flds.weight_f = 1;
        txe_chnl_reg.flds.strict_priority_f = 0;
        REG32(TXE_CHNL_x(2)) = txe_chnl_reg.data;

        /* Set up the block channel params */
        txe_chnl3_reg.data = 0;
        txe_chnl3_reg.flds.weight_f = 1;
        txe_chnl3_reg.flds.strict_priority_f = 0;
        REG32(TXE_CHNL_3) = txe_chnl3_reg.data;

        for (ring_num = 0; ring_num < NUM_RX_RINGS; ring_num++) {
            rxq_0_cache_t rxq_cache_reg;

            /* Set the cache size and start addr */
            memset(&rxq_cache_reg, 0, sizeof(rxq_cache_reg));
            rxq_cache_reg.flds.start_addr_f   = (128 * (ring_num + 8));
            rxq_cache_reg.flds.size_f         = 128;
            rxq_cache_reg.flds.lo_threshold_f = 64;
            REG32(RXQ_x_CACHE(ring_num)) = rxq_cache_reg.data;
        }

        /* Take DMA_GLOBAL out of reset and enable everything */
        dma_global_reg.flds.rst_n_f = 1;
        REG32(DMA_GLOBAL) = dma_global_reg.data;

        dma_global_reg.flds.en_f      = 1;
        dma_global_reg.flds.intr_en_f = 1;
        dma_global_reg.flds.en_f      = 1;
        dma_global_reg.flds.txe_en_f  = 1;
        dma_global_reg.flds.rxe_en_f = 1;

        /* We disable to cacheline split logic. This causes the DMA
           engine to ignore the cacheline size when processing requests.
           That speeds up the overall request since splitting will
           result in multiple requests.  With splitting the first
           request gets the first chunk of data back to the DMA engine
           earlier but the second and sebsequent chunks are delayed so
           the overall performance is degraded. */
        dma_global_reg.flds.en_txe_cacheline_split_f = 0;
        dma_global_reg.flds.en_rxe_cacheline_split_f = 0;
        dma_global_reg.flds.cacheline_size_f         = (idev->cache_align) >> 6;
        ipd_debug("idev->cache_align: 0x%x", idev->cache_align);
        dma_global_reg.flds.en_dm_cacheline_split_f  = 1;
        REG32(DMA_GLOBAL) = dma_global_reg.data;

        /* Set up the CPU packet schdeuler */
        REG32(SCH_QUANTUM_TXE_CNFG0) = 0x7f;
        REG32(SCH_QUANTUM_TXE_CNFG1) = 0x1ff;
        REG32(SCH_QUANTUM_TXE_CNFG2) = 0x7ff;
        cnfg_txe.flds.major_exp_bin0_f = 0xf;
        cnfg_txe.flds.major_exp_bin1_f = 0xf;
        cnfg_txe.flds.major_exp_bin2_f = 0xf;
        cnfg_txe.flds.major_exp_bin3_f = 0xf;
        cnfg_txe.flds.minor_exp_bin0_f = 0xe;
        cnfg_txe.flds.minor_exp_bin1_f = 0xe;
        cnfg_txe.flds.minor_exp_bin2_f = 0xe;
        cnfg_txe.flds.minor_exp_bin3_f = 0xe;
        REG32(CNFG_REF_AMOUNT_TXE) = cnfg_txe.data;

        REG32(SCH_QUANTUM_RXE_CNFG0) = 0x7f;
        REG32(SCH_QUANTUM_RXE_CNFG1) = 0x1ff;
        REG32(SCH_QUANTUM_RXE_CNFG2) = 0x7ff;
        cnfg_rxe.flds.major_exp_bin0_f = 0xf;
        cnfg_rxe.flds.major_exp_bin1_f = 0xf;
        cnfg_rxe.flds.major_exp_bin2_f = 0xf;
        cnfg_rxe.flds.major_exp_bin3_f = 0xf;
        cnfg_rxe.flds.minor_exp_bin0_f = 0xe;
        cnfg_rxe.flds.minor_exp_bin1_f = 0xe;
        cnfg_rxe.flds.minor_exp_bin2_f = 0xe;
        cnfg_rxe.flds.minor_exp_bin3_f = 0xe;
        REG32(CNFG_REF_AMOUNT_RXE) = cnfg_rxe.data;

        /* Make sure that the interface is now enabled */
        dma_intf_reg.flds.cpu_to_switchif_en_f = 1;
        dma_intf_reg.flds.switch_to_cpuif_en_f = 1;
        REG32(DMA_INTF) = dma_intf_reg.data;

        rc = inno_cpu_block_init_scratchpad_set(idev);
        if (rc != 0) {
            ipd_err("inno_cpu_block_init_scratchpad_set failed; rc: %d", rc);
            return rc;
        }
    }

    return rc;
}

/** @brief hw init function
 *
 *  @param [in] idev - innovium device
 */
static int
inno_hw_init (inno_device_t *idev)
{
    int i, err;
    intr_iac_pci_mask_t            intr_iac_pci_mask;
    int rc = 0;

    ipd_debug("inno hw init");
    ipd_debug("***** HS_ADR_3: 0x%x", REG32(SYNC_HSN_ADR_LO_3));
    rc = inno_cpu_block_init(idev);
    if (rc != 0) {
        ipd_err("inno_cpu_block_init failed with rc: %d", rc);
        return rc;
    }

    if (ipd_hw_init_done == 1) {
        inno_cleanup_resources(idev);
        inno_ipd_opened_before = 0;
    }
    inno_hw_swaps_clear(idev);
    smp_mb();

    /* Clear interrupts */
    REG32(idev->inno_intr_regs.intr_iac_cause) = 0xffffffff;
    for (i = 0; i < 32; i += 4) {
        REG32(idev->inno_intr_regs.intr_incr + i) = 0xffffffff;
    }
    err = inno_napi_init(idev);
    if (err) {
        ipd_err("%s: NAPI init error\n", __FUNCTION__);
    }

    INIT_LIST_HEAD(&idev->trapid_list);

    smp_mb();
    err = inno_hw_swaps_enable(idev);

    smp_mb();

    err = inno_syncmode_config(idev);
    smp_mb();

    err = inno_intr_init(idev);
    if (err) {
	    ipd_err("%s: Interrupts_init error: %d\n", __FUNCTION__, err);
	    return err;
    }
    smp_mb();
    if(idev->intr_type & INNO_INTR_MSI_ENABLED) {
        err = inno_msi_workaround(idev);
        if (err)
            ipd_err("Unable to apply inno work-around for MSI \n");
    }

#ifdef EVT_THREAD
    evt_thread = kthread_run(evt_thread_hndlr, idev, "InnoEvtThread");
    if(!evt_thread) {
	    ipd_info("Error creating InnoEvtThread\n");
    } else
	    ipd_info("InnoEvtThread created\n");
#endif

    /* Enable the IAC interrupts */
    intr_iac_pci_mask.data = 0;
    intr_iac_pci_mask.flds.hrr_cmd_done_f = 1;
    intr_iac_pci_mask.flds.hsr_cmd_done_f = 1;
    REG32(idev->inno_intr_regs.intr_iac_pci_mask) = intr_iac_pci_mask.data;


    if(idev->device_id == INNO_TERALYNX_PCI_DEVICE_ID ||
       idev->device_id == INNO_K2_PCI_DEVICE_ID) {
        idev->chip_hdr_len     = INNO_TERALYNX_CHIP_HDR_LEN;
        idev->chip_dbg_hdr_len = INNO_TERALYNX_CHIP_DBG_HDR_LEN;
    } else if (idev->device_id == MRVL_TL10_PCI_DEVICE_ID) {
        idev->chip_hdr_len     = INNO_TL10_CHIP_HDR_LEN;
        idev->chip_dbg_hdr_len = INNO_TL10_CHIP_DBG_HDR_LEN;
    } else {
        ipd_err("Invalid device: 0x%x\n", idev->device_id);
    }
    ipd_debug("Intr-type status!!: %s\n", inno_intrtype_status2str(idev->intr_type));
    inno_override_flow_control(idev, 0);
    ipd_hw_init_done = 1;
    inno_ipd_opened_before = 1;

    return 0;
}
/** @brief File close function
 *
 *  @return ERRNO
 */
static int
inno_close(struct inode *inode,
           struct file  *fp)
{
    inno_device_t  *idev = fp->private_data;

    if (idev == NULL) {
        return 0;
    }

    if (inno_ipd_opened == 0) {
        ipd_debug("Device closed before, proceeding without de-initialization");
        return -ENODEV;
    }

    if ((fp->f_mode & FMODE_WRITE) == 0) {
        /* Opened for read - just for IOCTL probe */
        goto close_done;
    }


    ipd_info("%s: boottype: %d\n", __FUNCTION__, ipd_boottype);
    if (ipd_boottype != IPD_BOOTTYPE_FAST){
        inno_cleanup_resources(idev);
        inno_ipd_opened_before = 0;
    }

    idev->inno_stats.inno_drv_stats.num_close++;

    inno_ipd_opened = 0;
    printk("%s closed successfully\n", inno_driver_name);
close_done:
    fp->private_data = NULL;

    return 0;
}


/** @brief File MMAP function
 *
 *  @return ERRNO
 */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
#define MMAP_VM_FLAGS    (VM_IO | VM_RESERVED)
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)
#define MMAP_VM_FLAGS    (VM_IO | VM_DONTEXPAND | VM_NODUMP)
#else
#define MMAP_VM_FLAGS    (VM_IO | VM_DONTEXPAND | VM_DONTDUMP)
#endif

static int
inno_mmap(struct file           *fp,
          struct vm_area_struct *vma)
{
    inno_device_t *idev;
    unsigned long start, kvirt;
    int           rc;

    /* Extract device */
    idev = fp->private_data;
    if (vma->vm_end < vma->vm_start) {
        ipd_err("mmap: invalid params %lx %lx",vma->vm_end, vma->vm_start);
        return -EINVAL;
    }
    vma->vm_private_data = idev;

    start = vma->vm_start & ~(PAGE_SIZE - 1);

    /* We use the page offset as the kernel virtual addr to map */
    kvirt = vma->vm_pgoff << PAGE_SHIFT;;

    vma->vm_ops       = &inno_mmap_mem_ops;
    vma->vm_flags    |= MMAP_VM_FLAGS;

    do {
        ipd_verbose("remap_pfn %p %p %p\n",(void *)start, (void *)kvirt, (void *) virt_to_phys((void *) kvirt));

        vma->vm_flags |= MMAP_VM_FLAGS;

        /* X86 systems have an I/O coherent cache;  PPC & ARM does not */
#if !defined(__i386__) && !defined(__x86_64__)
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#endif
        rc = remap_pfn_range(vma,
                             start,
                             kvirt >> PAGE_SHIFT,
                             PAGE_SIZE,
                             vma->vm_page_prot);
        start += PAGE_SIZE;
        kvirt += PAGE_SIZE;

    } while ((rc == 0) && (start < vma->vm_end));

    ipd_verbose("MMAP'd %p %p, %lx %x",(void *)vma->vm_start, (void *)vma->vm_end,
             vma->vm_pgoff, rc);

    return rc;
}

/* File operations for IPD */
static struct file_operations inno_fops =
{
    .owner   = THIS_MODULE,       /*prevents unloading when operations are in use*/
    .open    = inno_open,         /* OPEN entry point */
    .release = inno_close,        /* CLOSE entry point */
    .mmap    = inno_mmap,         /* MMAP entry point */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
    .ioctl          = inno_ioctl, /* IOCTL entry point */
#else
    .unlocked_ioctl = inno_ioctl, /* IOCTL entry point */
    .compat_ioctl   = inno_ioctl, /* IOCTL entry point */
#endif
};

/*****************************************************************
 *
 * Error operations
 *
 */
int
inno_pci_reg_reset(struct pci_dev *pdev)
{
    inno_device_t *idev = (inno_device_t *)pci_get_drvdata(pdev);
    int           i;
    hrr_size_t    hrr_size;


    for (i = 0; i < NUM_TX_RINGS; i++) {
        inno_ring_t *ring = &idev->tx_ring[i];
        if (ring->flags & INNO_RING_INIT) {
            /* (Re)write the ring registers */
            REG32(TXQ_x_BASE_HSN_HI(ring->num))    = UPPER32(ring->desc_ba);
            REG32(TXQ_x_BASE_HSN_LO(ring->num))    = LOWER32(ring->desc_ba);
            REG32(TXQ_x_BASE_HSN_HI_WB(ring->num)) = UPPER32(ring->wb_ba);
            REG32(TXQ_x_BASE_HSN_LO_WB(ring->num)) = LOWER32(ring->wb_ba);
            //    idev->pool_ba + ring->idx_offset;
            REG32(TXQ_x_CIDX_WB_HSN_HI(ring->num)) =
                UPPER32(idev->pool_ba + ring->idx_offset);
            REG32(TXQ_x_CIDX_WB_HSN_LO(ring->num)) =
                LOWER32(idev->pool_ba + ring->idx_offset);
            REG32(TXQ_x_DESC_RING(ring->num)) = ring->count;
            ring->pidx = 0;
            REG32(TXQ_x_DESC_PIDX(ring->num)) = ring->pidx;
            ring->cidx = 0;
            REG32(TXQ_x_DESC_CIDX(ring->num)) = ring->cidx;
        }
    }

    for (i = 0; i < NUM_RX_RINGS; i++) {
        inno_ring_t *ring = &idev->rx_ring[i];
        if (ring->flags & INNO_RING_INIT) {
            /* (Re)write the ring registers */
            REG32(RXQ_x_BASE_HSN_HI(ring->num))    = UPPER32(ring->desc_ba);
            REG32(RXQ_x_BASE_HSN_LO(ring->num))    = LOWER32(ring->desc_ba);
            REG32(RXQ_x_BASE_HSN_HI_WB(ring->num)) = UPPER32(ring->wb_ba);
            REG32(RXQ_x_BASE_HSN_LO_WB(ring->num)) = LOWER32(ring->wb_ba);
            REG32(RXQ_x_CIDX_WB_HSN_HI(ring->num)) =
                UPPER32(idev->pool_ba + ring->idx_offset);
            REG32(RXQ_x_CIDX_WB_HSN_LO(ring->num)) =
                LOWER32(idev->pool_ba + ring->idx_offset);
            REG32(RXQ_x_DESC_RING(ring->num)) = ring->count;
            ring->cidx = 1;
            REG32(RXQ_x_DESC_CIDX(ring->num)) = ring->cidx;
            ring->pidx = 0;
            REG32(RXQ_x_DESC_PIDX(ring->num)) = ring->pidx;
        }
    }

    if (idev->hrr.ba) {
        /* (Re)write the HRR info */
        REG32(HRR_ADR_HI)         = UPPER32(idev->hrr.ba);
        REG32(HRR_ADR_LO)         = LOWER32(idev->hrr.ba);
        hrr_size.flds.slot_num_f  = idev->hrr.count;
        hrr_size.flds.slot_size_f = idev->hrr.size;
        REG32(HRR_SIZE)           = hrr_size.data;
        REG32(HRR_PIDX_ADR_HI)    = UPPER32(idev->pool_ba + POOL_HRR_OFFSET);
        REG32(HRR_PIDX_ADR_LO)    = LOWER32(idev->pool_ba + POOL_HRR_OFFSET);
    }

    if (idev->learn.ba) {
        /* (Re)write the learn info */
        //    REG64(LEARN_ADR_LO) = idev->learn.ba;
        //learn_size.flds.slot_num_f = idev->learn.count;
        //    REG64(LEARN_PIDX_ADR_LO) = idev->pool_ba +
        //idev->learn.pidx_offset;
    }

    // Finally clear the link_reset register to allow writebacks to
    // resume */
    REG32(PCI_AXI_LINK_DOWN_INDICATOR_BIT_L0) = 0x0;
    return 0;
}


pci_ers_result_t
inno_error_detected(struct pci_dev *pdev,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
                    enum pci_channel_state state)
#else
                    pci_channel_state_t state)
#endif
{
    ipd_err("PCIe error detected!! Channel state %d\n", state);
    return PCI_ERS_RESULT_NEED_RESET;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
pci_ers_result_t
inno_link_reset(struct pci_dev *pdev)
{
    ipd_err("PCIe link reset!!\n");
    if (inno_pci_reg_reset(pdev)) {
        return PCI_ERS_RESULT_NEED_RESET;
    }
    return PCI_ERS_RESULT_RECOVERED;
}
#endif

pci_ers_result_t
inno_slot_reset(struct pci_dev *pdev)
{
    ipd_err("PCIe slot reset!!\n");
    if (inno_pci_reg_reset(pdev)) {
        return PCI_ERS_RESULT_NEED_RESET;
    }
    return PCI_ERS_RESULT_RECOVERED;
}


void
inno_resume(struct pci_dev *dev)
{
    ipd_err("PCIe resume!!\n");
}

/*****************************************************************
 *
 * Module operations
 *
 */

/** @brief Module registration
 *
 *  @return ERRNO
 */
static int __init
inno_init_module(void)
{
    int rc;

    ipd_trace("%s \n", inno_driver_name);

    /* Register all possible nodes as minor nums */
    rc = register_chrdev_region(inno_major_dev, MAX_INNO_DEVICES, "ipd");
    if (rc < 0) {
        /* So major is not availble try to allocate from kernel */
        rc = alloc_chrdev_region(&inno_major_dev, 0, MAX_INNO_DEVICES, "ipd");
        if (rc < 0) {
            ipd_err("Unable to alloc char device\n");
            return rc;
        }
        ipd_warn("Inno: the char device major is %d\n", MAJOR(inno_major_dev));
    }
   if ((inno_cl = class_create(THIS_MODULE, "ipd")) == NULL) {
       ipd_err("Inno: Unable to create ipd class\n");
       goto cl_fail;
   }
   if (device_create(inno_cl, NULL, inno_major_dev, NULL, "ipd") == NULL) {
       ipd_err("Inno: Unable to create ipd device\n");
       goto dev_create_fail;
    }
    cdev_init(&inno_cdev, &inno_fops);
    inno_cdev.owner = THIS_MODULE;
    rc = cdev_add(&inno_cdev, inno_major_dev, MAX_INNO_DEVICES);
    ipd_info("Inno: cdev major %d rc %d\n", MAJOR(inno_major_dev), rc);
    if (rc < 0) {
        ipd_err("Unable to create cdev major %d\n", MAJOR(inno_major_dev));
        goto char_fail;
    }

    rc = pci_register_driver(&inno_driver);
    if (rc) {
        ipd_err("Unable to register pci driver\n");
        goto cdev_fail;
    }

    inno_sysfs_init(inno_instances,MAX_INNO_DEVICES);
    printk("%s initialized. Version - %s\n", inno_driver_name, ipd_version);
    return 0;

cdev_fail:
    cdev_del(&inno_cdev);
char_fail:
    device_destroy(inno_cl,inno_major_dev);
dev_create_fail:
    class_destroy(inno_cl);
cl_fail:
    unregister_chrdev_region(inno_major_dev, MAX_INNO_DEVICES);
    return rc;
}


/** @brief Module deregistration
 *
 *  @return ERRNO
 */
static void __exit
inno_exit_module(void)
{
    int i;

    ipd_trace("inno exit\n");
    cdev_del(&inno_cdev);
    device_destroy(inno_cl,inno_major_dev);
    class_destroy(inno_cl);
    unregister_chrdev_region(inno_major_dev, MAX_INNO_DEVICES);
    for (i = 0; i < inno_num_devices; ++i) {
        if(inno_instances[i].pdev != NULL) {
            inno_reset(&inno_instances[i]);
        }
    }
    pci_unregister_driver(&inno_driver);
    inno_sysfs_deinit();
    printk("%s exited\n", inno_driver_name);
}


module_init(inno_init_module);
module_exit(inno_exit_module);

MODULE_AUTHOR("Marvell Technology, Inc.");
MODULE_DESCRIPTION("Teralynx Switch PCIe Driver");
MODULE_LICENSE("GPL");
