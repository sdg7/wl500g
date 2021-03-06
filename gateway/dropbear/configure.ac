#                                               -*- Autoconf -*-
# Process this file with autoconf and autoheader to produce a configure script.

# This Autoconf file was cobbled from various locations. In particular, a bunch
# of the platform checks have been taken straight from OpenSSH's configure.ac
# Huge thanks to them for dealing with the horrible platform-specifics :)

AC_PREREQ(2.59)
AC_INIT
AC_CONFIG_SRCDIR(buffer.c)

OLDCFLAGS=$CFLAGS
# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

if test -z "$LD" ; then
	LD=$CC
fi
AC_SUBST(LD)	

if test -z "$OLDCFLAGS" && test "$GCC" = "yes"; then
	AC_MSG_NOTICE(No \$CFLAGS set... using "-Os -W -Wall" for GCC)
	CFLAGS="-Os -W -Wall -Wno-pointer-sign"
fi

# large file support is useful for scp
AC_SYS_LARGEFILE

# Host specific options
# this isn't a definitive list of hosts, they are just added as required
AC_CANONICAL_HOST

case "$host" in

*-*-linux*)
	no_ptmx_check=1
	;;

*-*-solaris*)
	CFLAGS="$CFLAGS -I/usr/local/include"
	LDFLAGS="$LDFLAGS -L/usr/local/lib -R/usr/local/lib"
	conf_lastlog_location="/var/adm/lastlog"
	AC_MSG_CHECKING(for obsolete utmp and wtmp in solaris2.x)
	sol2ver=`echo "$host"| sed -e 's/.*[[0-9]]\.//'`
	if test "$sol2ver" -ge 8; then
		AC_MSG_RESULT(yes)
		AC_DEFINE(DISABLE_UTMP,,Disable utmp)
		AC_DEFINE(DISABLE_WTMP,,Disable wtmp)
	else
		AC_MSG_RESULT(no)
	fi
	AC_CHECK_LIB(socket, socket, LIBS="$LIBS -lsocket")
	AC_CHECK_LIB(nsl, yp_match, LIBS="$LIBS -lnsl")
	;;

*-*-aix*)
	AC_DEFINE(AIX,,Using AIX)
	# OpenSSH thinks it's broken. If it isn't, let me know.
	AC_DEFINE(BROKEN_GETADDRINFO,,Broken getaddrinfo)
	;;
	
*-*-hpux*)
	LIBS="$LIBS -lsec"
	# It's probably broken.
	AC_DEFINE(BROKEN_GETADDRINFO,,Broken getaddrinfo)
	;;
*-dec-osf*)
	AC_DEFINE(BROKEN_GETADDRINFO,,Broken getaddrinfo)
	;;
esac

AC_CHECK_TOOL(AR, ar, :)
AC_CHECK_TOOL(RANLIB, ranlib, :)
AC_CHECK_TOOL(STRIP, strip, :)
AC_CHECK_TOOL(INSTALL, install, :)

dnl Can't use login() or logout() with uclibc
AC_CHECK_DECL(__UCLIBC__, 
	[
	no_loginfunc_check=1
	AC_MSG_NOTICE([Using uClibc - login() and logout() probably don't work, so we won't use them.])
	],,,)

# Checks for libraries.
AC_CHECK_LIB(crypt, crypt, CRYPTLIB="-lcrypt")
AC_SUBST(CRYPTLIB)	

# Check if zlib is needed
AC_ARG_WITH(zlib,
	[  --with-zlib=PATH        Use zlib in PATH],
	[
		# option is given
		if test -d "$withval/lib"; then
			LDFLAGS="-L${withval}/lib ${LDFLAGS}"
		else
			LDFLAGS="-L${withval} ${LDFLAGS}"
		fi
		if test -d "$withval/include"; then
			CPPFLAGS="-I${withval}/include ${CPPFLAGS}"
		else
			CPPFLAGS="-I${withval} ${CPPFLAGS}"
		fi
	]
)

