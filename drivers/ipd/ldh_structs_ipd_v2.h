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

 /*  */

#ifndef __LDH_STRUCTS_IPD_V2_H__
#define __LDH_STRUCTS_IPD_V2_H__



typedef struct {
 /*  */
    uint8_t _rsvd_fixed_0_f : 1;
    uint8_t _rsvd_fixed_1_f : 1;
    uint8_t _rsvd_fixed_2_f : 1;
    uint8_t _rsvd_fixed_3_f : 4;
    uint8_t proc_type_f : 2;
    uint8_t _rsvd_fixed_5_f : 1;
    uint8_t _rsvd_fixed_6_f : 1;
    uint8_t _rsvd_fixed_7_f : 1;
    uint8_t _rsvd_fixed_8_f : 3;
    uint8_t vf_type_f : 3;
    uint8_t version_f : 4;
    uint8_t _rsvd_fixed_11_f : 2;
} ldh_fixed_format_t;

#define LDH_FIXED_FORMAT_SIZE 3

static inline void pack_ldh_fixed_format(ldh_fixed_format_t *val, uint8_t *packed)
{
    memset(packed, 0, 3);
    packed[0] |= (val->_rsvd_fixed_0_f & 0x1) << 7;
    packed[0] |= (val->_rsvd_fixed_1_f & 0x1) << 6;
    packed[0] |= (val->_rsvd_fixed_2_f & 0x1) << 5;
    packed[0] |= (val->_rsvd_fixed_3_f & 0xf) << 1;
    packed[0] |= (val->proc_type_f & 0x2) >> 1;
    packed[1] |= (val->proc_type_f & 0x1) << 7;
    packed[1] |= (val->_rsvd_fixed_5_f & 0x1) << 6;
    packed[1] |= (val->_rsvd_fixed_6_f & 0x1) << 5;
    packed[1] |= (val->_rsvd_fixed_7_f & 0x1) << 4;
    packed[1] |= (val->_rsvd_fixed_8_f & 0x7) << 1;
    packed[1] |= (val->vf_type_f & 0x4) >> 2;
    packed[2] |= (val->vf_type_f & 0x3) << 6;
    packed[2] |= (val->version_f & 0xf) << 2;
    packed[2] |= (val->_rsvd_fixed_11_f & 0x3) << 0;
}

static inline void unpack_ldh_fixed_format(uint8_t *packed, ldh_fixed_format_t *val)
{
    val->_rsvd_fixed_0_f = (uint8_t) (((uint8_t) (packed[0] & 0x80)) >> 7);
    val->_rsvd_fixed_1_f = (uint8_t) (((uint8_t) (packed[0] & 0x40)) >> 6);
    val->_rsvd_fixed_2_f = (uint8_t) (((uint8_t) (packed[0] & 0x20)) >> 5);
    val->_rsvd_fixed_3_f = (uint8_t) (((uint8_t) (packed[0] & 0x1e)) >> 1);
    val->proc_type_f = (uint8_t) (((uint8_t) (packed[0] & 0x1)) << 1) |
        (uint8_t) (((uint8_t) (packed[1] & 0x80)) >> 7);
    val->_rsvd_fixed_5_f = (uint8_t) (((uint8_t) (packed[1] & 0x40)) >> 6);
    val->_rsvd_fixed_6_f = (uint8_t) (((uint8_t) (packed[1] & 0x20)) >> 5);
    val->_rsvd_fixed_7_f = (uint8_t) (((uint8_t) (packed[1] & 0x10)) >> 4);
    val->_rsvd_fixed_8_f = (uint8_t) (((uint8_t) (packed[1] & 0xe)) >> 1);
    val->vf_type_f = (uint8_t) (((uint8_t) (packed[1] & 0x1)) << 2) |
        (uint8_t) (((uint8_t) (packed[2] & 0xc0)) >> 6);
    val->version_f = (uint8_t) (((uint8_t) (packed[2] & 0x3c)) >> 2);
    val->_rsvd_fixed_11_f = (uint8_t) (((uint8_t) (packed[2] & 0x3)) >> 0);
}

typedef struct {
 /*  */
    uint16_t _rsvd_vf0_0_f : 10;
    uint16_t slp_f : 10;
    uint8_t _rsvd_vf0_2_f : 2;
    uint8_t system_qos_f : 6;
    uint8_t _rsvd_vf0_4_f : 1;
    uint16_t ssp_f : 11;
    uint8_t _rsvd_vf0_6_f : 4;
    uint8_t _rsvd_vf0_7_f : 8;
    uint8_t _rsvd_vf0_8_f : 4;
    uint8_t cpu_metadata_f : 8;
    uint8_t _rsvd_vf0_10_f : 2;
    uint8_t cpu_queue_num_f : 6;
    uint64_t _rsvd_vf0_12_f : 64;
} ldh_vf0_format_t;

#define LDH_VF0_FORMAT_SIZE 17

