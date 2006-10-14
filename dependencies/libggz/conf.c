/*
 * File: conf.c
 * Author: Rich Gade
 * Project: GGZ Core Client Lib
 *          Modified from confio for use by server (rgade - 08/06/01)
 * Date: 11/27/00
 * $Id$
 *
 * Internal functions for handling configuration files
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ggz.h"

#include "misc.h" /* Internal data/functions */
#include "support.h"

/* The majority of this code deals with maintaining a set of lists to
 * cache all the configuration variables in memory until they need to
 * be committed.  There are three types of lists:
 *
 * 1) A list of configuration files containing
 *	a) the path to the configuration file on disk
 *	b) an integer handle which the caller uses to reference the file
 *	c) a pointer to a list of configuration file sections
 * 2) Section lists, which contain
 *	a) the section name
 *	b) a pointer to a list of key/value pair entries
 * 3) Entry lists, which contain
 *	a) the key (variable name)
 *	b) the value (variable data)
 */

#ifdef MKDIR_TAKES_ONE_ARG
#define mkdir(a, b) (mkdir) (a)
#endif


/****
 **** Structure definitions for our two list element types
 ****/

typedef struct conf_file_t {
        char            *path;
        int             handle;
        int             writeable;
        GGZList          *section_list;
} conf_file_t;

typedef struct conf_section_t {
        char            *name;
        GGZList          *entry_list;
} conf_section_t;

typedef struct conf_entry_t {
        char    *key;
        char    *value;
} conf_entry_t;

/* Our private functions and vars */
static GGZList * file_parser(const char *path);
static void parse_line(char *line, char **varname, char **varvalue);
static int section_compare(const void *a, const void *b);
static void *section_create(void *data);
static void section_destroy(void *data);
static int entry_compare(const void *a, const void *b);
static void *entry_create(void *data);
static void entry_destroy(void *data);
static conf_file_t * get_file_data(int handle);
static int make_path(const char *path, mode_t mode);

static GGZList	*file_list=NULL;


/* conf_read_string()
 *	Search the lists using the specified handle to find the string
 *	stored under section/key.
 *
 *	Returns:
 *	  - ptr to a ggz_malloc()'ed copy of value
 *	  - ptr to a ggz_malloc()'ed copy of default if not found
 *	  - NULL if not found and no default
 */
char * ggz_conf_read_string(int handle, const char *section,
			    const char *key, const char *def)
{
	conf_file_t	*f_data;
	GGZListEntry	*s_entry, *e_entry;
	conf_section_t	*s_data;
	conf_entry_t	e_srch, *e_data;

	/* Find this file entry in our file list */
	if((f_data = get_file_data(handle)) == NULL)
		goto do_default;

	/* Find the requested [Section] */
	s_entry = ggz_list_search(f_data->section_list, (void*)section);
	if(s_entry == NULL)
		goto do_default;
	s_data = ggz_list_get_data(s_entry);

	/* Locate the requested Key */
	e_srch.key = (char*)key;
	e_entry = ggz_list_search(s_data->entry_list, &e_srch);
	if(e_entry == NULL)
		goto do_default;
	e_data = ggz_list_get_data(e_entry);

	/* Duplicate the resulting value and return it to the caller */
	return ggz_strdup(e_data->value);

do_default:
	/* Any failure causes a branch to here to return */
	/* a default value if provided by the caller.	 */
	return ggz_strdup(def);
}


/* conf_read_int()
 *	This is simply a wrapper around read_string() to convert the
 *	string into an integer.
 *
 *	Returns:
 *	  - int value from variable, or def if not found
 */
int ggz_conf_read_int(int handle, const char *section, const char *key, int def)
{
	char	tmp[20], *tmp2;
	int	value;

	sprintf(tmp, "%d", def);
	tmp2 = ggz_conf_read_string(handle, section, key, tmp);
	value = atoi(tmp2);
	ggz_free(tmp2);

	return value;
}


/* conf_read_list()
 *	This is simply a wrapper around read_string() to convert the
 *	string into a list.
 *
 *	Returns:
 *	  - an array and count of entries via the arglist
 *	AND
 *	  - 0 on success
 *	  - -1 on error
 */
