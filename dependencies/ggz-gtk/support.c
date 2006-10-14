/*
 * File: support.c
 * Author: Justin Zaun
 * Project: GGZ GTK Client
 * $Id$
 *
 * Support code
 *
 * Copyright (C) 2004 GGZ Development Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
#endif
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include <stdlib.h>

#include <ggzcore.h>
#include <ggz.h>

#include "props.h"
#include "support.h"

GtkWidget *ggz_lookup_widget(GtkWidget * widget, const gchar * widget_name)
{
	GtkWidget *parent, *found_widget;

	found_widget = g_object_get_data(G_OBJECT(widget), widget_name);
	if (found_widget) return found_widget;

	for (;;) {
		if (GTK_IS_MENU(widget))
			parent =
			    gtk_menu_get_attach_widget(GTK_MENU(widget));
		else
			parent = widget->parent;
		if (parent == NULL)
			break;
		widget = parent;
	}

	found_widget = g_object_get_data(G_OBJECT(widget), widget_name);
	if (!found_widget)
		g_warning("Widget not found: %s", widget_name);
	return found_widget;
}

GdkPixbuf *load_pixbuf(const char *name)
{
	char *fullpath;
	GdkPixbuf *image;
	GError *error = NULL;

	fullpath = g_strdup_printf("%s/%s.png", GGZGTKDATADIR, name);
	image = gdk_pixbuf_new_from_file(fullpath, &error);
	if (image == NULL) {
		ggz_error_msg("Can't load pixmap %s", fullpath);
		printf("Can't load pixmap %s.\n", fullpath);
	}
	g_free(fullpath);

	return image;
}

GdkPixbuf *load_svg_pixbuf(const char *name, int width, int height)
{
	char *fullpath;
	GdkPixbuf *image;
	GError *error = NULL;

	fullpath = g_strdup_printf("%s/%s.svg", GGZGTKDATADIR, name);
	image = gdk_pixbuf_new_from_file_at_size(fullpath,
						 width, height,
						 &error);
	if (image == NULL) {
		ggz_error_msg("Can't load pixmap %s", fullpath);
		printf("Can't load SVG %s.\n", fullpath);
		g_free(fullpath);
		return load_pixbuf(name);
	}
	g_free(fullpath);

	return image;
}

char *nocasestrstr(char *text, char *tofind)
{
	char *ret = text, *find = tofind;

	while (1) {
		if (*find == 0)
			return ret;
		if (*text == 0)
			return 0;
		if (toupper(*find) != toupper(*text)) {
			ret = text + 1;
			find = tofind;
		} else
			find++;
		text++;
	}
}

int support_goto_url(gchar * url)
{
	char *command = NULL;
	char *browser_opt;
	char *lynx_opt;

	browser_opt =
	    ggzcore_conf_read_string("OPTIONS", "BROWSER", "None");

	if (!strcmp(browser_opt, "None")) {
		return 0;
	} else if (!strcmp(browser_opt, _("Galeon - New"))) {
		command = g_strdup_printf("galeon %s", url);
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Galeon - Existing"))) {
		command = g_strdup_printf("galeon -w %s", url);
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Gnome URL Handler"))) {
		command = g_strdup_printf("gnome-moz-remote %s", url);
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Konqueror - New"))) {
		command = g_strdup_printf("konqueror %s", url);
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Konqueror - Existing"))) {
		command = g_strdup_printf("konqueror %s", url);
		/*command =
		    g_strdup_printf("dcop konqueror default getWindows");
		support_exec(command);
		g_free(command);
		command =
		    g_strdup_printf
		    ("dcop konqueror konqueror-mainwindow#1 openURL %s",
		     url);
		*/
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Lynx"))) {
		lynx_opt =
		    ggzcore_conf_read_string("OPTIONS", "LYNX", "xterm");
		command = g_strdup_printf("%s -e lynx %s", lynx_opt, url);
		ggz_free(lynx_opt);
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Mozilla - New"))) {
		command = g_strdup_printf("mozilla %s", url);
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Mozilla - Existing"))) {
		command =
		    g_strdup_printf("mozilla -remote 'openURL(%s)'", url);
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Netscape - New"))) {
		command = g_strdup_printf("netscape %s", url);
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Netscape - Existing"))) {
		command =
		    g_strdup_printf("netscape -remote 'openURL(%s)'", url);
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Opera - New"))) {
		command = g_strdup_printf("opera %s", url);
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Opera - Existing"))) {
		command =
		    g_strdup_printf
		    ("opera -remote 'openURL(%s,new-window)'", url);
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Firefox - New"))) {
		command = g_strdup_printf("firefox %s", url);
		support_exec(command);
	} else if (!strcmp(browser_opt, _("Firefox - Existing"))) {
		command =
		    g_strdup_printf("firefox -remote 'openURL(%s)'", url);
		support_exec(command);
	} else {
		return 1;
	}

	ggz_free(browser_opt);

	g_free(command);

	return 1;
}

void support_exec(char *cmd)
{
	g_spawn_command_line_async(cmd, NULL);
}

