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

#include <linux/types.h>
#include <linux/if_vlan.h>
#include "pci_common_ipd.h"
#include "ipd.h"
#include "ldh_structs_ipd_v2.h"
#include "inno_ldh.h"
#include "inno_enet.h"
#include "inno_cmipd_v0.h"
#include "inno_cmipd_v2.h"

typedef struct ldh_v2_s {
    union {
        ldh_fixed_format_t ldh_fixed;
        ldh_vf0_t          vf0;
        ldh_vf1_t          vf1;
        ldh_vf2_t          vf2;
 /*  */
    } _u;
} inno_ldh_v2_t;


/* Unpack LDH into buffer */
int
inno_unpack_ldh_header_v2(uint8_t    *buf,
                          inno_ldh_t *ldh_hdr)
{
	inno_ldh_v2_t *ldh = (inno_ldh_v2_t  *) ldh_hdr;

    if ((buf == NULL) || (ldh == NULL)) {
        return -1;
    }

    unpack_ldh_fixed_format(buf, &ldh->_u.ldh_fixed);

    switch(ldh->_u.ldh_fixed.vf_type_f) {
    case 0:
        unpack_ldh_vf0(buf, &ldh->_u.vf0);
        break;
    default:
        ipd_err("Unpack-Unsupported VF %d",
               ldh->_u.ldh_fixed.vf_type_f);
        return -1;
    }
    return 0;
}

int
inno_vf2_queue_set_v2(uint8_t *buf,
                      uint8_t queue)
{
    inno_ldh_v2_t ldh;

    if (buf == NULL) {
        return -1;
    }

    unpack_ldh_fixed_format(buf, &ldh._u.ldh_fixed);
    if (ldh._u.ldh_fixed.vf_type_f == 2) {
        buf[4] &= 0xf;
        buf[4] |= (queue & 0xf) << 4;
    }
    else {
        ipd_err("Unpack-Unsupported VF %d",
                ldh._u.ldh_fixed.vf_type_f);
        return -1;
    }

    return 0;
}

uint32_t inno_get_ssp_v2(inno_ldh_t *ldh_hdr)
{
	inno_ldh_v2_t *ldh = (inno_ldh_v2_t  *) ldh_hdr;
	return ldh->_u.vf0.ssp_f;
}

bool
inno_debug_hdr_present_v2(inno_ldh_t *ldh_hdr)
{
	inno_ldh_v2_t *ldh = (inno_ldh_v2_t  *) ldh_hdr;
	if (ldh->_u.vf0._rsvd_fixed_0_f == 1) {
		return true;
	}
    return false;
}