int ggz_conf_read_list(int handle, const char *section, const char *key,
		   int *argcp, char ***argvp)
{
	int	index, rc;
	char	*p, *s1, *s2;
	char	*str, *tmp, *tmp2;
	char	saw_space=0, saw_backspace;

	str = ggz_conf_read_string(handle, section, key, NULL);

	if (str != NULL) {
		rc = 0;
		for (*argcp = 1, p = str; *p != '\0'; p++) {
			if (*p == '\\' && *(p+1)) {
				p++;
				if (saw_space) {
					*argcp += 1;
					saw_space = 0;
				}
			} else if (*p == ' ') {
				saw_space = 1;
			} else if (saw_space) {
				*argcp += 1;
				saw_space = 0;
			}
		}

		*argvp = (char **) ggz_malloc((*argcp + 1) * sizeof(char *));
		
		/* Manually null-terminate list. */
		(*argvp)[*argcp] = NULL;


		p = str;
		index = 0;
		do {
			tmp = p;

			for (saw_backspace = 0;
			     *p != '\0' && (saw_backspace ? 1 : (*p != ' '));
			     p++) {
				if (*p == '\\')
					saw_backspace = 1;
				else
					saw_backspace = 0;
			}

			tmp2 = ggz_malloc(p-tmp+1);
			(*argvp)[index] = strncpy(tmp2, tmp, p - tmp);
			tmp2[p-tmp] = '\0';
			s1 = s2 = (*argvp)[index++];

			while (*s1) {
				if (*s1 == '\\')
					s1++;
				if (!*s1) break;
				*s2++ = *s1++;
			}
			*s2 = '\0';

			while (*p && *p == ' ')
				p++;
		} while (*p);

		ggz_free(str);
	} else {
		rc = -1;
		*argcp = 0;
		*argvp = NULL;
	}

	return rc;
}


/* conf_write_string()
 *	Store the value specified into the list structures for the config
 *	file referred to by handle.  Will create a new section and/or
 *	key entry if needed.
 *
 *	Returns:
 *	  - 0 on no error
 *	  - -1 on error
 */
int ggz_conf_write_string(int handle, const char *section,
			  const char *key, const char *value)
{
	conf_file_t	*f_data;
	GGZListEntry	*s_entry;
	conf_section_t	*s_data;
	conf_entry_t	e_data;

	/* Find this file entry in our file list */
	if((f_data = get_file_data(handle)) == NULL)
		return -1;

	/* Is this confio writeable? */
	if(!f_data->writeable) {
		ggz_debug(GGZ_CONF_DEBUG,
			  "ggz_conf_write_string: file is read-only");
		return -1;
	}

	/* Find the requested [Section] */
	s_entry = ggz_list_search(f_data->section_list, (void*)section);
	if(s_entry == NULL) {
		/* We need to create a new [Section] */
		if(ggz_list_insert(f_data->section_list, (void*)section) < 0) {
			ggz_debug(GGZ_CONF_DEBUG,
				  "ggz_conf_write_string: insertion error");
			return -1;
		}
		s_entry = ggz_list_search(f_data->section_list, (void*)section);
	}
	s_data = ggz_list_get_data(s_entry);

	/* Insert the new value into the [Section]'s list */
	e_data.key = (char*)key;
	e_data.value = (char*)value;
	if(ggz_list_insert(s_data->entry_list, &e_data) < 0) {
		ggz_debug(GGZ_CONF_DEBUG,
			  "ggz_conf_write_string: insertion error");
			  
		return -1;
	}

	return 0;
}


/* conf_write_int()
 *	This is simply a wrapper around write_string() to convert the
 *	string into an integer.
 *
 *	Returns:
 *	  - 0 on success
 *	  - -1 on failure
 */
int ggz_conf_write_int(int handle, const char *section, const char *key, int value)
{
	char	tmp[20];

	sprintf(tmp, "%d", value);
	return ggz_conf_write_string(handle, section, key, tmp);
}


