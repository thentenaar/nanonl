/**
 * nanonl: Netlink Nfqueue Functions
 * Copyright (C) 2015 - 2017 Tim Hentenaar.
 *
 * Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux/version.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

#include "nl.h"
#include "nl_nfqueue.h"

/**
 * \brief Make a nfqueue config command message
 * \param[in] m   Netlink message buffer.
 * \param[in] cmd Netlink nfqueue command.
 * \param[in] pf  Protocol family (i.e. PF_INET).
 * \param[in] qn  Queue number.
 */
void nl_nfqueue_cfg_cmd(struct nlmsghdr *m, __u8 cmd, __u16 pf, __u16 qn)
{
	struct nfqnl_msg_config_cmd c;
	c.command = cmd;
	c.pf = htons(pf);

	nl_nfqueue_request(m, 0, NFQNL_MSG_CONFIG, (__u8)pf, qn);
	nl_add_attr(m, NFQA_CFG_CMD, &c, sizeof(c));
}

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
                     __u8 cmode, __u32 crange, __u32 maxlen, int want_ct)
{
	__u32 fl;
	struct nfqnl_msg_config_params params;
	params.copy_mode  = cmode;
	params.copy_range = htonl(crange);
	nl_nfqueue_cfg_cmd(m, NFQNL_CFG_CMD_BIND, pf, queue_num);
	nl_add_attr(m, NFQA_CFG_PARAMS, &params, sizeof(params));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
	if (want_ct) {
		fl = htonl(NFQA_CFG_F_CONNTRACK);
		nl_add_attr(m, NFQA_CFG_FLAGS, &fl, sizeof(fl));
		nl_add_attr(m, NFQA_CFG_MASK,  &fl, sizeof(fl));
	}
#else
	(void)want_ct;
#endif /* Linux < 3.6.0 */

	if (maxlen) {
		fl = htonl(maxlen);
		nl_add_attr(m, NFQA_CFG_QUEUE_MAXLEN, &fl, sizeof(fl));
	}
}

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
                        __u32 packet_id, __u32 verdict)
{
	struct nfqnl_msg_verdict_hdr v;
	v.verdict = htonl(verdict);
	v.id      = htonl(packet_id);
	nl_nfqueue_request(m, 0, NFQNL_MSG_VERDICT, 0, queue_num);
	nl_add_attr(m, NFQA_VERDICT_HDR, &v, sizeof(v));
}

/**
 * \brief Add a packet mark to a verdict message
 * \param[in] m    Netlink message buffer.
 * \param[in] mark Packet mark to set.
 *
 * Instruct the kernel to add the given mark to the newly-adjudicated
 * packet.
 */
void nl_nfqueue_verdict_mark(struct nlmsghdr *m, __u32 mark)
{
	mark = htonl(mark);
	nl_add_attr(m, NFQA_MARK, &mark, sizeof(__u32));
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
/**
 * \brief Add a connmark to a verdict message
 * \param[in] m    Netlink message buffer.
 * \param[in] mark Connmark to set.
 *
 * Instruct the kernel to add the given mark to the connection with
 * which this packet is associated.
 *
 * This requires Linux >= 3.6 with CONFIG_NF_CONNTRACK_MARK set.
 */
void nl_nfqueue_verdict_ctmark(struct nlmsghdr *m, __u32 mark)
{
	struct nlattr *nla;
	mark = htonl(mark);
	nla = nla_start(m, NFQA_CT);
	nla_add_attr(nla, CTA_MARK, &mark, sizeof(mark));
	nla_end(m, nla);
}
#endif

