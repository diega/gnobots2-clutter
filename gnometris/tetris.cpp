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
#include <config.h>

GdkImlibImage *image;
GdkPixmap *pix;

int LINES = 20;
int COLUMNS = 11;

int BLOCK_SIZE = 40;

int posx = COLUMNS / 2;
int posy = 0;

int blocknr = 0;
int rot = 0;
int color = 0;

int blocknr_next = -1;
int rot_next = -1;
int color_next = -1;

bool random_block_colors = false;
bool do_preview = true;

Tetris::Tetris(int cmdlLevel)
	: paused(false), timeoutId(-1), onePause(false), setupdialog(NULL), 
		cmdlineLevel(cmdlLevel), doPreviewTmp(do_preview), randomBlocksTmp(random_block_colors)
{
	w = gnome_app_new("gnometris", _("Gnometris"));
  gtk_window_set_policy(GTK_WINDOW(w), FALSE, FALSE, TRUE);
	gtk_signal_connect(GTK_OBJECT(w), "delete_event", (GtkSignalFunc)gameQuit, this);

	GnomeUIInfo game_menu[] = 
	{
		{ GNOME_APP_UI_ITEM, N_("_New"), "_Start a new game", gameNew, this, NULL, 
			GNOME_APP_PIXMAP_STOCK, (gpointer)GNOME_STOCK_MENU_NEW, 'n', GDK_CONTROL_MASK, NULL },
		{ GNOME_APP_UI_ITEM, N_("_Properties..."), NULL, gameProperties, this, NULL,
			GNOME_APP_PIXMAP_STOCK, (gpointer)GNOME_STOCK_MENU_PROP, 0, (GdkModifierType)0, NULL},
		{ GNOME_APP_UI_ITEM, N_("_Scores"), NULL, gameTopTen, this, NULL,
			GNOME_APP_PIXMAP_STOCK, (gpointer)GNOME_STOCK_MENU_SCORES, 0, (GdkModifierType)0, NULL},
		{ GNOME_APP_UI_ITEM, N_("E_xit"), "E_xit Gnometris", gameQuit, this, NULL, 
			GNOME_APP_PIXMAP_STOCK, (gpointer)GNOME_STOCK_MENU_EXIT, 'q', GDK_CONTROL_MASK, NULL },
		GNOMEUIINFO_END
	};

	GnomeUIInfo help_menu[] = 
	{
		{ GNOME_APP_UI_HELP, NULL, NULL, (gpointer)"gnometris", NULL, NULL, GNOME_APP_PIXMAP_NONE, NULL, 0, (GdkModifierType)0, NULL},
		{ GNOME_APP_UI_ITEM, N_("_About Gnometris"), NULL, gameAbout, this, NULL, 
			GNOME_APP_PIXMAP_STOCK, (gpointer)GNOME_STOCK_MENU_ABOUT, 0, (GdkModifierType)0, NULL },
		GNOMEUIINFO_END
	};

	GnomeUIInfo mainmenu[] = 
	{
		{ GNOME_APP_UI_SUBTREE, N_("_Game"), NULL, game_menu, NULL, NULL, GNOME_APP_PIXMAP_DATA, NULL, 0, (GdkModifierType)0, NULL },
		{ GNOME_APP_UI_SUBTREE, N_("_Help"), NULL, help_menu, NULL, NULL, GNOME_APP_PIXMAP_DATA, NULL, 0, (GdkModifierType)0, NULL },
		GNOMEUIINFO_END
	};

	gnome_app_create_menus(GNOME_APP(w), mainmenu);

  GtkWidget * hb = gtk_hbox_new(FALSE, 0);
	gnome_app_set_contents(GNOME_APP(w), hb);

	ops = new BlockOps();
	field = new Field(ops);

	gtk_widget_set_events(w, gtk_widget_get_events(w) | GDK_KEY_PRESS_MASK);
	
	GtkWidget *vb1 = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(vb1), 10);
	gtk_box_pack_start_defaults(GTK_BOX(vb1), field->getWidget());
	gtk_box_pack_start(GTK_BOX(hb), vb1, 0, 0, 0);
	field->show();

	gtk_signal_connect(GTK_OBJECT(w), "event", (GtkSignalFunc)eventHandler, this);
  
	GtkWidget *vb2 = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(vb2), 10);
	gtk_box_pack_end(GTK_BOX(hb), vb2, 0, 0, 0);
	
	preview = new Preview();
	
	gtk_box_pack_start(GTK_BOX(vb2), preview->getWidget(), 0, 0, 0);
	
	preview->show();

	scoreFrame = new ScoreFrame(cmdlineLevel);
	
	gtk_box_pack_end(GTK_BOX(vb2), scoreFrame->getWidget(), 0, 0, 0);

	gtk_widget_show(hb);
	gtk_widget_show(vb1);
	gtk_widget_show(vb2);
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

