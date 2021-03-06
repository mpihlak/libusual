/*
 * Alloc helpers.
 *
 * Copyright (c) 2009 Marko Kreen
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

#ifndef _USUAL_ALLOC_
#define _USUAL_ALLOC_

#include <usual/base.h>

#include <stdlib.h>

/*
 * Function: free
 *
 * Fix posix bug by accepting const pointer.
 */
static inline void sane_free(const void *p)
{
	free((void *)p);
}
#define free(x) sane_free(x)

/*
 * Function: zmalloc
 *
 * Zeroing malloc
 */
_MUSTCHECK
static inline void *zmalloc(size_t len)
{
	return calloc(1, len);
}

#endif

