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
on the worldwide web at http:

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
*******************************************************************************/



#ifndef __PCI_COMMON_IPD_H__
#define __PCI_COMMON_IPD_H__

#ifndef __KERNEL__
#include <stdint.h>
#include <assert.h>
#else
#include <linux/types.h>
#endif
#ifndef PCI_STRUCT_FIELD_ORDER_LO_HI
#define PCI_STRUCT_FIELD_ORDER_LO_HI (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#endif


typedef enum node__pci_reg_drv_ {
    PCI_REG_INVALID                                              = -1,
    AXI_TIMEOUT_CFG                                              = 0x00000038,
    CPU_TO_SWITCHIF_SRC                                          = 0x00004a90,
    BLK_AND_MSG_RXE                                              = 0x00004034,
    CNFG_REF_AMOUNT_RXE                                          = 0x00004058,
    CNFG_REF_AMOUNT_TXE                                          = 0x00004054,
    CPU_PTP_CONTROL                                              = 0x0001383c,
    CPU_PTP_DEVICETIMER_SNAPSHOT_31_0                            = 0x00013848,
    CPU_PTP_DEVICETIMER_SNAPSHOT_47_32                           = 0x0001384c,
    CPU_PTP_SYNCHRONIZEDTIME_SNAPSHOT_NS_31_0                    = 0x00013898,
    CPU_PTP_SYNCHRONIZEDTIME_SNAPSHOT_SEC_31_0                   = 0x00013890,
    CPU_PTP_SYNCHRONIZEDTIME_SNAPSHOT_SEC_47_32                  = 0x00013894,
    DESC_CIDX_WRBACK                                             = 0x0000401c,
    DESC_READ                                                    = 0x00004018,
    DESC_WRBACK                                                  = 0x00004014,
    DISABLE_STS_DONE                                             = 0x00004028,
    DM_STS                                                       = 0x00004020,
    DMA_GLOBAL                                                   = 0x00004000,
    DMA_INTF                                                     = 0x00004004,
    DMA_RXE_SWITCH_TO_CPU_QUEUE_OFFSET                           = 0x00004a04,
    ENDIAN_SWAP_DIS_IAC_AXIM                                     = 0x000137ec,
    ENDIAN_SWAP_DIS_IAC_AXIS                                     = 0x000137e0,
    ENDIAN_SWAP_DIS_IAC_CNTG                                     = 0x000137d4,
    ENDIAN_SWAP_DIS_MSI_WB                                       = 0x000137f8,
    ENDIAN_SWAP_DIS_PCIE_MTR                                     = 0x00013810,
    ENDIAN_SWAP_DIS_PCIE_TAR                                     = 0x00013804,
    ENDIAN_SWAP_ENA_IAC_AXIM                                     = 0x000137e8,
    ENDIAN_SWAP_ENA_IAC_AXIS                                     = 0x000137dc,
    ENDIAN_SWAP_ENA_IAC_CNTG                                     = 0x000137d0,
    ENDIAN_SWAP_ENA_MSI_WB                                       = 0x000137f4,
    ENDIAN_SWAP_ENA_PCIE_TAR                                     = 0x00013800,
    ENDIAN_SWAP_STS_PCIE_TAR                                     = 0x00013808,
    HI_WATERMARK_IMSG_0                                          = 0x000048c0,
    HI_WATERMARK_IMSG_1                                          = 0x000048c4,
    HI_WATERMARK_IMSG_2                                          = 0x000048c8,
    HI_WATERMARK_IMSG_3                                          = 0x000048cc,
    HI_WATERMARK_IMSG_4                                          = 0x000048d0,
    HI_WATERMARK_IMSG_5                                          = 0x000048d4,
    HI_WATERMARK_IMSG_6                                          = 0x00004fb0,
    HI_WATERMARK_IMSG_7                                          = 0x00004fb4,
    HOST0_SWITCH_TO_CPU_CLEAN_PKT_SIZE_CNTR                      = 0x00004494,
    HRR_ADR_HI                                                   = 0x00000014,
    HRR_ADR_LO                                                   = 0x00000010,
    HRR_PIDX                                                     = 0x0000001c,
    HRR_PIDX_ADR_HI                                              = 0x00000028,
    HRR_PIDX_ADR_LO                                              = 0x00000024,
    HRR_SIZE                                                     = 0x00000018,
    HSR_ADR_HI                                                   = 0x00000030,
    HSR_ADR_LO                                                   = 0x0000002c,
    IAC_MEM_INIT                                                 = 0x00001400,
    INTR_IAC_1_CAUSE                                             = 0x00001be4,
    INTR_IAC_1_PCI_MASK                                          = 0x00001bec,
    INTR_IAC_CAUSE                                               = 0x00001430,
    INTR_IAC_PCI_MASK                                            = 0x00001438,
    INTR_IMMR                                                    = 0x00018400,
    INTR_IMMR_1                                                  = 0x0001a800,
    INTR_INCR                                                    = 0x00018240,
    INTR_INCR_1                                                  = 0x0001a040,
    INTR_INMC                                                    = 0x000182a0,
    INTR_INMC_1                                                  = 0x0001a1c0,
    INTR_INMS                                                    = 0x00018280,
    INTR_INMS_1                                                  = 0x0001a140,
    INTR_PSC_AHI                                                 = 0x00018f4c,
    INTR_PSC_ALO                                                 = 0x00018f48,
    INTR_PSC_TMR                                                 = 0x00018f50,
    INTR_WB_AHI                                                  = 0x00018304,
    INTR_WB_ALO                                                  = 0x00018300,
    INTR_WB_CTL                                                  = 0x00018308,
    ISN_TIMEOUT_CFG                                              = 0x00000034,
    JTAG_IDCODE_O                                                = 0x0001113c,
    LEARN_MSG                                                    = 0x000048bc,
    LEARN_MSG_1                                                  = 0x00004fac,
    MCU_MEM_INIT                                                 = 0x00013318,
    PCI_AXI_LINK_DOWN_INDICATOR_BIT_L0                           = 0x0000a824,
    RXE_DMA_MSG_DATA_BASE_HI_0                                   = 0x00004840,
    RXE_DMA_MSG_DATA_BASE_LO_0                                   = 0x0000483c,
    RXE_DMA_MSG_DATA_CIDX_0                                      = 0x0000485c,
    RXE_DMA_MSG_DATA_PIDX_0                                      = 0x00004858,
    RXE_DMA_MSG_DESC_BASE_HI_0                                   = 0x00004848,
    RXE_DMA_MSG_DESC_BASE_LO_0                                   = 0x00004844,
    RXE_DMA_MSG_DESC_CIDX_0                                      = 0x00004864,
    RXE_DMA_MSG_DESC_PIDX_0                                      = 0x00004860,
    RXE_DMA_MSG_PIDX_WB_BASE_HI_0                                = 0x00004850,
    RXE_DMA_MSG_PIDX_WB_BASE_LO_0                                = 0x0000484c,
    RXE_DMA_MSG_SIZE_0                                           = 0x00004854,
    RXE_DMA_MSG_STATUS_0                                         = 0x00004880,
    RXE_IMSG                                                     = 0x0000400c,
    RXE_IMSG_WRBACK                                              = 0x00004010,
    RXE_TOTAL_PKT_SIZE_CNTR                                      = 0x00004564,
    RXQ                                                          = 0x00004030,
    RXQ_0                                                        = 0x000046a0,
    RXQ_0_BASE_HSN_HI                                            = 0x00004670,
    RXQ_0_BASE_HSN_HI_WB                                         = 0x00004678,
    RXQ_0_BASE_HSN_LO                                            = 0x0000466c,
    RXQ_0_BASE_HSN_LO_WB                                         = 0x00004674,
    RXQ_0_CACHE                                                  = 0x00004688,
    RXQ_0_CIDX_UPDATE                                            = 0x0000468c,
    RXQ_0_CIDX_UPDATE_CLIFF                                      = 0x00004690,
    RXQ_0_CIDX_UPDATE_PRECLIFF                                   = 0x00004698,
    RXQ_0_CIDX_UPDATE_PRESCALER                                  = 0x00004694,
    RXQ_0_CIDX_UPDATE_TMR_CTL                                    = 0x0000469c,
    RXQ_0_CIDX_WB_HSN_HI                                         = 0x00004680,
    RXQ_0_CIDX_WB_HSN_LO                                         = 0x0000467c,
    RXQ_0_DESC_CIDX                                              = 0x000040dc,
    RXQ_0_DESC_PIDX                                              = 0x000040e0,
    RXQ_0_DESC_RING                                              = 0x00004684,
    RXQ_0_SCH                                                    = 0x000046a4,
    RXQ_1                                                        = 0x000046e4,
    RXQ_1_BASE_HSN_HI                                            = 0x000046b4,
    RXQ_1_BASE_HSN_HI_WB                                         = 0x000046bc,
    RXQ_1_BASE_HSN_LO                                            = 0x000046b0,
    RXQ_1_BASE_HSN_LO_WB                                         = 0x000046b8,
    RXQ_1_CACHE                                                  = 0x000046cc,
    RXQ_1_CIDX_UPDATE                                            = 0x000046d0,
    RXQ_1_CIDX_UPDATE_CLIFF                                      = 0x000046d4,
    RXQ_1_CIDX_UPDATE_PRECLIFF                                   = 0x000046dc,
    RXQ_1_CIDX_UPDATE_PRESCALER                                  = 0x000046d8,
    RXQ_1_CIDX_UPDATE_TMR_CTL                                    = 0x000046e0,
    RXQ_1_CIDX_WB_HSN_HI                                         = 0x000046c4,
    RXQ_1_CIDX_WB_HSN_LO                                         = 0x000046c0,
    RXQ_1_DESC_CIDX                                              = 0x000040e4,
    RXQ_1_DESC_PIDX                                              = 0x000040e8,
    RXQ_1_DESC_RING                                              = 0x000046c8,
    RXQ_1_SCH                                                    = 0x000046e8,
    RXQ_2                                                        = 0x00004728,
    RXQ_2_BASE_HSN_HI                                            = 0x000046f8,
    RXQ_2_BASE_HSN_HI_WB                                         = 0x00004700,
    RXQ_2_BASE_HSN_LO                                            = 0x000046f4,
    RXQ_2_BASE_HSN_LO_WB                                         = 0x000046fc,
    RXQ_2_CACHE                                                  = 0x00004710,
    RXQ_2_CIDX_UPDATE                                            = 0x00004714,
    RXQ_2_CIDX_UPDATE_CLIFF                                      = 0x00004718,
    RXQ_2_CIDX_UPDATE_PRECLIFF                                   = 0x00004720,
    RXQ_2_CIDX_UPDATE_PRESCALER                                  = 0x0000471c,
    RXQ_2_CIDX_UPDATE_TMR_CTL                                    = 0x00004724,
    RXQ_2_CIDX_WB_HSN_HI                                         = 0x00004708,
    RXQ_2_CIDX_WB_HSN_LO                                         = 0x00004704,
    RXQ_2_DESC_CIDX                                              = 0x000040ec,
    RXQ_2_DESC_PIDX                                              = 0x000040f0,
    RXQ_2_DESC_RING                                              = 0x0000470c,
    RXQ_2_SCH                                                    = 0x0000472c,
    RXQ_3                                                        = 0x0000476c,
    RXQ_3_BASE_HSN_HI                                            = 0x0000473c,
    RXQ_3_BASE_HSN_HI_WB                                         = 0x00004744,
    RXQ_3_BASE_HSN_LO                                            = 0x00004738,
    RXQ_3_BASE_HSN_LO_WB                                         = 0x00004740,
    RXQ_3_CACHE                                                  = 0x00004754,
    RXQ_3_CIDX_UPDATE                                            = 0x00004758,
    RXQ_3_CIDX_UPDATE_CLIFF                                      = 0x0000475c,
    RXQ_3_CIDX_UPDATE_PRECLIFF                                   = 0x00004764,
    RXQ_3_CIDX_UPDATE_PRESCALER                                  = 0x00004760,
    RXQ_3_CIDX_UPDATE_TMR_CTL                                    = 0x00004768,
    RXQ_3_CIDX_WB_HSN_HI                                         = 0x0000474c,
    RXQ_3_CIDX_WB_HSN_LO                                         = 0x00004748,
    RXQ_3_DESC_CIDX                                              = 0x000040f4,
    RXQ_3_DESC_PIDX                                              = 0x000040f8,
    RXQ_3_DESC_RING                                              = 0x00004750,
    RXQ_3_SCH                                                    = 0x00004770,
    RXQ_4                                                        = 0x000047b0,
    RXQ_4_BASE_HSN_HI                                            = 0x00004780,
    RXQ_4_BASE_HSN_HI_WB                                         = 0x00004788,
    RXQ_4_BASE_HSN_LO                                            = 0x0000477c,
    RXQ_4_BASE_HSN_LO_WB                                         = 0x00004784,
    RXQ_4_CACHE                                                  = 0x00004798,
    RXQ_4_CIDX_UPDATE                                            = 0x0000479c,
    RXQ_4_CIDX_UPDATE_CLIFF                                      = 0x000047a0,
    RXQ_4_CIDX_UPDATE_PRECLIFF                                   = 0x000047a8,
    RXQ_4_CIDX_UPDATE_PRESCALER                                  = 0x000047a4,
    RXQ_4_CIDX_UPDATE_TMR_CTL                                    = 0x000047ac,
    RXQ_4_CIDX_WB_HSN_HI                                         = 0x00004790,
    RXQ_4_CIDX_WB_HSN_LO                                         = 0x0000478c,
    RXQ_4_DESC_CIDX                                              = 0x000040fc,
    RXQ_4_DESC_PIDX                                              = 0x00004100,
    RXQ_4_DESC_RING                                              = 0x00004794,
    RXQ_4_SCH                                                    = 0x000047b4,
    RXQ_5                                                        = 0x000047f4,
    RXQ_5_BASE_HSN_HI                                            = 0x000047c4,
    RXQ_5_BASE_HSN_HI_WB                                         = 0x000047cc,
    RXQ_5_BASE_HSN_LO                                            = 0x000047c0,
    RXQ_5_BASE_HSN_LO_WB                                         = 0x000047c8,
    RXQ_5_CACHE                                                  = 0x000047dc,
    RXQ_5_CIDX_UPDATE                                            = 0x000047e0,
    RXQ_5_CIDX_UPDATE_CLIFF                                      = 0x000047e4,
    RXQ_5_CIDX_UPDATE_PRECLIFF                                   = 0x000047ec,
    RXQ_5_CIDX_UPDATE_PRESCALER                                  = 0x000047e8,
    RXQ_5_CIDX_UPDATE_TMR_CTL                                    = 0x000047f0,
    RXQ_5_CIDX_WB_HSN_HI                                         = 0x000047d4,
    RXQ_5_CIDX_WB_HSN_LO                                         = 0x000047d0,
    RXQ_5_DESC_CIDX                                              = 0x00004104,
    RXQ_5_DESC_PIDX                                              = 0x00004108,
    RXQ_5_DESC_RING                                              = 0x000047d8,
    RXQ_5_SCH                                                    = 0x000047f8,
    RXQ_CIDX_INTR_0                                              = 0x00004804,
    RXQ_CIDX_INTR_1                                              = 0x00004808,
    RXQ_CIDX_INTR_2                                              = 0x0000480c,
    RXQ_CIDX_INTR_3                                              = 0x00004810,
    RXQ_CIDX_INTR_4                                              = 0x00004814,
    RXQ_CIDX_INTR_5                                              = 0x00004818,
    SBUF_CPU_TO_SWITCH_CTRL                                      = 0x000048d8,
    SBUF_SWITCH_TO_CPU_0_CNFG1                                   = 0x0000495c,
    SBUF_SWITCH_TO_CPU_0_CNFG2                                   = 0x00004954,
    SBUF_SWITCH_TO_CPU_0_CTRL                                    = 0x0000494c,
    SBUF_SWITCH_TO_CPU_0_PKT                                     = 0x00004960,
    SBUF_SWITCH_TO_CPU_1_CNFG1                                   = 0x00004984,
    SBUF_SWITCH_TO_CPU_1_CNFG2                                   = 0x0000497c,
    SBUF_SWITCH_TO_CPU_1_CTRL                                    = 0x00004974,
    SBUF_SWITCH_TO_CPU_1_PKT                                     = 0x00004988,
    SBUF_SWITCH_TO_CPU_2_CNFG1                                   = 0x000049ac,
    SBUF_SWITCH_TO_CPU_2_CNFG2                                   = 0x000049a4,
    SBUF_SWITCH_TO_CPU_2_CTRL                                    = 0x0000499c,
    SBUF_SWITCH_TO_CPU_2_PKT                                     = 0x000049b0,
    SBUF_SWITCH_TO_CPU_3_CNFG1                                   = 0x000049d4,
    SBUF_SWITCH_TO_CPU_3_CNFG2                                   = 0x000049cc,
    SBUF_SWITCH_TO_CPU_3_CTRL                                    = 0x000049c4,
    SBUF_SWITCH_TO_CPU_3_PKT                                     = 0x000049d8,
    SBUF_MCU_0_CNFG1                                             = 0x0000490c,
    SBUF_MCU_0_CNFG2                                             = 0x00004904,
    SBUF_MCU_0_CTRL                                              = 0x000048fc,
    SBUF_MCU_1_CNFG1                                             = 0x00004934,
    SBUF_MCU_1_CNFG2                                             = 0x0000492c,
    SBUF_MCU_1_CTRL                                              = 0x00004924,
    SCH_QUANTUM_RXE_CNFG0                                        = 0x00004048,
    SCH_QUANTUM_RXE_CNFG1                                        = 0x0000404c,
    SCH_QUANTUM_RXE_CNFG2                                        = 0x00004050,
    SCH_QUANTUM_TXE_CNFG0                                        = 0x0000403c,
    SCH_QUANTUM_TXE_CNFG1                                        = 0x00004040,
    SCH_QUANTUM_TXE_CNFG2                                        = 0x00004044,
    STS                                                          = 0x00004024,
    SYNC_CMD_0                                                   = 0x00000054,
    SYNC_DATA_0                                                  = 0x00000100,
    SYNC_HSN_ADR_HI_0                                            = 0x00000064,
    SYNC_HSN_ADR_LO_0                                            = 0x00000060,
    SYNC_HSN_ADR_LO_3                                            = 0x0000090c,
    SYNC_ISN_ADR_HI_0                                            = 0x0000006c,
    SYNC_ISN_ADR_LO_0                                            = 0x00000068,
    SYNC_MODE                                                    = 0x00000000,
    SYNC_STS_0                                                   = 0x00000058,
    TXE_CHNL_0                                                   = 0x0000405c,
    TXE_CHNL_1                                                   = 0x00004060,
    TXE_CHNL_2                                                   = 0x00004064,
    TXE_CHNL_3                                                   = 0x00004068,
    TXQ                                                          = 0x0000402c,
    TXQ_0                                                        = 0x00004158,
    TXQ_0_BASE_HSN_HI                                            = 0x00004128,
    TXQ_0_BASE_HSN_HI_WB                                         = 0x00004130,
    TXQ_0_BASE_HSN_LO                                            = 0x00004124,
    TXQ_0_BASE_HSN_LO_WB                                         = 0x0000412c,
    TXQ_0_CACHE                                                  = 0x00004140,
    TXQ_0_CIDX_UPDATE                                            = 0x00004144,
    TXQ_0_CIDX_UPDATE_CLIFF                                      = 0x00004148,
    TXQ_0_CIDX_UPDATE_PRECLIFF                                   = 0x00004150,
    TXQ_0_CIDX_UPDATE_PRESCALER                                  = 0x0000414c,
    TXQ_0_CIDX_UPDATE_TMR_CTL                                    = 0x00004154,
    TXQ_0_CIDX_WB_HSN_HI                                         = 0x00004138,
    TXQ_0_CIDX_WB_HSN_LO                                         = 0x00004134,
    TXQ_0_DESC_CIDX                                              = 0x0000406c,
    TXQ_0_DESC_PIDX                                              = 0x00004070,
    TXQ_0_DESC_RING                                              = 0x0000413c,
    TXQ_0_SCH                                                    = 0x0000415c,
    TXQ_1                                                        = 0x000041a0,
    TXQ_1_BASE_HSN_HI                                            = 0x00004170,
    TXQ_1_BASE_HSN_HI_WB                                         = 0x00004178,
    TXQ_1_BASE_HSN_LO                                            = 0x0000416c,
    TXQ_1_BASE_HSN_LO_WB                                         = 0x00004174,
    TXQ_1_CACHE                                                  = 0x00004188,
    TXQ_1_CIDX_UPDATE                                            = 0x0000418c,
    TXQ_1_CIDX_UPDATE_CLIFF                                      = 0x00004190,
    TXQ_1_CIDX_UPDATE_PRECLIFF                                   = 0x00004198,
    TXQ_1_CIDX_UPDATE_PRESCALER                                  = 0x00004194,
    TXQ_1_CIDX_UPDATE_TMR_CTL                                    = 0x0000419c,
    TXQ_1_CIDX_WB_HSN_HI                                         = 0x00004180,
    TXQ_1_CIDX_WB_HSN_LO                                         = 0x0000417c,
    TXQ_1_DESC_CIDX                                              = 0x00004074,
    TXQ_1_DESC_PIDX                                              = 0x00004078,
    TXQ_1_DESC_RING                                              = 0x00004184,
    TXQ_1_SCH                                                    = 0x000041a4,
    TXQ_2                                                        = 0x000041e8,
    TXQ_2_BASE_HSN_HI                                            = 0x000041b8,
    TXQ_2_BASE_HSN_HI_WB                                         = 0x000041c0,
    TXQ_2_BASE_HSN_LO                                            = 0x000041b4,
    TXQ_2_BASE_HSN_LO_WB                                         = 0x000041bc,
    TXQ_2_CACHE                                                  = 0x000041d0,
    TXQ_2_CIDX_UPDATE                                            = 0x000041d4,
    TXQ_2_CIDX_UPDATE_CLIFF                                      = 0x000041d8,
    TXQ_2_CIDX_UPDATE_PRECLIFF                                   = 0x000041e0,
    TXQ_2_CIDX_UPDATE_PRESCALER                                  = 0x000041dc,
    TXQ_2_CIDX_UPDATE_TMR_CTL                                    = 0x000041e4,
    TXQ_2_CIDX_WB_HSN_HI                                         = 0x000041c8,
    TXQ_2_CIDX_WB_HSN_LO                                         = 0x000041c4,
    TXQ_2_DESC_CIDX                                              = 0x0000407c,
    TXQ_2_DESC_PIDX                                              = 0x00004080,
    TXQ_2_DESC_RING                                              = 0x000041cc,
    TXQ_2_SCH                                                    = 0x000041ec,
    TXQ_3                                                        = 0x00004230,
    TXQ_3_BASE_HSN_HI                                            = 0x00004200,
    TXQ_3_BASE_HSN_HI_WB                                         = 0x00004208,
    TXQ_3_BASE_HSN_LO                                            = 0x000041fc,
    TXQ_3_BASE_HSN_LO_WB                                         = 0x00004204,
    TXQ_3_CACHE                                                  = 0x00004218,
    TXQ_3_CIDX_UPDATE                                            = 0x0000421c,
    TXQ_3_CIDX_UPDATE_CLIFF                                      = 0x00004220,
    TXQ_3_CIDX_UPDATE_PRECLIFF                                   = 0x00004228,
    TXQ_3_CIDX_UPDATE_PRESCALER                                  = 0x00004224,
    TXQ_3_CIDX_UPDATE_TMR_CTL                                    = 0x0000422c,
    TXQ_3_CIDX_WB_HSN_HI                                         = 0x00004210,
    TXQ_3_CIDX_WB_HSN_LO                                         = 0x0000420c,
    TXQ_3_DESC_CIDX                                              = 0x00004084,
    TXQ_3_DESC_PIDX                                              = 0x00004088,
    TXQ_3_DESC_RING                                              = 0x00004214,
    TXQ_3_SCH                                                    = 0x00004234,
    TXQ_4                                                        = 0x00004278,
    TXQ_4_BASE_HSN_HI                                            = 0x00004248,
    TXQ_4_BASE_HSN_HI_WB                                         = 0x00004250,
    TXQ_4_BASE_HSN_LO                                            = 0x00004244,
    TXQ_4_BASE_HSN_LO_WB                                         = 0x0000424c,
    TXQ_4_CACHE                                                  = 0x00004260,
    TXQ_4_CIDX_UPDATE                                            = 0x00004264,
    TXQ_4_CIDX_UPDATE_CLIFF                                      = 0x00004268,
    TXQ_4_CIDX_UPDATE_PRECLIFF                                   = 0x00004270,
    TXQ_4_CIDX_UPDATE_PRESCALER                                  = 0x0000426c,
    TXQ_4_CIDX_UPDATE_TMR_CTL                                    = 0x00004274,
    TXQ_4_CIDX_WB_HSN_HI                                         = 0x00004258,
    TXQ_4_CIDX_WB_HSN_LO                                         = 0x00004254,
    TXQ_4_DESC_CIDX                                              = 0x0000408c,
    TXQ_4_DESC_PIDX                                              = 0x00004090,
    TXQ_4_DESC_RING                                              = 0x0000425c,
    TXQ_4_SCH                                                    = 0x0000427c,
    TXQ_5                                                        = 0x000042c0,
    TXQ_5_BASE_HSN_HI                                            = 0x00004290,
    TXQ_5_BASE_HSN_HI_WB                                         = 0x00004298,
    TXQ_5_BASE_HSN_LO                                            = 0x0000428c,
    TXQ_5_BASE_HSN_LO_WB                                         = 0x00004294,
    TXQ_5_CACHE                                                  = 0x000042a8,
    TXQ_5_CIDX_UPDATE                                            = 0x000042ac,
    TXQ_5_CIDX_UPDATE_CLIFF                                      = 0x000042b0,
    TXQ_5_CIDX_UPDATE_PRECLIFF                                   = 0x000042b8,
    TXQ_5_CIDX_UPDATE_PRESCALER                                  = 0x000042b4,
    TXQ_5_CIDX_UPDATE_TMR_CTL                                    = 0x000042bc,
    TXQ_5_CIDX_WB_HSN_HI                                         = 0x000042a0,
    TXQ_5_CIDX_WB_HSN_LO                                         = 0x0000429c,
    TXQ_5_DESC_CIDX                                              = 0x00004094,
    TXQ_5_DESC_PIDX                                              = 0x00004098,
    TXQ_5_DESC_RING                                              = 0x000042a4,
    TXQ_5_SCH                                                    = 0x000042c4,
    UCM_DTM                                                      = 0x00004a08,
    PCI_REG_MAX                                                  = 0x00020201
} node_pci_reg_drv_t;


