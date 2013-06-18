/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2005-2011 by Andrey Butok. Freescale Semiconductor, Inc.
* Copyright 2003 by Alexey Shervashidze. Motorola SPS
*
***************************************************************************
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License Version 3 
* or later (the "LGPL").
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
* @file fnet_tcp.h
*
* @date Dec-19-2012
*
* @author Alexey Shervashidze, Andrey Butok.
*
* @version 0.1.27.0
*
* @brief Private. TCP protocol definitions.
*
***************************************************************************/

#ifndef _FNET_TCP_H_

#define _FNET_TCP_H_

#include "fnet_netbuf.h"
#include "fnet_netif.h"
#include "fnet_netif_prv.h"
//#include "fnet_prot.h"

/************************************************************************
*    Headers of options
*************************************************************************/

#define FNET_TCP_MSS_HEADER         (0x02040000) /* MSS option*/ 
#define FNET_TCP_WINDOW_HEADER      (0x30300)    /* Window scale option*/

/************************************************************************
*    Protocol structure
*************************************************************************/
extern struct fnet_prot_if fnet_tcp_prot_if;

/************************************************************************
*    Size of urgent data
*************************************************************************/
#define FNET_TCP_URGENT_DATA_SIZE   (1)

/************************************************************************
*    Not used value
*************************************************************************/
#define FNET_TCP_NOT_USED           (-1)

/************************************************************************
*    Control values for timers
*************************************************************************/
#define FNET_TCP_TIMER_OFF          (-1)    /* Switch off value.*/
#define FNET_TCP_TIMER_ON_INCREMENT (0)     /* Switch on value for increment timers.*/

/************************************************************************
*    Step of Initial sequence number (ISN)
*************************************************************************/
#define FNET_TCP_STEPISN            (64000)

/************************************************************************
*    Defaults values
*************************************************************************/
#define FNET_TCP_DEFAULT_MSS    (536)                      /* Default value of MSS.*/
#define FNET_TCP_TX_BUF_MAX     (FNET_CFG_SOCKET_TCP_TX_BUF_SIZE) /* Default maximum size for TCP send socket buffer.*/
#define FNET_TCP_RX_BUF_MAX     (FNET_CFG_SOCKET_TCP_RX_BUF_SIZE) /* Default maximum size for TCP receive socket buffer.*/

/************************************************************************
*    Periods of timers
*************************************************************************/
#define FNET_TCP_SLOWTIMO       (500)       /* Slow timer period (ms).*/
#define FNET_TCP_FASTTIMO       (200)       /* Fast timer period (ms).*/

/************************************************************************
*    Keepalive timer parameters                                          
*************************************************************************/
#define FNET_TCP_KEEPIDLE_DEFAULT   (14400) /* x FNET_TCP_SLOWTIMO  
                                             * Standart value for keepalive timer (2 hours).
                                             */
#define FNET_TCP_KEEPINTVL_DEFAULT  (150)   /* x FNET_TCP_SLOWTIMO  
                                             * Standart value for retransmission of the keepalive segment (75 sec).
                                             */
#define FNET_TCP_KEEPCNT_DEFAULT    (8)     /* Number of keepalive segments in state of retransmission.
                                             */


/************************************************************************
*    Initial value for the retransmission and persist timers (6 sec)     
*************************************************************************/
#define FNET_TCP_TIMERS_INIT    (12)

/************************************************************************
*    Limit of timers (60 sec)                                            
*************************************************************************/
#define FNET_TCP_TIMERS_LIMIT   (120)

/************************************************************************
*    Shifts of the retransmission variables                              
*************************************************************************/
#define FNET_TCP_RTT_SHIFT      (3) /* Smoothed round trip time shift.*/
#define FNET_TCP_RTTVAR_SHIFT   (2) /* Round trip time variance shift.*/

/************************************************************************
*    Maximal size of synchronized options
*************************************************************************/
#define FNET_TCP_MAX_OPT_SIZE       (7)

/************************************************************************
*    Maximal window size 
*************************************************************************/
#define FNET_TCP_MAXWIN             (4*1024)/*(0xffff)*/

/************************************************************************
*    Maximal value of the sequence number
*************************************************************************/
#define FNET_TCP_MAX_SEQ            (0xffffffff)

/************************************************************************
*    Maximal urgent pointer 
*************************************************************************/
#define FNET_TCP_MAX_URG_POINTER    (0xffff)

/************************************************************************
*    Maximal size of the receive buffer
*************************************************************************/
#define FNET_TCP_MAX_BUFFER         (0xfff0000)

