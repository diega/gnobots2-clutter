/*
 * File: module.c
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 11/23/00
 * $Id$
 *
 * This fils contains functions for handling client-side game modules
 *
 * Copyright (C) 2000 Brent Hendricks.
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
#  include <config.h>	/* Site-specific config */
#endif

#include <stdlib.h>
#include <string.h>

#include <ggz.h>

#include "game.h"
#include "module.h"

#define GGZ_MOD_RC "ggz.modules"


/* Structure describing particular client-side game module */
struct _GGZModule {

	/* Name of module */
	const char *name;

	/* Game module version */
	const char *version;

	/* Protocol engine implemented */
	const char *prot_engine;

	/* Protocol version implemented */
	const char *prot_version;

	/* Supported games */
	char **games;

	/* Module author */
	const char *author;

	/* Native frontend */
	const char *frontend;

	/* Hopepage for this module */
	const char *url;

	/* Commandline for executing module */
	char **argv;

	/* Path to icon for this game module */
	const char *icon;

	/* Path to help file */
	const char *help;

	/* Preferred runtime environment */
	GGZModuleEnvironment environment;
};

/* List of modules */
static GGZList *module_list;
static unsigned int num_modules;
static int mod_handle = -1;
static int embedded_module = 0;

/* static internal functions */
static GGZModule *_ggzcore_module_new(void);
#if 0
static void _ggzcore_module_init(GGZModule * module,
				 const char *name,
				 const char *version,
				 const char *prot_engine,
				 const char *prot_version,
				 const char *author,
				 const char *frontend,
				 const char *url,
				 const char *exec_path,
				 const char *icon_path,
				 const char *help_path);
#endif /* #if 0 */
static void _ggzcore_module_free(GGZModule * module);
static void _ggzcore_module_read(GGZModule * mod, char *id);
static int _ggzcore_module_add(GGZModule * module);

static char *_ggzcore_module_conf_filename(void);
static void _ggzcore_module_print(const GGZModule *);
static void _ggzcore_module_list_print(void);
/* Utility functions used by ggz_list */
static void _ggz_free_chars(char **argv);
static int _ggzcore_module_compare(const void *p, const void *q);
#if 0
static void *_ggzcore_module_create(void *p);
#endif /* #if 0 */
static void _ggzcore_module_destroy(void *p);


/* Publicly exported functions */


/* This returns the number of registered modules */
unsigned int ggzcore_module_get_num(void)
{
	return _ggzcore_module_get_num();
}


/* Returns how many modules support this game and protocol */
int ggzcore_module_get_num_by_type(const char *game,
				   const char *engine, const char *version)
{
	/* A NULL version means any version */
	if (!game || !engine)
		return -1;

	return _ggzcore_module_get_num_by_type(game, engine, version);
}


/* Returns n-th module that supports this game and protocol */
GGZModule *ggzcore_module_get_nth_by_type(const char *game,
					  const char *engine,
					  const char *version,
					  const unsigned int num)
{
	/* FIXME: should check bounds on num */
	if (!game || !engine || !version)
		return NULL;

	return _ggzcore_module_get_nth_by_type(game, engine, version, num);
}


/* This adds a local module to the list.  It returns 0 if successful or
   -1 on failure. */
int ggzcore_module_add(const char *name,
		       const char *version,
		       const char *prot_engine,
		       const char *prot_version,
		       const char *author,
		       const char *frontend,
		       const char *url,
		       const char *exe_path,
		       const char *icon_path,
		       const char *help_path,
		       GGZModuleEnvironment environment)
{
	return -1;
}


/* These functions lookup a particular property of a module.  I've added
   icon to the list we discussed at the meeting.  This is an optional xpm
   file that the module can provide to use for representing the game
   graphically.*/
const char *ggzcore_module_get_name(GGZModule * module)
{
	if (!module)
		return NULL;

	return _ggzcore_module_get_name(module);
}


const char *ggzcore_module_get_version(GGZModule * module)
{
	if (!module)
		return NULL;

	return _ggzcore_module_get_version(module);
}


const char *ggzcore_module_get_prot_engine(GGZModule * module)
{
	if (!module)
		return NULL;

	return _ggzcore_module_get_prot_engine(module);
}


const char *ggzcore_module_get_prot_version(GGZModule * module)
{
	if (!module)
		return NULL;

	return _ggzcore_module_get_prot_version(module);
}


const char *ggzcore_module_get_author(GGZModule * module)
{
	if (!module)
		return NULL;

	return _ggzcore_module_get_author(module);
}


const char *ggzcore_module_get_frontend(GGZModule * module)
{
	if (!module)
		return NULL;

	return _ggzcore_module_get_frontend(module);
}


const char *ggzcore_module_get_url(GGZModule * module)
{
	if (!module)
		return NULL;

	return _ggzcore_module_get_url(module);
}


