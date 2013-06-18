/**************************************************************************
* 
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
* @file fnet_mcf_serial.c
*
* @author Andrey Butok
*
* @date Oct-26-2012
*
* @version 0.0.15.0
*
* @brief ColdFire Serial port I/O functions.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_MCF

#include "fnet.h"

/********************************************************************/
void fnet_cpu_serial_putchar( long port_number, int character)
{
#if FNET_CFG_MCF_SCI /* Lasko */ 
    /* Wait for the transmitter to be empty.
     */  
    while (!FNET_MCF_SCI_S1_TDRE(port_number))
    {};
    /* Send a character by SCI */
    FNET_MCF_SCI_D(port_number) = (unsigned char)character;                
#endif    

#if FNET_CFG_MCF_UART  
    /* Wait until space is available in the FIFO. */
    while(!(FNET_MCF_UART_USR(port_number) & FNET_MCF_UART_USR_TXRDY))
    {};
    /* Send the character */
    FNET_MCF_UART_UTB(port_number) = (unsigned char)character;
#endif
      
}

/********************************************************************/
int fnet_cpu_serial_getchar( long port_number )
{
#if FNET_CFG_MCF_SCI /* Lasko */  
    /* If character received. */
    if(FNET_MCF_SCI_S1_RDRF(port_number))
        return FNET_MCF_SCI_D(port_number);
#endif    

#if FNET_CFG_MCF_UART  
    /* If character received. */
    if(FNET_MCF_UART_USR(port_number) & FNET_MCF_UART_USR_RXRDY)
        return FNET_MCF_UART_URB(port_number);
#endif    

    return FNET_ERR;    
}

/********************************************************************/
static inline void fnet_cpu_serial_gpio_init(long port_number) 
{
    /* Enable the proper UART pins */
    
#if FNET_CFG_CPU_MCF52259 || FNET_CFG_CPU_MCF52235
    switch (port_number)
    {
        case 2:
            /* Set Port UC to initialize URXD2/UTXD2. */
            FNET_MCF522XX_GPIO_PUCPAR = 0
                | FNET_MCF522XX_GPIO_PUCPAR_URXD2_URXD2
                | FNET_MCF522XX_GPIO_PUCPAR_UTXD2_UTXD2;
            break;
        case 1:
            /* Set Port UB to initialize URXD1/UTXD1. */
            FNET_MCF522XX_GPIO_PUBPAR = 0
                | FNET_MCF522XX_GPIO_PUBPAR_URXD1_URXD1
                | FNET_MCF522XX_GPIO_PUBPAR_UTXD1_UTXD1;
            break;
        /* case 0:*/
        default:
            /* Set Port UA to initialize URXD0/UTXD0. */
            FNET_MCF522XX_GPIO_PUAPAR = 0
                | FNET_MCF522XX_GPIO_PUAPAR_URXD0_URXD0
                | FNET_MCF522XX_GPIO_PUAPAR_UTXD0_UTXD0;
    }
#endif /* FNET_CFG_CPU_MCF52259 || FNET_CFG_CPU_MCF52235 */

#if FNET_CFG_CPU_MCF54418 /* Modelo */
	/* Set GPIO port register to enable PSC(port) signals */
	switch( port_number )
	{
		default:
		case 0:
			FNET_MCF5441X_GPIO_PAR_UART0 = (0
				| FNET_MCF5441X_GPIO_PAR_UART0_PAR_TXD_U0TXD
				| FNET_MCF5441X_GPIO_PAR_UART0_PAR_RXD_U0RXD);
			FNET_MCF_PMM_PPMLR0 &= ~(FNET_MCF_PMM_PPMLR0_CD24);	/* Enable clock */
	    	break;
		case 1:
			FNET_MCF5441X_GPIO_PAR_UART1 = (0
				| FNET_MCF5441X_GPIO_PAR_UART1_PAR_TXD_U1TXD
				| FNET_MCF5441X_GPIO_PAR_UART1_PAR_RXD_U1RXD);
			FNET_MCF_PMM_PPMLR0 &= ~(FNET_MCF_PMM_PPMLR0_CD25);	/* Enable clock */
			break;
		case 2:
			FNET_MCF5441X_GPIO_PAR_UART2 = (0
				| FNET_MCF5441X_GPIO_PAR_UART2_PAR_TXD_U2TXD
				| FNET_MCF5441X_GPIO_PAR_UART2_PAR_RXD_U2RXD);
			FNET_MCF_PMM_PPMLR0 &= ~(FNET_MCF_PMM_PPMLR0_CD26);	/* Enable clock */
			break;
		case 3:
			FNET_MCF5441X_GPIO_PAR_DSPIOWH = (0
				| FNET_MCF5441X_GPIO_PAR_DSPIOWH_PAR_SOUT_U3TXD
				| FNET_MCF5441X_GPIO_PAR_DSPIOWH_PAR_SIN_U3RXD);
			FNET_MCF_PMM_PPMLR0 &= ~(FNET_MCF_PMM_PPMLR0_CD27);	/* Enable clock */
			break;		
		case 4:
			FNET_MCF5441X_GPIO_PAR_UART0 = (0
				| FNET_MCF5441X_GPIO_PAR_UART0_PAR_CTS_U4TXD
				| FNET_MCF5441X_GPIO_PAR_UART0_PAR_RTS_U4RXD);
			FNET_MCF_PMM_PPMLR1 &= ~(FNET_MCF_PMM_PPMLR1_CD24);	/* Enable clock */
			break;	
		case 5:
			FNET_MCF5441X_GPIO_PAR_UART1 = (0
				| FNET_MCF5441X_GPIO_PAR_UART1_PAR_CTS_U5TXD
				| FNET_MCF5441X_GPIO_PAR_UART1_PAR_RTS_U5RXD);
			FNET_MCF_PMM_PPMLR1 &= ~(FNET_MCF_PMM_PPMLR1_CD25);	/* Enable clock */
			break;	
		case 6:
			FNET_MCF5441X_GPIO_PAR_UART2 = (0
				| FNET_MCF5441X_GPIO_PAR_UART2_PAR_CTS_U6TXD
				| FNET_MCF5441X_GPIO_PAR_UART2_PAR_RTS_U6RXD);
			FNET_MCF_PMM_PPMLR1 &= ~(FNET_MCF_PMM_PPMLR1_CD26);	/* Enable clock */
			break;			
	}
#endif	/* FNET_CFG_CPU_MCF54418 */
	
    
#if FNET_CFG_MCF_SCI /* V1 Lasco */  
       
    switch (port_number)
    {
        case 2:  /* Not tested.*/
            /* Enable pins. */
            FNET_MCF_PTEPF1_E6 = 3;
            FNET_MCF_PTEPF1_E7 = 3;    

            /* Enable bus clock to the SCI3 module.*/
            FNET_MCF_SCGC2_SCI3 = 1;
            break;
        default:
        /* case 1: */
            /* Enable pins. */
            FNET_MCF_PTDPF2_D2 = 2;
            FNET_MCF_PTDPF2_D3 = 2;  

            /* eNABLE bus clock to the SCI2 module.*/
            FNET_MCF_SCGC1_SCI2 = 1;
            break;
        case 0:
            /* Enable pins. */
            FNET_MCF_PTDPF2_D0 = 0x2;   /* Set PTD0 as TXD1 */
            FNET_MCF_PTDPF2_D1 = 0x2;   /* Set PTD1 as RXD1 */	 

            /* Enable bus clock to the SCI1 module.*/
            FNET_MCF_SCGC1_SCI1 = 1;
            break;
    }
    
#endif
    
    (void)port_number;
}

