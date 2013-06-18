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
* @file fapp_tftp.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.43.0
*
* @brief TFTP file loader implementation.
*
***************************************************************************/

#include "fapp.h"
#include "fapp_prv.h"
#include "fapp_tftp.h"
#include "fapp_mem.h"


/************************************************************************
*     Definitions.
*************************************************************************/
#define FAPP_TFTP_FAILED            "\nFailed!"
#define FAPP_TFTP_ERR               "\nTFTP Error: Code %d \"%s\"."
#define FAPP_TFTP_CHECKSUM_ERR      "\nChecksum error."

#define FAPP_TFTP_RX_HEADER_STR     "TFTP downloading \'%s\' (%s) from %s :  "
#define FAPP_TFTP_TX_HEADER_STR     "TFTP uploading \'%s\' (%s) to %s :  "  
#define FAPP_TFTP_ENTRYPOINT_STR    "\nEntry point set to 0x%08X"
#define FAPP_TFTP_COMPLETED_STR     "\nTFTP completed (%d bytes)"

#if FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD || FAPP_CFG_TFTPS_CMD

/************************************************************************
*     Function Prototypes
*************************************************************************/
#if FAPP_CFG_TFTP_RX_RAW
    int fapp_tftp_rx_handler_raw (fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char* data, unsigned long data_size);
#endif
#if FAPP_CFG_TFTP_TX_RAW
    int fapp_tftp_tx_handler_raw (fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char* data, unsigned long data_size);
#endif
#if FAPP_CFG_TFTP_RX_BIN
    int fapp_tftp_rx_handler_bin (fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char* data, unsigned long data_size);
#endif
#if FAPP_CFG_TFTP_TX_BIN
    int fapp_tftp_tx_handler_bin (fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char* data, unsigned long data_size);
#endif
#if FAPP_CFG_TFTP_RX_SREC
    int fapp_tftp_rx_handler_srec (fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char* data, unsigned long data_size);
#endif
#if FAPP_CFG_TFTP_TX_SREC
    int fapp_tftp_tx_handler_srec (fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char* data, unsigned long data_size);
#endif    
#endif


#if FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD
static fapp_tftp_handler_control_t fapp_tftp_handler_control;

/* Progress variables. */
const static char progress[] = {'\\','|','/','-'};
static unsigned char progress_counter = 0;

#endif


#if FAPP_CFG_TFTPS_CMD

/******* TFTP Firmware Server handler control structure. ******/
static fapp_tftp_handler_control_t fapp_tftps_handler_control;

static fnet_tftp_srv_desc_t fapp_tftp_srv_desc = 0; /* TFTP server descriptor. */

#endif /* FAPP_CFG_TFTPS_CMD*/


#if FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD || FAPP_CFG_TFTPS_CMD || FAPP_CFG_SETGET_CMD_TYPE  

#define FAPP_PARAMS_TFTP_FILE_TYPE_RAW_STR      "raw"   /* Raw binary file. */
#define FAPP_PARAMS_TFTP_FILE_TYPE_BIN_STR      "bin"   /* CodeWarrior binary file. */
#define FAPP_PARAMS_TFTP_FILE_TYPE_SREC_STR     "srec"  /* SREC file. */

struct image_type image_types[] =
{
    {FAPP_PARAMS_TFTP_FILE_TYPE_RAW, FAPP_PARAMS_TFTP_FILE_TYPE_RAW_STR, 
        #if FAPP_CFG_TFTP_RX_RAW
            fapp_tftp_rx_handler_raw
        #else
            0
        #endif
            , 
        #if FAPP_CFG_TFTP_TX_RAW    
            fapp_tftp_tx_handler_raw
        #else
            0    
        #endif    
    },
    {FAPP_PARAMS_TFTP_FILE_TYPE_BIN, FAPP_PARAMS_TFTP_FILE_TYPE_BIN_STR, 
        #if FAPP_CFG_TFTP_RX_BIN    
            fapp_tftp_rx_handler_bin
        #else
            0
        #endif
            , 
        #if FAPP_CFG_TFTP_TX_BIN    
            fapp_tftp_tx_handler_bin
        #else
            0
        #endif        
    },
    {FAPP_PARAMS_TFTP_FILE_TYPE_SREC, FAPP_PARAMS_TFTP_FILE_TYPE_SREC_STR, 
        #if FAPP_CFG_TFTP_RX_SREC    
            fapp_tftp_rx_handler_srec
        #else
            0
        #endif    
            , 
        #if FAPP_CFG_TFTP_TX_SREC    
            fapp_tftp_tx_handler_srec
        #else
            0
        #endif        
    }    
};


#define FAPP_TFTP_IMAGE_TYPE_SIZE  (sizeof(image_types)/sizeof(struct image_type))  
#define FAPP_TFTP_IMAGE_TYPE_DEFAULT   image_types[0]  /* Default image type. */

