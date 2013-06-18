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
* @file fapp_http.c
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.39.0
*
* @brief FNET Shell Demo implementation (HTTP Server Shell interface).
*
***************************************************************************/

#include "fapp.h"
#include "fapp_prv.h"
#include "fapp_http.h"

#if FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP

#include "fapp_fs.h"

static fnet_http_desc_t fapp_http_desc = 0; /* HTTP service descriptor. */

unsigned long fapp_http_string_buffer_respond(char * buffer, unsigned long buffer_size, char * eof, long *cookie);


/************************************************************************
* SSI definitions
*************************************************************************/
#if FNET_CFG_HTTP_SSI

#define FAPP_HTTP_SSI_COMMAND_ECHO  "echo"
#define FAPP_HTTP_SSI_BUFFER_MAX    sizeof("00:00:00:00:00:00")

struct fapp_http_echo_variable
{
    const char *variable;   
    const char *value;
};

static const struct fapp_http_echo_variable fapp_http_echo_variables[] =
{
    {"NAME", FNET_DESCRIPTION},
    {"CPU", FNET_CPU_STR},
    {"VERSION", FNET_VERSION},
    {"DATE", FNET_BUILD_DATE},
    {"COPYRIGHT", FNET_COPYRIGHT},
    {"LICENSE", FNET_LICENSE},
    {0,0} /* End of the table.*/
};

static char fapp_http_ssi_buffer[FAPP_HTTP_SSI_BUFFER_MAX];    /* Temporary buffer for run-time SSIs. */

int fapp_http_ssi_echo_handle(char * query, long *cookie);

/* SSI table */
static const struct fnet_http_ssi fapp_ssi_table[] =
{
    {FAPP_HTTP_SSI_COMMAND_ECHO, fapp_http_ssi_echo_handle, fapp_http_string_buffer_respond},//fapp_http_ssi_echo_respond},
    {0,0,0} /* End of the table. */
};

#endif /* FNET_CFG_HTTP_SSI */

/************************************************************************
* CGI definitions
*************************************************************************/
#if FNET_CFG_HTTP_CGI

#define CGI_MAX        sizeof("({ \"time\":\"00:00:00\",\"tx\":0000000000,\"rx\":0000000000})")

int fapp_http_cgi_stdata_handle(char * query, long *cookie);
int fapp_http_cgi_graph_handle(char * query, long *cookie);
#if FNET_CFG_HTTP_POST
int fapp_http_cgi_post_handle(char * query, long *cookie);
#endif

/* CGI table */
static const struct fnet_http_cgi fapp_cgi_table[] =
{
    {"stdata.cgi", fapp_http_cgi_stdata_handle, fapp_http_string_buffer_respond},
    {"graph.cgi", fapp_http_cgi_graph_handle, fapp_http_string_buffer_respond},
#if FNET_CFG_HTTP_POST    
    {"post.cgi", fapp_http_cgi_post_handle, fapp_http_string_buffer_respond},   
#endif    
    {0, 0, 0} /* End of the table. */
};
static char fapp_http_cgi_buffer[CGI_MAX]; /* CGI Temporary buffer. */

#endif /* FNET_CFG_HTTP_CGI */

/************************************************************************
* Authentification definitions
*************************************************************************/
#if FNET_CFG_HTTP_AUTHENTICATION_BASIC
static const struct fnet_http_auth fapp_auth_table[]=
{   
    {"auth", "fnet", "freescale", FNET_HTTP_AUTH_SCHEME_BASIC},
    {0, 0, 0, FNET_HTTP_AUTH_SCHEME_NONE}
}; 
#endif  

/************************************************************************
* POST definitions.
*************************************************************************/
#if FNET_CFG_HTTP_POST

int fapp_http_post_receive (char * buffer, unsigned long buffer_size, long *cookie);

static const struct fnet_http_post fapp_post_table[]=
{   
    {"post.cgi", 0, fapp_http_post_receive, 0},
    {0, 0, 0, 0}
};

#define FAPP_HTTP_POST_BUFFER_SIZE    (50)
static char fapp_http_post_buffer[FAPP_HTTP_POST_BUFFER_SIZE+1/* For zero-termination.*/];

