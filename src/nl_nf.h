/**
 * \file nl_nf.h
 *
 * nanonl: Netlink Netfilter functions
 * Copyright (C) 2015 - 2017 Tim Hentenaar.
 *
 * This code is Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */
#ifndef NL_NF_H
#define NL_NF_H

#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/netfilter/nfnetlink.h>

#include "nl.h"

/**
 * \def nl_nf_get_attr(m, t)
 * \param m Netlink message buffer
 * \param t NLA type
 *
 * Convenience wrapper around nl_get_attr().
 */
#define nl_nf_get_attr(m, t) \
	nl_get_attr((m), sizeof(struct nfgenmsg), (t))

/**
 * \def nl_nf_get_attrv(m, a)
 * \param m Netlink message buffer
 * \param a Array of \a struct nlattr *
 *
 * Convenience wrapper around nl_get_attrv().
 */
#define nl_nf_get_attrv(m, a)\
	nl_get_attrv((m), sizeof(struct nfgenmsg), (a), \
	             ((sizeof((a)) / sizeof(struct nlattr *)) - 1))

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
                    __u8 family, __u16 res_id);

#endif /* NL_NF_H */

