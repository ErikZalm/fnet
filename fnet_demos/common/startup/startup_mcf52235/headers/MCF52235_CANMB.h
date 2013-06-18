/* Coldfire C Header File
 * Copyright Freescale Semiconductor Inc
 * All rights reserved.
 *
 * 2008/05/23 Revision: 0.95
 *
 * (c) Copyright UNIS, a.s. 1997-2008
 * UNIS, a.s.
 * Jundrovska 33
 * 624 00 Brno
 * Czech Republic
 * http      : www.processorexpert.com
 * mail      : info@processorexpert.com
 */

#ifndef __MCF52235_CANMB_H__
#define __MCF52235_CANMB_H__


/*********************************************************************
*
* Flex Controller Area Network Module (FlexCAN) message buffers
*
*********************************************************************/

/* Register read/write macros */
#define MCF_CANMB_CODE0                      (*(vuint8 *)(0x401C0080))
#define MCF_CANMB_CTRL0                      (*(vuint8 *)(0x401C0081))
#define MCF_CANMB_TIME0                      (*(vuint16*)(0x401C0082))
#define MCF_CANMB_ID0                        (*(vuint32*)(0x401C0084))
#define MCF_CANMB_DATA_WORD_1_0              (*(vuint16*)(0x401C0088))
#define MCF_CANMB_DATA_WORD_2_0              (*(vuint16*)(0x401C008A))
#define MCF_CANMB_DATA_WORD_3_0              (*(vuint16*)(0x401C008C))
#define MCF_CANMB_DATA_WORD_4_0              (*(vuint16*)(0x401C008E))
#define MCF_CANMB_CODE1                      (*(vuint8 *)(0x401C0090))
#define MCF_CANMB_CTRL1                      (*(vuint8 *)(0x401C0091))
#define MCF_CANMB_TIME1                      (*(vuint16*)(0x401C0092))
#define MCF_CANMB_ID1                        (*(vuint32*)(0x401C0094))
#define MCF_CANMB_DATA_WORD_1_1              (*(vuint16*)(0x401C0098))
#define MCF_CANMB_DATA_WORD_2_1              (*(vuint16*)(0x401C009A))
#define MCF_CANMB_DATA_WORD_3_1              (*(vuint16*)(0x401C009C))
#define MCF_CANMB_DATA_WORD_4_1              (*(vuint16*)(0x401C009E))
#define MCF_CANMB_CODE2                      (*(vuint8 *)(0x401C00A0))
#define MCF_CANMB_CTRL2                      (*(vuint8 *)(0x401C00A1))
#define MCF_CANMB_TIME2                      (*(vuint16*)(0x401C00A2))
#define MCF_CANMB_ID2                        (*(vuint32*)(0x401C00A4))
#define MCF_CANMB_DATA_WORD_1_2              (*(vuint16*)(0x401C00A8))
#define MCF_CANMB_DATA_WORD_2_2              (*(vuint16*)(0x401C00AA))
#define MCF_CANMB_DATA_WORD_3_2              (*(vuint16*)(0x401C00AC))
#define MCF_CANMB_DATA_WORD_4_2              (*(vuint16*)(0x401C00AE))
#define MCF_CANMB_CODE3                      (*(vuint8 *)(0x401C00B0))
#define MCF_CANMB_CTRL3                      (*(vuint8 *)(0x401C00B1))
#define MCF_CANMB_TIME3                      (*(vuint16*)(0x401C00B2))
#define MCF_CANMB_ID3                        (*(vuint32*)(0x401C00B4))
#define MCF_CANMB_DATA_WORD_1_3              (*(vuint16*)(0x401C00B8))
#define MCF_CANMB_DATA_WORD_2_3              (*(vuint16*)(0x401C00BA))
#define MCF_CANMB_DATA_WORD_3_3              (*(vuint16*)(0x401C00BC))
#define MCF_CANMB_DATA_WORD_4_3              (*(vuint16*)(0x401C00BE))
#define MCF_CANMB_CODE4                      (*(vuint8 *)(0x401C00C0))
#define MCF_CANMB_CTRL4                      (*(vuint8 *)(0x401C00C1))
#define MCF_CANMB_TIME4                      (*(vuint16*)(0x401C00C2))
#define MCF_CANMB_ID4                        (*(vuint32*)(0x401C00C4))
#define MCF_CANMB_DATA_WORD_1_4              (*(vuint16*)(0x401C00C8))
#define MCF_CANMB_DATA_WORD_2_4              (*(vuint16*)(0x401C00CA))
#define MCF_CANMB_DATA_WORD_3_4              (*(vuint16*)(0x401C00CC))
#define MCF_CANMB_DATA_WORD_4_4              (*(vuint16*)(0x401C00CE))
#define MCF_CANMB_CODE5                      (*(vuint8 *)(0x401C00D0))
#define MCF_CANMB_CTRL5                      (*(vuint8 *)(0x401C00D1))
#define MCF_CANMB_TIME5                      (*(vuint16*)(0x401C00D2))
#define MCF_CANMB_ID5                        (*(vuint32*)(0x401C00D4))
#define MCF_CANMB_DATA_WORD_1_5              (*(vuint16*)(0x401C00D8))
#define MCF_CANMB_DATA_WORD_2_5              (*(vuint16*)(0x401C00DA))
#define MCF_CANMB_DATA_WORD_3_5              (*(vuint16*)(0x401C00DC))
#define MCF_CANMB_DATA_WORD_4_5              (*(vuint16*)(0x401C00DE))
#define MCF_CANMB_CODE6                      (*(vuint8 *)(0x401C00E0))
#define MCF_CANMB_CTRL6                      (*(vuint8 *)(0x401C00E1))
#define MCF_CANMB_TIME6                      (*(vuint16*)(0x401C00E2))
#define MCF_CANMB_ID6                        (*(vuint32*)(0x401C00E4))
#define MCF_CANMB_DATA_WORD_1_6              (*(vuint16*)(0x401C00E8))
#define MCF_CANMB_DATA_WORD_2_6              (*(vuint16*)(0x401C00EA))
#define MCF_CANMB_DATA_WORD_3_6              (*(vuint16*)(0x401C00EC))
#define MCF_CANMB_DATA_WORD_4_6              (*(vuint16*)(0x401C00EE))
#define MCF_CANMB_CODE7                      (*(vuint8 *)(0x401C00F0))
#define MCF_CANMB_CTRL7                      (*(vuint8 *)(0x401C00F1))
#define MCF_CANMB_TIME7                      (*(vuint16*)(0x401C00F2))
#define MCF_CANMB_ID7                        (*(vuint32*)(0x401C00F4))
#define MCF_CANMB_DATA_WORD_1_7              (*(vuint16*)(0x401C00F8))
#define MCF_CANMB_DATA_WORD_2_7              (*(vuint16*)(0x401C00FA))
#define MCF_CANMB_DATA_WORD_3_7              (*(vuint16*)(0x401C00FC))
#define MCF_CANMB_DATA_WORD_4_7              (*(vuint16*)(0x401C00FE))
#define MCF_CANMB_CODE8                      (*(vuint8 *)(0x401C0100))
#define MCF_CANMB_CTRL8                      (*(vuint8 *)(0x401C0101))
#define MCF_CANMB_TIME8                      (*(vuint16*)(0x401C0102))
#define MCF_CANMB_ID8                        (*(vuint32*)(0x401C0104))
#define MCF_CANMB_DATA_WORD_1_8              (*(vuint16*)(0x401C0108))
#define MCF_CANMB_DATA_WORD_2_8              (*(vuint16*)(0x401C010A))
#define MCF_CANMB_DATA_WORD_3_8              (*(vuint16*)(0x401C010C))
#define MCF_CANMB_DATA_WORD_4_8              (*(vuint16*)(0x401C010E))
#define MCF_CANMB_CODE9                      (*(vuint8 *)(0x401C0110))
#define MCF_CANMB_CTRL9                      (*(vuint8 *)(0x401C0111))
#define MCF_CANMB_TIME9                      (*(vuint16*)(0x401C0112))
#define MCF_CANMB_ID9                        (*(vuint32*)(0x401C0114))
#define MCF_CANMB_DATA_WORD_1_9              (*(vuint16*)(0x401C0118))
#define MCF_CANMB_DATA_WORD_2_9              (*(vuint16*)(0x401C011A))
#define MCF_CANMB_DATA_WORD_3_9              (*(vuint16*)(0x401C011C))
#define MCF_CANMB_DATA_WORD_4_9              (*(vuint16*)(0x401C011E))
#define MCF_CANMB_CODE10                     (*(vuint8 *)(0x401C0120))
#define MCF_CANMB_CTRL10                     (*(vuint8 *)(0x401C0121))
#define MCF_CANMB_TIME10                     (*(vuint16*)(0x401C0122))
#define MCF_CANMB_ID10                       (*(vuint32*)(0x401C0124))
#define MCF_CANMB_DATA_WORD_1_10             (*(vuint16*)(0x401C0128))
#define MCF_CANMB_DATA_WORD_2_10             (*(vuint16*)(0x401C012A))
#define MCF_CANMB_DATA_WORD_3_10             (*(vuint16*)(0x401C012C))
#define MCF_CANMB_DATA_WORD_4_10             (*(vuint16*)(0x401C012E))
#define MCF_CANMB_CODE11                     (*(vuint8 *)(0x401C0130))
#define MCF_CANMB_CTRL11                     (*(vuint8 *)(0x401C0131))
#define MCF_CANMB_TIME11                     (*(vuint16*)(0x401C0132))
#define MCF_CANMB_ID11                       (*(vuint32*)(0x401C0134))
#define MCF_CANMB_DATA_WORD_1_11             (*(vuint16*)(0x401C0138))
#define MCF_CANMB_DATA_WORD_2_11             (*(vuint16*)(0x401C013A))
#define MCF_CANMB_DATA_WORD_3_11             (*(vuint16*)(0x401C013C))
#define MCF_CANMB_DATA_WORD_4_11             (*(vuint16*)(0x401C013E))
#define MCF_CANMB_CODE12                     (*(vuint8 *)(0x401C0140))
#define MCF_CANMB_CTRL12                     (*(vuint8 *)(0x401C0141))
#define MCF_CANMB_TIME12                     (*(vuint16*)(0x401C0142))
#define MCF_CANMB_ID12                       (*(vuint32*)(0x401C0144))
#define MCF_CANMB_DATA_WORD_1_               (*(vuint16*)(0x401C0148))
#define MCF_CANMB_DATA_WORD_2_12             (*(vuint16*)(0x401C014A))
#define MCF_CANMB_DATA_WORD_3_12             (*(vuint16*)(0x401C014C))
#define MCF_CANMB_DATA_WORD_4_12             (*(vuint16*)(0x401C014E))
#define MCF_CANMB_CODE13                     (*(vuint8 *)(0x401C0150))
#define MCF_CANMB_CTRL13                     (*(vuint8 *)(0x401C0151))
#define MCF_CANMB_TIME13                     (*(vuint16*)(0x401C0152))
#define MCF_CANMB_ID13                       (*(vuint32*)(0x401C0154))
#define MCF_CANMB_DATA_WORD_1_13             (*(vuint16*)(0x401C0158))
#define MCF_CANMB_DATA_WORD_2_13             (*(vuint16*)(0x401C015A))
#define MCF_CANMB_DATA_WORD_3_13             (*(vuint16*)(0x401C015C))
#define MCF_CANMB_DATA_WORD_4_13             (*(vuint16*)(0x401C015E))
#define MCF_CANMB_CODE14                     (*(vuint8 *)(0x401C0160))
#define MCF_CANMB_CTRL14                     (*(vuint8 *)(0x401C0161))
#define MCF_CANMB_TIME14                     (*(vuint16*)(0x401C0162))
#define MCF_CANMB_ID14                       (*(vuint32*)(0x401C0164))
#define MCF_CANMB_DATA_WORD_1_14             (*(vuint16*)(0x401C0168))
#define MCF_CANMB_DATA_WORD_2_14             (*(vuint16*)(0x401C016A))
#define MCF_CANMB_DATA_WORD_3_14             (*(vuint16*)(0x401C016C))
#define MCF_CANMB_DATA_WORD_4_14             (*(vuint16*)(0x401C016E))
#define MCF_CANMB_CODE15                     (*(vuint8 *)(0x401C0170))
#define MCF_CANMB_CTRL15                     (*(vuint8 *)(0x401C0171))
#define MCF_CANMB_TIME15                     (*(vuint16*)(0x401C0172))
#define MCF_CANMB_ID15                       (*(vuint32*)(0x401C0174))
#define MCF_CANMB_DATA_WORD_1_15             (*(vuint16*)(0x401C0178))
#define MCF_CANMB_DATA_WORD_2_15             (*(vuint16*)(0x401C017A))
#define MCF_CANMB_DATA_WORD_3_15             (*(vuint16*)(0x401C017C))
#define MCF_CANMB_DATA_WORD_4_15             (*(vuint16*)(0x401C017E))
#define MCF_CANMB_CODE(x)                    (*(vuint8 *)(0x401C0080 + ((x)*0x10)))
#define MCF_CANMB_CTRL(x)                    (*(vuint8 *)(0x401C0081 + ((x)*0x10)))
#define MCF_CANMB_TIME(x)                    (*(vuint16*)(0x401C0082 + ((x)*0x10)))
#define MCF_CANMB_ID(x)                      (*(vuint32*)(0x401C0084 + ((x)*0x10)))
#define MCF_CANMB_DATA_WORD_1(x)             (*(vuint16*)(0x401C0088 + ((x)*0x10)))
#define MCF_CANMB_DATA_WORD_2(x)             (*(vuint16*)(0x401C008A + ((x)*0x10)))
#define MCF_CANMB_DATA_WORD_3(x)             (*(vuint16*)(0x401C008C + ((x)*0x10)))
#define MCF_CANMB_DATA_WORD_4(x)             (*(vuint16*)(0x401C008E + ((x)*0x10)))


