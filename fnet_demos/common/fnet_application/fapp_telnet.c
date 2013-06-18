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
* @file fapp_telnet.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.37.0
*
* @brief FNET Shell Demo implementation.
*
***************************************************************************/

#include "fapp.h"
#include "fapp_prv.h"
#include "fapp_telnet.h"
#include "fapp_mem.h"

#include "fnet_netbuf.h"

#if FAPP_CFG_SETGET_CMD
#include "fapp_setget.h"
#endif
#if FAPP_CFG_DHCP_CMD

#include "fapp_dhcp.h"

#endif
#if FAPP_CFG_HTTP_CMD || FAPP_CFG_EXP_CMD 

#include "fapp_http.h"
#include "fapp_fs.h"

#endif

#if FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD || FAPP_CFG_TFTPS_CMD

#include "fapp_tftp.h"

#endif

#if FAPP_CFG_TELNET_CMD && FNET_CFG_TELNET

#define FAPP_TELNET_PROMPT_STR     FAPP_CFG_SHELL_PROMPT

#if FAPP_CFG_TELNET_TEST_CMD
    void fapp_telnet_test_cmd( fnet_shell_desc_t desc );
#endif

#if FAPP_CFG_TELNET_CMD_OWN_SHELL
    void fapp_telnet_exit_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );

    /************************************************************************
    *     The table of the telnet shell commands.
    *************************************************************************/
    static const struct fnet_shell_command fapp_telnet_cmd_table [] =
    {
        { FNET_SHELL_CMD_TYPE_NORMAL, "help", 0, 0, (void *)fapp_help_cmd,"Display this help message", ""},

    #if FAPP_CFG_SETGET_CMD
        { FNET_SHELL_CMD_TYPE_NORMAL, "set", 0, 2, (void *)fapp_set_cmd,      "Set parameter", "[<parameter> <value>]"},
        { FNET_SHELL_CMD_TYPE_NORMAL, "get", 0, 1, (void *)fapp_get_cmd,    "Get parameters", "[<parameter>]" },
    #endif    
    #if FAPP_CFG_INFO_CMD
        { FNET_SHELL_CMD_TYPE_NORMAL, "info", 0, 0, (void *)fapp_info_cmd,    "Show detailed status", ""},
    #endif
    #if FAPP_CFG_DHCP_CMD && FNET_CFG_DHCP
        { FNET_SHELL_CMD_TYPE_NORMAL, "dhcp", 0, 1, (void *)fapp_dhcp_cmd,    "Start DHCP client", "[release]"},
    #endif
    #if FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP
        { FNET_SHELL_CMD_TYPE_NORMAL, "http", 0, 1, (void *)fapp_http_cmd,    "Start HTTP Server", "[release]"},
    #endif
    #if FAPP_CFG_EXP_CMD && FNET_CFG_FS
        { FNET_SHELL_CMD_TYPE_SHELL,  "exp", 0, 1, &fapp_fs_shell,   "File Explorer submenu...", ""},
    #endif
    #if FAPP_CFG_TFTP_CMD
        { FNET_SHELL_CMD_TYPE_NORMAL, "tftp", 0, 3, (void *)fapp_tftp_cmd,  "TFTP firmware loader", "[<image name>[<server ip>[<type>]]]"},
    #endif
    #if FAPP_CFG_TFTPUP_CMD
        { FNET_SHELL_CMD_TYPE_NORMAL, "tftpup", 0, 3, (void *)fapp_tftp_cmd,  "TFTP firmware uploader", "[<image name>[<server ip>[<type>]]]"},
    #endif  
    #if FAPP_CFG_TFTPS_CMD
        { FNET_SHELL_CMD_TYPE_NORMAL, "tftps", 0, 1, (void *)fapp_tftps_cmd,  "TFTP firmware server", "[release]"},
    #endif 

    #if FAPP_CFG_MEM_CMD    
        { FNET_SHELL_CMD_TYPE_NORMAL, "mem", 0, 0, (void *)fapp_mem_cmd,    "Show memory map", ""},
    #endif  
    #if FAPP_CFG_ERASE_CMD    
        { FNET_SHELL_CMD_TYPE_NORMAL, "erase", 1, 2, (void *)fapp_mem_erase_cmd,    "Erase flash memory", "all|[0x<erase address> <bytes>]"},
    #endif 
    #if FAPP_CFG_SAVE_CMD    
        { FNET_SHELL_CMD_TYPE_NORMAL, "save", 0, 0, (void *)fapp_save_cmd,    "Save parameters to the FLASH", ""},
    #endif 
    #if FAPP_CFG_GO_CMD    
        { FNET_SHELL_CMD_TYPE_NORMAL, "go", 0, 1, (void *)fapp_go_cmd,    "Start application at address", "[0x<address>]"},
    #endif 
    #if FAPP_CFG_RESET_CMD    
        { FNET_SHELL_CMD_TYPE_NORMAL, "reset", 0, 0, (void *)fapp_reset_cmd,    "Reset the board", ""},
    #endif   
        { FNET_SHELL_CMD_TYPE_NORMAL, "exit", 0, 0, (void *)fapp_telnet_exit_cmd,    "Close telnet session", ""},
#if FAPP_CFG_TELNET_TEST_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "test", 0, 0, (void *)fapp_telnet_test_cmd,    "Test", ""},
#endif            
        { FNET_SHELL_CMD_TYPE_END, 0, 0, 0, 0, 0, 0}     
    };