/************************************************************************
* NAME: fapp_tftp_image_type_by_index
*
* DESCRIPTION:
************************************************************************/
struct image_type *fapp_tftp_image_type_by_index (unsigned long index)
{
    struct image_type *result = &FAPP_TFTP_IMAGE_TYPE_DEFAULT;
    struct image_type *type = image_types;
    int i;
    
    for(i=0; i<FAPP_TFTP_IMAGE_TYPE_SIZE; i++)
    {
        if( type->index == index )
        {
            result = type;
            break;
        } 
        type++;  
    }
    
    return result;    
}

/************************************************************************
* NAME: fapp_tftp_image_type_by_index
*
* DESCRIPTION:
************************************************************************/
struct image_type *fapp_tftp_image_type_by_name (char *name)
{
    struct image_type *result = 0;
    struct image_type *type = image_types;
        int i;
    
    for(i=0; i<FAPP_TFTP_IMAGE_TYPE_SIZE; i++)
    {
        if(fnet_strcmp( type->name, name ) == 0)
        {
            result = type;
            break;
        } 
        type++;  
    }
    
    return result;   
}

#endif

#if FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD || FAPP_CFG_TFTPS_CMD

/************************************************************************
* NAME: fapp_tftp_tx_image_begin_end
*
* DESCRIPTION: 
************************************************************************/
static void fapp_tftp_tx_image_begin_end(unsigned char * FNET_COMP_PACKED_VAR *data_begin_p, unsigned char * FNET_COMP_PACKED_VAR *data_end_p)
{
    unsigned char *data_end = (unsigned char *)FAPP_TFTP_TX_MEM_REGION->address + FAPP_TFTP_TX_MEM_REGION->size;
    unsigned char *data_cur;
    unsigned long step = FAPP_TFTP_TX_MEM_REGION->erase_size;
    
    /* Find image start. */
    data_cur = (unsigned char *)FAPP_TFTP_TX_MEM_REGION->address;
    while( ((data_cur < data_end) && fapp_mem_region_is_protected((unsigned long)data_cur, step) == 1) )
    {
        data_cur += step;
    }
    *data_begin_p = data_cur;
    
    /* Find image end. */
    while((data_cur < data_end) && (fapp_mem_region_is_protected((unsigned long)data_cur, step) == 0) )
    {
        data_cur += step;
    }
    *data_end_p = data_cur;

}

/*======================== BIN ========================================*/
#if FAPP_CFG_TFTP_RX_BIN
/************************************************************************
* NAME: fapp_tftp_rx_handler_bin
*
* DESCRIPTION: 
************************************************************************/
static int fapp_tftp_rx_handler_bin (fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char* data, unsigned long n)
{
    struct fapp_tftp_rx_handler_bin * bin = &tftp_handler->rx_bin;
    int result = FNET_OK;
    
    while(n && (result == FNET_OK))
    {
        switch(bin->state)
        {
            default:
            case FAPP_TFTP_HANDLER_BIN_STATE_INIT:
                bin->state = FAPP_TFTP_HANDLER_BIN_STATE_GETHEADER;
                bin->header_index = 0;
            case FAPP_TFTP_HANDLER_BIN_STATE_GETHEADER: /* Get header. */
                if(bin->header_index<FAPP_TFTP_BIN_HEADER_SIZE)
                {
                    bin->header_bytes[bin->header_index++] = *data++;
                    n--;
                }
                else
                {
                    bin->state = FAPP_TFTP_HANDLER_BIN_STATE_GETDATA;   
                }    
                break;
            case FAPP_TFTP_HANDLER_BIN_STATE_GETDATA: /* Get data. */
                {
                    unsigned long copy_size;
                   
                    copy_size = bin->header.size; 
                    if(copy_size>n)
                       copy_size = n;     
                    
                    /* Copy to the destination. */
                    if( (result = fapp_mem_memcpy (desc, (void *) bin->header.address, data, copy_size )) == FNET_OK )
                    {
                        bin->header.address+=copy_size;
                        bin->header.size-=copy_size;
                        n-=copy_size;
                        data+=copy_size;
                        
                        if(bin->header.size == 0)
                        {
                            bin->state = FAPP_TFTP_HANDLER_BIN_STATE_INIT;
                        }
                    }
                }
                break;    
         }
    } 
    
    return result;    
}
#endif

