/*
 * File: conf.c
 * Author: Rich Gade
 * Project: GGZ Core Client Lib
 * Date: 11/30/00
 * $Id$
 *
 * External functions for handling configuration files
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

#ifdef HAVE_CONFIG_H
#  include <config.h>		/* Site-specific config */
#endif

#include <stdlib.h>
#include <string.h>

#include <ggz.h>

#include "ggzcore.h"
#include "conf.h"


/* Convenience error macros */
#define NULL_ARG(x) \
	ggz_debug(GGZCORE_DBG_CONF, "NULL argument passed to %s()", x)

#define NO_USER_FILE(x)	\
	ggz_debug(GGZCORE_DBG_CONF, \
		  "Config file write failed - %s() - no user config file", x)

#define NO_FILES(x) \
	ggz_debug(GGZCORE_DBG_CONF, \
		  "Config file read failed - %s() - no config files open", x)


/* Private variables */
static int g_handle = -1;
static int u_handle = -1;

/* Private functions */


/* ggzcore_conf_initialize()
 *	Opens the global and/or user configuration files for the frontend.
 *	Either g_path or u_path can be NULL if the file is not to be used.
 *	The user config file will be created if it does not exist.
 *
 *	Returns:
 *	  -1 on error
 *	  0 on success
 */
int ggzcore_conf_initialize(const char *g_path, const char *u_path)
{
	if(g_handle != -1 || u_handle != -1) {
		ggz_debug(GGZCORE_DBG_CONF,
			  "ggzcore_conf_initialize() called twice");
		/*return -1;*/
		if(g_handle != -1) ggz_conf_close(g_handle);
		if(u_handle != -1) ggz_conf_close(u_handle);
	}

	/* Let ggz_conf handle opening the files */
	if(g_path)
		g_handle = ggz_conf_parse(g_path, GGZ_CONF_RDONLY);
	if(u_path)
		u_handle = ggz_conf_parse(u_path, GGZ_CONF_RDWR | GGZ_CONF_CREATE);
					  

	/* We consider it "success" if EITHER file is sucessfully opened */
	if(g_handle != -1 || u_handle != -1)
		return 0;
	/* OR if neither file was wanted in the first place! */
	else if(g_path == NULL && u_path == NULL)
		return 0;
	else
		return -1;
}


/* ggzcore_conf_write_string()
 *	This will write a configuration string to the user config file.
 *
 *	Returns:
 *	  -1 on error
 *	  0 on success
 */
int ggzcore_conf_write_string(const char *section, const char *key, const char *value)
{
	/* Standard error checks */
	if(section == NULL || key == NULL || value == NULL) {
		NULL_ARG("ggzcore_conf_write_string");
		return -1;
	}
	if(u_handle == -1) {
		NO_USER_FILE("ggzcore_conf_write_string");
		return -1;
	}

	return ggz_conf_write_string(u_handle, section, key, value);
}


/* ggzcore_conf_write_int()
 *	This will write a configuration integer to the user config file.
 *
 *	Returns:
 *	  -1 on error
 *	  0 on success
 */
int ggzcore_conf_write_int(const char *section, const char *key, int value)
{
	/* Standard error checks */
	if(section == NULL || key == NULL) {
		NULL_ARG("ggzcore_conf_write_int");
		return -1;
	}
	if(u_handle == -1) {
		NO_USER_FILE("ggzcore_conf_write_int");
		return -1;
	}

	return ggz_conf_write_int(u_handle, section, key, value);
}


/* ggzcore_conf_write_list()
 *	This will write a configuration list to the user config file.
 *
 *	Returns:
 *	  -1 on error
 *	  0 on success
 */
int ggzcore_conf_write_list(const char *section, const char *key, int argc, char **argv)
{
	/* Standard error checks */
	if(section == NULL || key == NULL) {
		NULL_ARG("ggzcore_conf_write_list");
		return -1;
	}
	if(u_handle == -1) {
		NO_USER_FILE("ggzcore_conf_write_list");
		return -1;
	}

	return ggz_conf_write_list(u_handle, section, key, argc, argv);
}


/* ggzcore_conf_read_string()
 *	This will read a configuration string, attempting to get it
 *	first from the user override file, then the global file.
 *
 *	Returns:
 *	  NULL on error
 *	  ptr to string on success
 */