AC_ARG_ENABLE(zlib,
	[  --disable-zlib          Don't include zlib support],
	[
		if test "x$enableval" = "xno"; then
			AC_DEFINE(DISABLE_ZLIB,, Use zlib)
			AC_MSG_NOTICE(Disabling zlib)
		else
			AC_CHECK_LIB(z, deflate, , AC_MSG_ERROR([*** zlib missing - install first or check config.log ***]))
			AC_MSG_NOTICE(Enabling zlib)
		fi
	],
	[
		# if not disabled, check for zlib
		AC_CHECK_LIB(z, deflate, , AC_MSG_ERROR([*** zlib missing - install first or check config.log ***]))
		AC_MSG_NOTICE(Enabling zlib)
	]
)

# Check if pam is needed
AC_ARG_WITH(pam,
	[  --with-pam=PATH        Use pam in PATH],
	[
		# option is given
		if test -d "$withval/lib"; then
			LDFLAGS="-L${withval}/lib ${LDFLAGS}"
		else
			LDFLAGS="-L${withval} ${LDFLAGS}"
		fi
		if test -d "$withval/include"; then
			CPPFLAGS="-I${withval}/include ${CPPFLAGS}"
		else
			CPPFLAGS="-I${withval} ${CPPFLAGS}"
		fi
	]
)


AC_ARG_ENABLE(pam,
	[  --enable-pam          Try to include PAM support],
	[
		if test "x$enableval" = "xyes"; then
			AC_CHECK_LIB(pam, pam_authenticate, , AC_MSG_ERROR([*** PAM missing - install first or check config.log ***]))
			AC_MSG_NOTICE(Enabling PAM)
			AC_CHECK_FUNCS(pam_fail_delay)
		else
			AC_DEFINE(DISABLE_PAM,, Use PAM)
			AC_MSG_NOTICE(Disabling PAM)
		fi
	],
	[
		# disable it by default
		AC_DEFINE(DISABLE_PAM,, Use PAM)
		AC_MSG_NOTICE(Disabling PAM)
	]
)

AC_ARG_ENABLE(openpty,
	[  --disable-openpty       Don't use openpty, use alternative method],
	[
		if test "x$enableval" = "xno"; then
			AC_MSG_NOTICE(Not using openpty)
		else
			AC_MSG_NOTICE(Using openpty if available)
			AC_SEARCH_LIBS(openpty, util, [AC_DEFINE(HAVE_OPENPTY,,Have openpty() function)])
		fi
	],
	[
		AC_MSG_NOTICE(Using openpty if available)
		AC_SEARCH_LIBS(openpty, util, [AC_DEFINE(HAVE_OPENPTY)])
	]
)
		

AC_ARG_ENABLE(syslog,
	[  --disable-syslog        Don't include syslog support],
	[
		if test "x$enableval" = "xno"; then
			AC_DEFINE(DISABLE_SYSLOG,, Using syslog)
			AC_MSG_NOTICE(Disabling syslog)
		else
			AC_MSG_NOTICE(Enabling syslog)
		fi
	],
	[
		AC_MSG_NOTICE(Enabling syslog)
	]
)

AC_ARG_ENABLE(shadow,
	[  --disable-shadow        Don't use shadow passwords (if available)],
	[
		if test "x$enableval" = "xno"; then
			AC_MSG_NOTICE(Not using shadow passwords)
		else
			AC_CHECK_HEADERS([shadow.h])
			AC_MSG_NOTICE(Using shadow passwords if available)
		fi
	],
	[
		AC_CHECK_HEADERS([shadow.h])
		AC_MSG_NOTICE(Using shadow passwords if available)
	]
)
			

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h netinet/in.h netinet/tcp.h stdlib.h string.h sys/socket.h sys/time.h termios.h unistd.h crypt.h pty.h ioctl.h libutil.h libgen.h inttypes.h stropts.h utmp.h utmpx.h lastlog.h paths.h util.h netdb.h security/pam_appl.h pam/pam_appl.h netinet/in_systm.h sys/uio.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

AC_CHECK_TYPES([uint8_t, u_int8_t, uint16_t, u_int16_t, uint32_t, u_int32_t])
AC_CHECK_TYPES([struct sockaddr_storage])
AC_CHECK_TYPE([socklen_t], ,[
	AC_MSG_CHECKING([for socklen_t equivalent])
	AC_CACHE_VAL([curl_cv_socklen_t_equiv],
	[
	# Systems have either "struct sockaddr *" or
	# "void *" as the second argument to getpeername
	curl_cv_socklen_t_equiv=
	for arg2 in "struct sockaddr" void; do
		for t in int size_t unsigned long "unsigned long"; do
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>

			int getpeername (int, $arg2 *, $t *);
		]],[[
			$t len;
			getpeername(0,0,&len);
		]])],[
			curl_cv_socklen_t_equiv="$t"
			break
		])
		done
	done

	if test "x$curl_cv_socklen_t_equiv" = x; then
		AC_MSG_ERROR([Cannot find a type to use in place of socklen_t])
	fi
	])
	AC_MSG_RESULT($curl_cv_socklen_t_equiv)
	AC_DEFINE_UNQUOTED(socklen_t, $curl_cv_socklen_t_equiv,
			[type to use in place of socklen_t if not defined])],
	[#include <sys/types.h>
	#include <sys/socket.h>])

# for the fake-rfc2553 stuff - straight from OpenSSH

AC_CACHE_CHECK([for struct sockaddr_storage], ac_cv_have_struct_sockaddr_storage, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
		]],
		[[ if (sizeof(struct sockaddr_storage)) return 0 ]])],
		[ ac_cv_have_struct_sockaddr_storage="yes" ],
		[ ac_cv_have_struct_sockaddr_storage="no" ]
	)
])
if test "x$ac_cv_have_struct_sockaddr_storage" = "xyes" ; then
	AC_DEFINE(HAVE_STRUCT_SOCKADDR_STORAGE)
