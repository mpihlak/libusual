/*
 * libusual - Utility library for C
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

/*
 * Wrappers around regular I/O functions (send/recv/read/write)
 * that survive EINTR and also can log problems.
 */

#include <usual/safeio.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <usual/logging.h>

int safe_read(int fd, void *buf, int len)
{
	int res;
loop:
	res = read(fd, buf, len);
	if (res < 0 && errno == EINTR)
		goto loop;
	return res;
}

int safe_write(int fd, const void *buf, int len)
{
	int res;
loop:
	res = write(fd, buf, len);
	if (res < 0 && errno == EINTR)
		goto loop;
	return res;
}

int safe_recv(int fd, void *buf, int len, int flags)
{
	int res;
loop:
	res = recv(fd, buf, len, flags);
	if (res < 0 && errno == EINTR)
		goto loop;
	if (res < 0)
		log_noise("safe_recv(%d, %d) = %s", fd, len, strerror(errno));
	else if (cf_verbose > 2)
		log_noise("safe_recv(%d, %d) = %d", fd, len, res);
	return res;
}

int safe_send(int fd, const void *buf, int len, int flags)
{
	int res;
loop:
	res = send(fd, buf, len, flags);
	if (res < 0 && errno == EINTR)
		goto loop;
	if (res < 0)
		log_noise("safe_send(%d, %d) = %s", fd, len, strerror(errno));
	else if (cf_verbose > 2)
		log_noise("safe_send(%d, %d) = %d", fd, len, res);
	return res;
}

int safe_close(int fd)
{
	int res;
loop:
	/* by manpage, the close() could be interruptable
	   although it seems that at least in linux it cannot happen */
#ifndef WIN32
	res = close(fd);
#else
	/* Pending(this is necessary to wait for FIN of a client.) */
	log_debug("closesocket(%d)",fd);
	res = closesocket(fd);
#endif
	if (res < 0 && errno == EINTR)
		goto loop;
	return res;
}

int safe_recvmsg(int fd, struct msghdr *msg, int flags)
{
	int res;
loop:
	res = recvmsg(fd, msg, flags);
	if (res < 0 && errno == EINTR)
		goto loop;
	if (res < 0)
		log_warning("safe_recvmsg(%d, msg, %d) = %s", fd, flags, strerror(errno));
	else if (cf_verbose > 2)
		log_noise("safe_recvmsg(%d, msg, %d) = %d", fd, flags, res);
	return res;
}

int safe_sendmsg(int fd, const struct msghdr *msg, int flags)
{
	int res;
	int msgerr_count = 0;
loop:
	res = sendmsg(fd, msg, flags);
	if (res < 0 && errno == EINTR)
		goto loop;

	if (res < 0) {
		log_warning("safe_sendmsg(%d, msg[%d,%d], %d) = %s", fd,
			    (int)msg->msg_iov[0].iov_len,
			    (int)msg->msg_controllen,
			    flags, strerror(errno));

		/* with ancillary data on blocking socket OSX returns
		 * EMSGSIZE instead of blocking.  try to solve it by waiting */
		if (errno == EMSGSIZE && msgerr_count < 20) {
			struct timeval tv = {1, 0};
			log_warning("trying to sleep a bit");
			select(0, NULL, NULL, NULL, &tv);
			msgerr_count++;
			goto loop;
		}
	} else if (cf_verbose > 2)
		log_noise("safe_sendmsg(%d, msg, %d) = %d", fd, flags, res);
	return res;
}

static const char *sa2str(const struct sockaddr *sa)
{
	static char buf[256];

	if (sa->sa_family == AF_INET) {
		struct sockaddr_in *in = (struct sockaddr_in *)sa;
		snprintf(buf, sizeof(buf), "%s:%d", inet_ntoa(in->sin_addr), ntohs(in->sin_port));
	} if (sa->sa_family == AF_UNIX) {
		struct sockaddr_un *un = (struct sockaddr_un *)sa;
		snprintf(buf, sizeof(buf), "unix:%s", un->sun_path);
	} else {
		snprintf(buf, sizeof(buf), "sa2str: unknown proto");
	}
	return buf;
}

int safe_connect(int fd, const struct sockaddr *sa, socklen_t sa_len)
{
	int res;
loop:
	res = connect(fd, sa, sa_len);
	if (res < 0 && errno == EINTR)
		goto loop;
	if (res < 0 && (errno != EINPROGRESS || cf_verbose > 2))
		log_noise("connect(%d, %s) = %s", fd, sa2str(sa), strerror(errno));
	else if (cf_verbose > 2)
		log_noise("connect(%d, %s) = %d", fd, sa2str(sa), res);
	return res;
}

int safe_accept(int fd, struct sockaddr *sa, socklen_t *sa_len_p)
{
	int res;
loop:
	res = accept(fd, sa, sa_len_p);
	if (res < 0 && errno == EINTR)
		goto loop;
	if (res < 0)
		log_noise("safe_accept(%d) = %s", fd, strerror(errno));
	else if (cf_verbose > 2)
		log_noise("safe_accept(%d) = %d (%s)", fd, res, sa2str(sa));
	return res;
}

