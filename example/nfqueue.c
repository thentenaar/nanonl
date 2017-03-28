/**
 * nanonl: nfqueue: Simple NFQUEUE example
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

#include "../src/nl.h"
#include "../src/nl_nfqueue.h"

static int fd = -1;
static __u16 qn = 0;
static char buf[8192];
static struct nlmsghdr *m = (struct nlmsghdr *)(void *)buf;

int main(int argc, const char *argv[])
{
	__u32 pid;
	struct nfqnl_msg_packet_hdr *phdr;
	size_t len = sizeof(buf);

	if (argc > 1) qn = (__u16)atoi(argv[1]);
	memset(buf, 0, sizeof(buf));
	if ((fd = nl_open(NETLINK_NETFILTER, (__u32)getpid())) < 0) {
		perror("Unable to open netlink socket");
		goto ret;
	}

/* These calls aren't needed on Linux >= 3.8 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
	nl_nfqueue_unbind_pf(m, PF_INET);
	if ((__u32)nl_send(fd, 0, m) != m->nlmsg_len) {
		fputs("Failed to send pfunbind message\n", stderr);
		goto ret;
	}

	nl_nfqueue_bind_pf(m, PF_INET);
	if ((__u32)nl_send(fd, 0, m) != m->nlmsg_len) {
		fputs("Failed to send pfbind message\n", stderr);
		goto ret;
	}
#endif

	nl_nfqueue_bind(m, PF_INET, qn, NFQNL_COPY_PACKET, 0xffff, 0, 0);
	if ((__u32)nl_send(fd, 0, m) != m->nlmsg_len) {
		fputs("Failed to send bind message\n", stderr);
		goto unbind;
	} else printf("Bound to queue %u...\n", qn);
	fflush(stdout);

	do { /* Read and accept packets */
		errno = 0;
		len = sizeof(buf);
		memset(m, 0, sizeof(*m));

		puts("Waiting for packet...");
		if (nl_recv(fd, m, &len, &pid) < 0 || !len) {
			if (errno < 0) {
				fprintf(stderr, "Got netlink error #%d\n",
				        errno);
			} else perror("Failed to read packet");
			break;
		}

		phdr = NLA_DATA(nl_nf_get_attr(m, NFQA_PACKET_HDR));
		phdr->packet_id = ntohl(phdr->packet_id);
		printf("Accepting packet #%u\n", phdr->packet_id);
		nl_nfqueue_verdict(m, qn, phdr->packet_id, NF_ACCEPT);
		if ((__u32)nl_send(fd, 0, m) != m->nlmsg_len) {
			fputs("Failed to send verdict message\n", stderr);
			break;
		}
	} while(1);

unbind:
	nl_nfqueue_unbind(m, PF_INET, qn);
	if ((__u32)nl_send(fd, 0, m) != m->nlmsg_len)
		fputs("Failed to send unbind message\n", stderr);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
	nl_nfqueue_unbind_pf(m, PF_INET);
	if ((__u32)nl_send(fd, 0, m) != m->nlmsg_len)
		fputs("Failed to send pfunbind message\n", stderr);
#endif

ret:
	if (fd >= 0) close(fd);
	return EXIT_SUCCESS;
}

