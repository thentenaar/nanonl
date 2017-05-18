/**
 * nanonl: Common Netlink Functions
 * Copyright (C) 2015 - 2017 Tim Hentenaar.
 *
 * Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "nl.h"

/**
 * Ensure we have a good value for SOL_NETLINK (linux/sockets.h)
 */
#ifndef SOL_NETLINK
#define SOL_NETLINK 270
#endif

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

static void nl_set_sa(struct sockaddr_nl *sa, __u32 port)
{
	sa->nl_family = AF_NETLINK;
	sa->nl_pad    = 0;
	sa->nl_pid    = port;
	sa->nl_groups = 0;
}

/**
 * \brief Open a netlink socket.
 * \param[in] protocol Netlink protocol to use (e.g. \a NETLINK_ROUTE).
 * \param[in] port     Netlink port ID to bind to.
 * \return A valid file descriptor, or -1 on error (with \a errno set.)
 */
int nl_open(int protocol, __u32 port)
{
	int fd;
	struct sockaddr_nl sa;
	nl_set_sa(&sa, port);

	if ((fd = socket(AF_NETLINK, SOCK_RAW, protocol)) < 0 ||
	    bind(fd, (struct sockaddr *)&sa, sizeof(sa))) {
		if (fd >= 0) close(fd);
		return -1;
	}

	return fd;
}

/**
 * \brief Join or leave any number of multicast groups
 * \param[in] fd    Netlink fd
 * \param[in] join  either NL_MULTICAST_JOIN or NL_MULTICAST_LEAVE
 * \param[in] group A list of groups, terminated by zero.
 * \return 0 on success, non-zero on failure (with \a errno set.)
 */
int nl_multicast(int fd, int join, int group, ...)
{
	va_list ap;
	join = (join ? NETLINK_ADD_MEMBERSHIP : NETLINK_DROP_MEMBERSHIP);

	errno = EINVAL;
	if (fd < 0) goto err;

	va_start(ap, group);
	while (group) {
		if (group < 0) goto va_err;
		if (setsockopt(fd, SOL_NETLINK, join, &group, sizeof(int)))
			goto va_err;
		group = va_arg(ap, int);
	}
	errno = 0;

va_err:
	va_end(ap);

err:
	return !!errno;
}

/**
 * \brief Send a netlink message.
 * \param[in] fd  Netlink socket file descriptor.
 * \param[in] port Destination netlink port.
 * \param[in] msg Message to send.
 * \return Number of bytes sent, or -1 on error (with \a errno set.)
 *
 * In addition to the \a errno values set by \a sendmsg(2)
 * this function will set the following:
 *
 * \a EINVAL - \a msg is NULL, and thus invalid.
 */
ssize_t nl_send(int fd, __u32 port, struct nlmsghdr *msg)
{
	ssize_t i = -1;
	struct iovec iov;
	struct msghdr hdr;
	struct sockaddr_nl sa;
	nl_set_sa(&sa, port);

	if (!msg || !NLMSG_OK(msg, msg->nlmsg_len)) {
		errno = EINVAL;
		goto ret;
	}

	iov.iov_base       = msg;
	iov.iov_len        = msg->nlmsg_len;
	hdr.msg_name       = &sa;
	hdr.msg_namelen    = (socklen_t)sizeof(struct sockaddr_nl);
	hdr.msg_iov        = &iov;
	hdr.msg_iovlen     = 1;
	hdr.msg_control    = NULL;
	hdr.msg_controllen = 0;
	hdr.msg_flags      = 0;
	i = sendmsg(fd, &hdr, 0);

ret:
	return i;
}

/**
 * \brief Receive a netlink message.
 * \param[in]     fd   Netlink socket file descriptor.
 * \param[in]     msg  Buffer to write the received message.
 * \param[in]     len  Length (in bytes) of \a msg.
 * \param[out]    port Sender's port ID (set only if \a port is non-NULL.)
 * \return Number of bytes sent, or -1 on error (with \a errno set.)
 *
 * In addition to the \a errno values set by \a recvmsg(2)
 * this function will set the following:
 *
 * \a EINVAL   - An invalid parameter was passed to this function.
 * \a E2BIG    - The specified value for \a len is too big.
 * \a EMSGSIZE - \a msg is too small to hold the message. The message
 *               length will be written to \a len , and should be
 *               checked before the next attempt to receive the
 *               message. If you want to abandon the message,
 *               simply \a close the socket.
 */