fi

AC_CACHE_CHECK([for struct sockaddr_in6], ac_cv_have_struct_sockaddr_in6, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <netinet/in.h>
		]],
		[[ if (sizeof(struct sockaddr_in6)) return 0 ]])],
		[ ac_cv_have_struct_sockaddr_in6="yes" ],
		[ ac_cv_have_struct_sockaddr_in6="no" ]
	)
])
if test "x$ac_cv_have_struct_sockaddr_in6" = "xyes" ; then
	AC_DEFINE(HAVE_STRUCT_SOCKADDR_IN6,,Have struct sockaddr_in6)
fi

AC_CACHE_CHECK([for struct in6_addr], ac_cv_have_struct_in6_addr, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <netinet/in.h>
		]],
		[[ if (sizeof(struct in6_addr)) return 0 ]])],
		[ ac_cv_have_struct_in6_addr="yes" ],
		[ ac_cv_have_struct_in6_addr="no" ]
	)
])
if test "x$ac_cv_have_struct_in6_addr" = "xyes" ; then
	AC_DEFINE(HAVE_STRUCT_IN6_ADDR,,Have struct in6_addr)
fi

AC_CACHE_CHECK([for struct addrinfo], ac_cv_have_struct_addrinfo, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
		]],
		[[ if (sizeof(struct addrinfo)) return 0 ]])],
		[ ac_cv_have_struct_addrinfo="yes" ],
		[ ac_cv_have_struct_addrinfo="no" ]
	)
])
if test "x$ac_cv_have_struct_addrinfo" = "xyes" ; then
	AC_DEFINE(HAVE_STRUCT_ADDRINFO,,Have struct addrinfo)
fi


# IRIX has a const char return value for gai_strerror()
AC_CHECK_FUNCS(gai_strerror,[
	AC_DEFINE(HAVE_GAI_STRERROR)
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

const char *gai_strerror(int);]],[[
char *str;

str = gai_strerror(0);]])],[
		AC_DEFINE(HAVE_CONST_GAI_STRERROR_PROTO, 1,
		[Define if gai_strerror() returns const char *])])])

# for loginrec.c

