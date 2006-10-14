/*
 * File: module.h
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 12/01/00
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

#ifndef __MODULE_H_
#define __MODULE_H_

int _ggzcore_module_setup(void);
unsigned int _ggzcore_module_get_num(void);

void _ggzcore_module_set_embedded(void);
int _ggzcore_module_is_embedded(void);

/* Returns how many modules support this game and protocol */
int _ggzcore_module_get_num_by_type(const char *game,
				    const char *engine,
				    const char *version);

/* Returns n-th module that supports this game and protocol */
GGZModule *_ggzcore_module_get_nth_by_type(const char *game,
					   const char *engine,
					   const char *version,
					   const unsigned int num);


void _ggzcore_module_cleanup(void);

const char *_ggzcore_module_get_name(const GGZModule * module);
const char *_ggzcore_module_get_version(const GGZModule * module);
const char *_ggzcore_module_get_prot_engine(const GGZModule * module);
const char *_ggzcore_module_get_prot_version(const GGZModule * module);
const char *_ggzcore_module_get_author(const GGZModule * module);
const char *_ggzcore_module_get_frontend(const GGZModule * module);
const char *_ggzcore_module_get_url(const GGZModule * module);
const char *_ggzcore_module_get_icon_path(const GGZModule * module);
const char *_ggzcore_module_get_help_path(const GGZModule * module);
char **_ggzcore_module_get_argv(const GGZModule * module);
GGZModuleEnvironment _ggzcore_module_get_environment(const GGZModule *
						     module);

#endif /* __MODULE_H_ */
