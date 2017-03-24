/**
 * nanonl: Netlink Generic Functions
 *
 * Copyright (C) 2015 - 2017 Tim Hentenaar.
 *
 * Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "nl.h"
#include "nl_gen.h"

/**
 * \brief Create a netlink_generic request.
 * \param[in] m       Netlink message buffer.
 * \param[in] pid     Destination netlink port.
 * \param[in] family  Netlink_Generic family ID.
 * \param[in] cmd     Netlink_Generic command.
 * \param[in] version Netlink_Generic version.
 * \param[in] seq     Sequence number (0-based.)
 * \relates nl_request
 */
void nl_gen_request(struct nlmsghdr *m, __u32 pid, __u16 family,
                    __u8 cmd, __u8 version)
{
	struct genlmsghdr *ge = NLMSG_DATA(m);
	if (!m) return;
	nl_request(m, family, pid, GENL_HDRLEN);
	ge->cmd     = cmd;
	ge->version = version;
}

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
int nl_gen_find_family(struct nlmsghdr *m, const char *family)
{
	int ret = 0;
	size_t famlen;

	if (!m || !family || (famlen = strlen(family)) > 0xfffe) {
		errno = EINVAL;
		--ret;
		goto ret;
	}

	nl_gen_request(m, 0, GENL_ID_CTRL, CTRL_CMD_GETFAMILY, 1);
	nl_add_attr(m, CTRL_ATTR_FAMILY_NAME, family, famlen + 1);

ret:
	return ret;
}

