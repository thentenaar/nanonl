/**
 * \file nl_nfqueue.h
 *
 * nanonl: Netlink Netfilter Queue functions
 * Copyright (C) 2015 - 2017 Tim Hentenaar.
 *
 * This code is Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */
#ifndef NL_NFQUEUE_H
#define NL_NFQUEUE_H

#include <sys/types.h>
#include <linux/version.h>
#include <linux/netlink.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink_queue.h>

#include "nl_nf.h"

/**
 * \brief Create a netlink_nfqueue request.
 * \param[in] m    Netlink message buffer.
 * \param[in] pid  Destination netlink port.
 * \param[in] type message type (nfqnl_message_types).
 * \param[in] pf   address family for binding.
 * \param[in] qn   queue number (as given to iptables.)
 * \relates nl_request
 */
#define nl_nfqueue_request(m, pid, type, pf, qn) \
	nl_nf_request((m), (pid), NFNL_SUBSYS_QUEUE, (type), (pf), (qn))

/* These calls aren't needed on Linux >= 3.8 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
/**
 * \brief Bind to the given protocol family
 * \param[in] m  Netlink message buffer.
 * \param[in] pf Protocol family (i.e. PF_INET)
 */
#define nl_nfqueue_bind_pf(m, pf) \
	nl_nfqueue_cfg_cmd(NFQNL_CMD_CFG_PF_BIND, (pf))

/**
 * \brief Unbind from the given protocol family
 * \param[in] m  Netlink message buffer.
 * \param[in] pf Protocol family (i.e. PF_INET)
 */
#define nl_nfqueue_unbind_pf(m, pf) \
	nl_nfqueue_cfg_cmd(NFQNL_CMD_CFG_PF_UNBIND, (pf))
#else
#define nl_nfqueue_bind_pf(m, pf)
#define nl_nfqueue_unbind_pf(m, pf)
#endif /* Linux < 3.8.0 */

/**
 * \brief Unbind from a packet queue
 * \param[in] m         Netlink message buffer.
 * \param[in] pf        Protocol family (i.e. PF_INET).
 * \param[in] queue_num queue number (as given to iptables.)
 */
#define nl_nfqueue_unbind(m, pf, queue_num) \
	nl_nfqueue_cfg_cmd((m), NFQNL_CFG_CMD_UNBIND, (pf), (queue_num))

/**
 * \brief Make a nfqueue config command message
 * \param[in] m   Netlink message buffer.
 * \param[in] cmd Netlink nfqueue command.
 * \param[in] pf  Protocol family (i.e. PF_INET).
 * \param[in] qn  Queue number.
 */
void nl_nfqueue_cfg_cmd(struct nlmsghdr *m, __u8 cmd, __u16 pf, __u16 qn);

/**
 * \brief Bind to a packet queue
 * \param[in] m         Netlink message buffer.
 * \param[in] pf        Protocol family (i.e. PF_INET).
 * \param[in] queue_num queue number (as given to iptables.)
 * \param[in] cmode     Metadata only, or the whole packet (NFQNL_COPY_*)
 * \param[in] cange     Amount of packet data to copy (in bytes.)
 * \param[in] maxlen    If non-zero, the maximum queue length (in bytes.)
 * \param[in] want_ct   If non-zero, have the conntrack info sent also.
 *
 * NOTE: Getting the conntrack info requires the \a ip_conntrack_netlink
 * module to be lodaed or compiled in (CONFIG_NF_CT_NETLINK), and is only
 * available in Linux 3.6 or higher.
 */
void nl_nfqueue_bind(struct nlmsghdr *m, __u16 pf, __u16 queue_num,
                     __u8 cmode, __u32 crange, __u32 maxlen, int want_ct);

/**
 * \brief Create a packet verdict message
 * \param[in] m          Netlink message buffer.
 * \param[in] queue_num  queue number (as given to iptables.)
 * \param[in] packet_id  Metadata only, or the whole packet (NFQNL_COPY_*)
 * \param[in] verdict    Verdict for the packet (see: linux/netfilter.h)
 *
 * Usually, you'll want to pass NF_DROP, NF_ACCEPT, or NF_REPEAT as the
 * verdict. The verdict is passed to nf_reinject() in
 * net/netfilter/nf_queue.c
 */
void nl_nfqueue_verdict(struct nlmsghdr *m, __u16 queue_num,
                        __u32 packet_id, __u32 verdict);

/**
 * \brief Add a packet mark to a verdict message
 * \param[in] m    Netlink message buffer.
 * \param[in] mark Packet mark to set.
 *
 * Instruct the kernel to add the given mark to the newly-adjudicated
 * packet.
 */
void nl_nfqueue_verdict_mark(struct nlmsghdr *m, __u32 mark);

/**
 * \brief Add a connmark to a verdict message
 * \param[in] m    Netlink message buffer.
 * \param[in] mark Connmark to set.
 *
 * Instruct the kernel to add the given mark to the connection with
 * which this packet is associated.
 *
 * This requires CONFIG_NF_CONNTRACK_MARK in the kernel.
 */
void nl_nfqueue_verdict_ctmark(struct nlmsghdr *m, __u32 mark);

#endif /* NL_NFQUEUE_H */

