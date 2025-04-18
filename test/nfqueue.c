#include <string.h>
#include <arpa/inet.h>
#include <check.h>

#include "nfqueue.h"
#include "../src/nl_nf.c"
#include "../src/nl_nfqueue.c"

/* So that we don't overrun the line where we need this... */
#define NLMSG_TYPE_QUEUE_CFG \
	((NFNL_SUBSYS_QUEUE << 8) | NFQNL_MSG_CONFIG)

/* 8k is the maximum netlink packet size (from nl.c) */
extern char buf[NLMSG_GOODSIZE];
extern struct nlmsghdr *m;

static void setup(void)
{
	memset(buf, 0, NLMSG_GOODSIZE);
}

START_TEST(nfqueue_cmd_ignores_null)
{
	nl_nfqueue_cfg_cmd(NULL, 1, 2, 3);
	ck_assert(!*buf);
	ck_assert(!*(__u32 *)(void *)(buf + NLMSG_HDRLEN));
}
END_TEST

START_TEST(nfqueue_cmd)
{
	struct nlattr *nla = NULL;
	struct nfgenmsg *nf = NULL;
	struct nfqnl_msg_config_cmd *c = NULL;

	nf = NLMSG_DATA(m);
	nl_nfqueue_cfg_cmd(m, 64, PF_INET, 600);

	ck_assert(m->nlmsg_pid == 0);
	ck_assert(m->nlmsg_type == NLMSG_TYPE_QUEUE_CFG);
	ck_assert(m->nlmsg_flags & NLM_F_REQUEST);
	ck_assert(nf->nfgen_family == PF_INET);
	ck_assert(nf->version == NFNETLINK_V0);
	ck_assert(nf->res_id == htons(600));
	ck_assert(!!(nla = nl_nf_get_attr(m, NFQA_CFG_CMD)));
	ck_assert(nla && nla->nla_len == NLA_ALIGN(NLA_HDRLEN + sizeof(*c)));
	ck_assert(nla && (c = NLA_DATA(nla)) && c->command == 64);
	ck_assert(c && c->pf == htons(PF_INET));
}
END_TEST

START_TEST(nfqueue_bind)
{
	__u32 *fl = NULL;
	struct nlattr *nla = NULL;
	struct nfqnl_msg_config_params *p = NULL;

	nl_nfqueue_bind(m, PF_INET, 600, NFQNL_COPY_PACKET, 0xffff, 0, 1);
	ck_assert(!!(nla = nl_nf_get_attr(m, NFQA_CFG_PARAMS)));
	ck_assert(nla && !!(p = NLA_DATA(nla)));
	ck_assert(p && p->copy_mode == NFQNL_COPY_PACKET);
	ck_assert(p && p->copy_range == htonl(0xffff));
	ck_assert(!nl_nf_get_attr(m, NFQA_CFG_QUEUE_MAXLEN));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
	ck_assert(!!(nla = nl_nf_get_attr(m, NFQA_CFG_FLAGS)));
	ck_assert(nla && (fl = NLA_DATA(nla)));
	ck_assert(nla && fl && *fl == htonl(NFQA_CFG_F_CONNTRACK));
	ck_assert(!!(nla = nl_nf_get_attr(m, NFQA_CFG_MASK)));
	ck_assert(nla && (fl = NLA_DATA(nla)));
	ck_assert(nla && fl && *fl == htonl(NFQA_CFG_F_CONNTRACK));
#endif
}
END_TEST

START_TEST(nfqueue_bind_maxlen)
{
	__u32 *fl = NULL;
	struct nlattr *nla = NULL;

	nl_nfqueue_bind(m, 0, 0, 0, 0, 666, 0);
	ck_assert(!!(nla = nl_nf_get_attr(m, NFQA_CFG_QUEUE_MAXLEN)));
	ck_assert(nla && (fl = NLA_DATA(nla)));
	ck_assert(nla && fl && *fl == htonl(666));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
	ck_assert(!nl_nf_get_attr(m, NFQA_CFG_FLAGS));
	ck_assert(!nl_nf_get_attr(m, NFQA_CFG_MASK));
#endif
}
END_TEST


START_TEST(nfqueue_verdict)
{
	struct nlattr *nla = NULL;
	struct nfqnl_msg_verdict_hdr *v = NULL;

	nl_nfqueue_verdict(m, 0, 1234, NF_ACCEPT);
	ck_assert((m->nlmsg_type & 0xff) == NFQNL_MSG_VERDICT);
	ck_assert(!!(nla = nl_nf_get_attr(m, NFQA_VERDICT_HDR)));
	ck_assert(nla && (v = NLA_DATA(nla)));
	ck_assert(nla && v && v->verdict == htonl(NF_ACCEPT));
	ck_assert(nla && v && v->id == htonl(1234));
}
END_TEST

START_TEST(nfqueue_verdict_mark)
{
	struct nlattr *nla = NULL;
	nl_nfqueue_verdict(m, 0, 1234, NF_ACCEPT);
	nl_nfqueue_verdict_mark(m, 9999);
	ck_assert(!!(nla = nl_nf_get_attr(m, NFQA_MARK)));
	ck_assert(nla && *(__u32 *)NLA_DATA(nla) == htonl(9999));
}
END_TEST

START_TEST(nfqueue_verdict_ctmark)
{
	struct nlattr *nla = NULL, *nla2 = NULL;
	nl_nfqueue_verdict(m, 0, 1234, NF_ACCEPT);
	nl_nfqueue_verdict_ctmark(m, 6666);
	ck_assert(!!(nla = nl_nf_get_attr(m, NFQA_CT)));
	ck_assert(nla && (nla2 = nla_get_attr(nla, CTA_MARK)));
	ck_assert(nla2 && *(__u32 *)NLA_DATA(nla2) == htonl(6666));
}
END_TEST

Suite *nfqueue_suite(void)
{
	Suite *s;
	TCase *t;

	s = suite_create("Netfilter / Nfqueue Helpers");
	t = tcase_create("config");
	tcase_add_checked_fixture(t, setup, NULL);
	tcase_add_test(t, nfqueue_cmd_ignores_null);
	tcase_add_test(t, nfqueue_cmd);
	tcase_set_timeout(t, 1);
	suite_add_tcase(s, t);
	t = tcase_create("bind");
	tcase_add_checked_fixture(t, setup, NULL);
	tcase_add_test(t, nfqueue_bind);
	tcase_add_test(t, nfqueue_bind_maxlen);
	tcase_set_timeout(t, 1);
	suite_add_tcase(s, t);
	t = tcase_create("verdict");
	tcase_add_checked_fixture(t, setup, NULL);
	tcase_add_test(t, nfqueue_verdict);
	tcase_add_test(t, nfqueue_verdict_mark);
	tcase_add_test(t, nfqueue_verdict_ctmark);
	tcase_set_timeout(t, 1);
	suite_add_tcase(s, t);
	return s;
}

