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
* @file fapp_params.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.16.0
*
* @brief FNET Application parameters.
*
***************************************************************************/
#ifndef _FAPP_PARAMS_H_

#define _FAPP_PARAMS_H_

#include "fapp.h"

/**************************************************************************/ /*!
 * @brief Parameters-version string.@n
 * It defines version of the parameter structure saved in a persistent storage.
 ******************************************************************************/
#define FAPP_PARAMS_VERSION                 "04" /* Changed on any change in the param. structures.*/

/**************************************************************************/ /*!
 * @brief Signature string value.@n
 * It's used for simple check if configuration structure is present 
 * in a persistant storage.
 ******************************************************************************/
#define FAPP_PARAMS_SIGNATURE               "FNET"FAPP_PARAMS_VERSION

/**************************************************************************/ /*!
 * @brief The maximum length of the signature.
 ******************************************************************************/
#define FAPP_PARAMS_SIGNATURE_SIZE          (16)

/**************************************************************************/ /*!
 * @brief The maximum length of the boot-script.
 ******************************************************************************/
#define FAPP_PARAMS_BOOT_SCRIPT_SIZE        (60)

/**************************************************************************/ /*!
 * @brief The maximum length of the @c file_name field of the 
 * @ref fapp_params_tftp structure.
 ******************************************************************************/
#define FAPP_PARAMS_TFTP_FILE_NAME_SIZE     (40)


/**************************************************************************/ /*!
 * @brief Boot mode.
 ******************************************************************************/
typedef enum
{
    FAPP_PARAMS_BOOT_MODE_STOP      = (0),  /**< @brief Stop at the shell prompt. */
    FAPP_PARAMS_BOOT_MODE_GO        = (1),  /**< @brief Boot from flash. @n
                                             * The entry point address is defined 
                                             * by @ref fapp_params_boot.go_address.
                                             * The boot will be automatically 
                                             * started after @ref @ref fapp_params_boot.delay 
                                             * seconds.*/
    FAPP_PARAMS_BOOT_MODE_SCRIPT    = (2)   /**< @brief Start boot-script. @n
                                             * The boot-script commands located 
                                             * in the @ref fapp_params_boot.script
                                             * will be automatically started after 
                                             * @ref @ref fapp_params_boot.delay 
                                             * seconds.*/
} 
fapp_params_boot_mode_t;

/**************************************************************************/ /*!
 * @brief Image-file type.
 ******************************************************************************/
typedef enum
{
    FAPP_PARAMS_TFTP_FILE_TYPE_RAW  = (0),  /**< @brief Raw binary file. */
    FAPP_PARAMS_TFTP_FILE_TYPE_BIN  = (1),  /**< @brief CodeWarrior binary file.*/
    FAPP_PARAMS_TFTP_FILE_TYPE_SREC = (2)   /**< @brief SREC file.*/
} 
fapp_params_tftp_file_type_t;

/**************************************************************************/ /*!
 * @brief Application parameters structure used to save the FNET Stack 
 * specific configuration to a persistent storage.
 ******************************************************************************/
struct fapp_params_fnet
{
    unsigned long address;	/**< @brief Application IP address.*/
    unsigned long netmask;	/**< @brief Netmask.*/
    unsigned long gateway;	/**< @brief Gateway IP address.*/
    unsigned long dns;		/**< @brief DNS server address. */                            
    unsigned char mac[6];   /**< @brief Ethernet MAC address.*/
};

/**************************************************************************/ /*!
 * @brief Application parameter structure used to save the bootloader 
 * specific configuration to a persistent storage.
 ******************************************************************************/
struct fapp_params_boot
{
    unsigned long mode;                         /**< @brief Boot mode defined by the 
                                                 * @ref fapp_params_boot_mode_t.*/
    unsigned long delay;                        /**< @brief Boot delay.@n
											     * After bootup, the bootloader will wait 
											     * this number of seconds before it executes 
											     * the boot-mode defined by @c mode field.
											     * During this time a countdown is printed, 
											     * which can be interrupted by pressing any key. @n
											     * Set this variable to 0, to boot without delay. @n
											     * It's ignored for the @ref FAPP_PARAMS_BOOT_MODE_STOP
											     * mode.*/
    unsigned long go_address;                   /**< @brief Default entry point address
                                                 * to start execution at. @n
                                                 * It is used by bootloader in @ref FAPP_PARAMS_BOOT_MODE_GO
                                                 * mode as the default entry point. Also it is used 
                                                 * as the default address for the "go" shell command 
                                                 * if no address is provided to.*/                                      
    char script[FAPP_PARAMS_BOOT_SCRIPT_SIZE];	/**< @brief Command script string. @n
                             	                 * It is automatically executed when the 
                             	 	 	 	     * @ref FAPP_PARAMS_BOOT_MODE_SCRIPT mode
                                                 * is set and the initial countdown is not interrupted. @n
                                                 * This script may contain any command supported by 
                                                 * the application shell. The commands must be split 
                                                 * by semicolon operator. @n
                                                 * The string must be null-terminated.*/
};

/**************************************************************************/ /*!
 * @brief Application parameter structure used to save the TFTP loader 
 * specific configuration to a persistent storage.
 ******************************************************************************/
struct fapp_params_tftp
{
    struct sockaddr server_addr;                     /**< @brief This is the default TFTP server 
                                                      * socket address to be used for network download 
                                                      * if no address is provided to the "tftp" 
                                                      * shell command.*/
    unsigned long file_type;                         /**< @brief This is the default file type, defined 
                                                      * by the @ref fapp_params_tftp_file_type_t, 
                                                      * to be used for network download if no 
                                                      * type is provided to the "tftp" shell command. */                            
    unsigned long file_raw_address;                  /**< @brief Load address for raw-binary file 
                                                      * for the TFTP loader. @n
                                                      * It's used only if @c file_type is set to 
                                                      * @ref FAPP_PARAMS_TFTP_FILE_TYPE_RAW.
                                                      */                
    char file_name[FAPP_PARAMS_TFTP_FILE_NAME_SIZE]; /**< @brief This is the default file name
                                                      * to be loaded by TFTP loader if no 
                                                      * file name is provided to the "tftp" shell command. @n
                                                      * The string must be null-terminated.*/
};

/**************************************************************************/ /*!
 * @brief Main application  parameter structure used to save the
 * application specific configuration to a persistent storage.
 ******************************************************************************/
struct fapp_params_flash
{
    char signature[FAPP_PARAMS_SIGNATURE_SIZE]; /**< @brief Signature string.@n
                                                 * It's used for simple check if configuration 
                                                 * structure is present in a persistent storage.*/
    struct fapp_params_fnet fnet_params;        /**< @brief FNET TCP/IP stack specific 
                                                 * configuration parameters.*/
    struct fapp_params_boot boot_params;        /**< @brief Bootloader specific 
                                                 * configuration parameters*/
    struct fapp_params_tftp tftp_params;        /**< @brief TFTP loader specific 
                                                 * configuration parameters.*/
};

#endif /* _FAPP_PARAMS_H_ */
