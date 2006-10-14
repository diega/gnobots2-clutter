/*
 * File: init.c
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 9/15/00
 * $Id$
 *
 * Initialization code
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

#include <ggz.h>

#include "conf.h"
#include "gametype.h"
#include "ggzcore.h"
#include "memory.h"
#include "module.h"
#include "server.h"
#include "net.h"
#include "state.h"

#include <locale.h>
#include <libintl.h>
#include <signal.h>

int ggzcore_init(GGZOptions options)
{
#if 0
	if (options.flags & GGZ_OPT_PARSER) {
		ggz_debug(GGZCORE_DBG_CONF, "Parsing global conf file: %s",
			  options.global_conf);
		ggz_debug(GGZCORE_DBG_CONF, "Parsing user conf file: %s",
			  options.user_conf);
		ggzcore_conf_initialize(options.global_conf,
					options.user_conf);
	}
#endif

	/* This catalog must be preloaded by applications */
	bindtextdomain("ggzcore", PREFIX "/share/locale");

	/* Initialize various systems */
	if (options.flags & GGZ_OPT_MODULES)
		_ggzcore_module_setup();

	if (options.flags & GGZ_OPT_EMBEDDED)
		_ggzcore_module_set_embedded();

	if (options.flags & GGZ_OPT_RECONNECT)
		_ggzcore_server_set_reconnect();

	if (options.flags & GGZ_OPT_THREADED_IO)
		_ggzcore_server_set_threaded_io();

	/* Do not die if child process dies while we're communicating with it */
	signal(SIGPIPE, SIG_IGN);

	return 0;
}


void ggzcore_reload(void)
{
	_ggzcore_module_cleanup();
	_ggzcore_module_setup();
}


void ggzcore_destroy(void)
{
	_ggzcore_module_cleanup();
	ggz_conf_cleanup();
}
