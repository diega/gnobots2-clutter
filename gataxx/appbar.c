/* (C) 2003/2004 Sjoerd Langkemper
 * appbar.c - Status bar 
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more details see the file COPYING.
 */


#include <gnome.h>
#include <glib.h>
#include "gataxx.h"
#include "appbar.h"

static GtkWidget * white_score;
static GtkWidget * white_label;
static GtkWidget * black_score;
static GtkWidget * black_label;
static GtkWidget * appbar = NULL;

GtkWidget * appbar_new() {
	GtkWidget * hbox;

	appbar=gnome_appbar_new(FALSE, TRUE, GNOME_PREFERENCES_NEVER);
	hbox=gtk_hbox_new(TRUE, 5);

	white_label=gtk_label_new(_("Light:"));
	gtk_box_pack_start(GTK_BOX(hbox), white_label, FALSE, FALSE, 0);

	white_score=gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), white_score, FALSE, FALSE, 0);

	black_label=gtk_label_new(_("Dark:"));
	gtk_box_pack_start(GTK_BOX(hbox), black_label, FALSE, FALSE, 0);

	black_score=gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), black_score, FALSE, FALSE, 0);

	gtk_widget_show_all(hbox);
	gtk_box_pack_start(GTK_BOX(appbar), hbox, FALSE, FALSE, 0);

	return appbar;	
}
	
void appbar_set_white(int pieces) {
        gchar * buf;
  
	buf = g_strdup_printf ("%d", pieces);
	gtk_label_set_text(GTK_LABEL(white_score), buf); 
	g_free (buf);
}

void appbar_set_black(int pieces) {
	gchar * buf;

	buf = g_strdup_printf ("%d", pieces);
	gtk_label_set_text(GTK_LABEL(black_score), buf); 
	g_free (buf);
}

void appbar_set_status(gchar * status) {
	gnome_appbar_set_status(GNOME_APPBAR(appbar), status);
}

void appbar_set_turn (int player) 
{
  appbar_set_status (player == WHITE ? _("Light's move") : _("Dark's move"));
}