static inline void pack_ldh_vf0_format(ldh_vf0_format_t *val, uint8_t *packed)
{
    memset(packed, 0, 17);
    packed[1] |= (val->slp_f & 0x3f0) >> 4;
    packed[2] |= (val->slp_f & 0xf) << 4;
    packed[2] |= (val->system_qos_f & 0x30) >> 4;
    packed[3] |= (val->system_qos_f & 0xf) << 4;
    packed[3] |= (val->ssp_f & 0x700) >> 8;
    packed[4] |= (val->ssp_f & 0xff) << 0;
    packed[5] |= (val->_rsvd_vf0_7_f & 0xf0) >> 4;
    packed[6] |= (val->_rsvd_vf0_7_f & 0xf) << 4;
    packed[7] |= (val->cpu_metadata_f & 0xff) << 0;
    packed[8] |= (val->cpu_queue_num_f & 0x3f) << 0;
}

static inline void unpack_ldh_vf0_format(uint8_t *packed, ldh_vf0_format_t *val)
{
    val->slp_f = (uint16_t) (((uint16_t) (packed[1] & 0x3f)) << 4) |
        (uint16_t) (((uint16_t) (packed[2] & 0xf0)) >> 4);
    val->system_qos_f = (uint8_t) (((uint8_t) (packed[2] & 0x3)) << 4) |
        (uint8_t) (((uint8_t) (packed[3] & 0xf0)) >> 4);
    val->ssp_f = (uint16_t) (((uint16_t) (packed[3] & 0x7)) << 8) |
        (uint16_t) (((uint16_t) (packed[4] & 0xff)) >> 0);
    val->_rsvd_vf0_7_f = (uint8_t) (((uint8_t) (packed[5] & 0xf)) << 4) |
        (uint8_t) (((uint8_t) (packed[6] & 0xf0)) >> 4);
    val->cpu_metadata_f = (uint8_t) (((uint8_t) (packed[7] & 0xff)) >> 0);
    val->cpu_queue_num_f = (uint8_t) (((uint8_t) (packed[8] & 0x3f)) >> 0);
}

typedef struct {
 /*  */
    uint16_t _rsvd_vf1_0_f : 10;
    uint8_t proc_sibp_valid_f : 1;
    uint8_t _rsvd_vf1_2_f : 2;
    uint8_t proc_sibp_f : 7;
    uint8_t _rsvd_vf1_4_f : 2;
    uint8_t system_qos_f : 6;
    uint8_t _rsvd_vf1_6_f : 1;
    uint16_t ssp_f : 11;
    uint32_t _rsvd_vf1_8_f : 32;
    uint64_t _rsvd_vf1_9_f : 64;
} ldh_vf1_format_t;

#define LDH_VF1_FORMAT_SIZE 17

static inline void pack_ldh_vf1_format(ldh_vf1_format_t *val, uint8_t *packed)
{
    memset(packed, 0, 17);
    packed[1] |= (val->proc_sibp_valid_f & 0x1) << 5;
    packed[1] |= (val->proc_sibp_f & 0x70) >> 4;
    packed[2] |= (val->proc_sibp_f & 0xf) << 4;
    packed[2] |= (val->system_qos_f & 0x30) >> 4;
    packed[3] |= (val->system_qos_f & 0xf) << 4;
    packed[3] |= (val->ssp_f & 0x700) >> 8;
    packed[4] |= (val->ssp_f & 0xff) << 0;
}

static inline void unpack_ldh_vf1_format(uint8_t *packed, ldh_vf1_format_t *val)
{
    val->proc_sibp_valid_f = (uint8_t) (((uint8_t) (packed[1] & 0x20)) >> 5);
    val->proc_sibp_f = (uint8_t) (((uint8_t) (packed[1] & 0x7)) << 4) |
        (uint8_t) (((uint8_t) (packed[2] & 0xf0)) >> 4);
    val->system_qos_f = (uint8_t) (((uint8_t) (packed[2] & 0x3)) << 4) |
        (uint8_t) (((uint8_t) (packed[3] & 0xf0)) >> 4);
    val->ssp_f = (uint16_t) (((uint16_t) (packed[3] & 0x7)) << 8) |
        (uint16_t) (((uint16_t) (packed[4] & 0xff)) >> 0);
}

typedef struct {
 /*  */
    uint8_t _rsvd_vf2_0_f : 8;
    uint8_t _rsvd_vf2_1_f : 4;
    uint8_t _rsvd_vf2_2_f : 2;
    uint8_t _rsvd_vf2_3_f : 2;
    uint8_t _rsvd_vf2_4_f : 1;
    uint8_t _rsvd_vf2_5_f : 4;
    uint8_t _rsvd_vf2_6_f : 1;
    uint8_t _rsvd_vf2_7_f : 1;
    uint8_t _rsvd_vf2_8_f : 1;
    uint8_t _rsvd_vf2_9_f : 6;
    uint16_t _rsvd_vf2_10_f : 10;
    uint32_t _rsvd_vf2_11_f : 32;
    uint64_t _rsvd_vf2_12_f : 64;
} ldh_vf2_format_t;

#define LDH_VF2_FORMAT_SIZE 17

static inline void pack_ldh_vf2_format(ldh_vf2_format_t *val, uint8_t *packed)
{
    memset(packed, 0, 17);
    packed[0] |= (val->_rsvd_vf2_0_f & 0xff) << 0;
    packed[1] |= (val->_rsvd_vf2_1_f & 0xf) << 4;
    packed[1] |= (val->_rsvd_vf2_2_f & 0x3) << 2;
    packed[1] |= (val->_rsvd_vf2_3_f & 0x3) << 0;
    packed[2] |= (val->_rsvd_vf2_4_f & 0x1) << 7;
    packed[2] |= (val->_rsvd_vf2_5_f & 0xf) << 3;
    packed[2] |= (val->_rsvd_vf2_6_f & 0x1) << 2;
    packed[2] |= (val->_rsvd_vf2_7_f & 0x1) << 1;
    packed[2] |= (val->_rsvd_vf2_8_f & 0x1) << 0;
    packed[3] |= (val->_rsvd_vf2_10_f & 0x300) >> 8;
    packed[4] |= (val->_rsvd_vf2_10_f & 0xff) << 0;
}