/* conf_write_list()
 *	Converts an array of list entries into a text list that read_list()
 *	can parse.  The text is then written using write_string().
 *
 *	Note that the length of the list cannot exceed 1023 characters.  This
 *	limit is set as the maximum line length readable from a config file
 *	is 1024 characters including a carriage return that we don't represent
 *	here.
 *
 *	Returns:
 *	  - 0 on success
 *	  - -1 on failure
 */
int ggz_conf_write_list(int handle, const char *section, 
			const char *key, int argc, char **argv)
{
	int	i;
	char	buf[1023];
	char	*src, *dst, *eob;

	dst = buf;
	eob = buf+1023;
	for(i=0; i<argc; i++) {
		src = argv[i];
		while(*src != '\0') {
			if(*src == ' ') {
				if(dst >= eob)
					return -1;
				*dst++ = '\\';
			}
			if(dst >= eob)
				return -1;
			*dst++ = *src++;
		}
		if(dst >= eob)
			return -1;
		*dst++ = ' ';
	}

	dst--;
	*dst = '\0';

	return ggz_conf_write_string(handle, section, key, buf);
}


/* conf_remove_section()
 *	Removes the specified section from the config file pointed to by
 *	handle.
 *
 *	Returns:
 *	  - 0 on success
 *	  - 1 if [Section] did not exist (soft error)
 *	  - -1 on failure
 */
int ggz_conf_remove_section(int handle, const char *section)
{
	conf_file_t	*f_data;
	GGZListEntry	*s_entry;

	/* Find this file entry in our file list */
	if((f_data = get_file_data(handle)) == NULL)
		return -1;

	/* Is this confio writeable? */
	if(!f_data->writeable) {
		ggz_debug(GGZ_CONF_DEBUG,
			  "ggz_conf_remove_section: file is read-only");
			  
		return -1;
	}

	/* Find the requested [Section] */
	s_entry = ggz_list_search(f_data->section_list, (void*)section);
	if(s_entry == NULL)
		return 1;

	/* Since the entry list will automatically be destroyed, all */
	/* we need to do is remove this section entry */
	ggz_list_delete_entry(f_data->section_list, s_entry);

	return 0;
}


/* conf_remove_key()
 *	Removes the specified key from the config file pointed to by
 *	handle and section.
 *
 *	Returns:
 *	  - 0 on success
 *	  - 1 if [Section] or Key did not exist (soft error)
 *	  - -1 on failure
 */
int ggz_conf_remove_key(int handle, const char *section, const char *key)
{
	conf_file_t	*f_data;
	GGZListEntry	*s_entry, *e_entry;
	conf_section_t	*s_data;
	conf_entry_t	e_data;

	/* Find this file entry in our file list */
	if((f_data = get_file_data(handle)) == NULL)
		return -1;

	/* Is this confio writeable? */
	if(!f_data->writeable) {
		ggz_debug(GGZ_CONF_DEBUG,
			  "ggzcore_confio_remove_key: file is read-only");
		return -1;
	}

	/* Find the requested [Section] */
	s_entry = ggz_list_search(f_data->section_list, (void*)section);
	if(s_entry == NULL)
		return 1;
	s_data = ggz_list_get_data(s_entry);

	/* Find the requested Key */
	e_data.key = (char*)key;
	e_entry = ggz_list_search(s_data->entry_list, &e_data);
	if(e_entry == NULL)
		return 1;

	/* Remove the key's list entry */
	ggz_list_delete_entry(s_data->entry_list, e_entry);

	return 0;
}


/* conf_commit()
 *	This commits any changes that have been made to internal variables
 *	into the on-disk config file.  This should be done as frequently
 *	as the user feels necessary.
 *
 *	Returns:
 *	  - 0 on success
 *	  - -1 on failure
 */
