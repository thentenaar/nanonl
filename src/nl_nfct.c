/**
 * nanonl: Netlink Conntrack Functions
 * Copyright (C) 2015 - 2017 Tim Hentenaar.
 *
 * Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "nl.h"
#include "nl_nfct.h"

/**
 * \brief Request a dump of all conntrack entries
 * \param[in] m       Netlink message buffer
 * \param[in] l3proto Layer 3 protocol (NFPROTO_*)
 */
void nl_nfct_dump(struct nlmsghdr *m, __u8 l3proto)
{
	nl_nfct_get(m, l3proto);
	m->nlmsg_flags |= NLM_F_DUMP;
}

/**
 * \brief Create a new conntrack entry
 * \param[in] m       Netlink message buffer
 * \param[in] l3proto Layer 3 protocol (NFPROTO_*)
 *
 * Note; The attributes required to create the conntrack entry
 * must still be added to the message before sending.
 */
void nl_nfct_create(struct nlmsghdr *m, __u8 l3proto)
{
	nl_nfct_request(m, 0, IPCTNL_MSG_CT_NEW, l3proto);
	m->nlmsg_flags |= NLM_F_CREATE;
}

/**
 * \brief Update a conntrack entry
 * \param[in] m       Netlink message buffer
 * \param[in] l3proto Layer 3 protocol (NFPROTO_*)
 *
 * Note: At least one tuple must be added to the message in order
 * for the kernel to lookup the conntrack entry to modify.
 */
void nl_nfct_update(struct nlmsghdr *m, __u8 l3proto)
{
	nl_nfct_create(m, l3proto);
	m->nlmsg_flags = (__u16)(m->nlmsg_flags & ~NLM_F_CREATE);
}

