/**************************************************************************
*
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2005-2011 by Andrey Butok. Freescale Semiconductor, Inc.
*
***************************************************************************
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License Version 3 
* or later (the "LGPL").
*
* As a special exception, the copyright holders of the FNET project give you
* permission to link the FNET sources with independent modules to produce an
* executable, regardless of the license terms of these independent modules,
* and to copy and distribute the resulting executable under terms of your 
* choice, provided that you also meet, for each linked independent module,
* the terms and conditions of the license of that module.
* An independent module is a module which is not derived from or based 
* on this library. 
* If you modify the FNET sources, you may extend this exception 
* to your version of the FNET sources, but you are not obligated 
* to do so. If you do not wish to do so, delete this
* exception statement from your version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License
* and the GNU Lesser General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*
**********************************************************************/ /*!
*
* @file fnet_stm32_mac.h
*
* @author EvdZ and inca
*
* @date Jun-23-2013
*
* @version 0.0
*
* @brief STM32 MAC (FEC equivalent) module driver definitions.
*
***************************************************************************/

#include "fnet_config.h"
#if (FNET_STM32) && (FNET_CFG_CPU_ETH0 ||FNET_CFG_CPU_ETH1)

#include "fnet.h"
#include "fnet_eth_prv.h"
#include "fnet_error.h"
#include "fnet_debug.h"
#include "fnet_isr.h"
#include "fnet_prot.h"
#include "fnet_arp.h"
#include "fnet_timer_prv.h"
#include "fnet_loop.h"

#include "fnet_stdlib.h"


/* CPU-specific configuration.*/

#if FNET_MK     /* Kinetis.*/
    #define FNET_FEC_CLOCK_KHZ  FNET_MK_PERIPH_CLOCK_KHZ
    /* Transmit buffer descriptor queue. This pointer 
     * must be 32-bit aligned; however, it is recommended it be 
     * made 128-bit aligned (evenly divisible by 16).*/
    #define FNET_FEC_BUF_DESC_DIV       (16)
    /* The transmit buffer pointer, containing the address 
     * of the associated data buffer, 
     * must always be evenly divisible by 8.*/
    #define FNET_FEC_TX_BUF_DIV         (8)
    /* The receive buffer pointer, containing the address 
     * of the associated data buffer, 
     * must always be evenly divisible by 16.*/
    #define FNET_FEC_RX_BUF_DIV         (16)
#endif   


/* frequency of less than or equal to 2.5 MHz to be compliant with 
* the IEEE 802.3 MII specification. */
#define FNET_FEC_MII_CLOCK_KHZ      (2500)


#define FNET_FEC_BUF_SIZE           (((FNET_CFG_CPU_ETH0_MTU>FNET_CFG_CPU_ETH1_MTU)?FNET_CFG_CPU_ETH0_MTU:FNET_CFG_CPU_ETH1_MTU)+FNET_ETH_HDR_SIZE+FNET_ETH_CRC_SIZE+16) /* Ring Buffer sizes in bytes.*/
#define FNET_FEC_TX_BUF_NUM         (FNET_CFG_CPU_ETH_TX_BUFS_MAX)
#define FNET_FEC_RX_BUF_NUM         (FNET_CFG_CPU_ETH_RX_BUFS_MAX)


/************************************************************************
*     MII Register Indexes.
*************************************************************************/
#define FNET_FEC_MII_REG_CR          (0x0000)   /* Control Register */
#define FNET_FEC_MII_REG_SR          (0x0001)   /* Status Register */
#define FNET_FEC_MII_REG_IDR1        (0x0002)   /* Identification Register #1 */
#define FNET_FEC_MII_REG_IDR2        (0x0003)   /* Identification Register #2 */
#define FNET_FEC_MII_REG_ANAR        (0x0004)   /* Auto-Negotiation Advertisement Register */
#define FNET_FEC_MII_REG_ANLPAR      (0x0005)   /* Auto-Negotiation Link Partner Ability Register */
#define FNET_FEC_MII_REG_ANER        (0x0006)   /* Auto-Negotiation Expansion Register */
#define FNET_FEC_MII_REG_ANNPTR      (0x0007)   /* Auto-Negotiation Next Page TX Register */
#define FNET_FEC_MII_REG_ICR         (0x0010)   /* Interrupt Control Register */
#define FNET_FEC_MII_REG_PSR         (0x0011)   /* Proprietary Status Register */
#define FNET_FEC_MII_REG_PCR         (0x0012)   /* Proprietary Control Register */

