/**
 * \file nl.h
 *
 * nanonl: Common Netlink Functions
 * Copyright (C) 2015 - 2017 Tim Hentenaar.
 *
 * This code is Licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */
#ifndef NL_H
#define NL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

/* Re-define these to avoid implicit int promotion */
#undef NLA_ALIGN
#undef NLA_HDRLEN
#define NLA_ALIGN(X) (((X) + 3) & 0xfffc)
#define NLA_HDRLEN (NLA_ALIGN(sizeof(struct nlattr)))

/**
 * This macro is simply to make getting a byte-aligned
 * pointer to a specific byte offset within a struct
 * a little more readable.
 */
#define BYTE_OFF(X, O) ((void *)((char *)(X) + (size_t)(O)))

/**
 * Get a byte-aligned pointer to a netlink
 * attribute's payload.
 */
#define NLA_DATA(X) BYTE_OFF((X), (size_t)(NLA_HDRLEN))

/**
 * \brief Initialize a netlink request.
 * \param[in] m     Netlink message buffer.
 * \param[in] type  Netlink message type.
 * \param[in] port  Destination netlink port.
 * \param[in] len   Initial payload length.
 */
#define nl_request(m, type, port, len) \
	nl_msg((m), (__u16)(type), NLM_F_REQUEST, (port), (len))

/**
 * \brief Open a netlink socket.
 * \param[in] protocol Netlink protocol to use (e.g. \a NETLINK_ROUTE).
 * \param[in] port     Netlink port ID to bind to.
 * \return A valid file descriptor, or -1 on error (with \a errno set.)
 */
int nl_open(int protocol, __u32 port);

/**
 * \brief Join or leave any number of multicast groups
 * \param[in] fd    Netlink fd
 * \param[in] join  non-zero to join, zero to leave
 * \param[in] group A list of groups, terminated by zero.
 * \return 0 on success, non-zero on failure (-1 with \a errno set.)
 */
int nl_multicast(int fd, int join, int group, ...);

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
ssize_t nl_send(int fd, __u32 port, struct nlmsghdr *msg);

/**
 * \brief Receive a netlink message.
 * \param[in]     fd   Netlink socket file descriptor.
 * \param[in]     msg  Buffer to write the received message.
 * \param[in,out] len  Length (in bytes) of \a msg.
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
ssize_t nl_recv(int fd, struct nlmsghdr *msg, size_t *len, __u32 *port);

/**
 * \brief Initialize a netlink message.
 * \param[in] m     Netlink message buffer.
 * \param[in] type  Netlink message type.
 * \param[in] flags Netlink message flags.
 * \param[in] port  Destination netlink port.
 * \param[in] len   Initial payload length.
 */
void nl_msg(struct nlmsghdr *m, __u16 type, __u16 flags, __u32 port,
            size_t len);

/**
 * \brief Add a netlink attribute (NLA) to a packet.
 * \param[in] m    Netlink message buffer.
 * \param[in] type Attribute type.
 * \param[in] data Attribute data.
 * \param[in] len  Length of attribute data.
 */
void nl_add_attr(struct nlmsghdr *m, __u16 type, const void *data,
                 size_t len);

/**
 * \brief Add a nested netlink attribute to a packet
 * \param[in] m    Netlink message buffer.
 * \param[in] type Attribute type.
 * \return the nested attribute.
 */
struct nlattr *nla_start(struct nlmsghdr *m, __u16 type);

/**
 * \brief Add a netlink attribute (NLA) to a nested NLA
 * \param[in] nla  Netlink nested attribute.
 * \param[in] type Attribute type.
 * \param[in] data Attribute data.
 * \param[in] len  Length of attribute data.
 */
void nla_add_attr(struct nlattr *nla, __u16 type, const void *data,
                  size_t len);

/**
 * \brief Finalize a nested netlink attribute
 * \param[in] m   Netlink message buffer.
 * \param[in] nla Netlink nested attribute
 */
void nla_end(struct nlmsghdr *m, const struct nlattr *nla);

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
struct nlattr *nl_get_attr(struct nlmsghdr *m, size_t extra_len, __u16 type);

/**
 * \brief Get a NLA from within a nested NLA, by type.
 * \param[in] nla  Netlink nested attribute.
 * \param[in] type Attribute type.
 * \return Pointer to the requested attribute (if found, NULL otherwise.)
 */
struct nlattr *nla_get_attr(struct nlattr *nla, __u16 type);

#endif /* NL_H */

