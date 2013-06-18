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
* @file fnet_tftp_srv.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.25.0
*
* @brief TFTP Server implementation.
*
***************************************************************************/



#include "fnet_config.h"

#if FNET_CFG_TFTP_SRV

#include "fnet_tftp_srv.h"
#include "fnet_timer.h"
#include "fnet_eth.h"
#include "fnet_socket.h"
#include "fnet_debug.h"
#include "fnet_netif_prv.h"
#include "fnet_stdlib.h"
#include "fnet_debug.h"

#if FNET_CFG_DEBUG_TFTP_SRV    
    #define FNET_DEBUG_TFTP_SRV   FNET_DEBUG
#else
    #define FNET_DEBUG_TFTP_SRV(...)
#endif

/************************************************************************
*     Definitions
*************************************************************************/
#define FNET_TFTP_MODE_SIZE_MAX         (9)
#define FNET_TFTP_FILENAME_SIZE_MAX     (FNET_TFTP_DATA_SIZE_MAX-FNET_TFTP_MODE_SIZE_MAX)


#define FNET_TFTP_OPCODE_READ_REQUEST   (1)
#define FNET_TFTP_OPCODE_WRITE_REQUEST  (2)
#define FNET_TFTP_OPCODE_DATA           (3)
#define FNET_TFTP_OPCODE_ACK            (4)
#define FNET_TFTP_OPCODE_ERROR          (5)


#define FNET_TFTP_ERR_PARAMS            "ERROR: Wrong input parameters."
#define FNET_TFTP_ERR_SOCKET_CREATION   "ERROR: Socket creation error."
#define FNET_TFTP_ERR_SOCKET_BIND       "ERROR: Socket Error during bind."
#define FNET_TFTP_ERR_SERVICE           "ERROR: Service registration is failed."
#define FNET_TFTP_ERR_IS_INITIALIZED    "ERROR: TFTP Server is already initialized."




/* TFTP packets:*/
FNET_COMP_PACKED_BEGIN
struct fnet_tftp_packet_request
{
	unsigned short opcode FNET_COMP_PACKED;
	unsigned char  filename_mode[FNET_TFTP_DATA_SIZE_MAX] FNET_COMP_PACKED; /* Filename, Mode */
};
FNET_COMP_PACKED_END

FNET_COMP_PACKED_BEGIN
struct fnet_tftp_packet_data
{
	unsigned short opcode FNET_COMP_PACKED;
	unsigned short block_number FNET_COMP_PACKED;
	unsigned char data[FNET_TFTP_DATA_SIZE_MAX] FNET_COMP_PACKED;
};
FNET_COMP_PACKED_END

FNET_COMP_PACKED_BEGIN
struct fnet_tftp_packet_ack
{
	unsigned short opcode FNET_COMP_PACKED;
	unsigned short block_number FNET_COMP_PACKED;
};
FNET_COMP_PACKED_END

FNET_COMP_PACKED_BEGIN
struct fnet_tftp_packet_error
{
	unsigned short opcode FNET_COMP_PACKED;
	unsigned short error_code FNET_COMP_PACKED;
	char error_message[FNET_TFTP_DATA_SIZE_MAX] FNET_COMP_PACKED;
};
FNET_COMP_PACKED_END



const static char *fnet_tftp_srv_error[] =
{
   "Not defined", /* see error message (if any).*/
   "File not found",
   "Access violation",
   "Disk full or allocation exceeded",
   "Illegal TFTP operation",
   "Unknown transfer ID",
   "File already exists",
   "No such user"
};
    
#define FNET_TFTP_SRV_ERR_MAX   (sizeof(fnet_tftp_srv_error)/sizeof(char*))


/************************************************************************
*    TFTP server interface structure
*************************************************************************/
struct fnet_tftp_srv_if
{
    fnet_tftp_srv_state_t   state;                  /* Current state.*/
    SOCKET                  socket_listen;          /* Listening socket.*/
    SOCKET                  socket_transaction;     /* Socket servicing transaction.*/
    struct sockaddr         addr_transaction;
    