#define FNET_FEC_MII_REG_SR_LINK_STATUS (0x0004)
#define FNET_FEC_MII_REG_SR_AN_ABILITY  (0x0008)
#define FNET_FEC_MII_REG_SR_AN_COMPLETE (0x0020)

#define FNET_FEC_MII_REG_ANAR_NEXT_PAGE (0x8000)

#define FNET_FEC_MII_REG_CR_RESET       (0x8000)    /* Resetting a port is accomplished by setting this bit to 1.*/
#define FNET_FEC_MII_REG_CR_LOOPBACK    (0x4000)    /* Determines Digital Loopback Mode. */
#define FNET_FEC_MII_REG_CR_DATARATE    (0x2000)    /* Speed Selection bit.*/
#define FNET_FEC_MII_REG_CR_ANE         (0x1000)    /* Auto-Negotiation Enable bit. */
#define FNET_FEC_MII_REG_CR_PDWN        (0x0800)    /* Power Down bit. */
#define FNET_FEC_MII_REG_CR_ISOL        (0x0400)    /* Isolate bit.*/
#define FNET_FEC_MII_REG_CR_ANE_RESTART (0x0200)    /* Restart Auto-Negotiation bit.*/
#define FNET_FEC_MII_REG_CR_DPLX        (0x0100)    /* Duplex Mode bit.*/

#define FNET_FEC_MII_TIMEOUT            (0x10000)   /* Timeout counter for MII communications.*/