static inline void unpack_ldh_vf2_format(uint8_t *packed, ldh_vf2_format_t *val)
{
    val->_rsvd_vf2_0_f = (uint8_t) (((uint8_t) (packed[0] & 0xff)) >> 0);
    val->_rsvd_vf2_1_f = (uint8_t) (((uint8_t) (packed[1] & 0xf0)) >> 4);
    val->_rsvd_vf2_2_f = (uint8_t) (((uint8_t) (packed[1] & 0xc)) >> 2);
    val->_rsvd_vf2_3_f = (uint8_t) (((uint8_t) (packed[1] & 0x3)) >> 0);
    val->_rsvd_vf2_4_f = (uint8_t) (((uint8_t) (packed[2] & 0x80)) >> 7);
    val->_rsvd_vf2_5_f = (uint8_t) (((uint8_t) (packed[2] & 0x78)) >> 3);
    val->_rsvd_vf2_6_f = (uint8_t) (((uint8_t) (packed[2] & 0x4)) >> 2);
    val->_rsvd_vf2_7_f = (uint8_t) (((uint8_t) (packed[2] & 0x2)) >> 1);
    val->_rsvd_vf2_8_f = (uint8_t) (((uint8_t) (packed[2] & 0x1)) >> 0);
    val->_rsvd_vf2_10_f = (uint16_t) (((uint16_t) (packed[3] & 0x3)) << 8) |
        (uint16_t) (((uint16_t) (packed[4] & 0xff)) >> 0);
}

typedef struct {
 /*  */
    uint8_t _rsvd_vf3_0_f : 3;
    uint8_t _rsvd_vf3_1_f : 5;
    uint64_t _rsvd_vf3_2_f : 64;
    uint64_t _rsvd_vf3_3_f : 64;
} ldh_vf3_format_t;

#define LDH_VF3_FORMAT_SIZE 17

static inline void pack_ldh_vf3_format(ldh_vf3_format_t *val, uint8_t *packed)
{
    memset(packed, 0, 17);
    packed[0] |= (val->_rsvd_vf3_0_f & 0x7) << 5;
    packed[0] |= (val->_rsvd_vf3_1_f & 0x1f) << 0;
    packed[1] |= (val->_rsvd_vf3_2_f & 0xff00000000000000) >> 56;
    packed[2] |= (val->_rsvd_vf3_2_f & 0xff000000000000) >> 48;
    packed[3] |= (val->_rsvd_vf3_2_f & 0xff0000000000) >> 40;
    packed[4] |= (val->_rsvd_vf3_2_f & 0xff00000000) >> 32;
    packed[5] |= (val->_rsvd_vf3_2_f & 0xff000000) >> 24;
    packed[6] |= (val->_rsvd_vf3_2_f & 0xff0000) >> 16;
    packed[7] |= (val->_rsvd_vf3_2_f & 0xff00) >> 8;
    packed[8] |= (val->_rsvd_vf3_2_f & 0xff) << 0;
    packed[9] |= (val->_rsvd_vf3_3_f & 0xff00000000000000) >> 56;
    packed[10] |= (val->_rsvd_vf3_3_f & 0xff000000000000) >> 48;
    packed[11] |= (val->_rsvd_vf3_3_f & 0xff0000000000) >> 40;
    packed[12] |= (val->_rsvd_vf3_3_f & 0xff00000000) >> 32;
    packed[13] |= (val->_rsvd_vf3_3_f & 0xff000000) >> 24;
    packed[14] |= (val->_rsvd_vf3_3_f & 0xff0000) >> 16;
    packed[15] |= (val->_rsvd_vf3_3_f & 0xff00) >> 8;
    packed[16] |= (val->_rsvd_vf3_3_f & 0xff) << 0;
}

static inline void unpack_ldh_vf3_format(uint8_t *packed, ldh_vf3_format_t *val)
{
    val->_rsvd_vf3_0_f = (uint8_t) (((uint8_t) (packed[0] & 0xe0)) >> 5);
    val->_rsvd_vf3_1_f = (uint8_t) (((uint8_t) (packed[0] & 0x1f)) >> 0);
    val->_rsvd_vf3_2_f = (((uint64_t) (packed[1] & 0xff)) << 56) |
        (((uint64_t) (packed[2] & 0xff)) << 48) |
        (((uint64_t) (packed[3] & 0xff)) << 40) |
        (((uint64_t) (packed[4] & 0xff)) << 32) |
        (((uint64_t) (packed[5] & 0xff)) << 24) |
        (((uint64_t) (packed[6] & 0xff)) << 16) |
        (((uint64_t) (packed[7] & 0xff)) << 8) |
        (((uint64_t) (packed[8] & 0xff)) >> 0);
    val->_rsvd_vf3_3_f = (((uint64_t) (packed[9] & 0xff)) << 56) |
        (((uint64_t) (packed[10] & 0xff)) << 48) |
        (((uint64_t) (packed[11] & 0xff)) << 40) |
        (((uint64_t) (packed[12] & 0xff)) << 32) |
        (((uint64_t) (packed[13] & 0xff)) << 24) |
        (((uint64_t) (packed[14] & 0xff)) << 16) |
        (((uint64_t) (packed[15] & 0xff)) << 8) |
        (((uint64_t) (packed[16] & 0xff)) >> 0);
}

