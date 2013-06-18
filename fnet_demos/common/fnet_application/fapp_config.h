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
* @file fapp_config.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.33.0
*
* @brief FNET Demo Application Configuration.
*
***************************************************************************/
#ifndef _FAPP_CONFIG_H_

#define _FAPP_CONFIG_H_
#include "fnet_config.h"
#include "fapp_user_config.h"

/**************************************************************************/ /*!
 * @def      FAPP_CFG_NAME
 * @brief    Name of the application. 
 * @showinitializer
 ******************************************************************************/
#ifndef FAPP_CFG_NAME
    #define FAPP_CFG_NAME                       "FNET Application"       
#endif

/**************************************************************************/ /*!
 * @def      FAPP_CFG_SHELL_PROMPT
 * @brief    Prompt message used by the application shell. 
 * @showinitializer
 ******************************************************************************/
#ifndef FAPP_CFG_SHELL_PROMPT
    #define FAPP_CFG_SHELL_PROMPT   "FNET> "
#endif    

/**************************************************************************/ /*!
 * @def      FAPP_CFG_PARAMS_READ_FLASH
 * @brief    Reading of the configuration parameters from the Flash memory during
 *           the application bootup:
 *               - @c 1 = is enabled. @n
 *                        During bootup, the application read 
 *                        and use the most recently stored customer 
 *                        configuration data from the flash.
 *               - @c 0 = is disabled. @n
 *                        During bootup, the application uses 
 *                        compile-time parameters by default. 
 * @showinitializer
 ******************************************************************************/
#ifndef FAPP_CFG_PARAMS_READ_FLASH
    #define FAPP_CFG_PARAMS_READ_FLASH          (0)
#endif

/**************************************************************************/ /*!
 * @def      FAPP_CFG_PARAMS_REWRITE_FLASH
 * @brief    Rewriting of the configuration parameters in the Flash memory 
 *           duiring flashing of the application:
 *               - @c 1 = is enabled. @n
 *                        It will allocated the default parameter structure 
 *                        which  will rewrite the configurated parameters 
 *                        present in the Flash memory during flashing of 
 *                        the application.
 *               - @c 0 = is disabled. @n
 *                        The configuration parameters that present in the 
 *                        flash memory will stay untouchable during flashing. 
 * @showinitializer
 ******************************************************************************/
#ifndef FAPP_CFG_PARAMS_REWRITE_FLASH
    #define FAPP_CFG_PARAMS_REWRITE_FLASH       (0)
#endif

/**************************************************************************/ /*!
 * @def      FAPP_CFG_BOOTLOADER
 * @brief    Bootloader:
 *               - @c 1 = is enabled. @n
 *                        The application startup behaviour is defined 
 *                        by the @ref fapp_params_boot_mode_t mode.
 *               - @c 0 = is disabled. @n
 * @showinitializer
 ******************************************************************************/
#ifndef FAPP_CFG_BOOTLOADER
    #define FAPP_CFG_BOOTLOADER                (0)
#endif

/* CFM protection is enabled.
 * cfm.c have to be updated aklso.
 */
#ifndef FAPP_CFG_CFM_PROTECTION
    #define FAPP_CFG_CFM_PROTECTION             (0)
#endif

/* Startup script. */
#ifndef FAPP_CFG_STARTUP_SCRIPT_ENABLED
    #define FAPP_CFG_STARTUP_SCRIPT_ENABLED     (0)
#endif

#ifndef FAPP_CFG_STARTUP_SCRIPT
    #define FAPP_CFG_STARTUP_SCRIPT             ""
#endif

/* Preinstall FNET ISR into vector table.*/
#ifndef FAPP_CFG_PREINSTALL_INTERRUPTS
    #define FAPP_CFG_PREINSTALL_INTERRUPTS      (0)
#endif