    fnet_poll_desc_t        service_descriptor;
    
    fnet_tftp_srv_request_handler_t     request_handler;
    fnet_tftp_srv_data_handler_t        data_handler;   
    fnet_tftp_srv_complete_handler_t    complete_handler;
    int                     complete_status;        /* FNET_OK or FNET_ERR */                                      
    void                    *handler_param;         /* Handler specific parameter. */
    
    fnet_tftp_request_t     request_type;
    void                    (*request_send)(struct fnet_tftp_srv_if *tftp_srv_if);
    
    unsigned short          block_number_ack;       /* Acknoladged block number. */
    unsigned long           last_time;              /* Last receive time, used for timeout detection. */
    unsigned long           timeout;                /* Timeout in ms. */
    unsigned int            retransmit_max;
    unsigned int            retransmit_cur;
    union
    {
        struct fnet_tftp_packet_request packet_request;
        struct fnet_tftp_packet_data    packet_data;
        struct fnet_tftp_packet_ack     packet_ack;
        struct fnet_tftp_packet_error   packet_error;
    };
    unsigned long           packet_size;

    int                     tx_data_size;
}; 

/* The TFTP Server interface */ 
static struct fnet_tftp_srv_if tftp_srv_if_list[FNET_CFG_TFTP_SRV_MAX];

/************************************************************************
*     Function Prototypes
*************************************************************************/
static void fnet_tftp_srv_state_machine(void *tftp_srv_if_p);
static void fnet_tftp_srv_send_error(struct fnet_tftp_srv_if *tftp_srv_if, SOCKET s, unsigned short error_code, const char *error_message, struct sockaddr *dest_addr);


/************************************************************************
* NAME: fnet_tftp_srv_init
*
* DESCRIPTION: TFTP server initialization.
************************************************************************/
fnet_tftp_srv_desc_t fnet_tftp_srv_init( struct fnet_tftp_srv_params *params )
{
    struct sockaddr         local_addr;
    int                     i;
    struct fnet_tftp_srv_if *tftp_srv_if = 0;
 
    /* Check input paramters. */
    if((params == 0) || (params->data_handler == 0) || (params->request_handler==0))
    {
        FNET_DEBUG_TFTP_SRV("TFTP_SRV: Wrong init parameters.");
        goto ERROR_1;
    }

    /* Try to find free TFTP server descriptor. */
    for(i=0; i < FNET_CFG_TFTP_SRV_MAX; i++)
    {
        if(tftp_srv_if_list[i].state == FNET_TFTP_SRV_STATE_DISABLED)
        {
            tftp_srv_if = &tftp_srv_if_list[i]; 
        }
    }
    
    if(tftp_srv_if == 0)
    {
        /* No free TFTP server descriptor. */
        FNET_DEBUG_TFTP_SRV("TFTP_SRV: No free TFTP Server.");
        goto ERROR_1;
    }
    
    /* Reset interface structure. */
    fnet_memset_zero(tftp_srv_if, sizeof(struct fnet_tftp_srv_if)); 
    
    tftp_srv_if->request_handler    = params->request_handler;
    tftp_srv_if->data_handler       = params->data_handler;
    tftp_srv_if->complete_handler   = params->complete_handler; 
    tftp_srv_if->handler_param      = params->handler_param; 
    
    if(params->timeout == 0 )
        tftp_srv_if->timeout = FNET_CFG_TFTP_SRV_TIMEOUT;
    else
        tftp_srv_if->timeout = (unsigned long) params->timeout;
    tftp_srv_if->timeout = tftp_srv_if->timeout*1000/FNET_TIMER_PERIOD_MS;
    
    if(params->retransmit_max == 0 )
        tftp_srv_if->retransmit_max = FNET_CFG_TFTP_SRV_RETRANSMIT_MAX;
    else
        tftp_srv_if->retransmit_max = params->retransmit_max;

    
    tftp_srv_if->socket_transaction = SOCKET_INVALID;

    
    local_addr = params->address;
 
    if(local_addr.sa_port == 0)
        local_addr.sa_port = FNET_CFG_TFTP_SRV_PORT; /* Aply the default port. */
    
    if(local_addr.sa_family == AF_UNSPEC)
        local_addr.sa_family = AF_SUPPORTED; /* Asign supported families.*/

    /* Create listen socket */
    if((tftp_srv_if->socket_listen = socket(local_addr.sa_family, SOCK_DGRAM, 0)) == SOCKET_INVALID)
    {
        FNET_DEBUG_TFTP_SRV("TFTP_SRV: Socket creation error.");
        goto ERROR_1;
    }
    
    if(bind(tftp_srv_if->socket_listen, &local_addr, sizeof(local_addr)) == SOCKET_ERROR)
    {
        FNET_DEBUG_TFTP_SRV("TFTP_SRV: Socket bind error.");
        goto ERROR_2;
    }

    /* Register service. */
    tftp_srv_if->service_descriptor = fnet_poll_service_register(fnet_tftp_srv_state_machine, (void *) tftp_srv_if);
    
    if(tftp_srv_if->service_descriptor == (fnet_poll_desc_t)FNET_ERR)
    {
        FNET_DEBUG_TFTP_SRV("TFTP_SRV: Service registration error.");
        goto ERROR_2;
    }
 
 
    tftp_srv_if->state = FNET_TFTP_SRV_STATE_WAITING_REQUEST; /* => Send WAITING_REQUEST */

    return (fnet_tftp_srv_desc_t)tftp_srv_if;
ERROR_2:
    closesocket(tftp_srv_if->socket_listen);    
ERROR_1:
    return FNET_ERR;
}

