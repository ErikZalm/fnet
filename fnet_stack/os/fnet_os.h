/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
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
* @file fnet_os.h
*
* @author Andrey Butok
*
* @brief FNET OS API.
*
***************************************************************************/

#ifndef _FNET_OS_H_

#define _FNET_OS_H_

#include "fnet_config.h"

#if FNET_CFG_OS_MUTEX
    int fnet_os_mutex_init(void);
    void fnet_os_mutex_lock(void);
    void fnet_os_mutex_unlock(void);
    void fnet_os_mutex_release(void);
#else
    #define fnet_os_mutex_init()        FNET_OK
    #define fnet_os_mutex_lock()        {}
    #define fnet_os_mutex_unlock()      {}
    #define fnet_os_mutex_release()     {}
#endif

#if FNET_CFG_OS_ISR
    void fnet_os_isr(void);
#else
    #define fnet_os_isr(void)           {}
#endif

#if FNET_CFG_OS_EVENT
    int fnet_os_event_init(void);
    void fnet_os_event_wait(void);
    void fnet_os_event_raise(void);
#else
    #define fnet_os_event_init()        FNET_OK
    #define fnet_os_event_wait()        {}
    #define fnet_os_event_raise()       {}
#endif

//#if FNET_CFG_OS_TIMER
    int fnet_os_timer_init(unsigned int period_ms);
    void fnet_os_timer_release(void);
//#endif

#endif /* _FNET_OS_H_ */