/************************************************************************
*     FEC registers.
*************************************************************************/
typedef struct
{                    /* Detailed Memory Map (Control/Status Registers)*/
    fnet_uint32 EIR;                /* Interrupt even reg. */
    fnet_uint32 EIMR;               /* Interrupt mask reg. */
    fnet_uint32 reserved1[1]; 
    fnet_uint32 RDAR;               /* Receive descriptor active reg.*/
    fnet_uint32 TDAR;               /* Transmit descriptor active reg.*/
    fnet_uint32 reserved2[3];
    fnet_uint32 ECR;                /* Ethernet control reg.*/
    fnet_uint32 reserved3[6];
    fnet_uint32 MMFR;               /* MII management frame reg.*/
    fnet_uint32 MSCR;               /* MII speed control reg.*/
    fnet_uint32 reserved4[7];
#if FNET_CFG_CPU_ETH_MIB    
    fnet_uint32 MIBC;               /* MIB Control/Status Register.*/
#else
    fnet_uint32 reserved4_1;
#endif        
    fnet_uint32 reserved5[7];
    fnet_uint32 RCR;                /* Receive control reg.*/
    fnet_uint32 reserved6[15];
    fnet_uint32 TCR;                /* Transmit Control reg.*/
    fnet_uint32 reserved7[7];
    fnet_uint32 PALR;               /* Lower 32-bits of MAC address.*/
    fnet_uint32 PAUR;               /* Upper 16-bits of MAC address.*/
    fnet_uint32 OPD;                /* Opcode + Pause Duration.*/
    fnet_uint32 reserved8[10];
    fnet_uint32 IAUR;               /* Upper 32-bits of individual hash table.*/
    fnet_uint32 IALR;               /* Lower 32-bits of individual hash table.*/
    fnet_uint32 GAUR;               /* Upper 32-bits of group hash table.*/
    fnet_uint32 GALR;               /* Lower 32-bits of group hash table.*/
    fnet_uint32 reserved9[7];
    fnet_uint32 TFWR;               /* FIFO transmit water mark.*/
#if FNET_MK
    fnet_uint32 reserved10[14];
#else /* MCF || MPC */  
    fnet_uint32 reserved10[1];
    fnet_uint32 FRBR;               /* FIFO receive bound reg.*/
    fnet_uint32 FRSR;               /* FIFO receive start reg.*/
    fnet_uint32 reserved11[11];
#endif    
    fnet_uint32 ERDSR;              /* Pointer to receive descriptor ring.*/
    fnet_uint32 ETDSR;              /* Pointer to transmit descriptor ring.*/
    fnet_uint32 EMRBR;              /* Maximum receive buffer size.*/

#if FNET_MK || FNET_CFG_CPU_MCF54418   
    fnet_uint32 reserved12[1];
    fnet_uint32 RSFL;               /* Receive FIFO Section Full Threshold, offset: 0x190 */
    fnet_uint32 RSEM;               /* Receive FIFO Section Empty Threshold, offset: 0x194 */
    fnet_uint32 RAEM;               /* Receive FIFO Almost Empty Threshold, offset: 0x198 */
    fnet_uint32 RAFL;               /* Receive FIFO Almost Full Threshold, offset: 0x19C */
    fnet_uint32 TSEM;               /* Transmit FIFO Section Empty Threshold, offset: 0x1A0 */
    fnet_uint32 TAEM;               /* Transmit FIFO Almost Empty Threshold, offset: 0x1A4 */
    fnet_uint32 TAFL;               /* Transmit FIFO Almost Full Threshold, offset: 0x1A8 */
    fnet_uint32 TIPG;               /* Transmit Inter-Packet Gap, offset: 0x1AC */
    fnet_uint32 FTRL;               /* Frame Truncation Length, offset: 0x1B0 */
    fnet_uint32 reserved13[3];
    fnet_uint32 TACC;               /* Transmit Accelerator Function Configuration, offset: 0x1C0 */
    fnet_uint32 RACC;               /* Receive Accelerator Function Configuration, offset: 0x1C4 */
    fnet_uint32 reserved14[14];    
#else /* MCF || MPC */
    fnet_uint32 reserved12[29];
#endif    
    
#if FNET_CFG_CPU_ETH_MIB 
    /* Ethernet Management Information Base (MIB) Block Counters:*/
    fnet_uint32 RMON_T_DROP;
    fnet_uint32 RMON_T_PACKETS;
    fnet_uint32 RMON_T_BC_PKT;
    fnet_uint32 RMON_T_MC_PKT;
    fnet_uint32 RMON_T_CRC_ALIGN;
    fnet_uint32 RMON_T_UNDERSIZE;
    fnet_uint32 RMON_T_OVERSIZE;
    fnet_uint32 RMON_T_FRAG;
    fnet_uint32 RMON_T_JAB;
    fnet_uint32 RMON_T_COL;
    fnet_uint32 RMON_T_P64;
    fnet_uint32 RMON_T_P65TO127;
    fnet_uint32 RMON_T_P128TO255;
    fnet_uint32 RMON_T_P256TO511;
    fnet_uint32 RMON_T_P512TO1023;
    fnet_uint32 RMON_T_P1024TO2047;
    fnet_uint32 RMON_T_P_GTE2048;
    fnet_uint32 RMON_T_OCTETS;
    fnet_uint32 IEEE_T_DROP;
    fnet_uint32 IEEE_T_FRAME_OK;
    fnet_uint32 IEEE_T_1COL;
    fnet_uint32 IEEE_T_MCOL;
    fnet_uint32 IEEE_T_DEF;
    fnet_uint32 IEEE_T_LCOL;
    fnet_uint32 IEEE_T_EXCOL;
    fnet_uint32 IEEE_T_MACERR;
    fnet_uint32 IEEE_T_CSERR;
    fnet_uint32 IEEE_T_SQE;
    fnet_uint32 IEEE_T_FDXFC;
    fnet_uint32 IEEE_T_OCTETS_OK;
    fnet_uint32 reserved15[3];
    fnet_uint32 RMON_R_PACKETS;
    fnet_uint32 RMON_R_BC_PKT;
    fnet_uint32 RMON_R_MC_PKT;
    fnet_uint32 RMON_R_CRC_ALIGN;
    fnet_uint32 RMON_R_UNDERSIZE;
    fnet_uint32 RMON_R_OVERSIZE;
    fnet_uint32 RMON_R_FRAG;
    fnet_uint32 RMON_R_JAB;
    fnet_uint32 RMON_R_RESVD_0;
    fnet_uint32 RMON_R_P64;
    fnet_uint32 RMON_R_P65TO127;
    fnet_uint32 RMON_R_P128TO255;
    fnet_uint32 RMON_R_P256TO511;
    fnet_uint32 RMON_R_512TO1023;
    fnet_uint32 RMON_R_1024TO2047;
    fnet_uint32 RMON_R_P_GTE2048;
    fnet_uint32 RMON_R_OCTETS;
    fnet_uint32 IEEE_R_DROP;
    fnet_uint32 IEEE_R_FRAME_OK;
    fnet_uint32 IEEE_R_CRC;
    fnet_uint32 IEEE_R_ALIGN;
    fnet_uint32 IEEE_R_MACERR;
    fnet_uint32 IEEE_R_FDXFC;
    fnet_uint32 IEEE_R_OCTETS_OK;
#endif
#if 0 /* Not used. Present in Kinetis, Modelo.*/
    fnet_uint32 RESERVED_16[71];
    fnet_uint32 ATCR;                   /* Timer Control Register, offset: 0x400 */
    fnet_uint32 ATVR;                   /* Timer Value Register, offset: 0x404 */
    fnet_uint32 ATOFF;                  /* Timer Offset Register, offset: 0x408 */
    fnet_uint32 ATPER;                  /* Timer Period Register, offset: 0x40C */
    fnet_uint32 ATCOR;                  /* Timer Correction Register, offset: 0x410 */
    fnet_uint32 ATINC;                  /* Time-Stamping Clock Period Register, offset: 0x414 */
    fnet_uint32 ATSTMP;                 /* Timestamp of Last Transmitted Frame, offset: 0x418 */
    fnet_uint32 RESERVED_17[122];
    fnet_uint32 TGSR;                   /* Timer Global Status Register, offset: 0x604 */
    struct 
    {                                   /* offset: 0x608, array step: 0x8 */
        fnet_uint32 TCSR;               /*!< Timer Control Status Register, array offset: 0x608, array step: 0x8 */
        fnet_uint32 TCCR;               /*!< Timer Compare Capture Register, array offset: 0x60C, array step: 0x8 */
    } CHANNEL[4];
#endif    
}
fnet_fec_reg_t;

