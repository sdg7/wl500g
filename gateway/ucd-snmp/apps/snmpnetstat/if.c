/*****************************************************************
	Copyright 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
/*
 * Copyright (c) 1983,1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#include <config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <sys/types.h>

#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <stdio.h>
#include <signal.h>

#if HAVE_WINSOCK_H
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#endif

#include "main.h"
#include "asn1.h"
#include "snmp.h"
#include "snmp_api.h"
#include "snmp_impl.h"
#include "snmp_client.h"
#include "mib.h"
#include "snmp_debug.h"
#include "../../snmplib/system.h"

#include "netstat.h"

#define	YES	1
#define	NO	0

static void sidewaysintpr(unsigned int);

static oid oid_ifname[] =	{1, 3, 6, 1, 2, 1, 2, 2, 1, 2, 1};
static oid oid_ifinucastpkts[] ={1, 3, 6, 1, 2, 1, 2, 2, 1,11, 1};
static oid oid_cfg_nnets[] =	{1, 3, 6, 1, 2, 1, 2, 1, 0};
static oid oid_ipadentaddr[] =	{1, 3, 6, 1, 2, 1, 4,20, 1, 1, 0, 0, 0, 0};

#define IFNAME		2
#define IFMTU		4
#define IFOPERSTATUS	8
#define INOCTETS	10
#define INUCASTPKTS	11
#define INNUCASTPKTS	12
#define INERRORS	14
#define OUTOCTETS	16
#define OUTUCASTPKTS	17
#define OUTNUCASTPKTS	18
#define OUTERRORS	20
#define OUTQLEN		21

#define IPADDR		1
#define	IFINDEX		2
#define IPNETMASK	3


/*
 * Print a description of the network interfaces.
 */
