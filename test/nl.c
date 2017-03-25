#include <string.h>
#include <check.h>

#include "nl.h"
#include "../src/nl.h"
#include "../src/nl.c"

/* 8k is the maximum netlink packet size */
char buf[8192];
static struct nlmsghdr *m = (struct nlmsghdr *)(void *)buf;

static void setup(void)
{
	memset(buf, 0, sizeof(buf));
}

static void nla_setup(void)
{
	setup();
	nl_msg(m, 0xdead, 0xbabe, 0xfeedbeef, 0);
}

START_TEST(nl_msg_ignores_null)
{
	nl_msg(NULL, 0xdead, 0xbabe, 0xfeedbeef, 0xfefefefe);
	ck_assert(!*buf);
}
END_TEST

START_TEST(nl_msg_works)
{
	nl_msg(m, 0xdead, 0xbabe, 0xfeedbeef, 0xfefefefe);
	ck_assert(m->nlmsg_len   == NLMSG_LENGTH(NLMSG_ALIGN(0xfefefefe)));
	ck_assert(m->nlmsg_type  == 0xdead);
	ck_assert(m->nlmsg_flags == 0xbabe);
	ck_assert(m->nlmsg_seq   == 0);
	ck_assert(m->nlmsg_pid   == 0xfeedbeef);
}
END_TEST

START_TEST(nl_add_attr_no_data)
{
	struct nlattr *nla;
	nl_add_attr(m, 0xface, NULL, 666);
	nla = NLMSG_DATA(m);
	ck_assert(nla->nla_type == 0xface);
	ck_assert(nla->nla_len  == NLA_HDRLEN);
	ck_assert(NLMSG_OK(m, m->nlmsg_len));
}
END_TEST

START_TEST(nl_add_attr_no_len)
{
	struct nlattr *nla;
	nl_add_attr(m, 0xface, "test", 0);
	nla = NLMSG_DATA(m);
	ck_assert(nla->nla_type == 0xface);
	ck_assert(nla->nla_len  == NLA_HDRLEN);
	ck_assert(m->nlmsg_len  == (__u32)NLMSG_LENGTH(nla->nla_len));
	ck_assert(NLMSG_OK(m, m->nlmsg_len));
}
END_TEST

START_TEST(nl_add_attr_works)
{
	struct nlattr *nla;
	nl_add_attr(m, 0xface, "test", 4);
	nla = NLMSG_DATA(m);
	ck_assert(nla->nla_type == 0xface);
	ck_assert(nla->nla_len  == NLA_ALIGN(NLA_HDRLEN + 4));
	ck_assert(m->nlmsg_len  == (__u32)NLMSG_LENGTH(nla->nla_len));
	ck_assert(NLMSG_OK(m, m->nlmsg_len));
}
END_TEST

START_TEST(nl_add_attr_append)
{
	struct nlattr *nla;
	size_t len = NLMSG_LENGTH(100);
	m->nlmsg_len += NLMSG_ALIGN(100);
	nl_add_attr(m, 0xface, "test", 4);
	nla = BYTE_OFF(m, NLMSG_LENGTH(100));
	ck_assert(nla->nla_type == 0xface);
	ck_assert(nla->nla_len  == NLA_ALIGN(NLA_HDRLEN + 4));
	ck_assert(m->nlmsg_len  == NLMSG_ALIGN(nla->nla_len + len));
	ck_assert(NLMSG_OK(m, m->nlmsg_len));
}
END_TEST

START_TEST(nl_get_attr_no_attr)
{
	ck_assert(!nl_get_attr(NULL, 0, 0xdead));
	ck_assert(!nl_get_attr(m, 0, 0xdead));
}
END_TEST

START_TEST(nl_get_attr_one)
{
	struct nlattr *nla;
	nla = NLMSG_DATA(m);
	nl_add_attr(m, 0x3ace, "test", 4);
	ck_assert(nl_get_attr(m, 0, 0x3ace) == nla);
}
END_TEST

START_TEST(nl_get_attr_many)
{
	struct nlattr *nla, *nla2;
	nl_add_attr(m, 0x3ead, "aaaaaaaaaaaaa", 13);
	nla = NLMSG_DATA(m);
	nl_add_attr(m, 0x3eef, "bbbbbbbbbbbbb", 13);
	nla2 = BYTE_OFF(NLMSG_DATA(m), nla->nla_len);
	ck_assert(nl_get_attr(m, 0, 0x3ead) == nla);
	ck_assert(nl_get_attr(m, 0, 0x3eef) == nla2);
}
END_TEST

START_TEST(nl_get_attr_extra_data)
{
	struct nlattr *nla;
	m->nlmsg_len += NLMSG_ALIGN(100);
	nl_add_attr(m, 0x3ace, "test", 4);
	nla = BYTE_OFF(m, NLMSG_LENGTH(100));
	ck_assert(nl_get_attr(m, 100, 0x3ace) == nla);
}
END_TEST

START_TEST(nla_start_no_data)
{
	struct nlattr *nla;
	ck_assert(!!(nla = nla_start(m, 0x3ace)));
	ck_assert(nla->nla_type == (0x3ace | NLA_F_NESTED));
	ck_assert(nla->nla_len  == NLA_HDRLEN);
}
END_TEST

START_TEST(nla_start_append)
{
	__u32 off = m->nlmsg_len + NLMSG_ALIGN(100);
	m->nlmsg_len = off;
	ck_assert(nla_start(m, 0x3ace) == BYTE_OFF(m, off));
}
END_TEST