/* Bit definitions and macros for FNET_FEC_EIR */
#define FNET_FEC_EIR_UN                  (0x00080000)
#define FNET_FEC_EIR_RL                  (0x00100000)
#define FNET_FEC_EIR_LC                  (0x00200000)
#define FNET_FEC_EIR_EBERR               (0x00400000)
#define FNET_FEC_EIR_MII                 (0x00800000)
#define FNET_FEC_EIR_RXB                 (0x01000000)
#define FNET_FEC_EIR_RXF                 (0x02000000)
#define FNET_FEC_EIR_TXB                 (0x04000000)
#define FNET_FEC_EIR_TXF                 (0x08000000)
#define FNET_FEC_EIR_GRA                 (0x10000000)
#define FNET_FEC_EIR_BABT                (0x20000000)
#define FNET_FEC_EIR_BABR                (0x40000000)
#define FNET_FEC_EIR_HBERR               (0x80000000)

/* Bit definitions and macros for FNET_FEC_EIMR */
#define FNET_FEC_EIMR_UN                 (0x00080000)
#define FNET_FEC_EIMR_RL                 (0x00100000)
#define FNET_FEC_EIMR_LC                 (0x00200000)
#define FNET_FEC_EIMR_EBERR              (0x00400000)
#define FNET_FEC_EIMR_MII                (0x00800000)
#define FNET_FEC_EIMR_RXB                (0x01000000)
#define FNET_FEC_EIMR_RXF                (0x02000000)
#define FNET_FEC_EIMR_TXB                (0x04000000)
#define FNET_FEC_EIMR_TXF                (0x08000000)
#define FNET_FEC_EIMR_GRA                (0x10000000)
#define FNET_FEC_EIMR_BABT               (0x20000000)
#define FNET_FEC_EIMR_BABR               (0x40000000)
#define FNET_FEC_EIMR_HBERR              (0x80000000)

