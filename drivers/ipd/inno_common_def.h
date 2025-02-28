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

#ifndef _INNO_COMMON_DEF_H_
#define _INNO_COMMON_DEF_H_


/* Info header flags */
#define INNO_TX_TYPE_PIPELINE_LOOKUP    0x1 /* standard pipeline lookup; not supported for PTP packets */
#define INNO_TX_TYPE_ACL_BYPASS_LOOKUP  0x2 /* Goes through the pipeline bypassing the ACL lookup */
#define INNO_TX_TYPE_PTP_XCONNECT       0x4 /* Goes through the pipeline destined to the user specified sysport */
#define INNO_TX_QUEUE_NUM_VALID         0x8 /* Is TX queue number valid in the inno info header */

/* RX Info header flag */
#define INNO_RX_PKT_TYPE_SWITCH_TO_CPU  0x1
#define INNO_RX_PKT_TYPE_MIRROR_TO_CPU  0x2
#define INNO_RX_PKT_TYPE_ROUTE_TO_CPU   0x4

#define INNO_FWD_LAYER_TYPE_L2            0
#define INNO_FWD_LAYER_TYPE_IP            1
#define INNO_FWD_LAYER_TYPE_MPLS          2
#define INNO_FWD_LAYER_TYPE_BRIDGEROUTED  3
#define INNO_FWD_LAYER_TYPE_SYSLAYER      4
#define INNO_FWD_LAYER_TYPE_NON_IP_NON_MPLS  7

/* Info header for TX/RX packet for single netdev interface config */
typedef struct __attribute__((__packed__))  inno_info_header_s {
    uint8_t     ver;              /** Version info */
    uint16_t    ssp;              /** Source System Port */
    uint16_t    dsp;              /** Destination System Port */
    uint8_t     flag;             /** flag */
    uint8_t     queue;            /** CPU queue */
    uint8_t     trap;             /** Trap number */
    uint16_t    l2vni;            /** L2 VNI */
    uint8_t     rx_info;          /** RX packet additional information */
    uint16_t    l3vni;            /** L3 VNI */
    uint16_t    l3if_phy_index;   /** Physical Index to Layer-3 interface */
    uint8_t     fwd_hdr_offset;   /** Header offset in the packet that used for forwarding */
    uint8_t     fwd_layer_type:3; /** Indicates the type of the forwarding layer, defined as INNO_FWD_LAYER_TYPE_<> */
    uint8_t     rsvd2:5;          /** Reserved field */
    uint32_t    nhoif;            /** Nexthop of routed packet - supported for TL10 and later */
    uint8_t     rsvd[1];          /** Reserved field */
    uint8_t     ext_size;         /** Extension hdr size */
    uint8_t     ext_type;         /** Extension hdr type */
} inno_info_header_t;

typedef struct __attribute__((__packed__))  inno_ptp_header_s {
    uint64_t    ptp_timestamp_sec;
    uint32_t    ptp_timestamp_nanosec;
} inno_ptp_info_header_t;
#endif /* _INNO_COMMON_DEF_H_ */