/************************************************************************
*    Maximal window scale value
*************************************************************************/
#define FNET_TCP_MAX_WINSHIFT       (12)

/************************************************************************
*    Abort interval for the data retransmission and the connection termination
*************************************************************************/
#define FNET_TCP_ABORT_INTERVAL     (240/5) /* 240 x FNET_TCP_SLOWTIMO = 2 minutes/5 */

/************************************************************************
*    Abort interval for the connection establishment                      
*************************************************************************/
#define FNET_TCP_ABORT_INTERVAL_CON (150/5) /* 150 x FNET_TCP_SLOWTIMO = 75 sec/5 */

/************************************************************************
*    Number of repeated acknowledgments for the fast retransmission
*************************************************************************/
#define FNET_TCP_NUMBER_FOR_FAST_RET    (3)

/************************************************************************
*    Timewait delay                                       
*************************************************************************/
#define FNET_TCP_TIME_WAIT              (240/5) /* 240 x FNET_TCP_SLOWTIMO  = 2 minutes/5 */


/************************************************************************
*    Receiving of a byte, word and double word
*************************************************************************/

#define FNET_TCP_GETUCHAR(addr, offset)    (*(unsigned char*)((unsigned long)(addr)+offset))
#define FNET_TCP_GETUSHORT(addr, offset)   (*(unsigned short*)((unsigned long)(addr)+offset))
#define FNET_TCP_GETULONG(addr, offset)    (*(unsigned long*)((unsigned long)(addr)+offset))

/************************************************************************
*    Comparison of sequence numbers
*************************************************************************/

#define FNET_TCP_COMP_GE(a,b)   fnet_tcp_hit(b, b + 0x20000000, a)   /* Greater or equal.*/
#define FNET_TCP_COMP_G(a,b)    fnet_tcp_hit(b+1, b + 0x20000000, a) /* Greater.*/

/************************************************************************
*    Segment header fields
*************************************************************************/
//TBD eliminate these definitions => use structure header. 
#define FNET_TCP_DPORT(segment)        FNET_TCP_GETUSHORT(segment->data_ptr, 2)
#define FNET_TCP_SPORT(segment)        FNET_TCP_GETUSHORT(segment->data_ptr, 0)
#define FNET_TCP_WND(segment)          FNET_TCP_GETUSHORT(segment->data_ptr, 14)
#define FNET_TCP_FLAGS(segment)        FNET_TCP_GETUCHAR(segment->data_ptr, 13)&0x3f
#define FNET_TCP_SEQ(segment)          FNET_TCP_GETULONG(segment->data_ptr, 4)
#define FNET_TCP_ACK(segment)          FNET_TCP_GETULONG(segment->data_ptr, 8)
#define FNET_TCP_LENGTH(segment)       (((FNET_TCP_GETUCHAR(segment->data_ptr, 12)&0xf0)>>4)*4)
#define FNET_TCP_URG(segment)          FNET_TCP_GETUSHORT(segment->data_ptr, 18)
#define FNET_TCP_CHECKSUM(segment)     FNET_TCP_GETUSHORT(segment->data_ptr, 16)
#define FNET_TCP_SET_LENGTH(segment)   FNET_TCP_GETUCHAR(segment->data_ptr, 12)
#define FNET_TCP_SET_FLAGS(segment)    FNET_TCP_GETUCHAR(segment->data_ptr, 13)

/************************************************************************
*    Types of segments
*************************************************************************/
#define FNET_TCP_SGT_FIN            (0x01)
#define FNET_TCP_SGT_SYN            (0x02)
#define FNET_TCP_SGT_RST            (0x04)
#define FNET_TCP_SGT_PSH            (0x08)
#define FNET_TCP_SGT_ACK            (0x10)
#define FNET_TCP_SGT_URG            (0x20)

/************************************************************************
*    TCP options
*************************************************************************/
#define FNET_TCP_OTYPES_END         (0) /* End of Options.*/
#define FNET_TCP_OTYPES_NOP         (1) /* No Option.*/
#define FNET_TCP_OTYPES_MSS         (2) /* Maximal segment size.*/
#define FNET_TCP_OTYPES_WINDOW      (3) /* Scale window.*/

#define FNET_TCP_MSS_SIZE           (4) /* MSS option size.*/
#define FNET_TCP_WINDOW_SIZE        (3) /* Window scale option size.*/

/**************************************************************************/ /*!
 * @internal
 * @brief    TCP options structure.
 ******************************************************************************/