typedef union {
    struct axi_timeout_cfg {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t val_f:30;
        uint32_t rsvd_f:1;
        uint32_t en_f:1;
#else 
        uint32_t en_f:1;
        uint32_t rsvd_f:1;
        uint32_t val_f:30;
#endif 
    } flds;
    uint32_t data;
} axi_timeout_cfg_t;



typedef union {
    struct cpu_to_switchif_src {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t wght_0_f:4;
        uint32_t wght_1_f:4;
        uint32_t wght_2_f:4;
        uint32_t rsvd:20;
#else 
        uint32_t rsvd:20;
        uint32_t wght_2_f:4;
        uint32_t wght_1_f:4;
        uint32_t wght_0_f:4;
#endif 
    } flds;
    uint32_t data;
} cpu_to_switchif_src_t;



typedef union {
    struct blk_and_msg_rxe {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t blk_en_0_f:1;
        uint32_t blk_rstn_0_f:1;
        uint32_t msg_en_0_f:1;
        uint32_t msg_rstn_0_f:1;
        uint32_t rsvd_f:28;
#else 
        uint32_t rsvd_f:28;
        uint32_t msg_rstn_0_f:1;
        uint32_t msg_en_0_f:1;
        uint32_t blk_rstn_0_f:1;
        uint32_t blk_en_0_f:1;
#endif 
    } flds;
    uint32_t data;
} blk_and_msg_rxe_t;



typedef union {
    struct cnfg_ref_amount_rxe {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t major_exp_bin0_f:4;
        uint32_t major_exp_bin1_f:4;
        uint32_t major_exp_bin2_f:4;
        uint32_t major_exp_bin3_f:4;
        uint32_t minor_exp_bin0_f:4;
        uint32_t minor_exp_bin1_f:4;
        uint32_t minor_exp_bin2_f:4;
        uint32_t minor_exp_bin3_f:4;
#else 
        uint32_t minor_exp_bin3_f:4;
        uint32_t minor_exp_bin2_f:4;
        uint32_t minor_exp_bin1_f:4;
        uint32_t minor_exp_bin0_f:4;
        uint32_t major_exp_bin3_f:4;
        uint32_t major_exp_bin2_f:4;
        uint32_t major_exp_bin1_f:4;
        uint32_t major_exp_bin0_f:4;
#endif 
    } flds;
    uint32_t data;
} cnfg_ref_amount_rxe_t;



typedef union {
    struct cnfg_ref_amount_txe {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t major_exp_bin0_f:4;
        uint32_t major_exp_bin1_f:4;
        uint32_t major_exp_bin2_f:4;
        uint32_t major_exp_bin3_f:4;
        uint32_t minor_exp_bin0_f:4;
        uint32_t minor_exp_bin1_f:4;
        uint32_t minor_exp_bin2_f:4;
        uint32_t minor_exp_bin3_f:4;
#else 
        uint32_t minor_exp_bin3_f:4;
        uint32_t minor_exp_bin2_f:4;
        uint32_t minor_exp_bin1_f:4;
        uint32_t minor_exp_bin0_f:4;
        uint32_t major_exp_bin3_f:4;
        uint32_t major_exp_bin2_f:4;
        uint32_t major_exp_bin1_f:4;
        uint32_t major_exp_bin0_f:4;
#endif 
    } flds;
    uint32_t data;
} cnfg_ref_amount_txe_t;


typedef union {
    struct tl10_cpu_ptp_control {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t devicetimer_enable_f:1;
        uint32_t globaltimer_enable_f:1;
        uint32_t fracns_count_enable_f:1;
        uint32_t fracns_incr_load_f:1;
        uint32_t fracns_halfclock_f:1;
        uint32_t timers_snapshot_enable_f:1;
        uint32_t globaltimer_offset_load_f:1;
        uint32_t ppsout_en_f:1;
        uint32_t serialtod_out_en_f:1;
        uint32_t serialtod_bitperiod_f:4;
        uint32_t pps_in_gpio_en_f:1;
        uint32_t ts_wr_update_debug_f:3;
        uint32_t clkout_ts_wr_update_debug_f:3;
        uint32_t ptp_sel_clka_f:1;
        uint32_t ptp_sel_clkb_f:1;
        uint32_t rsvd:10;
#else 
        uint32_t rsvd:10;
        uint32_t ptp_sel_clkb_f:1;
        uint32_t ptp_sel_clka_f:1;
        uint32_t clkout_ts_wr_update_debug_f:3;
        uint32_t ts_wr_update_debug_f:3;
        uint32_t pps_in_gpio_en_f:1;
        uint32_t serialtod_bitperiod_f:4;
        uint32_t serialtod_out_en_f:1;
        uint32_t ppsout_en_f:1;
        uint32_t globaltimer_offset_load_f:1;
        uint32_t timers_snapshot_enable_f:1;
        uint32_t fracns_halfclock_f:1;
        uint32_t fracns_incr_load_f:1;
        uint32_t fracns_count_enable_f:1;
        uint32_t globaltimer_enable_f:1;
        uint32_t devicetimer_enable_f:1;
#endif 
    } tl10_flds;
    uint32_t data;
} cpu_ptp_control_t;


typedef union {
    struct tl10_cpu_ptp_devicetimer_snapshot_31_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t st_f:32;
#else 
        uint32_t st_f:32;
#endif 
    } tl10_flds;
    uint32_t data;
} cpu_ptp_devicetimer_snapshot_31_0_t;


typedef union {
    struct tl10_cpu_ptp_devicetimer_snapshot_47_32 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t st_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t st_f:16;
#endif 
    } tl10_flds;
    uint32_t data;
} cpu_ptp_devicetimer_snapshot_47_32_t;


typedef union {
    struct tl10_cpu_ptp_synchronizedtime_snapshot_ns_31_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t st_f:32;
#else 
        uint32_t st_f:32;
#endif 
    } tl10_flds;
    uint32_t data;
} cpu_ptp_synchronizedtime_snapshot_ns_31_0_t;


typedef union {
    struct tl10_cpu_ptp_synchronizedtime_snapshot_sec_31_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t st_f:32;
#else 
        uint32_t st_f:32;
#endif 
    } tl10_flds;
    uint32_t data;
} cpu_ptp_synchronizedtime_snapshot_sec_31_0_t;


typedef union {
    struct tl10_cpu_ptp_synchronizedtime_snapshot_sec_47_32 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t st_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t st_f:16;
#endif 
    } tl10_flds;
    uint32_t data;
} cpu_ptp_synchronizedtime_snapshot_sec_47_32_t;



typedef union {
    struct desc_cidx_wrback {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t endianess_f:32;
#else 
        uint32_t endianess_f:32;
#endif 
    } flds;
    uint32_t data;
} desc_cidx_wrback_t;



typedef union {
    struct desc_read {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t endianess_f:32;
#else 
        uint32_t endianess_f:32;
#endif 
    } flds;
    uint32_t data;
} desc_read_t;



typedef union {
    struct desc_wrback {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t endianess_f:32;
#else 
        uint32_t endianess_f:32;
#endif 
    } flds;
    uint32_t data;
} desc_wrback_t;