/* Bit definitions and macros for FNET_FEC_RDAR */
#define FNET_FEC_RDAR_R_DES_ACTIVE       (0x01000000)

/* Bit definitions and macros for FNET_MCF_FEC_TDAR */
#define FNET_FEC_TDAR_X_DES_ACTIVE       (0x01000000)

/* Bit definitions and macros for FNET_MCF_FEC_ECR */
#define FNET_FEC_ECR_RESET               (0x00000001)
#define FNET_FEC_ECR_ETHER_EN            (0x00000002)

/* Bit definitions and macros for FNET_MCF_FEC_MMFR */
#define FNET_FEC_MMFR_DATA(x)            (((x)&0x0000FFFF)<<0)
#define FNET_FEC_MMFR_TA(x)              (((x)&0x00000003)<<16)
#define FNET_FEC_MMFR_RA(x)              (((x)&0x0000001F)<<18)
#define FNET_FEC_MMFR_PA(x)              (((x)&0x0000001F)<<23)
#define FNET_FEC_MMFR_OP(x)              (((x)&0x00000003)<<28)
#define FNET_FEC_MMFR_ST(x)              (((x)&0x00000003)<<30)
#define FNET_FEC_MMFR_ST_01              (0x40000000)
#define FNET_FEC_MMFR_OP_READ            (0x20000000)
#define FNET_FEC_MMFR_OP_WRITE           (0x10000000)
#define FNET_FEC_MMFR_TA_10              (0x00020000)


/* Bit definitions and macros for FNET_MCF_FEC_MSCR */
#define FNET_FEC_MSCR_MII_SPEED(x)       (((x)&0x0000003F)<<1)
#define FNET_FEC_MSCR_DIS_PREAMBLE       (0x00000080)

/* Bit definitions and macros for FNET_MCF_FEC_MIBC */
#define FNET_FEC_MIBC_MIB_IDLE           (0x40000000)
#define FNET_FEC_MIBC_MIB_DISABLE        (0x80000000)

/* Bit definitions and macros for FNET_MCF_FEC_RCR */
#define FNET_FEC_RCR_LOOP                (0x00000001)
#define FNET_FEC_RCR_DRT                 (0x00000002)
#define FNET_FEC_RCR_MII_MODE            (0x00000004)
#define FNET_FEC_RCR_PROM                (0x00000008)
#define FNET_FEC_RCR_BC_REJ              (0x00000010)
#define FNET_FEC_RCR_FCE                 (0x00000020)
#define FNET_FEC_RCR_MAX_FL(x)           (((x)&0x000007FF)<<16)
#define FNET_FEC_RCR_RMII_MODE           (0x00000100)
#define FNET_FEC_RCR_RMII_10T            (0x00000200)

/* Bit definitions and macros for FNET_MCF_FEC_TCR */
#define FNET_FEC_TCR_GTS                 (0x00000001)
#define FNET_FEC_TCR_HBC                 (0x00000002)
#define FNET_FEC_TCR_FDEN                (0x00000004)
#define FNET_FEC_TCR_TFC_PAUSE           (0x00000008)
#define FNET_FEC_TCR_RFC_PAUSE           (0x00000010)

/* Bit definitions and macros for FNET_MCF_FEC_PAUR */
#define FNET_FEC_PAUR_TYPE(x)           (((x)&0x0000FFFF)<<0)
#define FNET_FEC_PAUR_PADDR2(x)         (((x)&0x0000FFFF)<<16)

/* Bit definitions and macros for FNET_MCF_FEC_OPD */
#define FNET_FEC_OPD_PAUSE_DUR(x)       (((x)&0x0000FFFF)<<0)
#define FNET_FEC_OPD_OPCODE(x)          (((x)&0x0000FFFF)<<16)

/* Bit definitions and macros for FNET_MCF_FEC_TFWR */
#define FNET_FEC_TFWR_X_WMRK(x)         (((x)&0x00000003)<<0)

/* Bit definitions and macros for FNET_MCF_FEC_FRBR */
#define FNET_FEC_FRBR_R_BOUND(x)        (((x)&0x000000FF)<<2)