ssize_t nl_recv(int fd, struct nlmsghdr *msg, size_t len, __u32 *port)
{
	ssize_t i;
	struct iovec iov;
	struct msghdr hdr;
	struct sockaddr_nl sa;
	int e = MSG_PEEK;

	if (!msg || !len || len < sizeof(struct nlmsghdr)) {
		errno = EINVAL;
		goto err;
	}

	/**
	 * If the message length is larger than SIZE_MAX >> 1,
	 * recvfrom() may return negative values even though no error
	 * occured, and thus so may this function. But, that's still
	 * a HELL of a lot of bytes to send in one call, and one hell
	 * of a large buffer.
	 *
	 * This will more likely be an indicator of something wrong
	 * elsewhere.
	 */
	if (len > (SIZE_MAX >> 1)) {
		errno = E2BIG;
		goto err;
	}

	iov.iov_base       = msg;
	iov.iov_len        = len;
	hdr.msg_name       = &sa;
	hdr.msg_namelen    = (socklen_t)sizeof(struct sockaddr_nl);
	hdr.msg_iov        = &iov;
	hdr.msg_iovlen     = 1;
	hdr.msg_control    = NULL;
	hdr.msg_controllen = 0;
	hdr.msg_flags      = 0;

	if (fcntl(fd, F_GETFL) & O_NONBLOCK)
		e |= MSG_DONTWAIT;

read:
	/* Read the netlink header to get the message length */
	if ((i = recvmsg(fd, &hdr, e)) < 0)
		goto err;
	if (!i && e & MSG_PEEK) goto ret;
	else if (e & MSG_PEEK) {
		e &= ~MSG_PEEK;
		if (port) *port = sa.nl_pid;
		goto read;
	}

	/* Is the buffer too small? */
	if ((size_t)msg->nlmsg_len > len || !NLMSG_OK(msg, (size_t)i)) {
		errno = EMSGSIZE;
		goto err;
	}

	/* Is this an error message? (error 0 is an ACK) */
	if (msg->nlmsg_type == NLMSG_ERROR) {
		if ((e = ((struct nlmsgerr *)NLMSG_DATA(msg))->error)) {
			errno = e;
			goto err;
		}
	}

ret:
	return i;

err:
	return -1;
}

/**
 * \brief Send a message and read the response
 * \param[in]     fd   Netlink socket file descriptor.
 * \param[in]     m    Buffer to send / recv.
 * \param[in,out] len  Length (in bytes) of \a m.
 * \param[in,out] port Destination port ID (the source port is returned)
 * \return number of bytes read on success, < 0 on error.
 */
ssize_t nl_transact(int fd, struct nlmsghdr *m, size_t len, __u32 *port)
{
	int flags = -1;
	ssize_t ret = -1;

	if (!m || !len)
		goto err;

	/* Ensure the socket is blocking */
	if ((flags = fcntl(fd, F_GETFL, 0)) == -1 ||
	    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK)) goto err;

	/* Tx / Rx */
	if ((size_t)nl_send(fd, port ? *port : 0, m) != m->nlmsg_len ||
	    (ret = nl_recv(fd, m, len, port)) <= 0) goto err;

err:
	if (flags != -1) fcntl(fd, F_SETFL, flags);
	return ret;
}

/**
 * \brief Initialize a netlink message.
 * \param[in] m     Netlink message buffer.
 * \param[in] type  Netlink message type.
 * \param[in] flags Netlink message flags.
 * \param[in] port  Destination netlink port.
 * \param[in] len   Initial payload length.
 */
void nl_msg(struct nlmsghdr *m, __u16 type, __u16 flags, __u32 port,
            size_t len)
{
	if (!m) return;
	m->nlmsg_len   = NLMSG_LENGTH(NLMSG_ALIGN((__u32)len));
	m->nlmsg_type  = type;
	m->nlmsg_flags = flags;
	m->nlmsg_seq   = 0;
	m->nlmsg_pid   = port;
}

/**
 * \brief Add a netlink attribute (NLA) to a packet.
 * \param[in] m    Netlink message buffer.
 * \param[in] type Attribute type.
 * \param[in] data Attribute data.
 * \param[in] len  Length of attribute data.
 */
