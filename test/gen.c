#include <string.h>
#include <errno.h>
#include <check.h>

#include "gen.h"
#include "../src/nl_gen.c"

/* 8k is the maximum netlink packet size (from nl.c) */
extern char buf[8192];
static struct nlmsghdr *m = (struct nlmsghdr *)(void *)buf;

static void setup(void)
{
	memset(buf, 0, sizeof(buf));
}

START_TEST(gen_find_family_invalid)
{
	errno = 0;
	ck_assert(nl_gen_find_family(NULL, "test") == -1);
	ck_assert(errno == EINVAL);
	errno = 0;
	ck_assert(nl_gen_find_family(m, NULL) == -1);
	ck_assert(errno == EINVAL);
}
END_TEST


START_TEST(gen_find_family)
{
	struct nlattr *nla = NULL;
	struct genlmsghdr *g = NLMSG_DATA(m);
	nl_gen_find_family(m, "test");
	ck_assert(m->nlmsg_pid == 0);
	ck_assert(m->nlmsg_type == GENL_ID_CTRL);
	ck_assert(m->nlmsg_flags & NLM_F_REQUEST);
	ck_assert(g->cmd == CTRL_CMD_GETFAMILY);
	ck_assert(g->version == 1);
	ck_assert(!!(nla = nl_get_attr(m, GENL_HDRLEN, CTRL_ATTR_FAMILY_NAME)));
	ck_assert(nla && nla->nla_len == NLA_ALIGN(NLA_HDRLEN + 5));
	ck_assert(nla && !memcmp(NLA_DATA(nla), "test", 5));
}
END_TEST

Suite *gen_suite(void)
{
	Suite *s;
	TCase *t;

	s = suite_create("Netlink_Generic Helpers");
	t = tcase_create("find_family");
	tcase_add_checked_fixture(t, setup, NULL);
	tcase_add_test(t, gen_find_family_invalid);
	tcase_add_test(t, gen_find_family);
	tcase_set_timeout(t, 1);
	suite_add_tcase(s, t);
	return s;
}

