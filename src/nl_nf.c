/**
 * nanonl: Netlink Netfilter Functions
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
#include "nl_nf.h"

/**
 * \brief Create a netlink_netfilter request.
 * \param[in] m       Netlink message buffer.
 * \param[in] pid     Destination netlink port.
 * \param[in] subsys  Netfilter subsystem (NFNL_SUBSYS_*).
 * \param[in] type    subsystem-dependent message type.
 * \param[in] family  address family (AF_*).
 * \param[in] res_id  Netfilter resource ID.
 * \relates nl_request
 */
void nl_nf_request(struct nlmsghdr *m, __u32 pid, __u8 subsys, __u8 type,
                    __u8 family, __u16 res_id)
{
	struct nfgenmsg *nf = BYTE_OFF(m, sizeof(*m));
	if (!m) return;
	nl_request(m, ((subsys << 8) | type) & 0xffff, pid, sizeof(*nf));
	nf->nfgen_family = family;
	nf->version      = NFNETLINK_V0;
	nf->res_id       = htons(res_id);
}