AC_CHECK_MEMBERS([struct utmp.ut_host, struct utmp.ut_pid, struct utmp.ut_type, struct utmp.ut_tv, struct utmp.ut_id, struct utmp.ut_addr, struct utmp.ut_addr_v6, struct utmp.ut_exit, struct utmp.ut_time],,,[
#include <sys/types.h>
#if HAVE_UTMP_H
#include <utmp.h>
#endif
])

AC_CHECK_MEMBERS([struct utmpx.ut_host, struct utmpx.ut_syslen, struct utmpx.ut_type, struct utmpx.ut_id, struct utmpx.ut_addr, struct utmpx.ut_addr_v6, struct utmpx.ut_time, struct utmpx.ut_tv],,,[
#include <sys/types.h>
#include <sys/socket.h>
#if HAVE_UTMPX_H
#include <utmpx.h>
#endif
])

AC_CHECK_MEMBERS([struct sockaddr_storage.ss_family],,,[
#include <sys/types.h>
#include <sys/socket.h>
])

AC_CHECK_FUNCS(endutent getutent getutid getutline pututline setutent)
AC_CHECK_FUNCS(utmpname)
AC_CHECK_FUNCS(endutxent getutxent getutxid getutxline pututxline )
AC_CHECK_FUNCS(setutxent utmpxname)
AC_CHECK_FUNCS(logout updwtmp logwtmp)

# OS X monotonic time
AC_CHECK_HEADERS([mach/mach_time.h])
AC_CHECK_FUNCS(mach_absolute_time)

AC_ARG_ENABLE(bundled-libtom,
[  --enable-bundled-libtom       Force using bundled libtomcrypt/libtommath even if a system version exists.
  --disable-bundled-libtom      Force using system libtomcrypt/libtommath, fail if it does not exist.
                                Default is to use system if available, otherwise bundled.],
	[
		if test "x$enableval" = "xyes"; then
			BUNDLED_LIBTOM=1
			AC_MSG_NOTICE(Forcing bundled libtom*)
		else
			BUNDLED_LIBTOM=0
			AC_CHECK_LIB(tommath, mp_exptmod, LIBTOM_LIBS="$LIBTOM_LIBS -ltommath", 
				[AC_MSG_ERROR([Missing system libtommath and --disable-bundled-libtom was specified])] )
			AC_CHECK_LIB(tomcrypt, register_cipher, LIBTOM_LIBS="$LIBTOM_LIBS -ltomcrypt", 
				[AC_MSG_ERROR([Missing system libtomcrypt and --disable-bundled-libtom was specified])] )
		fi
	],
	[
		BUNDLED_LIBTOM=0
		AC_CHECK_LIB(tommath, mp_exptmod, LIBTOM_LIBS="$LIBTOM_LIBS -ltommath", BUNDLED_LIBTOM=1)
		AC_CHECK_LIB(tomcrypt, register_cipher, LIBTOM_LIBS="$LIBTOM_LIBS -ltomcrypt", BUNDLED_LIBTOM=1)
	]
)

if test $BUNDLED_LIBTOM = 1 ; then
	AC_DEFINE(BUNDLED_LIBTOM,,Use bundled libtom) 
fi

AC_SUBST(LIBTOM_LIBS)
AC_SUBST(BUNDLED_LIBTOM)

dnl Added from OpenSSH 3.6.1p2's configure.ac