typedef union {
    struct disable_sts_done {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t dm_txq_0_f:1;
        uint32_t txe_txq_0_f:1;
        uint32_t dm_txq_1_f:1;
        uint32_t txe_txq_1_f:1;
        uint32_t dm_txq_2_f:1;
        uint32_t txe_txq_2_f:1;
        uint32_t dm_txq_3_f:1;
        uint32_t txe_txq_3_f:1;
        uint32_t dm_txq_4_f:1;
        uint32_t txe_txq_4_f:1;
        uint32_t dm_txq_5_f:1;
        uint32_t txe_txq_5_f:1;
        uint32_t dm_txq_6_f:1;
        uint32_t txe_txq_6_f:1;
        uint32_t dm_rxq_0_f:1;
        uint32_t rxe_rxq_0_f:1;
        uint32_t dm_rxq_1_f:1;
        uint32_t rxe_rxq_1_f:1;
        uint32_t dm_rxq_2_f:1;
        uint32_t rxe_rxq_2_f:1;
        uint32_t dm_rxq_3_f:1;
        uint32_t rxe_rxq_3_f:1;
        uint32_t dm_rxq_4_f:1;
        uint32_t rxe_rxq_4_f:1;
        uint32_t dm_rxq_5_f:1;
        uint32_t rxe_rxq_5_f:1;
        uint32_t rxe_rxq_imsg_f:1;
        uint32_t rsvd:5;
#else 
        uint32_t rsvd:5;
        uint32_t rxe_rxq_imsg_f:1;
        uint32_t rxe_rxq_5_f:1;
        uint32_t dm_rxq_5_f:1;
        uint32_t rxe_rxq_4_f:1;
        uint32_t dm_rxq_4_f:1;
        uint32_t rxe_rxq_3_f:1;
        uint32_t dm_rxq_3_f:1;
        uint32_t rxe_rxq_2_f:1;
        uint32_t dm_rxq_2_f:1;
        uint32_t rxe_rxq_1_f:1;
        uint32_t dm_rxq_1_f:1;
        uint32_t rxe_rxq_0_f:1;
        uint32_t dm_rxq_0_f:1;
        uint32_t txe_txq_6_f:1;
        uint32_t dm_txq_6_f:1;
        uint32_t txe_txq_5_f:1;
        uint32_t dm_txq_5_f:1;
        uint32_t txe_txq_4_f:1;
        uint32_t dm_txq_4_f:1;
        uint32_t txe_txq_3_f:1;
        uint32_t dm_txq_3_f:1;
        uint32_t txe_txq_2_f:1;
        uint32_t dm_txq_2_f:1;
        uint32_t txe_txq_1_f:1;
        uint32_t dm_txq_1_f:1;
        uint32_t txe_txq_0_f:1;
        uint32_t dm_txq_0_f:1;
#endif 
    } flds;
    uint32_t data;
} disable_sts_done_t;



typedef union {
    struct dm_sts {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t dm_txq_en_0_f:1;
        uint32_t dm_txq_en_1_f:1;
        uint32_t dm_txq_en_2_f:1;
        uint32_t dm_txq_en_3_f:1;
        uint32_t dm_txq_en_4_f:1;
        uint32_t dm_txq_en_5_f:1;
        uint32_t dm_txq_en_6_f:1;
        uint32_t dm_rxq_en_0_f:1;
        uint32_t dm_rxq_en_1_f:1;
        uint32_t dm_rxq_en_2_f:1;
        uint32_t dm_rxq_en_3_f:1;
        uint32_t dm_rxq_en_4_f:1;
        uint32_t dm_rxq_en_5_f:1;
        uint32_t rsvd:19;
#else 
        uint32_t rsvd:19;
        uint32_t dm_rxq_en_5_f:1;
        uint32_t dm_rxq_en_4_f:1;
        uint32_t dm_rxq_en_3_f:1;
        uint32_t dm_rxq_en_2_f:1;
        uint32_t dm_rxq_en_1_f:1;
        uint32_t dm_rxq_en_0_f:1;
        uint32_t dm_txq_en_6_f:1;
        uint32_t dm_txq_en_5_f:1;
        uint32_t dm_txq_en_4_f:1;
        uint32_t dm_txq_en_3_f:1;
        uint32_t dm_txq_en_2_f:1;
        uint32_t dm_txq_en_1_f:1;
        uint32_t dm_txq_en_0_f:1;
#endif 
    } flds;
    uint32_t data;
} dm_sts_t;



typedef union {
    struct dma_global {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t rst_n_f:1;
        uint32_t intr_en_f:1;
        uint32_t txe_en_f:1;
        uint32_t rxe_en_f:1;
        uint32_t cacheline_size_f:2;
        uint32_t en_txe_cacheline_split_f:1;
        uint32_t en_rxe_cacheline_split_f:1;
        uint32_t en_dm_cacheline_split_f:1;
        uint32_t rsvd_f:22;
#else 
        uint32_t rsvd_f:22;
        uint32_t en_dm_cacheline_split_f:1;
        uint32_t en_rxe_cacheline_split_f:1;
        uint32_t en_txe_cacheline_split_f:1;
        uint32_t cacheline_size_f:2;
        uint32_t rxe_en_f:1;
        uint32_t txe_en_f:1;
        uint32_t intr_en_f:1;
        uint32_t rst_n_f:1;
        uint32_t en_f:1;
#endif 
    } flds;
    uint32_t data;
} dma_global_t;



typedef union {
    struct dma_intf {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cpu_to_switchif_en_f:1;
        uint32_t switch_to_cpuif_en_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t switch_to_cpuif_en_f:1;
        uint32_t cpu_to_switchif_en_f:1;
#endif 
    } flds;
    uint32_t data;
} dma_intf_t;



typedef union {
    struct tl10_dma_rxe_switch_to_cpu_queue_offset {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cnfg_f:10;
        uint32_t rsvd:22;
#else 
        uint32_t rsvd:22;
        uint32_t cnfg_f:10;
#endif 
    } tl10_flds;
    struct tl_dma_rxe_switch_to_cpu_queue_offset {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cnfg_f:9;
        uint32_t rsvd:23;
#else 
        uint32_t rsvd:23;
        uint32_t cnfg_f:9;
#endif 
    } tl_flds;
    uint32_t data;
} dma_rxe_switch_to_cpu_queue_offset_t;



typedef union {
    struct endian_swap_dis_iac_axim {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t placeholder_f:1;
        uint32_t rsvd_f:31;
#else 
        uint32_t rsvd_f:31;
        uint32_t placeholder_f:1;
#endif 
    } flds;
    uint32_t data;
} endian_swap_dis_iac_axim_t;



typedef union {
    struct endian_swap_dis_iac_axis {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t placeholder_f:1;
        uint32_t rsvd_f:31;
#else 
        uint32_t rsvd_f:31;
        uint32_t placeholder_f:1;
#endif 
    } flds;
    uint32_t data;
} endian_swap_dis_iac_axis_t;



typedef union {
    struct endian_swap_dis_iac_cntg {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t placeholder_f:1;
        uint32_t rsvd_f:31;
#else 
        uint32_t rsvd_f:31;
        uint32_t placeholder_f:1;
#endif 
    } flds;
    uint32_t data;
} endian_swap_dis_iac_cntg_t;



typedef union {
    struct endian_swap_dis_msi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t placeholder_f:1;
        uint32_t rsvd_f:31;
#else 
        uint32_t rsvd_f:31;
        uint32_t placeholder_f:1;
#endif 
    } flds;
    uint32_t data;
} endian_swap_dis_msi_wb_t;



typedef union {
    struct endian_swap_dis_pcie_mtr {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t placeholder_f:1;
        uint32_t rsvd_f:31;
#else 
        uint32_t rsvd_f:31;
        uint32_t placeholder_f:1;
#endif 
    } flds;
    uint32_t data;
} endian_swap_dis_pcie_mtr_t;



typedef union {
    struct endian_swap_dis_pcie_tar {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t placeholder_f:1;
        uint32_t rsvd_f:31;
#else 
        uint32_t rsvd_f:31;
        uint32_t placeholder_f:1;
#endif 
    } flds;
    uint32_t data;
} endian_swap_dis_pcie_tar_t;



typedef union {
    struct endian_swap_ena_iac_axim {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t placeholder_f:1;
        uint32_t rsvd_f:31;
#else 
        uint32_t rsvd_f:31;
        uint32_t placeholder_f:1;
#endif 
    } flds;
    uint32_t data;
} endian_swap_ena_iac_axim_t;



typedef union {
    struct endian_swap_ena_iac_axis {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t placeholder_f:1;
        uint32_t rsvd_f:31;
#else 
        uint32_t rsvd_f:31;
        uint32_t placeholder_f:1;
#endif 
    } flds;
    uint32_t data;
} endian_swap_ena_iac_axis_t;



typedef union {
    struct endian_swap_ena_iac_cntg {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t placeholder_f:1;
        uint32_t rsvd_f:31;
#else 
        uint32_t rsvd_f:31;
        uint32_t placeholder_f:1;
#endif 
    } flds;
    uint32_t data;
} endian_swap_ena_iac_cntg_t;



typedef union {
    struct endian_swap_ena_msi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t placeholder_f:1;
        uint32_t rsvd_f:31;
#else 
        uint32_t rsvd_f:31;
        uint32_t placeholder_f:1;
#endif 
    } flds;
    uint32_t data;
} endian_swap_ena_msi_wb_t;



typedef union {
    struct endian_swap_ena_pcie_tar {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t placeholder_f:1;
        uint32_t rsvd_f:31;
#else 
        uint32_t rsvd_f:31;
        uint32_t placeholder_f:1;
#endif 
    } flds;
    uint32_t data;
} endian_swap_ena_pcie_tar_t;



typedef union {
    struct endian_swap_sts_pcie_tar {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t swap_enabled_f:1;
        uint32_t rsvd_f:31;
#else 
        uint32_t rsvd_f:31;
        uint32_t swap_enabled_f:1;
#endif 
    } flds;
    uint32_t data;
} endian_swap_sts_pcie_tar_t;



typedef union {
    struct tl10_hi_watermark_imsg_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:6;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t lvl_f:6;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl10_flds;
    struct tl_hi_watermark_imsg_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:5;
        uint32_t rsvd:25;
#else 
        uint32_t rsvd:25;
        uint32_t lvl_f:5;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl_flds;
    uint32_t data;
} hi_watermark_imsg_0_t;



typedef union {
    struct tl10_hi_watermark_imsg_1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:6;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t lvl_f:6;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl10_flds;
    struct tl_hi_watermark_imsg_1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:5;
        uint32_t rsvd:25;
#else 
        uint32_t rsvd:25;
        uint32_t lvl_f:5;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl_flds;
    uint32_t data;
} hi_watermark_imsg_1_t;



typedef union {
    struct tl10_hi_watermark_imsg_2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:6;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t lvl_f:6;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl10_flds;
    struct tl_hi_watermark_imsg_2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:5;
        uint32_t rsvd:25;
#else 
        uint32_t rsvd:25;
        uint32_t lvl_f:5;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl_flds;
    uint32_t data;
} hi_watermark_imsg_2_t;



typedef union {
    struct tl10_hi_watermark_imsg_3 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:6;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t lvl_f:6;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl10_flds;
    struct tl_hi_watermark_imsg_3 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:5;
        uint32_t rsvd:25;
#else 
        uint32_t rsvd:25;
        uint32_t lvl_f:5;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl_flds;
    uint32_t data;
} hi_watermark_imsg_3_t;



typedef union {
    struct tl10_hi_watermark_imsg_4 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:6;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t lvl_f:6;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl10_flds;
    struct tl_hi_watermark_imsg_4 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:5;
        uint32_t rsvd:25;
#else 
        uint32_t rsvd:25;
        uint32_t lvl_f:5;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl_flds;
    uint32_t data;
} hi_watermark_imsg_4_t;



typedef union {
    struct tl10_hi_watermark_imsg_5 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:6;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t lvl_f:6;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl10_flds;
    struct tl_hi_watermark_imsg_5 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:5;
        uint32_t rsvd:25;
#else 
        uint32_t rsvd:25;
        uint32_t lvl_f:5;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl_flds;
    uint32_t data;
} hi_watermark_imsg_5_t;



typedef union {
    struct tl10_hi_watermark_imsg_6 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:6;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t lvl_f:6;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl10_flds;
    uint32_t data;
} hi_watermark_imsg_6_t;



typedef union {
    struct tl10_hi_watermark_imsg_7 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t clr_f:1;
        uint32_t lvl_f:6;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t lvl_f:6;
        uint32_t clr_f:1;
        uint32_t en_f:1;
#endif 
    } tl10_flds;
    uint32_t data;
} hi_watermark_imsg_7_t;



typedef union {
    struct host0_switch_to_cpu_clean_pkt_size_cntr {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cnt_num_f:32;
#else 
        uint32_t cnt_num_f:32;
#endif 
    } flds;
    uint32_t data;
} host0_switch_to_cpu_clean_pkt_size_cntr_t;



typedef union {
    struct hrr_adr_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_hi_f:32;
#else 
        uint32_t adr_hi_f:32;
#endif 
    } flds;
    uint32_t data;
} hrr_adr_hi_t;



typedef union {
    struct hrr_adr_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_lo_f:32;
#else 
        uint32_t adr_lo_f:32;
#endif 
    } flds;
    uint32_t data;
} hrr_adr_lo_t;



typedef union {
    struct hrr_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t idx_f:8;
        uint32_t rsvd_f:24;
#else 
        uint32_t rsvd_f:24;
        uint32_t idx_f:8;
#endif 
    } flds;
    uint32_t data;
} hrr_pidx_t;



typedef union {
    struct hrr_pidx_adr_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_hi_f:32;
#else 
        uint32_t adr_hi_f:32;
#endif 
    } flds;
    uint32_t data;
} hrr_pidx_adr_hi_t;



typedef union {
    struct hrr_pidx_adr_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_lo_f:32;
#else 
        uint32_t adr_lo_f:32;
#endif 
    } flds;
    uint32_t data;
} hrr_pidx_adr_lo_t;



typedef union {
    struct hrr_size {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t slot_num_f:8;
        uint32_t rsvd_f:8;
        uint32_t slot_size_f:16;
#else 
        uint32_t slot_size_f:16;
        uint32_t rsvd_f:8;
        uint32_t slot_num_f:8;
#endif 
    } flds;
    uint32_t data;
} hrr_size_t;



typedef union {
    struct hsr_adr_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_hi_f:32;
#else 
        uint32_t adr_hi_f:32;
#endif 
    } flds;
    uint32_t data;
} hsr_adr_hi_t;



typedef union {
    struct hsr_adr_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_lo_f:32;
#else 
        uint32_t adr_lo_f:32;
#endif 
    } flds;
    uint32_t data;
} hsr_adr_lo_t;



typedef union {
    struct iac_mem_init {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t bram_f:1;
        uint32_t asc_cmn_f:1;
        uint32_t asc_comp0_f:1;
        uint32_t asc_comp1_f:1;
        uint32_t asc_comp2_f:1;
        uint32_t asc_comp3_f:1;
        uint32_t asc_comp4_f:1;
        uint32_t asc_comp5_f:1;
        uint32_t asc_comp6_f:1;
        uint32_t asc_comp7_f:1;
        uint32_t rsvd_f:22;
#else 
        uint32_t rsvd_f:22;
        uint32_t asc_comp7_f:1;
        uint32_t asc_comp6_f:1;
        uint32_t asc_comp5_f:1;
        uint32_t asc_comp4_f:1;
        uint32_t asc_comp3_f:1;
        uint32_t asc_comp2_f:1;
        uint32_t asc_comp1_f:1;
        uint32_t asc_comp0_f:1;
        uint32_t asc_cmn_f:1;
        uint32_t bram_f:1;
#endif 
    } flds;
    uint32_t data;
} iac_mem_init_t;



typedef union {
    struct tl10_intr_iac_1_cause {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t imsg6_hdr_err_f:1;
        uint32_t imsg7_hdr_err_f:1;
        uint32_t imsg6_perr_f:1;
        uint32_t imsg7_perr_f:1;
        uint32_t imsg6_rsp_switch_err_f:1;
        uint32_t imsg7_rsp_switch_err_f:1;
        uint32_t rsvd:26;
#else 
        uint32_t rsvd:26;
        uint32_t imsg7_rsp_switch_err_f:1;
        uint32_t imsg6_rsp_switch_err_f:1;
        uint32_t imsg7_perr_f:1;
        uint32_t imsg6_perr_f:1;
        uint32_t imsg7_hdr_err_f:1;
        uint32_t imsg6_hdr_err_f:1;
#endif 
    } tl10_flds;
    uint32_t data;
} intr_iac_1_cause_t;



typedef union {
    struct tl10_intr_iac_1_pci_mask {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t imsg6_hdr_err_f:1;
        uint32_t imsg7_hdr_err_f:1;
        uint32_t imsg6_perr_f:1;
        uint32_t imsg7_perr_f:1;
        uint32_t imsg6_rsp_switch_err_f:1;
        uint32_t imsg7_rsp_switch_err_f:1;
        uint32_t rsvd:26;
#else 
        uint32_t rsvd:26;
        uint32_t imsg7_rsp_switch_err_f:1;
        uint32_t imsg6_rsp_switch_err_f:1;
        uint32_t imsg7_perr_f:1;
        uint32_t imsg6_perr_f:1;
        uint32_t imsg7_hdr_err_f:1;
        uint32_t imsg6_hdr_err_f:1;
#endif 
    } tl10_flds;
    uint32_t data;
} intr_iac_1_pci_mask_t;



