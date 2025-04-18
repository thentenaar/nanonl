/**
 * \file nl_gen.h
 *
 * nanonl: Netlink Generic functions
 * Copyright (C) 2015 - 2025 Tim Hentenaar.
 *
 * This code is Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */
#ifndef NL_GEN_H
#define NL_GEN_H

#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>

#include "nl.h"

/**
 * \def nl_gen_get_attr(m, t)
 * \param m Netlink message buffer
 * \param t NLA type
 *
 * Convenience wrapper around nl_get_attr().
 */
#define nl_gen_get_attr(m, t) \
	nl_get_attr((m), sizeof(struct genlmsghdr), (t))

/**
 * \def nl_gen_get_attrv(m, a)
 * \param m Netlink message buffer
 * \param a Array of \a struct nlattr *
 *
 * Convenience wrapper around nl_get_attrv().
 */
#define nl_gen_get_attrv(m, a)\
	nl_get_attrv((m), sizeof(struct genlmsghdr), (a), \
	             ((sizeof((a)) / sizeof(struct nlattr *)) - 1))

/**
 * \brief Create a netlink_generic request.
 * \param[in] m       Netlink message buffer.
 * \param[in] pid     Destination netlink port.
 * \param[in] family  Netlink_Generic family ID.
 * \param[in] cmd     Netlink_Generic command.
 * \param[in] version Netlink_Generic version.
 * \relates nl_request
 */
void nl_gen_request(struct nlmsghdr *m, __u32 pid, __u16 family,
                    __u8 cmd, __u8 version);

/**
 * \brief Create a netlink_generic family ID lookup request.
 * \param m      Netlink message buffer.
 * \param family Family name.
 * \return 0 on success, non-zero on error (with \a errno set)
 *
 * This function will set \a errno to EINVAL if invalid arguments
 * are passed.
 *
 * To read the family ID from the response, do:
 * \code{.c}
 * struct nlmsghdr *m;
 * struct nlattr *nla;
 *
 * if ((nla = nl_gen_get_attr(m, CTRL_ATTR_FAMILY_ID)))
 *	return *(__u16 *)(NLA_DATA(nla));
 * \endcode
 */
int nl_gen_find_family(struct nlmsghdr *m, const char *family);

#endif /* NL_GEN_H */