/************************************************************************
* Default application parameters (allocated in the Flash memory).
*************************************************************************/
/* FNET TCP/IP stack default parameters ROM. */
#ifndef FAPP_CFG_PARAMS_IP_ADDR
    #define FAPP_CFG_PARAMS_IP_ADDR             FNET_CFG_ETH0_IP4_ADDR    /* Defined by FNET */
#endif

#ifndef FAPP_CFG_PARAMS_IP_MASK
    #define FAPP_CFG_PARAMS_IP_MASK             FNET_CFG_ETH0_IP4_MASK    /* Defined by FNET */
#endif

#ifndef FAPP_CFG_PARAMS_IP_GW
    #define FAPP_CFG_PARAMS_IP_GW               FNET_CFG_ETH0_IP4_GW      /* Defined by FNET */
#endif

#ifndef FAPP_CFG_PARAMS_IP_DNS
    #define FAPP_CFG_PARAMS_IP_DNS              FNET_CFG_ETH0_IP4_DNS     /* Defined by FNET */
#endif

#ifndef FAPP_CFG_PARAMS_MAC_ADDR
    #define FAPP_CFG_PARAMS_MAC_ADDR            (0) /* Defined by FNET */
#endif

/* Bootloader parameters. */
#ifndef FAPP_CFG_PARAMS_BOOT_MODE
    #define FAPP_CFG_PARAMS_BOOT_MODE           FAPP_PARAMS_BOOT_MODE_STOP 
#endif

#ifndef FAPP_CFG_PARAMS_BOOT_DELAY
    #define FAPP_CFG_PARAMS_BOOT_DELAY          (5) /*sec*/
#endif

#ifndef FAPP_CFG_PARAMS_BOOT_GO_ADDRESS
    #define FAPP_CFG_PARAMS_BOOT_GO_ADDRESS     (0) 
#endif

#ifndef FAPP_CFG_PARAMS_BOOT_SCRIPT
    #define FAPP_CFG_PARAMS_BOOT_SCRIPT         ""
#endif

/* TFTP parameters */
#ifndef FAPP_CFG_PARAMS_TFTP_SERVER
    #define FAPP_CFG_PARAMS_TFTP_SERVER         {0} 
#endif

#ifndef FAPP_CFG_PARAMS_TFTP_FILE_TYPE
    #define FAPP_CFG_PARAMS_TFTP_FILE_TYPE      FAPP_PARAMS_TFTP_FILE_TYPE_SREC
#endif

#ifndef FAPP_CFG_PARAMS_TFTP_FILE_NAME
    #define FAPP_CFG_PARAMS_TFTP_FILE_NAME      ""
#endif

#ifndef FAPP_CFG_PARAMS_TFTP_FILE_RAW_ADDRESS
    #define FAPP_CFG_PARAMS_TFTP_FILE_RAW_ADDRESS    (0) 
#endif



/************************************************************************
*    "info" command.
*************************************************************************/
#ifndef FAPP_CFG_INFO_CMD
    #define FAPP_CFG_INFO_CMD           (0)
#endif

/************************************************************************
*    "dhcp" command.
*************************************************************************/
#ifndef FAPP_CFG_DHCP_CMD
    #define FAPP_CFG_DHCP_CMD           (0)
    
    #ifndef FAPP_CFG_DHCP_CMD_DISCOVER_MAX
        #define FAPP_CFG_DHCP_CMD_DISCOVER_MAX  (-1) /* -1 means infinite. */
    #endif
    
#endif

/************************************************************************
*    "http" command.
*************************************************************************/
#ifndef FAPP_CFG_HTTP_CMD
    #define FAPP_CFG_HTTP_CMD           (0)
#endif

/************************************************************************
*    "exp" command.
*************************************************************************/
#ifndef FAPP_CFG_EXP_CMD
    #define FAPP_CFG_EXP_CMD            (0)
#endif