typedef union {
    struct intr_iac_cause {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t hrr_cmd_done_f:1;
        uint32_t hsr_cmd_done_f:1;
        uint32_t mc0_cmd_done_f:1;
        uint32_t mc1_cmd_done_f:1;
        uint32_t ascp_err_full_f:1;
        uint32_t ascp_err_head_f:1;
        uint32_t ascp_err_body_f:1;
        uint32_t ascp_err_det_f:1;
        uint32_t axi_async_err_f:1;
        uint32_t asc_err_det_f:1;
        uint32_t imsg0_hdr_err_f:1;
        uint32_t imsg1_hdr_err_f:1;
        uint32_t imsg2_hdr_err_f:1;
        uint32_t imsg3_hdr_err_f:1;
        uint32_t imsg4_hdr_err_f:1;
        uint32_t imsg5_hdr_err_f:1;
        uint32_t imsg0_perr_f:1;
        uint32_t imsg1_perr_f:1;
        uint32_t imsg2_perr_f:1;
        uint32_t imsg3_perr_f:1;
        uint32_t imsg4_perr_f:1;
        uint32_t imsg5_perr_f:1;
        uint32_t imsg0_rsp_switch_err_f:1;
        uint32_t imsg1_rsp_switch_err_f:1;
        uint32_t imsg2_rsp_switch_err_f:1;
        uint32_t imsg3_rsp_switch_err_f:1;
        uint32_t imsg4_rsp_switch_err_f:1;
        uint32_t imsg5_rsp_switch_err_f:1;
        uint32_t ecc_cerr_f:1;
        uint32_t ecc_uerr_f:1;
        uint32_t hsn_slverr_f:1;
        uint32_t msn_slverr_f:1;
#else 
        uint32_t msn_slverr_f:1;
        uint32_t hsn_slverr_f:1;
        uint32_t ecc_uerr_f:1;
        uint32_t ecc_cerr_f:1;
        uint32_t imsg5_rsp_switch_err_f:1;
        uint32_t imsg4_rsp_switch_err_f:1;
        uint32_t imsg3_rsp_switch_err_f:1;
        uint32_t imsg2_rsp_switch_err_f:1;
        uint32_t imsg1_rsp_switch_err_f:1;
        uint32_t imsg0_rsp_switch_err_f:1;
        uint32_t imsg5_perr_f:1;
        uint32_t imsg4_perr_f:1;
        uint32_t imsg3_perr_f:1;
        uint32_t imsg2_perr_f:1;
        uint32_t imsg1_perr_f:1;
        uint32_t imsg0_perr_f:1;
        uint32_t imsg5_hdr_err_f:1;
        uint32_t imsg4_hdr_err_f:1;
        uint32_t imsg3_hdr_err_f:1;
        uint32_t imsg2_hdr_err_f:1;
        uint32_t imsg1_hdr_err_f:1;
        uint32_t imsg0_hdr_err_f:1;
        uint32_t asc_err_det_f:1;
        uint32_t axi_async_err_f:1;
        uint32_t ascp_err_det_f:1;
        uint32_t ascp_err_body_f:1;
        uint32_t ascp_err_head_f:1;
        uint32_t ascp_err_full_f:1;
        uint32_t mc1_cmd_done_f:1;
        uint32_t mc0_cmd_done_f:1;
        uint32_t hsr_cmd_done_f:1;
        uint32_t hrr_cmd_done_f:1;
#endif 
    } flds;
    uint32_t data;
} intr_iac_cause_t;



typedef union {
    struct intr_iac_pci_mask {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t hrr_cmd_done_f:1;
        uint32_t hsr_cmd_done_f:1;
        uint32_t mc0_cmd_done_f:1;
        uint32_t mc1_cmd_done_f:1;
        uint32_t ascp_err_full_f:1;
        uint32_t ascp_err_head_f:1;
        uint32_t ascp_err_body_f:1;
        uint32_t ascp_err_det_f:1;
        uint32_t axi_async_err_f:1;
        uint32_t asc_err_det_f:1;
        uint32_t imsg0_hdr_err_f:1;
        uint32_t imsg1_hdr_err_f:1;
        uint32_t imsg2_hdr_err_f:1;
        uint32_t imsg3_hdr_err_f:1;
        uint32_t imsg4_hdr_err_f:1;
        uint32_t imsg5_hdr_err_f:1;
        uint32_t imsg0_perr_f:1;
        uint32_t imsg1_perr_f:1;
        uint32_t imsg2_perr_f:1;
        uint32_t imsg3_perr_f:1;
        uint32_t imsg4_perr_f:1;
        uint32_t imsg5_perr_f:1;
        uint32_t imsg0_rsp_switch_err_f:1;
        uint32_t imsg1_rsp_switch_err_f:1;
        uint32_t imsg2_rsp_switch_err_f:1;
        uint32_t imsg3_rsp_switch_err_f:1;
        uint32_t imsg4_rsp_switch_err_f:1;
        uint32_t imsg5_rsp_switch_err_f:1;
        uint32_t ecc_cerr_f:1;
        uint32_t ecc_uerr_f:1;
        uint32_t hsn_slverr_f:1;
        uint32_t msn_slverr_f:1;
#else 
        uint32_t msn_slverr_f:1;
        uint32_t hsn_slverr_f:1;
        uint32_t ecc_uerr_f:1;
        uint32_t ecc_cerr_f:1;
        uint32_t imsg5_rsp_switch_err_f:1;
        uint32_t imsg4_rsp_switch_err_f:1;
        uint32_t imsg3_rsp_switch_err_f:1;
        uint32_t imsg2_rsp_switch_err_f:1;
        uint32_t imsg1_rsp_switch_err_f:1;
        uint32_t imsg0_rsp_switch_err_f:1;
        uint32_t imsg5_perr_f:1;
        uint32_t imsg4_perr_f:1;
        uint32_t imsg3_perr_f:1;
        uint32_t imsg2_perr_f:1;
        uint32_t imsg1_perr_f:1;
        uint32_t imsg0_perr_f:1;
        uint32_t imsg5_hdr_err_f:1;
        uint32_t imsg4_hdr_err_f:1;
        uint32_t imsg3_hdr_err_f:1;
        uint32_t imsg2_hdr_err_f:1;
        uint32_t imsg1_hdr_err_f:1;
        uint32_t imsg0_hdr_err_f:1;
        uint32_t asc_err_det_f:1;
        uint32_t axi_async_err_f:1;
        uint32_t ascp_err_det_f:1;
        uint32_t ascp_err_body_f:1;
        uint32_t ascp_err_head_f:1;
        uint32_t ascp_err_full_f:1;
        uint32_t mc1_cmd_done_f:1;
        uint32_t mc0_cmd_done_f:1;
        uint32_t hsr_cmd_done_f:1;
        uint32_t hrr_cmd_done_f:1;
#endif 
    } flds;
    uint32_t data;
} intr_iac_pci_mask_t;



typedef union {
    struct tl_intr_immr {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t vec_f:5;
        uint32_t rvsrd_f:27;
#else 
        uint32_t rvsrd_f:27;
        uint32_t vec_f:5;
#endif 
    } tl_flds;
    uint32_t data;
} intr_immr_t;



typedef union {
    struct tl10_intr_immr_1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t vec_f:5;
        uint32_t rvsrd_f:27;
#else 
        uint32_t rvsrd_f:27;
        uint32_t vec_f:5;
#endif 
    } tl10_flds;
    uint32_t data;
} intr_immr_1_t;



typedef union {
    struct tl_intr_incr {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cause_f:32;
#else 
        uint32_t cause_f:32;
#endif 
    } tl_flds;
    uint32_t data;
} intr_incr_t;



typedef union {
    struct tl10_intr_incr_1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cause_f:32;
#else 
        uint32_t cause_f:32;
#endif 
    } tl10_flds;
    uint32_t data;
} intr_incr_1_t;



typedef union {
    struct tl_intr_inmc {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t clr_f:32;
#else 
        uint32_t clr_f:32;
#endif 
    } tl_flds;
    uint32_t data;
} intr_inmc_t;



typedef union {
    struct tl10_intr_inmc_1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t clr_f:32;
#else 
        uint32_t clr_f:32;
#endif 
    } tl10_flds;
    uint32_t data;
} intr_inmc_1_t;



typedef union {
    struct tl_intr_inms {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t mask_f:32;
#else 
        uint32_t mask_f:32;
#endif 
    } tl_flds;
    uint32_t data;
} intr_inms_t;



typedef union {
    struct tl10_intr_inms_1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t mask_f:32;
#else 
        uint32_t mask_f:32;
#endif 
    } tl10_flds;
    uint32_t data;
} intr_inms_1_t;



typedef union {
    struct intr_psc_ahi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_f:32;
#else 
        uint32_t adr_f:32;
#endif 
    } flds;
    uint32_t data;
} intr_psc_ahi_t;



typedef union {
    struct intr_psc_alo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_base_f:3;
        uint32_t adr_f:29;
#else 
        uint32_t adr_f:29;
        uint32_t adr_base_f:3;
#endif 
    } flds;
    uint32_t data;
} intr_psc_alo_t;



typedef union {
    struct intr_psc_tmr {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t val_f:32;
#else 
        uint32_t val_f:32;
#endif 
    } flds;
    uint32_t data;
} intr_psc_tmr_t;



typedef union {
    struct intr_wb_ahi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_f:32;
#else 
        uint32_t adr_f:32;
#endif 
    } flds;
    uint32_t data;
} intr_wb_ahi_t;



typedef union {
    struct intr_wb_alo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_base_f:3;
        uint32_t adr_f:29;
#else 
        uint32_t adr_f:29;
        uint32_t adr_base_f:3;
#endif 
    } flds;
    uint32_t data;
} intr_wb_alo_t;



typedef union {
    struct intr_wb_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t en_f:1;
        uint32_t rvsrd_f:31;
#else 
        uint32_t rvsrd_f:31;
        uint32_t en_f:1;
#endif 
    } flds;
    uint32_t data;
} intr_wb_ctl_t;



typedef union {
    struct isn_timeout_cfg {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t val_f:30;
        uint32_t rsvd_f:1;
        uint32_t en_f:1;
#else 
        uint32_t en_f:1;
        uint32_t rsvd_f:1;
        uint32_t val_f:30;
#endif 
    } flds;
    uint32_t data;
} isn_timeout_cfg_t;



typedef union {
    struct jtag_idcode_o {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t dft_status_f:32;
#else 
        uint32_t dft_status_f:32;
#endif 
    } flds;
    uint32_t data;
} jtag_idcode_o_t;



typedef union {
    struct tl10_learn_msg {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t xoff_0_f:6;
        uint32_t xoff_1_f:6;
        uint32_t xoff_2_f:6;
        uint32_t xoff_3_f:6;
        uint32_t rsvd:8;
#else 
        uint32_t rsvd:8;
        uint32_t xoff_3_f:6;
        uint32_t xoff_2_f:6;
        uint32_t xoff_1_f:6;
        uint32_t xoff_0_f:6;
#endif 
    } tl10_flds;
    struct tl_learn_msg {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t xoff_0_f:5;
        uint32_t xoff_1_f:5;
        uint32_t xoff_2_f:5;
        uint32_t xoff_3_f:5;
        uint32_t xoff_4_f:5;
        uint32_t xoff_5_f:5;
        uint32_t rsvd:2;
#else 
        uint32_t rsvd:2;
        uint32_t xoff_5_f:5;
        uint32_t xoff_4_f:5;
        uint32_t xoff_3_f:5;
        uint32_t xoff_2_f:5;
        uint32_t xoff_1_f:5;
        uint32_t xoff_0_f:5;
#endif 
    } tl_flds;
    uint32_t data;
} learn_msg_t;



typedef union {
    struct tl10_learn_msg_1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t xoff_4_f:6;
        uint32_t xoff_5_f:6;
        uint32_t xoff_6_f:6;
        uint32_t xoff_7_f:6;
        uint32_t rsvd:8;
#else 
        uint32_t rsvd:8;
        uint32_t xoff_7_f:6;
        uint32_t xoff_6_f:6;
        uint32_t xoff_5_f:6;
        uint32_t xoff_4_f:6;
#endif 
    } tl10_flds;
    uint32_t data;
} learn_msg_1_t;



typedef union {
    struct mcu_mem_init {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t mcu1_f:1;
        uint32_t mcu2_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t mcu2_f:1;
        uint32_t mcu1_f:1;
#endif 
    } flds;
    uint32_t data;
} mcu_mem_init_t;


typedef union {
    struct pci_axi_link_down_indicator_bit_l0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t clear_link_down_bit_to_proceed_f:1;
        uint32_t rsvd:31;
#else 
        uint32_t rsvd:31;
        uint32_t clear_link_down_bit_to_proceed_f:1;
#endif 
    } flds;
    uint32_t data;
} pci_axi_link_down_indicator_bit_l0_t;



typedef union {
    struct rxe_dma_msg_data_base_hi_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxe_dma_msg_data_base_hi_0_t;



typedef union {
    struct rxe_dma_msg_data_base_lo_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:3;
        uint32_t addr_f:29;
#else 
        uint32_t addr_f:29;
        uint32_t rsvd_f:3;
#endif 
    } flds;
    uint32_t data;
} rxe_dma_msg_data_base_lo_0_t;



typedef union {
    struct rxe_dma_msg_data_cidx_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t idx_f:21;
        uint32_t rsvd:11;
#else 
        uint32_t rsvd:11;
        uint32_t idx_f:21;
#endif 
    } flds;
    uint32_t data;
} rxe_dma_msg_data_cidx_0_t;



typedef union {
    struct rxe_dma_msg_data_pidx_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t idx_f:21;
        uint32_t rsvd:11;
#else 
        uint32_t rsvd:11;
        uint32_t idx_f:21;
#endif 
    } flds;
    uint32_t data;
} rxe_dma_msg_data_pidx_0_t;



typedef union {
    struct rxe_dma_msg_desc_base_hi_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxe_dma_msg_desc_base_hi_0_t;



typedef union {
    struct rxe_dma_msg_desc_base_lo_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:3;
        uint32_t addr_f:29;
#else 
        uint32_t addr_f:29;
        uint32_t rsvd_f:3;
#endif 
    } flds;
    uint32_t data;
} rxe_dma_msg_desc_base_lo_0_t;



typedef union {
    struct rxe_dma_msg_desc_cidx_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t idx_f:13;
        uint32_t rsvd:19;
#else 
        uint32_t rsvd:19;
        uint32_t idx_f:13;
#endif 
    } flds;
    uint32_t data;
} rxe_dma_msg_desc_cidx_0_t;



typedef union {
    struct rxe_dma_msg_desc_pidx_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t idx_f:13;
        uint32_t rsvd:19;
#else 
        uint32_t rsvd:19;
        uint32_t idx_f:13;
#endif 
    } flds;
    uint32_t data;
} rxe_dma_msg_desc_pidx_0_t;



typedef union {
    struct rxe_dma_msg_pidx_wb_base_hi_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxe_dma_msg_pidx_wb_base_hi_0_t;



typedef union {
    struct rxe_dma_msg_pidx_wb_base_lo_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxe_dma_msg_pidx_wb_base_lo_0_t;



typedef union {
    struct rxe_dma_msg_size_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t data_ring_size_f:20;
        uint32_t desc_ring_size_f:12;
#else 
        uint32_t desc_ring_size_f:12;
        uint32_t data_ring_size_f:20;
#endif 
    } flds;
    uint32_t data;
} rxe_dma_msg_size_0_t;



typedef union {
    struct rxe_dma_msg_status_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t sts_f:32;
#else 
        uint32_t sts_f:32;
#endif 
    } flds;
    uint32_t data;
} rxe_dma_msg_status_0_t;



typedef union {
    struct rxe_imsg {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t endianess_f:32;
#else 
        uint32_t endianess_f:32;
#endif 
    } flds;
    uint32_t data;
} rxe_imsg_t;



typedef union {
    struct rxe_imsg_wrback {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t endianess_f:32;
#else 
        uint32_t endianess_f:32;
#endif 
    } flds;
    uint32_t data;
} rxe_imsg_wrback_t;



typedef union {
    struct rxe_total_pkt_size_cntr {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cnt_num_f:32;
#else 
        uint32_t cnt_num_f:32;
#endif 
    } flds;
    uint32_t data;
} rxe_total_pkt_size_cntr_t;



typedef union {
    struct rxq {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t dm_en_0_f:1;
        uint32_t rxe_en_0_f:1;
        uint32_t dm_rst_n_0_f:1;
        uint32_t rxe_rst_n_0_f:1;
        uint32_t dm_en_1_f:1;
        uint32_t rxe_en_1_f:1;
        uint32_t dm_rst_n_1_f:1;
        uint32_t rxe_rst_n_1_f:1;
        uint32_t dm_en_2_f:1;
        uint32_t rxe_en_2_f:1;
        uint32_t dm_rst_n_2_f:1;
        uint32_t rxe_rst_n_2_f:1;
        uint32_t dm_en_3_f:1;
        uint32_t rxe_en_3_f:1;
        uint32_t dm_rst_n_3_f:1;
        uint32_t rxe_rst_n_3_f:1;
        uint32_t dm_en_4_f:1;
        uint32_t rxe_en_4_f:1;
        uint32_t dm_rst_n_4_f:1;
        uint32_t rxe_rst_n_4_f:1;
        uint32_t dm_en_5_f:1;
        uint32_t rxe_en_5_f:1;
        uint32_t dm_rst_n_5_f:1;
        uint32_t rxe_rst_n_5_f:1;
        uint32_t rsvd_f:8;
#else 
        uint32_t rsvd_f:8;
        uint32_t rxe_rst_n_5_f:1;
        uint32_t dm_rst_n_5_f:1;
        uint32_t rxe_en_5_f:1;
        uint32_t dm_en_5_f:1;
        uint32_t rxe_rst_n_4_f:1;
        uint32_t dm_rst_n_4_f:1;
        uint32_t rxe_en_4_f:1;
        uint32_t dm_en_4_f:1;
        uint32_t rxe_rst_n_3_f:1;
        uint32_t dm_rst_n_3_f:1;
        uint32_t rxe_en_3_f:1;
        uint32_t dm_en_3_f:1;
        uint32_t rxe_rst_n_2_f:1;
        uint32_t dm_rst_n_2_f:1;
        uint32_t rxe_en_2_f:1;
        uint32_t dm_en_2_f:1;
        uint32_t rxe_rst_n_1_f:1;
        uint32_t dm_rst_n_1_f:1;
        uint32_t rxe_en_1_f:1;
        uint32_t dm_en_1_f:1;
        uint32_t rxe_rst_n_0_f:1;
        uint32_t dm_rst_n_0_f:1;
        uint32_t rxe_en_0_f:1;
        uint32_t dm_en_0_f:1;
#endif 
    } flds;
    uint32_t data;
} rxq_t;



