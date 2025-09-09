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

/**
 * @file inno_ldh_v0.c
 * @brief Innovium Platform Driver Implementation for LDH processing
 */

#include <linux/types.h>
#include <linux/if_vlan.h>
#include "pci_common_ipd.h"
#include "ipd.h"
#include "ldh_structs_ipd_v0.h"
#include "inno_ldh.h"
#include "inno_enet.h"
#include "inno_cmipd_v0.h"

typedef struct ldh_v0_s {
    union {
        ldh_fixed_format_t ldh_fixed;
        ldh_vf0_t          vf0;
        ldh_vf1_t          vf1;
        ldh_vf2_t          vf2;
 /*  */
    } _u;
} inno_ldh_v0_t;


/* Unpack LDH into buffer */
int
inno_unpack_ldh_header_v0(uint8_t    *buf,
                          inno_ldh_t *ldh_hdr)
{
	inno_ldh_v0_t *ldh = (inno_ldh_v0_t  *) ldh_hdr;

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
inno_vf2_queue_set_v0(uint8_t *buf,
                      uint8_t queue)
{
    inno_ldh_v0_t ldh;

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

uint32_t inno_get_ssp_v0(inno_ldh_t *ldh_hdr)
{
	inno_ldh_v0_t *ldh = (inno_ldh_v0_t  *) ldh_hdr;
	return ldh->_u.vf0.ssp_f;
}

bool
inno_debug_hdr_present_v0(inno_ldh_t *ldh_hdr)
{
	inno_ldh_v0_t *ldh = (inno_ldh_v0_t  *) ldh_hdr;
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
    cfx_v0_t cmf = {};
    ipd_fields_v0_t ipdfld;
    uint32_t nh_hw_index = 0;
    int rc;
    rc = cfx_v0_init(&cmf);
    if (rc != 0)
        ipd_err("cm init failed rc = %d",rc);
    rc = cfx_v0_unpack(&cmf, buf);
    if (rc != 0)
        ipd_err("cm unpack failed rc = %d",rc);
    inno_unpack_cmhdr_v0(buf, &ipdfld);
    *lvni = (uint16_t)ipdfld.vni;
    *intf = (uint16_t)ipdfld.l3iif;

    switch(ipdfld.e1){
    case 1:
    /* No break on purpose - fallthrough */
    case 2:
    {
        nh_hw_index = (uint16_t) buf[9] | (uint16_t) (buf[8] << 8);
    }
    break;
    default:
        ipd_debug("ipd Unsupported EEP1 - %d\n", ipdfld.e1);
}

    if (cmf.ipd_f13.value[0] == 1){
    /* Destination is an MDG. Zero out the egress port in this case */
        ih->dsp = 0;
    } else {
    /* Destination is a Sysport */
        ih->dsp = (uint32_t)cmf.ipd_f14.value[0];
    }
    ih->fwd_hdr_offset = (uint8_t)(2*cmf.ipd_f4.value[0]);
    ih->nhoif = nh_hw_index;

    switch (cmf.ipd_f5.value[0]) {
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
            ih->fwd_layer_type = cmf.ipd_f5.value[0];
    }
    ipd_debug("E1 -%d VNI - 0x%x INTF - 0x%x fwd_layer_type - %d fwd_hdr_offset - %d sysdest_type - 0x%x sysdest_value - 0x%x nhoif - %d\n",
               ipdfld.e1, ipdfld.vni, ipdfld.l3iif, ih->fwd_layer_type, ih->fwd_hdr_offset, cmf.ipd_f13.value[0], cmf.ipd_f14.value[0],
               ih->nhoif);
}

void
inno_populate_inno_hdr_v0(inno_ldh_t         *ldh_hdr,
                          inno_info_header_t *ih,
						  inno_dma_alloc_t  *dma,
                          uint8_t           ext_hdr_type,
                          uint8_t           ext_hdrs_size)
{
	inno_ldh_v0_t *ldh = (inno_ldh_v0_t  *) ldh_hdr;

    switch (ldh->_u.ldh_fixed.vf_type_f){
    case 0:
        /* if cpu metadate is valid and mirro bit not set*/
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
            /* read the debug header based on its presence and ssp is valid */
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
        ipd_debug("ipd unsupported, vf_type: %u\n",ldh->_u.ldh_fixed.vf_type_f);
    }
    ih->ext_type = ext_hdr_type;
    ih->ext_size = ext_hdrs_size;
}
int
inno_unpack_ext_hdrs_v0(inno_device_t *idev,
                        inno_dma_alloc_t *dma,
                        inno_ldh_t *ldh_hdr,
                        uint32_t *ext_hdrs_size,
                        uint8_t  *ext_info_hdrs,
                        uint32_t *ext_info_hdrs_size,
                        uint8_t  *ext_hdr_type,
                        uint32_t *sp)
{
	inno_ldh_v0_t *ldh = (inno_ldh_v0_t  *) ldh_hdr;
	if (ldh->_u.vf0._rsvd_fixed_2_f == 1) {
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
		if(gre_proto_type == idev->inno_params_info.shim_gre_proto){
			offset+=8;
			pkt_ptr = (uint32_t*)(dma->vmaddr + offset);
			eh_type = htonl(*pkt_ptr)>>26;
			if(eh_type == 2){
				*ext_hdr_type = INNO_HEADER_EXTENSION_TYPE_SFLOW;
				*sp = htonl(*pkt_ptr)&0x7fff;
			}
		}
	}


	return 0;
}
