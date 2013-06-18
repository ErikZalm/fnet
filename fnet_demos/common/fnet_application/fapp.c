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
* @file fapp.c
*
* @author Andrey Butok
*
* @date Feb-5-2013
*
* @version 0.1.71.0
*
* @brief FNET Shell Demo implementation.
*
***************************************************************************/

#include "fapp.h"

#include "fapp_prv.h"

#if FAPP_CFG_SETGET_CMD
#include "fapp_setget.h"
#endif

#include "fapp_mem.h"

#if FAPP_CFG_DHCP_CMD && FNET_CFG_DHCP

#include "fapp_dhcp.h"

#endif

#if (FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP)|| (FAPP_CFG_EXP_CMD && FNET_CFG_FS)

#include "fapp_http.h"
#include "fapp_fs.h"

#endif

#if FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD || FAPP_CFG_TFTPS_CMD

#include "fapp_tftp.h"

#endif

#if FAPP_CFG_TELNET_CMD && FNET_CFG_TELNET

#include "fapp_telnet.h"

#endif

#if FAPP_CFG_DNS_CMD && FNET_CFG_DNS && FNET_CFG_DNS_RESOLVER

#include "fapp_dns.h"

#endif 

#if FAPP_CFG_BENCH_CMD

#include "fapp_bench.h"

#endif

#if FNET_CFG_PING && (FAPP_CFG_PING_CMD || FAPP_CFG_PING6_CMD)

#include "fapp_ping.h"

#endif


#include "fnet_netbuf.h"

/************************************************************************
*     Definitions.
*************************************************************************/
const char FAPP_DELIMITER_STR[] = "************************************************";
const char FAPP_CANCELLED_STR[] = "\nCancelled";
const char FAPP_TOCANCEL_STR[] = "Press [Ctr+C] to cancel.";

/* Error mesages */
const char FAPP_PARAM_ERR[] = "Error: Invalid paremeter \'%s\'";
const char FAPP_NET_ERR[]   = "Error: Network Interface is not configurated!\n";
const char FAPP_INIT_ERR[]  = "Error: %s initialization is failed!";



const char FAPP_SHELL_INFO_FORMAT_S[]  = " %-16s : %s";
const char FAPP_SHELL_INFO_FORMAT_D[]  = " %-16s : %u";

const char FAPP_SHELL_INFO_ENABLED[]  = "enabled";
const char FAPP_SHELL_INFO_DISABLED[]  = "disabled";
const char FAPP_SHELL_CANCELED_CTRL_C[]  = "Canceled by [Ctrl+C]!";

/* Service release command */
const char FAPP_COMMAND_RELEASE [] = "release";

#define FAPP_GO_STR             "go 0x%08X"
#define FAPP_SAVE_STR           "Parameters saved"
#define FAPP_SAVE_FAILED_STR    "Parameters saving failed!"
#define FAPP_BOOT_STR           "Press any key to stop (%s): %3d"

#define FAPP_PARAMS_LOAD_STR    "\n\nParameters loaded from Flash.\n"

#define FAPP_DUP_IP_WARN        "\n%s: %s has IP address conflict with another system on the network!\n"


fnet_netif_desc_t fapp_default_netif; /* Default netif. */

/************************************************************************
*     Function Prototypes
*************************************************************************/
static void fapp_init(void);
void fapp_shell_init(fnet_shell_desc_t desc);
#if FAPP_CFG_BOOTLOADER || FAPP_CFG_SETGET_CMD_BOOT
static void fapp_boot_mode_go(fnet_shell_desc_t desc);
static void fapp_boot_mode_script(fnet_shell_desc_t desc);
#endif
#if FAPP_CFG_GO_CMD || FAPP_CFG_BOOTLOADER || FAPP_CFG_SETGET_CMD_BOOT
static void fapp_go (fnet_shell_desc_t desc, unsigned long address);
#endif
#if FAPP_CFG_REINIT_CMD 
void fapp_reinit_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
#endif
#if FAPP_CFG_DEBUG_CMD 
void fapp_debug_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
#endif