/* Unpack ext header from buffer */
static void
inno_unpack_debug_header(uint8_t   *buf,
                         inno_info_header_t *ih,
                         uint16_t  *lvni,
                         uint16_t  *intf)
{
    uint8_t  e1 = 0;
    uint32_t vni = 0;
    uint32_t l3iif = 0;
    uint32_t t1 = 0, t2 = 0, t3 = 0;
    uint32_t nh_hw_index = 0;
    uint32_t sysdest_type = 0;
    uint32_t sysdest_value = 0;
    uint8_t  fwd_hdr_offset = 0;
    uint8_t  fwd_layer_type = 0;

    e1 = (buf[7] >> 1) & 0x7;

    switch(e1){
        case 0:
            vni = (uint32_t)buf[10] | (uint32_t)(buf[9] << 8);
            vni = (vni >> 3) & 0x1fff;
            break;
        case 1:
            nh_hw_index = (uint16_t) buf[10] | (uint16_t) (buf[9] << 8);
            vni = (uint32_t)buf[12] | (uint32_t)(buf[11] << 8);
            vni = (vni >> 3) & 0x1fff;
            t1 = (((uint32_t)(buf[12] & 0x7)) << 11);
            t2 = ((((uint32_t)buf[13] & 0xff)) << 3);
            t3 = ((buf[14] >> 5) & 0x7);
            l3iif = t1 | t2 | t3; // This is l2tag_action_ptr ??
            break;
        case 2:
            nh_hw_index = (uint16_t) buf[10] | (uint16_t) (buf[9] << 8);
            vni = (uint32_t)buf[12] | (uint32_t)(buf[11] << 8);
            vni = (vni >> 3) & 0x1fff;
            t1 = (((uint32_t)(buf[12] & 0x7)) << 11);
            t2 = ((((uint32_t)buf[13] & 0xff)) << 3);
            t3 = ((buf[14] >> 5) & 0x7);
            l3iif = t1 | t2 | t3;
            break;
        case 3:
            vni = (uint32_t) buf[12] | (uint32_t)(buf[11] << 8);
            vni = (vni >> 3) & 0x1fff;
            t1 = (((uint32_t)(buf[14] & 0x7)) << 11);
            t2 = ((((uint32_t)buf[15] & 0xff)) << 3);
            t3 = ((buf[16] >> 5) & 0x7);
            l3iif = t1 | t2 | t3;
            break;
        case 4:
            vni = ((uint32_t) ((buf[11] & 0xf) << 9) |
                   (uint32_t) ((buf[12] & 0xff) << 1) |
                   (uint32_t) ((buf[13] >> 7) & 0x1)) & 0x1fff;
            break;
        case 5:
            vni = ((uint32_t) ((buf[11] & 0xf) << 9) |
                   (uint32_t) ((buf[12] & 0xff) << 1) |
                   (uint32_t) ((buf[13] >> 7) & 0x1)) & 0x1fff;
            l3iif = ((uint32_t) ((buf[14] & 0x1) << 13) |
                     (uint32_t) ((buf[15] & 0xff) << 5) |
                     (uint32_t) ((buf[16] >> 3) & 0x1f)) & 0x3fff;
            break;
        default:
		    ipd_debug("ipd Unsupported EEP1 - %d\n", e1);
    }

    *lvni = (uint16_t)vni;
    *intf = (uint16_t)l3iif;
    sysdest_type  = (uint32_t) ((buf[5] >> 7) & 0x1);
    sysdest_value = (uint32_t) (((buf[5] & 0x7f) << 6) | ((buf[6] >> 2) & 0x3f));
    if (sysdest_type == 1){
        /* Destination is an MDG. Zero out the egress port in this case */
        ih->dsp = 0;
    } else {
        /* Destination is a Sysport */
        ih->dsp = sysdest_value;
    }
    fwd_hdr_offset = (uint8_t)(((buf[1] >> 5) & 0x7) | ((buf[0] & 0xf) << 3));
    ih->fwd_hdr_offset = (uint8_t)(2*fwd_hdr_offset);
    ih->nhoif = nh_hw_index;

    fwd_layer_type = (uint8_t)((buf[1] >> 2) & 0x7);
    switch (fwd_layer_type) {
        case 0:
            ih->fwd_layer_type = INNO_FWD_LAYER_TYPE_L2;
            break;
        case 1:
            ih->fwd_layer_type = INNO_FWD_LAYER_TYPE_IP;
            break;
        case 2:
            ih->fwd_layer_type = INNO_FWD_LAYER_TYPE_MPLS;
            break;
        case 3:
            ih->fwd_layer_type = INNO_FWD_LAYER_TYPE_BRIDGEROUTED;
            break;
        case 4:
            ih->fwd_layer_type = INNO_FWD_LAYER_TYPE_SYSLAYER;
            break;
        case 7:
            ih->fwd_layer_type = INNO_FWD_LAYER_TYPE_NON_IP_NON_MPLS;
            break;
        default:
            ih->fwd_layer_type = fwd_layer_type;
    }
    ipd_debug("E1 - %d VNI - 0x%x INTF - 0x%x fwd_layer_type - %d fwd_hdr_offset - %d sysdest_type - 0x%x sysdest_value - 0x%x nhoif - %d\n",
               e1, vni, l3iif, ih->fwd_layer_type, ih->fwd_hdr_offset, sysdest_type, sysdest_value, ih->nhoif);
}