void
intpr(int interval)
{
	oid varname[MAX_OID_LEN], *instance, *ifentry;
	int varname_len;
	int ifnum, cfg_nnets;
	oid curifip [4];
	struct variable_list *var;
	struct snmp_pdu *request, *response;
	int status;
	int ifindex;
	struct _if_info {
	    char name[128];
	    char ip[128], route[128];
	    int mtu;
	    int ipkts, ierrs, opkts, oerrs, operstatus, outqueue;
	    u_long netmask;
	    struct in_addr ifip, ifroute;
	} *if_table, *cur_if;
	int max_name = 0, max_ip = 0, max_route = 0, i;

	if (interval) {
		sidewaysintpr((unsigned)interval);
		return;
	}
	var = getvarbyname(Session, oid_cfg_nnets, sizeof(oid_cfg_nnets) / sizeof(oid));
	if (var && var->val.integer) {
	    cfg_nnets = *var->val.integer;
	    snmp_free_var(var);
	}
	else {
	    fprintf (stderr, "No response when requesting number of interfaces.\n");
	    return;
	}
	DEBUGMSGTL (("netstat:if", "cfg_nnets = %d\n", cfg_nnets));

	memset (curifip, 0, sizeof (curifip));
	if_table = (struct _if_info *) calloc (cfg_nnets, sizeof (*if_table));
	cur_if = if_table;

	for (ifnum = 1; ifnum <= cfg_nnets; ifnum++) {
		register char *cp;

		request = snmp_pdu_create (SNMP_MSG_GETNEXT);
		memmove (varname, oid_ipadentaddr, sizeof (oid_ipadentaddr));
		varname_len = sizeof (oid_ipadentaddr) / sizeof (oid);
		instance = varname + 9;
		memmove (varname + 10, curifip, sizeof (curifip));
		*instance = IFINDEX;
		snmp_add_null_var (request, varname, varname_len);
		*instance = IPADDR;
		snmp_add_null_var (request, varname, varname_len);
		*instance = IPNETMASK;
		snmp_add_null_var (request, varname, varname_len);

		status = snmp_synch_response (Session, request, &response);
		if (status != STAT_SUCCESS || response->errstat != SNMP_ERR_NOERROR) {
		    fprintf (stderr, "SNMP request failed after %d out of %d interfaces (IP)\n", ifnum, cfg_nnets);
		    cfg_nnets = ifnum;
		    break;
		}
		for (var = response->variables; var; var = var->next_variable) {
                  if (snmp_get_do_debugging()) {
		    print_variable (var->name, var->name_length, var);
                  }
		    switch (var->name [9]) {
		    case IFINDEX:
			ifindex = *var->val.integer;
			cur_if = if_table + ifindex - 1;
			break;
		    case IPADDR:
			memmove (curifip, var->name+10, sizeof (curifip));
			memmove (&cur_if->ifip, var->val.string, sizeof (u_long));
			break;
		    case IPNETMASK:
			memmove (&cur_if->netmask, var->val.string, sizeof (u_long));
		    }
		}
		cur_if->ifroute.s_addr = cur_if->ifip.s_addr & cur_if->netmask;
		if (cur_if->ifroute.s_addr)
			strcpy(cur_if->route, netname (cur_if->ifroute, 0));
		else strcpy(cur_if->route, "none");
		if ((i = strlen(cur_if->route)) > max_route) max_route = i;
		if (cur_if->ifip.s_addr)
			strcpy(cur_if->ip, routename (cur_if->ifip));
		else strcpy(cur_if->ip, "none");
		if ((i = strlen(cur_if->ip)) > max_ip) max_ip = i;

		snmp_free_pdu (response);

		memmove (varname, oid_ifname, sizeof(oid_ifname));
		varname_len = sizeof(oid_ifname) / sizeof(oid);
		ifentry = varname + 9;
		instance = varname + 10;
		request = snmp_pdu_create (SNMP_MSG_GET);

		*instance = ifnum;
		*ifentry = IFNAME;
		snmp_add_null_var (request, varname, varname_len);
		*ifentry = IFMTU;
		snmp_add_null_var (request, varname, varname_len);
		*ifentry = IFOPERSTATUS;
		snmp_add_null_var (request, varname, varname_len);
		*ifentry = INUCASTPKTS;
		snmp_add_null_var (request, varname, varname_len);
		*ifentry = INNUCASTPKTS;
		snmp_add_null_var (request, varname, varname_len);
		*ifentry = INERRORS;
		snmp_add_null_var (request, varname, varname_len);
		*ifentry = OUTUCASTPKTS;
		snmp_add_null_var (request, varname, varname_len);
		*ifentry = OUTNUCASTPKTS;
		snmp_add_null_var (request, varname, varname_len);
		*ifentry = OUTERRORS;
		snmp_add_null_var (request, varname, varname_len);
		*ifentry = OUTQLEN;
		snmp_add_null_var (request, varname, varname_len);

		status = snmp_synch_response (Session, request, &response);
		if (status != STAT_SUCCESS || response->errstat != SNMP_ERR_NOERROR) {
		    fprintf (stderr, "SNMP request failed after %d out of %d interfaces (IF)\n", ifnum, cfg_nnets);
		    cfg_nnets = ifnum;
		    break;
		}
		cur_if = if_table + ifnum - 1;
		for (var = response->variables; var; var = var->next_variable) {
                  if (snmp_get_do_debugging()) {
		    print_variable (var->name, var->name_length, var);
                  }
		    if (var->val.integer)
		    switch (var->name [9]) {
		    case OUTQLEN:
			cur_if->outqueue = *var->val.integer; break;
		    case OUTERRORS:
			cur_if->oerrs = *var->val.integer; break;
		    case INERRORS:
			cur_if->ierrs = *var->val.integer; break;
		    case IFMTU:
			cur_if->mtu = *var->val.integer; break;
		    case INUCASTPKTS:
			cur_if->ipkts += *var->val.integer; break;
		    case INNUCASTPKTS:
			cur_if->ipkts += *var->val.integer; break;
		    case OUTUCASTPKTS:
			cur_if->opkts += *var->val.integer; break;
		    case OUTNUCASTPKTS:
			cur_if->opkts += *var->val.integer; break;
		    case IFNAME:
			if (var->val_len >= sizeof(cur_if->name))
			    var->val_len = sizeof(cur_if->name) - 1;
			memmove (cur_if->name, var->val.string, var->val_len);
			cur_if->name [var->val_len] = 0;
			if ((i = strlen(cur_if->name)+1) > max_name) max_name = i;
			break;
		    case IFOPERSTATUS:
			cur_if->operstatus = *var->val.integer; break;
		    }
		}

		snmp_free_pdu (response);

		if (intrface != NULL && strcmp(cur_if->name, intrface) != 0) {
			cur_if->name [0] = 0;
			continue;
		}
		if (cur_if->operstatus != MIB_IFSTATUS_UP) {
			cp = strchr(cur_if->name, '\0');
			*cp++ = '*';
			*cp = '\0';
		}
	}

	printf("%*.*s %5.5s %*.*s %*.*s %9.9s %5.5s %9.9s %5.5s %5.5s",
		-max_name, max_name, "Name", "Mtu",
		-max_route, max_route, "Network",
		-max_ip, max_ip, "Address", "Ipkts", "Ierrs",
		"Opkts", "Oerrs", "Queue");
	putchar('\n');
	for (ifnum = 0, cur_if = if_table; ifnum < cfg_nnets; ifnum++, cur_if++) {
		if (cur_if->name [0] == 0) continue;
		printf("%-*.*s %5d ", max_name, max_name, cur_if->name, cur_if->mtu);
		printf("%-*.*s ", max_route, max_route, cur_if->route);
		printf("%-*.*s ", max_ip, max_ip, cur_if->ip);
		printf("%9d %5d %9d %5d %5d",
		    cur_if->ipkts, cur_if->ierrs,
		    cur_if->opkts, cur_if->oerrs,
		    cur_if->outqueue);
		putchar('\n');
	}
	free(if_table);
}
/*
 * Print a description of the network interfaces.
 */
