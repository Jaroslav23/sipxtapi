/*
* Copyright (c) 1996 by Internet Software Consortium.
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
/*  */

/* Import. */

#ifdef WINCE
#   include <types.h>
#else
#   include <sys/types.h>
#endif

/* Reordered includes and separated into win/vx --GAT */
#if defined (_WIN32) /* Use Columbia versions for win32 only --GAT */
#       include <winsock2.h>
#       include <resparse/wnt/netinet/in.h>
#       include <resparse/wnt/arpa/nameser.h>
#elif defined(_VXWORKS)
#       include <sys/socket.h>
#       include <netinet/in.h>
/* Use local lnameser.h for info missing from VxWorks version --GAT */
/* lnameser.h is a subset of resparse/wnt/arpa/nameser.h                */
#       include <resolv/nameser.h>
#       include <resparse/vxw/arpa/lnameser.h>
#endif


u_int ns_get16(const u_char *src) {
   u_int dst;

   NS_GET16(dst, src);
   return (dst);
}

u_long ns_get32(const u_char *src) {
   u_long dst;

   NS_GET32(dst, src);
   return (dst);
}

void ns_put16(u_int src, u_char *dst) {
   NS_PUT16(src, dst);
}

void ns_put32(u_long src, u_char *dst) {
   NS_PUT32(src, dst);
}
#endif /* __pingtel_on_posix__ */