#if FAPP_CFG_TFTP_TX_BIN
/************************************************************************
* NAME: fapp_tftp_tx_bin_gen
*
* DESCRIPTION: 
************************************************************************/
static /* inline */ void fapp_tftp_tx_bin_gen (struct fapp_tftp_tx_handler_bin *tx_bin)
{
    int send_size;

    send_size = tx_bin->data_end - tx_bin->data_start;
                
    if(send_size > FAPP_TFTP_BIN_DATA_MAX)
        send_size = FAPP_TFTP_BIN_DATA_MAX;
                
    if(send_size)
    {
         /* Byte count.*/
        tx_bin->bin_line.count = (unsigned long)send_size;
        /* Address.*/
        tx_bin->bin_line.address = (unsigned long)tx_bin->data_start;
        /* Data. */
        fnet_memcpy(tx_bin->bin_line.data, tx_bin->data_start, (unsigned int)send_size);
        tx_bin->data_start += send_size;
                
        tx_bin->bin_line_size = send_size+4+4; /* Save line size. */
    }
    else        
        tx_bin->bin_line_size = 0; /* End of image. */        

    /* Reset line_cur.*/
    tx_bin->bin_line_cur = (char*)&(tx_bin->bin_line); 

}

/************************************************************************
* NAME: fapp_tftp_tx_handler_bin
*
* DESCRIPTION: 
************************************************************************/
static int fapp_tftp_tx_handler_bin (fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char* data, unsigned long data_size)
{
    struct fapp_tftp_tx_handler_bin * tx_bin = &tftp_handler->tx_bin;
    
    unsigned long send_size;
    int result = 0;
    
    FNET_COMP_UNUSED_ARG(desc);
    
    /* Define start and end address.*/
    if((tx_bin->data_start == 0) && (tx_bin->data_end == 0)) /* Only first time. */
        fapp_tftp_tx_image_begin_end(&tx_bin->data_start, &tx_bin->data_end);

    while(data_size)
    {
        if(tx_bin->bin_line_size == 0)
        {
            /* Generate CW bin.*/
            fapp_tftp_tx_bin_gen (tx_bin);
        
            if(tx_bin->bin_line_size == 0)
                break; /* EOF */
        }

        send_size = (unsigned long)tx_bin->bin_line_size;   
        if(send_size > data_size)
            send_size = data_size;
            
        fnet_memcpy(data, tx_bin->bin_line_cur, send_size);
            
        data_size -= send_size;
        data += send_size;
        tx_bin->bin_line_cur += send_size;
        tx_bin->bin_line_size -= send_size;

        result += send_size;       
    }

    return result;
}  
#endif
/*======================== RAW ========================================*/

/************************************************************************
* NAME: fapp_tftp_rx_handler_raw
*
* DESCRIPTION: 
************************************************************************/
#if FAPP_CFG_TFTP_RX_RAW
static int fapp_tftp_rx_handler_raw (fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char* data, unsigned long n)
{
    struct fapp_tftp_rx_handler_raw * raw = &tftp_handler->rx_raw;
    int result;
    
    if(raw->dest == 0) /* Only one time. */
        raw->dest = fapp_params_tftp_config.file_raw_address;    
    
    /* Copy to the destination. */
    result = fapp_mem_memcpy (desc, (void *)raw->dest, data, n );

    raw->dest+=n;
    
    return result;    
}
#endif

#if FAPP_CFG_TFTP_TX_RAW
/************************************************************************
* NAME: fapp_tftp_tx_raw_gen
*
* DESCRIPTION: 
************************************************************************/
static /* inline */ void fapp_tftp_tx_raw_gen (struct fapp_tftp_tx_handler_raw *tx_raw)
{
    int send_size;

    send_size = tx_raw->data_end - tx_raw->data_start;
                
    if(send_size > FAPP_TFTP_RAW_DATA_MAX)
        send_size = FAPP_TFTP_RAW_DATA_MAX;
                
    if(send_size)
    {
        fnet_memcpy(tx_raw->data, tx_raw->data_start, (unsigned int)send_size);
        tx_raw->data_start += send_size;
    }

    tx_raw->data_size = send_size; /* Save line size. */        

    /* Reset line_cur.*/
    tx_raw->data_cur = tx_raw->data; 

}

/************************************************************************
* NAME: fapp_tftp_tx_handler_raw
*
* DESCRIPTION: 
************************************************************************/
static int fapp_tftp_tx_handler_raw (fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char* data, unsigned long data_size)
{
    struct fapp_tftp_tx_handler_raw * tx_raw = &tftp_handler->tx_raw;
    unsigned long send_size;
    int result = 0;
    
    FNET_COMP_UNUSED_ARG(desc);
    
    /* Define start and end address.*/
    if((tx_raw->data_start == 0) && (tx_raw->data_end == 0)) /* Only first time. */
        fapp_tftp_tx_image_begin_end(&tx_raw->data_start, &tx_raw->data_end);
        
    while(data_size)
    {
        /* Generate raw binary.*/
        if(tx_raw->data_size == 0)
        {
            fapp_tftp_tx_raw_gen (tx_raw);
        
            if(tx_raw->data_size == 0)
                break; /* EOF. */
        }
 
        send_size = (unsigned long)tx_raw->data_size;   
        if(send_size > data_size)
            send_size = data_size;
            
        fnet_memcpy(data, tx_raw->data_cur, send_size);
            
        data_size -= send_size;
        data += send_size;
        tx_raw->data_cur += send_size;
        tx_raw->data_size -= send_size;

        result += send_size;       
    }

    return result;
} 
#endif