typedef union {
    struct rxq_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cidx_wb_thres_f:16;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t dma_type_f:1;
        uint32_t chnl_map_f:3;
        uint32_t rsvd:11;
#else 
        uint32_t rsvd:11;
        uint32_t chnl_map_f:3;
        uint32_t dma_type_f:1;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t cidx_wb_thres_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_0_t;



typedef union {
    struct rxq_0_base_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_0_base_hsn_hi_t;



typedef union {
    struct rxq_0_base_hsn_hi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_0_base_hsn_hi_wb_t;



typedef union {
    struct rxq_0_base_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} rxq_0_base_hsn_lo_t;



typedef union {
    struct rxq_0_base_hsn_lo_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} rxq_0_base_hsn_lo_wb_t;



typedef union {
    struct rxq_0_cache {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:12;
        uint32_t size_f:12;
        uint32_t lo_threshold_f:8;
#else 
        uint32_t lo_threshold_f:8;
        uint32_t size_f:12;
        uint32_t start_addr_f:12;
#endif 
    } flds;
    uint32_t data;
} rxq_0_cache_t;



typedef union {
    struct rxq_0_cidx_update {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_0_cidx_update_t;



typedef union {
    struct rxq_0_cidx_update_cliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_0_cidx_update_cliff_t;



typedef union {
    struct rxq_0_cidx_update_precliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_0_cidx_update_precliff_t;



typedef union {
    struct rxq_0_cidx_update_prescaler {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_0_cidx_update_prescaler_t;



typedef union {
    struct rxq_0_cidx_update_tmr_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t restart_f:1;
        uint32_t enable_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t enable_f:1;
        uint32_t restart_f:1;
#endif 
    } flds;
    uint32_t data;
} rxq_0_cidx_update_tmr_ctl_t;



typedef union {
    struct rxq_0_cidx_wb_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_0_cidx_wb_hsn_hi_t;



typedef union {
    struct rxq_0_cidx_wb_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:2;
        uint32_t addr_f:30;
#else 
        uint32_t addr_f:30;
        uint32_t rsvd_f:2;
#endif 
    } flds;
    uint32_t data;
} rxq_0_cidx_wb_hsn_lo_t;



typedef union {
    struct rxq_0_desc_cidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_0_desc_cidx_t;



typedef union {
    struct rxq_0_desc_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_0_desc_pidx_t;



typedef union {
    struct rxq_0_desc_ring {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t size_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t size_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_0_desc_ring_t;



typedef union {
    struct rxq_0_sch {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} rxq_0_sch_t;



typedef union {
    struct rxq_1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cidx_wb_thres_f:16;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t dma_type_f:1;
        uint32_t chnl_map_f:3;
        uint32_t rsvd:11;
#else 
        uint32_t rsvd:11;
        uint32_t chnl_map_f:3;
        uint32_t dma_type_f:1;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t cidx_wb_thres_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_1_t;



typedef union {
    struct rxq_1_base_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_1_base_hsn_hi_t;



typedef union {
    struct rxq_1_base_hsn_hi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_1_base_hsn_hi_wb_t;



typedef union {
    struct rxq_1_base_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} rxq_1_base_hsn_lo_t;



typedef union {
    struct rxq_1_base_hsn_lo_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} rxq_1_base_hsn_lo_wb_t;



typedef union {
    struct rxq_1_cache {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:12;
        uint32_t size_f:12;
        uint32_t lo_threshold_f:8;
#else 
        uint32_t lo_threshold_f:8;
        uint32_t size_f:12;
        uint32_t start_addr_f:12;
#endif 
    } flds;
    uint32_t data;
} rxq_1_cache_t;



typedef union {
    struct rxq_1_cidx_update {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_1_cidx_update_t;



typedef union {
    struct rxq_1_cidx_update_cliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_1_cidx_update_cliff_t;



typedef union {
    struct rxq_1_cidx_update_precliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_1_cidx_update_precliff_t;



typedef union {
    struct rxq_1_cidx_update_prescaler {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_1_cidx_update_prescaler_t;



typedef union {
    struct rxq_1_cidx_update_tmr_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t restart_f:1;
        uint32_t enable_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t enable_f:1;
        uint32_t restart_f:1;
#endif 
    } flds;
    uint32_t data;
} rxq_1_cidx_update_tmr_ctl_t;



typedef union {
    struct rxq_1_cidx_wb_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_1_cidx_wb_hsn_hi_t;



typedef union {
    struct rxq_1_cidx_wb_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:2;
        uint32_t addr_f:30;
#else 
        uint32_t addr_f:30;
        uint32_t rsvd_f:2;
#endif 
    } flds;
    uint32_t data;
} rxq_1_cidx_wb_hsn_lo_t;



typedef union {
    struct rxq_1_desc_cidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_1_desc_cidx_t;



typedef union {
    struct rxq_1_desc_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_1_desc_pidx_t;



typedef union {
    struct rxq_1_desc_ring {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t size_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t size_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_1_desc_ring_t;



typedef union {
    struct rxq_1_sch {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} rxq_1_sch_t;



typedef union {
    struct rxq_2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cidx_wb_thres_f:16;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t dma_type_f:1;
        uint32_t chnl_map_f:3;
        uint32_t rsvd:11;
#else 
        uint32_t rsvd:11;
        uint32_t chnl_map_f:3;
        uint32_t dma_type_f:1;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t cidx_wb_thres_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_2_t;



typedef union {
    struct rxq_2_base_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_2_base_hsn_hi_t;



typedef union {
    struct rxq_2_base_hsn_hi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_2_base_hsn_hi_wb_t;



typedef union {
    struct rxq_2_base_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} rxq_2_base_hsn_lo_t;



typedef union {
    struct rxq_2_base_hsn_lo_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} rxq_2_base_hsn_lo_wb_t;



typedef union {
    struct rxq_2_cache {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:12;
        uint32_t size_f:12;
        uint32_t lo_threshold_f:8;
#else 
        uint32_t lo_threshold_f:8;
        uint32_t size_f:12;
        uint32_t start_addr_f:12;
#endif 
    } flds;
    uint32_t data;
} rxq_2_cache_t;



typedef union {
    struct rxq_2_cidx_update {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_2_cidx_update_t;



typedef union {
    struct rxq_2_cidx_update_cliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_2_cidx_update_cliff_t;



typedef union {
    struct rxq_2_cidx_update_precliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_2_cidx_update_precliff_t;



typedef union {
    struct rxq_2_cidx_update_prescaler {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_2_cidx_update_prescaler_t;



typedef union {
    struct rxq_2_cidx_update_tmr_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t restart_f:1;
        uint32_t enable_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t enable_f:1;
        uint32_t restart_f:1;
#endif 
    } flds;
    uint32_t data;
} rxq_2_cidx_update_tmr_ctl_t;



typedef union {
    struct rxq_2_cidx_wb_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_2_cidx_wb_hsn_hi_t;



typedef union {
    struct rxq_2_cidx_wb_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:2;
        uint32_t addr_f:30;
#else 
        uint32_t addr_f:30;
        uint32_t rsvd_f:2;
#endif 
    } flds;
    uint32_t data;
} rxq_2_cidx_wb_hsn_lo_t;



typedef union {
    struct rxq_2_desc_cidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_2_desc_cidx_t;



typedef union {
    struct rxq_2_desc_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_2_desc_pidx_t;



typedef union {
    struct rxq_2_desc_ring {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t size_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t size_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_2_desc_ring_t;



typedef union {
    struct rxq_2_sch {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} rxq_2_sch_t;



typedef union {
    struct rxq_3 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cidx_wb_thres_f:16;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t dma_type_f:1;
        uint32_t chnl_map_f:3;
        uint32_t rsvd:11;
#else 
        uint32_t rsvd:11;
        uint32_t chnl_map_f:3;
        uint32_t dma_type_f:1;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t cidx_wb_thres_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_3_t;



typedef union {
    struct rxq_3_base_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_3_base_hsn_hi_t;



typedef union {
    struct rxq_3_base_hsn_hi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_3_base_hsn_hi_wb_t;



typedef union {
    struct rxq_3_base_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} rxq_3_base_hsn_lo_t;



typedef union {
    struct rxq_3_base_hsn_lo_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} rxq_3_base_hsn_lo_wb_t;



typedef union {
    struct rxq_3_cache {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:12;
        uint32_t size_f:12;
        uint32_t lo_threshold_f:8;
#else 
        uint32_t lo_threshold_f:8;
        uint32_t size_f:12;
        uint32_t start_addr_f:12;
#endif 
    } flds;
    uint32_t data;
} rxq_3_cache_t;



typedef union {
    struct rxq_3_cidx_update {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_3_cidx_update_t;



typedef union {
    struct rxq_3_cidx_update_cliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_3_cidx_update_cliff_t;



typedef union {
    struct rxq_3_cidx_update_precliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_3_cidx_update_precliff_t;



typedef union {
    struct rxq_3_cidx_update_prescaler {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_3_cidx_update_prescaler_t;



typedef union {
    struct rxq_3_cidx_update_tmr_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t restart_f:1;
        uint32_t enable_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t enable_f:1;
        uint32_t restart_f:1;
#endif 
    } flds;
    uint32_t data;
} rxq_3_cidx_update_tmr_ctl_t;



typedef union {
    struct rxq_3_cidx_wb_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_3_cidx_wb_hsn_hi_t;



typedef union {
    struct rxq_3_cidx_wb_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:2;
        uint32_t addr_f:30;
#else 
        uint32_t addr_f:30;
        uint32_t rsvd_f:2;
#endif 
    } flds;
    uint32_t data;
} rxq_3_cidx_wb_hsn_lo_t;



typedef union {
    struct rxq_3_desc_cidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_3_desc_cidx_t;



typedef union {
    struct rxq_3_desc_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_3_desc_pidx_t;



typedef union {
    struct rxq_3_desc_ring {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t size_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t size_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_3_desc_ring_t;



typedef union {
    struct rxq_3_sch {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} rxq_3_sch_t;



typedef union {
    struct rxq_4 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cidx_wb_thres_f:16;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t dma_type_f:1;
        uint32_t chnl_map_f:3;
        uint32_t rsvd:11;
#else 
        uint32_t rsvd:11;
        uint32_t chnl_map_f:3;
        uint32_t dma_type_f:1;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t cidx_wb_thres_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_4_t;



typedef union {
    struct rxq_4_base_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_4_base_hsn_hi_t;



typedef union {
    struct rxq_4_base_hsn_hi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_4_base_hsn_hi_wb_t;



typedef union {
    struct rxq_4_base_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} rxq_4_base_hsn_lo_t;



typedef union {
    struct rxq_4_base_hsn_lo_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} rxq_4_base_hsn_lo_wb_t;



typedef union {
    struct rxq_4_cache {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:12;
        uint32_t size_f:12;
        uint32_t lo_threshold_f:8;
#else 
        uint32_t lo_threshold_f:8;
        uint32_t size_f:12;
        uint32_t start_addr_f:12;
#endif 
    } flds;
    uint32_t data;
} rxq_4_cache_t;



typedef union {
    struct rxq_4_cidx_update {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_4_cidx_update_t;



typedef union {
    struct rxq_4_cidx_update_cliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_4_cidx_update_cliff_t;



typedef union {
    struct rxq_4_cidx_update_precliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_4_cidx_update_precliff_t;



typedef union {
    struct rxq_4_cidx_update_prescaler {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_4_cidx_update_prescaler_t;



typedef union {
    struct rxq_4_cidx_update_tmr_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t restart_f:1;
        uint32_t enable_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t enable_f:1;
        uint32_t restart_f:1;
#endif 
    } flds;
    uint32_t data;
} rxq_4_cidx_update_tmr_ctl_t;



typedef union {
    struct rxq_4_cidx_wb_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_4_cidx_wb_hsn_hi_t;



typedef union {
    struct rxq_4_cidx_wb_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:2;
        uint32_t addr_f:30;
#else 
        uint32_t addr_f:30;
        uint32_t rsvd_f:2;
#endif 
    } flds;
    uint32_t data;
} rxq_4_cidx_wb_hsn_lo_t;



typedef union {
    struct rxq_4_desc_cidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_4_desc_cidx_t;



typedef union {
    struct rxq_4_desc_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_4_desc_pidx_t;



typedef union {
    struct rxq_4_desc_ring {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t size_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t size_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_4_desc_ring_t;



typedef union {
    struct rxq_4_sch {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} rxq_4_sch_t;



typedef union {
    struct rxq_5 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cidx_wb_thres_f:16;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t dma_type_f:1;
        uint32_t chnl_map_f:3;
        uint32_t rsvd:11;
#else 
        uint32_t rsvd:11;
        uint32_t chnl_map_f:3;
        uint32_t dma_type_f:1;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t cidx_wb_thres_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_5_t;



typedef union {
    struct rxq_5_base_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_5_base_hsn_hi_t;



typedef union {
    struct rxq_5_base_hsn_hi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_5_base_hsn_hi_wb_t;



typedef union {
    struct rxq_5_base_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} rxq_5_base_hsn_lo_t;



typedef union {
    struct rxq_5_base_hsn_lo_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} rxq_5_base_hsn_lo_wb_t;



typedef union {
    struct rxq_5_cache {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:12;
        uint32_t size_f:12;
        uint32_t lo_threshold_f:8;
#else 
        uint32_t lo_threshold_f:8;
        uint32_t size_f:12;
        uint32_t start_addr_f:12;
#endif 
    } flds;
    uint32_t data;
} rxq_5_cache_t;



typedef union {
    struct rxq_5_cidx_update {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_5_cidx_update_t;



typedef union {
    struct rxq_5_cidx_update_cliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_5_cidx_update_cliff_t;



typedef union {
    struct rxq_5_cidx_update_precliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_5_cidx_update_precliff_t;



typedef union {
    struct rxq_5_cidx_update_prescaler {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_5_cidx_update_prescaler_t;



typedef union {
    struct rxq_5_cidx_update_tmr_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t restart_f:1;
        uint32_t enable_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t enable_f:1;
        uint32_t restart_f:1;
#endif 
    } flds;
    uint32_t data;
} rxq_5_cidx_update_tmr_ctl_t;



typedef union {
    struct rxq_5_cidx_wb_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} rxq_5_cidx_wb_hsn_hi_t;



typedef union {
    struct rxq_5_cidx_wb_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:2;
        uint32_t addr_f:30;
#else 
        uint32_t addr_f:30;
        uint32_t rsvd_f:2;
#endif 
    } flds;
    uint32_t data;
} rxq_5_cidx_wb_hsn_lo_t;



typedef union {
    struct rxq_5_desc_cidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_5_desc_cidx_t;



typedef union {
    struct rxq_5_desc_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_5_desc_pidx_t;



typedef union {
    struct rxq_5_desc_ring {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t size_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t size_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_5_desc_ring_t;



typedef union {
    struct rxq_5_sch {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} rxq_5_sch_t;



typedef union {
    struct rxq_cidx_intr_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t fld_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t fld_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_cidx_intr_0_t;



typedef union {
    struct rxq_cidx_intr_1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t fld_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t fld_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_cidx_intr_1_t;



typedef union {
    struct rxq_cidx_intr_2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t fld_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t fld_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_cidx_intr_2_t;



typedef union {
    struct rxq_cidx_intr_3 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t fld_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t fld_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_cidx_intr_3_t;



typedef union {
    struct rxq_cidx_intr_4 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t fld_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t fld_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_cidx_intr_4_t;



typedef union {
    struct rxq_cidx_intr_5 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t fld_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t fld_f:16;
#endif 
    } flds;
    uint32_t data;
} rxq_cidx_intr_5_t;



typedef union {
    struct sbuf_cpu_to_switch_ctrl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rst_n_f:1;
        uint32_t freeze_f:1;
        uint32_t flush_f:1;
        uint32_t en_f:1;
        uint32_t rsvd:28;
#else 
        uint32_t rsvd:28;
        uint32_t en_f:1;
        uint32_t flush_f:1;
        uint32_t freeze_f:1;
        uint32_t rst_n_f:1;
#endif 
    } flds;
    uint32_t data;
} sbuf_cpu_to_switch_ctrl_t;



typedef union {
    struct sbuf_switch_to_cpu_0_cnfg1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:16;
        uint32_t size_f:16;
#else 
        uint32_t size_f:16;
        uint32_t start_addr_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_0_cnfg1_t;



typedef union {
    struct sbuf_switch_to_cpu_0_cnfg2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t xoff_threshold_f:16;
        uint32_t xon_threshold_f:16;
#else 
        uint32_t xon_threshold_f:16;
        uint32_t xoff_threshold_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_0_cnfg2_t;



typedef union {
    struct sbuf_switch_to_cpu_0_ctrl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rst_n_f:1;
        uint32_t freeze_f:1;
        uint32_t flush_f:1;
        uint32_t en_f:1;
        uint32_t rsvd:28;
#else 
        uint32_t rsvd:28;
        uint32_t en_f:1;
        uint32_t flush_f:1;
        uint32_t freeze_f:1;
        uint32_t rst_n_f:1;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_0_ctrl_t;



typedef union {
    struct sbuf_switch_to_cpu_0_pkt {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t count_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t count_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_0_pkt_t;



typedef union {
    struct sbuf_switch_to_cpu_1_cnfg1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:16;
        uint32_t size_f:16;
#else 
        uint32_t size_f:16;
        uint32_t start_addr_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_1_cnfg1_t;



typedef union {
    struct sbuf_switch_to_cpu_1_cnfg2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t xoff_threshold_f:16;
        uint32_t xon_threshold_f:16;
#else 
        uint32_t xon_threshold_f:16;
        uint32_t xoff_threshold_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_1_cnfg2_t;



typedef union {
    struct sbuf_switch_to_cpu_1_ctrl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rst_n_f:1;
        uint32_t freeze_f:1;
        uint32_t flush_f:1;
        uint32_t en_f:1;
        uint32_t rsvd:28;
#else 
        uint32_t rsvd:28;
        uint32_t en_f:1;
        uint32_t flush_f:1;
        uint32_t freeze_f:1;
        uint32_t rst_n_f:1;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_1_ctrl_t;



typedef union {
    struct sbuf_switch_to_cpu_1_pkt {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t count_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t count_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_1_pkt_t;



typedef union {
    struct sbuf_switch_to_cpu_2_cnfg1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:16;
        uint32_t size_f:16;
#else 
        uint32_t size_f:16;
        uint32_t start_addr_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_2_cnfg1_t;



typedef union {
    struct sbuf_switch_to_cpu_2_cnfg2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t xoff_threshold_f:16;
        uint32_t xon_threshold_f:16;
#else 
        uint32_t xon_threshold_f:16;
        uint32_t xoff_threshold_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_2_cnfg2_t;



typedef union {
    struct sbuf_switch_to_cpu_2_ctrl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rst_n_f:1;
        uint32_t freeze_f:1;
        uint32_t flush_f:1;
        uint32_t en_f:1;
        uint32_t rsvd:28;
#else 
        uint32_t rsvd:28;
        uint32_t en_f:1;
        uint32_t flush_f:1;
        uint32_t freeze_f:1;
        uint32_t rst_n_f:1;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_2_ctrl_t;



typedef union {
    struct sbuf_switch_to_cpu_2_pkt {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t count_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t count_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_2_pkt_t;



typedef union {
    struct sbuf_switch_to_cpu_3_cnfg1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:16;
        uint32_t size_f:16;
#else 
        uint32_t size_f:16;
        uint32_t start_addr_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_3_cnfg1_t;



typedef union {
    struct sbuf_switch_to_cpu_3_cnfg2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t xoff_threshold_f:16;
        uint32_t xon_threshold_f:16;
#else 
        uint32_t xon_threshold_f:16;
        uint32_t xoff_threshold_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_3_cnfg2_t;



typedef union {
    struct sbuf_switch_to_cpu_3_ctrl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rst_n_f:1;
        uint32_t freeze_f:1;
        uint32_t flush_f:1;
        uint32_t en_f:1;
        uint32_t rsvd:28;
#else 
        uint32_t rsvd:28;
        uint32_t en_f:1;
        uint32_t flush_f:1;
        uint32_t freeze_f:1;
        uint32_t rst_n_f:1;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_3_ctrl_t;



typedef union {
    struct sbuf_switch_to_cpu_3_pkt {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t count_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t count_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_switch_to_cpu_3_pkt_t;



typedef union {
    struct sbuf_mcu_0_cnfg1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:16;
        uint32_t size_f:16;
#else 
        uint32_t size_f:16;
        uint32_t start_addr_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_mcu_0_cnfg1_t;



typedef union {
    struct sbuf_mcu_0_cnfg2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t xoff_threshold_f:16;
        uint32_t xon_threshold_f:16;
#else 
        uint32_t xon_threshold_f:16;
        uint32_t xoff_threshold_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_mcu_0_cnfg2_t;



typedef union {
    struct sbuf_mcu_0_ctrl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rst_n_f:1;
        uint32_t freeze_f:1;
        uint32_t flush_f:1;
        uint32_t en_f:1;
        uint32_t rsvd:28;
#else 
        uint32_t rsvd:28;
        uint32_t en_f:1;
        uint32_t flush_f:1;
        uint32_t freeze_f:1;
        uint32_t rst_n_f:1;
#endif 
    } flds;
    uint32_t data;
} sbuf_mcu_0_ctrl_t;



typedef union {
    struct sbuf_mcu_1_cnfg1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:16;
        uint32_t size_f:16;
#else 
        uint32_t size_f:16;
        uint32_t start_addr_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_mcu_1_cnfg1_t;



typedef union {
    struct sbuf_mcu_1_cnfg2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t xoff_threshold_f:16;
        uint32_t xon_threshold_f:16;
#else 
        uint32_t xon_threshold_f:16;
        uint32_t xoff_threshold_f:16;
#endif 
    } flds;
    uint32_t data;
} sbuf_mcu_1_cnfg2_t;



typedef union {
    struct sbuf_mcu_1_ctrl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rst_n_f:1;
        uint32_t freeze_f:1;
        uint32_t flush_f:1;
        uint32_t en_f:1;
        uint32_t rsvd:28;
#else 
        uint32_t rsvd:28;
        uint32_t en_f:1;
        uint32_t flush_f:1;
        uint32_t freeze_f:1;
        uint32_t rst_n_f:1;
#endif 
    } flds;
    uint32_t data;
} sbuf_mcu_1_ctrl_t;



typedef union {
    struct sch_quantum_rxe_cnfg0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t k_f:24;
        uint32_t rsvd:8;
#else 
        uint32_t rsvd:8;
        uint32_t k_f:24;
#endif 
    } flds;
    uint32_t data;
} sch_quantum_rxe_cnfg0_t;



typedef union {
    struct sch_quantum_rxe_cnfg1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t k_f:24;
        uint32_t rsvd:8;
#else 
        uint32_t rsvd:8;
        uint32_t k_f:24;
#endif 
    } flds;
    uint32_t data;
} sch_quantum_rxe_cnfg1_t;



typedef union {
    struct sch_quantum_rxe_cnfg2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t k_f:24;
        uint32_t rsvd:8;
#else 
        uint32_t rsvd:8;
        uint32_t k_f:24;
#endif 
    } flds;
    uint32_t data;
} sch_quantum_rxe_cnfg2_t;



typedef union {
    struct sch_quantum_txe_cnfg0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t k_f:24;
        uint32_t rsvd:8;
#else 
        uint32_t rsvd:8;
        uint32_t k_f:24;
#endif 
    } flds;
    uint32_t data;
} sch_quantum_txe_cnfg0_t;



typedef union {
    struct sch_quantum_txe_cnfg1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t k_f:24;
        uint32_t rsvd:8;
#else 
        uint32_t rsvd:8;
        uint32_t k_f:24;
#endif 
    } flds;
    uint32_t data;
} sch_quantum_txe_cnfg1_t;



typedef union {
    struct sch_quantum_txe_cnfg2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t k_f:24;
        uint32_t rsvd:8;
#else 
        uint32_t rsvd:8;
        uint32_t k_f:24;
#endif 
    } flds;
    uint32_t data;
} sch_quantum_txe_cnfg2_t;



typedef union {
    struct sts {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t txe_txq_en_0_f:1;
        uint32_t txe_txq_en_1_f:1;
        uint32_t txe_txq_en_2_f:1;
        uint32_t txe_txq_en_3_f:1;
        uint32_t txe_txq_en_4_f:1;
        uint32_t txe_txq_en_5_f:1;
        uint32_t txe_txq_en_6_f:1;
        uint32_t rxe_rxq_en_0_f:1;
        uint32_t rxe_rxq_en_1_f:1;
        uint32_t rxe_rxq_en_2_f:1;
        uint32_t rxe_rxq_en_3_f:1;
        uint32_t rxe_rxq_en_4_f:1;
        uint32_t rxe_rxq_en_5_f:1;
        uint32_t rsvd:19;
#else 
        uint32_t rsvd:19;
        uint32_t rxe_rxq_en_5_f:1;
        uint32_t rxe_rxq_en_4_f:1;
        uint32_t rxe_rxq_en_3_f:1;
        uint32_t rxe_rxq_en_2_f:1;
        uint32_t rxe_rxq_en_1_f:1;
        uint32_t rxe_rxq_en_0_f:1;
        uint32_t txe_txq_en_6_f:1;
        uint32_t txe_txq_en_5_f:1;
        uint32_t txe_txq_en_4_f:1;
        uint32_t txe_txq_en_3_f:1;
        uint32_t txe_txq_en_2_f:1;
        uint32_t txe_txq_en_1_f:1;
        uint32_t txe_txq_en_0_f:1;
#endif 
    } flds;
    uint32_t data;
} sts_t;



typedef union {
    struct tl10_sync_cmd_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cmd_f:3;
        uint32_t order_f:1;
        uint32_t cmd3_f:1;
        uint32_t gp_f:1;
        uint32_t intr_en_f:1;
        uint32_t btype_f:2;
        uint32_t length_f:10;
        uint32_t posted_wr_f:1;
        uint32_t seqnum_f:12;
#else 
        uint32_t seqnum_f:12;
        uint32_t posted_wr_f:1;
        uint32_t length_f:10;
        uint32_t btype_f:2;
        uint32_t intr_en_f:1;
        uint32_t gp_f:1;
        uint32_t cmd3_f:1;
        uint32_t order_f:1;
        uint32_t cmd_f:3;
#endif 
    } tl10_flds;
    struct tl_sync_cmd_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cmd_f:3;
        uint32_t order_f:1;
        uint32_t cmd3_f:1;
        uint32_t gp_f:1;
        uint32_t intr_en_f:1;
        uint32_t btype_f:2;
        uint32_t length_f:10;
        uint32_t rsvd1_f:1;
        uint32_t seqnum_f:12;
#else 
        uint32_t seqnum_f:12;
        uint32_t rsvd1_f:1;
        uint32_t length_f:10;
        uint32_t btype_f:2;
        uint32_t intr_en_f:1;
        uint32_t gp_f:1;
        uint32_t cmd3_f:1;
        uint32_t order_f:1;
        uint32_t cmd_f:3;
#endif 
    } tl_flds;
    uint32_t data;
} sync_cmd_0_t;



typedef union {
    struct sync_data_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t data_f:32;
#else 
        uint32_t data_f:32;
#endif 
    } flds;
    uint32_t data;
} sync_data_0_t;



typedef union {
    struct sync_hsn_adr_hi_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_hi_f:32;
#else 
        uint32_t adr_hi_f:32;
#endif 
    } flds;
    uint32_t data;
} sync_hsn_adr_hi_0_t;



typedef union {
    struct sync_hsn_adr_lo_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_lo_f:32;
#else 
        uint32_t adr_lo_f:32;
#endif 
    } flds;
    uint32_t data;
} sync_hsn_adr_lo_0_t;



typedef union {
    struct sync_hsn_adr_lo_3 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_lo_f:32;
#else 
        uint32_t adr_lo_f:32;
#endif 
    } flds;
    uint32_t data;
} sync_hsn_adr_lo_3_t;



typedef union {
    struct sync_isn_adr_hi_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_hi_f:32;
#else 
        uint32_t adr_hi_f:32;
#endif 
    } flds;
    uint32_t data;
} sync_isn_adr_hi_0_t;



typedef union {
    struct sync_isn_adr_lo_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t adr_lo_f:32;
#else 
        uint32_t adr_lo_f:32;
#endif 
    } flds;
    uint32_t data;
} sync_isn_adr_lo_0_t;



typedef union {
    struct sync_mode {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t host_rsp_mode_f:1;
        uint32_t async_err_mode_f:1;
        uint32_t async_strict_abort_f:1;
        uint32_t force_async_strict_f:1;
        uint32_t rsvd_f:28;
#else 
        uint32_t rsvd_f:28;
        uint32_t force_async_strict_f:1;
        uint32_t async_strict_abort_f:1;
        uint32_t async_err_mode_f:1;
        uint32_t host_rsp_mode_f:1;
#endif 
    } flds;
    uint32_t data;
} sync_mode_t;



typedef union {
    struct sync_sts_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t status_f:5;
        uint32_t istatus_f:3;
        uint32_t rsvd0_f:1;
        uint32_t length_f:10;
        uint32_t rsvd1_f:1;
        uint32_t seqnum_f:12;
#else 
        uint32_t seqnum_f:12;
        uint32_t rsvd1_f:1;
        uint32_t length_f:10;
        uint32_t rsvd0_f:1;
        uint32_t istatus_f:3;
        uint32_t status_f:5;
#endif 
    } flds;
    uint32_t data;
} sync_sts_0_t;



typedef union {
    struct txe_chnl_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t credits_f:16;
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:8;
#else 
        uint32_t rsvd:8;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
        uint32_t credits_f:16;
#endif 
    } flds;
    uint32_t data;
} txe_chnl_0_t;



typedef union {
    struct txe_chnl_1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t credits_f:16;
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:8;
#else 
        uint32_t rsvd:8;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
        uint32_t credits_f:16;
#endif 
    } flds;
    uint32_t data;
} txe_chnl_1_t;



typedef union {
    struct txe_chnl_2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t credits_f:16;
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:8;
#else 
        uint32_t rsvd:8;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
        uint32_t credits_f:16;
#endif 
    } flds;
    uint32_t data;
} txe_chnl_2_t;



typedef union {
    struct txe_chnl_3 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} txe_chnl_3_t;



typedef union {
    struct txq {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t dm_en_0_f:1;
        uint32_t txe_en_0_f:1;
        uint32_t dm_rst_n_0_f:1;
        uint32_t txe_rst_n_0_f:1;
        uint32_t dm_en_1_f:1;
        uint32_t txe_en_1_f:1;
        uint32_t dm_rst_n_1_f:1;
        uint32_t txe_rst_n_1_f:1;
        uint32_t dm_en_2_f:1;
        uint32_t txe_en_2_f:1;
        uint32_t dm_rst_n_2_f:1;
        uint32_t txe_rst_n_2_f:1;
        uint32_t dm_en_3_f:1;
        uint32_t txe_en_3_f:1;
        uint32_t dm_rst_n_3_f:1;
        uint32_t txe_rst_n_3_f:1;
        uint32_t dm_en_4_f:1;
        uint32_t txe_en_4_f:1;
        uint32_t dm_rst_n_4_f:1;
        uint32_t txe_rst_n_4_f:1;
        uint32_t dm_en_5_f:1;
        uint32_t txe_en_5_f:1;
        uint32_t dm_rst_n_5_f:1;
        uint32_t txe_rst_n_5_f:1;
        uint32_t dm_en_6_f:1;
        uint32_t txe_en_6_f:1;
        uint32_t dm_rst_n_6_f:1;
        uint32_t txe_rst_n_6_f:1;
        uint32_t rsvd_f:4;
#else 
        uint32_t rsvd_f:4;
        uint32_t txe_rst_n_6_f:1;
        uint32_t dm_rst_n_6_f:1;
        uint32_t txe_en_6_f:1;
        uint32_t dm_en_6_f:1;
        uint32_t txe_rst_n_5_f:1;
        uint32_t dm_rst_n_5_f:1;
        uint32_t txe_en_5_f:1;
        uint32_t dm_en_5_f:1;
        uint32_t txe_rst_n_4_f:1;
        uint32_t dm_rst_n_4_f:1;
        uint32_t txe_en_4_f:1;
        uint32_t dm_en_4_f:1;
        uint32_t txe_rst_n_3_f:1;
        uint32_t dm_rst_n_3_f:1;
        uint32_t txe_en_3_f:1;
        uint32_t dm_en_3_f:1;
        uint32_t txe_rst_n_2_f:1;
        uint32_t dm_rst_n_2_f:1;
        uint32_t txe_en_2_f:1;
        uint32_t dm_en_2_f:1;
        uint32_t txe_rst_n_1_f:1;
        uint32_t dm_rst_n_1_f:1;
        uint32_t txe_en_1_f:1;
        uint32_t dm_en_1_f:1;
        uint32_t txe_rst_n_0_f:1;
        uint32_t dm_rst_n_0_f:1;
        uint32_t txe_en_0_f:1;
        uint32_t dm_en_0_f:1;
#endif 
    } flds;
    uint32_t data;
} txq_t;



typedef union {
    struct txq_0 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cidx_wb_thres_f:16;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t dma_type_f:1;
        uint32_t chnl_map_f:2;
        uint32_t rsvd:12;
#else 
        uint32_t rsvd:12;
        uint32_t chnl_map_f:2;
        uint32_t dma_type_f:1;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t cidx_wb_thres_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_0_t;



typedef union {
    struct txq_0_base_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_0_base_hsn_hi_t;



typedef union {
    struct txq_0_base_hsn_hi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_0_base_hsn_hi_wb_t;



typedef union {
    struct txq_0_base_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} txq_0_base_hsn_lo_t;



typedef union {
    struct txq_0_base_hsn_lo_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} txq_0_base_hsn_lo_wb_t;



typedef union {
    struct txq_0_cache {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:12;
        uint32_t size_f:12;
        uint32_t lo_threshold_f:8;
#else 
        uint32_t lo_threshold_f:8;
        uint32_t size_f:12;
        uint32_t start_addr_f:12;
#endif 
    } flds;
    uint32_t data;
} txq_0_cache_t;



typedef union {
    struct txq_0_cidx_update {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_0_cidx_update_t;



typedef union {
    struct txq_0_cidx_update_cliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_0_cidx_update_cliff_t;



typedef union {
    struct txq_0_cidx_update_precliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_0_cidx_update_precliff_t;



typedef union {
    struct txq_0_cidx_update_prescaler {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_0_cidx_update_prescaler_t;



typedef union {
    struct txq_0_cidx_update_tmr_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t restart_f:1;
        uint32_t enable_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t enable_f:1;
        uint32_t restart_f:1;
#endif 
    } flds;
    uint32_t data;
} txq_0_cidx_update_tmr_ctl_t;



typedef union {
    struct txq_0_cidx_wb_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_0_cidx_wb_hsn_hi_t;



typedef union {
    struct txq_0_cidx_wb_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:2;
        uint32_t addr_f:30;
#else 
        uint32_t addr_f:30;
        uint32_t rsvd_f:2;
#endif 
    } flds;
    uint32_t data;
} txq_0_cidx_wb_hsn_lo_t;



typedef union {
    struct txq_0_desc_cidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_0_desc_cidx_t;



typedef union {
    struct txq_0_desc_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_0_desc_pidx_t;



typedef union {
    struct txq_0_desc_ring {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t size_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t size_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_0_desc_ring_t;



typedef union {
    struct txq_0_sch {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} txq_0_sch_t;



typedef union {
    struct txq_1 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cidx_wb_thres_f:16;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t dma_type_f:1;
        uint32_t chnl_map_f:2;
        uint32_t rsvd:12;
#else 
        uint32_t rsvd:12;
        uint32_t chnl_map_f:2;
        uint32_t dma_type_f:1;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t cidx_wb_thres_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_1_t;



typedef union {
    struct txq_1_base_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_1_base_hsn_hi_t;



typedef union {
    struct txq_1_base_hsn_hi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_1_base_hsn_hi_wb_t;



typedef union {
    struct txq_1_base_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} txq_1_base_hsn_lo_t;



typedef union {
    struct txq_1_base_hsn_lo_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} txq_1_base_hsn_lo_wb_t;



typedef union {
    struct txq_1_cache {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:12;
        uint32_t size_f:12;
        uint32_t lo_threshold_f:8;
#else 
        uint32_t lo_threshold_f:8;
        uint32_t size_f:12;
        uint32_t start_addr_f:12;
#endif 
    } flds;
    uint32_t data;
} txq_1_cache_t;



typedef union {
    struct txq_1_cidx_update {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_1_cidx_update_t;



typedef union {
    struct txq_1_cidx_update_cliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_1_cidx_update_cliff_t;



typedef union {
    struct txq_1_cidx_update_precliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_1_cidx_update_precliff_t;



typedef union {
    struct txq_1_cidx_update_prescaler {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_1_cidx_update_prescaler_t;



typedef union {
    struct txq_1_cidx_update_tmr_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t restart_f:1;
        uint32_t enable_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t enable_f:1;
        uint32_t restart_f:1;
#endif 
    } flds;
    uint32_t data;
} txq_1_cidx_update_tmr_ctl_t;



typedef union {
    struct txq_1_cidx_wb_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_1_cidx_wb_hsn_hi_t;



typedef union {
    struct txq_1_cidx_wb_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:2;
        uint32_t addr_f:30;
#else 
        uint32_t addr_f:30;
        uint32_t rsvd_f:2;
#endif 
    } flds;
    uint32_t data;
} txq_1_cidx_wb_hsn_lo_t;



typedef union {
    struct txq_1_desc_cidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_1_desc_cidx_t;



typedef union {
    struct txq_1_desc_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_1_desc_pidx_t;



typedef union {
    struct txq_1_desc_ring {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t size_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t size_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_1_desc_ring_t;



typedef union {
    struct txq_1_sch {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} txq_1_sch_t;



typedef union {
    struct txq_2 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cidx_wb_thres_f:16;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t dma_type_f:1;
        uint32_t chnl_map_f:2;
        uint32_t rsvd:12;
#else 
        uint32_t rsvd:12;
        uint32_t chnl_map_f:2;
        uint32_t dma_type_f:1;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t cidx_wb_thres_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_2_t;



typedef union {
    struct txq_2_base_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_2_base_hsn_hi_t;



typedef union {
    struct txq_2_base_hsn_hi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_2_base_hsn_hi_wb_t;



typedef union {
    struct txq_2_base_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} txq_2_base_hsn_lo_t;



typedef union {
    struct txq_2_base_hsn_lo_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} txq_2_base_hsn_lo_wb_t;



typedef union {
    struct txq_2_cache {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:12;
        uint32_t size_f:12;
        uint32_t lo_threshold_f:8;
#else 
        uint32_t lo_threshold_f:8;
        uint32_t size_f:12;
        uint32_t start_addr_f:12;
#endif 
    } flds;
    uint32_t data;
} txq_2_cache_t;



typedef union {
    struct txq_2_cidx_update {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_2_cidx_update_t;



typedef union {
    struct txq_2_cidx_update_cliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_2_cidx_update_cliff_t;



typedef union {
    struct txq_2_cidx_update_precliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_2_cidx_update_precliff_t;



typedef union {
    struct txq_2_cidx_update_prescaler {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_2_cidx_update_prescaler_t;



typedef union {
    struct txq_2_cidx_update_tmr_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t restart_f:1;
        uint32_t enable_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t enable_f:1;
        uint32_t restart_f:1;
#endif 
    } flds;
    uint32_t data;
} txq_2_cidx_update_tmr_ctl_t;



typedef union {
    struct txq_2_cidx_wb_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_2_cidx_wb_hsn_hi_t;



typedef union {
    struct txq_2_cidx_wb_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:2;
        uint32_t addr_f:30;
#else 
        uint32_t addr_f:30;
        uint32_t rsvd_f:2;
#endif 
    } flds;
    uint32_t data;
} txq_2_cidx_wb_hsn_lo_t;



typedef union {
    struct txq_2_desc_cidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_2_desc_cidx_t;



typedef union {
    struct txq_2_desc_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_2_desc_pidx_t;



typedef union {
    struct txq_2_desc_ring {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t size_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t size_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_2_desc_ring_t;



typedef union {
    struct txq_2_sch {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} txq_2_sch_t;



typedef union {
    struct txq_3 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cidx_wb_thres_f:16;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t dma_type_f:1;
        uint32_t chnl_map_f:2;
        uint32_t rsvd:12;
#else 
        uint32_t rsvd:12;
        uint32_t chnl_map_f:2;
        uint32_t dma_type_f:1;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t cidx_wb_thres_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_3_t;



typedef union {
    struct txq_3_base_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_3_base_hsn_hi_t;



typedef union {
    struct txq_3_base_hsn_hi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_3_base_hsn_hi_wb_t;



typedef union {
    struct txq_3_base_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} txq_3_base_hsn_lo_t;



typedef union {
    struct txq_3_base_hsn_lo_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} txq_3_base_hsn_lo_wb_t;



typedef union {
    struct txq_3_cache {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:12;
        uint32_t size_f:12;
        uint32_t lo_threshold_f:8;
#else 
        uint32_t lo_threshold_f:8;
        uint32_t size_f:12;
        uint32_t start_addr_f:12;
#endif 
    } flds;
    uint32_t data;
} txq_3_cache_t;



typedef union {
    struct txq_3_cidx_update {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_3_cidx_update_t;



typedef union {
    struct txq_3_cidx_update_cliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_3_cidx_update_cliff_t;



typedef union {
    struct txq_3_cidx_update_precliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_3_cidx_update_precliff_t;



typedef union {
    struct txq_3_cidx_update_prescaler {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_3_cidx_update_prescaler_t;



typedef union {
    struct txq_3_cidx_update_tmr_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t restart_f:1;
        uint32_t enable_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t enable_f:1;
        uint32_t restart_f:1;
#endif 
    } flds;
    uint32_t data;
} txq_3_cidx_update_tmr_ctl_t;



typedef union {
    struct txq_3_cidx_wb_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_3_cidx_wb_hsn_hi_t;



typedef union {
    struct txq_3_cidx_wb_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:2;
        uint32_t addr_f:30;
#else 
        uint32_t addr_f:30;
        uint32_t rsvd_f:2;
#endif 
    } flds;
    uint32_t data;
} txq_3_cidx_wb_hsn_lo_t;



typedef union {
    struct txq_3_desc_cidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_3_desc_cidx_t;



typedef union {
    struct txq_3_desc_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_3_desc_pidx_t;



typedef union {
    struct txq_3_desc_ring {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t size_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t size_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_3_desc_ring_t;



typedef union {
    struct txq_3_sch {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} txq_3_sch_t;



typedef union {
    struct txq_4 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cidx_wb_thres_f:16;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t dma_type_f:1;
        uint32_t chnl_map_f:2;
        uint32_t rsvd:12;
#else 
        uint32_t rsvd:12;
        uint32_t chnl_map_f:2;
        uint32_t dma_type_f:1;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t cidx_wb_thres_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_4_t;



typedef union {
    struct txq_4_base_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_4_base_hsn_hi_t;



typedef union {
    struct txq_4_base_hsn_hi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_4_base_hsn_hi_wb_t;



typedef union {
    struct txq_4_base_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} txq_4_base_hsn_lo_t;



typedef union {
    struct txq_4_base_hsn_lo_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} txq_4_base_hsn_lo_wb_t;



typedef union {
    struct txq_4_cache {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:12;
        uint32_t size_f:12;
        uint32_t lo_threshold_f:8;
#else 
        uint32_t lo_threshold_f:8;
        uint32_t size_f:12;
        uint32_t start_addr_f:12;
#endif 
    } flds;
    uint32_t data;
} txq_4_cache_t;



typedef union {
    struct txq_4_cidx_update {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_4_cidx_update_t;



typedef union {
    struct txq_4_cidx_update_cliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_4_cidx_update_cliff_t;



typedef union {
    struct txq_4_cidx_update_precliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_4_cidx_update_precliff_t;



typedef union {
    struct txq_4_cidx_update_prescaler {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_4_cidx_update_prescaler_t;



typedef union {
    struct txq_4_cidx_update_tmr_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t restart_f:1;
        uint32_t enable_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t enable_f:1;
        uint32_t restart_f:1;
#endif 
    } flds;
    uint32_t data;
} txq_4_cidx_update_tmr_ctl_t;



typedef union {
    struct txq_4_cidx_wb_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_4_cidx_wb_hsn_hi_t;



typedef union {
    struct txq_4_cidx_wb_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:2;
        uint32_t addr_f:30;
#else 
        uint32_t addr_f:30;
        uint32_t rsvd_f:2;
#endif 
    } flds;
    uint32_t data;
} txq_4_cidx_wb_hsn_lo_t;



typedef union {
    struct txq_4_desc_cidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_4_desc_cidx_t;



typedef union {
    struct txq_4_desc_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_4_desc_pidx_t;



typedef union {
    struct txq_4_desc_ring {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t size_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t size_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_4_desc_ring_t;



typedef union {
    struct txq_4_sch {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} txq_4_sch_t;



typedef union {
    struct txq_5 {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t cidx_wb_thres_f:16;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t dma_type_f:1;
        uint32_t chnl_map_f:2;
        uint32_t rsvd:12;
#else 
        uint32_t rsvd:12;
        uint32_t chnl_map_f:2;
        uint32_t dma_type_f:1;
        uint32_t cidx_intr_cnt_coalse_en_f:1;
        uint32_t cidx_wb_thres_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_5_t;



typedef union {
    struct txq_5_base_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_5_base_hsn_hi_t;



typedef union {
    struct txq_5_base_hsn_hi_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_5_base_hsn_hi_wb_t;



typedef union {
    struct txq_5_base_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} txq_5_base_hsn_lo_t;



typedef union {
    struct txq_5_base_hsn_lo_wb {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:6;
        uint32_t addr_f:26;
#else 
        uint32_t addr_f:26;
        uint32_t rsvd_f:6;
#endif 
    } flds;
    uint32_t data;
} txq_5_base_hsn_lo_wb_t;



typedef union {
    struct txq_5_cache {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t start_addr_f:12;
        uint32_t size_f:12;
        uint32_t lo_threshold_f:8;
#else 
        uint32_t lo_threshold_f:8;
        uint32_t size_f:12;
        uint32_t start_addr_f:12;
#endif 
    } flds;
    uint32_t data;
} txq_5_cache_t;



typedef union {
    struct txq_5_cidx_update {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_5_cidx_update_t;



typedef union {
    struct txq_5_cidx_update_cliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_5_cidx_update_cliff_t;



typedef union {
    struct txq_5_cidx_update_precliff {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_5_cidx_update_precliff_t;



typedef union {
    struct txq_5_cidx_update_prescaler {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t timer_f:32;
#else 
        uint32_t timer_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_5_cidx_update_prescaler_t;



typedef union {
    struct txq_5_cidx_update_tmr_ctl {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t restart_f:1;
        uint32_t enable_f:1;
        uint32_t rsvd:30;
#else 
        uint32_t rsvd:30;
        uint32_t enable_f:1;
        uint32_t restart_f:1;
#endif 
    } flds;
    uint32_t data;
} txq_5_cidx_update_tmr_ctl_t;



typedef union {
    struct txq_5_cidx_wb_hsn_hi {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t addr_f:32;
#else 
        uint32_t addr_f:32;
#endif 
    } flds;
    uint32_t data;
} txq_5_cidx_wb_hsn_hi_t;



typedef union {
    struct txq_5_cidx_wb_hsn_lo {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t rsvd_f:2;
        uint32_t addr_f:30;
#else 
        uint32_t addr_f:30;
        uint32_t rsvd_f:2;
#endif 
    } flds;
    uint32_t data;
} txq_5_cidx_wb_hsn_lo_t;



typedef union {
    struct txq_5_desc_cidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_5_desc_cidx_t;



typedef union {
    struct txq_5_desc_pidx {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t num_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t num_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_5_desc_pidx_t;



typedef union {
    struct txq_5_desc_ring {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t size_f:16;
        uint32_t rsvd:16;
#else 
        uint32_t rsvd:16;
        uint32_t size_f:16;
#endif 
    } flds;
    uint32_t data;
} txq_5_desc_ring_t;



typedef union {
    struct txq_5_sch {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t weight_f:7;
        uint32_t strict_priority_f:1;
        uint32_t rsvd:24;
#else 
        uint32_t rsvd:24;
        uint32_t strict_priority_f:1;
        uint32_t weight_f:7;
#endif 
    } flds;
    uint32_t data;
} txq_5_sch_t;



typedef union {
    struct ucm_dtm {
#if PCI_STRUCT_FIELD_ORDER_LO_HI
        uint32_t sw_force_xoff_f:6;
        uint32_t sw_force_xoff_en_f:6;
        uint32_t rsvd:20;
#else 
        uint32_t rsvd:20;
        uint32_t sw_force_xoff_en_f:6;
        uint32_t sw_force_xoff_f:6;
#endif 
    } flds;
    uint32_t data;
} ucm_dtm_t;



static inline uint32_t RXQ_x(int index)
{
    switch (index) {
        case 0: return(RXQ_0);
        case 1: return(RXQ_1);
        case 2: return(RXQ_2);
        case 3: return(RXQ_3);
        case 4: return(RXQ_4);
        case 5: return(RXQ_5);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_BASE_HSN_HI(int index)
{
    switch (index) {
        case 0: return(RXQ_0_BASE_HSN_HI);
        case 1: return(RXQ_1_BASE_HSN_HI);
        case 2: return(RXQ_2_BASE_HSN_HI);
        case 3: return(RXQ_3_BASE_HSN_HI);
        case 4: return(RXQ_4_BASE_HSN_HI);
        case 5: return(RXQ_5_BASE_HSN_HI);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_BASE_HSN_HI_WB(int index)
{
    switch (index) {
        case 0: return(RXQ_0_BASE_HSN_HI_WB);
        case 1: return(RXQ_1_BASE_HSN_HI_WB);
        case 2: return(RXQ_2_BASE_HSN_HI_WB);
        case 3: return(RXQ_3_BASE_HSN_HI_WB);
        case 4: return(RXQ_4_BASE_HSN_HI_WB);
        case 5: return(RXQ_5_BASE_HSN_HI_WB);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_BASE_HSN_LO(int index)
{
    switch (index) {
        case 0: return(RXQ_0_BASE_HSN_LO);
        case 1: return(RXQ_1_BASE_HSN_LO);
        case 2: return(RXQ_2_BASE_HSN_LO);
        case 3: return(RXQ_3_BASE_HSN_LO);
        case 4: return(RXQ_4_BASE_HSN_LO);
        case 5: return(RXQ_5_BASE_HSN_LO);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_BASE_HSN_LO_WB(int index)
{
    switch (index) {
        case 0: return(RXQ_0_BASE_HSN_LO_WB);
        case 1: return(RXQ_1_BASE_HSN_LO_WB);
        case 2: return(RXQ_2_BASE_HSN_LO_WB);
        case 3: return(RXQ_3_BASE_HSN_LO_WB);
        case 4: return(RXQ_4_BASE_HSN_LO_WB);
        case 5: return(RXQ_5_BASE_HSN_LO_WB);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_CACHE(int index)
{
    switch (index) {
        case 0: return(RXQ_0_CACHE);
        case 1: return(RXQ_1_CACHE);
        case 2: return(RXQ_2_CACHE);
        case 3: return(RXQ_3_CACHE);
        case 4: return(RXQ_4_CACHE);
        case 5: return(RXQ_5_CACHE);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_CIDX_UPDATE(int index)
{
    switch (index) {
        case 0: return(RXQ_0_CIDX_UPDATE);
        case 1: return(RXQ_1_CIDX_UPDATE);
        case 2: return(RXQ_2_CIDX_UPDATE);
        case 3: return(RXQ_3_CIDX_UPDATE);
        case 4: return(RXQ_4_CIDX_UPDATE);
        case 5: return(RXQ_5_CIDX_UPDATE);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_CIDX_UPDATE_CLIFF(int index)
{
    switch (index) {
        case 0: return(RXQ_0_CIDX_UPDATE_CLIFF);
        case 1: return(RXQ_1_CIDX_UPDATE_CLIFF);
        case 2: return(RXQ_2_CIDX_UPDATE_CLIFF);
        case 3: return(RXQ_3_CIDX_UPDATE_CLIFF);
        case 4: return(RXQ_4_CIDX_UPDATE_CLIFF);
        case 5: return(RXQ_5_CIDX_UPDATE_CLIFF);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_CIDX_UPDATE_PRECLIFF(int index)
{
    switch (index) {
        case 0: return(RXQ_0_CIDX_UPDATE_PRECLIFF);
        case 1: return(RXQ_1_CIDX_UPDATE_PRECLIFF);
        case 2: return(RXQ_2_CIDX_UPDATE_PRECLIFF);
        case 3: return(RXQ_3_CIDX_UPDATE_PRECLIFF);
        case 4: return(RXQ_4_CIDX_UPDATE_PRECLIFF);
        case 5: return(RXQ_5_CIDX_UPDATE_PRECLIFF);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_CIDX_UPDATE_PRESCALER(int index)
{
    switch (index) {
        case 0: return(RXQ_0_CIDX_UPDATE_PRESCALER);
        case 1: return(RXQ_1_CIDX_UPDATE_PRESCALER);
        case 2: return(RXQ_2_CIDX_UPDATE_PRESCALER);
        case 3: return(RXQ_3_CIDX_UPDATE_PRESCALER);
        case 4: return(RXQ_4_CIDX_UPDATE_PRESCALER);
        case 5: return(RXQ_5_CIDX_UPDATE_PRESCALER);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_CIDX_UPDATE_TMR_CTL(int index)
{
    switch (index) {
        case 0: return(RXQ_0_CIDX_UPDATE_TMR_CTL);
        case 1: return(RXQ_1_CIDX_UPDATE_TMR_CTL);
        case 2: return(RXQ_2_CIDX_UPDATE_TMR_CTL);
        case 3: return(RXQ_3_CIDX_UPDATE_TMR_CTL);
        case 4: return(RXQ_4_CIDX_UPDATE_TMR_CTL);
        case 5: return(RXQ_5_CIDX_UPDATE_TMR_CTL);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_CIDX_WB_HSN_HI(int index)
{
    switch (index) {
        case 0: return(RXQ_0_CIDX_WB_HSN_HI);
        case 1: return(RXQ_1_CIDX_WB_HSN_HI);
        case 2: return(RXQ_2_CIDX_WB_HSN_HI);
        case 3: return(RXQ_3_CIDX_WB_HSN_HI);
        case 4: return(RXQ_4_CIDX_WB_HSN_HI);
        case 5: return(RXQ_5_CIDX_WB_HSN_HI);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_CIDX_WB_HSN_LO(int index)
{
    switch (index) {
        case 0: return(RXQ_0_CIDX_WB_HSN_LO);
        case 1: return(RXQ_1_CIDX_WB_HSN_LO);
        case 2: return(RXQ_2_CIDX_WB_HSN_LO);
        case 3: return(RXQ_3_CIDX_WB_HSN_LO);
        case 4: return(RXQ_4_CIDX_WB_HSN_LO);
        case 5: return(RXQ_5_CIDX_WB_HSN_LO);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_DESC_CIDX(int index)
{
    switch (index) {
        case 0: return(RXQ_0_DESC_CIDX);
        case 1: return(RXQ_1_DESC_CIDX);
        case 2: return(RXQ_2_DESC_CIDX);
        case 3: return(RXQ_3_DESC_CIDX);
        case 4: return(RXQ_4_DESC_CIDX);
        case 5: return(RXQ_5_DESC_CIDX);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_DESC_PIDX(int index)
{
    switch (index) {
        case 0: return(RXQ_0_DESC_PIDX);
        case 1: return(RXQ_1_DESC_PIDX);
        case 2: return(RXQ_2_DESC_PIDX);
        case 3: return(RXQ_3_DESC_PIDX);
        case 4: return(RXQ_4_DESC_PIDX);
        case 5: return(RXQ_5_DESC_PIDX);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_DESC_RING(int index)
{
    switch (index) {
        case 0: return(RXQ_0_DESC_RING);
        case 1: return(RXQ_1_DESC_RING);
        case 2: return(RXQ_2_DESC_RING);
        case 3: return(RXQ_3_DESC_RING);
        case 4: return(RXQ_4_DESC_RING);
        case 5: return(RXQ_5_DESC_RING);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_x_SCH(int index)
{
    switch (index) {
        case 0: return(RXQ_0_SCH);
        case 1: return(RXQ_1_SCH);
        case 2: return(RXQ_2_SCH);
        case 3: return(RXQ_3_SCH);
        case 4: return(RXQ_4_SCH);
        case 5: return(RXQ_5_SCH);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t RXQ_CIDX_INTR_x(int index)
{
    switch (index) {
        case 0: return(RXQ_CIDX_INTR_0);
        case 1: return(RXQ_CIDX_INTR_1);
        case 2: return(RXQ_CIDX_INTR_2);
        case 3: return(RXQ_CIDX_INTR_3);
        case 4: return(RXQ_CIDX_INTR_4);
        case 5: return(RXQ_CIDX_INTR_5);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t SBUF_SWITCH_TO_CPU_x_CNFG1(int index)
{    switch (index) {
        case 0: return(SBUF_SWITCH_TO_CPU_0_CNFG1);
        case 1: return(SBUF_SWITCH_TO_CPU_1_CNFG1);
        case 2: return(SBUF_SWITCH_TO_CPU_2_CNFG1);
        case 3: return(SBUF_SWITCH_TO_CPU_3_CNFG1);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t SBUF_SWITCH_TO_CPU_x_CNFG2(int index)
{    switch (index) {
        case 0: return(SBUF_SWITCH_TO_CPU_0_CNFG2);
        case 1: return(SBUF_SWITCH_TO_CPU_1_CNFG2);
        case 2: return(SBUF_SWITCH_TO_CPU_2_CNFG2);
        case 3: return(SBUF_SWITCH_TO_CPU_3_CNFG2);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t SBUF_SWITCH_TO_CPU_x_CTRL(int index)
{    switch (index) {
        case 0: return(SBUF_SWITCH_TO_CPU_0_CTRL);
        case 1: return(SBUF_SWITCH_TO_CPU_1_CTRL);
        case 2: return(SBUF_SWITCH_TO_CPU_2_CTRL);
        case 3: return(SBUF_SWITCH_TO_CPU_3_CTRL);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t SBUF_SWITCH_TO_CPU_x_PKT(int index)
{    switch (index) {
        case 0: return(SBUF_SWITCH_TO_CPU_0_PKT);
        case 1: return(SBUF_SWITCH_TO_CPU_1_PKT);
        case 2: return(SBUF_SWITCH_TO_CPU_2_PKT);
        case 3: return(SBUF_SWITCH_TO_CPU_3_PKT);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t SBUF_MCU_x_CNFG1(int index)
{    switch (index) {
        case 0: return(SBUF_MCU_0_CNFG1);
        case 1: return(SBUF_MCU_1_CNFG1);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t SBUF_MCU_x_CNFG2(int index)
{    switch (index) {
        case 0: return(SBUF_MCU_0_CNFG2);
        case 1: return(SBUF_MCU_1_CNFG2);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t SBUF_MCU_x_CTRL(int index)
{    switch (index) {
        case 0: return(SBUF_MCU_0_CTRL);
        case 1: return(SBUF_MCU_1_CTRL);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXE_CHNL_x(int index)
{
    switch (index) {
        case 0: return(TXE_CHNL_0);
        case 1: return(TXE_CHNL_1);
        case 2: return(TXE_CHNL_2);
        case 3: return(TXE_CHNL_3);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x(int index)
{
    switch (index) {
        case 0: return(TXQ_0);
        case 1: return(TXQ_1);
        case 2: return(TXQ_2);
        case 3: return(TXQ_3);
        case 4: return(TXQ_4);
        case 5: return(TXQ_5);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_BASE_HSN_HI(int index)
{
    switch (index) {
        case 0: return(TXQ_0_BASE_HSN_HI);
        case 1: return(TXQ_1_BASE_HSN_HI);
        case 2: return(TXQ_2_BASE_HSN_HI);
        case 3: return(TXQ_3_BASE_HSN_HI);
        case 4: return(TXQ_4_BASE_HSN_HI);
        case 5: return(TXQ_5_BASE_HSN_HI);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_BASE_HSN_HI_WB(int index)
{
    switch (index) {
        case 0: return(TXQ_0_BASE_HSN_HI_WB);
        case 1: return(TXQ_1_BASE_HSN_HI_WB);
        case 2: return(TXQ_2_BASE_HSN_HI_WB);
        case 3: return(TXQ_3_BASE_HSN_HI_WB);
        case 4: return(TXQ_4_BASE_HSN_HI_WB);
        case 5: return(TXQ_5_BASE_HSN_HI_WB);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_BASE_HSN_LO(int index)
{
    switch (index) {
        case 0: return(TXQ_0_BASE_HSN_LO);
        case 1: return(TXQ_1_BASE_HSN_LO);
        case 2: return(TXQ_2_BASE_HSN_LO);
        case 3: return(TXQ_3_BASE_HSN_LO);
        case 4: return(TXQ_4_BASE_HSN_LO);
        case 5: return(TXQ_5_BASE_HSN_LO);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_BASE_HSN_LO_WB(int index)
{
    switch (index) {
        case 0: return(TXQ_0_BASE_HSN_LO_WB);
        case 1: return(TXQ_1_BASE_HSN_LO_WB);
        case 2: return(TXQ_2_BASE_HSN_LO_WB);
        case 3: return(TXQ_3_BASE_HSN_LO_WB);
        case 4: return(TXQ_4_BASE_HSN_LO_WB);
        case 5: return(TXQ_5_BASE_HSN_LO_WB);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_CACHE(int index)
{
    switch (index) {
        case 0: return(TXQ_0_CACHE);
        case 1: return(TXQ_1_CACHE);
        case 2: return(TXQ_2_CACHE);
        case 3: return(TXQ_3_CACHE);
        case 4: return(TXQ_4_CACHE);
        case 5: return(TXQ_5_CACHE);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_CIDX_UPDATE(int index)
{
    switch (index) {
        case 0: return(TXQ_0_CIDX_UPDATE);
        case 1: return(TXQ_1_CIDX_UPDATE);
        case 2: return(TXQ_2_CIDX_UPDATE);
        case 3: return(TXQ_3_CIDX_UPDATE);
        case 4: return(TXQ_4_CIDX_UPDATE);
        case 5: return(TXQ_5_CIDX_UPDATE);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_CIDX_UPDATE_CLIFF(int index)
{
    switch (index) {
        case 0: return(TXQ_0_CIDX_UPDATE_CLIFF);
        case 1: return(TXQ_1_CIDX_UPDATE_CLIFF);
        case 2: return(TXQ_2_CIDX_UPDATE_CLIFF);
        case 3: return(TXQ_3_CIDX_UPDATE_CLIFF);
        case 4: return(TXQ_4_CIDX_UPDATE_CLIFF);
        case 5: return(TXQ_5_CIDX_UPDATE_CLIFF);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_CIDX_UPDATE_PRECLIFF(int index)
{
    switch (index) {
        case 0: return(TXQ_0_CIDX_UPDATE_PRECLIFF);
        case 1: return(TXQ_1_CIDX_UPDATE_PRECLIFF);
        case 2: return(TXQ_2_CIDX_UPDATE_PRECLIFF);
        case 3: return(TXQ_3_CIDX_UPDATE_PRECLIFF);
        case 4: return(TXQ_4_CIDX_UPDATE_PRECLIFF);
        case 5: return(TXQ_5_CIDX_UPDATE_PRECLIFF);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_CIDX_UPDATE_PRESCALER(int index)
{
    switch (index) {
        case 0: return(TXQ_0_CIDX_UPDATE_PRESCALER);
        case 1: return(TXQ_1_CIDX_UPDATE_PRESCALER);
        case 2: return(TXQ_2_CIDX_UPDATE_PRESCALER);
        case 3: return(TXQ_3_CIDX_UPDATE_PRESCALER);
        case 4: return(TXQ_4_CIDX_UPDATE_PRESCALER);
        case 5: return(TXQ_5_CIDX_UPDATE_PRESCALER);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_CIDX_UPDATE_TMR_CTL(int index)
{
    switch (index) {
        case 0: return(TXQ_0_CIDX_UPDATE_TMR_CTL);
        case 1: return(TXQ_1_CIDX_UPDATE_TMR_CTL);
        case 2: return(TXQ_2_CIDX_UPDATE_TMR_CTL);
        case 3: return(TXQ_3_CIDX_UPDATE_TMR_CTL);
        case 4: return(TXQ_4_CIDX_UPDATE_TMR_CTL);
        case 5: return(TXQ_5_CIDX_UPDATE_TMR_CTL);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_CIDX_WB_HSN_HI(int index)
{
    switch (index) {
        case 0: return(TXQ_0_CIDX_WB_HSN_HI);
        case 1: return(TXQ_1_CIDX_WB_HSN_HI);
        case 2: return(TXQ_2_CIDX_WB_HSN_HI);
        case 3: return(TXQ_3_CIDX_WB_HSN_HI);
        case 4: return(TXQ_4_CIDX_WB_HSN_HI);
        case 5: return(TXQ_5_CIDX_WB_HSN_HI);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_CIDX_WB_HSN_LO(int index)
{
    switch (index) {
        case 0: return(TXQ_0_CIDX_WB_HSN_LO);
        case 1: return(TXQ_1_CIDX_WB_HSN_LO);
        case 2: return(TXQ_2_CIDX_WB_HSN_LO);
        case 3: return(TXQ_3_CIDX_WB_HSN_LO);
        case 4: return(TXQ_4_CIDX_WB_HSN_LO);
        case 5: return(TXQ_5_CIDX_WB_HSN_LO);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_DESC_CIDX(int index)
{
    switch (index) {
        case 0: return(TXQ_0_DESC_CIDX);
        case 1: return(TXQ_1_DESC_CIDX);
        case 2: return(TXQ_2_DESC_CIDX);
        case 3: return(TXQ_3_DESC_CIDX);
        case 4: return(TXQ_4_DESC_CIDX);
        case 5: return(TXQ_5_DESC_CIDX);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_DESC_PIDX(int index)
{
    switch (index) {
        case 0: return(TXQ_0_DESC_PIDX);
        case 1: return(TXQ_1_DESC_PIDX);
        case 2: return(TXQ_2_DESC_PIDX);
        case 3: return(TXQ_3_DESC_PIDX);
        case 4: return(TXQ_4_DESC_PIDX);
        case 5: return(TXQ_5_DESC_PIDX);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_DESC_RING(int index)
{
    switch (index) {
        case 0: return(TXQ_0_DESC_RING);
        case 1: return(TXQ_1_DESC_RING);
        case 2: return(TXQ_2_DESC_RING);
        case 3: return(TXQ_3_DESC_RING);
        case 4: return(TXQ_4_DESC_RING);
        case 5: return(TXQ_5_DESC_RING);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}

static inline uint32_t TXQ_x_SCH(int index)
{
    switch (index) {
        case 0: return(TXQ_0_SCH);
        case 1: return(TXQ_1_SCH);
        case 2: return(TXQ_2_SCH);
        case 3: return(TXQ_3_SCH);
        case 4: return(TXQ_4_SCH);
        case 5: return(TXQ_5_SCH);
#ifdef __KERNEL__
        default: return -1;
#else
        default: assert(0);
#endif 
    }
}


#endif 