/************************************************************************
*    "tftp" command.
*************************************************************************/
#ifndef FAPP_CFG_TFTP_CMD
    #define FAPP_CFG_TFTP_CMD           (0)
#endif

/************************************************************************
*    "tftpup" command.
*************************************************************************/
#ifndef FAPP_CFG_TFTPUP_CMD
    #define FAPP_CFG_TFTPUP_CMD         (0)
#endif

/************************************************************************
*    "tftps" command.
*************************************************************************/
#ifndef FAPP_CFG_TFTPS_CMD
    #define FAPP_CFG_TFTPS_CMD          (0)
#endif

/************************************************************************
*    "telnet" command.
*************************************************************************/
#ifndef FAPP_CFG_TELNET_CMD
    #define FAPP_CFG_TELNET_CMD         (0)
#endif

#ifndef FAPP_CFG_TELNET_CMD_OWN_SHELL
    #define FAPP_CFG_TELNET_CMD_OWN_SHELL   (0)
#endif

/************************************************************************
*    "mem" command.
*************************************************************************/
#ifndef FAPP_CFG_MEM_CMD
    #define FAPP_CFG_MEM_CMD            (0)
#endif  

/************************************************************************
*    "erase" command.
*************************************************************************/
#ifndef FAPP_CFG_ERASE_CMD
    #define FAPP_CFG_ERASE_CMD          (0)
#endif 

/************************************************************************
*    "save" command.
*************************************************************************/
#ifndef FAPP_CFG_SAVE_CMD
    #define FAPP_CFG_SAVE_CMD           (0)
#endif 

/************************************************************************
*    "go" command.
*************************************************************************/
#ifndef FAPP_CFG_GO_CMD
    #define FAPP_CFG_GO_CMD             (0)
#endif 

/************************************************************************
*    "reset" command.
*************************************************************************/
#ifndef FAPP_CFG_RESET_CMD
    #define FAPP_CFG_RESET_CMD          (0)
#endif 

/************************************************************************
*    "reboot" command.
*************************************************************************/
#ifndef FAPP_CFG_REBOOT_CMD
    #define FAPP_CFG_REBOOT_CMD         (0)
#endif 

/************************************************************************
*    "bench" command.
*************************************************************************/
#ifndef FAPP_CFG_BENCH_CMD
    #define FAPP_CFG_BENCH_CMD          (0)
#endif 

/************************************************************************
*    "reinit" command. Used to test FNET release/init only.
*************************************************************************/
#ifndef FAPP_CFG_REINIT_CMD
    #define FAPP_CFG_REINIT_CMD         (0)
#endif 

/************************************************************************
*    "test" command. Used to test Telnet sending. For debug needs only
*************************************************************************/
#ifndef FAPP_CFG_TELNET_TEST_CMD
    #define FAPP_CFG_TELNET_TEST_CMD    (0)
#endif 

/************************************************************************
*    "dns" command.
*************************************************************************/
#ifndef FAPP_CFG_DNS_CMD
    #define FAPP_CFG_DNS_CMD            (0)
#endif

/************************************************************************
*    "bind" command.
*************************************************************************/
#ifndef FAPP_CFG_BIND_CMD
    #define FAPP_CFG_BIND_CMD           (0)
#endif

/************************************************************************
*    "unbind" command.
*************************************************************************/
#ifndef FAPP_CFG_UNBIND_CMD
    #define FAPP_CFG_UNBIND_CMD         (0)
#endif

/************************************************************************
*    "ping" command.
*************************************************************************/
#ifndef FAPP_CFG_PING_CMD
    #define FAPP_CFG_PING_CMD           (0)
#endif

/************************************************************************
*    "ping6" command. The same as "ping".
*************************************************************************/
#ifndef FAPP_CFG_PING6_CMD
    #define FAPP_CFG_PING6_CMD          (0)
#endif


/************************************************************************
*    "d" command. Used for DEBUGGING needs only.
*************************************************************************/
#ifndef FAPP_CFG_DEBUG_CMD
    #define FAPP_CFG_DEBUG_CMD          (0)