#endif

/************************************************************************
* NAME: fapp_http_string_buffer_respond
*
* DESCRIPTION:
*************************************************************************/
static unsigned long fapp_http_string_buffer_respond(char * buffer, unsigned long buffer_size, char * eof, long *cookie)
{
    unsigned long result = 0;
   
    char *string_buffer_ptr = (char *) *cookie;
    
    unsigned long send_size = fnet_strlen(string_buffer_ptr);
    
    
    *eof = 1; /* No aditional send by default. */
    
    if(buffer && buffer_size)
    {
	    if(send_size>buffer_size)
	        send_size = buffer_size; 
	 
	    fnet_memcpy(buffer, string_buffer_ptr, send_size);

	    string_buffer_ptr += send_size;
	    if(*string_buffer_ptr!='\0') /* If any data for sending.*/ 
	        *eof = 0;    /* Need more itteration. */

	    result = send_size;
	    
	    *cookie = (long)string_buffer_ptr; /* Save cgi_buffer_ptr as cookie.*/
    }
    
    return result;    
}


/************************************************************************
* NAME: fapp_http_ssi_echo_handle
*
* DESCRIPTION:
*************************************************************************/
#if FNET_CFG_HTTP_SSI
static int fapp_http_ssi_echo_handle(char * query, long *cookie)
{
    int result = FNET_OK;
    const struct fapp_http_echo_variable *  echo_var_ptr;
    fnet_netif_desc_t netif = fapp_default_netif;
    
    const char *ssi_buffer_ptr = 0;
    
    /* Find static echo value. */
	for(echo_var_ptr = fapp_http_echo_variables; echo_var_ptr->variable && echo_var_ptr->value; echo_var_ptr++)
	{
        if (!fnet_strcmp( query, echo_var_ptr->variable))                    
        {				 
            ssi_buffer_ptr = echo_var_ptr->value;
            break;
        }
    }
   
    /* Find run-time echo values. */
    if(ssi_buffer_ptr == 0)
    {
    #if FNET_CFG_IP4
        char ip_str[FNET_IP4_ADDR_STR_SIZE];
    #endif

        ssi_buffer_ptr = fapp_http_ssi_buffer; 
        if (!fnet_strcmp( query, "IP_ADDRESS"))
        {
        #if FNET_CFG_IP4
            fnet_ip4_addr_t ip_adr = fnet_netif_get_ip4_addr(netif);
            fnet_inet_ntoa(*(struct in_addr *)( &ip_adr), ip_str);
            fnet_snprintf(fapp_http_ssi_buffer, sizeof(fapp_http_ssi_buffer), "%s", ip_str);
        #else
            fnet_snprintf(fapp_http_ssi_buffer, sizeof(fapp_http_ssi_buffer), "...");            
        #endif /* FNET_CFG_IP4 */
        }
        else if (!fnet_strcmp( query, "SUBNET_MASK"))
        {
        #if FNET_CFG_IP4
            fnet_ip4_addr_t ip_adr = fnet_netif_get_ip4_subnet_mask(netif);
            fnet_inet_ntoa(*(struct in_addr *)( &ip_adr), ip_str);
            fnet_snprintf(fapp_http_ssi_buffer, sizeof(fapp_http_ssi_buffer), "%s", ip_str);
        #else
            fnet_snprintf(fapp_http_ssi_buffer, sizeof(fapp_http_ssi_buffer), "...");            
        #endif /* FNET_CFG_IP4 */            
        }
        else if (!fnet_strcmp( query, "GATEWAY"))
        {
        #if FNET_CFG_IP4
            fnet_ip4_addr_t ip_adr = fnet_netif_get_ip4_gateway(netif);
            fnet_inet_ntoa(*(struct in_addr *)( &ip_adr), ip_str);
            fnet_snprintf(fapp_http_ssi_buffer, sizeof(fapp_http_ssi_buffer), "%s", ip_str);
        #else
            fnet_snprintf(fapp_http_ssi_buffer, sizeof(fapp_http_ssi_buffer), "...");            
        #endif /* FNET_CFG_IP4 */            
        }
        else if (!fnet_strcmp( query, "MAC"))
        {
            fnet_mac_addr_t macaddr;
            char mac_str[FNET_MAC_ADDR_STR_SIZE];
            fnet_netif_get_hw_addr(netif, macaddr, sizeof(fnet_mac_addr_t));
            fnet_mac_to_str(macaddr, mac_str);
            fnet_snprintf(fapp_http_ssi_buffer, sizeof(fapp_http_ssi_buffer), "%s", mac_str);
        }
        else
        {
            result = FNET_ERR;
        }
    }
    
    *cookie = (long)ssi_buffer_ptr; /* Save ssi_buffer_ptr as cookie.*/
    
    return result;
}
#endif /*FNET_CFG_HTTP_SSI*/