/************************************************************************
*     The table of the main shell commands.
*************************************************************************/
const struct fnet_shell_command fapp_cmd_table [] =
{
    { FNET_SHELL_CMD_TYPE_NORMAL, "help",       0, 0, (void *)fapp_help_cmd,    "Display this help message", ""},
#if FAPP_CFG_SETGET_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "set",        0, 2, (void *)fapp_set_cmd,     "Set parameter", "[<parameter> <value>]"},
    { FNET_SHELL_CMD_TYPE_NORMAL, "get",        0, 1, (void *)fapp_get_cmd,     "Get parameters", "[<parameter>]" },
#endif  
#if FAPP_CFG_BIND_CMD && FNET_CFG_IP6
    { FNET_SHELL_CMD_TYPE_NORMAL, "bind",       1, 1, (void *)fapp_bind_cmd,    "Bind IPv6 Address", "<IP6 address>"},
#endif
#if FAPP_CFG_UNBIND_CMD && FNET_CFG_IP6
    { FNET_SHELL_CMD_TYPE_NORMAL, "unbind",     1, 1, (void *)fapp_unbind_cmd,  "Unbind IPv6 Address", "<IP6 address>"},
#endif    
#if FAPP_CFG_INFO_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "info",       0, 0, (void *)fapp_info_cmd,    "Show detailed status", ""},
#endif
#if FAPP_CFG_DHCP_CMD && FNET_CFG_DHCP && FNET_CFG_IP4
    { FNET_SHELL_CMD_TYPE_NORMAL, "dhcp",       0, 1, (void *)fapp_dhcp_cmd,    "Start DHCP client", "[release]"},
#endif
#if FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP
    { FNET_SHELL_CMD_TYPE_NORMAL, "http",       0, 1, (void *)fapp_http_cmd,    "Start HTTP Server", "[release]"},
#endif
#if FAPP_CFG_EXP_CMD && FNET_CFG_FS
    { FNET_SHELL_CMD_TYPE_SHELL,  "exp",        0, 1, (void *)&fapp_fs_shell,   "File Explorer submenu...", ""},
#endif    
#if FAPP_CFG_TFTP_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "tftp",       0, 3, (void *)fapp_tftp_cmd,    "TFTP firmware loader", "[<image name>[<server ip>[<type>]]]"},
#endif
#if FAPP_CFG_TFTPUP_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "tftpup",     0, 3, (void *)fapp_tftp_cmd,    "TFTP firmware uploader", "[<image name>[<server ip>[<type>]]]"},
#endif  
#if FAPP_CFG_TFTPS_CMD  
    { FNET_SHELL_CMD_TYPE_NORMAL, "tftps",      0, 1, (void *)fapp_tftps_cmd,   "TFTP firmware server", "[release]"},
#endif
#if FAPP_CFG_TELNET_CMD && FNET_CFG_TELNET
    { FNET_SHELL_CMD_TYPE_NORMAL, "telnet",     0, 1, (void *)fapp_telnet_cmd,  "Start Telnet Server", "[release]"},
#endif 
#if FAPP_CFG_DNS_CMD && FNET_CFG_DNS && FNET_CFG_DNS_RESOLVER && FNET_CFG_IP4
    { FNET_SHELL_CMD_TYPE_NORMAL, "dns",        1, 1, (void *)fapp_dns_cmd,     "Resolve IP address from <host name>", "<host name>"},
#endif 
#if FAPP_CFG_MEM_CMD    
    { FNET_SHELL_CMD_TYPE_NORMAL, "mem",        0, 0, (void *)fapp_mem_cmd,     "Show memory map", ""},
#endif  
#if FAPP_CFG_ERASE_CMD    
    { FNET_SHELL_CMD_TYPE_NORMAL, "erase",      1, 2, (void *)fapp_mem_erase_cmd,   "Erase flash memory", "all|[0x<erase address> <bytes>]"},
#endif 
#if FAPP_CFG_SAVE_CMD       
    { FNET_SHELL_CMD_TYPE_NORMAL, "save",       0, 0, (void *)fapp_save_cmd,    "Save parameters to the FLASH", ""},
#endif 
#if FAPP_CFG_GO_CMD    
    { FNET_SHELL_CMD_TYPE_NORMAL, "go",         0, 1, (void *)fapp_go_cmd,      "Start application at address", "[0x<address>]"},
#endif 
#if FAPP_CFG_RESET_CMD    
    { FNET_SHELL_CMD_TYPE_NORMAL, "reset",      0, 0, (void *)fapp_reset_cmd,   "Reset the board", ""},
#endif    
#if FAPP_CFG_REBOOT_CMD /* Used only for IPv6 TAHI Testing.*/   
    { FNET_SHELL_CMD_TYPE_NORMAL, "reboot",     0, 0, (void *)fapp_reset_cmd,   "Reset the board", ""},
#endif  
#if FAPP_CFG_BENCH_CMD   
    { FNET_SHELL_CMD_TYPE_NORMAL, "benchrx",    0, 2, (void *)fapp_benchrx_cmd, "Receiver Benchmark", "[tcp|udp [multicast ip]"},
    { FNET_SHELL_CMD_TYPE_NORMAL, "benchtx",    1, 5, (void *)fapp_benchtx_cmd, "Transmitter Benchmark", "<remote ip>[tcp|udp[<message size>\r\n\t[<number of messages>[<number of iterations>]]]"},
#endif
#if FAPP_CFG_REINIT_CMD   /* Used to test FNET release/init only. */
    { FNET_SHELL_CMD_TYPE_NORMAL, "reinit",     0, 0, (void *)fapp_reinit_cmd,  "Reinit application", ""},
#endif
#if FNET_CFG_PING && FAPP_CFG_PING_CMD 
    { FNET_SHELL_CMD_TYPE_NORMAL, "ping",       1, 14, (void *)fapp_ping_cmd,    "Send ECHO requests", "[-c <count>][-i <seconds>]\n\r\t[-p <pattern>][-s <size>][-h <hoplimit/ttl>] <ip>"}, /* -s -n should be ignored.*/
#endif
#if FNET_CFG_PING && FAPP_CFG_PING6_CMD 
    { FNET_SHELL_CMD_TYPE_NORMAL, "ping6",      1, 14, (void *)fapp_ping_cmd,   "Send ECHO requests", "[-c <count>][-i <seconds>]\n\r\t[-p <pattern>][-s <size>][-h <hoplimit/ttl>] <ip>"}, /* -s -n should be ignored.*/
#endif

#if FAPP_CFG_DEBUG_CMD   /* Used for DEBUGING needs only. */
    { FNET_SHELL_CMD_TYPE_NORMAL, "d",          0, 0, (void *)fapp_debug_cmd,   "Print debug info.", ""},
#endif
    { FNET_SHELL_CMD_TYPE_END,     0,           0, 0, 0, 0, 0}  
};

