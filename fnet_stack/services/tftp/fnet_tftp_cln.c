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
* @file fnet_tftp_cln.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.29.0
*
* @brief TFTP Client implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_TFTP_CLN

#include "fnet_tftp_cln.h"
#include "fnet_timer.h"
#include "fnet_eth.h"
#include "fnet_socket.h"
#include "fnet_debug.h"
#include "fnet_netif_prv.h"
#include "fnet_stdlib.h"
#include "fnet_debug.h"

#if FNET_CFG_DEBUG_TFTP_CLN    
    #define FNET_DEBUG_TFTP   FNET_DEBUG
#else
    #define FNET_DEBUG_TFTP(...)
#endif

/************************************************************************
*     Definitions
*************************************************************************/
#define FNET_TFTP_MODE                  "octet"

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
#define FNET_TFTP_ERR_IS_INITIALIZED    "ERROR: TFTP is already initialized."


static void fnet_tftp_cln_state_machine(void *fnet_tftp_cln_if_p);

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


#if 0 /* Actually it's not needed as a TFT server sends error string message with packet. */
    const static char *fnet_tftp_error[] =
    {
        "Not defined, see error message (if any).",
        "File not found.",
        "Access violation.",
        "Disk full or allocation exceeded.",
        "Illegal TFTP operation.",
        "Unknown transfer ID.",
        "File already exists.",
        "No such user."
    };
#endif

/* Default error message. */
static char FNET_TFTP_DEFAULT_ERROR[] = "Connection failed";

		
/************************************************************************
*    TFTP-client interface structure
*************************************************************************/
typedef struct
{
    SOCKET socket_client;
    fnet_poll_desc_t service_descriptor;
    fnet_tftp_request_t request_type;
    fnet_tftp_cln_state_t state;        /* Current state.*/
    struct sockaddr server_addr;           /* TFTP Server address. */
    fnet_tftp_cln_handler_t handler;    /* Callback function. */
    void *handler_param;                /* Handler specific parameter. */
    unsigned short server_port;         /* TFTP Server port number for data transfer. */    
    unsigned short block_number_ack;    /* Acknoladged block number. */
    unsigned long last_time;            /* Last receive time, used for timeout detection. */
    unsigned long timeout;              /* Timeout in ms. */
    union
    {
        struct fnet_tftp_packet_request packet_request;
        struct fnet_tftp_packet_data    packet_data;
        struct fnet_tftp_packet_ack     packet_ack;
        struct fnet_tftp_packet_error   packet_error;
    };
    unsigned long packet_size;

    int tx_data_size;
} 
fnet_tftp_if_t;

/* TFTP-client interface */
static fnet_tftp_if_t fnet_tftp_if;

/************************************************************************
* NAME: fnet_tftp_cln_init
*
* DESCRIPTION: TFTP-client initialization.
************************************************************************/
int fnet_tftp_cln_init( struct fnet_tftp_cln_params *params )
{
    struct sockaddr addr_client;
    unsigned char *data_ptr;
  
    /* Check input parameters. */
    if((params == 0) || (params->server_addr.sa_family == AF_UNSPEC) || 
        fnet_socket_addr_is_unspecified(&params->server_addr) || 
        (params->file_name == 0) || (params->handler == 0))
    {
        FNET_DEBUG_TFTP(FNET_TFTP_ERR_PARAMS);
        goto ERROR;
    }

    if(fnet_tftp_if.state != FNET_TFTP_CLN_STATE_DISABLED)
    {
        FNET_DEBUG_TFTP(FNET_TFTP_ERR_IS_INITIALIZED);
        goto ERROR;
    }
   
    /* Reset interface structure. */
    fnet_memset_zero(&fnet_tftp_if, sizeof(fnet_tftp_if_t)); 
    
    fnet_tftp_if.server_addr = params->server_addr;
    
    if(fnet_tftp_if.server_addr.sa_port == 0)
        fnet_tftp_if.server_addr.sa_port = FNET_CFG_TFTP_CLN_PORT; /* Default TFTP server port number.*/
    
    fnet_tftp_if.handler = params->handler;
    fnet_tftp_if.handler_param = params->handler_param;
    
    if(params->timeout == 0 )
    {
        fnet_tftp_if.timeout = FNET_CFG_TFTP_CLN_TIMEOUT*1000;
    }
    else
    {
        fnet_tftp_if.timeout = (unsigned long) params->timeout*1000;
    }


    /* Create client socket */
    if((fnet_tftp_if.socket_client = socket(params->server_addr.sa_family, SOCK_DGRAM, 0)) == SOCKET_INVALID)
    {
        FNET_DEBUG_TFTP(FNET_TFTP_ERR_SOCKET_CREATION);
        goto ERROR;
    }
    
    /* Bind socket (optional) */
    fnet_memset_zero(&addr_client, sizeof(addr_client));
    addr_client.sa_family = params->server_addr.sa_family;
    
    if ( bind( fnet_tftp_if.socket_client, (struct sockaddr*)&addr_client, sizeof(addr_client)) == SOCKET_ERROR  )
    {
        FNET_DEBUG_TFTP(FNET_TFTP_ERR_SOCKET_BIND);
        goto ERROR_1;
    }

    fnet_tftp_if.request_type = params->request_type;
    
    /* Prepare request. */
    switch(fnet_tftp_if.request_type)
    {
        case FNET_TFTP_REQUEST_WRITE:
            fnet_tftp_if.packet_request.opcode = FNET_HTONS(FNET_TFTP_OPCODE_WRITE_REQUEST);
            break;
        case FNET_TFTP_REQUEST_READ:
            fnet_tftp_if.packet_request.opcode = FNET_HTONS(FNET_TFTP_OPCODE_READ_REQUEST);
            break;
        default:
            goto ERROR_1;        
    }    

    data_ptr = fnet_tftp_if.packet_request.filename_mode;
    fnet_strncpy( (char*)data_ptr, params->file_name, FNET_TFTP_FILENAME_SIZE_MAX);
    data_ptr += fnet_strlen(params->file_name)+1;
    fnet_strncpy((char*)data_ptr,FNET_TFTP_MODE, FNET_TFTP_MODE_SIZE_MAX);
	
    fnet_tftp_if.packet_size = sizeof(fnet_tftp_if.packet_request.opcode)+fnet_strlen(params->file_name)+1+sizeof(FNET_TFTP_MODE);
	

    /* Register TFTP service. */
    fnet_tftp_if.service_descriptor = fnet_poll_service_register(fnet_tftp_cln_state_machine, (void *) &fnet_tftp_if);
    if(fnet_tftp_if.service_descriptor == (fnet_poll_desc_t)FNET_ERR)
    {
        FNET_DEBUG_TFTP(FNET_TFTP_ERR_SERVICE);
        goto ERROR_1;
    }
    
    fnet_tftp_if.state = FNET_TFTP_CLN_STATE_SEND_REQUEST; /* => Send REQUEST */
    
    
    return FNET_OK;
ERROR_1:
    closesocket(fnet_tftp_if.socket_client);

ERROR:
    return FNET_ERR;



}

