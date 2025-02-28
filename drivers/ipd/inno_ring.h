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

#ifndef __INNO_RING_H__
#define __INNO_RING_H__

/** @file inno_ring.h
 *
 * @brief Header file for innovium ring H/W and public S/S structs
 *
 */
#include <linux/types.h>
#include <asm/byteorder.h>

/* A few gloabl defines */
#define NUM_TX_RINGS         6
#define NUM_TX_SWITCH_RINGS  4
#define TX_TO_MCU0_RING      4
#define TX_TO_MCU1_RING      5
#define RX_RING_OFFSET       7
#define NUM_RX_CPU_RINGS     4
#define NUM_RX_RINGS         6
#define NUM_RX_SWITCH_RINGS  4
#define RX_FROM_MCU0_RING    4
#define RX_FROM_MCU1_RING    5
#define SYNC_WB_SIZE       (4 + 0x100)

#define RING_IDX_MASKED(idx)    ((idx) & 0x7fff)

#define RING_SPACE(size, pidx, cidx)                                 \
    ((((pidx) >> 15) == ((cidx) >> 15)) ? ((size) + (cidx) - (pidx)) \
     : (RING_IDX_MASKED(cidx) - RING_IDX_MASKED(pidx)))

#define RING_IDX_INCR(size, idx) \
    idx =                        \
        ((RING_IDX_MASKED(++idx) != size) ? (idx) : (((idx) & 0x8000) ^ 0x8000))

#define RING_IDX_COMPL(idx) \
    ((idx) ^ 0x8000)

#define LEARN_IDX_MASKED(idx)    ((idx) & 0xfff)

#define LEARN_SPACE(size, cidx, pidx)                                \
    ((((pidx) >> 12) == ((cidx) >> 12)) ? ((pidx) - (cidx)) \
     : ((size) - LEARN_IDX_MASKED(cidx) + LEARN_IDX_MASKED(pidx)))

#define LEARN_IDX_INCR(size, idx) \
    idx =                        \
        ((LEARN_IDX_MASKED(++idx) != size) ? (idx) : (((idx) & 0x1000) ^ 0x1000))

#define LEARN_IDX_COMPL(idx) \
    ((idx) ^ 0x1000)

#define LEARN_DATA_IDX_MASKED(idx)    ((idx) & 0xfffff)
/* Wraparound bit */
#define LEARN_DATA_IDX_WA(idx)    ((idx) & 0x100000)
/* Increment and wraparound the learn idx */
#define LEARN_DATA_IDX_INCR_WA(size, idx, incr) \
    (((LEARN_DATA_IDX_MASKED(idx+(incr)))%size) | (LEARN_DATA_IDX_WA(idx+(incr))))

#define LEARN_MSGADDR_DIFF(b, a, size) \
    ((b>a) ? b-a : (size - a + b))

#define LEARN_DATA_INCR(size, idx, incr) \
    idx = ((LEARN_DATA_IDX_MASKED(idx+(incr)) < size) ? \
    (idx+(incr)) : \
    (((LEARN_DATA_IDX_INCR_WA(size, idx, incr)) & 0x1fffff) ^ 0x100000))

/* Increment learn data offset by incr
 * When the device skips writing to the hole at
 * end of the page, skip those bytes as well.
 * if the data pidx nearing 4K boundary, there aren't
 * whole 24 bytes the device goes to next 4K boundary
 * skipping some space.
 * Note that msgaddr is in bytes and incr is in DW
 */
#define LEARN_DATA_IDX_INCR(size, idx, incr) \
    LEARN_DATA_INCR(size, idx, incr); \
    if (learn->prev_wbmsgaddr != 0) { \
        if ( LEARN_MSGADDR_DIFF(wb.msgaddr, learn->prev_wbmsgaddr, (size << 3)) > LEARN_DATA_LEN) { \
            LEARN_DATA_INCR(size, idx, (LEARN_MSGADDR_DIFF(wb.msgaddr, learn->prev_wbmsgaddr, (size << 3)) - LEARN_DATA_LEN)>>3); \
        } \
    } \
    learn->prev_wbmsgaddr = wb.msgaddr


#define LEARN_DATA_IDX_COMPL(idx) \
    ((idx) ^ 0x100000)

/*
 * Hardware-defined data structures
 */
/** @brief TX descriptor ring entry
 *
 */
typedef struct inno_tx_desc {
    __be32 hsn_upper;                 /** HSN address upper 32 bits */
    __be32 hsn_lower;                 /** HSN address lower 31 bits + 0 */
    __be32 msn_addr;                  /** MSN addr for block xfers */
#if defined(__LITTLE_ENDIAN_BITFIELD)
    __u32      dd    : 1,                 /** Done flag */
               eop   : 1,                 /** End of packet */
               sop   : 1,                 /** Start of packet */
               rsvrd : 13,
               length : 16;               /** Packet/block length */
#elif defined(__BIG_ENDIAN_BITFIELD)
    __u32      length : 16,               /** Packet/block length */
               rsvrd : 13,
               sop   : 1,                 /** Start of packet */
               eop   : 1,                 /** End of packet */
               dd    : 1;                 /** Done flag */
#else
#error  "Adjust your <asm/byteorder.h> defines"
#endif
} __attribute__((packed)) inno_tx_desc_t;