int ggz_conf_commit(int handle)
{
	conf_file_t	*f_data;
	GGZListEntry	*s_entry, *e_entry;
	conf_section_t	*s_data;
	conf_entry_t	*e_data;
	FILE		*c_file;
	int		firstline=1;

	/* Find this file entry in our file list */
	if((f_data = get_file_data(handle)) == NULL)
		return -1;

	/* Is this confio writeable? */
	if(!f_data->writeable) {
		ggz_debug(GGZ_CONF_DEBUG,
			  "ggzcore_confio_commit: file is read-only");
		return -1;
	}

	/* Open our configuration file for writing */
	if((c_file = fopen(f_data->path, "w")) == NULL) {
		ggz_debug(GGZ_CONF_DEBUG, "Unable to write config file %s",
			  f_data->path);
		return -1;
	}

	/* Now step through each section, writing each entry from it's list */
	s_entry = ggz_list_head(f_data->section_list);
	while(s_entry) {
		s_data = ggz_list_get_data(s_entry);

		/* Output a line for our [SectionID] */
		if(firstline) {
			fprintf(c_file, "[%s]\n", s_data->name);
			firstline = 0;
		} else
			fprintf(c_file, "\n[%s]\n", s_data->name);

		/* Bounce through the key entry list */
		e_entry = ggz_list_head(s_data->entry_list);
		while(e_entry) {
			e_data = ggz_list_get_data(e_entry);

			/* Output a line for our Key = Value */
			fprintf(c_file, "%s = %s\n", e_data->key,
						     e_data->value);

			e_entry = ggz_list_next(e_entry);
		}

		s_entry = ggz_list_next(s_entry);
	}

	fclose(c_file);

	return 0;
}


/* conf_cleanup()
 *	This destroys all lists for configuration files including the main
 *	list.  Note that new configuration files may be opened after this
 *	function is called, as confio will reinitialize itself automatically.
 *
 *	WARNING - It might not be immediately recognized, but this function
 *	does NOT commit any changes before freeing up all allocated memory.
 *	The caller is assumed to have committed the config files before using
 *	this routine to clear out old files.
 */
void ggz_conf_cleanup(void)
{
	GGZListEntry	*f_entry;
	conf_file_t	*f_data;

	f_entry = ggz_list_head(file_list);
	while(f_entry) {
		f_data = ggz_list_get_data(f_entry);
		ggz_list_free(f_data->section_list);
		ggz_free(f_data->path);
		ggz_free(f_data);
		f_entry = ggz_list_next(f_entry);
	}

	ggz_list_free(file_list);
	file_list = NULL;
}

/* conf_close(handle)
 * Closes one configuration handle.
 */
void ggz_conf_close(int handle)
{
	conf_file_t *f_data = NULL;
	GGZListEntry *f_entry;

	if(!file_list) return;

	/* Find file entry and list entry in our file list */
	f_entry = ggz_list_head(file_list);
	while(f_entry) {
		f_data = ggz_list_get_data(f_entry);
		if(f_data->handle == handle)
			break;
		f_entry = ggz_list_next(f_entry);
	}
	if(f_entry) {
		ggz_list_delete_entry(file_list, f_entry);

		/* No automatic destroy function defined */
		ggz_list_free(f_data->section_list);
		ggz_free(f_data->path);
		ggz_free(f_data);
	}
}

/* conf_parse(path)
 *	Load up and parse a configuration file into a set of linked lists.
 *
 *	Returns:
 *	  - an integer handle which the caller can use to access the variables
 *	  - -1 on failure
 */