static int
inno_unpack_ptp_ext_header(inno_device_t *idev,
                           inno_dma_alloc_t *dma,
                          inno_ldh_v2_t *ldh,
                           uint8_t  *ext_info_hdrs,
                           uint32_t *ptp_info_hdr_size)
{
    uint8_t *buf;
    uint64_t idts1 = 0, idts2 = 0, temp = 0;
    cpu_ptp_control_t cpu_ptp_ctrl;
    uint32_t idts_diff = 0;
    inno_ptp_info_header_t ptp_info_hdr = {0};
    uint32_t idts2_31_0, idts2_47_32, igts_sec_31_0, igts_sec_47_32, igts_nsec_31_0;
    uint64_t igts_sec;
    uint32_t igts_nsec;

	if (ldh->_u.vf0._rsvd_fixed_0_f) {
        /* if debug header is present */
        buf = dma->vmaddr + LDH_VF0_SIZE + idev->chip_dbg_hdr_len;
    } else {
        buf = dma->vmaddr + LDH_VF0_SIZE;
    }

    /* go to PTP SHIM header */
    buf += PTP_OUTER_ENCAP_SIZE;

    temp  = *(uint64_t*)(buf+8);
    idts1 = __builtin_bswap64(temp);

    idts1 = idts1 & 0xffffffffffff;

    /* trigger timers snapshot */
    cpu_ptp_ctrl.data = 0;
    cpu_ptp_ctrl.data = REG32(CPU_PTP_CONTROL);
    // CHECK why it was using k2???
    cpu_ptp_ctrl.tl10_flds.timers_snapshot_enable_f = 1;
    REG32(CPU_PTP_CONTROL) = cpu_ptp_ctrl.data;
    /* read igts and idts2 from device */
    idts2_31_0 = REG32(CPU_PTP_DEVICETIMER_SNAPSHOT_31_0);
    idts2_47_32 = REG32(CPU_PTP_DEVICETIMER_SNAPSHOT_47_32);
    igts_sec_31_0 = REG32(CPU_PTP_SYNCHRONIZEDTIME_SNAPSHOT_SEC_31_0);
    igts_sec_47_32 = REG32(CPU_PTP_SYNCHRONIZEDTIME_SNAPSHOT_SEC_47_32);
    igts_nsec_31_0 = REG32(CPU_PTP_SYNCHRONIZEDTIME_SNAPSHOT_NS_31_0);

    /* not required to set snapshot back to 0 */

 /*  */
    /* 48bit device timer */
    idts2 = (((uint64_t)idts2_47_32) << 32) | idts2_31_0;
    if(idts2 >= idts1) {
        idts_diff = idts2 - idts1;
    } else {
        /* devicetimer rolled over */
        idts_diff = NANOSEC_PER_SEC - idts1 + idts2;
    }

    igts_sec = (((uint64_t)igts_sec_47_32) << 32) | igts_sec_31_0;
    igts_nsec = igts_nsec_31_0;

    if(igts_nsec >= idts_diff) {
        ptp_info_hdr.ptp_timestamp_nanosec = igts_nsec - idts_diff;
        ptp_info_hdr.ptp_timestamp_sec = igts_sec;
    } else {
        ptp_info_hdr.ptp_timestamp_nanosec = NANOSEC_PER_SEC - igts_nsec + idts_diff;
        ptp_info_hdr.ptp_timestamp_sec = igts_sec - 1;
    }

    *ptp_info_hdr_size = sizeof(inno_ptp_info_header_t);
    memcpy((uint8_t *)ext_info_hdrs, (uint8_t *)&ptp_info_hdr, sizeof(inno_ptp_info_header_t));

    ipd_debug("ptp timestamp sec: %llu ns: %u",
              ptp_info_hdr.ptp_timestamp_sec,
              ptp_info_hdr.ptp_timestamp_nanosec);
    return 0;
}