void nl_add_attr(struct nlmsghdr *m, __u16 type, const void *data,
                 size_t len)
{
	struct nlattr *attr;
	if (!m) return;

	attr = BYTE_OFF(m, NLMSG_ALIGN(m->nlmsg_len));
	attr->nla_type = type;
	attr->nla_len  = NLA_HDRLEN;
	len &= 0xffff;

	if (data && len) {
		if (len < 4) memset(NLA_DATA(attr), 0, NLA_ALIGN(len));
		memcpy(NLA_DATA(attr), data, len);
		attr->nla_len = (__u16)NLA_ALIGN(NLA_HDRLEN + len);
	}

	m->nlmsg_len = NLMSG_ALIGN(m->nlmsg_len + attr->nla_len);
}

/**
 * \brief Add a nested netlink attribute to a packet
 * \param[in] m    Netlink message buffer.
 * \param[in] type Attribute type.
 * \return the nested attribute.
 */
struct nlattr *nla_start(struct nlmsghdr *m, __u16 type)
{
	struct nlattr *attr;
	if (!m) return NULL;

	attr = BYTE_OFF(m, NLMSG_ALIGN(m->nlmsg_len));
	attr->nla_type = type | NLA_F_NESTED;
	attr->nla_len  = NLA_HDRLEN;
	return attr;
}

/**
 * \brief Add a netlink attribute (NLA) to a nested NLA
 * \param[in] nla  Netlink nested attribute.
 * \param[in] type Attribute type.
 * \param[in] data Attribute data.
 * \param[in] len  Length of attribute data.
 */
void nla_add_attr(struct nlattr *nla, __u16 type, const void *data,
                  size_t len)
{
	struct nlattr *attr;

	if (!nla) return;
	attr = (struct nlattr *)BYTE_OFF(nla, nla->nla_len);
	attr->nla_type = type;
	attr->nla_len  = NLA_HDRLEN;
	len &= 0xffff;

	if (data && len) {
		if (len < 4) memset(NLA_DATA(attr), 0, NLA_ALIGN(len));
		memcpy(NLA_DATA(attr), data, len);
		attr->nla_len = (__u16)NLA_ALIGN(NLA_HDRLEN + len);
	}

	nla->nla_len = (__u16)NLA_ALIGN(nla->nla_len + attr->nla_len);
}

/**
 * \brief Finalize a nested netlink attribute
 * \param[in] m   Netlink message buffer.
 * \param[in] nla Netlink nested attribute
 */
void nla_end(struct nlmsghdr *m, const struct nlattr *nla)
{
	if (!m || !nla) return;
	m->nlmsg_len += NLMSG_ALIGN(nla->nla_len);
}

/**
 * \brief Get a netlink attribute (NLA) by its type.
 * \param[in] m         Netlink message buffer.
 * \param[in] extra_len Length of extra headers (e.g. struct genlmsghdr.)
 * \param[in] type      Attribute type.
 * \return Pointer to the requested attribute (if found, NULL otherwise.)
 *
 * NOTE: \a extra_len will be automatically aligned to the correct boundary.
 *
 * Reading data from the NLA works like so:
 * \code{.c}
 * struct nlmsghdr *m;
 * struct nlattr *nla;
 *
 * if ((nla = nl_get_attr(m, 0, MY_ATTR_TYPE)))
 *	return *(mytype *)(NLA_DATA(nla));
 * \endcode
 */
struct nlattr *nl_get_attr(struct nlmsghdr *m, size_t extra_len, __u16 type)
{
	void *attr;
	size_t len;
	struct nlattr *nla;

	if (!m || !NLMSG_OK(m, m->nlmsg_len)) goto ret;
	attr = BYTE_OFF(NLMSG_DATA(m), NLMSG_ALIGN((__u32)extra_len));
	while ((char *)attr - (char *)m < m->nlmsg_len) {
		nla = attr;
		if ((nla->nla_type & NLA_TYPE_MASK) == type)
			return nla;
		len = NLA_ALIGN(nla->nla_len ? nla->nla_len : NLA_HDRLEN);
		attr = (char *)attr + len;
	}

ret:
	return NULL;
}

/**
 * \brief Get a NLA from within a nested NLA, by type.
 * \param[in] nla  Netlink nested attribute.
 * \param[in] type Attribute type.
 * \return Pointer to the requested attribute (if found, NULL otherwise.)
 */
struct nlattr *nla_get_attr(struct nlattr *nla, __u16 type)
{
	struct nlattr *a;
	void *attr;
	size_t len;

	if (!nla || !(nla->nla_type & NLA_F_NESTED))
		goto ret;