/************************************************************************
* NAME: fnet_tftp_srv_send_error
*
* DESCRIPTION: Send TFTP error packet.
************************************************************************/
static void fnet_tftp_srv_send_error(struct fnet_tftp_srv_if *tftp_srv_if, SOCKET s, unsigned short error_code, const char *error_message, struct sockaddr *dest_addr)
{
    /*        2 bytes   2 bytes        string    1 byte
             ------------------------------------------
    ERROR   |   05   |  ErrorCode   |  ErrMsg  |  0    |
             ------------------------------------------
    */

    tftp_srv_if->packet_error.opcode = FNET_HTONS(FNET_TFTP_OPCODE_ERROR);
    tftp_srv_if->packet_error.error_code = fnet_htons(error_code);
    
    if((error_message == 0) && (error_code < FNET_TFTP_SRV_ERR_MAX))
        error_message = fnet_tftp_srv_error[error_code]; /* Stanndard error message acording to RFC783. */
    
    if(error_message)
        fnet_strncpy( tftp_srv_if->packet_error.error_message, error_message, FNET_TFTP_DATA_SIZE_MAX-1 );
    else
        tftp_srv_if->packet_error.error_message[0] = 0;
    
    sendto(s, (char*)&tftp_srv_if->packet_error, (int)(fnet_strlen(error_message)+(2+2+1)), 0,
                                        dest_addr, sizeof(*dest_addr));
}

/************************************************************************
* NAME: fnet_tftp_srv_send_data
*
* DESCRIPTION: Send TFTP data packet.
************************************************************************/
static void fnet_tftp_srv_send_data(struct fnet_tftp_srv_if *tftp_srv_if)
{
    /* Send data. */                                            
    tftp_srv_if->packet_data.opcode = FNET_HTONS(FNET_TFTP_OPCODE_DATA);
    tftp_srv_if->packet_data.block_number = fnet_htons(tftp_srv_if->block_number_ack);
    sendto(tftp_srv_if->socket_transaction, (char*)&tftp_srv_if->packet_data, (4+tftp_srv_if->tx_data_size), 0,
            &tftp_srv_if->addr_transaction, sizeof(tftp_srv_if->addr_transaction) );
    /* Reset timeout. */
    tftp_srv_if->last_time = fnet_timer_ticks();        
}