typedef struct {
    uint8_t _rsvd_fixed_0_f : 1;
    uint8_t _rsvd_fixed_1_f : 1;
    uint8_t _rsvd_fixed_2_f : 1;
    uint8_t _rsvd_fixed_3_f : 4;
    uint8_t proc_type_f : 2;
    uint8_t _rsvd_fixed_5_f : 1;
    uint8_t _rsvd_fixed_6_f : 1;
    uint8_t _rsvd_fixed_7_f : 1;
    uint8_t _rsvd_fixed_8_f : 3;
    uint8_t vf_type_f : 3;
    uint8_t version_f : 4;
    uint8_t _rsvd_fixed_11_f : 2;
    uint16_t _rsvd_vf0_0_f : 10;
    uint16_t slp_f : 10;
    uint8_t _rsvd_vf0_2_f : 2;
    uint8_t system_qos_f : 6;
    uint8_t _rsvd_vf0_4_f : 1;
    uint16_t ssp_f : 11;
    uint8_t _rsvd_vf0_6_f : 4;
    uint8_t _rsvd_vf0_7_f : 8;
    uint8_t _rsvd_vf0_8_f : 4;
    uint8_t cpu_metadata_f : 8;
    uint8_t _rsvd_vf0_10_f : 2;
    uint8_t cpu_queue_num_f : 6;
    uint64_t _rsvd_vf0_12_f : 64;
} ldh_vf0_t;

#define LDH_VF0_SIZE 20

static inline void pack_ldh_vf0(ldh_vf0_t *val, uint8_t *packed)
{
    memset(packed, 0, 20);
    packed[0] |= (val->_rsvd_fixed_0_f & 0x1) << 7;
    packed[0] |= (val->_rsvd_fixed_1_f & 0x1) << 6;
    packed[0] |= (val->_rsvd_fixed_2_f & 0x1) << 5;
    packed[0] |= (val->_rsvd_fixed_3_f & 0xf) << 1;
    packed[0] |= (val->proc_type_f & 0x2) >> 1;
    packed[1] |= (val->proc_type_f & 0x1) << 7;
    packed[1] |= (val->_rsvd_fixed_5_f & 0x1) << 6;
    packed[1] |= (val->_rsvd_fixed_6_f & 0x1) << 5;
    packed[1] |= (val->_rsvd_fixed_7_f & 0x1) << 4;
    packed[1] |= (val->_rsvd_fixed_8_f & 0x7) << 1;
    packed[1] |= (val->vf_type_f & 0x4) >> 2;
    packed[2] |= (val->vf_type_f & 0x3) << 6;
    packed[2] |= (val->version_f & 0xf) << 2;
    packed[2] |= (val->_rsvd_fixed_11_f & 0x3) << 0;
    packed[4] |= (val->slp_f & 0x3f0) >> 4;
    packed[5] |= (val->slp_f & 0xf) << 4;
    packed[5] |= (val->system_qos_f & 0x30) >> 4;
    packed[6] |= (val->system_qos_f & 0xf) << 4;
    packed[6] |= (val->ssp_f & 0x700) >> 8;
    packed[7] |= (val->ssp_f & 0xff) << 0;
    packed[8] |= (val->_rsvd_vf0_7_f & 0xf0) >> 4;
    packed[9] |= (val->_rsvd_vf0_7_f & 0xf) << 4;
    packed[10] |= (val->cpu_metadata_f & 0xff) << 0;
    packed[11] |= (val->cpu_queue_num_f & 0x3f) << 0;
}

