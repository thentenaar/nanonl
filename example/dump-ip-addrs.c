/**
 * nanonl: dump-ip-addrs: Dump all IP addresses
 * Copyright (C) 2015 - 2017 Tim Hentenaar.
 *
 * Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/rtnetlink.h>
#include "../src/nl.h"
#include "../src/nl_ifaddr.h"

static int fd = -1;
static char buf[BUFSIZ];
static char addrbuf[INET6_ADDRSTRLEN];
static struct nlmsghdr *m = (struct nlmsghdr *)(void *)buf;

int main(void)
{
	__u32 len;
	ssize_t br;
	struct nlmsghdr *e;
	struct ifaddrmsg *ifa;
	struct nlattr *nla;
	const char *label;
	const char *disp;
	__u8 family = AF_INET;

	memset(buf, 0, sizeof(buf));
	if ((fd = nl_open(NETLINK_ROUTE, (__u32)getpid())) < 0) {
		perror("Unable to open netlink socket");
		goto ret;
	}

dump_addrs:
	nl_ifa_get_addr(m, family);
	if ((__u32)nl_send(fd, 0, m) != m->nlmsg_len) {
		fputs("Failed to send address request\n", stderr);
		goto ret;
	}

read:
	if ((br = nl_recv(fd, m, BUFSIZ, NULL)) <= 0) {
		fputs("Failed to read address response\n", stderr);
		goto ret;
	} else len = (__u32)br;

	for (e = m; NLMSG_OK(e, len); e = NLMSG_NEXT(e, len)) {
		if (e->nlmsg_type == NLMSG_DONE) {
			if (family == AF_INET) {
				family = AF_INET6;
				goto dump_addrs;
			} else goto ret;
		} else if (e->nlmsg_type != RTM_NEWADDR) continue;

		/* Lookup the interface name */
		label = NULL;
		ifa = NLMSG_DATA(e);
		if ((nla = nl_get_attr(e, sizeof(*ifa), IFA_LABEL)))
			label = NLA_DATA(nla);

		/* Now, print the address */
		if ((nla = nl_get_attr(e, sizeof(*ifa), IFA_ADDRESS))) {
			printf("%s has %s address: %s\n",
			       label ? label : "<none>",
			       (family == AF_INET) ? "v4": "v6",
			       inet_ntop(ifa->ifa_family, NLA_DATA(nla),
			                 addrbuf, sizeof(addrbuf)));
		}
	}
	goto read;

ret:
	if (fd >= 0) close(fd);
	return EXIT_SUCCESS;
}