void
inno_populate_inno_hdr_v2(inno_ldh_t         *ldh_hdr,
                          inno_info_header_t *ih,
						  inno_dma_alloc_t  *dma,
                          uint8_t           ext_hdr_type,
                          uint8_t           ext_hdrs_size)
{
	inno_ldh_v2_t *ldh = (inno_ldh_v2_t  *) ldh_hdr;

    switch (ldh->_u.ldh_fixed.vf_type_f){
    case 0:
        /* if cpu metadate is valid and mirror bit not set */
        if (ldh->_u.vf0.cpu_metadata_f && !ldh->_u.vf0._rsvd_fixed_2_f){

	        ih->ssp = ldh->_u.vf0.ssp_f;
	        ih->queue = ldh->_u.vf0.cpu_queue_num_f;
	        ih->trap = ldh->_u.vf0.cpu_metadata_f;
	        /* read the debug header based on its presence */
	        if (ldh->_u.vf0._rsvd_fixed_0_f) {
                uint16_t vni;
                uint16_t intf;

		        inno_unpack_debug_header((dma->vmaddr + LDH_VF0_SIZE), ih, &vni, &intf);
                if (ih->fwd_layer_type == INNO_FWD_LAYER_TYPE_IP ||
                    ih->fwd_layer_type == INNO_FWD_LAYER_TYPE_BRIDGEROUTED ||
                    ih->fwd_layer_type == INNO_FWD_LAYER_TYPE_NON_IP_NON_MPLS) {
                    ih->l3vni = vni;
                    ih->l3if_phy_index = intf;
                } else {
                    ih->l2vni = vni;
                }
            }
	     } else if (ldh->_u.vf0._rsvd_fixed_2_f == 1) {
           /* Mirror Copy */
	        ih->ssp = ldh->_u.vf0.ssp_f;
	        ih->queue = ldh->_u.vf0.cpu_queue_num_f;
	        ih->trap = ldh->_u.vf0.cpu_metadata_f;
            ih->rx_info |= INNO_RX_PKT_TYPE_MIRROR_TO_CPU;
	        /* read the debug header based on its presence and ssp id valid*/
	        if (ldh->_u.vf0._rsvd_fixed_0_f && ih->ssp < NUM_SYSPORTS) {
                uint16_t vni;
                uint16_t intf;

		        inno_unpack_debug_header((dma->vmaddr + LDH_VF0_SIZE), ih, &vni, &intf);
                if (ih->fwd_layer_type == INNO_FWD_LAYER_TYPE_IP ||
                    ih->fwd_layer_type == INNO_FWD_LAYER_TYPE_BRIDGEROUTED ||
                    ih->fwd_layer_type == INNO_FWD_LAYER_TYPE_NON_IP_NON_MPLS) {
                    ih->l3vni = vni;
                    ih->l3if_phy_index = intf;
                } else {
                    ih->l2vni = vni;
                }
            }
         } else {
            /* CPU Metadata is 0 and Mirror bit not set.
             * Packet being switched or Routed to CPU.
             */
	        ih->ssp = ldh->_u.vf0.ssp_f;
	        ih->queue = ldh->_u.vf0.cpu_queue_num_f;
	        /* read the debug header based on its presence */
	        if (ldh->_u.vf0._rsvd_fixed_0_f) {
                uint16_t vni;
                uint16_t intf;

		        inno_unpack_debug_header((dma->vmaddr + LDH_VF0_SIZE), ih, &vni, &intf);
                if (ih->fwd_layer_type == INNO_FWD_LAYER_TYPE_IP ||
                    ih->fwd_layer_type == INNO_FWD_LAYER_TYPE_BRIDGEROUTED ||
                    ih->fwd_layer_type == INNO_FWD_LAYER_TYPE_NON_IP_NON_MPLS) {
                    ih->l3vni = vni;
                    ih->l3if_phy_index = intf;
                    ih->rx_info |= INNO_RX_PKT_TYPE_ROUTE_TO_CPU;
                } else {
                    ih->l2vni = vni;
                    ih->rx_info |= INNO_RX_PKT_TYPE_SWITCH_TO_CPU;
                }
            } else {
                ih->rx_info |= INNO_RX_PKT_TYPE_SWITCH_TO_CPU;
            }
         }
         break;

    default:
		ipd_debug("ipd unsupported vf_type: %u\n",ldh->_u.ldh_fixed.vf_type_f);
    }

    ih->ext_type = ext_hdr_type;
    ih->ext_size = ext_hdrs_size;
}

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
int
inno_unpack_ext_hdrs_v2(inno_device_t *idev,
                        inno_dma_alloc_t *dma,
                        inno_ldh_t *ldh_hdr,
                        uint32_t *ext_hdrs_size,
                        uint8_t  *ext_info_hdrs,
                        uint32_t *ext_info_hdrs_size,
                        uint8_t  *ext_hdr_type,
                        uint32_t *sp)
{
	uint8_t  *curr_info_hdr = ext_info_hdrs;
	inno_ldh_v2_t *ldh = (inno_ldh_v2_t  *) ldh_hdr;

	/* Check if packet is encapped with PTP shim header */
	/* For now, only checking if the packet is received on PTP-event-messaging queue, as all
	 * the packets on this queue will be encapsulated with PTP shim header by device
     */
 /*  */
	if(idev->inno_netdev.ptp_event_queue_valid == 1 &&
			idev->inno_netdev.ptp_event_queue_id == ldh->_u.vf0.cpu_queue_num_f) {
		uint32_t ptp_info_hdr_size = 0;

		*ext_hdrs_size += PTP_ENCAP_HDRS_SIZE;

		ipd_debug("ptp ext hdr present");
		inno_unpack_ptp_ext_header(idev, dma, ldh, curr_info_hdr, &ptp_info_hdr_size);

		*ext_info_hdrs_size += ptp_info_hdr_size;
		curr_info_hdr += ptp_info_hdr_size;
		*ext_hdr_type = INNO_HEADER_EXTENSION_TYPE_PTP;
	}

	if ((ldh->_u.vf0._rsvd_fixed_2_f == 1)) {
		uint32_t *pkt_ptr, gre_proto_type, eh_type, offset;
		struct ethhdr         *ehdr;

		if (ldh->_u.vf0._rsvd_fixed_0_f) {
			/* if debug header is present */
			offset = LDH_VF0_SIZE + idev->chip_dbg_hdr_len;
		} else {
			offset = LDH_VF0_SIZE;
		}
		ehdr = (struct ethhdr*)(dma->vmaddr + offset);
		ipd_debug("RX packet proto is 0x%x\n", htons(ehdr->h_proto));

		if(ehdr->h_proto == htons(ETH_P_8021Q)) {
			offset += (VLAN_ETH_HLEN + 20);
		} else {
			offset += (VLAN_ETH_HLEN - VLAN_HLEN + 20);
		}
		pkt_ptr = (uint32_t*)(dma->vmaddr + offset);
		gre_proto_type = htonl(*pkt_ptr)&0xffff;
		if(gre_proto_type == idev->inno_params_info.shim_gre_proto) {
			offset+=8;
			pkt_ptr = (uint32_t*)(dma->vmaddr + offset);
			eh_type = htonl(*pkt_ptr)>>26;
			if(eh_type == 2){
				*ext_hdr_type = INNO_HEADER_EXTENSION_TYPE_SFLOW;
                *sp = htonl(*pkt_ptr)&0x7fff;
			}
		}
	}

	/* any other ext headers, check here; if yes, do something like this */
	/*
	 * if(xxx_ext_hdr_present) {
	 *     inno_unpack_xxx_ext_header(idev, dma, ldh_hdr, curr_info_hdr, &xxx_info_hdr_size);
	 *     *ext_info_hdrs_size += xxx_info_hdr_size;
	 *     curr_info_hdr += xxx_info_hdr_size;
	 * }
	 */

	return 0;
}