#if FNET_CFG_HTTP_CGI
/************************************************************************
* NAME: fapp_http_cgi_stdata_handle
*
* DESCRIPTION:
*************************************************************************/
static int fapp_http_cgi_stdata_handle(char * query, long *cookie)
{
    unsigned long time, t_hour, t_min, t_sec;
	struct fnet_netif_statistics statistics;

    FNET_COMP_UNUSED_ARG(query);
    
	/* Get Time. */    
	time = fnet_timer_ticks();
	t_hour = time/FNET_TIMER_TICK_IN_HOUR;
	t_min  = (time%FNET_TIMER_TICK_IN_HOUR)/FNET_TIMER_TICK_IN_MIN;
	t_sec  = (time%FNET_TIMER_TICK_IN_MIN)/FNET_TIMER_TICK_IN_SEC;
	
	/* Get statistics. */
    fnet_memset_zero( &statistics, sizeof(struct fnet_netif_statistics) );
    fnet_netif_get_statistics(fapp_default_netif, &statistics);

	/* Write to the temprorary buffer. */
    fnet_snprintf(fapp_http_cgi_buffer, sizeof(fapp_http_cgi_buffer), "({\"time\":\"%02d:%02d:%02d\",\"tx\":%d,\"rx\":%d})", 
                             t_hour, t_min, t_sec, statistics.tx_packet, statistics.rx_packet);

    *cookie = (long)fapp_http_cgi_buffer; /* Save fapp_http_cgi_buffer as cookie.*/
                                 
    return FNET_OK;
}

#define FAPP_HTTP_CGI_GRAPH_MIN     (30)
static int fapp_http_cgi_rand_next;
/************************************************************************
* NAME: fapp_http_cgi_rand
*
* DESCRIPTION:
*************************************************************************/
static unsigned int fapp_http_cgi_rand(void)
{
  fapp_http_cgi_rand_next = fapp_http_cgi_rand_next * 11 + 12;
  return (unsigned int)(((fapp_http_cgi_rand_next>>4) & 0xFF) + FAPP_HTTP_CGI_GRAPH_MIN);
}

/************************************************************************
* NAME: fapp_http_cgi_graph_handle
*
* DESCRIPTION:
*************************************************************************/
static int fapp_http_cgi_graph_handle(char * query, long *cookie)
{
    int q1= (int)fapp_http_cgi_rand();
    int q2= (int)fapp_http_cgi_rand();
    int q3= (int)fapp_http_cgi_rand();
    int q4= (int)fapp_http_cgi_rand();
    int q5= (int)fapp_http_cgi_rand();

    FNET_COMP_UNUSED_ARG(query);

	/* Wrie to the temprorary buffer. */
    fnet_snprintf(fapp_http_cgi_buffer, sizeof(fapp_http_cgi_buffer), 
                        "({\"q1\":%d,\"q2\":%d,\"q3\":%d,\"q4\":%d,\"q5\":%d})", 
                        q1, q2, q3, q4, q5);

    *cookie = (long)fapp_http_cgi_buffer; /* Save fapp_http_cgi_buffer as cookie.*/                        
    
    return FNET_OK;
}

