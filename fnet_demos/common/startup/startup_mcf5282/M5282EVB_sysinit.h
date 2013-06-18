/*
 * File:		m5282evb_sysinit.h
 * Purpose:		Generic Power-on Reset configuration
 *
 * Notes:
 *
 */

#ifndef __M5282EVB_SYSINIT_H__
#define __M5282EVB_SYSINIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if ENABLE_UART_SUPPORT==1 

#define TERMINAL_PORT       0
#define TERMINAL_BAUD       kBaud19200

#endif  /* ENABLE_UART_SUPPORT==1 */

#define SYSTEM_CLOCK_KHZ  64000     /* system bus frequency in kHz */


/* 
 * Memory map definitions from linker command files 
 */
extern uint8 __SDRAM[];
extern uint8 __SDRAM_SIZE[];
extern uint8 __EXT_FLASH[];
extern uint8 __EXT_FLASH_SIZE[];

/* 
 * Memory Map Info 
 */
#define SDRAM_ADDRESS			(uint32)__SDRAM
#define SDRAM_SIZE				(uint32)__SDRAM_SIZE

#define EXT_FLASH_ADDRESS		(uint32)__EXT_FLASH
#define EXT_FLASH_SIZE			(uint32)__EXT_FLASH_SIZE


/********************************************************************/
/* __initialize_hardware Startup code routine
 * 
 * __initialize_hardware is called by the startup code right after reset, 
 * with interrupt disabled and SP pre-set to a valid memory area.
 * Here you should initialize memory and some peripherics;
 * at this point global variables are not initialized yet.
 * The startup code will initialize SP on return of this function.
 */
void __initialize_hardware(void);

/********************************************************************/
/* __initialize_system Startup code routine
 * 
 * __initialize_system is called by the startup code when all languages 
 * specific initialization are done to allow additional hardware setup.
 */ 
void __initialize_system(void);



#ifdef __cplusplus
}
#endif

#endif /* __M5282EVB_SYSINIT_H__ */