/* Shell command-line buffer.*/
static char fapp_cmd_line_buffer[FAPP_CFG_SHELL_MAX_LINE_LENGTH];

/************************************************************************
*     The main shell control data structure.
*************************************************************************/
const struct fnet_shell fapp_shell =
{
    fapp_cmd_table,         /* cmd_table */
    FAPP_CFG_SHELL_PROMPT,  /* prompt_str */
    fapp_shell_init,        /* shell_init */
};

fnet_shell_desc_t fapp_shell_desc = 0; /* Shell descriptor. */

#if FAPP_CFG_BOOTLOADER || FAPP_CFG_SETGET_CMD_BOOT

/* Boot-mode string matching. */
#define FAPP_PARAMS_BOOT_MODE_STOP_STR      "stop"   /* Stop at the shell prompt */
#define FAPP_PARAMS_BOOT_MODE_GO_STR        "go"     /* Boot from flash.*/
#define FAPP_PARAMS_BOOT_MODE_SCRIPT_STR    "script" /* Start boot-script.*/

/* Boot modes. */
const struct boot_mode boot_modes[] =
{
    {FAPP_PARAMS_BOOT_MODE_STOP,    FAPP_PARAMS_BOOT_MODE_STOP_STR, 0},
    {FAPP_PARAMS_BOOT_MODE_GO,      FAPP_PARAMS_BOOT_MODE_GO_STR, fapp_boot_mode_go},
    {FAPP_PARAMS_BOOT_MODE_SCRIPT,  FAPP_PARAMS_BOOT_MODE_SCRIPT_STR, fapp_boot_mode_script}
};