/************************************************************************
* NAME: fnet_tftp_cln_state_machine
*
* DESCRIPTION: TFTP-client state machine.
************************************************************************/
static void fnet_tftp_cln_state_machine( void *fnet_tftp_cln_if_p )
{
    struct sockaddr addr;
    int addr_len;
    int sent_size;
    int received;    
    fnet_tftp_if_t *tftp_if = (fnet_tftp_if_t *)fnet_tftp_cln_if_p;

    switch(tftp_if->state)
    {
        /*---- SEND_REQUEST --------------------------------------------*/
        case FNET_TFTP_CLN_STATE_SEND_REQUEST:
            addr = fnet_tftp_if.server_addr;
            
            /* ---- Send request ---- */
            sent_size = sendto(tftp_if->socket_client, (char*)&tftp_if->packet_request, (int)tftp_if->packet_size, 0,
                                &addr, sizeof(addr));

        	if (sent_size!=tftp_if->packet_size)
        	{
        		goto ERROR;
        	}	
            else
            {
                tftp_if->last_time = fnet_timer_ticks();
                tftp_if->state = FNET_TFTP_CLN_STATE_HANDLE_REQUEST;
            }		
            break;
        /*---- HANDLE_REQUEST -----------------------------------------------*/
        case  FNET_TFTP_CLN_STATE_HANDLE_REQUEST:
            /* Receive data */
            addr_len = sizeof(addr);
            
            received = recvfrom  (tftp_if->socket_client, (char*)&tftp_if->packet_data, sizeof(struct fnet_tftp_packet_data), 0,
                               &addr, &addr_len );

            if(received >= 4)
            { 
                /* Is it for us. */
                if(!fnet_socket_addr_are_equal(&addr, &tftp_if->server_addr))
			    {
				    FNET_DEBUG_TFTP( "\nWARNING: Block not from our server!" );
			    }
			    /* Error. */ 
			    else if ( tftp_if->packet_data.opcode == FNET_HTONS(FNET_TFTP_OPCODE_ERROR) )
			    {
				    tftp_if->handler(tftp_if->request_type, (unsigned char *)&tftp_if->packet_error.error_message[0], fnet_htons(tftp_if->packet_error.error_code), FNET_ERR, tftp_if->handler_param);
				    tftp_if->state = FNET_TFTP_CLN_STATE_RELEASE;
		    	}
		    	/* Received Data. */
		    	else if( (tftp_if->request_type == FNET_TFTP_REQUEST_READ) && (tftp_if->packet_data.opcode == FNET_HTONS(FNET_TFTP_OPCODE_DATA)) ) 
		    	{
				    if(tftp_if->server_port == 0)
				        tftp_if->server_port = addr.sa_port; /* Save port of the first session only. */ 

				    /* Is it our session. */
                    if(tftp_if->server_port == addr.sa_port) 
                    {
                        /* Send ACK */
                        tftp_if->packet_ack.opcode = FNET_HTONS(FNET_TFTP_OPCODE_ACK);
                        sendto(tftp_if->socket_client, (char*)&tftp_if->packet_ack, sizeof(struct fnet_tftp_packet_ack), 0,
                                        (struct sockaddr*)&addr, sizeof(addr) );
                        
                        /* Reset timeout. */
                        tftp_if->last_time = fnet_timer_ticks();
                        
                        /* Message the application. */
                        if((tftp_if->block_number_ack+1) == fnet_htons(tftp_if->packet_data.block_number))
                        {
                           
                            tftp_if->block_number_ack ++;

                            /* Call Rx handler. */
                            if(tftp_if->handler(tftp_if->request_type, (unsigned char *)&tftp_if->packet_data.data[0], (unsigned short)(received - 4), FNET_OK, tftp_if->handler_param) == FNET_ERR)
                            {
                                tftp_if->state = FNET_TFTP_CLN_STATE_RELEASE;
                                break;
                            }
                            
                            /* Check return result.*/
                            if(received < sizeof(struct fnet_tftp_packet_data)) /* EOF */
                            {
                                fnet_tftp_if.state = FNET_TFTP_CLN_STATE_RELEASE;   /* => RELEASE */
                            }
                        }
                    }
                }
                /* Received ACK. */
                else if ((tftp_if->request_type == FNET_TFTP_REQUEST_WRITE) && (tftp_if->packet_data.opcode == FNET_HTONS(FNET_TFTP_OPCODE_ACK))) 
    		    {
                    if(tftp_if->block_number_ack == 0)
                        tftp_if->server_port = addr.sa_port; /* Save port number of the first ACK only. */
        
                    /* Is it our session. */
                    if(tftp_if->server_port == addr.sa_port) 
                    {
                        if(tftp_if->block_number_ack == fnet_ntohs(tftp_if->packet_data.block_number)) /* Correct ACK. */
                        {
                            /* Last ACK. */
                            if(tftp_if->block_number_ack && (tftp_if->tx_data_size < sizeof(tftp_if->packet_data.data))) 
                            {
                                tftp_if->state = FNET_TFTP_CLN_STATE_RELEASE;
                                break;
                            }
                            else /* More data to send. */
                            {
                                tftp_if->block_number_ack++;
                                    
                                if((tftp_if->tx_data_size = tftp_if->handler(tftp_if->request_type, (unsigned char *)&tftp_if->packet_data.data[0], 
                                                                            (unsigned short)(sizeof(tftp_if->packet_data.data)), 
                                                                            FNET_OK, tftp_if->handler_param)) == FNET_ERR)
                                {
                                    tftp_if->state = FNET_TFTP_CLN_STATE_RELEASE;
                                    break;
                                }
                            }                                                                            
                        }
                        /* else: Resend last packet. */
                                                                                    
                        /* Send data. */                                            
                        tftp_if->packet_data.opcode = FNET_HTONS(FNET_TFTP_OPCODE_DATA);
                        tftp_if->packet_data.block_number = fnet_htons(tftp_if->block_number_ack);
                        sendto(tftp_if->socket_client, (char*)&tftp_if->packet_data, (4+tftp_if->tx_data_size), 0,
                                        &addr, sizeof(addr) );
                        /* Reset timeout. */
                        tftp_if->last_time = fnet_timer_ticks(); 
                    }                                                   
                
                }
                /* Wrong opration. */
                else 
                    goto ERROR;
            }
            else
            {
                /* Check error. Check timeout */
                if((received == SOCKET_ERROR)||
                    (fnet_timer_get_interval(tftp_if->last_time, fnet_timer_ticks()) > (fnet_tftp_if.timeout/FNET_TIMER_PERIOD_MS)))
                {
                    goto ERROR;
                }

            }
            break;            
        /*---- RELEASE -------------------------------------------------*/    
        case FNET_TFTP_CLN_STATE_RELEASE:
            fnet_tftp_cln_release();            
            break;            
    }

return;
    
ERROR:
    tftp_if->state = FNET_TFTP_CLN_STATE_RELEASE; /* => RELEASE */
    tftp_if->handler(tftp_if->request_type, (unsigned char *)FNET_TFTP_DEFAULT_ERROR, 0, FNET_ERR, tftp_if->handler_param);        
}

/************************************************************************
* NAME: fnet_tftp_cln_release
*
* DESCRIPTION: TFTP-client release.
************************************************************************/
void fnet_tftp_cln_release(void)
{
    if(fnet_tftp_if.state != FNET_TFTP_CLN_STATE_DISABLED)
    {
        /* Close socket. */
        closesocket(fnet_tftp_if.socket_client);
    
        /* Unregister the tftp service. */
        fnet_poll_service_unregister( fnet_tftp_if.service_descriptor );
    
        fnet_tftp_if.state = FNET_TFTP_CLN_STATE_DISABLED; 
    }
}

/************************************************************************
* NAME: fnet_tftp_cln_state
*
* DESCRIPTION: This function returns a current state of the TFTP client.
************************************************************************/
fnet_tftp_cln_state_t fnet_tftp_cln_state()
{
    return fnet_tftp_if.state;
}




#endif