int ggz_conf_parse(const char *path, const GGZConfType options)
{
	static int	next_handle=0;

	conf_file_t	*file_data = NULL;
	GGZList		*section_list;
	GGZListEntry	*file_entry;

	int		opt_create, opt_rdonly, opt_rdwr;
	int		t_file;


	/* Our first time through we need to intialize the file list */
	if(!file_list)
		file_list = ggz_list_create(NULL, NULL, NULL, GGZ_LIST_ALLOW_DUPS);

	/* Check for insane options */
	opt_create = ((options & GGZ_CONF_CREATE) == GGZ_CONF_CREATE);
	opt_rdonly = ((options & GGZ_CONF_RDONLY) == GGZ_CONF_RDONLY);
	opt_rdwr = ((options & GGZ_CONF_RDWR) == GGZ_CONF_RDWR);

	if((opt_rdonly && (opt_rdwr || opt_create)) ||
	   (!opt_rdonly && !opt_rdwr)) {
		ggz_error_msg("ggzcore_conf_parse: Invalid options");
		return -1;
	}

	/* Check to see if the file exists */
	if(access(path, F_OK)) {
		/* Create the file if requested and we can */
		if(opt_create) {
			make_path(path, 0700);
			t_file = open(path, O_RDWR | O_CREAT | O_EXCL,
					    S_IRUSR | S_IWUSR);
			if(t_file != -1)
				close(t_file);
			else {
				ggz_error_sys("Unable to create file %s", path);
				return -1;
			}
		}
	}

	/* Check read or writablity of file */
	if(opt_rdonly && access(path, R_OK)) {
		ggz_error_sys("Unable to read file %s", path);
		return -1;
	}
	if(opt_rdwr && access(path, R_OK | W_OK)) {
		ggz_error_sys("Unable to read or write file %s", path);
		return -1;
	}

	/* See if this path is already opened */
	/* Note this code can easily be fooled by using different */
	/* relative paths to the same file. */
	file_entry = ggz_list_head(file_list);
	while(file_entry) {
		file_data = ggz_list_get_data(file_entry);
		if(!strcmp(file_data->path, path))
			break;
		file_entry = ggz_list_next(file_entry);
	}
	if(file_entry) {
		/* Check if we need to enable writing */
		if(opt_rdwr && !file_data->writeable)
			file_data->writeable = opt_rdwr;

		/* Return existing handle */
		return file_data->handle;
	}

	/* Go do the dirty work and give us a section_list */
	section_list = file_parser(path);
	if(!section_list)
		/* There was an error :( */
		return -1;

	/* Build our file list data entry */
	file_data = ggz_malloc(sizeof(conf_file_t));
	file_data->path = ggz_strdup(path);
	file_data->handle = next_handle;
	file_data->writeable = opt_rdwr;
	file_data->section_list = section_list;

	/* Now store the new list entry */
	if(ggz_list_insert(file_list, file_data) < 0) {
		/* ACK - PUKE */
		ggz_list_free(section_list);
		return -1;
	}

	return next_handle++;
}


/* file_parser()
 *	Opens the file 'path', builds a section list and associated
 *	entry lists for all parsed variable entries, then closes the
 *	file.
 *
 *	Returns:
 *	  - ptr to a section list
 *	  - NULL on failure
 */
static GGZList * file_parser(const char *path)
{
	int		c_file;
	GGZFile		*c_struct;
	char		*line;
	char		*varname, *varvalue;
	int		linenum = 0;
	GGZList		*s_list;
	GGZListEntry	*s_entry;
	conf_section_t	*s_data=NULL;
	conf_entry_t	*e_data=NULL;
	struct stat		st;

	/* Create the section list */
	s_list = ggz_list_create(section_compare,
			      section_create,
			      section_destroy,
			      GGZ_LIST_REPLACE_DUPS);
	if(!s_list)
		return NULL;

	/* Check for file mode */
	if((stat(path, &st)) || (!S_ISREG(st.st_mode))) {
		ggz_error_msg("File %s is not a regular file", path);
		return NULL;
	}

	/* Open the input config file */
	if((c_file = open(path, O_RDONLY)) == -1) {
		/* This should be impossible now, due to checks made earlier? */
		ggz_error_sys("Unable to read file %s", path);
		return NULL;
	}

	/* Get a GGZFile struct */
	c_struct = ggz_get_file_struct(c_file);

	/* Setup some temp storage to use */
	e_data = ggz_malloc(sizeof(conf_entry_t));
	varvalue = NULL;

	/* Read individual lines and pass them off to be parsed */
	while((line = ggz_read_line(c_struct)) != NULL) {
		linenum++;
		parse_line(line, &varname, &varvalue);
		if(varname == NULL) {
			ggz_free(line);
			continue;	/* Blank line or comment */
		}
		if(varvalue == NULL) {
			/* Might be a [SectionID] */
			if(varname[0] == '['
			   && varname[strlen(varname)-1] == ']') {
				/* It is, so setup a new section entry */
				varname[strlen(varname)-1] = '\0';
				varname++;
				if(ggz_list_insert(s_list, varname) < 0)
					ggz_error_sys_exit("list insert error: file_parser");
				s_entry = ggz_list_search(s_list, varname);
				s_data = ggz_list_get_data(s_entry);
			} else
				ggz_error_msg("Syntax error, %s (line %d)",
					 path, linenum);
			ggz_free(line);
			continue;
		}

		/* We have a valid varname/varvalue, add them to entry list */
		if(!s_data) {
			/* We haven't seen a [SectionID] yet :( */
			ggz_error_msg("Syntax error, %s (line %d)",
				 path, linenum);
			ggz_free(line);
			continue;
		}
		e_data->key = varname;
		e_data->value = varvalue;
		if(ggz_list_insert(s_data->entry_list, e_data) < 0)
			ggz_error_sys_exit("list insert error: file_parser");
		ggz_free(line);
	}

	/* Cleanup after ourselves */
	ggz_free(e_data);
	ggz_free_file_struct(c_struct);
	close(c_file);

	return s_list;
}