const char *ggzcore_module_get_icon_path(GGZModule * module)
{
	if (!module)
		return NULL;

	return _ggzcore_module_get_icon_path(module);
}


const char *ggzcore_module_get_help_path(GGZModule * module)
{
	if (!module)
		return NULL;

	return _ggzcore_module_get_help_path(module);
}


char **ggzcore_module_get_argv(GGZModule * module)
{
	if (!module)
		return NULL;

	return _ggzcore_module_get_argv(module);
}


GGZModuleEnvironment ggzcore_module_get_environment(GGZModule * module)
{
	if (!module)
		return GGZ_ENVIRONMENT_PASSIVE;

	return _ggzcore_module_get_environment(module);
}

/* Internal library functions (prototypes in module.h) */


/* _ggzcore_module_setup()
 *	Opens the global game module file
 *
 *	Returns:
 *	  -1 on error
 *	  0 on success
 */
int _ggzcore_module_setup(void)
{
	char *file;
	char **ids;
	char **games;
	int i, j, count_types, count_modules, status;
	GGZModule *module;

	if (mod_handle != -1) {
		ggz_debug(GGZCORE_DBG_MODULE,
			  "module_setup() called twice");
		return -1;
	}


	module_list = ggz_list_create(_ggzcore_module_compare, NULL,
				      _ggzcore_module_destroy, 0);
	num_modules = 0;

	file = _ggzcore_module_conf_filename();
	ggz_debug(GGZCORE_DBG_MODULE, "Reading %s", file);
	mod_handle = ggz_conf_parse(file, GGZ_CONF_RDONLY);
	/* Free up space taken by name */
	ggz_free(file);

	if (mod_handle == -1) {
		ggz_debug(GGZCORE_DBG_MODULE,
			  "Unable to load module conffile");
		return -1;
	}

	/* Read in list of supported gametypes */
	status = ggz_conf_read_list(mod_handle, "Games", "*Engines*",
				    &count_types, &games);
	if (status < 0) {
		ggz_debug(GGZCORE_DBG_MODULE, "Couldn't read engine list");
		return -1;
	}
	ggz_debug(GGZCORE_DBG_MODULE,
		  "%d game engines supported", count_types);

	for (i = 0; i < count_types; i++) {
		status = ggz_conf_read_list(mod_handle, "Games", games[i],
					    &count_modules, &ids);


		ggz_debug(GGZCORE_DBG_MODULE,
			  "%d modules for %s", count_modules, games[i]);

		for (j = 0; j < count_modules; j++) {
			module = _ggzcore_module_new();
			_ggzcore_module_read(module, ids[j]);
			_ggzcore_module_add(module);
			ggz_debug(GGZCORE_DBG_MODULE, "Module %d: %s", j,
				  ids[j]);

		}

		_ggz_free_chars(ids);
	}

	_ggz_free_chars(games);

	_ggzcore_module_list_print();

	return 0;
}


unsigned int _ggzcore_module_get_num(void)
{
	return num_modules;
}


void _ggzcore_module_set_embedded(void)
{
	embedded_module = 1;
}


int _ggzcore_module_is_embedded(void)
{
	return embedded_module;
}


/* FIXME: do this right.  We should parse through module_list not
   re-read the config file */
int _ggzcore_module_get_num_by_type(const char *game,
				    const char *engine,
				    const char *version)
{
	int count, status, i, numcount;
	char **ids;
	GGZModule module;

	/* Get total count for this engine (regardless of version) */
	status =
	    ggz_conf_read_list(mod_handle, "Games", engine, &count, &ids);

	if (status < 0)
		return 0;

	numcount = count;
	for (i = 0; i < count; i++) {
		_ggzcore_module_read(&module, ids[i]);
		/* Subtract out modules that aren't the same protocol */
		if (ggz_strcmp(engine, module.prot_engine) != 0
		    || (version
			&& ggz_strcmp(version, module.prot_version) != 0)
		    /* || game not included in game list */
		    )
			numcount--;
	}

	_ggz_free_chars(ids);


	return numcount;
}


/* FIXME: do this right.  We should parse through module_list not
   re-read the config file */