/*======================== SREC ========================================*/
#if FAPP_CFG_TFTP_RX_SREC
/************************************************************************
* NAME: fapp_tftp_rx_handler_srec
*
* DESCRIPTION: 
************************************************************************/
static int fapp_tftp_rx_handler_srec (fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char* data, unsigned long n)
{
    struct fapp_tftp_rx_handler_srec * srec = &tftp_handler->rx_srec;
    int result = FNET_OK;
    char tmp[2];
    char tmp_data;
    char *p = 0;
    int i;
    unsigned char checksum;
    
    tmp[1]='\0';
    
    while(n && (result == FNET_OK))
    {
        switch(srec->state)
        {
            default:
            case FAPP_TFTP_RX_HANDLER_SREC_STATE_INIT:
                srec->state = FAPP_TFTP_RX_HANDLER_SREC_STATE_GETSTART;
                break;
            case FAPP_TFTP_RX_HANDLER_SREC_STATE_GETSTART:
                if(*data++ == 'S')
                {
                    srec->state = FAPP_TFTP_RX_HANDLER_SREC_STATE_GETTYPE;
                }
                n--;
                break;
            case FAPP_TFTP_RX_HANDLER_SREC_STATE_GETTYPE:
                srec->type = *data++;
                n--;
                srec->record_hex_index=0; /* Reset hex index. */
                srec->record.count = 0xFF; /* Trick. */
                srec->state = FAPP_TFTP_RX_HANDLER_SREC_STATE_GETDATA;
                break;
            case FAPP_TFTP_RX_HANDLER_SREC_STATE_GETDATA:
                tmp[0] = (char)*data++;
                n--;
                tmp_data = (char)fnet_strtoul(tmp,&p,16); /* Char to integer.*/
                if ((tmp_data == 0) && (p == tmp))
                {
                    result = FNET_ERR;
                    break;
                }
                
                srec->record_bytes[srec->record_hex_index>>1] = (unsigned char)((srec->record_bytes[srec->record_hex_index>>1] & (0xF<<(4*(srec->record_hex_index%2))))
                                                          + (tmp_data<<(4*((srec->record_hex_index+1)%2))));
                 
                if(srec->record_hex_index > ((srec->record.count<<1)))
                {
                    char type;
                    char *addr = 0;
                    
                    /* Check checksum. */
                    checksum = 0;
                    for(i=0; i< srec->record.count; i++)
                    {
                        checksum+=srec->record_bytes[i];    
                    }
                   
                    if(srec->record_bytes[srec->record.count] != (unsigned char)~checksum)
                    {
                        result = FNET_ERR;
                        fnet_shell_println(desc, FAPP_TFTP_CHECKSUM_ERR);
                        break;     
                    }
                    
                    /* Handle S[type].*/
                    type = (char)(srec->type - '0'); /* Convert from character to number.*/
                    if((type == 1)|| (type == 9))
                    {
                        addr = (char *)( ((unsigned long)((srec->record.data[0])&0xFFL)<< 8 ) + (unsigned long)((srec->record.data[1])&0xFFL));
                    
                    }
                    
                    if((type == 2)|| (type == 8))
                    {
                        addr = (char *)( ((unsigned long)((srec->record.data[0])&0xFFL)<< 16) + 
                             ((unsigned long)((srec->record.data[1])&0xFFL)<< 8 ) + (unsigned long)((srec->record.data[2])&0xFFL));
                    
                    }
                    
                    if((type == 3)|| (type == 7))
                    {
                        addr = (char *)(((unsigned long)((srec->record.data[0])&0xFFL)<< 24) + ((unsigned long)((srec->record.data[1])&0xFFL)<< 16) + 
                             ((unsigned long)((srec->record.data[2])&0xFFL)<< 8 ) + (unsigned long)((srec->record.data[3])&0xFFL));
                    }
                    
                    if((type > 0) && (type < 4)) /* Data sequence. */
                    {
                        /* Copy data to the destination. */
                        result = fapp_mem_memcpy (desc, (void *)addr, srec->record.data + (1+type), (unsigned int)(srec->record.count - (2+type)) );
                    }
                    
                    if((type > 6) && (type < 10)) /* End of block. */
                    {
                        /* Set entry point. */
                        fapp_params_boot_config.go_address = (unsigned long)addr;
                        fnet_shell_println(desc, FAPP_TFTP_ENTRYPOINT_STR, addr);
                    }
 
                    srec->state = FAPP_TFTP_RX_HANDLER_SREC_STATE_GETSTART;
                }
                
                srec->record_hex_index++;
                break;
        }
    }
    
    return result;
}    
#endif