static inline void unpack_ldh_vf0(uint8_t *packed, ldh_vf0_t *val)
{
    val->_rsvd_fixed_0_f = (uint8_t) (((uint8_t) (packed[0] & 0x80)) >> 7);
    val->_rsvd_fixed_1_f = (uint8_t) (((uint8_t) (packed[0] & 0x40)) >> 6);
    val->_rsvd_fixed_2_f = (uint8_t) (((uint8_t) (packed[0] & 0x20)) >> 5);
    val->_rsvd_fixed_3_f = (uint8_t) (((uint8_t) (packed[0] & 0x1e)) >> 1);
    val->proc_type_f = (uint8_t) (((uint8_t) (packed[0] & 0x1)) << 1) |
        (uint8_t) (((uint8_t) (packed[1] & 0x80)) >> 7);
    val->_rsvd_fixed_5_f = (uint8_t) (((uint8_t) (packed[1] & 0x40)) >> 6);
    val->_rsvd_fixed_6_f = (uint8_t) (((uint8_t) (packed[1] & 0x20)) >> 5);
    val->_rsvd_fixed_7_f = (uint8_t) (((uint8_t) (packed[1] & 0x10)) >> 4);
    val->_rsvd_fixed_8_f = (uint8_t) (((uint8_t) (packed[1] & 0xe)) >> 1);
    val->vf_type_f = (uint8_t) (((uint8_t) (packed[1] & 0x1)) << 2) |
        (uint8_t) (((uint8_t) (packed[2] & 0xc0)) >> 6);
    val->version_f = (uint8_t) (((uint8_t) (packed[2] & 0x3c)) >> 2);
    val->_rsvd_fixed_11_f = (uint8_t) (((uint8_t) (packed[2] & 0x3)) >> 0);
    val->slp_f = (uint16_t) (((uint16_t) (packed[4] & 0x3f)) << 4) |
        (uint16_t) (((uint16_t) (packed[5] & 0xf0)) >> 4);
    val->system_qos_f = (uint8_t) (((uint8_t) (packed[5] & 0x3)) << 4) |
        (uint8_t) (((uint8_t) (packed[6] & 0xf0)) >> 4);
    val->ssp_f = (uint16_t) (((uint16_t) (packed[6] & 0x7)) << 8) |
        (uint16_t) (((uint16_t) (packed[7] & 0xff)) >> 0);
    val->_rsvd_vf0_7_f = (uint8_t) (((uint8_t) (packed[8] & 0xf)) << 4) |
        (uint8_t) (((uint8_t) (packed[9] & 0xf0)) >> 4);
    val->cpu_metadata_f = (uint8_t) (((uint8_t) (packed[10] & 0xff)) >> 0);
    val->cpu_queue_num_f = (uint8_t) (((uint8_t) (packed[11] & 0x3f)) >> 0);
}

typedef struct {
    uint8_t _rsvd_fixed_0_f : 1;
    uint8_t _rsvd_fixed_1_f : 1;
    uint8_t _rsvd_fixed_2_f : 1;
    uint8_t _rsvd_fixed_3_f : 4;
    uint8_t proc_type_f : 2;
    uint8_t _rsvd_fixed_5_f : 1;
    uint8_t _rsvd_fixed_6_f : 1;
    uint8_t _rsvd_fixed_7_f : 1;
    uint8_t _rsvd_fixed_8_f : 3;
    uint8_t vf_type_f : 3;
    uint8_t version_f : 4;
    uint8_t _rsvd_fixed_11_f : 2;
    uint16_t _rsvd_vf1_0_f : 10;
    uint8_t proc_sibp_valid_f : 1;
    uint8_t _rsvd_vf1_2_f : 2;
    uint8_t proc_sibp_f : 7;
    uint8_t _rsvd_vf1_4_f : 2;
    uint8_t system_qos_f : 6;
    uint8_t _rsvd_vf1_6_f : 1;
    uint16_t ssp_f : 11;
    uint32_t _rsvd_vf1_8_f : 32;
    uint64_t _rsvd_vf1_9_f : 64;
} ldh_vf1_t;

#define LDH_VF1_SIZE 20

static inline void pack_ldh_vf1(ldh_vf1_t *val, uint8_t *packed)
{
    memset(packed, 0, 20);
    packed[0] |= (val->_rsvd_fixed_0_f & 0x1) << 7;
    packed[0] |= (val->_rsvd_fixed_1_f & 0x1) << 6;
    packed[0] |= (val->_rsvd_fixed_2_f & 0x1) << 5;
    packed[0] |= (val->_rsvd_fixed_3_f & 0xf) << 1;
    packed[0] |= (val->proc_type_f & 0x2) >> 1;
    packed[1] |= (val->proc_type_f & 0x1) << 7;
    packed[1] |= (val->_rsvd_fixed_5_f & 0x1) << 6;
    packed[1] |= (val->_rsvd_fixed_6_f & 0x1) << 5;
    packed[1] |= (val->_rsvd_fixed_7_f & 0x1) << 4;
    packed[1] |= (val->_rsvd_fixed_8_f & 0x7) << 1;
    packed[1] |= (val->vf_type_f & 0x4) >> 2;
    packed[2] |= (val->vf_type_f & 0x3) << 6;
    packed[2] |= (val->version_f & 0xf) << 2;
    packed[2] |= (val->_rsvd_fixed_11_f & 0x3) << 0;
    packed[4] |= (val->proc_sibp_valid_f & 0x1) << 5;
    packed[4] |= (val->proc_sibp_f & 0x70) >> 4;
    packed[5] |= (val->proc_sibp_f & 0xf) << 4;
    packed[5] |= (val->system_qos_f & 0x30) >> 4;
    packed[6] |= (val->system_qos_f & 0xf) << 4;
    packed[6] |= (val->ssp_f & 0x700) >> 8;
    packed[7] |= (val->ssp_f & 0xff) << 0;
}

