#ifndef __tetris_h__
#define __tetris_h__

/*
 * written by J. Marcin Gorycki <marcin.gorycki@intel.com>
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

#include <config.h>
#include <gnome.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gconf/gconf-client.h>

#include "sound.h"

extern int LINES;
extern int COLUMNS;

extern int BLOCK_SIZE;

enum SlotType 
{
	EMPTY, 
	FALLING, 
	LAYING
};

struct Block
{
	SlotType what;
	int color;
	GnomeCanvasItem* item;
};

extern GdkPixmap *pix;
extern GdkPixbuf **pic;

extern int color_next;
extern int blocknr_next;
extern int rot_next;

extern int blocknr;
extern int rot;
extern int color;

extern int posx;
extern int posy;

extern int nr_of_colors;

extern bool random_block_colors;

class Field;
class Preview;
class BlockOps;
class ScoreFrame;
class HighScores;

class Tetris
{
public:
	Tetris(int cmdlLevel);
	~Tetris();
	
	GtkWidget * getWidget()	{return w;}
	void togglePause();
	void generate();
	void endOfGame();
	void setupPixmap();

private:
	GtkWidget * w;

        GList * themeList;
        
	char *blockPixmap;
	char *bgPixmap;
	
	Field *field;
	Preview *preview;
	BlockOps *ops;
	ScoreFrame *scoreFrame;
	HighScores *high_scores;

	GConfClient *gconf_client;
	
	bool paused;
	int timeoutId;
	bool onePause;
	
	bool inPlay;

	void generateTimer(int level);

	static gint keyPressHandler(GtkWidget *widget, GdkEvent *event, Tetris *t);
	static gint keyReleaseHandler(GtkWidget *widget, GdkEvent *event, Tetris *t);
	static gchar *decodeDropData(gchar * data, gint type);
	void saveBgOptions();
	static void decodeColour (guint16 *data, Tetris *t);
	static void resetColour (Tetris *t);
	static void dragDrop(GtkWidget *widget, GdkDragContext *context,
			     gint x, gint y, GtkSelectionData *data, 
			     guint info, guint time, Tetris *t);
	static int timeoutHandler(void *d);
	static int gameQuit(GtkAction *action, void *d);
	static int gameNew(GtkAction *action, void *d);
	static int focusOut(GtkWidget *widget, GdkEvent *e, Tetris *t);
	static int gamePause(GtkAction *action, void *d);
	static int gameEnd(GtkAction *action, void *d);
	static int gameHelp(GtkAction *action, void *d);
	static int gameAbout(GtkAction *action, void *d);
	static int gameTopTen(GtkAction *action, void *d);
	static int gameProperties(GtkAction *action, void *d);
	static void setupdialogDestroy(GtkWidget *widget, void *d);
	static void setupdialogResponse(GtkWidget *dialog, gint response_id, void *d);
	static void setSound (GtkWidget *widget, gpointer data);
 	static void setRotateCounterClockWise(GtkWidget *widget, void *d);
	static void setSelectionPreview(GtkWidget *widget, void *d);
	static void setSelectionBlocks(GtkWidget *widget, void *d);
	static void setSelection (GtkWidget *widget, void *data);
	static void setBGSelection (GtkWidget *widget, void *data);

	static void lineFillHeightChanged (GtkWidget *spin, gpointer data);
	static void lineFillProbChanged (GtkWidget *spin, gpointer data);
	static void startingLevelChanged (GtkWidget *spin, gpointer data);
	
	static void gconfNotify (GConfClient *tmp_client, guint cnx_id, GConfEntry *tmp_entry, gpointer tmp_data);
	static gchar *gconfGetString (GConfClient *client, const char *key, const char *default_val);
	static int gconfGetInt (GConfClient *client, const char *key, int default_val);
	static gboolean gconfGetBoolean (GConfClient *client, const char *key, gboolean default_val);
	void initOptions ();
	void setOptions ();
	void writeOptions ();
        void setupScoreState ();
	
	GdkPixbuf *image;
	GdkPixbuf *bgimage;
	gboolean usebg;

	GdkColor bgcolour;

	void fillMenu(GtkWidget *menu, char *pixname, char *dirname, GList ** list, bool addnone = false);
	
	GtkWidget *setupdialog;
	GtkWidget *sentry;
	int startingLevel;
	int cmdlineLevel;

	int line_fill_height;
	int line_fill_prob;

	GtkWidget *fill_height_spinner;
	GtkWidget *fill_prob_spinner;
	GtkWidget *do_preview_toggle;
	GtkWidget *random_block_colors_toggle;
	GtkWidget *rotate_counter_clock_wise_toggle;
	GtkWidget *sound_toggle;

	Preview *theme_preview;

	int moveLeft;
	int moveRight;
	int moveDown;
	int moveDrop;
	int moveRotate;
	int movePause;
	
	GtkAction *new_game_action;
	GtkAction *pause_action;
	GtkAction *resume_action;
	GtkAction *scores_action;
	GtkAction *end_game_action;
	GtkAction *preferences_action;

	void manageFallen();

	bool fastFall;
	int fastFallPoints;
        bool dropBlock;
};

#endif // __tetris_h__