dnl allow user to disable some login recording features
AC_ARG_ENABLE(lastlog,
	[  --disable-lastlog       Disable use of lastlog even if detected [no]],
	[ AC_DEFINE(DISABLE_LASTLOG,,Disable use of lastlog()) ]
)
AC_ARG_ENABLE(utmp,
	[  --disable-utmp          Disable use of utmp even if detected [no]],
	[ AC_DEFINE(DISABLE_UTMP,,Disable use of utmp) ]
)
AC_ARG_ENABLE(utmpx,
	[  --disable-utmpx         Disable use of utmpx even if detected [no]],
	[ AC_DEFINE(DISABLE_UTMPX,,Disable use of utmpx) ]
)
AC_ARG_ENABLE(wtmp,
	[  --disable-wtmp          Disable use of wtmp even if detected [no]],
	[ AC_DEFINE(DISABLE_WTMP,,Disable use of wtmp) ]
)
AC_ARG_ENABLE(wtmpx,
	[  --disable-wtmpx         Disable use of wtmpx even if detected [no]],
	[ AC_DEFINE(DISABLE_WTMPX,,Disable use of wtmpx) ]
)
AC_ARG_ENABLE(loginfunc,
	[  --disable-loginfunc     Disable use of login() etc. [no]],
	[ no_loginfunc_check=1
	AC_MSG_NOTICE(Not using login() etc) ]
)
AC_ARG_ENABLE(pututline,
	[  --disable-pututline     Disable use of pututline() etc. ([uw]tmp) [no]],
	[ AC_DEFINE(DISABLE_PUTUTLINE,,Disable use of pututline()) ]
)
AC_ARG_ENABLE(pututxline,
	[  --disable-pututxline    Disable use of pututxline() etc. ([uw]tmpx) [no]],
	[ AC_DEFINE(DISABLE_PUTUTXLINE,,Disable use of pututxline()) ]
)
AC_ARG_WITH(lastlog,
  [  --with-lastlog=FILE|DIR specify lastlog location [common locations]],
	[
		if test "x$withval" = "xno" ; then	
			AC_DEFINE(DISABLE_LASTLOG)
		else
			conf_lastlog_location=$withval
		fi
	]
)

if test -z "$no_loginfunc_check"; then
	dnl    Checks for libutil functions (login(), logout() etc, not openpty() )
	AC_SEARCH_LIBS(login, util bsd, [AC_DEFINE(HAVE_LOGIN,,Have login() function)])
	AC_CHECK_FUNCS(logout updwtmp logwtmp)
fi

dnl lastlog, [uw]tmpx? detection
dnl  NOTE: set the paths in the platform section to avoid the
dnl   need for command-line parameters
dnl lastlog and [uw]tmp are subject to a file search if all else fails

dnl lastlog detection
dnl  NOTE: the code itself will detect if lastlog is a directory
AC_MSG_CHECKING([if your system defines LASTLOG_FILE])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <utmp.h>
#ifdef HAVE_LASTLOG_H
#  include <lastlog.h>
#endif
#ifdef HAVE_PATHS_H
#  include <paths.h>
#endif
#ifdef HAVE_LOGIN_H
# include <login.h>
#endif
	]],
	[[ char *lastlog = LASTLOG_FILE; ]])],
	[ AC_MSG_RESULT(yes) ],
	[
		AC_MSG_RESULT(no)
		AC_MSG_CHECKING([if your system defines _PATH_LASTLOG])
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <utmp.h>
#ifdef HAVE_LASTLOG_H
#  include <lastlog.h>
#endif
#ifdef HAVE_PATHS_H
#  include <paths.h>
#endif
		]],
		[[ char *lastlog = _PATH_LASTLOG; ]])],
		[ AC_MSG_RESULT(yes) ],
		[
			AC_MSG_RESULT(no)
			system_lastlog_path=no
		])
	]
)

if test -z "$conf_lastlog_location"; then
	if test x"$system_lastlog_path" = x"no" ; then
		for f in /var/log/lastlog /usr/adm/lastlog /var/adm/lastlog /etc/security/lastlog ; do
				if (test -d "$f" || test -f "$f") ; then
					conf_lastlog_location=$f
				fi
		done
		if test -z "$conf_lastlog_location"; then
			AC_MSG_WARN([** Cannot find lastlog **])
			dnl Don't define DISABLE_LASTLOG - that means we don't try wtmp/wtmpx
		fi
	fi
fi

if test -n "$conf_lastlog_location"; then
	AC_DEFINE_UNQUOTED(CONF_LASTLOG_FILE, "$conf_lastlog_location", lastlog file location)
fi	

