/*
 * written by J. Marcin Gorycki <mgo@olicom.dk>
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

#include "tetris.h"
#include "field.h"
#include "blockops.h"
#include "preview.h"
#include "scoreframe.h"

#include <gdk/gdkkeysyms.h>

GdkImlibImage *image;
GdkPixmap *pix;

int score = 0;

int posx = COLUMNS / 2;
int posy = 0;

int blocknr = 0;
int rot = 0;
int color = 0;

int blocknr_next = -1;
int rot_next = -1;
int color_next = -1;

Tetris::Tetris()
	: paused(false), timeoutId(-1), onePause(false)
{
	w = gnome_app_new("gtetris", _("GTetris"));
  gtk_window_set_policy(GTK_WINDOW(w), FALSE, FALSE, TRUE);
	gtk_signal_connect(GTK_OBJECT(w), "delete_event", (GtkSignalFunc)gameQuit, this);

	GnomeUIInfo game_menu[] = 
	{
		{ GNOME_APP_UI_ITEM, N_("_New"), "_Start a new game", gameNew, this, NULL, 
			GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 'n', GDK_CONTROL_MASK, NULL },
		{ GNOME_APP_UI_ITEM, N_("_Scores"), NULL, gameTopTen, this, NULL,
			GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SCORES, 0, 0, NULL},
		{ GNOME_APP_UI_ITEM, N_("E_xit"), "E_xit GTetris", gameQuit, this, NULL, 
			GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 'q', GDK_CONTROL_MASK, NULL },
		GNOMEUIINFO_END
	};

	GnomeUIInfo help_menu[] = 
	{
		{ GNOME_APP_UI_HELP, NULL, NULL, "gtetris", NULL, NULL,GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
		{ GNOME_APP_UI_ITEM, N_("_About GTetris"), NULL, gameAbout, this, NULL, 
			GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT, 0, 0, NULL },
		GNOMEUIINFO_END
	};

	GnomeUIInfo mainmenu[] = 
	{
		{ GNOME_APP_UI_SUBTREE, N_("_Game"), NULL, game_menu, NULL, NULL, GNOME_APP_PIXMAP_DATA, NULL, 0, 0, NULL },
		{ GNOME_APP_UI_SUBTREE, N_("_Help"), NULL, help_menu, NULL, NULL, GNOME_APP_PIXMAP_DATA, NULL, 0, 0, NULL },
		GNOMEUIINFO_END
	};

	gnome_app_create_menus(GNOME_APP(w), mainmenu);

  GtkWidget * hb = gtk_hbox_new(FALSE, 0);
	gnome_app_set_contents(GNOME_APP(w), hb);

	ops = new BlockOps();
	field = new Field(ops);

	gtk_widget_set_events(w, gtk_widget_get_events(w) | GDK_KEY_PRESS_MASK);
	
	gtk_box_pack_start_defaults(GTK_BOX(hb), field->getWidget());
	field->show();
	
	gtk_signal_connect(GTK_OBJECT(w), "event", (GtkSignalFunc)eventHandler, this);
  
	GtkWidget *vb = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(vb), 10);
	gtk_container_add(GTK_CONTAINER(hb), vb);
	
	preview = new Preview();
	
	gtk_box_pack_start(GTK_BOX(vb), preview->getWidget(), 0, 0, 0);
	
	preview->show();

	scoreFrame = new ScoreFrame();
	
	gtk_box_pack_end(GTK_BOX(vb), scoreFrame->getWidget(), 1, 1, 0);

	gtk_widget_show(hb);
	gtk_widget_show(vb);
	scoreFrame->show();
	gtk_widget_show(w);
}

Tetris::~Tetris()
{
	delete ops;
	delete field;
	delete preview;
	delete scoreFrame;
}

int
Tetris::gameQuit(GtkWidget *widget, void *d)
{
  gtk_exit(0);
	return TRUE;
}

void
Tetris::manageFallen()
{
	ops->fallingToLaying();
	
	ops->checkFullLines();
	scoreFrame->setScore(score);
	scoreFrame->setLines(score);
	
	generate();
}

int
Tetris::timeoutHandler(void *d)
{
	Tetris *t = (Tetris*) d;
	
	if (t->paused)
		return 1;

 	if (t->onePause)
 	{
		t->onePause = false;
		gtk_widget_draw(t->field->getWidget(), NULL);
	}
 	else
	{
		bool res = t->ops->moveBlockDown();
		
		gtk_widget_draw(t->field->getWidget(), NULL);

		if (res)
			t->manageFallen();
	}

	return 1;	
}

gint
Tetris::eventHandler(GtkWidget *widget, GdkEvent *event, void *d)
{
	Tetris *t = (Tetris*) d;
	
	if (t->timeoutId == -1)
		return FALSE;
	
	bool res = false;
	
	switch (event->type)
	{
	case GDK_KEY_PRESS:
		GdkEventKey *e = (GdkEventKey*)event;
		switch(e->keyval)
		{
		case GDK_Left:
			res = t->ops->moveBlockLeft();
			break;
		case GDK_Right:
			res = t->ops->moveBlockRight();
			break;
		case GDK_Up:
			res = t->ops->rotateBlock();
			break;
		case GDK_Down:
			t->ops->dropBlock();
			res = true;
			t->manageFallen();
			break;
		case GDK_space:
			t->togglePause();
			return TRUE;			
		}
	}

	if (res)
		gtk_widget_draw(t->field->getWidget(), NULL);

	return (res == true);
}

void
Tetris::togglePause()
{
	paused = !paused;
}

void
Tetris::generate()
{
	if (ops->generateFallingBlock())
	{
		ops->putBlockInField(false);
		preview->putBlock();
		gtk_widget_draw(preview->getWidget(), NULL);
		onePause = true;
	}
	else
	{
		gtk_timeout_remove(timeoutId);
		timeoutId = -1;
		endOfGame();
	}
}

void
Tetris::endOfGame()
{
	int pos = gnome_score_log(score, NULL, TRUE);
	showScores(_("GTetris"), pos);
}

void
Tetris::showScores(gchar *title, guint pos)
{
	gnome_scores_display(title, "gtetris", NULL, pos);
}

int
Tetris::gameNew(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	
	t->timeoutId = gtk_timeout_add(300, timeoutHandler, t);
	t->ops->emptyField();
	t->preview->clear();

	score = 0;
	t->scoreFrame->setScore(score);
	t->scoreFrame->setLines(score);
	t->scoreFrame->setLevel(0);

	t->ops->generateFallingBlock();
	t->preview->putBlock();
	gtk_widget_draw(t->field->getWidget(), NULL);
	gtk_widget_draw(t->preview->getWidget(), NULL);
}

int
Tetris::gameAbout(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;

	GtkWidget *about;

	const gchar *authors[] = {"J. Marcin Gorycki", NULL};

	about = gnome_about_new(_("GTetris"), TETRIS_VERSION, "(C) 1999 J. Marcin Gorycki", 
													(const char **)authors,
													_("Written for my wife, Matylda\n"
														"Send comments and bug reports to: mgo@olicom.dk"), NULL);
	gnome_dialog_set_parent(GNOME_DIALOG(about), GTK_WINDOW(t->w));
	gtk_window_set_modal(GTK_WINDOW(about), TRUE);

	gtk_widget_show(about);

	return TRUE;
}

int
Tetris::gameTopTen(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	t->showScores(_("GTetris"), 0);

	return TRUE;
}