typedef struct
{
    int flags;              /* Flags*/
    unsigned short mss;     /* TCP_MSS option. Maximal segment size*/
    int keep_idle;          /* TCP_KEEPIDLE option. */
    int keep_intvl;         /* TCP_KEEPINTVL option. */
    int keep_cnt;           /* TCP_KEEPCNT option. */

} fnet_tcp_sockopt_t;

/************************************************************************
*    TCP header structure. //TBD use it instead of definitions.
*************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    unsigned short  source_port     FNET_COMP_PACKED;       /* Source port number.*/
    unsigned short  destination_port    FNET_COMP_PACKED;   /* Destination port number.*/
    unsigned long   sequence_number     FNET_COMP_PACKED;   /* Sequence Number.*/
    unsigned long   ack_number  FNET_COMP_PACKED;           /* Sequence Number.*/
    unsigned short  hdrlength__flags FNET_COMP_PACKED;      /* (4 bits) Number of 32 bit words in the TCP Header. (6 bits) Reserved. (6bits) Flags.*/
    unsigned short  window  FNET_COMP_PACKED;               /* Window.*/
    unsigned short  checksum    FNET_COMP_PACKED;           /* Checksum.*/
    unsigned short  urgent_ptr  FNET_COMP_PACKED;           /* Urgent pointer.*/
} fnet_tcp_header_t;
FNET_COMP_PACKED_END

#define FNET_TCP_HEADER_GET_HDRLENGTH(x)    (fnet_ntohs(x->hdrlength__flags)>>12)
#define FNET_TCP_HEADER_GET_FLAGS(x)        (fnet_ntohs(x->hdrlength__flags)&0x3F)

/************************************************************************
*    TCP header size without options
*************************************************************************/
#define FNET_TCP_SIZE_HEADER        (20)

/************************************************************************
*    Maximal optios size
*************************************************************************/
#define FNET_TCP_SIZE_OPTIONS       (40)

/************************************************************************
*    Default TTL 
*************************************************************************/
#define FNET_TCP_TTL_DEFAULT        (64)

/************************************************************************
*    Acknowledgement parameters
*************************************************************************/
#define FNET_TCP_AP_NO_SENDING          (1) /* Ackonwledgment is already sent.*/
#define FNET_TCP_AP_SEND_IMMEDIATELLY   (2) /* Ackonwledgment must be sent immediatelly.*/
#define FNET_TCP_AP_SEND_WITH_DELAY     (4) /* Ackonwledgment can be sent with delay.*/
#define FNET_TCP_AP_FIN_ACK             (8) /* Acknowledgment of the final segment.*/

/************************************************************************
*    Flags of control block
*************************************************************************/
#define FNET_TCP_CBF_CLOSE          (0x01)  /* Socket must be closed and deleted.*/
#define FNET_TCP_CBF_RCVURGENT      (0x02)  /* Urgent byte is received.*/
#define FNET_TCP_CBF_FIN_SENT       (0x04)  /* Final segment is sent.*/
#define FNET_TCP_CBF_FIN_RCVD       (0x08)  /* Final segment is received.*/
#define FNET_TCP_CBF_FORCE_SEND     (0x10)  /* Data must be sent in any case.*/
#define FNET_TCP_CBF_RCVD_SCALE     (0x20)  /* Another side uses the scale option.*/
#define FNET_TCP_CBF_SEND_TIMEOUT   (0x40)  /* Silly window avoidance flag.*/
#define FNET_TCP_CBF_INSND          (0x80)  /* The fnet_tcp_snd function is executed now.*/

/************************************************************************
*    Standart states for TCP ( described in RFC793)
*************************************************************************/
typedef enum
{
    FNET_TCP_CS_NO_STATE = 0,
    FNET_TCP_CS_SYN_SENT = 1,
    FNET_TCP_CS_SYN_RCVD = 2,
    FNET_TCP_CS_LISTENING = 3,
    FNET_TCP_CS_ESTABLISHED = 4,
    FNET_TCP_CS_FIN_WAIT_1 = 5,
    FNET_TCP_CS_FIN_WAIT_2 = 6,
    FNET_TCP_CS_CLOSE_WAIT = 7,
    FNET_TCP_CS_CLOSING    = 8,
    FNET_TCP_CS_LAST_ACK   = 9,
    FNET_TCP_CS_TIME_WAIT  = 10
} fnet_tcp_connection_state_t;

/************************************************************************
*     Timing  states
*************************************************************************/
typedef enum
{
    TCP_TS_ACK_RECEIVED = 0,    /* Acknowledgment of the timing segments is received.*/
    TCP_TS_SEGMENT_SENT = 1,    /* Timing segments are sent.*/
    TCP_TS_SEGMENT_LOST = 2    /* Timing segments are lost.*/
} fnet_tcp_timing_state_t;