/************************************************************************
* NAME: fnet_tftp_srv_send_ack
*
* DESCRIPTION: Send TFTP acknowledge packet.
************************************************************************/
static void fnet_tftp_srv_send_ack(struct fnet_tftp_srv_if *tftp_srv_if)
{
    /* Send acknowledge. */                                            
    tftp_srv_if->packet_ack.opcode = FNET_HTONS(FNET_TFTP_OPCODE_ACK);
    tftp_srv_if->packet_ack.block_number = fnet_htons(tftp_srv_if->block_number_ack);
    sendto(tftp_srv_if->socket_transaction, (char*)&tftp_srv_if->packet_ack, sizeof(struct fnet_tftp_packet_ack), 0,
            &tftp_srv_if->addr_transaction, sizeof(tftp_srv_if->addr_transaction) );
    /* Reset timeout. */
    tftp_srv_if->last_time = fnet_timer_ticks();        
}

/************************************************************************
* NAME: fnet_tftp_srv_send_ack
*
* DESCRIPTION: Call TFTP data handler.
************************************************************************/
static int fnet_tftp_srv_data_handler(struct fnet_tftp_srv_if *tftp_srv_if, unsigned short data_size)
{
    fnet_tftp_error_t   error_code = FNET_TFTP_ERROR_NOT_DEFINED;
    char                *error_message = 0;
    int                 result;

    if((result = tftp_srv_if->data_handler( tftp_srv_if->request_type,
                                        (unsigned char *)&tftp_srv_if->packet_data.data[0], 
                                        data_size,
                                        &error_code,
                                        &error_message,
                                        tftp_srv_if->handler_param)) == FNET_ERR)
    {                                
        /* Send error. */
        fnet_tftp_srv_send_error(tftp_srv_if, tftp_srv_if->socket_transaction, error_code, error_message, &tftp_srv_if->addr_transaction);
        tftp_srv_if->state = FNET_TFTP_SRV_STATE_CLOSE;   /* => CLOSE */
    }
    else                            
        tftp_srv_if->block_number_ack ++;
        
    return result;    
}