#if FNET_CFG_HTTP_POST
/************************************************************************
* NAME: fapp_http_cgi_post_handle
*
* DESCRIPTION:
*************************************************************************/
static int fapp_http_cgi_post_handle(char * query, long *cookie)
{
    FNET_COMP_UNUSED_ARG(query);

    *cookie = (long)fapp_http_post_buffer; /* Save fapp_http_post_buffer as cookie.*/                        
    
    return FNET_OK;
}
#endif

#endif /*FNET_CFG_HTTP_CGI*/


#if FNET_CFG_HTTP_POST
/************************************************************************
* NAME: fapp_http_post_receive
*
* DESCRIPTION:
*************************************************************************/
static int fapp_http_post_receive (char *buffer, unsigned long buffer_size, long *cookie)
{
    long post_buffer_index = *cookie;
    long post_buffer_free_size = FAPP_HTTP_POST_BUFFER_SIZE - post_buffer_index;
    long receive_size = (long)buffer_size;
    
    if(post_buffer_free_size)
    {
        if(receive_size > post_buffer_free_size)
	        receive_size = post_buffer_free_size; 
	 
	    fnet_memcpy(&fapp_http_post_buffer[post_buffer_index], buffer, (unsigned)receive_size);
	    post_buffer_index += receive_size;
	    fapp_http_post_buffer[post_buffer_index] = '\0';
    
        *cookie = post_buffer_index; /* Save buffer index as cookie.*/
    }
    
    return FNET_OK;
}

#endif /*FNET_CFG_HTTP_POST*/

/************************************************************************
* NAME: fapp_http_release
*
* DESCRIPTION: Releases HTTP server.
*************************************************************************/
void fapp_http_release()
{
    fnet_http_release(fapp_http_desc);
    fapp_http_desc = 0;    
}

/************************************************************************
* NAME: fapp_http_cmd
*
* DESCRIPTION: Run HTTP server.
*************************************************************************/
void fapp_http_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    struct fnet_http_params params;
    fnet_http_desc_t http_desc;

    FNET_COMP_UNUSED_ARG(desc);

    if(argc == 1) /* By default is "init".*/
    {
        fnet_memset_zero(&params, sizeof(struct fnet_http_params));
        
        params.root_path = FAPP_HTTP_MOUNT_NAME;    /* Root directory path */
        params.index_path = FAPP_HTTP_INDEX_FILE;   /* Index file path, relative to the root_path */
    #if FNET_CFG_HTTP_SSI
        params.ssi_table = fapp_ssi_table;
    #endif
    #if FNET_CFG_HTTP_CGI            
        params.cgi_table = fapp_cgi_table;
    #endif        
    #if FNET_CFG_HTTP_AUTHENTICATION_BASIC
        params.auth_table = fapp_auth_table;
    #endif  
    #if FNET_CFG_HTTP_POST
        params.post_table = fapp_post_table;
    #endif       

        /* Enable HTTP server */
        http_desc = fnet_http_init(&params);
        if(http_desc != FNET_ERR)
        {
            fnet_shell_println(desc, FAPP_DELIMITER_STR);
            fnet_shell_println(desc, " HTTP server started.");
            fapp_netif_addr_print(desc, AF_SUPPORTED, fapp_default_netif, FNET_FALSE);
            fnet_shell_println(desc, FAPP_DELIMITER_STR);
            
            fapp_http_desc = http_desc;
        }
        else
        {
            fnet_shell_println(desc, FAPP_INIT_ERR, "HTTP");
        }
    }
    else if(argc == 2 && fnet_strcasecmp(&FAPP_COMMAND_RELEASE[0], argv[1]) == 0) /* [release] */
    {
        fapp_http_release();
    }
    else
    {
        fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1]);
    }
}

/************************************************************************
* NAME: fapp_http_info
*
* DESCRIPTION:
*************************************************************************/
void fapp_http_info(fnet_shell_desc_t desc)
{
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "HTTP Server", 
                        fnet_http_enabled(fapp_http_desc) ? FAPP_SHELL_INFO_ENABLED : FAPP_SHELL_INFO_DISABLED);
}


#endif /* FAPP_CFG_HTTP_CMD */
