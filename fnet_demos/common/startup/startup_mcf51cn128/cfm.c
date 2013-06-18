/* CFM init */
#include "fapp_mem.h"

/* Sector Size.*/
#define FAPP_CFM_SECTOR_SIZE    (2*1024) 

/* Number of sectors occupied by the application.*/
#define FAPP_APPLICATION_SECTOR_NUMBER    (FAPP_APPLICATION_SIZE/FAPP_CFM_SECTOR_SIZE+(FAPP_APPLICATION_SIZE%FAPP_CFM_SECTOR_SIZE?1:0))

/* CFM Protection register value.*/
#define FAPP_CFMPROT_REG	(((((~FAPP_APPLICATION_SECTOR_NUMBER)<<1)+1)<<16)+0xFFFF)

#pragma define_section cfmconfig ".cfmconfig"  far_absolute R
#pragma explicit_zero_data on

__declspec(cfmconfig)  unsigned long _cfm[4] = 
{
	0xFFFFFFFF, /* KEY_UPPER (0x00000400) */
	0xFFFFFFFF, /* KEY_LOWER (0x00000404) */
	0xFFFFFFFF, /* reserved  (0x00000408) */
#if FAPP_CFG_CFM_PROTECTION	
	FAPP_CFMPROT_REG,   /* (0x0000040C) rserved, NVPROT (0x0000040D), 
	                     * (0x0000040E) rserved, NVOPT (0x0000040F) */
#else
    0xFFFFFFFF, /* No protection. */
#endif	

};
