#ifndef __tetris_h__
#define __tetris_h__

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

#include <config.h>
#include <gnome.h>

#define TETRIS_VERSION "0.2.0"

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
};

extern GdkPixmap *pix;

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
extern bool do_preview;

class	Field;
class Preview;
class	BlockOps;
class ScoreFrame;

class Tetris
{
public:
	Tetris(int cmdlLevel);
	~Tetris();
	
	GtkWidget * getWidget()	{return w;}
	void togglePause();
	void generate();
	void endOfGame();
	
private:
	GtkWidget * w;

	Field *field;
	Preview *preview;
	BlockOps *ops;
	ScoreFrame *scoreFrame;
	
	bool paused;
	int timeoutId;
	bool onePause;
	
	static gint eventHandler(GtkWidget *widget, GdkEvent *event, void *d);
	static int timeoutHandler(void *d);
	static int gameQuit(GtkWidget *widget, void *d);
	static int gameNew(GtkWidget *widget, void *d);
	static int gameAbout(GtkWidget *widget, void *d);
	static int gameTopTen(GtkWidget *widget, void *d);
	static int gameProperties(GtkWidget *widget, void *d);
	static void setupdialogDestroy(GtkWidget *widget, void *d);
	static void doSetup(GtkWidget *widget, void *d);
	static void setSelectionPreview(GtkWidget *widget, void *d);
	static void setSelectionBlocks(GtkWidget *widget, void *d);
		
	GtkWidget *setupdialog;
	GtkWidget *sentry;
	int startingLevel;
	int cmdlineLevel;
	bool doPreviewTmp;
	bool randomBlocksTmp;
	
	void manageFallen();
	void showScores(gchar *title, guint pos);
};

#endif // __tetris_h__