static inline void unpack_ldh_vf1(uint8_t *packed, ldh_vf1_t *val)
{
    val->_rsvd_fixed_0_f = (uint8_t) (((uint8_t) (packed[0] & 0x80)) >> 7);
    val->_rsvd_fixed_1_f = (uint8_t) (((uint8_t) (packed[0] & 0x40)) >> 6);
    val->_rsvd_fixed_2_f = (uint8_t) (((uint8_t) (packed[0] & 0x20)) >> 5);
    val->_rsvd_fixed_3_f = (uint8_t) (((uint8_t) (packed[0] & 0x1e)) >> 1);
    val->proc_type_f = (uint8_t) (((uint8_t) (packed[0] & 0x1)) << 1) |
        (uint8_t) (((uint8_t) (packed[1] & 0x80)) >> 7);
    val->_rsvd_fixed_5_f = (uint8_t) (((uint8_t) (packed[1] & 0x40)) >> 6);
    val->_rsvd_fixed_6_f = (uint8_t) (((uint8_t) (packed[1] & 0x20)) >> 5);
    val->_rsvd_fixed_7_f = (uint8_t) (((uint8_t) (packed[1] & 0x10)) >> 4);
    val->_rsvd_fixed_8_f = (uint8_t) (((uint8_t) (packed[1] & 0xe)) >> 1);
    val->vf_type_f = (uint8_t) (((uint8_t) (packed[1] & 0x1)) << 2) |
        (uint8_t) (((uint8_t) (packed[2] & 0xc0)) >> 6);
    val->version_f = (uint8_t) (((uint8_t) (packed[2] & 0x3c)) >> 2);
    val->_rsvd_fixed_11_f = (uint8_t) (((uint8_t) (packed[2] & 0x3)) >> 0);
    val->proc_sibp_valid_f = (uint8_t) (((uint8_t) (packed[4] & 0x20)) >> 5);
    val->proc_sibp_f = (uint8_t) (((uint8_t) (packed[4] & 0x7)) << 4) |
        (uint8_t) (((uint8_t) (packed[5] & 0xf0)) >> 4);
    val->system_qos_f = (uint8_t) (((uint8_t) (packed[5] & 0x3)) << 4) |
        (uint8_t) (((uint8_t) (packed[6] & 0xf0)) >> 4);
    val->ssp_f = (uint16_t) (((uint16_t) (packed[6] & 0x7)) << 8) |
        (uint16_t) (((uint16_t) (packed[7] & 0xff)) >> 0);
}

typedef struct {
    uint8_t _rsvd_fixed_0_f : 1;
    uint8_t _rsvd_fixed_1_f : 1;
    uint8_t _rsvd_fixed_2_f : 1;
    uint8_t _rsvd_fixed_3_f : 4;
    uint8_t proc_type_f : 2;
    uint8_t _rsvd_fixed_5_f : 1;
    uint8_t _rsvd_fixed_6_f : 1;
    uint8_t _rsvd_fixed_7_f : 1;
    uint8_t _rsvd_fixed_8_f : 3;
    uint8_t vf_type_f : 3;
    uint8_t version_f : 4;
    uint8_t _rsvd_fixed_11_f : 2;
    uint8_t _rsvd_vf2_0_f : 8;
    uint8_t _rsvd_vf2_1_f : 4;
    uint8_t _rsvd_vf2_2_f : 2;
    uint8_t _rsvd_vf2_3_f : 2;
    uint8_t _rsvd_vf2_4_f : 1;
    uint8_t _rsvd_vf2_5_f : 4;
    uint8_t _rsvd_vf2_6_f : 1;
    uint8_t _rsvd_vf2_7_f : 1;
    uint8_t _rsvd_vf2_8_f : 1;
    uint8_t _rsvd_vf2_9_f : 6;
    uint16_t _rsvd_vf2_10_f : 10;
    uint32_t _rsvd_vf2_11_f : 32;
    uint64_t _rsvd_vf2_12_f : 64;
} ldh_vf2_t;

#define LDH_VF2_SIZE 20

static inline void pack_ldh_vf2(ldh_vf2_t *val, uint8_t *packed)
{
    memset(packed, 0, 20);
    packed[0] |= (val->_rsvd_fixed_0_f & 0x1) << 7;
    packed[0] |= (val->_rsvd_fixed_1_f & 0x1) << 6;
    packed[0] |= (val->_rsvd_fixed_2_f & 0x1) << 5;
    packed[0] |= (val->_rsvd_fixed_3_f & 0xf) << 1;
    packed[0] |= (val->proc_type_f & 0x2) >> 1;
    packed[1] |= (val->proc_type_f & 0x1) << 7;
    packed[1] |= (val->_rsvd_fixed_5_f & 0x1) << 6;
    packed[1] |= (val->_rsvd_fixed_6_f & 0x1) << 5;
    packed[1] |= (val->_rsvd_fixed_7_f & 0x1) << 4;
    packed[1] |= (val->_rsvd_fixed_8_f & 0x7) << 1;
    packed[1] |= (val->vf_type_f & 0x4) >> 2;
    packed[2] |= (val->vf_type_f & 0x3) << 6;
    packed[2] |= (val->version_f & 0xf) << 2;
    packed[2] |= (val->_rsvd_fixed_11_f & 0x3) << 0;
    packed[3] |= (val->_rsvd_vf2_0_f & 0xff) << 0;
    packed[4] |= (val->_rsvd_vf2_1_f & 0xf) << 4;
    packed[4] |= (val->_rsvd_vf2_2_f & 0x3) << 2;
    packed[4] |= (val->_rsvd_vf2_3_f & 0x3) << 0;
    packed[5] |= (val->_rsvd_vf2_4_f & 0x1) << 7;
    packed[5] |= (val->_rsvd_vf2_5_f & 0xf) << 3;
    packed[5] |= (val->_rsvd_vf2_6_f & 0x1) << 2;
    packed[5] |= (val->_rsvd_vf2_7_f & 0x1) << 1;
    packed[5] |= (val->_rsvd_vf2_8_f & 0x1) << 0;
    packed[6] |= (val->_rsvd_vf2_10_f & 0x300) >> 8;
    packed[7] |= (val->_rsvd_vf2_10_f & 0xff) << 0;
}

