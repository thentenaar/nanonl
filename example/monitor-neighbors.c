/**
 * nanonl: monitor-nd: Monitor the neighbor table.
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
#include "../src/nl_nd.h"

static int fd = -1;
static char buf[8192];
static char addrbuf[INET6_ADDRSTRLEN];
static struct nlattr *attrs[NDA_MAX + 1];
static struct nlmsghdr *m = (struct nlmsghdr *)(void *)buf;

int main(void)
{
	__u32 pid;
	unsigned long i;
	struct nlmsghdr *e;
	struct ndmsg *ndm;
	const char *disp;
	char state[9];
	const unsigned char *lladdr;
	size_t len = sizeof(buf);

	memset(buf, 0, sizeof(buf));
	if ((fd = nl_open(NETLINK_ROUTE, (__u32)getpid())) < 0) {
		perror("Unable to open netlink socket");
		goto ret;
	}

	if (nl_multicast(fd, NL_MULTICAST_JOIN, RTNLGRP_NEIGH, 0)) {
		perror("Unable to join the neighbor group");
		goto ret;
	} else puts("Waiting for events...");


loop:
	errno = 0;
	len = sizeof(buf);
	memset(m, 0, sizeof(*m));

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
		if (e->nlmsg_type == RTM_NEWNEIGH)      disp = "new";
		else if (e->nlmsg_type == RTM_DELNEIGH) disp = "del";
		else continue;
		ndm = NLMSG_DATA(e);

		state[8] = 0;
		memset(state, '-', 8);
		if (ndm->ndm_state & NUD_INCOMPLETE) state[0] = 'i';
		if (ndm->ndm_state & NUD_REACHABLE)  state[1] = 'r';
		if (ndm->ndm_state & NUD_STALE)      state[2] = 's';
		if (ndm->ndm_state & NUD_DELAY)      state[3] = 'd';
		if (ndm->ndm_state & NUD_PROBE)      state[4] = 'p';
		if (ndm->ndm_state & NUD_FAILED)     state[5] = 'f';
		if (ndm->ndm_state & NUD_NOARP)      state[6] = 'N';
		if (ndm->ndm_state & NUD_PERMANENT)  state[7] = 'P';

		memset(attrs, 0, sizeof(attrs));
		nl_nd_get_attrv(e, attrs);
		if (attrs[NDA_DST] && attrs[NDA_LLADDR]) {
			inet_ntop(ndm->ndm_family,
			          NLA_DATA(attrs[NDA_DST]),
			          addrbuf, sizeof(addrbuf));
			printf("[%s] %s %-46s @ ", disp, state, addrbuf);
			lladdr = (const unsigned char *)(NLA_DATA(attrs[NDA_LLADDR]));
			for (i = 0; i < attrs[NDA_LLADDR]->nla_len - NLA_HDRLEN; ++i)
				printf(i ? ":%02x" : "%02x", lladdr[i]);
			putchar('\n');
		}
	}
	goto loop;

ret:
	if (fd >= 0) close(fd);
	return EXIT_SUCCESS;
}

