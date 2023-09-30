/**
 * nanonl: Netlink Neighbor Discovery Functions
 *
 * Copyright (C) 2015 - 2017 Tim Hentenaar.
 *
 * Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "nl.h"
#include "nl_nd.h"

/**
 * \brief Create a netlink neighbor discovery request.
 * \param[in] m       Netlink message buffer.
 * \param[in] pid     Destination netlink port.
 * \param[in] family  Address family (AF_INET[6]).
 * \param[in] type    Message type (i.e. RTM_GETNEIGH).
 * \param[in] ifindex Interface index.
 * \param[in] state   Cache entry state (NUD_*).
 * \param[in] flags   Cache entry flags (NTF_*).
 * \relates nl_request
 */
void nl_nd_request(struct nlmsghdr *m, __u32 pid, __u8 family, __u8 type,
                   __s32 ifindex, __u8 state, __u8 flags)
{
	struct ndmsg *nd = BYTE_OFF(m, sizeof(*m));
	if (!m) return;
	nl_request(m, type, pid, sizeof(*nd));
	nd->ndm_family  = family;
	nd->ndm_ifindex = ifindex;
	nd->ndm_state   = state;
	nd->ndm_flags   = flags;

	if (type == RTM_GETNEIGH)
		m->nlmsg_flags |= NLM_F_DUMP;
	if (type == RTM_NEWNEIGH)
		m->nlmsg_flags |= NLM_F_CREATE | NLM_F_REPLACE;
}

