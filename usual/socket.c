/*
 * socket helpers.
 *
 * Copyright (c) 2007-2009 Marko Kreen, Skype Technologies OÜ
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */ 

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <usual/socket.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <usual/logging.h>

/* toggle non-blocking flag */
bool socket_set_nonblocking(int fd, bool non_block)
{
	int flags;

	/* get old flags */
	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return false;

	/* flip O_NONBLOCK */
	if (non_block)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;

	/* set new flags */
	if (fcntl(fd, F_SETFL, flags) < 0)
		return false;
	return true;
}

/* initial socket setup */
bool socket_setup(int sock, bool non_block)
{
	int res;

	/* close fd on exec */
	res = fcntl(sock, F_SETFD, FD_CLOEXEC);
	if (res < 0)
		return false;

#ifdef SO_NOSIGPIPE
	/* disallow SIGPIPE, if possible */
	val = 1;
	res = setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof(val));
	if (res < 0)
		return false;
#endif

	/* when no data available, return EAGAIN instead blocking */
	if (!socket_set_nonblocking(sock, non_block))
		return false;

	return true;
}