#if FAPP_CFG_TFTP_TX_SREC
/************************************************************************
* NAME: fapp_tftp_tx_srec_gen
*
* DESCRIPTION: 
************************************************************************/
static /*inline*/ void fapp_tftp_tx_srec_gen (struct fapp_tftp_tx_handler_srec *tx_srec)
{
    unsigned char checksum;
    int send_size;
    int i;    
   
    switch(tx_srec->state)
    {
        /* === Block header.===*/
        case FAPP_TFTP_TX_HANDLER_SREC_STATE_INIT:
            fnet_sprintf(&tx_srec->srec_line.S, "S0030000FC\n");
            tx_srec->state = FAPP_TFTP_TX_HANDLER_SREC_STATE_DATA; 
            break;
        /* === Data sequence. ===*/    
        case FAPP_TFTP_TX_HANDLER_SREC_STATE_DATA:

            send_size = tx_srec->data_end - tx_srec->data_start;
 
            fnet_sprintf(&tx_srec->srec_line.S, "S3");
                
            if(send_size > FAPP_TFTP_SREC_DATA_MAX)
                send_size = FAPP_TFTP_SREC_DATA_MAX;
                
            if(send_size)
            {
                /* Byte count.*/
                checksum = (unsigned char) (send_size+4/*Address*/+1/*Checksum.*/);
                fnet_sprintf(tx_srec->srec_line.count, "%02X", checksum);
                /* Address.*/
                fnet_sprintf(tx_srec->srec_line.address, "%08X", tx_srec->data_start);
                /* Calculate checksum on address field. */
                checksum += (unsigned char)((unsigned long)tx_srec->data_start + ((unsigned long)tx_srec->data_start>>8) 
                            + ((unsigned long)tx_srec->data_start>>16)+ ((unsigned long)tx_srec->data_start>>24));
                /* Data. */
                for(i=0; i < send_size; i++)
                {
                    checksum += *tx_srec->data_start;
                    fnet_sprintf((char*)&tx_srec->srec_line.data[i], "%02X", *tx_srec->data_start);
                    tx_srec->data_start++;
                }
                /* Checksum.*/
                fnet_sprintf((char*)&tx_srec->srec_line.data[send_size], "%02X\n", (unsigned char)~checksum);
            }

            if(tx_srec->data_end == tx_srec->data_start) /* End of image.*/
                tx_srec->state = FAPP_TFTP_TX_HANDLER_SREC_STATE_EOB;         
            break;
        /*=== End of block. ===*/    
        case FAPP_TFTP_TX_HANDLER_SREC_STATE_EOB:
            fnet_sprintf(&tx_srec->srec_line.S, "S705");
            checksum = 5;
            /* Starting address.*/
            fnet_sprintf(tx_srec->srec_line.address, "%08X", fapp_params_boot_config.go_address);
            /* Calculate checksum on address field. */
            checksum += (unsigned char)(fapp_params_boot_config.go_address + (fapp_params_boot_config.go_address>>8) 
                       + (fapp_params_boot_config.go_address>>16)+ (fapp_params_boot_config.go_address>>24));
            /* Checksum.*/
            fnet_sprintf((char*)&tx_srec->srec_line.data[0], "%02X", (unsigned char)~checksum);           
            tx_srec->state = FAPP_TFTP_TX_HANDLER_SREC_STATE_END;
            break;        
        /* EOF*/    
        case FAPP_TFTP_TX_HANDLER_SREC_STATE_END:
            tx_srec->srec_line.S = 0;
            break;   
    
    }
    
    /* Save line size. */
    tx_srec->srec_line_size = (int)fnet_strlen((char*)(&tx_srec->srec_line));
    /* Reset line_cur.*/
    tx_srec->srec_line_cur = (char*)&(tx_srec->srec_line); 
}



/************************************************************************
* NAME: fapp_tftp_tx_handler_srec
*
* DESCRIPTION: 
************************************************************************/
static int fapp_tftp_tx_handler_srec (fapp_tftp_handler_control_t *tftp_handler, fnet_shell_desc_t desc, unsigned char* data, unsigned long data_size)
{
    struct fapp_tftp_tx_handler_srec *tx_srec = &tftp_handler->tx_srec;
    unsigned long send_size;
    int result = 0;

    FNET_COMP_UNUSED_ARG(desc);
    
    /* Define start and end address.*/
    if((tx_srec->data_start == 0) && (tx_srec->data_end == 0)) /* Only first time. */
        fapp_tftp_tx_image_begin_end(&tx_srec->data_start, &tx_srec->data_end);

    while(data_size)
    {
        if(tx_srec->srec_line_size == 0)
        {
            /* Generate srec.*/
            fapp_tftp_tx_srec_gen (tx_srec);
        
            if(tx_srec->srec_line_size == 0)
                break; /*EOF*/
        }
        
        send_size = (unsigned long)tx_srec->srec_line_size;   
        if(send_size > data_size)
            send_size = data_size;
            
        fnet_memcpy(data, tx_srec->srec_line_cur, send_size);
            
        data_size -= send_size;
        data += send_size;
        tx_srec->srec_line_cur += send_size;
        tx_srec->srec_line_size -= send_size;

        result += send_size;       
    }

    return result;
}    
#endif /* FAPP_CFG_TFTP_TX_SREC */