	attr = NLA_DATA(nla);
	while ((char *)attr - (char *)nla < nla->nla_len) {
		a = attr;
		if ((a->nla_type & NLA_TYPE_MASK) == type)
			return a;
		len = NLA_ALIGN(a->nla_len ? a->nla_len : NLA_HDRLEN);
		attr = (char *)attr + len;
	}

ret:
	return NULL;
}

/**
 * \brief Gather an array of netlink attributes from a message
 * \param[in]     m         Netlink message buffer.
 * \param[in]     extra_len Length of extra headers (if any.)
 * \param[in,out] attrs     Array of NLA pointers.
 * \param[in]     n         Number of elements in \a attrs.
 * \return Number of NLAs found
 *
 * This function allows you to gather up to a given number of NLAs from
 * a netlink message in one go. For each NLA found in \a m, the
 * element in \a attrs corresponding to that NLA's type will be set
 * to point to the NLA (assuming \a attrs is large enough.)
 *
 * This mimics the interface commonly used in the kernel code for parsing
 * sets of NLAs.
 *
 * NOTE: Each attribute type will be in the range 0 < y <= X_MAX
 * where X denotes a particular attribute enum (i.e. IFA_MAX for
 * interface address attributes.) Attribute types larger than \a n
 * will be ignored.
 *
 * \code{.c}
 * struct nlmsghdr *m;
 * struct nlattr *attrs[IFA_MAX + 1];
 * char buf[INET6_ADDRSTRLEN];
 *
 * memset(attrs, 0, sizeof(attrs));
 * if (!nl_get_attrv(m, sizeof(struct ifaddrmsg), attrs, IFA_MAX))
 * 	nothing_found;
 * if (attrs[IFA_LABEL] && attrs[IFA_ADDRESS])
 * 	printf("Label: %s, Addr: %s\n",
 * 	       (char *)attrs[IFA_ADDRESS].data,
 * 	       inet_ntop(PF_INET, attrs[IFA_LABEL].data,
 * 	       buf, sizeof(buf)));
 * \endcode
 */
__u16 nl_get_attrv(struct nlmsghdr *m, size_t extra_len,
                   struct nlattr *attrs[], __u16 n)
{
	void *attr;
	struct nlattr *nla;
	size_t len;
	__u16 type, found = 0;

	if (!attrs || !n || !m || !NLMSG_OK(m, m->nlmsg_len)) goto ret;
	attr = BYTE_OFF(NLMSG_DATA(m), NLMSG_ALIGN((__u32)extra_len));
	while ((char *)attr - (char *)m < m->nlmsg_len) {
		nla = attr;
		type = (__u16)(nla->nla_type & NLA_TYPE_MASK);
		if (type > 0 && type <= n) {
			attrs[type] = nla;
			if (++found == n) break;
		}

		len = NLA_ALIGN(nla->nla_len ? nla->nla_len : NLA_HDRLEN);
		attr = (char *)attr + len;
	}

ret:
	return found;
}

/**
 * \brief Gather an array of netlink attributes from a nested NLA
 * \param[in]     nla   Nested NLA.
 * \param[in,out] attrs Array of NLA pointers.
 * \param[in]     n     Number of elements in \a attrs.
 * \return Number of NLAs found
 *
 * This function allows you to gather up to a given number of NLAs from
 * a nested NLA in one go. This is \a nl_get_attrv() but for nested NLAs.
 *
 * \code{.c}
 * struct nlattr *nla;
 * struct nlattr *attrs[CTA_IP_MAX + 1];
 *
 * memset(attrs, 0, sizeof(attrs));
 * if (!nla_get_attrv(nla, attrs, CTA_IP_MAX))
 * 	nothing_found;
 * if (attrs[CTA_IP_V4_SRC] && attrs[CTA_IP_V4_DST])
 * 	...;
 * \endcode
 */
__u16 nla_get_attrv(struct nlattr *nla, struct nlattr *attrs[], __u16 n)
{
	struct nlattr *a;
	void *attr;
	size_t len;
	__u16 type, found = 0;

	if (!nla || !(nla->nla_type & NLA_F_NESTED) || !nla->nla_len)
		goto ret;

	attr = NLA_DATA(nla);
	while ((char *)attr - (char *)nla < nla->nla_len) {
		a = attr;
		type = (__u16)(a->nla_type & NLA_TYPE_MASK);
		if (type > 0 && type <= n) {
			attrs[type] = a;
			if (++found == n) break;
		}

		len = NLA_ALIGN(a->nla_len ? a->nla_len : NLA_HDRLEN);
		attr = (char *)attr + len;
	}

ret:
	return found;
}