GGZModule *_ggzcore_module_get_nth_by_type(const char *game,
					   const char *engine,
					   const char *version,
					   const unsigned int num)
{
	int i, total, status, count;
	char **ids;
	GGZModule *module, *found = NULL;
	GGZListEntry *entry;

	status =
	    ggz_conf_read_list(mod_handle, "Games", engine, &total, &ids);

	ggz_debug(GGZCORE_DBG_MODULE, "Found %d modules matching %s",
		  total, engine);

	if (status < 0)
		return NULL;

	if (num >= total) {
		_ggz_free_chars(ids);
		return NULL;
	}

	count = 0;
	for (i = 0; i < total; i++) {
		module = _ggzcore_module_new();
		_ggzcore_module_read(module, ids[i]);
		if (ggz_strcmp(version, module->prot_version) == 0) {
			/* FIXME:  also check to see if 'game' is in supported list */
			if (count++ == num) {
				/* Now find same module in list */
				entry =
				    ggz_list_search(module_list, module);
				found = ggz_list_get_data(entry);
				_ggzcore_module_free(module);
				break;
			}
		}
		_ggzcore_module_free(module);
	}

	/* Free the rest of the ggz_conf memory */
	_ggz_free_chars(ids);


	/* Return found module (if any) */
	return found;
}


const char *_ggzcore_module_get_name(const GGZModule * module)
{
	return module->name;
}


const char *_ggzcore_module_get_version(const GGZModule * module)
{
	return module->version;
}


const char *_ggzcore_module_get_prot_engine(const GGZModule * module)
{
	return module->prot_engine;
}


const char *_ggzcore_module_get_prot_version(const GGZModule * module)
{
	return module->prot_version;
}


const char *_ggzcore_module_get_author(const GGZModule * module)
{
	return module->author;
}


const char *_ggzcore_module_get_frontend(const GGZModule * module)
{
	return module->frontend;
}


const char *_ggzcore_module_get_url(const GGZModule * module)
{
	return module->url;
}


const char *_ggzcore_module_get_icon_path(const GGZModule * module)
{
	return module->icon;
}


const char *_ggzcore_module_get_help_path(const GGZModule * module)
{
	return module->help;
}


char **_ggzcore_module_get_argv(const GGZModule * module)
{
	return module->argv;
}


GGZModuleEnvironment _ggzcore_module_get_environment(const GGZModule *
						     module)
{
	return module->environment;
}


void _ggzcore_module_cleanup(void)
{
	if (module_list)
		ggz_list_free(module_list);
	num_modules = 0;

	ggz_conf_close(mod_handle);
	mod_handle = -1;
}


/* Static functions internal to this file */

static GGZModule *_ggzcore_module_new(void)
{
	GGZModule *module;

	module = ggz_malloc(sizeof(*module));

	return module;
}

#if 0
static void _ggzcore_module_init(GGZModule * module,
				 const char *name,
				 const char *version,
				 const char *prot_engine,
				 const char *prot_version,
				 const char *author,
				 const char *frontend,
				 const char *url,
				 const char *exec_path,
				 const char *icon_path,
				 const char *help_path)
{
	module->name = ggz_strdup(name);
	module->version = ggz_strdup(version);
	module->prot_engine = ggz_strdup(prot_engine);
	module->prot_version = ggz_strdup(prot_version);
	module->author = ggz_strdup(author);
	module->frontend = ggz_strdup(frontend);
	module->url = ggz_strdup(url);
	/* module->path = ggz_strdup(exec_path); */
	module->icon = ggz_strdup(icon_path);
	module->help = ggz_strdup(help_path);
}
#endif /* #if 0 */


static void _ggzcore_module_free(GGZModule * module)
{

	if (module->name)
		ggz_free(module->name);
	if (module->version)
		ggz_free(module->version);
	if (module->prot_engine)
		ggz_free(module->prot_engine);
	if (module->prot_version)
		ggz_free(module->prot_version);
	if (module->author)
		ggz_free(module->author);
	if (module->frontend)
		ggz_free(module->frontend);
	if (module->url)
		ggz_free(module->url);
	if (module->icon)
		ggz_free(module->icon);
	if (module->help)
		ggz_free(module->help);
	if (module->games)
		_ggz_free_chars(module->games);
	if (module->argv)
		_ggz_free_chars(module->argv);

	ggz_free(module);
}


static int _ggzcore_module_add(GGZModule * module)
{
	int status;

	if ((status = ggz_list_insert(module_list, module)) == 0)
		num_modules++;

	return status;
}


static char *_ggzcore_module_conf_filename(void)
{
	char *filename;
	int new_len;

	/* Allow for extra slash and newline when concatenating */
	new_len = strlen(GGZCONFDIR) + strlen(GGZ_MOD_RC) + 2;
	filename = ggz_malloc(new_len);

	strcpy(filename, GGZCONFDIR);
	strcat(filename, "/");
	strcat(filename, GGZ_MOD_RC);

	return filename;
}