#endif 



/************************************************************************
*    "set/get" command.
*************************************************************************/
/* FNET TCP/IP stack set/get parameters. */
#ifndef FAPP_CFG_SETGET_CMD_IP
    #define FAPP_CFG_SETGET_CMD_IP      (0)
#endif
#ifndef FAPP_CFG_SETGET_CMD_GATEWAY
    #define FAPP_CFG_SETGET_CMD_GATEWAY (0)
#endif
#ifndef FAPP_CFG_SETGET_CMD_NETMASK
    #define FAPP_CFG_SETGET_CMD_NETMASK (0)
#endif
#ifndef FAPP_CFG_SETGET_CMD_MAC
    #define FAPP_CFG_SETGET_CMD_MAC     (0)
#endif

/* Bootloader set/get parameters. */
#ifndef FAPP_CFG_SETGET_CMD_BOOT
    #define FAPP_CFG_SETGET_CMD_BOOT    (0)
#endif
#ifndef FAPP_CFG_SETGET_CMD_DELAY
    #define FAPP_CFG_SETGET_CMD_DELAY   (0)
#endif
#ifndef FAPP_CFG_SETGET_CMD_SCRIPT
    #define FAPP_CFG_SETGET_CMD_SCRIPT  (0)
#endif
#ifndef FAPP_CFG_SETGET_CMD_RAW
    #define FAPP_CFG_SETGET_CMD_RAW     (0)
#endif

/* TFTP set/get parameters */
#ifndef FAPP_CFG_SETGET_CMD_TFTP
    #define FAPP_CFG_SETGET_CMD_TFTP    (0)
#endif
#ifndef FAPP_CFG_SETGET_CMD_IMAGE
    #define FAPP_CFG_SETGET_CMD_IMAGE   (0)
#endif
#ifndef FAPP_CFG_SETGET_CMD_TYPE
    #define FAPP_CFG_SETGET_CMD_TYPE    (0)
#endif
#ifndef FAPP_CFG_SETGET_CMD_GO
    #define FAPP_CFG_SETGET_CMD_GO      (0)
#endif

/* DNS set/get parameters */
#ifndef FAPP_CFG_SETGET_CMD_DNS
    #define FAPP_CFG_SETGET_CMD_DNS     (0)
#endif

#ifndef FAPP_CFG_SETGET_CMD
#define FAPP_CFG_SETGET_CMD             (FAPP_CFG_SETGET_CMD_IP|FAPP_CFG_SETGET_CMD_GATEWAY|\
                                         FAPP_CFG_SETGET_CMD_NETMASK|FAPP_CFG_SETGET_CMD_MAC|\
                                         FAPP_CFG_SETGET_CMD_BOOT|FAPP_CFG_SETGET_CMD_DELAY|\
                                         FAPP_CFG_SETGET_CMD_SCRIPT|FAPP_CFG_SETGET_CMD_RAW|\
                                         FAPP_CFG_SETGET_CMD_TFTP|FAPP_CFG_SETGET_CMD_IMAGE|\
                                         FAPP_CFG_SETGET_CMD_TYPE|FAPP_CFG_SETGET_CMD_GO|\
                                         FAPP_CFG_SETGET_CMD_DNS)
#endif


#ifndef FAPP_CFG_PARAMS_BOOT
#define FAPP_CFG_PARAMS_BOOT            (FAPP_CFG_SETGET_CMD_BOOT|FAPP_CFG_SETGET_CMD_DELAY|\
                                         FAPP_CFG_SETGET_CMD_SCRIPT|FAPP_CFG_SETGET_CMD_RAW|\
                                         FAPP_CFG_SETGET_CMD_GO)
#endif