dnl utmp detection
AC_MSG_CHECKING([if your system defines UTMP_FILE])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <utmp.h>
#ifdef HAVE_PATHS_H
#  include <paths.h>
#endif
	]],
	[[ char *utmp = UTMP_FILE; ]])],
	[ AC_MSG_RESULT(yes) ],
	[ AC_MSG_RESULT(no)
	  system_utmp_path=no ]
)
if test -z "$conf_utmp_location"; then
	if test x"$system_utmp_path" = x"no" ; then
		for f in /etc/utmp /usr/adm/utmp /var/run/utmp; do
			if test -f $f ; then
				conf_utmp_location=$f
			fi
		done
		if test -z "$conf_utmp_location"; then
			AC_DEFINE(DISABLE_UTMP)
		fi
	fi
fi
if test -n "$conf_utmp_location"; then
	AC_DEFINE_UNQUOTED(CONF_UTMP_FILE, "$conf_utmp_location", utmp file location)
fi	

dnl wtmp detection
AC_MSG_CHECKING([if your system defines WTMP_FILE])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#ifdef HAVE_UTMP_H
#  include <utmp.h>
#endif
#ifdef HAVE_PATHS_H
#  include <paths.h>
#endif
	]],
	[[ char *wtmp = WTMP_FILE; ]])],
	[ AC_MSG_RESULT(yes) ],
	[ AC_MSG_RESULT(no)
	  system_wtmp_path=no ]
)
if test -z "$conf_wtmp_location"; then
	if test x"$system_wtmp_path" = x"no" ; then
		for f in /usr/adm/wtmp /var/log/wtmp; do
			if test -f $f ; then
				conf_wtmp_location=$f
			fi
		done
		if test -z "$conf_wtmp_location"; then
			AC_DEFINE(DISABLE_WTMP)
		fi
	fi
fi
if test -n "$conf_wtmp_location"; then
	AC_DEFINE_UNQUOTED(CONF_WTMP_FILE, "$conf_wtmp_location", wtmp file location)
fi	


dnl utmpx detection - I don't know any system so perverse as to require
dnl  utmpx, but not define UTMPX_FILE (ditto wtmpx.) No doubt it's out
dnl  there, though.
AC_MSG_CHECKING([if your system defines UTMPX_FILE])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <utmp.h>
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif
#ifdef HAVE_PATHS_H
#  include <paths.h>
#endif
	]],
	[[ char *utmpx = UTMPX_FILE; ]])],
	[ AC_MSG_RESULT(yes) ],
	[ AC_MSG_RESULT(no)
	  system_utmpx_path=no ]
)
if test -z "$conf_utmpx_location"; then
	if test x"$system_utmpx_path" = x"no" ; then
		AC_DEFINE(DISABLE_UTMPX)
	fi
else
	AC_DEFINE_UNQUOTED(CONF_UTMPX_FILE, "$conf_utmpx_location", utmpx file location)
fi	

dnl wtmpx detection
AC_MSG_CHECKING([if your system defines WTMPX_FILE])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#ifdef HAVE_UTMP_H
#  include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#  include <utmpx.h>
#endif
#ifdef HAVE_PATHS_H
#  include <paths.h>
#endif
	]],
	[[ char *wtmpx = WTMPX_FILE; ]])],
	[ AC_MSG_RESULT(yes) ],
	[ AC_MSG_RESULT(no)
	  system_wtmpx_path=no ]
)
if test -z "$conf_wtmpx_location"; then
	if test x"$system_wtmpx_path" = x"no" ; then
		AC_DEFINE(DISABLE_WTMPX)
	fi
else
	AC_DEFINE_UNQUOTED(CONF_WTMPX_FILE, "$conf_wtmpx_location", wtmpx file location)
fi	

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([dup2 getspnam getusershell memset putenv select socket strdup clearenv strlcpy strlcat daemon basename _getpty getaddrinfo freeaddrinfo getnameinfo fork writev])

AC_SEARCH_LIBS(basename, gen, AC_DEFINE(HAVE_BASENAME))