/* Bit definitions and macros for FNET_MCF_FEC_FRSR */
#define FNET_FEC_FRSR_R_FSTART(x)       (((x)&0x000000FF)<<2)

/* Bit definitions and macros for FNET_MCF_FEC_ERDSR */
#define FNET_FEC_ERDSR_R_DES_START(x)   (((x)&0x3FFFFFFF)<<2)

/* Bit definitions and macros for FNET_MCF_FEC_ETDSR */
#define FNET_FEC_ETDSR_X_DES_START(x)   (((x)&0x3FFFFFFF)<<2)

/* Bit definitions and macros for FNET_MCF_FEC_EMRBR */
#define FNET_FEC_EMRBR_R_BUF_SIZE(x)    (((x)&0x0000007F)<<4)

/*  Bit definitions and macros for TACC */
#define FNET_FEC_TACC_SHIFT16           (0x00000001)
#define FNET_FEC_TACC_IPCHK             (0x00000008)
#define FNET_FEC_TACC_PROCHK            (0x00000010)

/* Bit definitions and macros for RACC */
#define FNET_FEC_RACC_PADREM            (0x00000001)
#define FNET_FEC_RACC_IPDIS             (0x00000002)
#define FNET_FEC_RACC_PRODIS            (0x00000004)
#define FNET_FEC_RACC_LINEDIS           (0x00000040)
#define FNET_FEC_RACC_SHIFT16           (0x00000080)


/*  Bit definitions and macros for TFWR */
#define FNET_FEC_TFWR_STRFWD             (0x00000100)

/************************************************************************
*     Ethernet Buffer Descriptor definitions (status field)
*************************************************************************/
#define FNET_FEC_RX_BD_E        (0x8000)
#define FNET_FEC_RX_BD_R01      (0x4000)
#define FNET_FEC_RX_BD_W        (0x2000)
#define FNET_FEC_RX_BD_R02      (0x1000)
#define FNET_FEC_RX_BD_L        (0x0800)
#define FNET_FEC_RX_BD_M        (0x0100)
#define FNET_FEC_RX_BD_BC       (0x0080)
#define FNET_FEC_RX_BD_MC       (0x0040)
#define FNET_FEC_RX_BD_LG       (0x0020)
#define FNET_FEC_RX_BD_NO       (0x0010)
#define FNET_FEC_RX_BD_SH       (0x0008)
#define FNET_FEC_RX_BD_CR       (0x0004)
#define FNET_FEC_RX_BD_OV       (0x0002)
#define FNET_FEC_RX_BD_TR       (0x0001)

#define FNET_FEC_TX_BD_R        (0x8000)
#define FNET_FEC_TX_BD_TO1      (0x4000)
#define FNET_FEC_TX_BD_W        (0x2000)
#define FNET_FEC_TX_BD_TO2      (0x1000)
#define FNET_FEC_TX_BD_L        (0x0800)
#define FNET_FEC_TX_BD_TC       (0x0400)
#define FNET_FEC_TX_BD_DEF      (0x0200)
#define FNET_FEC_TX_BD_HB       (0x0100)
#define FNET_FEC_TX_BD_LC       (0x0080)
#define FNET_FEC_TX_BD_RL       (0x0040)
#define FNET_FEC_TX_BD_UN       (0x0002)
#define FNET_FEC_TX_BD_CSL      (0x0001)



/* Ethernet Buffer descriptor */
FNET_COMP_PACKED_BEGIN
typedef struct
{
    volatile unsigned short status FNET_COMP_PACKED;     /* Control and status info.*/
    unsigned short length FNET_COMP_PACKED;              /* Data length.*/
    unsigned char *buf_ptr FNET_COMP_PACKED;             /* Buffer pointer.*/
}
fnet_fec_buf_desc_t;
FNET_COMP_PACKED_END