/************************************************************************
*    TCP timers structure
*************************************************************************/
typedef struct
{
    int abort;              /* Main timer (used for timing of the abort interval)*/
    int keepalive;          /* Keepalive timer. 
                             * It detects when the other end on an otherwise idle connection crashes or reboots.*/
    int connection;         /* Connection timer.*/
    int retransmission;     /* Retransmission timer. 
                             * It is used when expecting an acknowledgment from the other end. */
    int persist;            /* Persist timer. 
                             * It keeps window size information flowing even if the other end closes its receive window.*/
    int delayed_ack;        /* Delayed acknowledgment timer.*/
    int round_trip;         /* Round trip timer.*/
} fnet_tcp_timers_t;


/************************************************************************
*    Control block structure
*************************************************************************/
typedef struct
{
    /* Receive variables.*/
#if !FNET_CFG_TCP_DISCARD_OUT_OF_ORDER    
    fnet_netbuf_t *tcpcb_rcvchain;      /* Temporary buffer.*/
    unsigned long tcpcb_count;          /* Size of data in the temporary buffer.*/
#endif    
    unsigned long tcpcb_rcvcountmax;    /* Size of the input and temporary buffers.*/
    
    unsigned long tcpcb_rcvack;         /* Highest acknowledged number of sent segments.*/
    unsigned long tcpcb_maxrcvack;      /* Maximal acknowledgment.*/
    unsigned long tcpcb_timingack;      /* Acknowledgment number for timing.*/
    unsigned long tcpcb_rcvwnd;         /* Window.*/
    unsigned short tcpcb_rcvmss;        /* Maximal RX segment size (MSS).*/
    unsigned char tcpcb_recvscale;      /* Scale of the window.*/
#if FNET_CFG_TCP_URGENT    
    unsigned long tcpcb_rcvurgseq;      /* Sequence number of the urgent byte.*/
    long tcpcb_rcvurgmark;              /* Number of bytes before the urgent character (-1 if urgent character is not present)*/
    char tcpcb_iobc;                    /* Input character of the urgent data.*/
#endif /* FNET_CFG_TCP_URGENT */     

    /* Send variables.*/
    unsigned long tcpcb_sndseq;         /* Current sequence number.*/
    unsigned long tcpcb_sndack;         /* Acknowledgment number.*/
    unsigned long tcpcb_sndwnd;         /* Window.*/
    unsigned long tcpcb_maxwnd;         /* Maximal window of another side.*/
    unsigned long tcpcb_cwnd;           /* Congestion window.*/
    unsigned long tcpcb_pcount;         /* Counter of the tcpcb_cwnd parts.*/
    unsigned long tcpcb_ssthresh;       /* Slow start threshold.*/
    unsigned short tcpcb_sndmss;        /* Maximal segment size (MSS).*/    
    unsigned char tcpcb_sendscale;      /* Scale of the window.*/
#if FNET_CFG_TCP_URGENT     
    unsigned long tcpcb_sndurgseq;      /* Sequence number of the urgent byte.*/
#endif /* FNET_CFG_TCP_URGENT */     

    /* Input buffer variables.*/
    unsigned long tcpcb_newfreercvsize; /* Free size of the input buffer.*/

    /* Retransmission variables.*/
    int tcpcb_fastretrcounter;          /* Repeated acknowledgment counter (for fast retransmission).*/
    int tcpcb_rto;                      /* Retransmission timeout.*/
    int tcpcb_crto;                     /* Current retransmission timeout.*/
    int tcpcb_cprto;                    /* Current retransmission timeout for persist timer.*/
    unsigned long tcpcb_retrseq;        /* Sequenc number of the retransmitting data.*/
    unsigned short tcpcb_srtt;          /* Smoothed round trip time.*/
    unsigned short tcpcb_rttvar;        /* Round trip time variance.*/
    fnet_tcp_timing_state_t tcpcb_timing_state;   /* Timing state, defined by fnet_tcp_timing_state_t.*/

    /* Timers.*/
    fnet_tcp_timers_t tcpcb_timers;     /* Structure of the timers.*/

    fnet_tcp_connection_state_t tcpcb_connection_state;         /* Connection state, defined by fnet_tcp_connection_state_t.*/
    fnet_tcp_connection_state_t tcpcb_prev_connection_state;    /* Previous connection state (used only for simultaneous open),
                                         * defined by fnet_tcp_connection_state_t.*/
    /* Flags.*/
    unsigned long tcpcb_flags; 
} fnet_tcp_control_t;


/*************************************************************************/


#endif