/** @brief TX writeback ring entry
 *
 */
/* TX writeback ring entry */
typedef struct inno_tx_wb {
#if defined(__LITTLE_ENDIAN_BITFIELD)
    __u32 rsvrd1 : 27,
          crc    : 1,                     /** CRC error */
          err    : 1,                     /** Error */
          rsvrd2 : 2,
          dd     : 1;                     /** Done flag */
#elif defined(__BIG_ENDIAN_BITFIELD)
    __u32 dd     : 1,                     /** Done flag */
          rsvrd2 : 2,
          err    : 1,                     /** Error */
          crc    : 1,                     /** CRC error */
          rsvrd1 : 27;
#else
#error  "Adjust your <asm/byteorder.h> defines"
#endif
} __attribute__((packed)) inno_tx_wb_t;

/** @brief X descriptor ring entry
 *
 */
/* RX descriptor ring entry */
typedef struct inno_rx_desc {
    volatile uint32_t hsn_upper;         /** HSN address upper 32 bits */
    volatile uint32_t hsn_lower;         /** HSN address lower 31 bits + 0 */
} inno_rx_desc_t;

/** @brief RX writeback ring entry
 *
 */
/* RX writeback ring entry */

typedef struct inno_rx_wb {
    __be32      rsvrd1 : 32;
#if defined(__LITTLE_ENDIAN_BITFIELD)
    __u32      dd     : 1,                /** Done flag */
               eop    : 1,                /** End of packet */
               sop    : 1,                /** Start of packet */
               err    : 1,                /** Error */
               ecc    : 1,                /** ECC error */
               rsvrd2 : 11,
               length : 16;               /** Received length */
#elif defined(__BIG_ENDIAN_BITFIELD)
    __u32      length : 16,               /** Received length */
               rsvrd2 : 11,
               ecc    : 1,                /** ECC error */
               err    : 1,                /** Error */
               sop    : 1,                /** Start of packet */
               eop    : 1,                /** End of packet */
               dd     : 1;                /** Done flag */
#else
#error  "Adjust your <asm/byteorder.h> defines"
#endif
} __attribute__((packed)) inno_rx_wb_t;

/** @brief Learn writeback ring entry
 *
 */
/* Learn writeback ring entry */

typedef struct inno_learn_wb {
    __be32      msgaddr: 32;
#if defined(__LITTLE_ENDIAN_BITFIELD)
    __u32      dd     : 1,           /** Done flag */
               eop    : 1,           /** End of packet */
               sop    : 1,           /** Start of packet */
               err    : 1,           /** Error */
               ecc    : 1,           /** ECC error */
               rsvrd2 : 11,
               length : 16;               /** Received length */
#elif defined(__BIG_ENDIAN_BITFIELD)
    __u32      length : 16,               /** Received length */
               rsvrd2 : 11,
               ecc    : 1,           /** ECC error */
               err    : 1,           /** Error */
               sop    : 1,           /** Start of packet */
               eop    : 1,           /** End of packet */
               dd     : 1;           /** Done flag */
#else
#error  "Adjust your <asm/byteorder.h> defines"
#endif
} __attribute__((packed)) inno_learn_wb_t;

/** @brief PIC status chain writeback entry
 *
 */
/* PIC status chain writeback entry */

typedef struct inno_pic_st_wb {
#if defined(__LITTLE_ENDIAN_BITFIELD)
    __u32    ib_id            :3,
             pic_id           :3,
             lane_id          :3,
             link_speed       :4,
             fec_type         :2,
             enabled          :1,
             signal_detected  :1,
             autoneg_master   :1,
             autoneg_training :1,
             link_up          :1,
             rx_link_active   :1,
             tx_link_active   :1,
             local_fault      :1,
             remote_fault     :1,
             error            :1,
             parity           :1,
             reserved         :6;
#elif defined(__BIG_ENDIAN_BITFIELD)
    __u32    reserved         :6,
             parity           :1,
             error            :1,
             remote_fault     :1,
             local_fault      :1,
             tx_link_active   :1,
             rx_link_active   :1,
             link_up          :1,
             autoneg_training :1,
             autoneg_master   :1,
             signal_detected  :1,
             enabled          :1,
             fec_type         :2,
             link_speed       :4,
             lane_id          :3,
             pic_id           :3,
             ib_id            :3;
#else
#error  "Adjust your <asm/byteorder.h> defines"
#endif
} __attribute__((packed)) inno_pic_st_wb_t;

/** @brief async host response ring entry
 *
 */
typedef struct inno_async_hrr {
    sync_sts_0_t status;                /** Status like sync */
    uint8_t      data[256];             /** Returned data */
} __attribute__((packed)) inno_async_hrr_t;

/** @brief learn ring entry
 *
 */
typedef struct inno_learn_data_s {
    uint8_t data[144];                  /** Returned data */
} __attribute__((packed)) inno_learn_data_t;

#endif /* __INNO_RING_H__ */