/* FEC/ENET Module Control data structure */
typedef struct
{
    volatile fnet_fec_reg_t  *reg;               /* Pointer to the eth registers. */
    volatile fnet_fec_reg_t  *reg_phy;           /* Pointer to the eth registers, used for comunication with phy. */
    unsigned int             vector_number;      /* Vector number of the Ethernet Receive Frame interrupt.*/
    fnet_fec_buf_desc_t      *tx_buf_desc;       /* Tx Buffer Descriptors.*/
    fnet_fec_buf_desc_t      *tx_buf_desc_cur;   /* Points to the descriptor of the current outcoming buffer.*/
    fnet_fec_buf_desc_t      *rx_buf_desc;       /* Rx Buffer Descriptors.*/
    fnet_fec_buf_desc_t      *rx_buf_desc_cur;   /* Points to the descriptor of the current incoming buffer.*/
    unsigned char            phy_addr;
    unsigned char            tx_buf_desc_num;    /* Number of allocated Tx Buffer Descriptors.*/    
    unsigned char            rx_buf_desc_num;    /* Number of allocated Tx Buffer Descriptors.*/  
#if FNET_CFG_MULTICAST    
    fnet_uint32              GALR_double;
    fnet_uint32              GAUR_double;
#endif
    fnet_uint8 tx_buf_desc_buf[(FNET_FEC_TX_BUF_NUM * sizeof(fnet_fec_buf_desc_t)) + (FNET_FEC_BUF_DESC_DIV-1)];
    fnet_uint8 rx_buf_desc_buf[(FNET_FEC_RX_BUF_NUM * sizeof(fnet_fec_buf_desc_t)) + (FNET_FEC_BUF_DESC_DIV-1)];
    fnet_uint8 tx_buf[FNET_FEC_TX_BUF_NUM][FNET_FEC_BUF_SIZE + (FNET_FEC_TX_BUF_DIV-1)];
    fnet_uint8 rx_buf[FNET_FEC_RX_BUF_NUM][FNET_FEC_BUF_SIZE + (FNET_FEC_RX_BUF_DIV-1)];    
}
fnet_fec_if_t;

/* FEC driver API */
extern const fnet_netif_api_t fnet_fec_api;
/* Ethernet specific control data structure.*/
#if FNET_CFG_CPU_ETH0
    extern fnet_fec_if_t fnet_fec0_if;
#endif
#if FNET_CFG_CPU_ETH1
    extern fnet_fec_if_t fnet_fec1_if;
#endif

/************************************************************************
*     Function Prototypes
*************************************************************************/
int fnet_stm32_init(fnet_netif_t *netif);
void fnet_stm32_release(fnet_netif_t *netif);
void fnet_stm32_input(fnet_netif_t *netif);
int fnet_stm32_get_hw_addr(fnet_netif_t *netif, unsigned char * hw_addr);
int fnet_stm32_set_hw_addr(fnet_netif_t *netif, unsigned char * hw_addr);
int fnet_stm32_is_connected(fnet_netif_t *netif);
int fnet_stm32_get_statistics(struct fnet_netif *netif, struct fnet_netif_statistics * statistics);
void fnet_stm32_output(fnet_netif_t *netif, unsigned short type, const fnet_mac_addr_t dest_addr, fnet_netbuf_t* nb);

/* Ethernet IO initialization.*/
void fnet_eth_io_init(void) ;
/* Ethernet On-chip Physical Transceiver initialization and/or reset. */
void fnet_eth_phy_init(fnet_fec_if_t *ethif);

int fnet_stm32_mii_write(fnet_fec_if_t *ethif, int reg_addr, fnet_uint16 data);
int fnet_stm32_mii_read(fnet_fec_if_t *ethif, int reg_addr, fnet_uint16 *data); 

#if FNET_CFG_MULTICAST      
void fnet_stm32_multicast_join(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
void fnet_stm32_multicast_leave(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
#endif /* FNET_CFG_MULTICAST */

/* For debug needs.*/
void fnet_stm32_output_frame(fnet_netif_t *netif, char* frame, int frame_size);
int fnet_stm32_input_frame(fnet_netif_t *netif, char* buf, int buf_size);    
void fnet_stm32_debug_mii_print_regs(fnet_netif_t *netif);
void fnet_stm32_stop(fnet_netif_t *netif);
void fnet_stm32_resume(fnet_netif_t *netif);


#endif /* (FNET_STM32) && FNET_CFG_ETH */