/************************************************************************
* NAME: fnet_tftp_srv_state_machine
*
* DESCRIPTION: TFTP server state machine.
************************************************************************/
static void fnet_tftp_srv_state_machine( void *fnet_tftp_srv_if_p )
{
    struct sockaddr         addr;
    int                     addr_len;      
    int                     received;    
    struct fnet_tftp_srv_if *tftp_srv_if = (struct fnet_tftp_srv_if *)fnet_tftp_srv_if_p;
    fnet_tftp_error_t       error_code;
    char                    *error_message;
    char                    *filename;        /* null-terminated.*/
    char                    *mode;            /* null-terminated.*/
    int                     i;
    int                     result;

    switch(tftp_srv_if->state)
    {
        /*---- WAITING_REQUEST --------------------------------------------*/
        case FNET_TFTP_SRV_STATE_WAITING_REQUEST:
		    addr_len = sizeof(addr);
            
            received = recvfrom(tftp_srv_if->socket_listen, (char*)&tftp_srv_if->packet_request, 
                               (int)sizeof(struct fnet_tftp_packet_request), 0,
                               &addr, &addr_len );

            if(received >= 4)
            {
                /* Extract opcode, filename and mode. */
               
                /*         2 bytes   string      1 byte   string     1 byte
                *         ----------------------------------------------------
                * RRQ/   | 01/02   |  Filename  |   0    |   Mode   |    0   |
                * WRQ     ----------------------------------------------------
                */
                result = FNET_OK;
                
                /* Set default error message. */
                error_code = FNET_TFTP_ERROR_NOT_DEFINED;
                error_message = 0; 
                
                switch(tftp_srv_if->packet_request.opcode)
                {
                    case FNET_HTONS(FNET_TFTP_OPCODE_READ_REQUEST):
                        FNET_DEBUG_TFTP_SRV("TFTP_SRV: Get Read request.");
                        tftp_srv_if->request_type = FNET_TFTP_REQUEST_READ;
                        break;
                    case FNET_HTONS(FNET_TFTP_OPCODE_WRITE_REQUEST): 
                        FNET_DEBUG_TFTP_SRV("TFTP_SRV: Get Write request.");                   
                        tftp_srv_if->request_type = FNET_TFTP_REQUEST_WRITE;
                        break;    
                    default:
                        FNET_DEBUG_TFTP_SRV("TFTP_SRV: Get wrong request (%d).", fnet_ntohs(tftp_srv_if->packet_request.opcode));                     
                        result = FNET_ERR;
                        return;    
                }
                 
                if(result == FNET_OK)
                {
                    received -= 2;
                    /* Get filename. */
                    filename = (char *)tftp_srv_if->packet_request.filename_mode;
                    for(i = 0; i < received; i++)
                    {
                        if(filename[i] == 0)
                        {
                            break; /* Found end of file name. */
                        }
                    }
                    
                    i++;
                    /* Get mode.*/
                    mode = &filename[i];
                    
                    for(; i < received; i++)
                    {
                        if(filename[i] == 0)
                        {
                            break; /* Found end of mode. */
                        }
                    }
                    
                    if( i < received)
                    {                   
                        result = tftp_srv_if->request_handler(tftp_srv_if->request_type,
                                                        &addr,
                                                        filename,        /* null-terminated.*/
                                                        mode,            /* null-terminated.*/
                                                        &error_code,     
                                                        &error_message, 
                                                        tftp_srv_if->handler_param);
                    }
                    else
                        result = FNET_ERR;    
                }
               
                if(result == FNET_OK)
                {
                    tftp_srv_if->complete_status = FNET_ERR; /* Set default value.*/
                    
                    /* Create a socket for the new transaction. */
                    if((tftp_srv_if->socket_transaction = socket(addr.sa_family, SOCK_DGRAM, 0)) == SOCKET_INVALID)
                    {
                        FNET_DEBUG_TFTP_SRV("TFTP_SRV: Socket creation error.");
                        fnet_tftp_srv_send_error(tftp_srv_if, tftp_srv_if->socket_listen, FNET_TFTP_ERROR_NOT_DEFINED, 0, &addr);
                        tftp_srv_if->state = FNET_TFTP_SRV_STATE_CLOSE;   /* => CLOSE */
                    }
                    else
                    {
                        /* Save the client address.*/
                        tftp_srv_if->addr_transaction = addr; 
                        
                        /* Bind new socket. */
                        addr.sa_port = FNET_HTONS(0);
                        fnet_memset_zero(addr.sa_data, sizeof(addr.sa_data));
    
                        if(bind(tftp_srv_if->socket_transaction, &addr, sizeof(addr)) == SOCKET_ERROR)
                        {
                            FNET_DEBUG_TFTP_SRV("TFTP_SRV: Socket bind error.");
                            fnet_tftp_srv_send_error(tftp_srv_if, tftp_srv_if->socket_listen, FNET_TFTP_ERROR_NOT_DEFINED, 0, &addr);
                            tftp_srv_if->state = FNET_TFTP_SRV_STATE_CLOSE;   /* => CLOSE */
                        }
                        else
                        {
                            tftp_srv_if->block_number_ack = 0;
                            
                            /* REQUEST_WRITE. */
                            if(tftp_srv_if->request_type == FNET_TFTP_REQUEST_WRITE)
                            {
                                tftp_srv_if->request_send = fnet_tftp_srv_send_ack;
                            }
                            /* REQUEST_READ. */
                            else 
                            {
                                /* Data handler.*/
                                if((tftp_srv_if->tx_data_size = fnet_tftp_srv_data_handler(tftp_srv_if, (unsigned short)(sizeof(tftp_srv_if->packet_data.data)))) == FNET_ERR)
                                    break;
                                
                                tftp_srv_if->request_send = fnet_tftp_srv_send_data;                                                   
                            }

                            /* Send. */                                            
                            tftp_srv_if->request_send(tftp_srv_if);
                            tftp_srv_if->state = FNET_TFTP_SRV_STATE_HANDLE_REQUEST; /* => HANDLE_REQUEST */       
                            tftp_srv_if->retransmit_cur = 0;
                        }
                    
                    }
                }
                else
                    fnet_tftp_srv_send_error(tftp_srv_if, tftp_srv_if->socket_listen, error_code, error_message, &addr);
            } 
            break;
        /*---- HANDLE_REQUEST -----------------------------------------------*/
        case  FNET_TFTP_SRV_STATE_HANDLE_REQUEST:
            addr_len = sizeof(addr); 

            received = recvfrom(tftp_srv_if->socket_transaction, (char*)&tftp_srv_if->packet_data, sizeof(struct fnet_tftp_packet_data), 0,
                                &addr, &addr_len );
           
            if(received >= 4)
            { 
                FNET_DEBUG_TFTP_SRV("TFTP_SRV:HANDLE_REQUEST");
                /* Error. */
                if ( (received == SOCKET_ERROR) || (tftp_srv_if->packet_data.opcode == FNET_HTONS(FNET_TFTP_OPCODE_ERROR)) )
    			{
    				    tftp_srv_if->state = FNET_TFTP_SRV_STATE_CLOSE;
    		    }
    		    /* Check TID. */
                else if ( (tftp_srv_if->addr_transaction.sa_port != addr.sa_port) ||
                            !fnet_socket_addr_are_equal(&tftp_srv_if->addr_transaction, &addr) )
			    {
				    FNET_DEBUG_TFTP_SRV( "\nWARNING: Block not from our server!" );
				    fnet_tftp_srv_send_error(tftp_srv_if, tftp_srv_if->socket_transaction, FNET_TFTP_ERROR_UNKNOWN_TID, 0, &addr);
				    tftp_srv_if->state = FNET_TFTP_SRV_STATE_CLOSE;
			    }
                /* Received ACK. */
                else if ((tftp_srv_if->request_type == FNET_TFTP_REQUEST_READ) && (tftp_srv_if->packet_data.opcode == FNET_HTONS(FNET_TFTP_OPCODE_ACK))) 
    			{
                    if(tftp_srv_if->block_number_ack == fnet_ntohs(tftp_srv_if->packet_data.block_number)) /* Correct ACK. */
                    {
                        /* If last ACK. */
                        if(tftp_srv_if->tx_data_size < sizeof(tftp_srv_if->packet_data.data)) 
                        {
                            tftp_srv_if->complete_status = FNET_OK;
                            tftp_srv_if->state = FNET_TFTP_SRV_STATE_CLOSE;
                            break;
                        }
                        /* More data to send. */
                        else 
                        {
                            /* Data handler.*/
                            if((tftp_srv_if->tx_data_size = fnet_tftp_srv_data_handler(tftp_srv_if, (unsigned short)(sizeof(tftp_srv_if->packet_data.data)))) == FNET_ERR)
                                break;
                                        
                            tftp_srv_if->retransmit_cur = 0;    
                        }                                                                            
                    }
                    /* else: Resend last packet. */
                             
                    /* Send. */                                            
                    tftp_srv_if->request_send(tftp_srv_if);
                }
		    	/* Received Data. */
		    	else if ((tftp_srv_if->request_type == FNET_TFTP_REQUEST_WRITE) && (tftp_srv_if->packet_data.opcode == FNET_HTONS(FNET_TFTP_OPCODE_DATA)) ) 
		    	{
                    if((tftp_srv_if->block_number_ack + 1) == fnet_ntohs(tftp_srv_if->packet_data.block_number))
                    {
                        /* Data handler.*/
                        if(fnet_tftp_srv_data_handler(tftp_srv_if, (unsigned short)(received - 4)) == FNET_ERR)
                            break;

                        /* Check return result.*/
                        if(received < sizeof(struct fnet_tftp_packet_data)) /* EOF */
                        {
                            tftp_srv_if->complete_status = FNET_OK;
                            tftp_srv_if->state = FNET_TFTP_SRV_STATE_CLOSE;   /* => CLOSE */
                        }
                        
                        tftp_srv_if->retransmit_cur = 0;
                    }
                    /* Send ACK. */
                    tftp_srv_if->request_send(tftp_srv_if);
                }                
                else /* Wrong opration. */
                {
                    fnet_tftp_srv_send_error(tftp_srv_if, tftp_srv_if->socket_transaction, FNET_TFTP_ERROR_ILLEGAL_OPERATION, 0, &addr);
                    tftp_srv_if->state = FNET_TFTP_SRV_STATE_CLOSE;
                }
		    }    
            /* Error. */
            if ( received == SOCKET_ERROR) 
            {
    	        tftp_srv_if->state = FNET_TFTP_SRV_STATE_CLOSE;
            }
            /* Check timeout */
            else if(fnet_timer_get_interval(tftp_srv_if->last_time, fnet_timer_ticks()) > (tftp_srv_if->timeout))
            {
                /* Retransmit */  
                if(tftp_srv_if->retransmit_cur < tftp_srv_if->retransmit_max)
                {
                    tftp_srv_if->retransmit_cur++;
                    tftp_srv_if->request_send(tftp_srv_if);
                }
                else
                    tftp_srv_if->state = FNET_TFTP_SRV_STATE_CLOSE;
            }
            
            break;
            /*---- CLOSING --------------------------------------------*/
        case FNET_TFTP_SRV_STATE_CLOSE:
            FNET_DEBUG_TFTP_SRV("TFTP_SRV: STATE_CLOSING");

            if(tftp_srv_if->socket_transaction != SOCKET_INVALID)
            {
                    closesocket(tftp_srv_if->socket_transaction);
                    tftp_srv_if->socket_transaction = SOCKET_INVALID;
            }
                
            /* Call complete handler. */
            if(tftp_srv_if->complete_handler)
                tftp_srv_if->complete_handler(tftp_srv_if->request_type, tftp_srv_if->complete_status, tftp_srv_if->handler_param);
                
            tftp_srv_if->state = FNET_TFTP_SRV_STATE_WAITING_REQUEST; /*=> WAITING_REQUEST */
            break;                       
    }            
}