#endif /* FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD || FAPP_CFG_TFTPS_CMD*/


#if FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD
/************************************************************************
* NAME: fapp_tftp_handler
*
* DESCRIPTION: TFTP RX and TX handler. 
************************************************************************/
static int fapp_tftp_handler (fnet_tftp_request_t request_type, unsigned char* data_ptr, unsigned short data_size, int result, void *shl_desc)
{
    fnet_shell_desc_t desc = (fnet_shell_desc_t)shl_desc;

    if(result == FNET_OK)
    {
        /* Print image download progress. */
        fnet_shell_putchar(desc, '\b');
        fnet_shell_putchar(desc, progress[progress_counter++]); /* Print progress. */
        progress_counter &= 0x3;

        /* -- FNET_TFTP_REQUEST_READ -- */
        if(request_type == FNET_TFTP_REQUEST_READ)
        {
            result = fapp_tftp_handler_control.current_type->rx_handler(&fapp_tftp_handler_control,desc, data_ptr, data_size);
        }
        /* -- FNET_TFTP_REQUEST_WRITE -- */
        else
        {
    #if 0  
            /* Just Hello World example, only for debug.*/
            result = fnet_sprintf((char*)data_ptr, "Hello World!\n");
    #else
            result = fapp_tftp_handler_control.current_type->tx_handler(&fapp_tftp_handler_control, desc, data_ptr, data_size);
    #endif            
            data_size = (unsigned short)result;
        }
            
        if(result == FNET_ERR)
            goto ERROR;
        else
        {
            fapp_tftp_handler_control.image_size += data_size; 
            
            /* Check EOF. */    
            if(data_size < FNET_TFTP_DATA_SIZE_MAX)
            {
                fnet_shell_println(desc, FAPP_TFTP_COMPLETED_STR, fapp_tftp_handler_control.image_size);  
                fnet_shell_unblock(desc); /* Unblock shell. */
            }
        }
    }
    else       
    {
    ERROR:    
        fnet_shell_println(desc, FAPP_TFTP_ERR, data_size, data_ptr );  
        fnet_shell_unblock(desc);           /* Unblock shell. */
        fnet_shell_script_release(desc);    /* Clear script. */    
    }
    
    return result;
}


/************************************************************************
* NAME: fapp_tftp_on_ctrlc
*
* DESCRIPTION: 
************************************************************************/
static void fapp_tftp_on_ctrlc(fnet_shell_desc_t desc)
{
    /* Release TFTP. */
    fnet_tftp_cln_release();
    fnet_shell_putchar(desc, '\b'); /* Clear progress. */
    fnet_shell_println(desc, FAPP_CANCELLED_STR);
    fnet_shell_script_release(desc);   /* Clear script. */       
}