#ifndef FAPP_CFG_PARAMS_TFTP
#define FAPP_CFG_PARAMS_TFTP            (FAPP_CFG_SETGET_CMD_TFTP|FAPP_CFG_SETGET_CMD_IMAGE|\
                                         FAPP_CFG_SETGET_CMD_TYPE|FAPP_CFG_SETGET_CMD_GO|\
                                         FAPP_CFG_TFTP_CMD|FAPP_CFG_TFTPUP_CMD|FAPP_CFG_TFTPS_CMD)
#endif


#ifndef FAPP_CFG_TFTPS_ON_WRITE_REQUEST_SCRIPT
    #define FAPP_CFG_TFTPS_ON_WRITE_REQUEST_SCRIPT      ""
#endif

#ifndef FAPP_CFG_TFTPS_AFTER_WRITE_REQUEST_SCRIPT    
    #define FAPP_CFG_TFTPS_AFTER_WRITE_REQUEST_SCRIPT   ""
#endif


/************************************************************************
*    TFTP data handlers
*************************************************************************/
#ifndef FAPP_CFG_TFTP_RX_RAW
    #define FAPP_CFG_TFTP_RX_RAW            (1)
#endif

#ifndef FAPP_CFG_TFTP_TX_RAW
    #define FAPP_CFG_TFTP_TX_RAW            (1)
#endif

#ifndef FAPP_CFG_TFTP_RX_BIN
    #define FAPP_CFG_TFTP_RX_BIN            (1)
#endif

#ifndef FAPP_CFG_TFTP_TX_BIN
    #define FAPP_CFG_TFTP_TX_BIN            (1)
#endif

#ifndef FAPP_CFG_TFTP_RX_SREC
    #define FAPP_CFG_TFTP_RX_SREC           (1)
#endif

#ifndef FAPP_CFG_TFTP_TX_SREC
    #define FAPP_CFG_TFTP_TX_SREC           (1)
#endif


#if !FAPP_CFG_TFTP_CMD && !FAPP_CFG_TFTPUP_CMD && !FAPP_CFG_TFTPS_CMD
    #undef FAPP_CFG_TFTP_RX_RAW
    #undef FAPP_CFG_TFTP_TX_RAW
    #undef FAPP_CFG_TFTP_RX_BIN
    #undef FAPP_CFG_TFTP_TX_BIN
    #undef FAPP_CFG_TFTP_RX_SREC
    #undef FAPP_CFG_TFTP_TX_SREC
    #define FAPP_CFG_TFTP_RX_RAW            (0)
    #define FAPP_CFG_TFTP_TX_RAW            (0)
    #define FAPP_CFG_TFTP_RX_BIN            (0)
    #define FAPP_CFG_TFTP_TX_BIN            (0)
    #define FAPP_CFG_TFTP_RX_SREC           (0)
    #define FAPP_CFG_TFTP_TX_SREC           (0)
#endif


/************************************************************************/
#ifndef FAPP_CFG_SHELL_MAX_LINE_LENGTH
    #define FAPP_CFG_SHELL_MAX_LINE_LENGTH  (60)
#endif 


/************************************************************************
*    Check conflict of FNET and FAPP parameters.
*************************************************************************/
#if !FNET_CFG_FLASH 
    #undef FAPP_CFG_SAVE_CMD
    #define FAPP_CFG_SAVE_CMD   (0)
#endif

/************************************************************************
*    Memory parameters
*************************************************************************/
/* Start address of the ROM memory, reserved/protected for the application. Used by the bootloader application.*/
#ifndef FAPP_CFG_APPLICATION_ADDRESS
#define FAPP_CFG_APPLICATION_ADDRESS      FNET_CFG_CPU_FLASH_ADDRESS
#endif

/* ROM memory size, reserved/protected for the application. Used by the bootloader application.*/
#ifndef FAPP_CFG_APPLICATION_SIZE 
#define FAPP_CFG_APPLICATION_SIZE         (48*1024) /* 48 KB */
#endif

#endif