#define BOOT_MODE_COUNT     (sizeof(boot_modes)/sizeof(struct boot_mode))
#define BOOT_MODE_DEFAULT   boot_modes[0]


/************************************************************************
* NAME: fapp_boot_mode_by_index
*
* DESCRIPTION:
************************************************************************/
const struct boot_mode *fapp_boot_mode_by_index (unsigned long index)
{
    const struct boot_mode *result = &BOOT_MODE_DEFAULT;
    const struct boot_mode *mode = boot_modes;
    while( mode->name )
    {
        if( mode->index == index )
        {
            result = mode;
            break;
        } 
        mode++;  
    }
    
    return result;    
}

/************************************************************************
* NAME: fapp_boot_mode_by_name
*
* DESCRIPTION:
************************************************************************/
const struct boot_mode *fapp_boot_mode_by_name (char *name)
{
    const struct boot_mode *result = 0;
    const struct boot_mode *mode = boot_modes;
    while( mode->name )
    {
        if(fnet_strcmp( mode->name, name ) == 0)
        {
            result = mode;
            break;
        } 
        mode++;  
    }
    
    return result;   
}

/************************************************************************
* NAME: fapp_boot_mode_go
*
* DESCRIPTION: 
************************************************************************/
static void fapp_boot_mode_go(fnet_shell_desc_t desc)
{
    fapp_go (desc, fapp_params_boot_config.go_address);
}

/************************************************************************
* NAME: fapp_boot_mode_script
*
* DESCRIPTION: 
************************************************************************/
static void fapp_boot_mode_script(fnet_shell_desc_t desc)
{
    fnet_shell_println(desc, "\n%s",fapp_params_boot_config.script);
    fnet_shell_script(desc, fapp_params_boot_config.script);
}
#endif /* FAPP_CFG_BOOTLOADER */

/************************************************************************
* NAME: fapp_boot
*
* DESCRIPTION: 
************************************************************************/
static void fapp_boot(fnet_shell_desc_t desc)
{
/* Bootloader. */
#if FAPP_CFG_BOOTLOADER  

    /* The bootloader wait some time for a key over a serial connection.*/
    /* The wait time is given by the boot_delay parameter.*/
    int delay;

    const struct boot_mode *mode;

    mode = fapp_boot_mode_by_index (fapp_params_boot_config.mode);    
        

    if(mode->handler)
    {
        delay = (int)fapp_params_boot_config.delay;
        
        fnet_shell_printf(desc, FAPP_BOOT_STR, mode->name, delay);

        while(--delay >= 0)
        {
            if( fnet_shell_getchar(desc) != FNET_ERR)
                break;
                
            fnet_timer_delay(FNET_TIMER_TICK_IN_SEC); /* 1 sec. delay. */
            fnet_shell_printf(desc, "\b\b\b\b %3d", delay);
        }
       
        fnet_shell_println(desc, "");
       
        if(delay < 0)
        {
            /* Auto-start*/
            mode->handler(desc);
        }
    }
    else
#endif    
    {
        /* Default startup script. */
        #if FAPP_CFG_STARTUP_SCRIPT_ENABLED
            fnet_shell_println(desc, "Startup script: %s", FAPP_CFG_STARTUP_SCRIPT);
            fnet_shell_script(desc, FAPP_CFG_STARTUP_SCRIPT );
        #endif
    }
    
    FNET_COMP_UNUSED_ARG(desc);
}