void 
Tetris::setupdialogDestroy(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	gtk_widget_destroy(t->setupdialog);
	t->setupdialog = NULL;
}

void
Tetris::doSetup(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	t->cmdlineLevel = 0;
  t->startingLevel = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(t->sentry));
	do_preview = t->doPreviewTmp;
	random_block_colors = t->randomBlocksTmp;
	
	gnome_config_set_int("/gnometris/Properties/StartingLevel", t->startingLevel);
	gnome_config_set_int("/gnometris/Properties/DoPreview", do_preview);
	gnome_config_set_int("/gnometris/Properties/RandomBlockColors", random_block_colors);
	gnome_config_sync();
	
	t->scoreFrame->setLevel(t->startingLevel);
	setupdialogDestroy(widget, d);
}

void 
Tetris::setSelectionPreview(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	t->doPreviewTmp = GTK_TOGGLE_BUTTON(widget)->active;
}

void 
Tetris::setSelectionBlocks(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	t->randomBlocksTmp = GTK_TOGGLE_BUTTON(widget)->active;
}

int 
Tetris::gameProperties(GtkWidget *widget, void *d)
{
  GtkWidget *allBoxes;
	GtkWidget *box, *box2;
	GtkWidget *button, *label;
	GtkWidget *frame;
  GtkObject *adj;

	Tetris *t = (Tetris*) d;
	
  if (t->setupdialog) 
		return FALSE;

	t->setupdialog = gtk_window_new(GTK_WINDOW_DIALOG);

	gtk_container_border_width(GTK_CONTAINER(t->setupdialog), 10);
	GTK_WINDOW(t->setupdialog)->position = GTK_WIN_POS_MOUSE;
	gtk_window_set_title(GTK_WINDOW(t->setupdialog), _("Gnometris setup"));
	gtk_signal_connect(GTK_OBJECT(t->setupdialog), "delete_event",
										 GTK_SIGNAL_FUNC(setupdialogDestroy), d);

	allBoxes = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(t->setupdialog), allBoxes);

	frame = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(allBoxes), frame, TRUE, TRUE, 0);
	
	box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), box);

	gtk_widget_show(box);
	gtk_widget_show(frame);

	box2 = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(box2), 10);
	gtk_box_pack_start(GTK_BOX(box), box2, TRUE, TRUE, 0);
	label = gtk_label_new(_("Starting Level:"));
	gtk_box_pack_start(GTK_BOX(box2), label, 0, 0, 0);
	gtk_widget_show(label);

  t->startingLevel = gnome_config_get_int_with_default("/gnometris/Properties/StartingLevel=1", NULL);
	random_block_colors = gnome_config_get_int_with_default("/gnometris/Properties/RandomBlockColors=0", NULL) != 0;
	do_preview = gnome_config_get_int_with_default("/gnometris/Properties/DoPreview=0", NULL) != 0;

  adj = gtk_adjustment_new(t->startingLevel, 1, 10, 1, 5, 10);
	t->sentry = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 10, 0);
  gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(t->sentry),
																		GTK_UPDATE_ALWAYS
#ifndef HAVE_GTK_SPIN_BUTTON_SET_SNAP_TO_TICKS
																		| GTK_UPDATE_SNAP_TO_TICKS
#endif
																		);
