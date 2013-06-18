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
* @file fapp_tftp.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.17.0
*
* @brief FNET Shell Demo API.
*
***************************************************************************/

#ifndef _FAPP_TFTP_H_

#define _FAPP_TFTP_H_

#include "fapp_config.h"

#if FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD || FAPP_CFG_TFTPS_CMD || FAPP_CFG_SETGET_CMD_TYPE  


/********************** RAW **************************************/
/* RAW Rx handler control structure. */
struct fapp_tftp_rx_handler_raw
{
    unsigned long dest; /* Destination address */
};

/* RAW Tx handler control structure. */
#define FAPP_TFTP_RAW_DATA_MAX     (128)
struct fapp_tftp_tx_handler_raw
{
    unsigned char data[FAPP_TFTP_RAW_DATA_MAX];
    int data_size;
    unsigned char *data_cur;
    unsigned char *data_start;
    unsigned char *data_end;
};

/******************************* BIN *****************************/
/* Binary file format is address (4
 * bytes), byte count (4 bytes), and
 * data bytes (variable length).
 */
typedef enum 
{
    FAPP_TFTP_HANDLER_BIN_STATE_INIT = 0,
    FAPP_TFTP_HANDLER_BIN_STATE_GETHEADER,
    FAPP_TFTP_HANDLER_BIN_STATE_GETDATA
}
fapp_tftp_handler_bin_state_t;


#define FAPP_TFTP_BIN_HEADER_SIZE (8)


/* TFTP BIN handler control structure. */
FNET_COMP_PACKED_BEGIN
struct fapp_tftp_rx_handler_bin
{
    union 
    {
        struct
        {
            unsigned long address	FNET_COMP_PACKED;
            unsigned long size 		FNET_COMP_PACKED;
        } header 					FNET_COMP_PACKED;
        
        unsigned char header_bytes[FAPP_TFTP_BIN_HEADER_SIZE]	FNET_COMP_PACKED;
    }	FNET_COMP_PACKED;
    int header_index 					FNET_COMP_PACKED;
    fapp_tftp_handler_bin_state_t state FNET_COMP_PACKED;
};
FNET_COMP_PACKED_END

/* TFTP SREC TX handler control structure. */
#define FAPP_TFTP_BIN_DATA_MAX     (252)
FNET_COMP_PACKED_BEGIN
struct fapp_tftp_tx_handler_bin
{
    struct
    {
        unsigned long address 	FNET_COMP_PACKED;
        unsigned long count 	FNET_COMP_PACKED;
        unsigned char data[FAPP_TFTP_BIN_DATA_MAX]	FNET_COMP_PACKED;
    } bin_line 					FNET_COMP_PACKED;
    int bin_line_size 			FNET_COMP_PACKED;
    char *bin_line_cur 			FNET_COMP_PACKED;
    unsigned char *data_start 	FNET_COMP_PACKED;
    unsigned char *data_end 	FNET_COMP_PACKED;
};
FNET_COMP_PACKED_END

/********************** SREC **************************************/


/* SREC Tx Handler states */ 
typedef enum 
{
    FAPP_TFTP_RX_HANDLER_SREC_STATE_INIT = 0,
    FAPP_TFTP_RX_HANDLER_SREC_STATE_GETSTART,
    FAPP_TFTP_RX_HANDLER_SREC_STATE_GETTYPE,
    FAPP_TFTP_RX_HANDLER_SREC_STATE_GETCOUNT,
    FAPP_TFTP_RX_HANDLER_SREC_STATE_GETDATA
}
fapp_tftp_rx_handler_srec_state_t;

/* SREC Rx Handler states */ 
typedef enum 
{
    FAPP_TFTP_TX_HANDLER_SREC_STATE_INIT = 0,
    FAPP_TFTP_TX_HANDLER_SREC_STATE_DATA,
    FAPP_TFTP_TX_HANDLER_SREC_STATE_EOB,
    FAPP_TFTP_TX_HANDLER_SREC_STATE_END
}
fapp_tftp_tx_handler_srec_state_t;


/*
+-------------------//------------------//-----------------------+
| type | count | address  |            data           | checksum |
+-------------------//------------------//-----------------------+
*/


/* TFTP SREC handler control structure. */
FNET_COMP_PACKED_BEGIN
struct fapp_tftp_rx_handler_srec
{
    fapp_tftp_rx_handler_srec_state_t state FNET_COMP_PACKED;
    unsigned char type 						FNET_COMP_PACKED;	/* Record type */
    int record_hex_index 					FNET_COMP_PACKED;
    union 
    {
        struct
        {
            unsigned char count 	FNET_COMP_PACKED;	/* Byte count */
            unsigned char data[255] FNET_COMP_PACKED; 	/* Address + Data */
        } record	FNET_COMP_PACKED;
        unsigned char record_bytes[255+1]	FNET_COMP_PACKED;
    }	FNET_COMP_PACKED;
};
FNET_COMP_PACKED_END
/* TFTP SREC TX handler control structure. */
#define FAPP_TFTP_SREC_DATA_MAX     (28)
FNET_COMP_PACKED_BEGIN
struct fapp_tftp_tx_handler_srec
{
    fapp_tftp_tx_handler_srec_state_t state	FNET_COMP_PACKED;
    struct
    {
        char S 			FNET_COMP_PACKED;
        char type 		FNET_COMP_PACKED;
        char count[2] 	FNET_COMP_PACKED;
        char address[8] FNET_COMP_PACKED;
        short data[FAPP_TFTP_SREC_DATA_MAX + 1/*checksum*/ +1/*\r\n*/+ 1/*eol*/] FNET_COMP_PACKED;
    } srec_line 		FNET_COMP_PACKED;
    int srec_line_size 	FNET_COMP_PACKED;
    char *srec_line_cur FNET_COMP_PACKED;
    unsigned char *data_start 	FNET_COMP_PACKED;
    unsigned char *data_end 	FNET_COMP_PACKED;
};
FNET_COMP_PACKED_END


/******* TFTP handler control structure. ******/
typedef struct  
{
    union
    {
        struct fapp_tftp_rx_handler_raw rx_raw;
        struct fapp_tftp_tx_handler_raw tx_raw;
        struct fapp_tftp_rx_handler_bin rx_bin;
        struct fapp_tftp_tx_handler_bin tx_bin;
        struct fapp_tftp_rx_handler_srec rx_srec;
        struct fapp_tftp_tx_handler_srec tx_srec;
    };
    struct image_type *current_type;
    unsigned long image_size;
} fapp_tftp_handler_control_t; 

/* Image type. */
struct image_type
{
    unsigned char index;
    char * name;
    int(* rx_handler)(fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char *data, unsigned long n);
    int(* tx_handler)(fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char *data, unsigned long n);
};

struct image_type *fapp_tftp_image_type_by_index (unsigned long index);
struct image_type *fapp_tftp_image_type_by_name (char *name);
#endif

#if FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD || FAPP_CFG_TFTPS_CMD
/* Region for TFTP upload.*/
#define FAPP_TFTP_TX_MEM_REGION (&fapp_mem_regions[0])
#endif

#if FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD
void fapp_tftp_cmd( fnet_shell_desc_t desc, int argc, char ** );
#endif 

#if FAPP_CFG_TFTPS_CMD
void fapp_tftps_cmd( fnet_shell_desc_t desc, int argc, char ** argv );
void fapp_tftps_info( fnet_shell_desc_t desc );
void fapp_tftps_release( void );
#endif

#endif

