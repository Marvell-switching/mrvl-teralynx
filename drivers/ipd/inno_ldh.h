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

/*
 * inno_ldh.h
 *
 */
#ifndef __INNO_LDH_H__
#define __INNO_LDH_H__

/* 20 Bytes of LDH */
#define LDH_HEADER_MAX_SIZE  20
/* LDH + Debug header */
#define CHIP_HEADER_SIZE (idev->chip_hdr_len + idev->chip_dbg_hdr_len)
/* ptp shim header size */
#define PTP_SHIM_HDR_SIZE   16
/* ptp outer encapsulations: OuterL2(14B) + OuterL3(20B) + GRE(4B) */
#define PTP_OUTER_ENCAP_SIZE    (14 + 20 + 4)
/* ptp inner encapsulations: InnerL2(18B) + InnerL3(20B) + UDP(8B) */
#define L2_IPV4_UDP_HDR_SIZE    (18 + 20 + 8)

/* ptp total device header size PTP_OUTER_ENCAP_SIZE + PTP_SHIM */
#define PTP_ENCAP_HDRS_SIZE (PTP_OUTER_ENCAP_SIZE + PTP_SHIM_HDR_SIZE)

#define EXT_HDRS_MAX_LEN    128

#define NANOSEC_PER_SEC     1000000000

typedef enum inno_header_ext_type_s {
	INNO_HEADER_EXTENSION_TYPE_NONE = 0,
	INNO_HEADER_EXTENSION_TYPE_PTP = 1,
	INNO_HEADER_EXTENSION_TYPE_SFLOW = 2
}inno_header_ext_type_t;

typedef struct ldh_s {
    union {
        uint8_t ldh_fixed[LDH_HEADER_MAX_SIZE+4];
        uint8_t vf0[LDH_HEADER_MAX_SIZE+4];
        uint8_t vf1[LDH_HEADER_MAX_SIZE+4];
        uint8_t vf2[LDH_HEADER_MAX_SIZE+4];
 /*  */
    } _u;
} inno_ldh_t;

#endif //__INNO_LDH_H__