/************************************************************************
* NAME: fapp_reset_cmd
*
* DESCRIPTION: Reset command reboots the system. 
************************************************************************/
#if FAPP_CFG_RESET_CMD || FAPP_CFG_REBOOT_CMD
void fapp_reset_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    FNET_COMP_UNUSED_ARG(desc);
    FNET_COMP_UNUSED_ARG(argc);
    FNET_COMP_UNUSED_ARG(argv);

    fnet_release(); /* Release FNET stack (it's optional).*/
    
    fnet_cpu_reset();
}
#endif

/************************************************************************
* NAME: fapp_debug_cmd
*
* DESCRIPTION: Print your DEBUG info. 
************************************************************************/
#if FAPP_CFG_DEBUG_CMD
void fapp_debug_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    FNET_COMP_UNUSED_ARG(desc);
    FNET_COMP_UNUSED_ARG(argc);
    FNET_COMP_UNUSED_ARG(argv);

    /* Put here you debugging information.*/
    fnet_eth_debug_mii_print_regs(fapp_default_netif);
}
#endif

/************************************************************************
* NAME: fapp_dup_ip_handler
*
* DESCRIPTION: IP address is duplicated.
************************************************************************/
#if FNET_CFG_IP4
static void fapp_dup_ip_handler( fnet_netif_desc_t netif )
{
    char name[FNET_NETIF_NAMELEN];
    char ip_str[FNET_IP4_ADDR_STR_SIZE];
    fnet_ip4_addr_t addr;
    
    fnet_netif_get_name( netif, name, FNET_NETIF_NAMELEN );
    addr = fnet_netif_get_ip4_addr( netif );
    fnet_inet_ntoa(*(struct in_addr *)( &addr), ip_str);

    fnet_printf(FAPP_DUP_IP_WARN, name, ip_str);
}
#endif /* FNET_CFG_IP4 */


/************************************************************************
* NAME: fapp_init
*
* DESCRIPTION: FNET Application initialization.
************************************************************************/
static void fapp_init(void)
{
    static unsigned char stack_heap[FNET_CFG_HEAP_SIZE];
    struct fnet_init_params init_params;
    
    struct fnet_shell_params shell_params;

    /* Input parameters for FNET stack initialization */
    init_params.netheap_ptr = stack_heap;
    init_params.netheap_size = FNET_CFG_HEAP_SIZE;

    
    /* Add event handler on duplicated IP address */
#if FNET_CFG_IP4    
    fnet_netif_dupip_handler_init (fapp_dup_ip_handler);
#endif    
            
    /* Init FNET stack */
    if(fnet_init(&init_params) != FNET_ERR)
    {
        if((fapp_default_netif = fnet_netif_get_default()) == FNET_NULL)
        {
            fnet_printf(FAPP_NET_ERR);
        }

    #if FAPP_CFG_PARAMS_READ_FLASH
        /* During bootup, the most recently stored customer configuration data will be read and used to configure the interfaces.*/
        if(fapp_params_from_flash() == FNET_OK)
        {
            fnet_printf(FAPP_PARAMS_LOAD_STR);
        }
    #endif
            
    #if (FAPP_CFG_EXP_CMD && FNET_CFG_FS) || (FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP)   
        fapp_fs_mount(); /* Init FS and mount FS Image. */
    #endif   
            
        /* Init main shell. */
        shell_params.shell = &fapp_shell;
        shell_params.cmd_line_buffer = fapp_cmd_line_buffer;
        shell_params.cmd_line_buffer_size = sizeof(fapp_cmd_line_buffer);
        shell_params.stream = FNET_SERIAL_STREAM_DEFAULT;
        shell_params.echo = 1;
            
        if((fapp_shell_desc = fnet_shell_init(&shell_params)) != FNET_ERR)
        {
            fapp_boot(fapp_shell_desc);
        }
        else
            fnet_printf(FAPP_INIT_ERR, "Shell");
    }
    else
        fnet_printf(FAPP_INIT_ERR, "FNET");

}