void
intpro(int interval)
{
	oid varname[MAX_OID_LEN], *instance, *ifentry;
	int varname_len;
	int ifnum, cfg_nnets;
	oid curifip [4];
	struct variable_list *var;
	struct snmp_pdu *request, *response;
	int status;
	int ifindex;
	struct _if_info {
	    char name[128];
	    char ip[128], route[128];
	    unsigned long ioctets, ierrs, ooctets, oerrs, operstatus, outqueue;
	    u_long netmask;
	    struct in_addr ifip, ifroute;
	} *if_table, *cur_if;
	int max_name = 0, max_route = 0, max_ip = 0, i;

	if (interval) {
		sidewaysintpr((unsigned)interval);
		return;
	}
	var = getvarbyname(Session, oid_cfg_nnets, sizeof(oid_cfg_nnets) / sizeof(oid));
	if (var && var->val.integer) {
	    cfg_nnets = *var->val.integer;
	    snmp_free_var(var);
	}
	else {
	    fprintf (stderr, "No response when requesting number of interfaces.\n");
	    return;
	}
	DEBUGMSGTL (("netstat:if", "cfg_nnets = %d\n", cfg_nnets));

	memset (curifip, 0, sizeof (curifip));
	if_table = (struct _if_info *) calloc (cfg_nnets, sizeof (*if_table));
	cur_if = if_table;

	for (ifnum = 1; ifnum <= cfg_nnets; ifnum++) {
		register char *cp;

		request = snmp_pdu_create (SNMP_MSG_GETNEXT);
		memmove (varname, oid_ipadentaddr, sizeof (oid_ipadentaddr));
		varname_len = sizeof (oid_ipadentaddr) / sizeof (oid);
		instance = varname + 9;
		memmove (varname + 10, curifip, sizeof (curifip));
		*instance = IFINDEX;
		snmp_add_null_var (request, varname, varname_len);
		*instance = IPADDR;
		snmp_add_null_var (request, varname, varname_len);
		*instance = IPNETMASK;
		snmp_add_null_var (request, varname, varname_len);

		status = snmp_synch_response (Session, request, &response);
		if (status != STAT_SUCCESS || response->errstat != SNMP_ERR_NOERROR) {
		    fprintf (stderr, "SNMP request failed for interface %d, variable %ld out of %d interfaces (IP)\n", ifnum, response->errindex, cfg_nnets);
		    cfg_nnets = ifnum;
		    break;
		}
		for (var = response->variables; var; var = var->next_variable) {
                  if (snmp_get_do_debugging()) {
		    print_variable (var->name, var->name_length, var);
                  }
		    switch (var->name [9]) {
		    case IFINDEX:
			ifindex = *var->val.integer;
			cur_if = if_table + ifindex - 1;
			break;
		    case IPADDR:
			memmove (curifip, var->name+10, sizeof (curifip));
			memmove (&cur_if->ifip, var->val.string, sizeof (u_long));
			break;
		    case IPNETMASK:
			memmove (&cur_if->netmask, var->val.string, sizeof (u_long));
		    }
		}
		cur_if->ifroute.s_addr = cur_if->ifip.s_addr & cur_if->netmask;
		if (cur_if->ifroute.s_addr)
			strcpy(cur_if->route, netname (cur_if->ifroute, 0));
		else strcpy(cur_if->route, "none");
		if ((i = strlen(cur_if->route)) > max_route) max_route = i;
		if (cur_if->ifip.s_addr)
			strcpy(cur_if->ip, routename (cur_if->ifip));
		else strcpy(cur_if->ip, "none");
		if ((i = strlen(cur_if->ip)) > max_ip) max_ip = i;

		snmp_free_pdu (response);

		memmove (varname, oid_ifname, sizeof(oid_ifname));
		varname_len = sizeof(oid_ifname) / sizeof(oid);
		ifentry = varname + 9;
		instance = varname + 10;
		request = snmp_pdu_create (SNMP_MSG_GET);

		*instance = ifnum;
		*ifentry = IFNAME;
		snmp_add_null_var (request, varname, varname_len);
		*ifentry = IFOPERSTATUS;
		snmp_add_null_var (request, varname, varname_len);
		*ifentry = INOCTETS;
		snmp_add_null_var (request, varname, varname_len);
		*ifentry = OUTOCTETS;
		snmp_add_null_var (request, varname, varname_len);

		status = snmp_synch_response (Session, request, &response);
		if (status != STAT_SUCCESS || response->errstat != SNMP_ERR_NOERROR) {
		    fprintf (stderr, "SNMP request failed for interface %d, variable %ld out of %d interfaces (IF)\n", ifnum, response->errindex, cfg_nnets);
		    cfg_nnets = ifnum;
		    break;
		}
		cur_if = if_table + ifnum - 1;
		for (var = response->variables; var; var = var->next_variable) {
                  if (snmp_get_do_debugging()) {
		    print_variable (var->name, var->name_length, var);
                  }
		    switch (var->name [9]) {
		    case OUTQLEN:
			cur_if->outqueue = *var->val.integer; break;
		    case OUTERRORS:
			cur_if->oerrs = *var->val.integer; break;
		    case INERRORS:
			cur_if->ierrs = *var->val.integer; break;
		    case INOCTETS:
			cur_if->ioctets += *var->val.integer; break;
		    case OUTOCTETS:
			cur_if->ooctets += *var->val.integer; break;
		    case IFNAME:
			if (var->val_len >= sizeof(cur_if->name))
			    var->val_len = sizeof(cur_if->name) - 1;
			memmove (cur_if->name, var->val.string, var->val_len);
			cur_if->name [var->val_len] = 0;
			if ((i = strlen(cur_if->name)+1) > max_name) max_name = i;
			break;
		    case IFOPERSTATUS:
			cur_if->operstatus = *var->val.integer; break;
		    }
		}

		snmp_free_pdu (response);

		if (intrface != NULL && strcmp(cur_if->name, intrface) != 0) {
			cur_if->name [0] = 0;
			continue;
		}
		if (cur_if->operstatus != MIB_IFSTATUS_UP) {
			cp = (char *) strchr(cur_if->name, '\0');
			*cp++ = '*';
			*cp = '\0';
		}
	}

	printf("%*.*s %*.*s %*.*s %12.12s %12.12s ",
		-max_name, max_name, "Name",
		-max_route, max_route, "Network",
		-max_ip, max_ip, "Address", "Ioctets", "Ooctets");
	putchar('\n');
	for (ifnum = 0, cur_if = if_table; ifnum < cfg_nnets; ifnum++, cur_if++) {
		if (cur_if->name [0] == 0) continue;
		printf("%*.*s ", -max_name, max_name, cur_if->name);
		printf("%*.*s ", -max_route, max_route, cur_if->route);
		printf("%*.*s ", -max_ip, max_ip, cur_if->ip);
		printf("%12lu %12lu",
		    cur_if->ioctets,
		    cur_if->ooctets);
		putchar('\n');
	}
	free(if_table);
}