#ifdef HAVE_GTK_SPIN_BUTTON_SET_SNAP_TO_TICKS
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(t->sentry), 1);
#endif
  gtk_box_pack_end(GTK_BOX(box2), t->sentry, FALSE, 0, 0);
	gtk_widget_show(t->sentry);
	gtk_widget_show(box2);

	GtkWidget *cb = gtk_check_button_new_with_label(_("Preview next block"));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(cb), do_preview);
	gtk_signal_connect(GTK_OBJECT(cb), "clicked", (GtkSignalFunc)setSelectionPreview, d);
	gtk_box_pack_start(GTK_BOX(box), cb, 0, 0, 0);
	gtk_widget_show(cb);

	cb = gtk_check_button_new_with_label(_("Random block colors"));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(cb), random_block_colors);
	gtk_signal_connect(GTK_OBJECT(cb), "clicked", (GtkSignalFunc)setSelectionBlocks, d);
	gtk_box_pack_start(GTK_BOX(box), cb, 0, 0, 0);
	gtk_widget_show(cb);

	box = gtk_hbox_new(TRUE, 5);
	gtk_box_pack_start(GTK_BOX(allBoxes), box, TRUE, TRUE, 0);
	button = gnome_stock_button(GNOME_STOCK_BUTTON_OK);
	gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(doSetup), d);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 5);
  gtk_widget_show(button);
  button = gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL);
	gtk_signal_connect(GTK_OBJECT(button), "clicked", (GtkSignalFunc)setupdialogDestroy, d);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 5);
  gtk_widget_show(button);
	
	gtk_widget_show(box);
	gtk_widget_show(allBoxes);
	gtk_widget_show(t->setupdialog);
}

int
Tetris::gameQuit(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;

	gtk_widget_destroy(t->w);
	gtk_main_quit();

	return TRUE;
}

void
Tetris::manageFallen()
{
	ops->fallingToLaying();

	int levelBefore = scoreFrame->getLevel();
	ops->checkFullLines(scoreFrame);
	int levelAfter = scoreFrame->getLevel();
	if (levelBefore != levelAfter)
	{
		gtk_timeout_remove(timeoutId);

		int intv = 1000 - 100 * (levelAfter - 1);
		if (intv <= 0)
			intv = 100;
		
		timeoutId = gtk_timeout_add(intv, timeoutHandler, this);
	}
	
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
	
	if (event->type == GDK_KEY_PRESS)
	{
		if (((GdkEventKey*)event)->keyval == GDK_space)
		{
			t->togglePause();
			return TRUE;
		}
	}
	
	if (t->paused)
		return FALSE;
	
	bool res = false;
	bool keyEvent = false;
	
	switch (event->type)
	{
	case GDK_KEY_PRESS: 
	{
		GdkEventKey *e = (GdkEventKey*)event;
		keyEvent = true;
		
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
		default:
			return FALSE;
		}
		break;
	}
	default:
		break;
	}

	if (res)
		gtk_widget_draw(t->field->getWidget(), NULL);

	return (keyEvent == true);
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
		gtk_widget_draw(preview->getWidget(), NULL);
		onePause = true;
	}
	else
	{
		gtk_timeout_remove(timeoutId);
		timeoutId = -1;
		blocknr_next = -1;
		
		endOfGame();
	}
}

void
Tetris::endOfGame()
{
	int pos = gnome_score_log(scoreFrame->getScore(), NULL, TRUE);
	showScores("Gnometris", pos);
}

void
Tetris::showScores(gchar *title, guint pos)
{
	gnome_scores_display(title, "gnometris", NULL, pos);
}

int
Tetris::gameNew(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;

	if (t->timeoutId != -1)
		return FALSE;

  int level = t->cmdlineLevel ? t->cmdlineLevel :
		gnome_config_get_int_with_default("/gnometris/Properties/StartingLevel=1", NULL);
	t->scoreFrame->setLevel(level);
	random_block_colors = gnome_config_get_int_with_default("/gnometris/Properties/RandomBlockColors=0", NULL) != 0;
	do_preview = gnome_config_get_int_with_default("/gnometris/Properties/DoPreview=0", NULL) != 0;
	
	t->timeoutId = gtk_timeout_add(1000 - 100 * (level - 1), timeoutHandler, t);
	t->ops->emptyField();

	t->scoreFrame->resetLines();

	t->ops->generateFallingBlock();
	gtk_widget_draw(t->field->getWidget(), NULL);
	gtk_widget_draw(t->preview->getWidget(), NULL);

	return TRUE;
}

int
Tetris::gameAbout(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;

	GtkWidget *about;

	const gchar *authors[] = {"J. Marcin Gorycki", NULL};

	about = gnome_about_new("Gnometris", TETRIS_VERSION, "(C) 1999 J. Marcin Gorycki", 
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
	t->showScores("Gnometris", 0);

	return TRUE;
}











