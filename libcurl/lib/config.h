#ifndef _CURL_CONFIG_DOS_H
#define _CURL_CONFIG_DOS_H

/* lib/config.dos - Hand crafted config file for MSDOS.
 *
 * $Id: config.dos,v 1.24 2008-11-03 15:24:44 giva Exp $
 */
#if defined(DJGPP)
  #define OS  "MSDOS/djgpp"
#elif defined(__HIGHC__)
  #define OS  "MSDOS/HighC"
#elif defined(__WATCOMC__)
  #define OS  "MSDOS/Watcom"
#else
  #define OS  "MSDOS/?"
#endif

#define PACKAGE  "curl"

#define HAVE_ARPA_INET_H       1
#define HAVE_FCNTL_H           1
#define HAVE_FIONBIO           1
#define HAVE_GETADDRINFO       1
#define HAVE_GETNAMEINFO       1
#define HAVE_GETPROTOBYNAME    1
#define HAVE_GETTIMEOFDAY      1
#define HAVE_IO_H              1
#define HAVE_IOCTLSOCKET       1
#define HAVE_LOCALE_H          1
#define HAVE_LONGLONG          1
#define HAVE_MEMORY_H          1
#define HAVE_NETDB_H           1
#define HAVE_NETINET_IN_H      1
#define HAVE_NETINET_TCP_H     1
#define HAVE_NET_IF_H          1
#define HAVE_PROCESS_H         1
#define HAVE_RECV              1
#define HAVE_RECVFROM          1
#define HAVE_SELECT            1
#define HAVE_SEND              1
#define HAVE_SETJMP_H          1
#define HAVE_SETLOCALE         1
#define HAVE_SETMODE           1
#define HAVE_SIGNAL            1
#define HAVE_SOCKET            1
#define HAVE_SPNEGO            1
#define HAVE_STRDUP            1
#define HAVE_STRICMP           1
#define HAVE_STRTOLL           1
#define HAVE_STRUCT_TIMEVAL    1
#define HAVE_STRUCT_IN6_ADDR   1
#define HAVE_SYS_IOCTL_H       1
#define HAVE_SYS_SOCKET_H      1
#define HAVE_SYS_STAT_H        1
#define HAVE_SYS_TYPES_H       1
#define HAVE_TERMIOS_H         1
#define HAVE_TIME_H            1
#define HAVE_UNISTD_H          1

#define NEED_MALLOC_H          1

#define RETSIGTYPE             void
#define SIZEOF_LONG_DOUBLE     16
#define STDC_HEADERS           1
#define TIME_WITH_SYS_TIME     1

/* Qualifiers for send(), recv(), recvfrom() and getnameinfo(). */

#define SEND_TYPE_ARG1         int
#define SEND_QUAL_ARG2         const
#define SEND_TYPE_ARG2         void *
#define SEND_TYPE_ARG3         int
#define SEND_TYPE_ARG4         int
#define SEND_TYPE_RETV         int

#define RECV_TYPE_ARG1         int
#define RECV_TYPE_ARG2         void *
#define RECV_TYPE_ARG3         int
#define RECV_TYPE_ARG4         int
#define RECV_TYPE_RETV         int

#define RECVFROM_TYPE_ARG1     int
#define RECVFROM_TYPE_ARG2     void
#define RECVFROM_TYPE_ARG3     int
#define RECVFROM_TYPE_ARG4     int
#define RECVFROM_TYPE_ARG5     struct sockaddr
#define RECVFROM_TYPE_ARG6     int
#define RECVFROM_TYPE_RETV     int
#define RECVFROM_TYPE_ARG2_IS_VOID 1

#define GETNAMEINFO_QUAL_ARG1  const
#define GETNAMEINFO_TYPE_ARG1  struct sockaddr *
#define GETNAMEINFO_TYPE_ARG2  int
#define GETNAMEINFO_TYPE_ARG46 int
#define GETNAMEINFO_TYPE_ARG7  int

#define BSD

/* #define MALLOCDEBUG */

#ifdef USE_ZLIB     /* on cmd-line */
  #define HAVE_ZLIB_H            1
  #define HAVE_LIBZ              1
#endif

#ifdef USE_SSLEAY   /* on cmd-line */
  #define HAVE_CRYPTO_CLEANUP_ALL_EX_DATA 1
  #define HAVE_OPENSSL_ENGINE_H  1
  #define OPENSSL_NO_KRB5        1
  #define USE_OPENSSL            1
#endif

#if 0
/* to disable LDAP */
#define CURL_DISABLE_LDAP        1
#endif
#define HTTP_ONLY                1 /* quake2 needs no other */

#define in_addr_t  u_long
#define socklen_t  int

#if defined(__HIGHC__) || \
    (defined(__GNUC__) && __GNUC__ < 4)   /* gcc 4.x built-in ? */
#define ssize_t    int
#endif

#define CURL_CA_BUNDLE  getenv("CURL_CA_BUNDLE")

/* Target HAVE_x section
 */
#if defined(DJGPP)
  #define HAVE_BASENAME   1
  #define HAVE_STRNICMP   1
  #define HAVE_SIGACTION  1
  #define HAVE_SIGSETJMP  1
  #define HAVE_SYS_TIME_H 1
  #define HAVE_VARIADIC_MACROS_GCC 1

  #if (DJGPP_MINOR >= 4)
  /*#define HAVE_STRLCAT  1*/
  #endif

  /* Because djgpp <= 2.03 doesn't have snprintf() etc. */
  #if (DJGPP_MINOR < 4)
    #define _MPRINTF_REPLACE
  #endif

#elif defined(__WATCOMC__)
  #define HAVE_STRCASECMP 1

#elif defined(__HIGHC__)
  #define HAVE_SYS_TIME_H 1
#endif

#undef word
#undef byte

#endif  /* _CURL_CONFIG_DOS_H */
