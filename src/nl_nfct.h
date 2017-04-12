/**
 * \file nl_nfct.h
 *
 * nanonl: Netlink Conntrack functions
 * Copyright (C) 2015 - 2017 Tim Hentenaar.
 *
 * This code is Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */
#ifndef NL_NFCT_H
#define NL_NFCT_H

#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

#include "nl_nf.h"

/**
 * \brief Create a netlink_conntrack request.
 * \param[in] m    Netlink message buffer.
 * \param[in] pid  Destination netlink port.
 * \param[in] type message type (IPCTNL_MSG_CT_*).
 * \param[in] l3p  Layer 3 protocol (NFPROTO_*).
 * \relates nl_request
 */
#define nl_nfct_request(m, pid, type, l3p) \
	nl_nf_request((m), (pid), NFNL_SUBSYS_CTNETLINK, (type), (l3p), 0)

/**
 * \brief Get a conntrack entry
 * \param[in] m   Netlink message buffer
 * \param[in] l3p Layer 3 protocol (NFPROTO_*)
 *
 * Note: At least one tuple (or CT_MARK and CT_MARK_MASK) must be added
 * to the message in order for the kernel to lookup the conntrack entry.
 */
#define nl_nfct_get(m, l3p) \
	nl_nfct_request((m), 0, IPCTNL_MSG_CT_GET, (l3p))

/**
 * \brief Flush one/all conntrack entries
 * \param[in] m       Netlink message buffer
 * \param[in] l3proto Layer 3 protocol (NFPROTO_*)
 *
 * Note: At least one tuple (or CT_MARK and CT_MARK_MASK) must be added
 * to the message in order for the kernel to lookup the specific entry
 * to delete.
 *
 * If no other attributes are added, all entries will be flushed.
 */
#define nl_nfct_delete(m, l3p) \
	nl_nfct_request((m), 0, IPCTNL_MSG_CT_DELETE, (l3p))

/**
 * \brief Request a dump of all conntrack entries
 * \param[in] m       Netlink message buffer
 * \param[in] l3proto Layer 3 protocol (NFPROTO_*)
 */
void nl_nfct_dump(struct nlmsghdr *m, __u8 l3proto);

/**
 * \brief Create a new conntrack entry
 * \param[in] m       Netlink message buffer
 * \param[in] l3proto Layer 3 protocol (NFPROTO_*)
 *
 * Note; The attributes required to create the conntrack entry
 * must still be added to the message before sending.
 */
void nl_nfct_create(struct nlmsghdr *m, __u8 l3proto);

/**
 * \brief Update a conntrack entry
 * \param[in] m       Netlink message buffer
 * \param[in] l3proto Layer 3 protocol (NFPROTO_*)
 *
 * Note: At least one tuple must be added to the message in order
 * for the kernel to lookup the conntrack entry to modify.
 */
void nl_nfct_update(struct nlmsghdr *m, __u8 l3proto);

#endif /* NL_NFCT_H */

