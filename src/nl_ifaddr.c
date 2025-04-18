/**
 * nanonl: Netlink Interface Address Functions
 * Copyright (C) 2015 - 2025 Tim Hentenaar.
 *
 * Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <arpa/inet.h>

#include "nl.h"
#include "nl_ifaddr.h"

/**
 * \brief Create a netlink interface address request.
 * \param[in] m          Netlink message buffer.
 * \param[in] pid        Destination netlink port.
 * \param[in] type       One of: RTM_GETADDR, RTM_NEWADDR, RTM_DELADDR
 * \param[in] family     Address family (AF_INET[6]).
 * \param[in] prefix_len Prefix length.
 * \param[in] flags      Request flags (IFA_F_*).
 * \param[in] scope      Address scope (RT_SCOPE_*).
 * \param[in] ifindex    Interface index.
 * \relates nl_request
 */
void nl_ifa_request(struct nlmsghdr *m, __u32 pid, __u8 type, __u8 family,
                    __u8 prefix_len, __u8 flags, __u8 scope, int ifindex)
{
	struct ifaddrmsg *ifa = BYTE_OFF(m, sizeof *m);
	if (!m) return;
	nl_request(m, type, pid, sizeof *ifa);
	ifa->ifa_family    = family;
	ifa->ifa_prefixlen = prefix_len;
	ifa->ifa_flags     = flags;
	ifa->ifa_scope     = scope;
	ifa->ifa_index     = (__u32)ifindex;
	if (type == RTM_GETADDR) m->nlmsg_flags |= NLM_F_DUMP;
}

