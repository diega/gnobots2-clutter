/*
 * File: msg.c
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 9/15/00
 * $Id$
 *
 * Debug and error messages
 *
 * Copyright (C) 2000-2002 Brent Hendricks.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> /* For strcasecmp */
#include <sys/types.h>
#include <unistd.h>

#include "ggz.h"

#include "misc.h" /* Internal data/functions */
#include "support.h"

#if GGZ_HAVE_SYSLOG_H
# include <syslog.h>
#else
/** @brief Log types for ggz logging functions.
 *
 *  @see GGZDebugHandlerFunc.
 *  @note If syslog.h is present the syslog types are used instead.
 */
enum {
	LOG_DEBUG,	/**< A debugging message from ggz_debug. */
	LOG_NOTICE,	/**< A general notice from ggz_log. */
	LOG_ERR,	/**< An error from ggz_error_[msg|sys]. */
	LOG_CRIT	/**< An error from ggz_error_[msg|sys]_exit. */
};
#endif

/* Workhorse function for actually outputting messages */
static void err_doit(int priority, const char *prefix,
                     const char *fmt, va_list ap,
                     char err);

/* A function to fork and dump core, allowing the original process
   to continue. */
static void dump_core(void);

/* Debug file pointer */
static FILE * debug_file;

/* List of registered types */
static GGZList *debug_types;

/* Flag for whether debugging output is enabled */
static char debug_enabled;

static GGZDebugHandlerFunc handler_func = NULL;


void ggz_debug_init(const char **types, const char* file)
{
	int i;
	
	if (file && (debug_file = fopen(file, "a")) == NULL)
		ggz_error_sys_exit("fopen() to open %s", file);


	/* Parse through the provided types */
	if (types) {
		for (i = 0; types[i]; i++)
			ggz_debug_enable(types[i]);
	}
	
	/* We do the actual enabling last, so none of the steps up to
           this point will generate debugging messages */
	debug_enabled = 1;
}


GGZDebugHandlerFunc ggz_debug_set_func(GGZDebugHandlerFunc func)
{
	GGZDebugHandlerFunc old_handler = handler_func;
	handler_func = func;
	return old_handler;
}

static int ggz_list_strcasecmp(const void *a, const void *b)
{
	const char *s_a = a, *s_b = b;

	return strcasecmp(s_a, s_b);
}

/* FIXME: allow specifying NULL to designate enabling all? */
void ggz_debug_enable(const char *type)
{
	/* Make sure type exists and debugging is enabled */
	if (type) {
		
		/* if the list doesn't exist, create it */
		if (!debug_types) {
			/* Setup list of debugging types */
			debug_types = ggz_list_create(ggz_list_strcasecmp,
						      ggz_list_create_str,
						      ggz_list_destroy_str,
						      GGZ_LIST_REPLACE_DUPS);
		}
		
		ggz_list_insert(debug_types, (char*)type);
	}
}


void ggz_debug_disable(const char *type)
{
	GGZListEntry *entry;

	if (type && debug_types) {
		if ( (entry = ggz_list_search(debug_types, (char*)type)))
			ggz_list_delete_entry(debug_types, entry);
	}
}


static void debug_doit(int priority,
                       const char *type, const char *fmt,
                       va_list ap)
{
	if (debug_enabled) {
		/* If type list exists, see if type was enabled...*/
		if (debug_types && ggz_list_search(debug_types, (char*)type)) {
			err_doit(priority, type, fmt, ap, 0);
		}
	}
}


void ggz_debug(const char *type, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	debug_doit(LOG_DEBUG, type, fmt, ap);
	va_end(ap);
}


void ggz_log(const char *type, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	debug_doit(LOG_NOTICE, type, fmt, ap);
	va_end(ap);
}


/* This feature will fork the process and dump core, so that a core file will
   be generated without hurting the running process.

   But, it's disabled because it breaks some things (like KDE). */
static void dump_core(void)
{
#if 0
	int pid = fork();

	if (pid < 0)
		ggz_error_sys_exit("Fork failed");
	else if (pid == 0)
		abort();
	else
		if (waitpid(pid, NULL, 0) <= 0)
			ggz_error_sys_exit("Wait failed");
#endif
}


void ggz_error_sys(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(LOG_ERR, NULL, fmt, ap, 1);
	va_end(ap);

	dump_core();
}


void ggz_error_sys_exit(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(LOG_CRIT, NULL, fmt, ap, 1);
	va_end(ap);

	abort();
}


void ggz_error_msg(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(LOG_ERR, NULL, fmt, ap, 0);
	va_end(ap);

	dump_core();
}


void ggz_error_msg_exit(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(LOG_CRIT, NULL, fmt, ap, 0);
	va_end(ap);

	abort();
}


void ggz_debug_cleanup(GGZCheckType check)
{
	GGZList *list;
	
	/* Turn off debug handling first so nothing after this
           generates a message */
	debug_enabled = 0;
	
	if (debug_types) {
		/* Turn off debug handling by setting types to NULL */
		list = debug_types;
		debug_types = NULL;
		ggz_list_free(list);
	}

	/* Perform any checks specified */
	if (check & GGZ_CHECK_MEM)
		ggz_memory_check();

	if (debug_file) {
		fclose(debug_file);
		debug_file = NULL;
	}
}


static void err_doit(int priority, const char* prefix,
                     const char *fmt, va_list ap,
                     char err)
{
	char buf[4096];

#if 0
	snprintf(buf, sizeof(buf), "[%d]: ", getpid());
#else
	buf[0] = '\0';
#endif

	if (prefix)
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
		         "(%s) ", prefix);
	

	vsnprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), fmt, ap);
	if (err)
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
		         ": %s", strerror(errno));
			
	if (handler_func) {
		(*handler_func)(priority, buf);
	} else if (debug_file) {
		fputs(buf, debug_file);
		fputs("\n", debug_file);
	} else {
		/* Use stderr if no file is given */
		fflush(stdout);
		fputs(buf, stderr);
		fputs("\n", stderr);
	}
	
	fflush(NULL);
}

#if 0 /* currently unused */
/* For debug purposes only */
void ggz_debug_debug(void) 
{
	GGZListEntry *entry;
	
	printf("Debugging subsystem status\n");
	printf("Debug file is %p\n", debug_file);
	if (debug_types) {
		printf("%d debugging types defined:\n",
		       ggz_list_count(debug_types));
		for (entry = ggz_list_head(debug_types);
		     entry != NULL;
		     entry = ggz_list_next(entry)) {
			printf("-- %s\n", (char*)ggz_list_get_data(entry));
		}

	}
	else 
		printf("No debugging types list\n");
}
#endif