#define	MAXIF	128
struct	iftot {
	char	ift_name[128];		/* interface name */
	int	ift_ip;			/* input packets */
	int	ift_ie;			/* input errors */
	int	ift_op;			/* output packets */
	int	ift_oe;			/* output errors */
	int	ift_co;			/* collisions */
} iftot[MAXIF];

u_char	signalled;			/* set if alarm goes off "early" */

/*
 * timerSet sets or resets the timer to fire in "interval" seconds.
 * timerPause waits only if the timer has not fired.
 * timing precision is not considered important.
 */

#if (defined(WIN32) || defined(cygwin))
static int sav_int;
static time_t timezup;
static void
timerSet(int interval_seconds)
{
    sav_int = interval_seconds;
	timezup = time(0) + interval_seconds;
}
/* you can do better than this ! */
static void
timerPause(void)
{
	time_t now;
	while (time(&now) < timezup)
#ifdef WIN32
		Sleep(400);
#else
	{
		struct timeval tx;
		tx.tv_sec = 0;
		tx.tv_usec = 400 * 1000; /* 400 milliseconds */
		select( 0,0,0,0,&tx);
	}
#endif
}

#else

/*
 * Called if an interval expires before sidewaysintpr has completed a loop.
 * Sets a flag to not wait for the alarm.
 */
