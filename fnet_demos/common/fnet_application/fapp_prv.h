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
* @file fapp_prv.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.24.0
*
* @brief FNET Application private definitions.
*
***************************************************************************/

#ifndef _FAPP_PRV_H_

#define _FAPP_PRV_H_

#include "fapp.h"
#include "fapp_params_prv.h"


/************************************************************************
*    Definitions.
*************************************************************************/
extern const char FAPP_HELP_STR[];
extern const char FAPP_DELIMITER_STR[];
extern const char FAPP_CANCELLED_STR[];
extern const char FAPP_TOCANCEL_STR[];
extern const char FAPP_COMMAND_RELEASE[]; /* Service release command */

extern const char FAPP_PARAM_ERR[];
extern const char FAPP_NET_ERR[];
extern const char FAPP_INIT_ERR[];

extern const char FAPP_SHELL_INFO_FORMAT_S[];
extern const char FAPP_SHELL_INFO_FORMAT_D[];
extern const char FAPP_SHELL_INFO_ENABLED[];
extern const char FAPP_SHELL_INFO_DISABLED[];
extern const char FAPP_SHELL_CANCELED_CTRL_C[];

/************************************************************************
*     Function Prototypes
*************************************************************************/
void fapp_info_print( fnet_shell_desc_t desc );
void fapp_help_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
void fapp_shell_init( fnet_shell_desc_t desc );
void fapp_netif_info_print( fnet_shell_desc_t desc, fnet_netif_desc_t netif );
void fapp_info_cmd( fnet_shell_desc_t desc, int argc, char ** argv );
void fapp_save_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
void fapp_reset_cmd( fnet_shell_desc_t desc, int argc, char ** argv );
void fapp_go_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
void fapp_bind_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
void fapp_unbind_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
void fapp_netif_addr_print(fnet_shell_desc_t desc, fnet_address_family_t family, fnet_netif_desc_t netif, int print_type);

extern fnet_netif_desc_t fapp_default_netif;
extern const struct fnet_shell_command fapp_cmd_table [];

/* Boot mode structure. */
struct boot_mode
{
    unsigned char index;
    char * name;
    void(* handler)(fnet_shell_desc_t desc);
};

const struct boot_mode *fapp_boot_mode_by_name (char *name);
const struct boot_mode *fapp_boot_mode_by_index (unsigned long index);


#endif
