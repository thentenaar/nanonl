/**
 * nanonl: genl-find-family: simple NETLINK_GENERIC example
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

#include "../src/nl.h"
#include "../src/nl_gen.h"

static int fd = -1;
const char *family = "nlctrl";
static char buf[8192];
static struct nlmsghdr *m = (struct nlmsghdr *)(void *)buf;

int main(int argc, const char *argv[])
{
	__u32 pid;
	size_t len = sizeof(buf);
	struct nlattr *nla;

	if (argc > 1) family = argv[1];
	memset(buf, 0, sizeof(buf));
	if ((fd = nl_open(NETLINK_GENERIC, (__u32)getpid())) < 0) {
		perror("Unable to open netlink socket");
		goto ret;
	}

	if (nl_gen_find_family(m, family) < 0) {
		perror("Unable to build family lookup request");
		goto ret;
	}

	errno = 0;
	if (nl_transact(fd, m, len, &pid) <= 0) {
		if (errno < 0)
			fprintf(stderr, "Got netlink error #%d\n", errno);
		else perror("Failed to do lookup request");
		goto ret;
	}

	printf("Got respone from port 0x%08x\n", pid);
	if (!(nla = nl_gen_get_attr(m, CTRL_ATTR_FAMILY_ID))) {
		fputs("The response is missing the family ID\n", stderr);
		goto ret;
	}
	printf("'%s' has family ID: %u\n", family, *(__u16 *)NLA_DATA(nla));

ret:
	if (fd >= 0) close(fd);
	return EXIT_SUCCESS;
}

