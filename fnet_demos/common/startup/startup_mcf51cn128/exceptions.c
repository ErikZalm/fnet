/*
 * File:    exceptions.c
 * Purpose: Generic exception handling for ColdFire processors
 *
 */

#include "derivative.h"
#include "exceptions.h"
#include "startcf.h"
#include "fapp_config.h"
#include "fnet.h"

#define VECTORDISPLAY(MESSAGE)                    asm { nop; };
#define VECTORDISPLAY2(MESSAGE,MESSAGE2)          asm { nop; };
#define VECTORDISPLAY3(MESSAGE,MESSAGE2,MESSAGE3) asm { nop; };


#ifdef __cplusplus
extern "C" {
#endif

extern unsigned long far _SP_INIT[];
extern unsigned long __VECTOR_RAM[];

/***********************************************************************/
/*
 * Handling of the TRK ColdFire libs (printf support in Debugger Terminal) 
 * 
 * To enable this support:  
 * - Set CONSOLE_IO_SUPPORT  1 in this file; this will enable 
 *   TrapHandler_printf for the trap #14 exception.
 *
 * - Make sure the file console_io_cf.c is in your project. 
 *
 * - In the debugger make sure that in the Connection "Setup" dialog, 
 *   "Debug Options" property page, the check box 
 *   "Enable Terminal printf support" is set.
 *   
 *
 * 
 */
#define CONSOLE_IO_SUPPORT  0 

#if CONSOLE_IO_SUPPORT
asm void TrapHandler_printf(void) {
   HALT
   RTE
}
#endif                                                   

/***********************************************************************/
/*
 * This is the handler for all exceptions which are not common to all 
 * ColdFire Chips.  
 *
 * Called by mcf_exception_handler
 * 
 */
void derivative_interrupt(unsigned long vector)
{
   if (vector < 64 || vector > 192) {
      VECTORDISPLAY2("User Defined Vector #%d\n",vector);
   }
}

/***********************************************************************
 *
 * This is the exception handler for all  exceptions common to all 
 * chips ColdFire.  Most exceptions do nothing, but some of the more 
 * important ones are handled to some extent.
 *
 * Called by asm_exception_handler 
 *
 * The ColdFire family of processors has a simplified exception stack
 * frame that looks like the following:
 *
 *              3322222222221111 111111
 *              1098765432109876 5432109876543210
 *           8 +----------------+----------------+
 *             |         Program Counter         |
 *           4 +----------------+----------------+
 *             |FS/Fmt/Vector/FS|      SR        |
 *   SP -->  0 +----------------+----------------+
 *
 * The stack self-aligns to a 4-byte boundary at an exception, with
 * the FS/Fmt/Vector/FS field indicating the size of the adjustment
 * (SP += 0,1,2,3 bytes).
 *             31     28 27      26 25    18 17      16 15                                  0
 *           4 +---------------------------------------+------------------------------------+
 *             | Format | FS[3..2] | Vector | FS[1..0] |                 SR                 |
 *   SP -->  0 +---------------------------------------+------------------------------------+
 */ 
#define MCF5XXX_RD_SF_FORMAT(PTR)   \
   ((*((unsigned short *)(PTR)) >> 12) & 0x00FF)

#define MCF5XXX_RD_SF_VECTOR(PTR)   \
   ((*((unsigned short *)(PTR)) >>  2) & 0x00FF)

#define MCF5XXX_RD_SF_FS(PTR)    \
   ( ((*((unsigned short *)(PTR)) & 0x0C00) >> 8) | (*((unsigned short *)(PTR)) & 0x0003) )

#define MCF5XXX_SF_SR(PTR)    *(((unsigned short *)(PTR))+1)

#define MCF5XXX_SF_PC(PTR)    *((unsigned long *)(PTR)+1)

#define MCF5XXX_EXCEPTFMT     "%s -- PC = %#08X\n"


void mcf_exception_handler(void *framepointer) 
{
   volatile unsigned long exceptionStackFrame = (*(unsigned long *)(framepointer)); 
   volatile unsigned short stackFrameSR       = MCF5XXX_SF_SR(framepointer);  
   volatile unsigned short stackFrameWord     = (*(unsigned short *)(framepointer));  
   volatile unsigned long  stackFrameFormat   = (unsigned long)MCF5XXX_RD_SF_FORMAT(&stackFrameWord);
   volatile unsigned long  stackFrameFS       = (unsigned long)MCF5XXX_RD_SF_FS(&stackFrameWord);
   volatile unsigned long  stackFrameVector   = (unsigned long)MCF5XXX_RD_SF_VECTOR(&stackFrameWord);
   volatile unsigned long  stackFramePC       = MCF5XXX_SF_PC(framepointer);

   switch (stackFrameFormat)
   {
      case 4:
      case 5:
      case 6:
      case 7:
         break;
      default:
         VECTORDISPLAY3(MCF5XXX_EXCEPTFMT,"Illegal stack type", stackFramePC);
         break;
   }

   switch (stackFrameVector)
   {
   case 2:
      VECTORDISPLAY3(MCF5XXX_EXCEPTFMT, "Access Error", stackFramePC);
      switch (stackFrameFS)
      {
         case 4:
            VECTORDISPLAY("Error on instruction fetch\n");
            break;
         case 8:
            VECTORDISPLAY("Error on operand write\n");
            break;
         case 9:
            VECTORDISPLAY("Attempted write to write-protected space\n");
            break;
         case 12:
            VECTORDISPLAY("Error on operand read\n");
            break;
         default:
            VECTORDISPLAY("Reserved Fault Status Encoding\n");
            break;
      }
      break;
   case 3:
      VECTORDISPLAY3(MCF5XXX_EXCEPTFMT, "Address Error", stackFramePC);
      switch (stackFrameFS)
      {
         case 4:
            VECTORDISPLAY("Error on instruction fetch\n");
            break;
         case 8:
            VECTORDISPLAY("Error on operand write\n");
            break;
         case 9:
            VECTORDISPLAY("Attempted write to write-protected space\n");
            break;
         case 12:
            VECTORDISPLAY("Error on operand read\n");
            break;
         default:
            VECTORDISPLAY("Reserved Fault Status Encoding\n");
            break;
      }
      break;
   case 4:
      VECTORDISPLAY3(MCF5XXX_EXCEPTFMT, "Illegal instruction", stackFramePC);
      break;
   case 8:
      VECTORDISPLAY3(MCF5XXX_EXCEPTFMT, "Privilege violation", stackFramePC);
      break;
   case 9:
      VECTORDISPLAY3(MCF5XXX_EXCEPTFMT, "Trace Exception", stackFramePC);
      break;
   case 10:
      VECTORDISPLAY3(MCF5XXX_EXCEPTFMT, "Unimplemented A-Line Instruction", 	stackFramePC);
      break;
   case 11:
      VECTORDISPLAY3(MCF5XXX_EXCEPTFMT, "Unimplemented F-Line Instruction", 	stackFramePC);
      break;
   case 12:
      VECTORDISPLAY3(MCF5XXX_EXCEPTFMT, "Debug Interrupt", stackFramePC);
      break;
   case 14:
      VECTORDISPLAY3(MCF5XXX_EXCEPTFMT, "Format Error", stackFramePC);
      break;
   case 15:
      VECTORDISPLAY3(MCF5XXX_EXCEPTFMT, "Unitialized Interrupt", stackFramePC);
      break;
   case 24:
      VECTORDISPLAY3(MCF5XXX_EXCEPTFMT, "Spurious Interrupt", stackFramePC);
      break;
   case 25:
   case 26:
   case 27:
   case 28:
   case 29:
   case 30:
   case 31:
      VECTORDISPLAY2("Autovector interrupt level %d\n", stackFrameVector - 24);
      break;
   case 32:
   case 33:
   case 34:
   case 35:
   case 36:
   case 37:
   case 38:
   case 39:
   case 40:
   case 41:
   case 42:
   case 43:
   case 44:
   case 45:
   case 46:
   case 47:
      VECTORDISPLAY2("TRAP #%d\n", stackFrameVector - 32);
      break;
   case 5:
   case 6:
   case 7:
   case 13:
   case 16:
   case 17:
   case 18:
   case 19:
   case 20:
   case 21:
   case 22:
   case 23:
   case 48:
   case 49:
   case 50:
   case 51:
   case 52:
   case 53:
   case 54:
   case 55:
   case 56:
   case 57:
   case 58:
   case 59:
   case 60:
   case 61:
   case 62:
   case 63:
      VECTORDISPLAY2("Reserved: #%d\n", stackFrameVector);
      break;
   default:
      derivative_interrupt(stackFrameVector);
   break;
   }
}

__declspec(register_abi)
asm void asm_exception_handler(void)
{
   link     a6,#0 
   lea     -20(sp), sp
   movem.l d0-d2/a0-a1, (sp)
   lea     24(sp),a0   /* A0 point to exception stack frame on the stack */
   jsr     mcf_exception_handler
   movem.l (sp), d0-d2/a0-a1
   lea     20(sp), sp
   unlk a6
   rte
}


typedef void (* vectorTableEntryType)(void);


#pragma define_section vectortable ".vectortable" far_absolute R

/*
 *  MCF51CN128 vector table
 *  CF V1 has 114 vector + SP_INIT in the vector table (115 entries)
 */
#if FAPP_CFG_PREINSTALL_INTERRUPTS
    const
#endif 
__declspec(vectortable) vectorTableEntryType _vect[115] = {   /* Interrupt vector table */
   (vectorTableEntryType)&_SP_INIT, /*   0 (0x000) Initial supervisor SP      */
   _startup,                        /*   1 (0x004) Initial PC                 */
   asm_exception_handler,           /*   2 (0x008) Access Error               */
   asm_exception_handler,           /*   3 (0x00C) Address Error              */
   asm_exception_handler,           /*   4 (0x010) Illegal Instruction        */
   asm_exception_handler,           /*   5 (0x014) Reserved                   */
   asm_exception_handler,           /*   6 (0x018) Reserved                   */
   asm_exception_handler,           /*   7 (0x01C) Reserved                   */
   asm_exception_handler,           /*   8 (0x020) Privilege Violation        */
   asm_exception_handler,           /*   9 (0x024) Trace                      */
   asm_exception_handler,           /*  10 (0x028) Unimplemented A-Line       */
   asm_exception_handler,           /*  11 (0x02C) Unimplemented F-Line       */
   asm_exception_handler,           /*  12 (0x030) Debug Interrupt            */
   asm_exception_handler,           /*  13 (0x034) Reserved                   */
   asm_exception_handler,           /*  14 (0x038) Format Error               */
   asm_exception_handler,           /*  15 (0x03C) Unitialized Int            */
   asm_exception_handler,           /*  16 (0x040) Reserved                   */
   asm_exception_handler,           /*  17 (0x044) Reserved                   */
   asm_exception_handler,           /*  18 (0x048) Reserved                   */
   asm_exception_handler,           /*  19 (0x04C) Reserved                   */
   asm_exception_handler,           /*  20 (0x050) Reserved                   */
   asm_exception_handler,           /*  21 (0x054) Reserved                   */
   asm_exception_handler,           /*  22 (0x058) Reserved                   */
   asm_exception_handler,           /*  23 (0x05C) Reserved                   */
   asm_exception_handler,           /*  24 (0x060) Spurious Interrupt         */
   asm_exception_handler,           /*  25 (0x064) Reserved                   */
   asm_exception_handler,           /*  26 (0x068) Reserved                   */
   asm_exception_handler,           /*  27 (0x06C) Reserved                   */
   asm_exception_handler,           /*  28 (0x070) Reserved                   */
   asm_exception_handler,           /*  29 (0x074) Reserved                   */
   asm_exception_handler,           /*  30 (0x078) Reserved                   */
   asm_exception_handler,           /*  31 (0x07C) Reserved                   */
   asm_exception_handler,           /*  32 (0x080) TRAP #0                    */
   asm_exception_handler,           /*  33 (0x084) TRAP #1                    */
   asm_exception_handler,           /*  34 (0x088) TRAP #2                    */
   asm_exception_handler,           /*  35 (0x08C) TRAP #3                    */
   asm_exception_handler,           /*  36 (0x090) TRAP #4                    */
   asm_exception_handler,           /*  37 (0x094) TRAP #5                    */
   asm_exception_handler,           /*  38 (0x098) TRAP #6                    */
   asm_exception_handler,           /*  39 (0x09C) TRAP #7                    */
   asm_exception_handler,           /*  40 (0x0A0) TRAP #8                    */
   asm_exception_handler,           /*  41 (0x0A4) TRAP #9                    */
   asm_exception_handler,           /*  42 (0x0A8) TRAP #10                   */
   asm_exception_handler,           /*  43 (0x0AC) TRAP #11                   */
   asm_exception_handler,           /*  44 (0x0B0) TRAP #12                   */
   asm_exception_handler,           /*  45 (0x0B4) TRAP #13                   */
#if CONSOLE_IO_SUPPORT   
   TrapHandler_printf,              /*  46 (0x0B8) TRAP #14                   */
#else
   asm_exception_handler,           /*  46 (0x0B8) TRAP #14                   */
#endif   
   asm_exception_handler,           /*  47 (0x0BC) TRAP #15                   */
   asm_exception_handler,           /*  48 (0x0C0) Reserved                   */
   asm_exception_handler,           /*  49 (0x0C4) Reserved                   */
   asm_exception_handler,           /*  50 (0x0C8) Reserved                   */
   asm_exception_handler,           /*  51 (0x0CC) Reserved                   */
   asm_exception_handler,           /*  52 (0x0D0) Reserved                   */
   asm_exception_handler,           /*  53 (0x0D4) Reserved                   */
   asm_exception_handler,           /*  54 (0x0D8) Reserved                   */
   asm_exception_handler,           /*  55 (0x0DC) Reserved                   */
   asm_exception_handler,           /*  56 (0x0E0) Reserved                   */
   asm_exception_handler,           /*  57 (0x0E4) Reserved                   */
   asm_exception_handler,           /*  58 (0x0E8) Reserved                   */
   asm_exception_handler,           /*  59 (0x0EC) Reserved                   */
   asm_exception_handler,           /*  60 (0x0F0) Reserved                   */
   asm_exception_handler,           /*  61 (0x0F4) unsinstr                   */
   asm_exception_handler,           /*  62 (0x0F8) Reserved                   */
   asm_exception_handler,           /*  63 (0x0FC) Reserved                   */
   asm_exception_handler,           /*  64 (0x100) irq                        */
   asm_exception_handler,           /*  65 (0x104) lvd                        */
   asm_exception_handler,           /*  66 (0x108) lol                        */
   asm_exception_handler,           /*  67 (0x10C) tpm1ch0                    */
   asm_exception_handler,           /*  68 (0x110) tpm1ch1                    */
   asm_exception_handler,           /*  69 (0x114) tpm1ch2                    */
   asm_exception_handler,           /*  70 (0x118) tpm1ovf                    */
   asm_exception_handler,           /*  71 (0x11C) mtim1                      */
   asm_exception_handler,           /*  72 (0x120) tpm2ch0                    */
   asm_exception_handler,           /*  73 (0x124) tpm2ch1                    */
   asm_exception_handler,           /*  74 (0x128) tpm2ch2                    */
   asm_exception_handler,           /*  75 (0x12C) tpm2ovf                    */
   asm_exception_handler,           /*  76 (0x130) spi1                       */
   asm_exception_handler,           /*  77 (0x134) spi2                       */
   asm_exception_handler,           /*  78 (0x138) mtim2                      */
   asm_exception_handler,           /*  79 (0x13C) sci1err                    */
   asm_exception_handler,           /*  80 (0x140) sci1rx                     */
   asm_exception_handler,           /*  81 (0x144) sci1tx                     */
   asm_exception_handler,           /*  82 (0x148) sci2err                    */
   asm_exception_handler,           /*  83 (0x14C) sci2rx                     */
   asm_exception_handler,           /*  84 (0x150) sci2tx                     */
   asm_exception_handler,           /*  85 (0x154) sci3or                     */
   asm_exception_handler,           /*  86 (0x158) fectxf                     */
#if FAPP_CFG_PREINSTALL_INTERRUPTS   
    fnet_cpu_isr,                   /*  87 (0x15C) fecrxf                     */
#else
    asm_exception_handler,          /*  87 (0x15C) fecrxf                     */
#endif   
   asm_exception_handler,           /*  88 (0x160) fecother                   */
   asm_exception_handler,           /*  89 (0x164) fechberr                   */
   asm_exception_handler,           /*  90 (0x168) fecbabr                    */
   asm_exception_handler,           /*  91 (0x16C) fecbabt                    */
   asm_exception_handler,           /*  92 (0x170) fecgra                     */
   asm_exception_handler,           /*  93 (0x174) fectxb                     */
   asm_exception_handler,           /*  94 (0x178) fecrxb                     */
   asm_exception_handler,           /*  95 (0x17C) fecmii                     */
   asm_exception_handler,           /*  96 (0x180) feceberr                   */
   asm_exception_handler,           /*  97 (0x184) feclc                      */
   asm_exception_handler,           /*  98 (0x188) fecrl                      */
   asm_exception_handler,           /*  99 (0x18C) fecun                      */
   asm_exception_handler,           /* 100 (0x190) sci3err                    */
   asm_exception_handler,           /* 101 (0x194) sci3rx                     */
   asm_exception_handler,           /* 102 (0x198) sci3tx                     */
   asm_exception_handler,           /* 103 (0x19C) L7swi                      */
   asm_exception_handler,           /* 104 (0x1A0) L6swi                      */
   asm_exception_handler,           /* 105 (0x1A4) L5swi                      */
   asm_exception_handler,           /* 106 (0x1A8) L4swi                      */
   asm_exception_handler,           /* 107 (0x___) L3swi                      */
   asm_exception_handler,           /* 108 (0x___) L2swi                      */
   asm_exception_handler,           /* 109 (0x___) L1swi                      */
   asm_exception_handler,           /* 110 (0x___) iic1                       */
   asm_exception_handler,           /* 111 (0x___) iic2                       */
   asm_exception_handler,           /* 112 (0x___) adc                        */
   asm_exception_handler,           /* 113 (0x___) keyboard                   */
#if FAPP_CFG_PREINSTALL_INTERRUPTS   
    fnet_cpu_isr,                   /* 114 (0x___) rtc                        */
#else
    asm_exception_handler,          /* 114 (0x___) rtc                        */
#endif   
   
};

/********************************************************************
 * MCF5xxx ASM utility functions
 */
__declspec(standard_abi)  
static asm void mcf5xxx_wr_vbr(unsigned long) { /* Set VBR */
	move.l	4(SP),D0
    movec d0,VBR 
	nop
	rts	
}	

/********************************************************************
 * MCF5xxx startup copy functions:
 *
 * Set VBR and performs RAM vector table initializatiom.
 * The following symbol should be defined in the lcf:
 * __VECTOR_RAM
 *
 * _vect is the start of the exception table in the code
 * In case _vect address is different from __VECTOR_RAM,
 * the vector table is copied from _vect to __VECTOR_RAM.
 * In any case VBR is set to __VECTOR_RAM.
 */ 
void initialize_exceptions(void)
{
	/*
	 * Memory map definitions from linker command files used by mcf5xxx_startup
	 */

	register unsigned long n;
    
	/* 
     * Copy the vector table to RAM 
     */
	if (__VECTOR_RAM != (unsigned long*)_vect)
	{
		for (n = 0; n < 256; n++)
			__VECTOR_RAM[n] = (unsigned long)_vect[n];
	}
	mcf5xxx_wr_vbr((unsigned long)__VECTOR_RAM);
}


#ifdef __cplusplus
}
#endif


