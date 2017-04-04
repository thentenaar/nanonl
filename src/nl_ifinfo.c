/**
 * nanonl: Netlink Interface Info Functions
 *
 * Copyright (C) 2015 - 2017 Tim Hentenaar.
 *
 * Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <arpa/inet.h>

#include "nl.h"
#include "nl_ifinfo.h"

static void op_by_name(struct nlmsghdr *m, __u8 op, const char *name)
{
	nl_ifi_request(m, 0, op, AF_UNSPEC, 0, 0, 0);
	nl_add_attr(m, IFLA_IFNAME, name, strlen(name) + 1);
}

/**
 * \brief Get link information (by interface name)
 * \param m    Netlink message buffer
 * \param name Interface name (char *)
 */
void nl_ifi_get_by_name(struct nlmsghdr *m, const char *name)
{
	if (!m || !name) return;
	op_by_name(m, RTM_GETLINK, name);
}

/**
 * \brief Delete an interface (by name)
 * \param m    Netlink message buffer
 * \param name Interface name (char *)
 */
void nl_ifi_del_by_name(struct nlmsghdr *m, const char *name)
{
	if (!m || !name) return;
	op_by_name(m, RTM_DELLINK, name);
}

/**
 * \brief Create a netlink interface link info request
 * \param[in] m       Netlink message buffer
 * \param[in] pid     Destination netlink port
 * \param[in] type    One of: RTM_GETLINK, RTM_NEWLINK, RTM_DELLINK
 * \param[in] family  Address family (AF_INET[6])
 * \param[in] devtype Device type (i.e. ARPHRD_ETHER)
 * \parma[in] ifindex Interface index
 * \param[in] flags   Interface flags (IFF_*)
 * \relates nl_request
 */
void nl_ifi_request(struct nlmsghdr *m, __u32 pid, __u8 type,
                    __u8 family, unsigned short devtype, int ifindex,
                    unsigned int flags)
{
	struct ifinfomsg *ifi = BYTE_OFF(m, sizeof(*m));
	if (!m) return;
	nl_request(m, type, pid, sizeof(*ifi));
	ifi->ifi_family = family;
	ifi->ifi_type   = devtype;
	ifi->ifi_index  = ifindex;
	ifi->ifi_flags  = flags;
	ifi->ifi_change = UINT_MAX;
}