char * ggzcore_conf_read_string(const char *section, const char *key, const char *def)
{
	char	*s = NULL;

	/* Standard error checks */
	if(section == NULL || key == NULL) {
		NULL_ARG("ggzcore_conf_read_string");
		return NULL;
	}
	if(g_handle == -1 && u_handle == -1) {
		NO_FILES("ggzcore_conf_read_string");
		return NULL;
	}

	/* Check the user file first, then the global */
	if(u_handle != -1)
		s = ggz_conf_read_string(u_handle, section, key, def);
	if(!s && g_handle != -1)
		s = ggz_conf_read_string(g_handle, section, key, def);

	return s;
}


/* ggzcore_conf_read_int()
 *	This will read a configuration integer, attempting to get it
 *	first from the user override file, then the global file.
 *
 *	Returns:
 *	  int from config file on success
 *	  default on failure
 */
int ggzcore_conf_read_int(const char *section, const char *key, int def)
{
	char	*s = NULL;
	int	val;

	/* Standard error checks */
	if(section == NULL || key == NULL) {
		NULL_ARG("ggzcore_conf_read_int");
		return def;
	}
	if(g_handle == -1 && u_handle == -1) {
		NO_FILES("ggzcore_conf_read_int");
		return def;
	}

	/* Check the user file first, then the global */
	/* Note, we get these as strings, otherwise we'd be unable to tell */
	/* if the first return was actually found or we were just getting  */
	/* the default integer.	*/
	if(u_handle != -1)
		s = ggz_conf_read_string(u_handle, section, key, NULL);
	if(!s && g_handle != -1)
		s = ggz_conf_read_string(g_handle, section, key, NULL);

	/* If nothing in either file, give them their default */
	if(!s)
		return def;

	/* Convert the string and free up the returned string var */
	val = atoi(s);
	ggz_free(s);

	return val;
}


/* ggzcore_conf_read_list()
 *	This will read a configuration list, attempting to get it
 *	first from the user override file, then the global file.
 *
 *	Returns:
 *	  an array and count of entries via the arglist
 *	AND
 *	  -1 on error
 *	  0 on success
 */
int ggzcore_conf_read_list(const char *section, const char *key, int *argcp, char ***argvp)
{
	int	rc = -1;

	/* Standard error checks */
	if(section == NULL || key == NULL) {
		NULL_ARG("ggzcore_conf_read_list");
		return -1;
	}
	if(g_handle == -1 && u_handle == -1) {
		NO_FILES("ggzcore_conf_read_list");
		return -1;
	}

	/* Check the user file first, then the global */
	if(u_handle != -1)
		rc = ggz_conf_read_list(u_handle, section, key,
					       argcp, argvp);
	if(rc == -1 && g_handle != -1)
		rc = ggz_conf_read_list(g_handle, section, key,
					       argcp, argvp);

	/* Just return the status from the last read_list call */
	return rc;
}


/* ggzcore_conf_remove_section()
 *	Removes the specified section from the user config file.
 *
 *	Returns:
 *	  0 on success
 *	  1 if [Section] did not exist (soft error)
 *	  -1 on failure
 */
int ggzcore_conf_remove_section(const char *section)
{
	/* Standard error checks */
	if(section == NULL) {
		NULL_ARG("ggzcore_conf_remove_section");
		return -1;
	}
	if(u_handle == -1) {
		NO_USER_FILE("ggzcore_conf_remove_section");
		return -1;
	}

	return ggz_conf_remove_section(u_handle, section);
}


/* ggzcore_conf_remove_key()
 *	Removes the specified key entry from the user config file.
 *
 *	Returns:
 *	  0 on success
 *	  1 if [Section] or Key did not exist (soft error)
 *	  -1 on failure
 */
int ggzcore_conf_remove_key(const char *section, const char *key)
{
	/* Standard error checks */
	if(section == NULL || key == NULL) {
		NULL_ARG("ggzcore_conf_remove_key");
		return -1;
	}
	if(u_handle == -1) {
		NO_USER_FILE("ggzcore_conf_remove_key");
		return -1;
	}

	return ggz_conf_remove_key(u_handle, section, key);
}


/* ggzcore_conf_commit()
 *	Commits any changes that have been made to the user config file.
 *
 *	Returns:
 *	  0 on success
 *	  -1 on failure
 */
int ggzcore_conf_commit(void)
{
	/* Standard error checks */
	if(u_handle == -1) {
		NO_USER_FILE("ggzcore_conf_commit");
		return -1;
	}

	return ggz_conf_commit(u_handle);
}