/************************************************************************
* NAME: fapp_init
*
* DESCRIPTION: FNET Application release.
************************************************************************/
#if FAPP_CFG_REINIT_CMD 
static void fapp_release(fnet_shell_desc_t desc)
{
    #if FAPP_CFG_DHCP_CMD && FNET_CFG_DHCP
    fapp_dhcp_release();        /* Release DHCP client. */
    #endif
    
    #if FAPP_CFG_TELNET_CMD && FNET_CFG_TELNET    /* Release TELNET server. */   
    fapp_telnet_release();
    #endif
 
    #if FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP      /* Release HTTP server. */ 
    fapp_http_release();
    #endif    
 
    #if FAPP_CFG_TFTPS_CMD      /* Release TFTP server. */
    fapp_tftps_release();
    #endif   

    #if (FAPP_CFG_EXP_CMD && FNET_CFG_FS) || (FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP)   
    fapp_fs_unmount();          /* Unmount and release FS. */
    #endif 
    
    fnet_shell_release(desc);   /* Release shell. */
    
    fnet_release();             /* Release the FNET stack.*/
}
#endif /* FAPP_CFG_REINIT_CMD */

/************************************************************************
* NAME: fapp
*
* DESCRIPTION: main() of the shell demo.
************************************************************************/
void fapp_main()
{
    fapp_init();
    
    /* Polling services.*/
    while(1)
    {
       fnet_poll_services();
    }
}

/************************************************************************
* NAME: fapp_netif_info_print
*
* DESCRIPTION: Print detailed address information about default
*              network interface.
************************************************************************/
void fapp_netif_info_print( fnet_shell_desc_t desc, fnet_netif_desc_t netif)
{
    char name[FNET_NETIF_NAMELEN];

    fnet_netif_get_name(netif, name, FNET_NETIF_NAMELEN);
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "Interface", name);
    
    fapp_netif_addr_print(desc, AF_SUPPORTED, netif, FNET_TRUE);
    
#if FNET_CFG_IP4
    {    
        fnet_ip4_addr_t ip_addr;
        char ip_str[FNET_IP4_ADDR_STR_SIZE];
          
        ip_addr = fnet_netif_get_ip4_subnet_mask(netif);
        fnet_inet_ntoa(*(struct in_addr *)( &ip_addr), ip_str);
        fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "IPv4 Subnet mask", ip_str);
        
        ip_addr = fnet_netif_get_ip4_gateway(netif);
        fnet_inet_ntoa(*(struct in_addr *)( &ip_addr), ip_str);
        fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "IPv4 Gateway", ip_str);

      
    #if FNET_CFG_DNS
        ip_addr = fnet_netif_get_ip4_dns(netif);    
        fnet_inet_ntoa(*(struct in_addr *)( &ip_addr), ip_str);
        fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "IPv4 DNS", ip_str);
    #endif
    }
#endif /* FNET_CFG_IP4 */
}

/************************************************************************
* NAME: fapp_netif_addr_print
*
* DESCRIPTION: Print Interface IP addresses. 
************************************************************************/
void fapp_netif_addr_print(fnet_shell_desc_t desc, fnet_address_family_t family, fnet_netif_desc_t netif, int print_type)
{
	char    ip_str[FNET_IP_ADDR_STR_SIZE]={0};
    
#if FNET_CFG_IP4
    if(family & AF_INET)
    {
        fnet_ip4_addr_t local_ip;
        
        local_ip = fnet_netif_get_ip4_addr(netif);
        fnet_shell_printf(desc, FAPP_SHELL_INFO_FORMAT_S, "IPv4 Address", fnet_inet_ntoa(*(struct in_addr *)(&local_ip), ip_str) ); 
        if(print_type)
            fnet_shell_println(desc," <%s>", fnet_netif_get_ip4_addr_automatic(netif) ? "automatic" : "manual");
        else
            fnet_shell_println(desc, "");  
    }
#endif /* FNET_CFG_IP4 */
#if FNET_CFG_IP6
    if(family & AF_INET6)
    {
        int result;
        unsigned int n;
        fnet_netif_ip6_addr_info_t addr_info;
        
        /* Print all assigned IPv6 addreses.*/
        for(n=0;;n++) 
        {
            result = fnet_netif_get_ip6_addr (netif, n, &addr_info);
            
            if(result == FNET_TRUE)
            {
                fnet_inet_ntop(AF_INET6, (char*)(&addr_info.address), ip_str, sizeof(ip_str)); 
            
                fnet_shell_printf(desc, FAPP_SHELL_INFO_FORMAT_S, "IPv6 Address", ip_str);
                if(print_type)
                    fnet_shell_println(desc," <%s>", (addr_info.type == FNET_NETIF_IP6_ADDR_TYPE_AUTOCONFIGURABLE)  ? "autoconfigurable" : "manual"); 
                else
                    fnet_shell_println(desc, "");                     
            }
            else
               break;
        };
    }
#endif /* FNET_CFG_IP6 */
}