/************************************************************************
* NAME: fapp_tftp_cmd
*
* DESCRIPTION: Start TFTP-client loader/uploader. 
************************************************************************/
void fapp_tftp_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    struct fnet_tftp_cln_params tftp_params;
    char ip_str[FNET_IP_ADDR_STR_SIZE];

    /* Set parameters of the TFTP-client service. */

    /* "tftpup" TFTP firmware uploader. */
    if(fnet_strcasecmp("tftpup", argv[0]) == 0) 
        tftp_params.request_type = FNET_TFTP_REQUEST_WRITE; /* TFTP write request. */
    /* "tftp" TFTP firmware loader. */
    else  
        tftp_params.request_type = FNET_TFTP_REQUEST_READ; /* TFTP read request. */
       
    tftp_params.handler = fapp_tftp_handler;
    tftp_params.file_name = fapp_params_tftp_config.file_name;
    
    /* TFTP Server IP address. */
    tftp_params.server_addr = fapp_params_tftp_config.server_addr;
    
    tftp_params.handler_param = (void *)desc;
    tftp_params.timeout = 0; /* Default timeout. */
    
    /* Reset fapp_tftp_handler_control. */
    fnet_memset_zero(&fapp_tftp_handler_control, sizeof(fapp_tftp_handler_control));
    
    fapp_tftp_handler_control.current_type = fapp_tftp_image_type_by_index(fapp_params_tftp_config.file_type); /* Get type. */
    
    /* Change default parameters. */
    if(argc >= 2) 
    {
        tftp_params.file_name = argv[1];
        
        if(argc >= 3) /* image name */
        {
            if(fnet_inet_ptos(argv[2], &tftp_params.server_addr) == FNET_ERR)
            {
                fnet_shell_println(desc, FAPP_PARAM_ERR, argv[2]);   /* Wrong TFTP Server IP address. */
                fnet_shell_script_release(desc);                    /* Clear script. */    
                return;
            }
            
            if(argc >= 4 && ((fapp_tftp_handler_control.current_type = fapp_tftp_image_type_by_name (argv[3]))==0)) /* image type */
            {
                fnet_shell_println(desc, FAPP_PARAM_ERR, argv[3]); /* Wrong image type. */
                fnet_shell_script_release(desc);   /* Clear script. */    
                return; 
            }
        }
    }
    
    if(fapp_tftp_handler_control.current_type && 
        (((tftp_params.request_type == FNET_TFTP_REQUEST_WRITE) && fapp_tftp_handler_control.current_type->tx_handler) || 
        ((tftp_params.request_type == FNET_TFTP_REQUEST_READ) && fapp_tftp_handler_control.current_type->rx_handler))&&
        (fnet_tftp_cln_init( &tftp_params ) != FNET_ERR))
    {
        fnet_shell_println(desc, FAPP_TOCANCEL_STR);
        fnet_shell_printf(desc, (tftp_params.request_type == FNET_TFTP_REQUEST_READ)?FAPP_TFTP_RX_HEADER_STR:FAPP_TFTP_TX_HEADER_STR, 
                          tftp_params.file_name, fapp_tftp_handler_control.current_type->name, 
                          fnet_inet_ntop(tftp_params.server_addr.sa_family, tftp_params.server_addr.sa_data, ip_str, sizeof(ip_str)) );
                          
        fnet_shell_block(desc, fapp_tftp_on_ctrlc);
    }
    else
    {
        fnet_shell_println(desc, FAPP_INIT_ERR, "TFTP");
        fnet_shell_script_release(desc);   /* Critical error. Clear script. */    
    }
}


#endif /*FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD*/




/********************* TFTP firmware server *****************************************/
#if FAPP_CFG_TFTPS_CMD

#define FAPP_TFTPS_ERROR_BUSY "Busy"
/************************************************************************
* NAME: fapp_tftps_request_handler
*
* DESCRIPTION: TFTP server request handler.
*************************************************************************/
static int fapp_tftps_request_handler(fnet_tftp_request_t request_type,
                                                const struct sockaddr *address,
                                                char* filename,        /* null-terminated.*/
                                                char* mode,            /* null-terminated.*/
                                                fnet_tftp_error_t *error_code,     /* optional default is 0) error code [0-7] RFC783, if result==FNET_ERR*/
                                                char* *error_message, /* optinal, if result==FNET_ERR*/
                                                void *shl_desc)
{
    int result;
    fnet_shell_desc_t desc = (fnet_shell_desc_t)shl_desc;
    char ip_str[FNET_IP_ADDR_STR_SIZE];
      
    FNET_COMP_UNUSED_ARG(mode);
    FNET_COMP_UNUSED_ARG(error_message); 
    
    /* Check Filename for firmware (up)load. */
    if(fnet_strcmp(fapp_params_tftp_config.file_name, filename) == 0)
    {
        /* Reset fapp_tftps_handler_control. */
        fnet_memset_zero(&fapp_tftps_handler_control, sizeof(fapp_tftps_handler_control));
    
        /* Get  handler type. */
        fapp_tftps_handler_control.current_type = fapp_tftp_image_type_by_index(fapp_params_tftp_config.file_type); 

        if(fapp_tftps_handler_control.current_type &&
           (((request_type == FNET_TFTP_REQUEST_WRITE) && fapp_tftps_handler_control.current_type->rx_handler) || 
           ((request_type == FNET_TFTP_REQUEST_READ) && fapp_tftps_handler_control.current_type->tx_handler)))
        {
            fnet_shell_println(desc, (request_type == FNET_TFTP_REQUEST_READ)?FAPP_TFTP_TX_HEADER_STR:FAPP_TFTP_RX_HEADER_STR, 
                               filename, fapp_tftps_handler_control.current_type->name, 
                               fnet_inet_ntop(address->sa_family, (char*)(address->sa_data), ip_str, sizeof(ip_str)) ); 
                
            /* Do erase all, on WRITE request.*/
            if(request_type == FNET_TFTP_REQUEST_WRITE)
            {
                fnet_shell_script(desc, FAPP_CFG_TFTPS_ON_WRITE_REQUEST_SCRIPT);
            }
         
            result = FNET_OK;
        }
        else
        {
            *error_code = FNET_TFTP_ERROR_ILLEGAL_OPERATION;
            result = FNET_ERR;
        }

    }
    else
    {
        *error_code = FNET_TFTP_ERROR_FILE_NOT_FOUND;
        result = FNET_ERR;
    }
    
    return result;
}                                                

