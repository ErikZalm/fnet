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
* @file fnet_timer.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.15.0
*
* @brief FNET Timer API.
*
***************************************************************************/

#ifndef _FNET_TIMER_H

#define _FNET_TIMER_H

/*! @addtogroup fnet_timer
* An application can use the @ref fnet_timer_ticks() function to get a number of 
* ticks (periods, defined by the @ref FNET_TIMER_PERIOD_MS) since the
* hardware timer startup.
*/

/*! @{ */

/**************************************************************************/ /*!
 * @brief Timer period in milliseconds (period of one timer tick).
 ******************************************************************************/
#define FNET_TIMER_PERIOD_MS        (100) /* Do not change it.*/

/**************************************************************************/ /*!
 * @brief Number of timer ticks in one hour.
 ******************************************************************************/
#define FNET_TIMER_TICK_IN_HOUR     ((1000*60*60)/FNET_TIMER_PERIOD_MS)

/**************************************************************************/ /*!
 * @brief Number of timer ticks in one minute.
 ******************************************************************************/
#define FNET_TIMER_TICK_IN_MIN      ((1000*60)/FNET_TIMER_PERIOD_MS)

/**************************************************************************/ /*!
 * @brief Number of timer ticks in one second.
 ******************************************************************************/
#define FNET_TIMER_TICK_IN_SEC      (1000/FNET_TIMER_PERIOD_MS)

/***************************************************************************/ /*!
 *
 * @brief    Gets the timer counter value in ticks.
 *
 *
 * @return   This function returns a current value of the timer counter in ticks.
 *
 * @see fnet_timer_seconds()
 *
 ******************************************************************************
 *
 * This function returns a current value of the timer counter that 
 * contains a number of periods from the moment of the hardware 
 * timer initialization (it's done in the FNET stack initialization).@n
 * The period of one timer tick is defined by the @ref FNET_TIMER_PERIOD_MS.
 *
 ******************************************************************************/
unsigned long fnet_timer_ticks( void );

/***************************************************************************/ /*!
 *
 * @brief    Gets the timer counter value in seconds.
 *
 *
 * @return   This function returns a current value of the timer counter 
 *           in seconds.
 *
 * @see fnet_timer_ticks()
 *
 ******************************************************************************
 *
 * This function returns a current value of the timer counter in seconds, 
 * from the moment of the hardware timer initialization 
 * (it's done in the FNET stack initialization).
 *
 ******************************************************************************/
unsigned long fnet_timer_seconds( void );

/***************************************************************************/ /*!
 *
 * @brief    Gets the timer counter value in milliseconds.
 *
 *
 * @return   This function returns a current value of the timer counter 
 *           in milliseconds.
 *
 * @see fnet_timer_ms()
 *
 ******************************************************************************
 *
 * This function returns a current value of the timer counter in milliseconds, 
 * from the moment of the hardware timer initialization 
 * (it's done in the FNET stack initialization).@n
 *
 ******************************************************************************/
unsigned long fnet_timer_ms( void );

/***************************************************************************/ /*!
 *
 * @brief    Converts milliseconds to timer ticks.
 *
 * @param    time_ms Time value in milliseconds.
 *
 * @return   This function returns the time value in timer ticks.
 *
 ******************************************************************************
 *
 * This function converts the time value @c time_ms in milliseconds to the time
 * value in timer ticks.@n
 * The period of one timer tick is defined by the @ref FNET_TIMER_PERIOD_MS.
 *
 ******************************************************************************/
unsigned long fnet_timer_ms2ticks( unsigned long time_ms );

/***************************************************************************/ /*!
 *
 * @brief    Calculates an interval between two moments in time.
 *
 * @param    start Start time in ticks.
 *
 * @param    end   End time in ticks.
 *
 * @return   This function returns an interval value between two time moments
 *           (in timer ticks).
 *
 ******************************************************************************
 *
 * This function calculates an interval between two moments in time, @c start 
 * and @c end. 
 * This function takes into account also a possible counter overrun @c (start>end).
 *
 ******************************************************************************/
unsigned long fnet_timer_get_interval( unsigned long start, unsigned long end );

/***************************************************************************/ /*!
 *
 * @brief    Performs a delay for the given number of timer ticks.
 *
 * @param    delay_ticks Time value used for delay, in ticks.
 *
 ******************************************************************************
 *
 * This function performs a delay for a given number of timer ticks. 
 * The function is blocked, till the @c delay_ticks expires. 
 *
 ******************************************************************************/
void fnet_timer_delay( unsigned long delay_ticks );

/*! @} */

#if FNET_CFG_OS_TIMER
    #define FNET_HW_TIMER_INIT    fnet_os_timer_init
    #define FNET_HW_TIMER_RELEASE fnet_os_timer_release
#else /* By default */
    #define FNET_HW_TIMER_INIT    fnet_cpu_timer_init
    #define FNET_HW_TIMER_RELEASE fnet_cpu_timer_release
#endif /* FNET_CFG_OS_TIMER */


#endif /* _FNET_TIMER_H */