static inline void unpack_ldh_vf2(uint8_t *packed, ldh_vf2_t *val)
{
    val->_rsvd_fixed_0_f = (uint8_t) (((uint8_t) (packed[0] & 0x80)) >> 7);
    val->_rsvd_fixed_1_f = (uint8_t) (((uint8_t) (packed[0] & 0x40)) >> 6);
    val->_rsvd_fixed_2_f = (uint8_t) (((uint8_t) (packed[0] & 0x20)) >> 5);
    val->_rsvd_fixed_3_f = (uint8_t) (((uint8_t) (packed[0] & 0x1e)) >> 1);
    val->proc_type_f = (uint8_t) (((uint8_t) (packed[0] & 0x1)) << 1) |
        (uint8_t) (((uint8_t) (packed[1] & 0x80)) >> 7);
    val->_rsvd_fixed_5_f = (uint8_t) (((uint8_t) (packed[1] & 0x40)) >> 6);
    val->_rsvd_fixed_6_f = (uint8_t) (((uint8_t) (packed[1] & 0x20)) >> 5);
    val->_rsvd_fixed_7_f = (uint8_t) (((uint8_t) (packed[1] & 0x10)) >> 4);
    val->_rsvd_fixed_8_f = (uint8_t) (((uint8_t) (packed[1] & 0xe)) >> 1);
    val->vf_type_f = (uint8_t) (((uint8_t) (packed[1] & 0x1)) << 2) |
        (uint8_t) (((uint8_t) (packed[2] & 0xc0)) >> 6);
    val->version_f = (uint8_t) (((uint8_t) (packed[2] & 0x3c)) >> 2);
    val->_rsvd_fixed_11_f = (uint8_t) (((uint8_t) (packed[2] & 0x3)) >> 0);
    val->_rsvd_vf2_0_f = (uint8_t) (((uint8_t) (packed[3] & 0xff)) >> 0);
    val->_rsvd_vf2_1_f = (uint8_t) (((uint8_t) (packed[4] & 0xf0)) >> 4);
    val->_rsvd_vf2_2_f = (uint8_t) (((uint8_t) (packed[4] & 0xc)) >> 2);
    val->_rsvd_vf2_3_f = (uint8_t) (((uint8_t) (packed[4] & 0x3)) >> 0);
    val->_rsvd_vf2_4_f = (uint8_t) (((uint8_t) (packed[5] & 0x80)) >> 7);
    val->_rsvd_vf2_5_f = (uint8_t) (((uint8_t) (packed[5] & 0x78)) >> 3);
    val->_rsvd_vf2_6_f = (uint8_t) (((uint8_t) (packed[5] & 0x4)) >> 2);
    val->_rsvd_vf2_7_f = (uint8_t) (((uint8_t) (packed[5] & 0x2)) >> 1);
    val->_rsvd_vf2_8_f = (uint8_t) (((uint8_t) (packed[5] & 0x1)) >> 0);
    val->_rsvd_vf2_10_f = (uint16_t) (((uint16_t) (packed[6] & 0x3)) << 8) |
        (uint16_t) (((uint16_t) (packed[7] & 0xff)) >> 0);
}

typedef struct {
    uint8_t _rsvd_fixed_0_f : 1;
    uint8_t _rsvd_fixed_1_f : 1;
    uint8_t _rsvd_fixed_2_f : 1;
    uint8_t _rsvd_fixed_3_f : 4;
    uint8_t proc_type_f : 2;
    uint8_t _rsvd_fixed_5_f : 1;
    uint8_t _rsvd_fixed_6_f : 1;
    uint8_t _rsvd_fixed_7_f : 1;
    uint8_t _rsvd_fixed_8_f : 3;
    uint8_t vf_type_f : 3;
    uint8_t version_f : 4;
    uint8_t _rsvd_fixed_11_f : 2;
    uint8_t _rsvd_vf3_0_f : 3;
    uint8_t _rsvd_vf3_1_f : 5;
    uint64_t _rsvd_vf3_2_f : 64;
    uint64_t _rsvd_vf3_3_f : 64;
} ldh_vf3_t;

#define LDH_VF3_SIZE 20