/************************************************************************
* NAME: fapp_info_print
*
* DESCRIPTION: Display detailed information about the stack.
************************************************************************/
void fapp_info_print( fnet_shell_desc_t desc )
{
    struct fnet_netif_statistics statistics;
    char mac_str[FNET_MAC_ADDR_STR_SIZE];
    fnet_mac_addr_t macaddr;
    fnet_netif_desc_t netif = fapp_default_netif;         

    fapp_netif_info_print(desc, netif);

    /* HW address, if any */
    if(fnet_netif_get_hw_addr(netif, macaddr, sizeof(fnet_mac_addr_t)) == FNET_OK)
    {
        fnet_mac_to_str(macaddr, mac_str);
        fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "MAC Address", mac_str);
    }    
    
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "Link Status", fnet_netif_connected(netif) ? "connected" : "unconnected");

    if(fnet_netif_get_statistics(netif, &statistics) == FNET_OK)
    {
        fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "TX Packets", statistics.tx_packet);
        fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "RX Packets", statistics.rx_packet);
    }
    
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Free Heap", fnet_free_mem_status());

#if FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP

    fapp_http_info(desc);

#endif

#if FAPP_CFG_DHCP_CMD && FNET_CFG_DHCP && FNET_CFG_IP4
    
    fapp_dhcp_info(desc);
    
#endif

#if FAPP_CFG_TELNET_CMD && FNET_CFG_TELNET

    fapp_telnet_info(desc);
    
#endif   

#if FAPP_CFG_TFTPS_CMD

    fapp_tftps_info(desc);
    
#endif 


}

/************************************************************************
* NAME: fapp_info_cmd
*
* DESCRIPTION:
************************************************************************/
#if FAPP_CFG_INFO_CMD
void fapp_info_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    FNET_COMP_UNUSED_ARG(argc);
    FNET_COMP_UNUSED_ARG(argv);

    fapp_info_print(desc);
}
#endif

/************************************************************************
* NAME: fapp_shell_init
*
* DESCRIPTION: Main Shell initial function.
************************************************************************/
void fapp_shell_init( fnet_shell_desc_t desc )
{
    fnet_shell_println(desc, "\n%s", FAPP_DELIMITER_STR);
    fnet_shell_println(desc, " %s", FAPP_CFG_NAME);

    /* FNET Header */
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    
    fnet_shell_println(desc, " %s for %s", FNET_DESCRIPTION, FNET_CPU_STR);
    fnet_shell_println(desc, " Version %s", FNET_VERSION);
    fnet_shell_println(desc, " Built %s by %s", FNET_BUILD_DATE, FNET_COMP_STR);
    fnet_shell_println(desc, " %s", FNET_COPYRIGHT);
    fnet_shell_println(desc, " %s", FNET_LICENSE);
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    fapp_info_print(desc);
    fnet_shell_println(desc, "\n Enter 'help' for command list.");
    fnet_shell_println(desc, "%s\n", FAPP_DELIMITER_STR);
}

/************************************************************************
* NAME: fapp_go_cmd
*
* DESCRIPTION:  
************************************************************************/
#if FAPP_CFG_GO_CMD
void fapp_go_cmd ( fnet_shell_desc_t desc, int argc, char ** argv )
{
	unsigned long address;
	char *p;

	if (argc == 2)
	{
		address = fnet_strtoul(argv[1],&p,16);
		
		if (p == argv[1])
        {
            fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1] );     
            return;
        }
    }
    else
        address = fapp_params_boot_config.go_address;
  
	fapp_go(desc, address);	
}
#endif

