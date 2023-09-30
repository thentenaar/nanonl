/**
 * \file nl_ifinfo.h
 *
 * nanonl: Netlink Interface Info functions
 * Copyright (C) 2015 - 2017 Tim Hentenaar.
 *
 * This code is Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */
#ifndef N_IFINFO_H
#define N_IFINFO_H

#include <sys/types.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_link.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "nl.h"

/**
 * \def nl_ifi_index(m)
 * \param m Pointer to a struct ifinfomsg
 * \return The interface index (int)
 *
 * Get the interface index from a struct ifinfomsg.
 */
#define nl_ifi_index(m) \
	((int)((struct ifinfomsg *)(m))->ifi_index)

/**
 * \def nl_ifi_get_attr(m, t)
 * \param m Netlink message buffer
 * \param t IFLA attribute type
 *
 * Convenience wrapper around nl_get_attr(). This works only because
 * of the fact that \a struct rtattr and \a struct nlattr should be
 * the same (the kernel uses the same functions to operate on both.)
 */
#define nl_ifi_get_attr(m, t) \
	nl_get_attr((m), sizeof(struct ifinfomsg), (t))

/**
 * \def nl_ifi_get_attrv(m, a)
 * \param m Netlink message buffer
 * \param a Array of \a struct nlattr *
 *
 * Convenience wrapper around nl_get_attrv().
 */
#define nl_ifi_get_attrv(m, a)\
	nl_get_attrv((m), sizeof(struct ifinfomsg), (a), \
	             ((sizeof((a)) / sizeof(struct nlattr *)) - 1))

/**
 * \def nl_ifi_get(m, idx, family, scope)
 * \param m      Netlink message buffer
 * \param family Address family (AF_UNSPEC for all)
 * \param idx    Interface index (or 0)
 *
 * Construct a request to get the link information for the interface with
 * the given index.
 *
 * This may return a multi-part message.
 */
#define nl_ifi_get(m, family, type, idx) \
	nl_ifi_request((m), 0, RTM_GETLINK, (family), 0, (idx), 0)

/**
 * \def nl_ifi_new_link(m, idx, family, scope)
 * \param m      Netlink message buffer
 * \param type   Device type (ARPHRD_*)
 * \param idx    Interface index
 * \param family Address family
 * \param flags  Interface flags (IFF_*)
 *
 * Construct a request to create / modify an interface with the
 * given (or desired) index. The interface details must be specified
 * by adding their respective attributes to the message.
 */
#define nl_ifi_new_link(m, type, idx, family, flags) \
	nl_ifi_request((m), 0, RTM_NEWLINK, (family), (type), (idx),\
	                  (flags))

/**
 * \def nl_ifa_del_link(m, idx, family, scope)
 * \param m      Netlink message buffer
 * \param idx    Interface index
 * \param family Address family
 *
 * Construct a request to remove the interface with the given index.
 */
#define nl_ifi_del_link(m, idx) \
	nl_ifi_request((m), 0, RTM_DELLINK, 0, 0, (idx), 0)

/**
 * \brief Get link information (by interface name)
 * \param m    Netlink message buffer
 * \param name Interface name (char *)
 */
void nl_ifi_get_by_name(struct nlmsghdr *m, const char *name);

/**
 * \brief Delete an interface (by name)
 * \param m    Netlink message buffer
 * \param name Interface name (char *)
 */
void nl_ifi_del_by_name(struct nlmsghdr *m, const char *name);

/**
 * \brief Create a netlink interface link info request.
 * \param[in] m       Netlink message buffer
 * \param[in] pid     Destination netlink port
 * \param[in] type    One of: RTM_GETLINK, RTM_NEWLINK, RTM_DELLINK
 * \param[in] family  Address family (AF_INET[6])
 * \param[in] devtype Device type (i.e. ARPHRD_ETHER)
 * \param[in] ifindex Interface index
 * \param[in] flags   Interface flags (IFF_*)
 * \relates nl_request
 */
void nl_ifi_request(struct nlmsghdr *m, __u32 pid, __u8 type,
                    __u8 family, unsigned short devtype, int ifindex,
                    unsigned int flags);

#endif /* NL_IFINFO_H */