/* parse_line()
 *	Parses a single line of input into a left half and right half
 *	separated by an optional equals sign.  Pointers to the lhs (varname)
 *	and rhs (varvalue) are returned via the argument list.
 */
static void parse_line(char *p, char **varname, char **varvalue)
{
	char csave, *psave, *sol;

	/* Save start-of-line in sol */
	sol = p;

	*varname = NULL;
	/* Skip over whitespace */
	while((*p == ' ' || *p == '\t' || *p == '\n') && *p != '\0')
		p++;
	if(*p == '\0' || *p == '#')
		return;		/* The line is a comment */

	*varname = p;

	*varvalue = NULL;
	/* Skip until we find an equals sign */
	while(*p != '=' && *p != '\n' && *p != '\0')
		p++;
	csave = *p;
	psave = p;

	if(*p == '=') {
		/* Found '=', now backspace to remove trailing space */
		do {
			p--;
		} while(p >= sol && (*p == ' ' || *p == '\t' || *p == '\n'));
		p++;
	}
	*p = '\0';
	p = psave;
	p++;

	if(**varname == '\0' || p == sol) {
		*varname = NULL;
		return;
	}

	if(csave == '\n' || csave == '\0')
		return;		/* There is no argument */

	/* There appears to be an argument, skip to the start of it */
	while((*p == ' ' || *p == '\t' || *p == '\n' || *p == '=')&& *p != '\0')
		p++;
	if(*p == '\0')
		return;		/* Argument is missing */

	/* There definitely is an argument */
	*varvalue = p;

	/* Terminate it ... */
	while(*p != '\n' && *p != '\0')
		p++;
	/* Found EOL, now backspace over whitespace to remove trailing space */
	p--;
	while(*p == ' ' || *p == '\t' || *p == '\n')
		p--;
	p++;
	/* Finally terminate it with a NUL */
	*p = '\0';	/* Might have already been the NUL, but who cares? */
}


/* get_file_data()
 *	Convenience function to return a pointer to the file data structure
 *	for a specified conf file handle.
 *
 *	Returns:
 *	  - ptr to conf file data structure
 *	  - NULL on error
 */
static conf_file_t * get_file_data(int handle)
{
	GGZListEntry	*f_entry;
	conf_file_t	*f_data=NULL;

	f_entry = ggz_list_head(file_list);
	while(f_entry) {
		f_data = ggz_list_get_data(f_entry);
		if(f_data->handle == handle)
			break;
		f_entry = ggz_list_next(f_entry);
	}
	if(f_entry == NULL) {
		ggz_debug(GGZ_CONF_DEBUG,
			  "get_file_data:  Invalid conf handle requested");
		f_data = NULL;
	}

	return f_data;
}


/* The following three functions perform list comparisons  */
/* and data creation and destruction for the section lists */
static int section_compare(const void *a, const void *b)
{
	/* Note that this function is a little odd since the 'b' passed
	   is not expected to be the full struct, but just the name str.
	   This is because the whole section list is a hack and every
	   time an insertion or lookup is done all that's passed in is the
	   name string.  The callback functions here are expected to do
	   the rest of the work of making the section.  See also
	   section_destroy. */
	const conf_section_t *s_a = a;
	const char *s_b = b;

	return strcmp(s_a->name, s_b);
}

