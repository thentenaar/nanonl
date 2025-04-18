/**
 * \file nl_nd.h
 *
 * nanonl: Netlink Neighbor Discovery Functions
 * Copyright (C) 2015 - 2025 Tim Hentenaar.
 *
 * This code is Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */
#ifndef NL_ND_H
#define NL_ND_H

#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/neighbour.h>

#include "nl.h"

/**
 * \def nl_nd_get_attr(m, t)
 * \param m Netlink message buffer
 * \param t ND attribute type
 *
 * Convenience wrapper around nl_get_attr(). This works only because
 * of the fact that \a struct rtattr and \a struct nlattr should be
 * the same (the kernel uses the same functions to operate on both.)
 */
#define nl_nd_get_attr(m, t) \
	nl_get_attr((m), sizeof(struct ndmsg), (t))

/**
 * \def nl_nd_get_attrv(m, a)
 * \param m Netlink message buffer
 * \param a Array of \a struct nlattr *
 *
 * Convenience wrapper around nl_get_attrv().
 */
#define nl_nd_get_attrv(m, a)\
	nl_get_attrv((m), sizeof(struct ndmsg), (a), \
	             ((sizeof((a)) / sizeof(struct nlattr *)) - 1))

/**
 * \def nl_nd_get_neighbors(m, family)
 * \brief Get a dump of the neighbor table for a given family
 * \param m      Netlink message buffer
 * \param family Address family
 */
#define nl_nd_get_neighbors(m, family)\
	nl_nd_request((m), 0, (family), RTM_GETNEIGH, 0, 0, 0);

/**
 * \def nl_nd_new_neighbor(m, family, ifindex, state, flags)
 * \brief Add or update a neighbor entry
 * \param m       Netlink message buffer
 * \param family  Address family
 * \param ifindex Interface index
 * \param state   Cache entry state (NUD_*)
 * \param flags   Cache entry flags (NTF_*)
 */
#define nl_nd_new_neighbor(m, family, ifindex, state, flags)\
	ml_nd_request((m), 0, (family), RTM_NEWNEIGH, (ifindex), (state),\
	              (flags));

/**
 * \def nl_nd_del_neighbor(m, family, ifindex, state, flags)
 * \brief Delete a neighbor entry
 * \param m       Netlink message buffer
 * \param family  Address family
 * \param ifindex Interface index
 * \param flags   Cache entry flags (NTF_*)
 */
#define nl_nd_delete_neighbor(m, family, ifindex, flags)\
	nl_nd_request((m), 0, (family), RTM_DELNEIGH, (ifindex), 0, (flags));

/**
 * \brief Create a netlink neighbor discovery request.
 * \param[in] m       Netlink message buffer.
 * \param[in] pid     Destination netlink port.
 * \param[in] family  Address family (AF_INET[6]).
 * \param[in] type    Message Type (i.e. RTM_GETNEIGH).
 * \param[in] ifindex Interface index.
 * \param[in] state   Cache entry state (NUD_*).
 * \param[in] flags   Cache entry flags (NTF_*).
 * \relates nl_request
 */
void nl_nd_request(struct nlmsghdr *m, __u32 pid, __u8 family, __u8 type,
                   __s32 ifindex, __u8 state, __u8 flags);

#endif /* NL_ND_H */

