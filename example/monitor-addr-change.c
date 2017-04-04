/**
 * nanonl: monitor-addr-change: Simple multicast example.
 *
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

static int fd = -1;
static char buf[8192];
static char addrbuf[INET6_ADDRSTRLEN];
static struct nlmsghdr *m = (struct nlmsghdr *)(void *)buf;

int main(void)
{
	__u32 pid;
	struct nlmsghdr *e;
	struct ifaddrmsg *ifa;
	struct nlattr *nla;
	const char *label;
	const char *disp;
	size_t len = sizeof(buf);

	memset(buf, 0, sizeof(buf));
	if ((fd = nl_open(NETLINK_ROUTE, (__u32)getpid())) < 0) {
		perror("Unable to open netlink socket");
		goto ret;
	}

	if (nl_multicast(fd, 1, RTNLGRP_IPV4_IFADDR, RTNLGRP_IPV6_IFADDR, 0)) {
		perror("Unable to join multicast groups");
		goto ret;
	}

	do {
		errno = 0;
		len = sizeof(buf);
		memset(m, 0, sizeof(*m));

		puts("Waiting for events...");
		if (nl_recv(fd, m, len, &pid) <= 0) {
			if (errno < 0) {
				fprintf(stderr, "Got netlink error #%d\n",
				        errno);
			} else perror("Failed to read message");
			break;
		}

		/* These may be multi-part */
		for (e = m; NLMSG_OK(e, len); e = NLMSG_NEXT(e, len)) {
			if (e->nlmsg_type == NLMSG_DONE) break;

			/* Determine the address disposition */
			if (e->nlmsg_type == RTM_NEWADDR) disp = "acquired";
			else if (e->nlmsg_type == RTM_DELADDR) disp = "lost";
			else continue;

			/* Lookup the interface name */
			label = NULL;
			ifa = NLMSG_DATA(e);
			if ((nla = nl_get_attr(e, sizeof(*ifa), IFA_LABEL)))
				label = NLA_DATA(nla);
			else continue;

			/* Now, print the address */
			if ((nla = nl_get_attr(e, sizeof(*ifa), IFA_ADDRESS))) {
				printf("%s has %s address: %s\n", label, disp,
				       inet_ntop(ifa->ifa_family, NLA_DATA(nla), addrbuf,
				       sizeof(addrbuf)));
			}	
		}
	} while(1);

ret:
	if (fd >= 0) close(fd);
	return EXIT_SUCCESS;
}