static void *section_create(void *data)
{
	/* Note that this function is a little odd since the data passed
	   is not expected to be the full struct, but just the name str.
	   See the comment in section_compare. */
	conf_section_t	*dst;

	dst = ggz_malloc(sizeof(conf_section_t));

	/* Copy the section name and create an entry list */
	dst->name = ggz_strdup(data);
	dst->entry_list = ggz_list_create(entry_compare,
				       entry_create,
				       entry_destroy,
				       GGZ_LIST_REPLACE_DUPS);
	if(!dst->entry_list) {
		ggz_free(dst->name);
		ggz_free(dst);
		dst = NULL;
	}

	return dst;
}

static void section_destroy(void *data)
{
	conf_section_t	*s_data;

	s_data = data;
	ggz_free(s_data->name);
	ggz_list_free(s_data->entry_list);
	ggz_free(s_data);
}


/* The following three functions perform list comparisons */
/* and data creation and destruction for the entry lists  */
static int entry_compare(const void *a, const void *b)
{
	const conf_entry_t *e_a = a, *e_b = b;

	return strcmp(e_a->key, e_b->key);
}

static void *entry_create(void *data)
{
	conf_entry_t	*src, *dst;

	src = data;
	dst = ggz_malloc(sizeof(conf_entry_t));

	/* Copy the key and value data */
	dst->key = ggz_strdup(src->key);
	dst->value = ggz_strdup(src->value);

	return dst;
}

static void entry_destroy(void *data)
{
	conf_entry_t	*e_data;

	e_data = data;
	ggz_free(e_data->key);
	ggz_free(e_data->value);
	ggz_free(e_data);
}

/* make_path()
 *	Routine to create all directories needed to build 'path'
 */
int make_path(const char *full, mode_t mode)
{
	struct stat stats;
	size_t len = strlen(full) + 1;
	char copy_buf[len], path[len];
	char *copy = copy_buf;

	strcpy(copy, full);
	path[0] = '\0';

	/* Skip preceding / */
	if (copy[0] == '/')
		copy++;

	do {
		char *next = strchr(copy, '/');

		/* If this is the last token, it's the file name - break */
		if (!next) break;
		
		*next = '\0';

		/* While there's still stuff left, it's a directory */
		strcat(strcat(path, "/"), copy);

		if (mkdir(path, mode) < 0
		    && (stat(path, &stats) < 0 || !S_ISDIR(stats.st_mode))) {
			return -1;
		}

		copy = next + 1;
	} while (1);

	return 0;
}


int ggz_conf_get_sections(int handle, int *argcp, char ***argvp)
{
	conf_file_t	*f_data;
	GGZListEntry	*s_entry;
	conf_section_t	*s_data;
	int count=0;
	char **sections=NULL;

	if((f_data = get_file_data(handle)) == NULL)
		return -1;

	s_entry = ggz_list_head(f_data->section_list);
	while(s_entry) {
		s_data = ggz_list_get_data(s_entry);
		sections = ggz_realloc(sections, ++count * sizeof(char *));
		sections[count-1] = ggz_strdup(s_data->name);
		s_entry = ggz_list_next(s_entry);
	}

	*argcp = count;
	*argvp = sections;

	return 0;
}


int ggz_conf_get_keys(int handle, const char *section, int *argcp, char ***argvp)
{
	conf_file_t	*f_data;
	GGZListEntry	*s_entry, *e_entry;
	conf_section_t	*s_data;
	conf_entry_t	*e_data;
	int count=0;
	char **keys=NULL;

	if((f_data = get_file_data(handle)) == NULL)
		return -1;

	/* Find the requested [Section] */
	s_entry = ggz_list_search(f_data->section_list, (void*)section);
	if(s_entry == NULL)
		return -1;
	s_data = ggz_list_get_data(s_entry);

	e_entry = ggz_list_head(s_data->entry_list);
	while(e_entry) {
		e_data = ggz_list_get_data(e_entry);
		keys = ggz_realloc(keys, ++count * sizeof(char *));
		keys[count-1] = ggz_strdup(e_data->key);
		e_entry = ggz_list_next(e_entry);
	}

	*argcp = count;
	*argvp = keys;

	return 0;
}
