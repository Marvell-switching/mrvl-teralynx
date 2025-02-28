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

#ifndef _INNO_ENET_H_
#define _INNO_ENET_H_

#include <linux/types.h>
#include <linux/kernel.h>
#include <net/genetlink.h>
#include "inno_common_def.h"

/* Base MTU without CPU headers */
#define DEFAULT_INNO_MAX_MTU_SIZE 9456
/* MTU with CPU headers */
#define DEFAULT_INNO_CPU_MTU_SIZE (DEFAULT_INNO_MAX_MTU_SIZE + CHIP_HEADER_SIZE)

/* Depth of TX/RX descriptor */
#define INNO_TX_COUNT    256
#define INNO_RX_COUNT    256

#define INNO_RX_INTR     (1 << 0)
#define INNO_TX_INTR     (1 << 1)
#define INNO_MEM_INTR    (1 << 2)

#define NETDEV_INFO_HEADER_SIZE sizeof(inno_info_header_t)

/* LDH + Debug header */
#define TL_DBG_HDR_SIZE     32

/* Enums for SFLOW netlink metadata */
enum {
    INNO_SFLOW_GENL_ATTR_IIFINDEX,
    INNO_SFLOW_GENL_ATTR_OIFINDEX,
    INNO_SFLOW_GENL_ATTR_ORIGSIZE,
    INNO_SFLOW_GENL_ATTR_SAMPLE_GROUP,
    INNO_SFLOW_GENL_ATTR_GROUP_SEQ,
    INNO_SFLOW_GENL_ATTR_SAMPLE_RATE,
    INNO_SFLOW_GENL_ATTR_DATA,
    INNO_SFLOW_GENL_ATTR_MAX
};

/* Enums for pgenl netlink metadata */
enum {
  INNO_PGENL_ATTR_SOURCE_IFINDEX,
  INNO_PGENL_ATTR_DEST_IFINDEX,
  INNO_PGENL_ATTR_CONTEXT,
  INNO_PGENL_ATTR_PAYLOAD,
  INNO_PGENL_ATTR_MAX
};

typedef enum {
    PIPELINE_LOOKUP = 1,
    PIPELINE_BYPASS = 2,
    PIPELINE_PTP    = 3
}syshdr_tx_type;

typedef enum {
    VLAN_TAG_ORIGINAL=0,
    VLAN_TAG_STRIP,
    VLAN_TAG_KEEP
}vlan_tag_action;

typedef enum {
    INNO_INTF_DOWN,
    INNO_INTF_UP,
}inno_intf_state;

typedef enum {
    INNO_LINK_DOWN,
    INNO_LINK_UP
}inno_link_state;

typedef struct inno_enet_adapter {
    uint16_t      sysport;          /** Associated netdev num (0-255) */
    inno_device_t *idev;            /** Associated idevice */
} inno_enet_adapter_t;

int
inno_napi_deinit(inno_device_t  *idev);

int
inno_napi_init(inno_device_t  *idev);

int
inno_netdev_create(inno_device_t *idev,
                   inno_ioctl_netdev_t *ioctl);

int
inno_netdev_delete(inno_device_t *idev,
                   uint16_t      sysport);

int
inno_netdev_get(inno_device_t *idev,
                inno_ioctl_netdev_info_t *ninfo);

int
inno_netdev_set(inno_device_t *idev,
                inno_ioctl_netdev_info_t ninfo);

int
inno_netlink_create(inno_device_t  *idev,
                    inno_ioctl_netlink_t *ioctl);

int
inno_netlink_delete(inno_device_t  *idev,
                    inno_ioctl_netlink_t *ioctl);

void
inno_netlink_deinit(inno_device_t  *idev);
#define UNUSED(x)    UNUSED_ ## x __attribute__((unused))

int
inno_netlink_set(inno_device_t *idev,
                 inno_ioctl_netlink_info_t ioctl);

void
inno_populate_inno_hdr_v0(inno_ldh_t         *ldh_hdr,
                          inno_info_header_t *ih,
                          inno_dma_alloc_t  *dma,
                          uint8_t           ext_hdr_type,
                          uint8_t           ext_hdrs_size);
int inno_unpack_ldh_header_v0(uint8_t *buf,
                              inno_ldh_t *ldh_hdr);
bool inno_debug_hdr_present_v0(inno_ldh_t *ldh_hdr);
int
inno_vf2_queue_set_v0(uint8_t *buf,
                      uint8_t queue);
uint32_t inno_get_ssp_v0(inno_ldh_t *ldh_hdr);
int
inno_unpack_ext_hdrs_v0(inno_device_t *idev,
                        inno_dma_alloc_t *dma,
                        inno_ldh_t *ldh_hdr,
                        uint32_t *ext_hdrs_size,
                        uint8_t  *ext_info_hdrs,
                        uint32_t *ext_info_hdrs_size,
                        uint8_t  *ext_hdr_type,
                        uint32_t *sp);

void
inno_populate_inno_hdr_v1(inno_ldh_t         *ldh_hdr,
                          inno_info_header_t *ih,
                          inno_dma_alloc_t  *dma,
                          uint8_t           ext_hdr_type,
                          uint8_t           ext_hdrs_size);
int inno_unpack_ldh_header_v1(uint8_t *buf,
                              inno_ldh_t *ldh_hdr);
bool inno_debug_hdr_present_v1(inno_ldh_t *ldh_hdr);
uint32_t inno_get_ssp_v1(inno_ldh_t *ldh_hdr);
int
inno_unpack_ext_hdrs_v1(inno_device_t *idev,
                        inno_dma_alloc_t *dma,
                        inno_ldh_t *ldh_hdr,
                        uint32_t *ext_hdrs_size,
                        uint8_t  *ext_info_hdrs,
                        uint32_t *ext_info_hdrs_size,
                        uint8_t  *ext_hdr_type,
                        uint32_t *sp);

/* TL10 header handlers */
void inno_populate_inno_hdr_v2(inno_ldh_t         *ldh_hdr,
                               inno_info_header_t *ih,
                               inno_dma_alloc_t  *dma,
                               uint8_t           ext_hdr_type,
                               uint8_t           ext_hdrs_size);
int inno_unpack_ldh_header_v2(uint8_t *buf,
                              inno_ldh_t *ldh_hdr);
bool inno_debug_hdr_present_v2(inno_ldh_t *ldh_hdr);
int
inno_vf2_queue_set_v2(uint8_t *buf,
                      uint8_t queue);
uint32_t inno_get_ssp_v2(inno_ldh_t *ldh_hdr);
int inno_unpack_ext_hdrs_v2(inno_device_t *idev,
                            inno_dma_alloc_t *dma,
                            inno_ldh_t *ldh_hdr,
                            uint32_t *ext_hdrs_size,
                            uint8_t  *ext_info_hdrs,
                            uint32_t *ext_info_hdrs_size,
                            uint8_t  *ext_hdr_type,
                            uint32_t *sp);

#endif /* _INNO_ENET_H_ */