static inline void pack_ldh_vf3(ldh_vf3_t *val, uint8_t *packed)
{
    memset(packed, 0, 20);
    packed[0] |= (val->_rsvd_fixed_0_f & 0x1) << 7;
    packed[0] |= (val->_rsvd_fixed_1_f & 0x1) << 6;
    packed[0] |= (val->_rsvd_fixed_2_f & 0x1) << 5;
    packed[0] |= (val->_rsvd_fixed_3_f & 0xf) << 1;
    packed[0] |= (val->proc_type_f & 0x2) >> 1;
    packed[1] |= (val->proc_type_f & 0x1) << 7;
    packed[1] |= (val->_rsvd_fixed_5_f & 0x1) << 6;
    packed[1] |= (val->_rsvd_fixed_6_f & 0x1) << 5;
    packed[1] |= (val->_rsvd_fixed_7_f & 0x1) << 4;
    packed[1] |= (val->_rsvd_fixed_8_f & 0x7) << 1;
    packed[1] |= (val->vf_type_f & 0x4) >> 2;
    packed[2] |= (val->vf_type_f & 0x3) << 6;
    packed[2] |= (val->version_f & 0xf) << 2;
    packed[2] |= (val->_rsvd_fixed_11_f & 0x3) << 0;
    packed[3] |= (val->_rsvd_vf3_0_f & 0x7) << 5;
    packed[3] |= (val->_rsvd_vf3_1_f & 0x1f) << 0;
    packed[4] |= (val->_rsvd_vf3_2_f & 0xff00000000000000) >> 56;
    packed[5] |= (val->_rsvd_vf3_2_f & 0xff000000000000) >> 48;
    packed[6] |= (val->_rsvd_vf3_2_f & 0xff0000000000) >> 40;
    packed[7] |= (val->_rsvd_vf3_2_f & 0xff00000000) >> 32;
    packed[8] |= (val->_rsvd_vf3_2_f & 0xff000000) >> 24;
    packed[9] |= (val->_rsvd_vf3_2_f & 0xff0000) >> 16;
    packed[10] |= (val->_rsvd_vf3_2_f & 0xff00) >> 8;
    packed[11] |= (val->_rsvd_vf3_2_f & 0xff) << 0;
    packed[12] |= (val->_rsvd_vf3_3_f & 0xff00000000000000) >> 56;
    packed[13] |= (val->_rsvd_vf3_3_f & 0xff000000000000) >> 48;
    packed[14] |= (val->_rsvd_vf3_3_f & 0xff0000000000) >> 40;
    packed[15] |= (val->_rsvd_vf3_3_f & 0xff00000000) >> 32;
    packed[16] |= (val->_rsvd_vf3_3_f & 0xff000000) >> 24;
    packed[17] |= (val->_rsvd_vf3_3_f & 0xff0000) >> 16;
    packed[18] |= (val->_rsvd_vf3_3_f & 0xff00) >> 8;
    packed[19] |= (val->_rsvd_vf3_3_f & 0xff) << 0;
}

static inline void unpack_ldh_vf3(uint8_t *packed, ldh_vf3_t *val)
{
    val->_rsvd_fixed_0_f = (uint8_t) (((uint8_t) (packed[0] & 0x80)) >> 7);
    val->_rsvd_fixed_1_f = (uint8_t) (((uint8_t) (packed[0] & 0x40)) >> 6);
    val->_rsvd_fixed_2_f = (uint8_t) (((uint8_t) (packed[0] & 0x20)) >> 5);
    val->_rsvd_fixed_3_f = (uint8_t) (((uint8_t) (packed[0] & 0x1e)) >> 1);
    val->proc_type_f = (uint8_t) (((uint8_t) (packed[0] & 0x1)) << 1) |
        (uint8_t) (((uint8_t) (packed[1] & 0x80)) >> 7);
    val->_rsvd_fixed_5_f = (uint8_t) (((uint8_t) (packed[1] & 0x40)) >> 6);
    val->_rsvd_fixed_6_f = (uint8_t) (((uint8_t) (packed[1] & 0x20)) >> 5);
    val->_rsvd_fixed_7_f = (uint8_t) (((uint8_t) (packed[1] & 0x10)) >> 4);
    val->_rsvd_fixed_8_f = (uint8_t) (((uint8_t) (packed[1] & 0xe)) >> 1);
    val->vf_type_f = (uint8_t) (((uint8_t) (packed[1] & 0x1)) << 2) |
        (uint8_t) (((uint8_t) (packed[2] & 0xc0)) >> 6);
    val->version_f = (uint8_t) (((uint8_t) (packed[2] & 0x3c)) >> 2);
    val->_rsvd_fixed_11_f = (uint8_t) (((uint8_t) (packed[2] & 0x3)) >> 0);
    val->_rsvd_vf3_0_f = (uint8_t) (((uint8_t) (packed[3] & 0xe0)) >> 5);
    val->_rsvd_vf3_1_f = (uint8_t) (((uint8_t) (packed[3] & 0x1f)) >> 0);
    val->_rsvd_vf3_2_f = (((uint64_t) (packed[4] & 0xff)) << 56) |
        (((uint64_t) (packed[5] & 0xff)) << 48) |
        (((uint64_t) (packed[6] & 0xff)) << 40) |
        (((uint64_t) (packed[7] & 0xff)) << 32) |
        (((uint64_t) (packed[8] & 0xff)) << 24) |
        (((uint64_t) (packed[9] & 0xff)) << 16) |
        (((uint64_t) (packed[10] & 0xff)) << 8) |
        (((uint64_t) (packed[11] & 0xff)) >> 0);
    val->_rsvd_vf3_3_f = (((uint64_t) (packed[12] & 0xff)) << 56) |
        (((uint64_t) (packed[13] & 0xff)) << 48) |
        (((uint64_t) (packed[14] & 0xff)) << 40) |
        (((uint64_t) (packed[15] & 0xff)) << 32) |
        (((uint64_t) (packed[16] & 0xff)) << 24) |
        (((uint64_t) (packed[17] & 0xff)) << 16) |
        (((uint64_t) (packed[18] & 0xff)) << 8) |
        (((uint64_t) (packed[19] & 0xff)) >> 0);
}


#endif /* __LDH_STRUCTS_IPD_V2_H__ */
