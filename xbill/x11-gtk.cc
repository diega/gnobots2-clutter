#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "objects.h"
#include "x11.h"

#include <gtk/gtk.h>

extern GtkWidget *toplevel, *menubar, *field;
extern GtkWidget *aboutbox, *rulesbox, *storybox;
extern GtkWidget *warpbox, *quitbox, *newgamebox, *pausebox;
extern GtkWidget *scorebox, *endgamebox;

static void display_scores();
static GtkWidget *CreateDialogIcon (char *title, int buttonmask,
		GdkPixmap *icon, const char *text, const char *buttonlabel,
		GtkSignalFunc callback);

#ifndef GNOMEUIINFO_ITEM_STOCK_DATA
#define GNOMEUIINFO_ITEM_STOCK_DATA(label, tip, cb, data, xpm) \
                   {GNOME_APP_UI_ITEM, label, tip, cb, data, NULL, \
                    GNOME_APP_PIXMAP_STOCK, xpm, 0, (GdkModifierType)0, NULL}
#endif
#define GNOMEUIINFO_ITEM_NONE_DATA(label, tip, cb, data) \
                   {GNOME_APP_UI_ITEM, label, tip, cb, data, NULL, \
	            GNOME_APP_PIXMAP_NONE, NULL, 0, (GdkModifierType)0, NULL}
static GnomeUIInfo game_menu[] = {
  GNOMEUIINFO_MENU_NEW_GAME_ITEM((void *) popup, &newgamebox),
  GNOMEUIINFO_MENU_PAUSE_GAME_ITEM((void *) popup, &pausebox),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_ITEM_STOCK_DATA(_("Warp to level..."), NULL, (void *) popup, &warpbox,
			      GTK_STOCK_GO_FORWARD),
  GNOMEUIINFO_MENU_SCORES_ITEM((void *) display_scores, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_QUIT_ITEM((void *) popup, &quitbox),
  GNOMEUIINFO_END
};
  
static GnomeUIInfo help_menu[] = {
  GNOMEUIINFO_HELP ("gnome-xbill"),
  GNOMEUIINFO_MENU_ABOUT_ITEM((void *) popup, &aboutbox),
  GNOMEUIINFO_END
};
static GnomeUIInfo menus[] = {
  GNOMEUIINFO_MENU_GAME_TREE(game_menu),
  GNOMEUIINFO_MENU_HELP_TREE(help_menu),
  GNOMEUIINFO_END
};
GtkWidget *pausebutton = NULL;

void CreateMenuBar(GtkWidget *app) {
	gnome_app_create_menus(GNOME_APP(app), menus);
	pausebutton = game_menu[1].widget;
}

static gint  popdown(GtkWidget *w) {
  gtk_main_quit();
  return TRUE; /* don't kill window */
}

GtkWidget *CreatePixmapBox(char *title, GdkPixmap *pixmap, const char *text) {
        GtkWidget *win, *hbox, *vbox, *wid;

	win = gtk_dialog_new_with_buttons(title,
			NULL,
			(GtkDialogFlags)0,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL);
	g_signal_connect(GTK_OBJECT(win), "destroy",
			   GTK_SIGNAL_FUNC(popdown), NULL);
	g_signal_connect(GTK_OBJECT(win), "response",
			   GTK_SIGNAL_FUNC(popdown), NULL);
	vbox = GTK_DIALOG(win)->vbox;

	wid = gtk_image_new_from_pixmap(game.logo.pix, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, TRUE, 0);
	gtk_widget_show(wid);

	if (pixmap) {
	        wid = gtk_image_new_from_pixmap(pixmap, NULL);
		gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, TRUE, 0);
		gtk_widget_show(wid);
	}
	if (text) {
	        wid = gtk_label_new(text);
		gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, TRUE, 0);
		gtk_widget_show(wid);
	}

	return win;
}

void warp_apply(GtkDialog *b, gint arg1, GtkEntry *entry) {
	if (arg1 == GTK_RESPONSE_CANCEL)
		return;

        game.warp_to_level(atoi(gtk_entry_get_text(entry)));
}

GtkWidget *CreateEnterText (char *title, const char *text,
			    GtkSignalFunc callback) {
        GtkWidget *win, *vbox, *entry, *wid;

	win = gtk_dialog_new_with_buttons(title,
			NULL,
			(GtkDialogFlags)0,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(win), GTK_RESPONSE_OK);
	g_signal_connect(GTK_OBJECT(win), "destroy",
			   GTK_SIGNAL_FUNC(popdown), NULL);
	g_signal_connect(GTK_OBJECT(win), "response",
			   GTK_SIGNAL_FUNC(popdown), NULL);
	vbox = GTK_DIALOG(win)->vbox;

	wid = gtk_label_new(text);
	gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, TRUE, 0);
	gtk_widget_show(wid);

	entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, TRUE, 0);
	gtk_widget_show(entry);

	g_signal_connect(win, "response", callback, entry);
	return win;
}

