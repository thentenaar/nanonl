/**
 * \file nl_ifaddr.h
 *
 * nanonl: Netlink Interface Address functions
 * Copyright (C) 2015 - 2025 Tim Hentenaar.
 *
 * This code is Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */
#ifndef NL_IFADDR_H
#define NL_IFADDR_H

#include <sys/types.h>
#include <linux/if_addr.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "nl.h"

/**
 * \def nl_ifa_index(m)
 * \param m Pointer to a struct ifaddrmsg
 * \return The interface index (cast to int)
 *
 * Get the interface index from a struct ifaddrmsg.
 */
#define nl_ifa_index(m) \
	((int)((struct ifaddrmsg *)(m))->ifa_index)

/**
 * \def nl_ifa_scope(m)
 * \param m Pointer to a struct ifaddrmsg
 * \return The address scope (RT_SCOPE_*)
 *
 * Get the address scope from a struct ifaddrmsg.
 */
#define nl_ifa_scope(m) \
	((int)((struct ifaddrmsg *)(m))->ifa_scope)

/**
 * \def nl_ifa_get_attr(m, t)
 * \param m Netlink message buffer
 * \param t IFA attribute type
 *
 * Convenience wrapper around nl_get_attr(). This works only because
 * of the fact that \a struct rtattr and \a struct nlattr should be
 * the same (the kernel uses the same functions to operate on both.)
 */
#define nl_ifa_get_attr(m, t) \
	nl_get_attr((m), sizeof(struct ifaddrmsg), (t))

/**
 * \def nl_ifa_get_attrv(m, a)
 * \param m Netlink message buffer
 * \param a Array of \a struct nlattr *
 *
 * Convenience wrapper around nl_get_attrv().
 */
#define nl_ifa_get_attrv(m, a)\
	nl_get_attrv((m), sizeof(struct ifaddrmsg), (a), \
	             ((sizeof((a)) / sizeof(struct nlattr *)) - 1 ))

/**
 * \def nl_ifa_get_addr(m, idx, family, scope)
 * \param m      Netlink message buffer
 * \param idx    Interface index (or 0 for all)
 * \param family Address family
 * \param scope  Address scope (RT_SCOPE_*)
 *
 * Construct a request to get a dump of the addresses for
 * all interfaces for the given address family.
 *
 * This may return a multi-part message with multiple addresses.
 */
#define nl_ifa_get_addr(m, family) \
	nl_ifa_request((m), 0, RTM_GETADDR, (family), 0, 0, 0, 0)

/**
 * \def nl_ifa_new_addr(m, idx, family, scope)
 * \param m      Netlink message buffer
 * \param idx    Interface index
 * \param family Address family
 * \param scope  Address scope (RT_SCOPE_*)
 *
 * Construct a request to add an address to an interface with
 * the given index and scope. The address details must be specified
 * by adding their respective attributes to the message.
 */
#define nl_ifa_new_addr(m, idx, family, scope) \
	nl_ifa_request((m), 0, RTM_NEWADDR, (family), 0, 0, (scope), (idx))

/**
 * \def nl_ifa_del_addr(m, idx, family, scope)
 * \param m      Netlink message buffer
 * \param idx    Interface index
 * \param family Address family
 * \param scope  Address scope (RT_SCOPE_*)
 *
 * Construct a request to remove an address from an interface with
 * the given index and scope. The address details must be specified
 * by adding their respective attributes to the message.
 */
#define nl_ifa_del_addr(m, idx, family, scope) \
	nl_ifa_request((m), 0, RTM_DELADDR, (family), 0, 0, (scope), (idx))

/**
 * \brief Create a netlink interface address request
 * \param[in] m          Netlink message buffer
 * \param[in] pid        Destination netlink port
 * \param[in] type       One of: RTM_GETADDR, RTM_NEWADDR, RTM_DELADDR
 * \param[in] family     Address family (AF_INET[6])
 * \param[in] prefix_len Prefix length
 * \param[in] flags      Request flags (IFA_F_*)
 * \param[in] scope      Address scope (RT_SCOPE_*)
 * \param[in] ifindex    Interface index
 * \relates nl_request
 */
void nl_ifa_request(struct nlmsghdr *m, __u32 pid, __u8 type, __u8 family,
                    __u8 prefix_len, __u8 flags, __u8 scope, int ifindex);

#endif /* NL_IFADDR_H */