# Solaris needs ptmx
if test -z "$no_ptmx_check" ; then
	if test x"$cross_compiling" = x"no" ; then
		if test -e /dev/ptmx ; then
			AC_DEFINE(USE_DEV_PTMX,,Use /dev/ptmx)
		fi
	else
		AC_MSG_NOTICE([Not checking for /dev/ptmx, we're cross-compiling])
	fi
fi

if test -z "$no_ptc_check" ; then
	if test x"$cross_compiling" = x"no" ; then
		if test -e /dev/ptc ; then
			AC_DEFINE(HAVE_DEV_PTS_AND_PTC,,Use /dev/ptc & /dev/pts)
		fi
	else
		AC_MSG_NOTICE([Not checking for /dev/ptc & /dev/pts since we're cross-compiling])
	fi
fi

AC_EXEEXT

# XXX there must be a nicer way to do this
if test $BUNDLED_LIBTOM = 1 ; then
AS_MKDIR_P(libtomcrypt/src/ciphers/aes)
AS_MKDIR_P(libtomcrypt/src/ciphers/safer)
AS_MKDIR_P(libtomcrypt/src/ciphers/twofish)
AS_MKDIR_P(libtomcrypt/src/encauth/ccm)
AS_MKDIR_P(libtomcrypt/src/encauth/eax)
AS_MKDIR_P(libtomcrypt/src/encauth/gcm)
AS_MKDIR_P(libtomcrypt/src/encauth/ocb)
AS_MKDIR_P(libtomcrypt/src/hashes)
AS_MKDIR_P(libtomcrypt/src/hashes/chc)
AS_MKDIR_P(libtomcrypt/src/hashes/helper)
AS_MKDIR_P(libtomcrypt/src/hashes/sha2)
AS_MKDIR_P(libtomcrypt/src/hashes/whirl)
AS_MKDIR_P(libtomcrypt/src/mac/hmac)
AS_MKDIR_P(libtomcrypt/src/mac/omac)
AS_MKDIR_P(libtomcrypt/src/mac/pelican)
AS_MKDIR_P(libtomcrypt/src/mac/pmac)
AS_MKDIR_P(libtomcrypt/src/mac/f9)
AS_MKDIR_P(libtomcrypt/src/mac/xcbc)
AS_MKDIR_P(libtomcrypt/src/math/fp)
AS_MKDIR_P(libtomcrypt/src/misc/base64)
AS_MKDIR_P(libtomcrypt/src/misc/crypt)
AS_MKDIR_P(libtomcrypt/src/misc/mpi)
AS_MKDIR_P(libtomcrypt/src/misc/pkcs5)
AS_MKDIR_P(libtomcrypt/src/modes/cbc)
AS_MKDIR_P(libtomcrypt/src/modes/cfb)
AS_MKDIR_P(libtomcrypt/src/modes/ctr)
AS_MKDIR_P(libtomcrypt/src/modes/ecb)
AS_MKDIR_P(libtomcrypt/src/modes/ofb)
AS_MKDIR_P(libtomcrypt/src/modes/f8)
AS_MKDIR_P(libtomcrypt/src/modes/lrw)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/bit)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/boolean)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/choice)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/ia5)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/integer)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/object_identifier)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/octet)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/printable_string)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/sequence)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/set)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/short_integer)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/utctime)
AS_MKDIR_P(libtomcrypt/src/pk/asn1/der/utf8)
AS_MKDIR_P(libtomcrypt/src/pk/dh)
AS_MKDIR_P(libtomcrypt/src/pk/dsa)
AS_MKDIR_P(libtomcrypt/src/pk/ecc)
AS_MKDIR_P(libtomcrypt/src/pk/katja)
AS_MKDIR_P(libtomcrypt/src/pk/pkcs1)
AS_MKDIR_P(libtomcrypt/src/pk/rsa)
AS_MKDIR_P(libtomcrypt/src/prngs)
LIBTOM_FILES="libtomcrypt/Makefile libtommath/Makefile"
fi
AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES(Makefile $LIBTOM_FILES)
AC_OUTPUT

AC_MSG_NOTICE()
if test $BUNDLED_LIBTOM = 1 ; then
AC_MSG_NOTICE(Using bundled libtomcrypt and libtommath)
else
AC_MSG_NOTICE(Using system libtomcrypt and libtommath)
fi

AC_MSG_NOTICE()
AC_MSG_NOTICE(Now edit options.h to choose features.)
