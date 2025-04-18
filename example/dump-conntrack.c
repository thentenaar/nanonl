/**
 * nanonl: dump-ct Dump conntrack entries
 * Copyright (C) 2015 - 2025 Tim Hentenaar.
 *
 * Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <endian.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/rtnetlink.h>

#include "../src/nl.h"
#include "../src/nl_nfct.h"

static int fd = -1;
static char buf[NLMSG_GOODSIZE << 1];
static char addrbuf[INET6_ADDRSTRLEN];
static struct nlattr *attrs[CTA_MAX + 1];
static struct nlattr *tattrs[CTA_TUPLE_MAX + 1];
static struct nlattr *ipattrs[CTA_IP_MAX + 1];
static struct nlattr *pattrs[CTA_PROTO_MAX + 1];
static struct nlattr *o[CTA_COUNTERS_MAX + 1];
static struct nlattr *r[CTA_COUNTERS_MAX + 1];
static struct nlmsghdr *m = (struct nlmsghdr *)(void *)buf;

int main(void)
{
	__u32 pid = 0, mark = 0;
	struct nlmsghdr *e;
	size_t len;
	unsigned short port;

	memset(buf, 0, sizeof buf);
	if ((fd = nl_open(NETLINK_NETFILTER, (__u32)getpid())) < 0) {
		perror("Unable to open netlink socket");
		goto ret;
	}

	nl_nfct_dump(m, 0, 1);
	if ((__u32)nl_send(fd, 0, m) != m->nlmsg_len) {
		fputs("Failed to dump conntrack entries\n", stderr);
		goto ret;
	}

	errno = 0;
	len = sizeof buf;
	memset(m, 0, sizeof *m);
	if (nl_recv(fd, m, len, &pid) <= 0) {
		if (errno < 0) {
			fprintf(stderr, "Got netlink error #%d\n",
				errno);
		} else perror("Failed to read message");
		goto ret;
	}

	/* These may be multi-part */
	for (e = m; NLMSG_OK(e, len); e = NLMSG_NEXT(e, len)) {
		if (e->nlmsg_type == NLMSG_DONE) break;
		if ((e->nlmsg_type & 0xff) != IPCTNL_MSG_CT_NEW)
			continue;

		mark = 0;
		memset(attrs, 0, sizeof attrs);
		memset(tattrs, 0, sizeof tattrs);
		memset(pattrs, 0, sizeof pattrs);
		memset(o, 0, sizeof o);
		memset(r, 0, sizeof r);

		nl_nf_get_attrv(e, attrs);
		nla_get_attrv(attrs[CTA_TUPLE_ORIG], tattrs, CTA_TUPLE_MAX);
		nla_get_attrv(tattrs[CTA_TUPLE_IP], ipattrs, CTA_IP_MAX);
		nla_get_attrv(tattrs[CTA_TUPLE_PROTO], pattrs, CTA_PROTO_MAX);

		if (attrs[CTA_MARK])
			mark = ntohl(*(__u32 *)NLA_DATA(attrs[CTA_MARK]));

		/* Print the source address / port */
		memset(addrbuf, 0, sizeof addrbuf);
		if (ipattrs[CTA_IP_V6_SRC]) {
			inet_ntop(AF_INET6,
			          NLA_DATA(ipattrs[CTA_IP_V6_SRC]),
			          addrbuf, sizeof addrbuf);
		} else if (ipattrs[CTA_IP_V4_SRC]) {
			inet_ntop(AF_INET,
			          NLA_DATA(ipattrs[CTA_IP_V4_SRC]),
			          addrbuf, sizeof addrbuf);
		} else continue;

		port = 0;
		if (pattrs[CTA_PROTO_SRC_PORT]) {
			port = ntohs(*(unsigned short *)NLA_DATA(
			             pattrs[CTA_PROTO_SRC_PORT]));
		}

		if (mark) printf("[mark=0x%08x] ", mark);
		printf("%s (%u) -> ", addrbuf, port);

		/* Print the destination address / port */
		memset(addrbuf, 0, sizeof addrbuf);
		if (ipattrs[CTA_IP_V6_DST]) {
			inet_ntop(AF_INET6,
			          NLA_DATA(ipattrs[CTA_IP_V6_DST]),
			          addrbuf, sizeof addrbuf);
		} else if (ipattrs[CTA_IP_V4_DST]) {
			inet_ntop(AF_INET,
			          NLA_DATA(ipattrs[CTA_IP_V4_DST]),
			          addrbuf, sizeof addrbuf);
		}

		port = 0;
		if (pattrs[CTA_PROTO_DST_PORT]) {
			port = ntohs(*(unsigned short *)NLA_DATA(
			             pattrs[CTA_PROTO_DST_PORT]));
		}
		printf("%s (%u) ", addrbuf, port);

		/* Print the counters if we have them */
		nla_get_attrv(attrs[CTA_COUNTERS_ORIG],  o, CTA_COUNTERS_MAX);
		nla_get_attrv(attrs[CTA_COUNTERS_REPLY], r, CTA_COUNTERS_MAX);
		if (o[CTA_COUNTERS_BYTES]) {
			printf("%" PRIu64 " bytes (orig) ",
			       be64toh(*(uint64_t *)NLA_DATA(o[CTA_COUNTERS_BYTES]))
			);
		}

		if (r[CTA_COUNTERS_BYTES]) {
			printf("%" PRIu64 " bytes (reply) ",
			       be64toh(*(uint64_t *)NLA_DATA(r[CTA_COUNTERS_BYTES]))
			);
		}

		putchar('\n');
	}

ret:
	if (fd >= 0) close(fd);
	return EXIT_SUCCESS;
}