/************************************************************************
* NAME: fapp_tftps_data_handler
*
* DESCRIPTION: TFTP server data handler.
*************************************************************************/
static int fapp_tftps_data_handler(fnet_tftp_request_t request,
                                        unsigned char *data_ptr, 
                                        unsigned short data_size, 
                                        fnet_tftp_error_t *error_code,     /* error code [0-7] RFC783, if result==FNET_ERR*/
                                        char* *error_message, /* optinal, if result==FNET_ERR*/
                                        void *shl_desc)
{                                        
    fnet_shell_desc_t desc = (fnet_shell_desc_t)shl_desc;
    int result;
    
    FNET_COMP_UNUSED_ARG(error_code);
    FNET_COMP_UNUSED_ARG(error_message);
    
    /* REQUEST_WRITE */
    if(request == FNET_TFTP_REQUEST_WRITE)
    {
            if((result = fapp_tftps_handler_control.current_type->rx_handler(&fapp_tftps_handler_control, desc, data_ptr, data_size)) == FNET_ERR)
            {
                fnet_shell_println(desc, FAPP_TFTP_FAILED);  
            }
            else
            {
                fapp_tftps_handler_control.image_size += data_size;
                
                /* Check EOF. */    
                if(data_size < FNET_TFTP_DATA_SIZE_MAX)
                {
                    fnet_shell_println(desc, FAPP_TFTP_COMPLETED_STR, fapp_tftps_handler_control.image_size);  
 
                    /* Set "go" state and save settings..*/
                    fnet_shell_script(desc, FAPP_CFG_TFTPS_AFTER_WRITE_REQUEST_SCRIPT);
                }
            }
    }
    /* REQUEST_READ */
    else 
    {

#if 0       /* Just Hello World example, only for debug.*/
        result = fnet_sprintf((char*)data_ptr, "Hello World!\n");
        fnet_shell_ptintln(desc, "Completed.");
#else
        if((result = fapp_tftps_handler_control.current_type->tx_handler(&fapp_tftps_handler_control, desc, data_ptr, data_size)) == FNET_ERR)
        {
            fnet_shell_println(desc, FAPP_TFTP_FAILED);  
        }
        else /* OK. */
        {
            fapp_tftps_handler_control.image_size += result; 
            
            /* Check EOF. */    
            if(result < FNET_TFTP_DATA_SIZE_MAX)
            {
                fnet_shell_println(desc, FAPP_TFTP_COMPLETED_STR, fapp_tftps_handler_control.image_size);  
            }
        }
#endif            

    }
    
    return result;
}

/************************************************************************
* NAME: fapp_tftps_release
*
* DESCRIPTION: Releases TFTP server.
*************************************************************************/
void fapp_tftps_release()
{
    fnet_tftp_srv_release(fapp_tftp_srv_desc);
    fapp_tftp_srv_desc = 0;    
}

/************************************************************************
* NAME: fapp_tftps_cmd
*
* DESCRIPTION: Run TFTP server.
*************************************************************************/
void fapp_tftps_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    struct fnet_tftp_srv_params params;
    fnet_tftp_srv_desc_t tftp_srv_desc;

    if(argc == 1) /* By default, the argument is "init".*/
    {
        /* Reset parameters structure. */
        fnet_memset_zero(&params, sizeof(params));

        params.handler_param = (void *)desc;
        params.request_handler = fapp_tftps_request_handler;
        params.data_handler = fapp_tftps_data_handler;

        /* Enable TFTP server */
        tftp_srv_desc = fnet_tftp_srv_init(&params);
        if(tftp_srv_desc != FNET_ERR)
        {
            fnet_shell_println(desc, FAPP_DELIMITER_STR);
            fnet_shell_println(desc, " TFTP Server started.");
            fapp_netif_addr_print(desc, AF_SUPPORTED, fapp_default_netif, FNET_FALSE);
            fnet_shell_println(desc, FAPP_DELIMITER_STR);
            fapp_tftp_srv_desc = tftp_srv_desc;
        }
        else
        {
            fnet_shell_println(desc, FAPP_INIT_ERR, "TFTP Server");
        }
        
    }
    else if(argc == 2 && fnet_strcasecmp(&FAPP_COMMAND_RELEASE[0], argv[1]) == 0) /* [release] */
    {
        fapp_tftps_release();
    }
    else
    {
        fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1]);
    }
}


/************************************************************************
* NAME: fapp_tftps_info
*
* DESCRIPTION:
*************************************************************************/
void fapp_tftps_info(fnet_shell_desc_t desc)
{
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "TFTP Server",
                fnet_tftp_srv_state(fapp_tftp_srv_desc) != FNET_TFTP_SRV_STATE_DISABLED ? FAPP_SHELL_INFO_ENABLED : FAPP_SHELL_INFO_DISABLED);
}


#endif







