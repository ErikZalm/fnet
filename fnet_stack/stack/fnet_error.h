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
* @file fnet_error.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.11.0
*
* @brief Socket error API definitions.
*
***************************************************************************/
#ifndef _FNET_ERROR_H_

#define _FNET_ERROR_H_

/*! @addtogroup fnet_error
* When a socket call indicates a failure, it is possible to call the @ref
* fnet_error_get() function to determine the error value.@n
* Another way to determine the error value for a socket is to examine its 
* @ref SO_ERROR option.@n
* Possible values for socket errors are defined by the @ref fnet_error_t.
*/
/*! @{ */

/**************************************************************************/ /*!
 * @brief General return codes, used by most of API functions.
 ******************************************************************************/
typedef enum
{
    FNET_OK  = (0), /**< No error.
                     */
    FNET_ERR = (-1) /**< There is error.
                     */
} fnet_return_t;

/**************************************************************************/ /*!
 * @brief Possible socket error codes, returned by the @ref fnet_error_get(),
 * or used by the @ref SO_ERROR option.
 ******************************************************************************/
typedef enum
{
    FNET_ERR_OK             = (0),  /**< @brief There is no error.
                                     */
    FNET_ERR_BAD_DESC       = (1),  /**< @brief Bad socket descriptor.@n
                                     *   An operation was attempted on a socket descriptor
                                     *   that does not refer to a valid socket.
                                     */
    FNET_ERR_NO_DESC        = (2),  /**< @brief No more socket descriptors are available.@n
                                     *   An application has opened too many sockets.
                                     *   The maximum number of available socket descriptors
                                     *   is defined by the @ref FNET_CFG_SOCKET_MAX.
                                     */
    FNET_ERR_NOMEM          = (3),  /**< @brief Cannot allocate the memory.@n
                                     *   The error is indicating a lack of required
                                     *   memory resources.
                                     */
    FNET_ERR_AGAIN          = (4),  /**< @brief Try again, a retry at some time later may be successful.@n
                                     *   This error is returned from operations on sockets
                                     *   that cannot be completed immediately. It is a non-fatal error
                                     *   and the operation should be retried later.@n
                                     *   For example, it is normal for the @ref FNET_ERR_AGAIN to be reported as
                                     *   the result from the @ref connect() calling on a @ref SOCK_STREAM socket,
                                     *   since some time must elapse for the connection to be established.
                                     */
    FNET_ERR_BOUNDREQ       = (5),  /**< @brief The socket has not been bound with @ref bind().@n
                                     *   A socket must be bound to an address before calling
                                     *   @ref getsockname(), @ref recvfrom(), @ref recv(), and @ref listen().
                                     */
    FNET_ERR_INVAL          = (6),  /**< @brief Invalid argument.@n
                                     *   An invalid argument was supplied.
                                     */
    FNET_ERR_DESTADDRREQ    = (7),  /**< @brief Destination address required.@n
                                     *   A required address was omitted from an operation on a socket.
                                     *   For example, this error will be returned, if the @ref sendto()
                                     *   or @ref connect() is called with the remote address of @ref INADDR_ANY.
                                     */
    FNET_ERR_MSGSIZE        = (8),  /**< @brief Message too long.@n
                                     *   A message sent on a datagram socket was larger than the internal
                                     *   message buffer, or some other network limit, or the buffer,
                                     *   which is used to receive a datagram was smaller than the datagram itself.
                                     */
    FNET_ERR_NOPROTOOPT     = (9),  /**< @brief Bad protocol option. @n
                                     *   An unknown, invalid, or unsupported option or level was specified
                                     *   in the @ref getsockopt(), or @ref setsockopt() call, or a socket
                                     *   received Parameter Problem ICMP Error Message.
                                     */
    FNET_ERR_PROTONOSUPPORT = (10), /**< @brief Protocol not supported.@n
                                     *   This error occurs if an application attempts to call @ref socket()
                                     *   and the requested protocol has not been configured into the system,
                                     *   or no implementation for it exists.
                                     */
    FNET_ERR_OPNOTSUPP      = (11), /**< @brief Operation not supported.@n
                                     *   The attempted operation is not supported for the type
                                     *   of socket referenced. This occurs, when socket cannot support
                                     *   the requested operation, for example trying to accept
                                     *   a connection on a datagram socket.
                                     */
    FNET_ERR_AFNOSUPPORT    = (12), /**< @brief Address family not supported by the protocol family.@n
                                     *   The stack supports only the @ref AF_INET family.
                                     *   This error will be returned, if an address of a wrong family
                                     *   is used for a socket in @ref socket(), @ref sendto(), @ref connect(),
                                     *   @ref bind(), @ref getsockname(), and in @ref getpeername().
                                     */
    FNET_ERR_ADDRINUSE      = (14), /**< @brief Address already in use.@n
                                     *   This error occurs, if an application attempts to @ref bind() or
                                     *   @ref connect() a socket to an IP address and port that has
                                     *   already been used for an existing socket, or a socket that
                                     *   wasn't closed properly, or one that is still in the process of closing.
                                     *   Only one usage of each socket address is permitted.
                                     */
    FNET_ERR_ADDRNOTAVAIL   = (15), /**< @brief Cannot assign the requested address.@n
                                     *   The requested address is not valid in its context.
                                     *   It normally results from an attempt to @ref bind() to an address
                                     *   that is not valid for the local machine. This may also result
                                     *   from @ref connect(), when the remote address or port is not
                                     *   valid for a remote machine (for example address or port is 0).
                                     */
    FNET_ERR_NETUNREACH     = (16), /**< @brief The network is unreachable.@n
                                     *   This error occurs, if socket cannot function at this time,
                                     *   because the underlying network interface it uses to provide
                                     *   the network services is unavailable.
                                     */
    FNET_ERR_CONNABORTED    = (17), /**< @brief Software caused the connection abort.@n
                                     *   An established connection was aborted due to a data
                                     *   transmission or connection timeouts.
                                     */
    FNET_ERR_CONNRESET      = (18), /**< @brief Connection reset by peer. @n
                                     *   A connection was forcibly closed by the remote host.
                                     *   This normally results, if the peer application on the remote
                                     *   host has suddenly stopped, the host has rebooted, or the
                                     *   remote host used a "hard close" on the remote socket.@n
                                     *   For the UDP sockets, the remote host was unable to deliver
                                     *   a previously sent UDP datagram and responded with
                                     *   a "Port Unreachable" ICMP packet. The application
                                     *   should close the socket as it is no longer usable.
                                     */
    FNET_ERR_ISCONN         = (19), /**< @brief Socket is already connected.@n
                                     *   A @ref connect() or @ref listen() request was made on
                                     *   an already connected socket.
                                     */
    FNET_ERR_NOTCONN        = (20), /**< @brief Socket is not connected.@n
                                     *   A request to send or receive data was not allowed
                                     *   because the socket is not connected.
                                     */
    FNET_ERR_SHUTDOWN       = (21), /**< @brief The socket has been shut down.@n
                                     *   A request to send or receive data was not allowed because
                                     *   the socket had already been shut down in that direction
                                     *   with a previous @ref shutdown() call.
                                     */
    FNET_ERR_INPROGRESS     = (22), /**< @brief The action is in progress.@n
                                     *   An operation was attempted on a
                                     *   socket that already had an operation in
                                     *   progress - in other words calling @ref connect() a second time on a
                                     *   socket that is already connecting.
                                     */
    FNET_ERR_TIMEDOUT       = (23), /**< @brief The connection has timed out.@n
                                     *   A connection attempt failed because the connected party did
                                     *   not properly respond after a period of time.
                                     */
    FNET_ERR_HOSTUNREACH    = (24), /**< @brief No route to a host.@n
                                     *   A socket operation was attempted to an unreachable host.
                                     */
    FNET_ERR_SYSNOTREADY    = (25), /**< @brief Network subsystem is unavailable.@n
                                     *   The stack is not initialized.
                                     */
    FNET_ERR_CONNCLOSED     = (26), /**< @brief Connection closed by peer. @n
                                     * The final (FIN) segment arrived and there is no data 
                                     * in the socket receive buffer.  
                                     * The remote host closed connection and will 
                                     * not send any data in the current connection. @n
                                     * The application should close the socket as it 
                                     * is no longer usable.
                                     */
    FNET_ERR_IPDISABLED    = (27)   /**< @brief IP operation is disabled. @n
                                     * It happens when Duplicate Address Detection 
                                     * fails for interface link-local address, 
                                     * formed from an interface identifier based on the hardware address.
                                     */
} fnet_error_t;

/***************************************************************************/ /*!
 *
 * @brief Returns the last error that occurred.
 *
 * @return This function returns the error code for the last socket
 *         operation that failed.@n
 *         The error codes are defined by the @ref fnet_error_t.
 *
 * @see fnet_error_set()
 *
 ******************************************************************************
 *
 *   When a particular socket function indicates that an error has occurred,
 *   this function should be called to retrieve the appropriate error code.@n
 *   A successful socket function call, or a call to @ref fnet_error_get(),
 *   does not reset the error code. To reset the error code, use the fnet_error_set()
 *   function call with error set to zero (@ref FNET_ERR_OK).
 *
 ******************************************************************************/
int fnet_error_get( void );

/***************************************************************************/ /*!
 *
 * @brief Sets the error code.
 *
 * @param error  Error code.
 *
 * @see fnet_error_get()
 *
 ******************************************************************************
 *
 *   This function is used to set or reset the error code. @n
 *   Note that any subsequent socket routine called by the application will
 *   override the error code, as set by this routine.
 *
 ******************************************************************************/
void fnet_error_set( int error );


/*! @} */

#endif