/************************************************************************
* NAME: fnet_tftp_srv_release
*
* DESCRIPTION: TFTP server release.
************************************************************************/
void fnet_tftp_srv_release(fnet_tftp_srv_desc_t desc)
{
    struct fnet_tftp_srv_if *tftp_srv_if = (struct fnet_tftp_srv_if *)desc;
    
    if(tftp_srv_if && (tftp_srv_if->state != FNET_TFTP_SRV_STATE_DISABLED))
    {
        if(tftp_srv_if->state == FNET_TFTP_SRV_STATE_HANDLE_REQUEST)
            fnet_tftp_srv_send_error(tftp_srv_if, tftp_srv_if->socket_transaction, FNET_TFTP_ERROR_NOT_DEFINED, 0, &tftp_srv_if->addr_transaction);
        closesocket(tftp_srv_if->socket_listen);
        closesocket(tftp_srv_if->socket_transaction);
        
        fnet_poll_service_unregister(tftp_srv_if->service_descriptor); /* Delete service.*/
        tftp_srv_if->state = FNET_TFTP_SRV_STATE_DISABLED;
    }
}

/************************************************************************
* NAME: fnet_tftp_srv_state
*
* DESCRIPTION: This function returns a current state of the TFTP server.
************************************************************************/
fnet_tftp_srv_state_t fnet_tftp_srv_state(fnet_tftp_srv_desc_t desc)
{
    struct fnet_tftp_srv_if *tftp_srv_if = (struct fnet_tftp_srv_if *) desc;
    fnet_tftp_srv_state_t result;
    
    if(tftp_srv_if)
        result = tftp_srv_if->state;
    else
        result = FNET_TFTP_SRV_STATE_DISABLED;
    
    return result;
}

#endif