/* Other macros */
#define MCF_CANMB_BYTE(x,y)                  (*(vuint8 *)(0x401C0088 + ((x)*0x10)+y))


/* Bit definitions and macros for MCF_CANMB_CODE */
#define MCF_CANMB_CODE_CODE(x)               (((x)&0xF)<<0)

/* Bit definitions and macros for MCF_CANMB_CTRL */
#define MCF_CANMB_CTRL_LENGTH(x)             (((x)&0xF)<<0)
#define MCF_CANMB_CTRL_RTR                   (0x10)
#define MCF_CANMB_CTRL_IDE                   (0x20)
#define MCF_CANMB_CTRL_SRR                   (0x40)

/* Bit definitions and macros for MCF_CANMB_TIME */
#define MCF_CANMB_TIME_TIME_STAMP(x)         (((x)&0xFFFF)<<0)

/* Bit definitions and macros for MCF_CANMB_ID */
#define MCF_CANMB_ID_EXT(x)                  (((x)&0x3FFFF)<<0)
#define MCF_CANMB_ID_STD(x)                  (((x)&0x7FF)<<0x12)

/* Bit definitions and macros for MCF_CANMB_DATA_WORD_1 */
#define MCF_CANMB_DATA_WORD_1_DATA_BYTE_1(x) (((x)&0xFF)<<0)
#define MCF_CANMB_DATA_WORD_1_DATA_BYTE_0(x) (((x)&0xFF)<<0x8)

/* Bit definitions and macros for MCF_CANMB_DATA_WORD_2 */
#define MCF_CANMB_DATA_WORD_2_DATA_BYTE_3(x) (((x)&0xFF)<<0)
#define MCF_CANMB_DATA_WORD_2_DATA_BYTE_2(x) (((x)&0xFF)<<0x8)

/* Bit definitions and macros for MCF_CANMB_DATA_WORD_3 */
#define MCF_CANMB_DATA_WORD_3_DATA_BYTE_5(x) (((x)&0xFF)<<0)
#define MCF_CANMB_DATA_WORD_3_DATA_BYTE_4(x) (((x)&0xFF)<<0x8)

/* Bit definitions and macros for MCF_CANMB_DATA_WORD_4 */
#define MCF_CANMB_DATA_WORD_4_DATA_BYTE_7(x) (((x)&0xFF)<<0)
#define MCF_CANMB_DATA_WORD_4_DATA_BYTE_6(x) (((x)&0xFF)<<0x8)


#endif /* __MCF52235_CANMB_H__ */
