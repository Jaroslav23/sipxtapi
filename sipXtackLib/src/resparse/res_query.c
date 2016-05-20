/*
 * Copyright (c) 1988, 1993
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 	This product includes software developed by the University of
 * 	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 * Portions Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifndef __pingtel_on_posix__
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)res_query.c 8.1 (Berkeley) 6/4/93";
static char orig_rcsid = "From: Id: res_query.c,v 8.14 1997/06/09 17:47:05 halley Exp $";
static char rcsid[] = "";
#endif /* LIBC_SCCS and not lint */

#ifdef WINCE
#   include <types.h>
#else
#   include <sys/types.h>
#endif
#ifndef WINCE /* no errno.h under WinCE */
#include <errno.h>
#endif

#include <os/OsMutexC.h>

/* Reordered includes and separated into win/vx --GAT */
#if defined(_WIN32)
#   include <resparse/wnt/sys/param.h>
#   include <winsock2.h>
#   include <resparse/wnt/netinet/in.h>
#   include <resparse/wnt/arpa/inet.h>
#   include <resparse/wnt/arpa/nameser.h>
#   include <resparse/wnt/nterrno.h>  /* Additional errors not in errno.h --GAT */
#   include <resparse/wnt/resolv/resolv.h>
#elif defined (_VXWORKS)
#   include <netdb.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
/* Use local lnameser.h for info missing from VxWorks version --GAT */
/* lnameser.h is a subset of resparse/wnt/arpa/nameser.h                */
#   include <resolv/nameser.h>
#   include <resparse/vxw/arpa/lnameser.h>
/* Use local lresolv.h for info missing from VxWorks version --GAT */
/* lresolv.h is a subset of resparse/wnt/resolv/resolv.h               */
#   include <resolv/resolv.h>
#   include <resparse/vxw/resolv/lresolv.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "resparse/types.h"
#include "resparse/res_config.h"
#include <os/OsDefs.h>

/* following unnecessary, in errno.h (VxWorks) or nterrno.h (win32) --GAT */
/* #define ECONNREFUSED    146  */ /* Connection refused */
#define NETDB_INTERNAL  -1      /* see errno, in netdb.h */
                                /* issetugid(), function can not be found */

#if PACKETSZ > 1024
#define MAXPACKET PACKETSZ
#else
#define MAXPACKET 1024
#endif

extern OsMutexC resGlobalLock;

/*
 * Formulate a normal query, send, and await answer.
 * Returned answer is placed in supplied buffer "answer".
 * Perform preliminary check of answer, returning success only
 * if no error is indicated and the answer count is nonzero.
 * Return the size of the response on success, -1 on error.
 * Error number is left in h_reserrno.
 *
 * Caller must parse answer and determine whether it answers the question.
 */
int res_query(const char *name,       /* domain name */
              int class, int type,        /* class and type of query */
              u_char *answer,         /* buffer to put answer */
              int anslen             /* size of answer buffer */)
{
    u_char buf[MAXPACKET];
    HEADER *hp = (HEADER *) answer;
    int n;

    hp->rcode = NOERROR;    /* default */

    acquireMutex(resGlobalLock);
    if ((_sip_res.options & RES_INIT) == 0 && res_init() == -1) {
      releaseMutex(resGlobalLock);
      return (-1);
	}
   releaseMutex(resGlobalLock);

#ifdef DEBUG
    if (_sip_res.options & RES_DEBUG)
        printf(";; res_query(%s, %d, %d)\n", name, class, type);
#endif

    n = res_mkquery(QUERY, name, class, type, NULL, 0, NULL,
                    buf, sizeof(buf));
    if (n <= 0) {
#ifdef DEBUG
        if (_sip_res.options & RES_DEBUG)
            printf(";; res_query: mkquery failed\n");
#endif
        return (n);
	}
	n = res_send(buf, n, answer, anslen);
	if (n < 0) {
#ifdef DEBUG
        if (_sip_res.options & RES_DEBUG)
            printf(";; res_query: send error\n");
#endif
        return (n);
	}

    if (hp->rcode != NOERROR || ntohs((u_short)hp->ancount) == 0) {
#ifdef DEBUG
        if (_sip_res.options & RES_DEBUG)
            printf(";; rcode = %d, ancount=%d\n", hp->rcode,
                   ntohs((u_short)hp->ancount));
#endif
        switch (hp->rcode) {
        case NXDOMAIN:
            break;
        case SERVFAIL:
            break;
        case NOERROR:
            break;
        case FORMERR:
        case NOTIMP:
        case REFUSED:
        default:
            break;
        }
        return (-1);
    }
    return (n);
}

/*
 * Perform a call on res_query on the concatenation of name and domain,
 * removing a trailing dot from name if domain is NULL.
 */
int res_local_querydomain(const char *name,
                          const char *domain,
                          int class, int type,   /* class and type of query */
                          u_char *answer,        /* buffer to put answer */
                          int anslen             /* size of answer */)
{
    char nbuf[MAXDNAME];
    const char *longname = nbuf;
    int n, d;

    acquireMutex(resGlobalLock);
    if ((_sip_res.options & RES_INIT) == 0 && res_init() == -1) {
       releaseMutex(resGlobalLock);
       return (-1);
    }
    releaseMutex(resGlobalLock);

#ifdef DEBUG
    if (_sip_res.options & RES_DEBUG)
        printf(";; res_local_querydomain(%s, %s, %d, %d)\n",
               name, domain?domain:"<Nil>", class, type);
#endif
    if (domain == NULL) {
        /*
         * Check for trailing '.';
         * copy without '.' if present.
         */
        n = strlen(name);
        if (n >= MAXDNAME) {
            return (-1);
        }
        n--;
        if (n >= 0 && name[n] == '.') {
            strncpy(nbuf, name, n);
            nbuf[n] = '\0';
        } else
            longname = name;
    } else {
        n = strlen(name);
        d = strlen(domain);
        if (n + d + 1 >= MAXDNAME) {
            return (-1);
        }
    sprintf(nbuf, "%s.%s", name, domain);
    }
    return (res_query(longname, class, type, answer, anslen));
}

#endif /* __pingtel_on_posix__ */