GtkWidget *CreateDialog (char *title, int buttonmask, GdkPixmap *icon,
			 const char *text, const char *buttonlabel,
			 GtkSignalFunc callback) {
	return CreateDialogIcon (title, buttonmask, icon,
				 text, buttonlabel,
				 callback);
}

static
GtkWidget *CreateDialogIcon (char *title, int buttonmask, GdkPixmap *icon,
		const char *text, const char *buttonlabel,
		GtkSignalFunc callback)
{
	GtkWidget *win, *hbox, *wid, *button;

	win = gtk_dialog_new();

	gtk_window_set_title (GTK_WINDOW(win), title);
	g_signal_connect(GTK_OBJECT(win), "destroy",
			GTK_SIGNAL_FUNC(popdown), NULL);
	g_signal_connect(GTK_OBJECT(win), "response",
			GTK_SIGNAL_FUNC(popdown), NULL);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(win)->vbox), hbox,
			TRUE, TRUE, 0);
	gtk_widget_show(hbox);

	if (icon != NULL) {
		wid = gtk_image_new_from_pixmap(icon, NULL);
		gtk_box_pack_start(GTK_BOX(hbox), wid, FALSE, TRUE, 0);
		gtk_widget_show(wid);
	}

	wid = gtk_label_new(text);
	g_object_set_data(G_OBJECT(win), "key", wid);
	gtk_box_pack_start(GTK_BOX(hbox), wid, FALSE, TRUE, 0);
	gtk_widget_show(wid);

	if (buttonmask&OK) {
		gtk_dialog_add_buttons(GTK_DIALOG(win),
				GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
		if (callback)
			g_signal_connect(GTK_DIALOG(win), "response",
					callback, NULL);
	}
	if (buttonmask&CANCEL) {
		gtk_dialog_add_buttons(GTK_DIALOG(win),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	}
	gtk_dialog_set_default_response(GTK_DIALOG(win), GTK_RESPONSE_OK);
	return win;
}

void UI::update_hsbox(char *str) {
        /* this is no longer needed (because of gnome_scores) */
}

void UI::update_scorebox(int level, int score) {
        GtkLabel *label;
	char *str;

	label = GTK_LABEL(g_object_get_data(G_OBJECT(scorebox), "key"));
	str = g_strdup_printf (_("After Level %d:     \nYour score: %d"), level, score);
	gtk_label_set_text (label, str);
	g_free (str);
}

void show_scores(int pos) {
  gnome_scores_display(_("Gnome xBill Scores"), "gnome-xbill", NULL, pos);
}

static void display_scores() {
  show_scores(0);
}

/**********************/
/* Callback functions */
/**********************/

void new_game_cb (GtkDialog *dialog, gint arg1, gpointer user_data) {
	if (arg1 == GTK_RESPONSE_CANCEL)
		return;
	game.start(1);
}

void quit_game_cb (GtkDialog *dialog, gint arg1, gpointer user_data) {
	if (arg1 == GTK_RESPONSE_CANCEL)
		return;
	game.quit();
}

/*void get_coords (guint *x, guint *y) {
	XWindowAttributes wattr;
	Window junk;
	int rx, ry;
	XGetWindowAttributes (ui.display, ui.window, &wattr);
	XTranslateCoordinates (ui.display, ui.window, wattr.root,
		-wattr.border_width, -wattr.border_width, &rx, &ry, &junk);
	*x=rx+20;
	*y=ry+40;
}
*/

void popup (GtkWidget *mi, GtkWidget **box) {
  ui.pause_game();
  gtk_widget_show_all(*box);
  gtk_grab_add(*box);
  gtk_main();
  gtk_grab_remove(*box);
  gtk_widget_hide(*box);
  ui.resume_game();
}

/******************/
/* Event handlers */
/******************/
void leave_window_eh(GtkWidget *w, GdkEventCrossing *event) {
	ui.pause_game();
}

void enter_window_eh(GtkWidget *w, GdkEventCrossing *event) {
	ui.resume_game();
}

void redraw_window_eh(GtkWidget *w, GdkEventExpose *event) {
	ui.refresh();
}

void button_press_eh(GtkWidget *w, GdkEventButton *event) {
	game.button_press((int)event->x, (int)event->y);
}

void button_release_eh(GtkWidget *w, GdkEventButton *event) {
	game.button_release((int)event->x, (int)event->y);
}

gint timer_eh() {
	game.update();
	return TRUE;
}