#endif


/************************************************************************
*     The main shell control data structure.
*************************************************************************/
const struct fnet_shell fapp_telnet_shell =
{
#if FAPP_CFG_TELNET_CMD_OWN_SHELL /* Owm command table.*/
    fapp_telnet_cmd_table,                                 
#else  /* Use same commands as main shell. */  
    fapp_cmd_table,
#endif    
    FAPP_TELNET_PROMPT_STR,     /* prompt_str */
    fapp_shell_init,            /* shell_init */
};


static fnet_telnet_desc_t fapp_telnet_desc = 0; /* Telnet descriptor. */


/************************************************************************
* NAME: fapp_telnet_release
*
* DESCRIPTION: Releases TELNET server.
*************************************************************************/
void fapp_telnet_release()
{
    fnet_telnet_release(fapp_telnet_desc);
    fapp_telnet_desc = 0;    
}

/************************************************************************
* NAME: fapp_telnet_cmd
*
* DESCRIPTION: Run Telnet server.
*************************************************************************/
void fapp_telnet_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    struct fnet_telnet_params   params;
    fnet_telnet_desc_t          telnet_desc;

    if(argc == 1) /* By default is "init".*/
    {
        fnet_memset_zero(&params, sizeof(struct fnet_telnet_params));
        params.shell= &fapp_telnet_shell;
        
        /* Init Telnet server */
        telnet_desc = fnet_telnet_init(&params);
        if(telnet_desc != FNET_ERR)
        {
            fnet_shell_println(desc, FAPP_DELIMITER_STR);
            fnet_shell_println(desc, " Telnet Server started.");
            fapp_netif_addr_print(desc, AF_SUPPORTED, fapp_default_netif, FNET_FALSE);
            fnet_shell_println(desc, FAPP_DELIMITER_STR);
            
            fapp_telnet_desc = telnet_desc;
        }
        else
        {
            fnet_shell_println(desc, FAPP_INIT_ERR, "Telnet");
        }
       
    }
    else if(argc == 2 && fnet_strcasecmp(&FAPP_COMMAND_RELEASE[0], argv[1]) == 0) /* [release] */
    {
        fapp_telnet_release();
    }
    else
    {
        fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1]);
    }
}

/************************************************************************
* NAME: fapp_telnet_exit_cmd
*
* DESCRIPTION: 
************************************************************************/
#if FAPP_CFG_TELNET_CMD_OWN_SHELL
static void fapp_telnet_exit_cmd ( fnet_shell_desc_t desc, int argc, char ** argv )
{
    FNET_COMP_UNUSED_ARG(desc);
    FNET_COMP_UNUSED_ARG(argc);
	FNET_COMP_UNUSED_ARG(argv);

    fnet_telnet_close_session(fapp_telnet_desc);
}
#endif

/************************************************************************
* NAME: fapp_telnet_info
*
* DESCRIPTION:
*************************************************************************/
void fapp_telnet_info(fnet_shell_desc_t desc)
{
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "TELNET Server", 
                        fnet_telnet_enabled(fapp_telnet_desc) ? FAPP_SHELL_INFO_ENABLED : FAPP_SHELL_INFO_DISABLED);
}


#if FAPP_CFG_TELNET_TEST_CMD
/************************************************************************
* NAME: fapp_telnet_test_cmd
*
* DESCRIPTION: "test" command. Used to test Telnet sending.
* For debug needs only
************************************************************************/
void fapp_telnet_test_cmd( fnet_shell_desc_t desc )
{
	//char szBuf[40];
	//int i;

	//fnet_strcpy (szBuf, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    //for (i = 0; i < 15000; i++)
    while(1)
	{
		//fnet_shell_printf(desc, "%s", szBuf);
		fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Free Heap", fnet_free_mem_status());
		fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "MAX Heap", fnet_malloc_max());
	}
}
#endif

#endif