/************************************************************************
* NAME: fapp_go
*
* DESCRIPTION:   Jump to application entry point.
************************************************************************/
#if FAPP_CFG_GO_CMD || FAPP_CFG_BOOTLOADER || FAPP_CFG_SETGET_CMD_BOOT
static void fapp_go ( fnet_shell_desc_t desc, unsigned long address )
{
    if((fapp_params_boot_config.go_address == 0) || (*(fnet_uint32*)address == 0xffffffff))
    {
        fnet_printf("\nThere is no code on user application startup vector.\n");
    }
    else
    {    
        
      fnet_shell_println(desc, FAPP_GO_STR, address);
   
      fnet_release(); /* Release the FNET stack.*/
     
      (( void(*)() )FNET_CPU_INSTRUCTION_ADDR(address))(); /* Jump. */
    }
}
#endif

/************************************************************************
* NAME: fapp_save_cmd
*
* DESCRIPTION: Save environment variables to persistent storage. 
************************************************************************/
#if FAPP_CFG_SAVE_CMD 
void fapp_save_cmd ( fnet_shell_desc_t desc, int argc, char ** argv )
{
    (void)argc;
	(void)argv;

    if(fapp_params_to_flash() == FNET_OK)
    {
        fnet_shell_println(desc, FAPP_SAVE_STR);
    }
    else
    {
        fnet_shell_script_release(desc);   /* Critical error. Clear script. */
        fnet_shell_println(desc, FAPP_SAVE_FAILED_STR);
    }
}
#endif

/************************************************************************
* NAME: fapp_help_cmd
*
* DESCRIPTION: Display command help. 
************************************************************************/
void fapp_help_cmd ( fnet_shell_desc_t desc, int argc, char ** argv )
{
    (void)argc;
	(void)argv;

    fnet_shell_help(desc);
}

/************************************************************************
* NAME: fapp_reinit_cmd
*
* DESCRIPTION: Used to test FNET release/init only. 
************************************************************************/
#if FAPP_CFG_REINIT_CMD 
void fapp_reinit_cmd ( fnet_shell_desc_t desc, int argc, char ** argv )
{
    (void)argc;
	(void)argv;

    fapp_release(desc); /* FAPP release. */

    fapp_init();        /* FAPP reinit.*/
}
#endif

/************************************************************************
* NAME: fapp_bind_cmd
*
* DESCRIPTION: Binds IPv6 address to the default interface. 
************************************************************************/
#if FAPP_CFG_BIND_CMD && FNET_CFG_IP6
void fapp_bind_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    fnet_netif_desc_t   netif = fapp_default_netif;
    fnet_ip6_addr_t     addr;
    
    FNET_COMP_UNUSED_ARG(argc);
    
    if(fnet_inet_pton(AF_INET6, argv[1], &addr, sizeof(addr)) == FNET_OK)
    {
        fnet_netif_bind_ip6_addr(netif, &addr, FNET_NETIF_IP6_ADDR_TYPE_MANUAL);
    }
    else
    {
        fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1]);
    }
}
#endif

/************************************************************************
* NAME: fapp_unbind_cmd
*
* DESCRIPTION: Unbinds IPv6 address from the default interface. 
************************************************************************/
#if FAPP_CFG_UNBIND_CMD && FNET_CFG_IP6
void fapp_unbind_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    fnet_netif_desc_t   netif = fapp_default_netif;
    fnet_ip6_addr_t     addr;
    
    FNET_COMP_UNUSED_ARG(argc);
    
    if(fnet_inet_pton(AF_INET6, argv[1], &addr, sizeof(addr)) == FNET_OK)
    {
        fnet_netif_unbind_ip6_addr(netif, &addr);
    }
    else
    {
        fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1]);
    }
}
#endif











