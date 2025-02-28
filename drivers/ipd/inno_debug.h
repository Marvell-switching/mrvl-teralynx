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

#ifndef __IPD_STATS_H__
#define __IPD_STATS_H__

#define IPD_LOGLEVEL_CRIT     0   /* critical issue */
#define IPD_LOGLEVEL_ERR      1   /* error condition */
#define IPD_LOGLEVEL_WARN     2   /* warning condition */
#define IPD_LOGLEVEL_NOT      3   /* notice condition */
#define IPD_LOGLEVEL_INFO     4   /* information log */
#define IPD_LOGLEVEL_TRACE    5   /* code path tracing */
#define IPD_LOGLEVEL_DEBUG    6   /* debug information */
#define IPD_LOGLEVEL_VERBOSE  7   /* verbose logging */

#define ipd_crit(fmt, ...)      printk(KERN_CRIT "IPD-CRIT:%s:%d: " pr_fmt(fmt), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ipd_err(fmt, ...)     if (ipd_loglevel>=IPD_LOGLEVEL_ERR) \
                                printk(KERN_ERR "IPD-ERR:%s:%d: " pr_fmt(fmt), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ipd_warn(fmt, ...)    if (ipd_loglevel>=IPD_LOGLEVEL_WARN) \
                                printk(KERN_WARNING "IPD-WARN:%s:%d: " pr_fmt(fmt), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ipd_not(fmt, ...)     if (ipd_loglevel>=IPD_LOGLEVEL_NOT) \
                                printk(KERN_INFO "IPD-NOT:%s:%d: " pr_fmt(fmt), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ipd_info(fmt, ...)    if (ipd_loglevel>=IPD_LOGLEVEL_INFO) \
                                printk(KERN_INFO "IPD-INFO:%s:%d: " pr_fmt(fmt), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ipd_trace(fmt, ...)   if (ipd_loglevel>=IPD_LOGLEVEL_TRACE) \
                                printk(KERN_INFO "IPD-TRACE:%s:%d: " pr_fmt(fmt), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ipd_debug(fmt, ...)   if (ipd_loglevel>=IPD_LOGLEVEL_DEBUG) \
                                printk(KERN_INFO "IPD-DEBUG:%s:%d: " pr_fmt(fmt), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ipd_cont(fmt, ...)    if (ipd_loglevel>=IPD_LOGLEVEL_DEBUG) \
                                printk(KERN_CONT pr_fmt(fmt), ##__VA_ARGS__)
#define ipd_verbose(fmt, ...) if (ipd_loglevel>=IPD_LOGLEVEL_VERBOSE) \
                                printk(KERN_INFO "IPD-VERBOSE:%s:%d: " pr_fmt(fmt), __FUNCTION__, __LINE__, ##__VA_ARGS__)

extern uint32_t ipd_loglevel;

/** @brief Ring stats
 *
 */
typedef struct inno_ring_stats {
    uint64_t descs;                     /** Packets sent/recvd */
    uint64_t packets;                   /** Packets sent/recvd */
    uint64_t bytes;                     /** Total bytes sent/recvd */
    uint64_t drops;                     /** Pacekts dropped */
    uint64_t ring_full;                 /** No space in ring */
} inno_ring_stats_t;

typedef struct inno_interrupt_stats {
    uint64_t num_int[NUM_MSIX_VECS];
}inno_rupt_stats_t;

typedef struct inno_driver_stats {
    uint32_t num_open;
    uint32_t num_close;
}inno_driver_stats_t;

typedef struct inno_tx_stats {
    uint64_t            tx_bypass_packets;
    uint64_t            tx_lookup_packets;
    uint64_t            tx_ptp_packets;
}inno_tx_stats_t;

typedef struct inno_wb_stats {
    uint64_t wb_crc;
    uint64_t wb_err;
} inno_wb_stats_t;

typedef struct inno_stats{
    inno_driver_stats_t inno_drv_stats;
    inno_rupt_stats_t   inno_rupt_stats;
    inno_ring_stats_t   tx_ring_stats[NUM_TX_RINGS];
    inno_ring_stats_t   rx_ring_stats[NUM_RX_RINGS];
    inno_tx_stats_t     tx_stats;
    inno_wb_stats_t     wb_stats;
}inno_stats_t;

inline static char *vec2str(int vector)
{
    switch(vector) {
        case MSIX_VECTOR_NAPI:
            return "napi";
        case MSIX_VECTOR_ASYNC:
            return "async";
        case MSIX_VECTOR_TX:
            return "tx";
        case MSIX_VECTOR_RX:
            return "rx";
        case MSIX_VECTOR_RX_MCU0:
            return "rx-mcu0";
        case MSIX_VECTOR_ERROR:
            return "err";
        case MSIX_VECTOR_LEARN:
            return "lrn";
        case MSIX_VECTOR_PIC:
            return "pic";
        default:
            return "unk";
    }
}

#endif /* __IPD_STATS_H__ */