RETSIGTYPE
catchalarm(int sig)
{
	signalled = YES;
}

static void
timerSet(int interval_seconds)
{
#ifdef HAVE_SIGSET
	(void)sigset(SIGALRM, catchalarm);
#else
	(void)signal(SIGALRM, catchalarm);
#endif
	signalled = NO;
	(void)alarm(interval_seconds);
}

static void
timerPause(void)
{
#ifdef HAVE_SIGHOLD
	sighold(SIGALRM);
	if (! signalled) {
		sigpause(SIGALRM);
	}
#else
	int oldmask;
	oldmask = sigblock(sigmask(SIGALRM));
	if (! signalled) {
		sigpause(0);
	}
	sigsetmask(oldmask);
#endif
}

#endif /* !WIN32 && !cygwin */

/*
 * Print a running summary of interface statistics.
 * Repeat display every interval seconds, showing statistics
 * collected over that interval.  Assumes that interval is non-zero.
 * First line printed at top of screen is always cumulative.
 */
static void
sidewaysintpr(unsigned int interval)
{
	register struct iftot *ip, *total;
	register int line;
	struct iftot *lastif, *sum, *interesting, ifnow, *now = &ifnow;
	struct variable_list *var;
	oid varname[MAX_OID_LEN], *instance, *ifentry;
	int varname_len;
	int ifnum, cfg_nnets;

	lastif = iftot;
	sum = iftot + MAXIF - 1;
	total = sum - 1;
	interesting = iftot;
	var = getvarbyname(Session, oid_cfg_nnets, sizeof(oid_cfg_nnets) / sizeof(oid));
	if (var) {
	    cfg_nnets = *var->val.integer;
	    snmp_free_var(var);
	}
	else
	    return;
	memmove(varname, oid_ifname, sizeof(oid_ifname));
	varname_len = sizeof(oid_ifname) / sizeof(oid);
	for (ifnum = 1, ip = iftot; ifnum <= cfg_nnets; ifnum++) {
		char *cp;

		ip->ift_name[0] = '(';
		varname[10] = ifnum;
		var = getvarbyname(Session, varname, varname_len);
		if (var){
		    if (var->val_len >= (sizeof(ip->ift_name) - 3))
		        var->val_len = (sizeof(ip->ift_name) - 4);
		    memmove(ip->ift_name + 1, var->val.string, var->val_len);
		    snmp_free_var(var);
		}
                cp = (char *) strchr(ip->ift_name, ' ');
                if ( cp != NULL )
                  *cp = '\0';
		if (intrface && strcmp(ip->ift_name + 1, intrface) == 0)
			interesting = ip;
		ip->ift_name[15] = '\0';
		cp = (char *) strchr(ip->ift_name, '\0');
		sprintf(cp, ")");
		ip++;
		if (ip >= iftot + MAXIF - 2)
			break;
	}
	lastif = ip;

	timerSet(interval);

banner:
	printf("    input   %-6.6s    output       ", interesting->ift_name);
	if (lastif - iftot > 0)
		printf("     input  (Total)    output");
	for (ip = iftot; ip < iftot + MAXIF; ip++) {
		ip->ift_ip = 0;
		ip->ift_ie = 0;
		ip->ift_op = 0;
		ip->ift_oe = 0;
		ip->ift_co = 0;
	}
	putchar('\n');
	printf("%8.8s %5.5s %8.8s %5.5s %5.5s ",
		"packets", "errs", "packets", "errs", "colls");
	if (lastif - iftot > 0)
		printf("%8.8s %5.5s %8.8s %5.5s %5.5s ",
			"packets", "errs", "packets", "errs", "colls");
	putchar('\n');
	fflush(stdout);
	line = 0;
loop:
	sum->ift_ip = 0;
	sum->ift_ie = 0;
	sum->ift_op = 0;
	sum->ift_oe = 0;
	sum->ift_co = 0;
	memmove(varname, oid_ifinucastpkts, sizeof(oid_ifinucastpkts));
	varname_len = sizeof(oid_ifinucastpkts) / sizeof(oid);
	ifentry = varname + 9;
	instance = varname + 10;
	for (ifnum = 1, ip = iftot; ifnum <= cfg_nnets && ip < lastif; ip++, ifnum++) {
		memset(now, 0, sizeof(*now));
		*instance = ifnum;
		*ifentry = INUCASTPKTS;
		var = getvarbyname(Session, varname, varname_len);
		if (var) {
		    now->ift_ip = *var->val.integer;
		    snmp_free_var(var);
		}
		*ifentry = INNUCASTPKTS;
		var = getvarbyname(Session, varname, varname_len);
		if (var) {
		    now->ift_ip += *var->val.integer;
		    snmp_free_var(var);
		}
		*ifentry = INERRORS;
		var = getvarbyname(Session, varname, varname_len);
		if (var) {
		    now->ift_ie = *var->val.integer;
		    snmp_free_var(var);
		}
		*ifentry = OUTUCASTPKTS;
		var = getvarbyname(Session, varname, varname_len);
		if (var) {
		    now->ift_op = *var->val.integer;
		    snmp_free_var(var);
		}
		*ifentry = OUTNUCASTPKTS;
		var = getvarbyname(Session, varname, varname_len);
		if (var) {
		    now->ift_op += *var->val.integer;
		    snmp_free_var(var);
		}
		*ifentry = OUTERRORS;
		var = getvarbyname(Session, varname, varname_len);
		if (var) {
		    now->ift_oe = *var->val.integer;
		    snmp_free_var(var);
		}

		if (ip == interesting)
			printf("%8d %5d %8d %5d %5d ",
				now->ift_ip - ip->ift_ip,
				now->ift_ie - ip->ift_ie,
				now->ift_op - ip->ift_op,
				now->ift_oe - ip->ift_oe,
				now->ift_co - ip->ift_co);
		ip->ift_ip = now->ift_ip;
		ip->ift_ie = now->ift_ie;
		ip->ift_op = now->ift_op;
		ip->ift_oe = now->ift_oe;
		ip->ift_co = now->ift_co;
		sum->ift_ip += ip->ift_ip;
		sum->ift_ie += ip->ift_ie;
		sum->ift_op += ip->ift_op;
		sum->ift_oe += ip->ift_oe;
		sum->ift_co += ip->ift_co;
	}
	if (lastif - iftot > 0)
		printf("%8d %5d %8d %5d %5d ",
			sum->ift_ip - total->ift_ip,
			sum->ift_ie - total->ift_ie,
			sum->ift_op - total->ift_op,
			sum->ift_oe - total->ift_oe,
			sum->ift_co - total->ift_co);
	*total = *sum;
	putchar('\n');
	fflush(stdout);
	line++;

	timerPause();
	timerSet(interval);

	if (line == 21)
		goto banner;
	goto loop;
	/*NOTREACHED*/
}

