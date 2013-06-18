/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
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
* @file fapp_ping.c
*
* @author Andrey Butok
*
* @date Feb-5-2013
*
* @version 0.1.9.0
*
* @brief FNET Shell Demo implementation (Ping).
*
***************************************************************************/

#include "fapp.h"
#include "fapp_prv.h"


#if FNET_CFG_PING && (FAPP_CFG_PING_CMD || FAPP_CFG_PING6_CMD)

#include "fapp_ping.h"

/************************************************************************
*     Definitions.
*************************************************************************/
#define FAPP_PING_BUFFER_SIZE       (100)
#define FAPP_PING_IDENTIFIER        (1)

#define FAPP_PING_STR_REPLY         "Reply from %s"        
#define FAPP_PING_STR_TIMEOUT       "Request timed out."


/************************************************************************
*     Function Prototypes
*************************************************************************/
static void fapp_ping_handler(int result, unsigned long packet_count, struct sockaddr *target_addr, long cookie);
static void fapp_ping_on_ctrlc(fnet_shell_desc_t desc);

/************************************************************************
* NAME: fapp_ping_handler
*
* DESCRIPTION:
************************************************************************/
static void fapp_ping_handler (int result, unsigned long packet_count, struct sockaddr *target_addr, long cookie)
{
    char                ip_str[FNET_IP_ADDR_STR_SIZE];
    fnet_shell_desc_t   desc = (fnet_shell_desc_t)cookie;
    
    if(result == FNET_OK)
    {
        fnet_shell_println(desc, FAPP_PING_STR_REPLY, fnet_inet_ntop(target_addr->sa_family, target_addr->sa_data, ip_str, sizeof(ip_str)));
    }    
    else if(result == FNET_ERR_TIMEDOUT)
    {
        fnet_shell_println(desc, FAPP_PING_STR_TIMEOUT);
    }
    else
    {
        fnet_shell_println(desc, "Error = %d", result);
    }   
         
    if(packet_count == 0)
    {
        fnet_shell_unblock(desc); /* Unblock the shell. */    
    } 
}

/************************************************************************
* NAME: fapp_ping_on_ctrlc
*
* DESCRIPTION:
************************************************************************/
static void fapp_ping_on_ctrlc(fnet_shell_desc_t desc)
{
    /* Release PING. */
    fnet_ping_release();
    fnet_shell_println( desc, FAPP_CANCELLED_STR);  
}

/************************************************************************
* NAME: fapp_ping_cmd
*
* DESCRIPTION: Ping command. 
************************************************************************/
void fapp_ping_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    struct fnet_ping_params ping_params;
    int                     i;
    char                    *p;
    unsigned long           value;
    char                    ip_str[FNET_IP_ADDR_STR_SIZE];
    
    
    fnet_memset_zero(&ping_params, sizeof(ping_params));
    
    ping_params.cookie = desc;
    ping_params.handler = fapp_ping_handler;
    ping_params.packet_size = FAPP_PING_DEFAULT_SIZE;
    ping_params.timeout = FAPP_PING_DEFAULT_TIMEOUT;
    ping_params.pattern = FAPP_PING_DEFAULT_PATTERN;
    ping_params.ttl = FAPP_PING_DEFAULT_HOP_LIMIT;
    ping_params.packet_count = FAPP_PING_DEFAULT_NUMBER;
    
    /* Last parameter must be ip address.*/
    i = argc-1;
    if(fnet_inet_ptos(argv[i], &ping_params.target_addr) == FNET_ERR)
    {
        goto ERROR_PARAMETER;
    }
    else
    {
        //TBD Optimise parameters parsing.
        if(argc > 2) /* There are additional parameters */
        { 
            /* [-c <count>][-i <seconds>]\n\t[-p <pattern>][-s <size>][-h <hoplimit/ttl>] */
            for(i=1; i<argc-1; i++)
            {
                if (!fnet_strcmp(argv[i], "-c"))
                {
                    i++;
                    value = fnet_strtoul(argv[i], &p, 10);
		            if ((value == 0) && (p == argv[i]))
		            {
		                goto ERROR_PARAMETER;
		            }
		            else
		                ping_params.packet_count = value;
                }
                else if (!fnet_strcmp(argv[i], "-i"))
                {
                    i++;
                    value = fnet_strtoul(argv[i], &p, 10);
		            if ((value == 0) && (p == argv[i]))
		            {
		                goto ERROR_PARAMETER;
		            }
		            else
		                ping_params.timeout = value*1000;
                }
                else if (!fnet_strcmp(argv[i], "-p"))
                {
                    i++;
                    value = fnet_strtoul(argv[i], &p, 10);
		            if ((value == 0) && (p == argv[i]))
		            {
		                goto ERROR_PARAMETER;
		            }
		            else
		                ping_params.pattern = (unsigned char)value;
                }
                else if (!fnet_strcmp(argv[i], "-s"))
                {
                    i++;
                    value = fnet_strtoul(argv[i], &p, 10);
		            if ((value == 0) && (p == argv[i]))
		            {
		                goto ERROR_PARAMETER;
		            }
		            else
		                ping_params.packet_size = (unsigned short)value;
                }
                else if (!fnet_strcmp(argv[i], "-h"))
                {
                    i++;
                    value = fnet_strtoul(argv[i], &p, 10);
		            if ((value == 0) && (p == argv[i]))
		            {
		                goto ERROR_PARAMETER;
		            }
		            else
		                ping_params.ttl = (unsigned char)value;
                }
                else if (!fnet_strcmp(argv[i], "-n"))
                {
                    /* Just ignore  the -n parameter.*/
                }
                else if (!fnet_strcmp(argv[i], "-I"))
                {
                    i++;
                    /* Just ignore  the -I parameter and its value.*/
                }                
                else /* Wrong parameter.*/
                {
                    goto ERROR_PARAMETER;
                }
            }
        }
       
        if(fnet_ping_request(&ping_params) == FNET_OK)
        {
            fnet_shell_println(desc, FAPP_DELIMITER_STR);
            fnet_shell_println(desc, " PING" );
            fnet_shell_println(desc, FAPP_DELIMITER_STR);
            fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "Remote IP addr", fnet_inet_ntop(ping_params.target_addr.sa_family, ping_params.target_addr.sa_data, ip_str, sizeof(ip_str)));
            fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Message Size", ping_params.packet_size>FNET_CFG_PING_PACKET_MAX?FNET_CFG_PING_PACKET_MAX:ping_params.packet_size);
            fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Num. of messages", ping_params.packet_count);
            fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Pattern", ping_params.pattern);    
            fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Hoplimit (TTL)", ping_params.ttl);      
            fnet_shell_println(desc, FAPP_TOCANCEL_STR);
            fnet_shell_println(desc, FAPP_DELIMITER_STR);

            fnet_shell_block(desc, fapp_ping_on_ctrlc); /* Block shell. */
        }
        else
        {
            fnet_shell_println(desc, FAPP_INIT_ERR, "PING");
        }
    }

    return;
    
ERROR_PARAMETER:
    fnet_shell_println(desc, FAPP_PARAM_ERR, argv[i]);
    return;    
}

#endif /* FAPP_CFG_PING_CMD || FAPP_CFG_PING6_CMD */