static void _ggzcore_module_read(GGZModule * mod, char *id)
{
	int argc;
	char *environment;
	/* FIXME: check for errors on all of these */

	/* Note: the memory allocated here is freed in _ggzcore_module_free */
	mod->name = ggz_conf_read_string(mod_handle, id, "Name", NULL);
	mod->version =
	    ggz_conf_read_string(mod_handle, id, "Version", NULL);
	mod->prot_engine =
	    ggz_conf_read_string(mod_handle, id, "ProtocolEngine", NULL);
	mod->prot_version =
	    ggz_conf_read_string(mod_handle, id, "ProtocolVersion", NULL);
	ggz_conf_read_list(mod_handle, id, "SupportedGames", &argc,
			   &mod->games);
	mod->author = ggz_conf_read_string(mod_handle, id, "Author", NULL);
	mod->frontend =
	    ggz_conf_read_string(mod_handle, id, "Frontend", NULL);
	mod->url = ggz_conf_read_string(mod_handle, id, "Homepage", NULL);
	ggz_conf_read_list(mod_handle, id, "CommandLine", &argc,
			   &mod->argv);
	mod->icon = ggz_conf_read_string(mod_handle, id, "IconPath", NULL);
	mod->help = ggz_conf_read_string(mod_handle, id, "HelpPath", NULL);

	environment =
	    ggz_conf_read_string(mod_handle, id, "Environment", NULL);
	if (!environment)
		mod->environment = GGZ_ENVIRONMENT_XWINDOW;
	else if (!ggz_strcmp(environment, "xwindow"))
		mod->environment = GGZ_ENVIRONMENT_XWINDOW;
	else if (!ggz_strcmp(environment, "xfullscreen"))
		mod->environment = GGZ_ENVIRONMENT_XFULLSCREEN;
	else if (!ggz_strcmp(environment, "passive"))
		mod->environment = GGZ_ENVIRONMENT_PASSIVE;
	else if (!ggz_strcmp(environment, "console"))
		mod->environment = GGZ_ENVIRONMENT_CONSOLE;
	else
		mod->environment = GGZ_ENVIRONMENT_XWINDOW;
	if (environment)
		ggz_free(environment);
}


static void _ggzcore_module_print(const GGZModule * module)
{
	int i = 0;

	ggz_debug(GGZCORE_DBG_MODULE, "Name: %s", module->name);
	ggz_debug(GGZCORE_DBG_MODULE, "Version: %s", module->version);
	ggz_debug(GGZCORE_DBG_MODULE, "ProtocolEngine: %s",
		  module->prot_engine);
	ggz_debug(GGZCORE_DBG_MODULE, "ProtocolVersion: %s",
		  module->prot_version);
	if (module->games)
		while (module->games[i]) {
			ggz_debug(GGZCORE_DBG_MODULE, "Game[%d]: %s", i,
				  module->games[i]);
			++i;
		}

	ggz_debug(GGZCORE_DBG_MODULE, "Author: %s", module->author);
	ggz_debug(GGZCORE_DBG_MODULE, "Frontend: %s", module->frontend);
	ggz_debug(GGZCORE_DBG_MODULE, "URL: %s", module->url);
	ggz_debug(GGZCORE_DBG_MODULE, "Icon: %s", module->icon);
	ggz_debug(GGZCORE_DBG_MODULE, "Help: %s", module->help);
	while (module->argv && module->argv[i]) {
		ggz_debug(GGZCORE_DBG_MODULE, "Argv[%d]: %s", i,
			  module->argv[i]);
		++i;
	}
}


static void _ggzcore_module_list_print(void)
{
	GGZListEntry *cur;

	for (cur = ggz_list_head(module_list); cur;
	     cur = ggz_list_next(cur))
		_ggzcore_module_print(ggz_list_get_data(cur));
}


/* Utility functions used by ggz_list */

static void _ggz_free_chars(char **argv)
{
	int i;

	for (i = 0; argv[i]; i++)
		ggz_free(argv[i]);

	ggz_free(argv);
}


/* Match game module by 'name', 'prot_engine', 'prot_version' */
static int _ggzcore_module_compare(const void *p, const void *q)
{
	int compare;
	const GGZModule *pmod = p, *qmod = q;

	compare = ggz_strcmp(pmod->name, qmod->name);
	if (compare != 0)
		return compare;

	compare = ggz_strcmp(pmod->prot_engine, qmod->prot_engine);
	if (compare != 0)
		return compare;

	compare = ggz_strcmp(pmod->prot_version, qmod->prot_version);
	if (compare != 0)
		return compare;

	compare = ggz_strcmp(pmod->frontend, qmod->frontend);

	return compare;
}

#if 0
static void *_ggzcore_module_create(void *p)
{
	GGZModule *new, *src = p;

	new = _ggzcore_module_new();

	_ggzcore_module_init(new, src->name, src->version,
			     src->prot_engine, src->prot_version,
			     src->author, src->frontend, src->url,
			     src->argv[0], src->icon, src->help);


	return new;
}
#endif /* #if 0 */

static void _ggzcore_module_destroy(void *p)
{
	/* Quick sanity check */
	if (!p)
		return;

	_ggzcore_module_free(p);
}