START_TEST(nla_add_attr_no_data)
{
	struct nlattr *nla, *nla2;
	nl_add_attr(m, 0x3ace, NULL, 0);
	nla = NLMSG_DATA(m);
	nla2 = NLA_DATA(nla);
	nla_add_attr(nla, 0x2bef, NULL, 666);
	ck_assert(nla != nla2);
	ck_assert(nla->nla_len == NLA_HDRLEN + NLA_ALIGN(NLA_HDRLEN));
	ck_assert(nla2->nla_type == 0x2bef);
	ck_assert(nla2->nla_len  == NLA_HDRLEN);
}
END_TEST

START_TEST(nla_add_attr_no_len)
{
	struct nlattr *nla, *nla2;
	nl_add_attr(m, 0x3ace, NULL, 0);
	nla = NLMSG_DATA(m);
	nla2 = NLA_DATA(nla);
	nla_add_attr(nla, 0x2bef, "aaaabbbbccdde", 0);
	ck_assert(nla->nla_len == NLA_HDRLEN + NLA_ALIGN(NLA_HDRLEN));
	ck_assert(nla2->nla_len  == NLA_HDRLEN);
}
END_TEST

START_TEST(nla_add_attr_data)
{
	struct nlattr *nla, *nla2;
	nl_add_attr(m, 0x3ace, NULL, 0);
	nla = NLMSG_DATA(m);
	nla2 = (struct nlattr *)NLA_DATA(nla);
	nla_add_attr(nla, 0x2bef, "aaa", 3);
	ck_assert(nla->nla_len == (NLA_HDRLEN << 1) + NLA_ALIGN(3));
	ck_assert(nla2->nla_len == NLA_HDRLEN + NLA_ALIGN(3));

}
END_TEST

START_TEST(nla_end_works)
{
	__u32 len;
	struct nlattr *nla, *nla2;
	len = m->nlmsg_len;
	nla = nla_start(m, 0x3ace);
	nla2 = (struct nlattr *)NLA_DATA(nla);
	nla_add_attr(nla, 0x2bef, "aaa", 3);
	nla_end(m, nla);
	ck_assert(nla->nla_len == (NLA_HDRLEN << 1) + NLA_ALIGN(3));
	ck_assert(nla2->nla_len == NLA_HDRLEN + NLA_ALIGN(3));
	ck_assert(m->nlmsg_len == len + NLMSG_ALIGN(nla->nla_len));
}
END_TEST

START_TEST(nla_get_attr_ignores_null)
{
	ck_assert(!nla_get_attr(NULL, 0x3ace));
}
END_TEST

START_TEST(nla_get_attr_ignores_non_nested)
{
	struct nlattr *nla;
	nla = nla_start(m, 0x2bef);
	nla->nla_type = (__u16)(nla->nla_type & ~NLA_F_NESTED);
	nla_add_attr(nla, 0x3ace, "xxx", 3);
	nla_end(m, nla);
	ck_assert(!nla_get_attr(nla, 0x3ace));
}
END_TEST

START_TEST(nla_get_attr_works)
{
	struct nlattr *nla, *nla2;
	nla = nla_start(m, 0x3ace);
	nla2 = (struct nlattr *)NLA_DATA(nla);
	nla_add_attr(nla, 0x2bef, "aaa", 3);
	nla_end(m, nla);
	ck_assert(nla_get_attr(nla, 0x2bef) == nla2);
}
END_TEST

Suite *nl_suite(void)
{
	Suite *s;
	TCase *t;

	s = suite_create("Core Netlink Helpers");
	t = tcase_create("message construction");
	tcase_add_checked_fixture(t, setup, NULL);
	tcase_add_test(t, nl_msg_ignores_null);
	tcase_add_test(t, nl_msg_works);
	tcase_set_timeout(t, 1);
	suite_add_tcase(s, t);
	t = tcase_create("attribute construction");
	tcase_add_checked_fixture(t, nla_setup, NULL);
	tcase_add_test(t, nl_add_attr_no_data);
	tcase_add_test(t, nl_add_attr_no_len);
	tcase_add_test(t, nl_add_attr_works);
	tcase_add_test(t, nl_add_attr_append);
	tcase_set_timeout(t, 1);
	suite_add_tcase(s, t);
	t = tcase_create("attribute lookup");
	tcase_add_checked_fixture(t, nla_setup, NULL);
	tcase_add_test(t, nl_get_attr_no_attr);
	tcase_add_test(t, nl_get_attr_one);
	tcase_add_test(t, nl_get_attr_many);
	tcase_add_test(t, nl_get_attr_extra_data);
	tcase_set_timeout(t, 1);
	suite_add_tcase(s, t);
	t = tcase_create("nested attributes");
	tcase_add_checked_fixture(t, nla_setup, NULL);
	tcase_add_test(t, nla_start_no_data);
	tcase_add_test(t, nla_start_append);
	tcase_add_test(t, nla_add_attr_no_data);
	tcase_add_test(t, nla_add_attr_no_len);
	tcase_add_test(t, nla_add_attr_data);
	tcase_add_test(t, nla_end_works);
	tcase_add_test(t, nla_get_attr_ignores_null);
	tcase_add_test(t, nla_get_attr_ignores_non_nested);
	tcase_add_test(t, nla_get_attr_works);
	tcase_set_timeout(t, 1);
	suite_add_tcase(s, t);
	return s;
}