/********************************************************************/
void fnet_cpu_serial_init(long port_number, unsigned long baud_rate)
{
	/*
	 * Initialize UART for serial communications
	 */
	fnet_uint16 ubgs;
    
    /* Init GPIO.*/
    fnet_cpu_serial_gpio_init(port_number); 
    

#if FNET_CFG_MCF_UART  
	/* Reset Transmitter. */
	FNET_MCF_UART_UCR(port_number) = FNET_MCF_UART_UCR_RESET_TX;

	/* Reset Receiver. */
	FNET_MCF_UART_UCR(port_number) = FNET_MCF_UART_UCR_RESET_RX;

	/* Reset Mode Register. */
	FNET_MCF_UART_UCR(port_number) = FNET_MCF_UART_UCR_RESET_MR;

	/* No parity, 8-bits per character. */
	FNET_MCF_UART_UMR(port_number) = (0
		| FNET_MCF_UART_UMR_PM_NONE
		| FNET_MCF_UART_UMR_BC_8 );

	/* No echo or loopback, 1 stop bit. */
	FNET_MCF_UART_UMR(port_number) = (0
		| FNET_MCF_UART_UMR_CM_NORMAL
		| FNET_MCF_UART_UMR_SB_STOP_BITS_1);

	/* Set Rx and Tx baud by SYSTEM CLOCK. */
	FNET_MCF_UART_UCSR(port_number) = (0
		| FNET_MCF_UART_UCSR_RCS_SYS_CLK
		| FNET_MCF_UART_UCSR_TCS_SYS_CLK);

	/*
	 * Mask all UART interrupts
	 */
	FNET_MCF_UART_UIMR(port_number) = 0;

	/*
	 * Calculate baud settings
	 */

	ubgs = (fnet_uint16)(FNET_CFG_CPU_CLOCK_HZ/(baud_rate * 32));

	FNET_MCF_UART_UBG1(port_number) = (fnet_uint8)((ubgs >> 8) & 0xFF);
	FNET_MCF_UART_UBG2(port_number) = (fnet_uint8)(ubgs & 0xFF);


	/*
	 * Enable receiver and transmitter
	 */
	FNET_MCF_UART_UCR(port_number) = (fnet_uint8)(0
		| FNET_MCF_UART_UCR_TX_ENABLED
		| FNET_MCF_UART_UCR_RX_ENABLED);

#endif		

#if FNET_CFG_MCF_SCI /* V1 Lasko */  
    /*
     *   Serial Communication Interface (SCI) Initialization.
     *   (8N1 operation, interrupts disabled, no hardware flow-control)
     */
    
    /* Calculate baud settings
     * SCI baud rate equals BUSCLK/(16*BR) 
     */
    ubgs = (unsigned short)(FNET_CFG_CPU_CLOCK_HZ/(16* baud_rate));
    FNET_MCF_SCI_BDH(port_number) = (unsigned char)((ubgs & 0x1F00) >> 8);
    FNET_MCF_SCI_BDL(port_number) = (unsigned char)(ubgs & 0x00FF);
    
    /* Normal operation:
     * Sart + 8 data bits (lsb first) + Sop. 
     * RxD and TxD use separate pins. 
     */
    FNET_MCF_SCI_C1(port_number) = 0;
    
    /* Enable transmitter and receiver. Disable all interrupts.
     */
    FNET_MCF_SCI_C2(port_number) = (FNET_MCF_SCI_C2_TE_MASK | FNET_MCF_SCI_C2_RE_MASK);
     
    /* Disable all errors interrupts 
     */  
    FNET_MCF_SCI_C3(port_number) = 0;

#endif
		
}

#endif /*FNET_MCF*/
