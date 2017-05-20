/**
 * nanonl: dump-neighbors: Dump all neighbor table entries
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
static char buf[BUFSIZ];
static char addrbuf[INET6_ADDRSTRLEN];
static struct nlattr *attrs[NDA_MAX + 1];
static struct nlmsghdr *m = (struct nlmsghdr *)(void *)buf;

int main(void)
{
	__u32 len;
	ssize_t br;
	struct nlmsghdr *e;
	struct ndmsg *ndm;
	char state[9];
	unsigned long i;
	const unsigned char *lladdr;
	__u8 family = AF_INET;

	memset(buf, 0, sizeof(buf));
	if ((fd = nl_open(NETLINK_ROUTE, (__u32)getpid())) < 0) {
		perror("Unable to open netlink socket");
		goto ret;
	}

dump_addrs:
	nl_nd_get_neighbors(m, family);
	if ((__u32)nl_send(fd, 0, m) != m->nlmsg_len) {
		fputs("Failed to send neighbor request\n", stderr);
		goto ret;
	}

read:
	if ((br = nl_recv(fd, m, BUFSIZ, NULL)) <= 0) {
		fputs("Failed to read neighbors\n", stderr);
		goto ret;
	} else len = (__u32)br;

	for (e = m; NLMSG_OK(e, len); e = NLMSG_NEXT(e, len)) {
		if (e->nlmsg_type == NLMSG_DONE) {
			if (family == AF_INET) {
				family = AF_INET6;
				goto dump_addrs;
			} else goto ret;
		} else if (e->nlmsg_type != RTM_NEWNEIGH) continue;
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
			printf("%s %-46s @ ", state, addrbuf);
			lladdr = (const unsigned char *)(NLA_DATA(attrs[NDA_LLADDR]));
			for (i = 0; i < attrs[NDA_LLADDR]->nla_len - NLA_HDRLEN; ++i)
				printf(i ? ":%02x" : "%02x", lladdr[i]);
			putchar('\n');
		}
	}
	goto read;

ret:
	if (fd >= 0) close(fd);
	return EXIT_SUCCESS;
}

