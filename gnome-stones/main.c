/* gnome-stones - main.c
 *
 * Time-stamp: <1998/08/28 05:59:45 carsten>
 *
 * Copyright (C) 1998 Carsten Schaar
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <config.h>
#include <gnome.h>
#include <stdlib.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "game.h"
#include "cave.h"
#include "status.h"
#include "player.h"

#undef USE_GNOME_CANVAS
#undef USE_KEY_RELEASE


#define APP_NAME "gnome-stones"


/* Definitions */

#define START_DELAY 3000
#define END_DELAY 3000
#define CURTAIN_DELAY 50
#define COUNTDOWN_DELAY 20

#define GAME_COLS  20
#define GAME_ROWS 12
#define GAME_SCROLL_MAX 3
#define GAME_SCROLL_MIN 6
#define STONE_SIZE  32


#ifdef USE_KEY_RELEASE
#define GAME_EVENTS (GDK_KEY_PRESS_MASK             |\
		     GDK_KEY_RELEASE_MASK)
;
#else
#define GAME_EVENTS (GDK_KEY_PRESS_MASK)
#endif


/*****************************************************************************/
/* Global Variables */

/* This variable specifies the currently played game.  If 'game' is
   equal to 'NULL', than no game is loaded.  */

static GStonesGame *game= NULL;

/* The currently played cave.  This cave must be a cave, that belongs
   to the game 'game'.  */

static GStonesCave *cave= NULL;

/* The data about the player.  */

static GStonesPlayer *player= NULL;

/* You may start a game in different cavs.  'start_cave' decides, in
   which cave the player will start.  */

static guint start_cave= 0;

/* If you use a joystick as input device, this variable holds the
   device's id.  Setting it to GDK_CORE_POINTER disables the Joystick
   support.  */

static guint32 joystick_deviceid= GDK_CORE_POINTER;
static gfloat  joystick_switch_level= 0.5;


/* The game can be in different states.  These state decide, how to
   react if some events occur.  */

typedef enum
{
  STATE_STARTUP,
  STATE_TITLE,
  STATE_CURTAIN,
  STATE_WAITING_TO_START,
  STATE_RUNNING,
  STATE_COUNTDOWN,
} GameState;

static GameState state= STATE_TITLE;


/* Other stuff.  */

static GtkWidget   *game_widget;
static GdkPixmap   *stone_images;
static GdkPixmap   *title_template;
static GdkPixmap   *title_image;

static gint         player_x_direction= 0;
static gint         player_y_direction= 0;
static gboolean     player_push       = FALSE;


/****************************************************************************/
/* Some constants.  */

/* This message will be printed in the statusline, if no message was
   specified in a game file.  */

static char *default_message= N_("Gnome-Stones (c) 1998 Carsten Schaar");


/****************************************************************************/

void
curtain_start (GStonesCave *cave);


static void
game_start_cb (GtkWidget *widget, gpointer data);



/****************************************************************************/
/* Image stuff

   The following function loads an image and cuts it into pieces.
   These pieces are put into the 'image_table'.  */


#ifdef USE_GNOME_CANVAS
static GdkImlibImage *image_table[16][16];
#endif

static void
load_images (void)
{
  gchar         *filename;
  GdkImlibImage *image;

#ifdef USE_GNOME_CANVAS
  gint ix;
  gint iy;
#endif
  
  /* filename= gnome_unconditional_pixmap_file ("gnome-stones.png"); */
  filename= gnome_pixmap_file ("gnome-stones/tiles.png");
  
  image= gdk_imlib_load_image (filename);
#ifdef USE_GNOME_CANVAS
  /* Now cut the image into small pieces of 'STONE_SIZE' size.  */
  for (ix= 0; ix < 16; ix++)
    for (iy= 0; iy < 16; iy++)
      image_table[ix][iy]= 
	gdk_imlib_crop_and_clone_image (image, ix*STONE_SIZE, iy*STONE_SIZE,
					STONE_SIZE, STONE_SIZE);
#endif
  gdk_imlib_render (image, image->rgb_width, image->rgb_height);
  
  stone_images= gdk_imlib_move_image (image);

  /* Now the title image.  */
  filename= gnome_pixmap_file ("gnome-stones/title.png");

  image= gdk_imlib_load_image (filename);

  gdk_imlib_render (image, image->rgb_width, image->rgb_height);
  title_template= gdk_imlib_copy_image (image);
  title_image   = gdk_imlib_move_image (image);  

  /* Redraw the game widget.  */
  gtk_widget_draw (game_widget, NULL);
}


/****************************************************************************/

/* Game widget handling.  */

typedef enum
{
  GAME_DISPLAY_IMAGE,
  GAME_DISPLAY_CAVE
} GameDisplayMode;

typedef enum
{
  GAME_CURTAIN_NONE,
  GAME_CURTAIN_OPENING,
  GAME_CURTAIN_CLOSING
} GameCurtainMode;

static GameDisplayMode display_mode= GAME_DISPLAY_IMAGE;
static GameCurtainMode curtain_mode= GAME_CURTAIN_NONE;

static guint x_offset= 0;
static guint y_offset= 0;

static guint curtain = 0;

#ifdef USE_GNOME_CANVAS
static GnomeCanvasItem *game_items[GAME_COLS][GAME_ROWS];
static GdkImlibImage   *game_current_images[GAME_COLS][GAME_ROWS];
#endif

static gint
game_widget_expose_event_cb (GtkWidget *widget, GdkEventExpose *event, 
			     gpointer data)
{
  GdkRectangle *area;
  int x1, y1, x2, y2, x, y;

  area= &event->area;

  if (state == STATE_TITLE)
    {
      gdk_draw_pixmap (widget->window,
		       widget->style->black_gc, title_image,
		       area->x, area->y, 
		       area->x, area->y,
		       area->width, area->height);
      return TRUE;
    }

  x1 = area->x/STONE_SIZE+x_offset;
  y1 = area->y/STONE_SIZE+y_offset;
  x2 = (area->x+area->width )/STONE_SIZE+x_offset;
  y2 = (area->y+area->height)/STONE_SIZE+y_offset;
  
  for (x = x1; x <= x2; x++)
    for (y = y1; y <= y2; y++)
      {
	
	gboolean cave_mode;
	gint ix= 0;
	gint iy= 0;
	
	if ((curtain_mode == GAME_CURTAIN_CLOSING && 
	     (x+y > curtain+x_offset+y_offset)) ||
	    (curtain_mode == GAME_CURTAIN_OPENING && 
	     (x+y < curtain+x_offset+y_offset)))
	  {
	    cave_mode= TRUE;
	    ix= 4;
	    iy= 0;
	  }
	else if (display_mode == GAME_DISPLAY_IMAGE)
	  {
	    cave_mode= FALSE;
	  }
	else if (cave)
	  {
	    cave_mode= TRUE;

	    cave_get_image_index (cave, x+1, y+1, &ix, &iy);
	  }

	if (cave_mode)
	  gdk_draw_pixmap (widget->window,
			   widget->style->black_gc, stone_images,
			   ix*STONE_SIZE, iy*STONE_SIZE, 
			   (x-x_offset)*STONE_SIZE, 
			   (y-y_offset)*STONE_SIZE,
			   STONE_SIZE, STONE_SIZE);
	else
	  gdk_draw_pixmap (widget->window,
			   widget->style->black_gc, title_image,
			   (x-x_offset)*STONE_SIZE, 
			   (y-y_offset)*STONE_SIZE, 
			   (x-x_offset)*STONE_SIZE, 
			   (y-y_offset)*STONE_SIZE,
			   STONE_SIZE, STONE_SIZE);
      }

  return TRUE;
}


#ifdef USE_GNOME_CANVAS
static void
game_update_image (GtkWidget *widget)
{
  GdkImlibImage *image;
  int x1, y1, x2, y2, x, y;

  x1 = x_offset;
  y1 = y_offset;
  x2 = GAME_COLS+x_offset-1;
  y2 = GAME_ROWS+y_offset-1;
  
  for (x = x1; x <= x2; x++)
    for (y = y1; y <= y2; y++)
      {
	
	gboolean cave_mode;
	gint ix= 0;
	gint iy= 0;
	
	if ((curtain_mode == GAME_CURTAIN_CLOSING && 
	     (x+y > curtain+x_offset+y_offset)) ||
	    (curtain_mode == GAME_CURTAIN_OPENING && 
	     (x+y < curtain+x_offset+y_offset)))
	  {
	    cave_mode= TRUE;
	    ix= 4;
	    iy= 0;
	  }
	else if (display_mode == GAME_DISPLAY_IMAGE)
	  {
	    cave_mode= FALSE;
	  }
	else
	  {
	    cave_mode= TRUE;

	    cave_get_image_index (cave, x+1, y+1, &ix, &iy);
	  }
	image= image_table[ix][iy];
	
	if (image != game_current_images[x-x_offset][y-y_offset])
	  {
	    gnome_canvas_item_set (game_items[x-x_offset][y-y_offset],
				   "image", image,
				   NULL);
	    game_current_images[x-x_offset][y-y_offset]= image;
	  }
      }

  gnome_canvas_update_now (GNOME_CANVAS (widget));
}
#endif



static void
game_update_title (void)
{
  /* Replace image with template.  */
  gdk_draw_pixmap (title_image,
		   game_widget->style->black_gc, title_template,
		   0, 0,
		   0, 0, GAME_COLS*STONE_SIZE, GAME_ROWS*STONE_SIZE);

  if (game)
    {
      static char *gametitle= N_("Game title:");
      static char *cavename = N_("Start cave:");

      guint    height;
      GdkFont *font;
      GList   *tmp= g_list_nth (game->start_caves, start_cave);
      
      font= gdk_font_load ("-adobe-helvetica-bold-r-normal--24-240-75-75-p-138-iso8859-1");
      if (!font)
	font= gdk_font_load ("fixed");

      height= font->ascent+font->descent;

      gdk_draw_text   (title_image,
		       font,
		       game_widget->style->black_gc,
		       30, GAME_ROWS*STONE_SIZE-30-height*3-height/2,
		       _(gametitle),
		       strlen (_(gametitle)));

      gdk_draw_text   (title_image,
		       font,
		       game_widget->style->black_gc,
		       30, GAME_ROWS*STONE_SIZE-30-height*2-height/2,
		       game->title,
		       strlen (game->title));
      
      gdk_draw_text   (title_image,
		       font,
		       game_widget->style->black_gc,
		       30, GAME_ROWS*STONE_SIZE-30-height,
		       _(cavename),
		       strlen (_(cavename)));

      gdk_draw_text   (title_image,
		       font,
		       game_widget->style->black_gc,
		       30, GAME_ROWS*STONE_SIZE-30,
		       tmp->data,
		       strlen (tmp->data));

      gdk_font_unref (font);
    }  

  /* Redraw the game widget.  */
  gtk_widget_draw (game_widget, NULL);
}


static guint last_keyval= GDK_VoidSymbol;

static gint
game_widget_key_press_callback (GtkWidget   *widget,
				GdkEventKey *event,
				gpointer     client_data)
{
  player_push= event->state & GDK_SHIFT_MASK;
  
  switch (event->keyval)
    {
    case GDK_KP_8:
    case GDK_KP_Up:
    case GDK_Up:
      if (state == STATE_RUNNING)
	{
	  /* Move player.  */
	  player_x_direction=  0;
	  player_y_direction= -1;
	  last_keyval      = event->keyval;
	  return TRUE;
	}
      else if ((state == STATE_TITLE) && game)
	{
	  if (start_cave+1 < g_list_length (game->start_caves))
	    {
	      start_cave++;
	      game_update_title ();
	    }
	  return TRUE;
	}
      return FALSE;
    case GDK_KP_4:
    case GDK_KP_Left:
    case GDK_Left:
      player_x_direction= -1;
      player_y_direction=  0;
      last_keyval      = event->keyval;
      return TRUE;
    case GDK_KP_6:
    case GDK_KP_Right:
    case GDK_Right:
      player_x_direction=  1;
      player_y_direction=  0;
      last_keyval      = event->keyval;
      return TRUE;
    case GDK_KP_2:
    case GDK_KP_Down:
    case GDK_Down:
      if (state == STATE_RUNNING)
	{
	  /* Move player.  */
	  player_x_direction=  0;
	  player_y_direction=  1;
	  last_keyval      = event->keyval;
	  return TRUE;
	}
      else if ((state == STATE_TITLE) && game)
	{
	  if (start_cave > 0)
	    {
	      start_cave--;
	      game_update_title ();
	    }
	  return TRUE;
	}
      return FALSE;
    case GDK_Escape:
      if ((state == STATE_RUNNING) && cave)
	{
	  if (player_push)
	    curtain_start (NULL);
	  else
	    cave_player_die (cave);
	}
      return TRUE;
    case GDK_P:
    case GDK_p:
      if ((state == STATE_RUNNING) && cave)
	{
	  cave_toggle_pause_mode (cave);
	  
	  if (cave->flags & CAVE_PAUSING)
	    gnome_appbar_set_status (GNOME_APPBAR (statusbar), _("Pause"));
	  else
	    gnome_appbar_refresh (GNOME_APPBAR (statusbar));
	}
      return FALSE;
    case GDK_KP_Enter:
    case GDK_Return:
      if (state == STATE_TITLE)
	{
	  game_start_cb (NULL, NULL);
	  return TRUE; 
	}
      return FALSE;
    }

  return FALSE;
};


static gint
game_widget_key_release_callback (GtkWidget   *widget,
				  GdkEventKey *event,
				  gpointer     client_data)
{
  if (last_keyval == event->keyval)
    {
      player_x_direction=  0;
      player_y_direction=  0;
      last_keyval       =  GDK_VoidSymbol;
      return TRUE;
    }

  return FALSE;
}


static GtkWidget *
game_widget_create (void)
{
  GtkWidget *widget;
#ifdef USE_GNOME_CANVAS
  GtkWidget *canvas;
#endif

  gtk_widget_push_visual (gdk_imlib_get_visual ());
  gtk_widget_push_colormap (gdk_imlib_get_colormap ());

  widget= gtk_drawing_area_new ();
#ifdef USE_GNOME_CANVAS
  canvas= gnome_canvas_new ();
#endif

  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();
  
  gtk_widget_set_events (widget, 
			 gtk_widget_get_events (widget) | GAME_EVENTS);

  gtk_drawing_area_size (GTK_DRAWING_AREA (widget),
			 GAME_COLS * STONE_SIZE,
			 GAME_ROWS * STONE_SIZE);

#ifdef USE_GNOME_CANVAS
  /* Now for some experimantal gnome canvas stuff.  */
  gnome_canvas_set_size (GNOME_CANVAS (canvas), 
			 GAME_COLS*STONE_SIZE,
			 GAME_ROWS*STONE_SIZE);
  gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas),
				  0, 0,
				  GAME_COLS*STONE_SIZE,
				  GAME_ROWS*STONE_SIZE);

  /* Now we fill our canvas with images.  */
  {
    GdkImlibImage *im;
    guint x;
    guint y;
    
    im= gdk_imlib_load_image ("frame.png");
    for (x= 0; x < GAME_COLS ; x+= 1)
      for (y= 0; y < GAME_COLS ;  y+= 1)
	{
	  game_items[x][y]=
	    gnome_canvas_item_new (GNOME_CANVAS_GROUP (gnome_canvas_root 
						       (GNOME_CANVAS 
							(canvas))),
				   gnome_canvas_image_get_type (),
				   "image", im,
				   "x", (double) x*STONE_SIZE,
				   "y", (double) y*STONE_SIZE,
				   "width", (double) im->rgb_width,
				   "height", (double) im->rgb_height,
				   "anchor", GTK_ANCHOR_NW,
				   NULL);
	  
	}
  }
  
  gtk_widget_show (canvas);
#endif

  gtk_widget_show (widget);

  gtk_signal_connect (GTK_OBJECT (widget), "expose_event", 
		      (GtkSignalFunc) game_widget_expose_event_cb, 0);
  gtk_widget_set_extension_events (widget, GDK_EXTENSION_EVENTS_ALL);

#ifdef USE_GNOME_CANVAS
  return canvas;
#else
  return widget;
#endif
}


static void
game_widget_caluculate_offset (GStonesCave *cave)
{
  static gboolean x_scrolling= TRUE;
  static gboolean y_scrolling= TRUE;

  gint x_rel;
  gint y_rel;
  
  x_rel= cave->player_x-x_offset;
  y_rel= cave->player_y-y_offset;
  
  x_scrolling= x_scrolling || (x_rel < 3) || (x_rel > GAME_COLS+1-3) || 
    (x_offset+GAME_COLS > cave->width);
  y_scrolling= y_scrolling || (y_rel < 3) || (y_rel > GAME_ROWS+1-3) || 
    (y_offset+GAME_ROWS > cave->height);
  
  if (x_scrolling)
    {
      if (((x_rel < 7) || (x_offset+GAME_COLS > cave->width)) && (x_offset > 0))
	{
	  x_offset--;
	}
      else if ((x_rel > GAME_COLS+1-7) && (x_offset+GAME_COLS < cave->width))
	{
	  x_offset++;
	}
      else
	x_scrolling= FALSE;
    }

  if (y_scrolling)
    {      
      if (((y_rel < 5) || (y_offset+GAME_ROWS > cave->height)) && (y_offset > 0))
	{
	  y_offset--;
	}
      else if ((y_rel > GAME_ROWS+1-5) && (y_offset+GAME_ROWS < cave->height))
	{
	  y_offset++;
	}
      else
	y_scrolling= FALSE;
    }
}



/****************************************************************************/
/* Additional Games stuff.  */

typedef struct _GameFile GameFile;

struct _GameFile
{
  gchar *filename;
  gchar *gametitle;
  guint  caves;
};

GList *games= NULL;


static gint
compare_game_names (const GameFile *file, const char *name)
{
  return strcmp (file->filename, name);
}


static GameFile *
add_game (const char *filename)
{
  char     *prefix= NULL;
  GList    *tmp;
  GameFile *file;
  
  /* Maybe this game is already in the games list.  */
  tmp= g_list_find_custom (games, (gpointer) filename, 
			   (GCompareFunc) compare_game_names);
  if (tmp) return (GameFile *) tmp->data;
  
  /* FIXME: add some checks, if it's realy a Gnome-Stones game.  */
  file= g_malloc (sizeof (GameFile));
  
  if (file)
    {
      file->filename = g_strdup (filename);

      prefix= g_copy_strings ("=", filename, "=/", NULL);
      gnome_config_push_prefix (prefix);
      
      file->gametitle= gnome_config_get_string ("General/Title");
      file->caves    = gnome_config_get_int    ("Caves/Number");

      gnome_config_pop_prefix ();
      g_free (prefix);
      
      games= g_list_append (games, file);
    }

  return file;
}


static void
scan_game_directories (const char *directory)
{
  DIR *dir;
  
  dir= opendir (directory);
  if (dir)
    {
      struct dirent *entry;
      
      while ((entry = readdir (dir)) != NULL)
	{
	  char *filename= g_copy_strings (directory, "/", entry->d_name, NULL);
	  struct stat sbuf;
	  
	  if ((stat (filename, &sbuf)== 0) && S_ISREG (sbuf.st_mode))
	    add_game (filename);

	  g_free (filename);
	}
      
      closedir (dir);
    }
}


static gboolean
scan_public_game_directories (void)
{
  scan_game_directories (CAVESDIR);

  return TRUE;
}


static gboolean
scan_private_game_directories (void)
{
  char *path= gnome_util_home_file ("GnomeStonesGames");
  
  scan_game_directories (path);

  g_free (path);
  return TRUE;
}


/****************************************************************************/
/* Timeout stuff

   In the following, we list all needed timeout handles.  */

static guint curtain_timeout= 0;
static guint iteration_timeout= 0;
static guint countdown_timeout= 0;


/****************************************************************************/
/* Curtain stuff
   
   The following function implement the opening and closing curtain
   between the different caves.  */

static gint
start_cave_delay_timeout (gpointer data);

static void
iteration_start (GStonesCave *cave);

static gint
curtain_open_timeout_function (gpointer data)
{
  GStonesCave *newcave= (GStonesCave *) data;

  if (curtain > 0)
    {
      /* The curtain is still being opened.  */
      
      curtain--;
      gtk_widget_draw (game_widget, NULL);
      return TRUE;
    }
  
  curtain_mode= GAME_CURTAIN_NONE;

  if (newcave)
    {
      state= STATE_WAITING_TO_START;
      gtk_timeout_add (START_DELAY, start_cave_delay_timeout, newcave);
    }
  else
    state= STATE_TITLE;
  
  return FALSE;
}


static gint
curtain_close_timeout_function (gpointer data)
{
  GStonesCave *newcave= (GStonesCave *) data;
      
  if (curtain > 0)
    {
      /* The curtain is not closed yet.  */
      
      curtain--;
      gtk_widget_draw (game_widget, NULL);
      return TRUE;
    }
  
  /* If the iteration has been running, we stop it now.  */
  gtk_timeout_remove (iteration_timeout);

  /* An existing cave must be deleted.  */
  if (cave)
    {
      cave_free (cave);
      cave= NULL;
    }
  
  if (newcave)
    {
      display_mode= GAME_DISPLAY_CAVE;
      status_set_mode (STATUS_GAME_INFO);
      
      iteration_start (newcave);

      /* Print message to screen.  */
      if (newcave->message)
	gnome_appbar_set_default (GNOME_APPBAR (statusbar), cave->message);
      else
	gnome_appbar_set_default (GNOME_APPBAR (statusbar), _(default_message));
    }
  else
    display_mode= GAME_DISPLAY_IMAGE;


  /* We can now open curtain.  */
  curtain_mode= GAME_CURTAIN_OPENING;
  curtain     = GAME_COLS+GAME_ROWS;
  
  curtain_timeout= 
    gtk_timeout_add (CURTAIN_DELAY, curtain_open_timeout_function, newcave);

  return FALSE;
}


/* Start a curtain sequence.  

   'cave' determines, what happens after the curtain sequence.
   Setting 'cave' to NULL means, that the title image will be shown
   after the sequence.  Setting 'cave' to a non NULL value, lets the
   program load a new cave and start it.  */

void
curtain_start (GStonesCave *cave)
{
  state       = STATE_CURTAIN;
  curtain_mode= GAME_CURTAIN_CLOSING;
  curtain     = GAME_COLS+GAME_ROWS;

  curtain_timeout= 
    gtk_timeout_add (CURTAIN_DELAY, curtain_close_timeout_function, cave);
}



/****************************************************************************/


static gint
start_cave_delay_timeout (gpointer data)
{
  GStonesCave *cave= (GStonesCave *) data;
  
  if (state == STATE_WAITING_TO_START)
    { 
      status_set_mode (STATUS_CAVE_INFO);
      cave_start (cave);
      
      state= STATE_RUNNING;
    }

  return FALSE;
}



/*****************************************************************************/
/* Countdown stuff

   The following functions are needed to implement the timer countdown,
   when the player has finished a level.  */

static gint
countdown_timeout_function (gpointer data)
{
  GStonesCave *cave   = (GStonesCave *)data;
  GStonesCave *newcave= NULL;
  
  if (state == STATE_COUNTDOWN)
    {
      if (cave->timer > 0)
	{
	  cave->timer-= 1000;
	  if (cave->timer < 0) cave->timer= 0;

	  player_set_time (player, cave->timer, FALSE);
	  
	  /* FIXME: The added value should be configureable on a per
             game basis.  */
	  player_inc_score (player, 1);
	  
	  return TRUE;
	}
      
      if (cave->next)
	newcave= cave_load (game, cave->next);
	
      curtain_start (newcave);

      if (!newcave)
	{
	  gint pos;

	  pos= gnome_score_log (player->score, NULL, TRUE);

	  /* FIXME: somewhere is a bug, that makes this needed.  */
	  if (pos > 10) pos= 0;

	  gnome_scores_display ("Gnome-Stones", APP_NAME, NULL, pos);
	  gnome_app_flash (GNOME_APP (app), _("Congratulations, you win!"));
	}
    }
  return FALSE;
}

/* Start a countdown.  

   'cavename' determines, what happens after the countdown.
   Setting 'cavename' to NULL means, the title image will be shown
   after the countdown.  Setting 'cavename' to a non NULL value, lets
   the program load a new cave and start it.  */

void
countdown_start (GStonesCave *cave)
{
  /* When showing the countdown, there is no iteration in the
     background anymore.  */
  gtk_timeout_remove (iteration_timeout);

  state    = STATE_COUNTDOWN;
  
  countdown_timeout= 
    gtk_timeout_add (COUNTDOWN_DELAY, 
		     (GtkFunction) countdown_timeout_function, 
		     (gpointer) cave);
}



/*****************************************************************************/

void
joystick_get_information (gint *x_direction, gint *y_direction)
{
  /* FIXME: This function should only return a joystick movement, if
     the game_widget has focus.  */
  if (joystick_deviceid != GDK_CORE_POINTER)
    {
      gdouble x;
      gdouble y;
      
      gint xi;
      gint yi;
      
      gint xdir= 0;
      gint ydir= 0;
      
      GdkModifierType mask;
      
      gdk_input_window_get_pointer (GTK_WIDGET (game_widget)->window, 
				    joystick_deviceid, 
				    &x, &y, NULL, NULL, NULL, &mask);
      gdk_window_get_origin (GTK_WIDGET (game_widget)->window, &xi, &yi);
      
      x= ((gdouble) xi+x)/gdk_screen_width ()*100.0;
      y= ((gdouble) yi+y)/gdk_screen_height ()*100.0;
      
      if (x > joystick_switch_level)
	xdir= 1;
      else if (x < -joystick_switch_level)
	xdir= -1;
      else if (y > joystick_switch_level)
	ydir= 1;
      else if (y < -joystick_switch_level)
	ydir= -1;
      
      if (x_direction) *x_direction= xdir;
      if (y_direction) *y_direction= ydir;
    }
}


CaveFlags flags= 0;

static gint
iteration_timeout_function (gpointer data)
{
  GStonesCave *cave  = (GStonesCave *)data;
  gint         retval= TRUE;

  if (state == STATE_TITLE)
    return TRUE;

  joystick_get_information (&player_x_direction, &player_y_direction);
  
  cave_iterate (cave, player_x_direction, player_y_direction, player_push);

#ifndef USE_KEY_RELEASE
  player_x_direction= 0;
  player_y_direction= 0;
#endif 
  
  if (!(flags & CAVE_FINISHED) && (cave->flags & CAVE_FINISHED))
    {
      if (!(cave->flags & CAVE_PLAYER_EXISTS) && !cave->is_intermission)
	{
	  player_inc_lifes (player, -1);
	  
	  if (player->lifes != 0)
	    { 
	      /* Restart cave.  */
	      GStonesCave *newcave= cave_load (game, cave->name);
	      
	      curtain_start (newcave);
	    }
	  else
	    {
	      /* That's it.  No lifes anymore.  */
	      gint pos= gnome_score_log (player->score, NULL, TRUE);

	      /* FIXME: somewhere is a bug, that makes this needed.  */
	      if (pos > 10) pos= 0;
	      
	      gnome_scores_display ("Gnome-Stones", APP_NAME, NULL, pos);
	      gnome_app_flash (GNOME_APP (app), _("Game over!"));
	      curtain_start (NULL);
	    }
	}
      else
	countdown_start (cave);
      
      retval= TRUE;
      
    }

  /* Manage scrolling.  */
  game_widget_caluculate_offset (cave);

#ifdef USE_GNOME_CANVAS  
  game_update_image (game_widget);
#else
  gtk_widget_draw (game_widget, NULL);
#endif
  flags= cave->flags;

  return retval;
}


static void
iteration_start (GStonesCave *newcave)
{
  /* FIXME: bad hack.  */
  cave = newcave;
  flags= 0;

  cave_set_player (cave, player);
  status_set_cave (cave->name);
  
  iteration_timeout= 
    gtk_timeout_add (game->frame_rate, iteration_timeout_function, cave);
}



/****************************************************************************/
/* Additional game stuff.  */


static void
load_game (GameFile *file)
{
  char         buffer[256];
  GStonesGame *newgame;
  
  g_return_if_fail (file);
  
  /* We don't need to load a game twice.  */
  if (game && (strcmp (game->filename, file->filename) == 0))
    {
      /* Give feedback in statusbar.  */
      sprintf (buffer, "Game '%s' already loaded.", file->filename);
      gnome_app_flash (GNOME_APP (app), buffer);
      return;
    }
  
  /* Load game.  */
  newgame= game_load (file->filename);

  if (newgame)
    {
      /* Remove all running timeouts.  */
      gtk_timeout_remove (curtain_timeout);
      gtk_timeout_remove (iteration_timeout);
      gtk_timeout_remove (countdown_timeout);

      /* Set display into title mode.  */
      display_mode= GAME_DISPLAY_IMAGE;
      curtain_mode= GAME_CURTAIN_NONE;

      /* Set program state into title state.  */
      state= STATE_TITLE;

      /* Remove any existing game and cave.  */
      if (cave) cave_free (cave);
      if (game) game_free (game);

      /* Set the global variables, concerning the game to some default
         values.  We don't need to remove 'player'.  This will be done
         automatically, when starting a game.  */
      cave      = NULL;
      game      = newgame;
      start_cave= 0;

      /* Update the title image.  */
      game_update_title ();

      /* Store filename.  */
      /* FIXME: should additionally use session management.  */
      gnome_config_set_string ("GnomeStones/Preferences/Game", file->filename);
      gnome_config_sync ();

      /* Give feedback in statusbar.  */
      sprintf (buffer, "Game '%s' loaded.", file->filename);
      gnome_app_flash (GNOME_APP (app), buffer);
    }
}


static void
load_game_by_number (guint n)
{
  GList *tmp= g_list_nth (games, n);

  g_return_if_fail (tmp != NULL);
  
  load_game ((GameFile *) tmp->data);
}


static void
load_game_by_name (const char *filename)
{
  GameFile *file= add_game (filename);
  
  if (file)
    load_game (file);
}


static gboolean
load_default_game (void)
{
  char     *filename;
  gboolean  def;
  
  filename= gnome_config_get_string_with_default 
    ("GnomeStones/Preferences/Game", &def);

  if (def)
    load_game_by_name (CAVESDIR"/default.caves");
  else
    load_game_by_name (filename);

  g_free (filename);

  return TRUE;
}



/****************************************************************************/
/* Preferences dialog stuff.  */

typedef struct _PreferencesData PreferencesData;

struct _PreferencesData
{
  /* General information.  */
  GnomePropertyBox *property_box;
  
  /* Page one.  */
  GtkWidget        *game_list;
  gint              selected_game;

  /* Page two. */
  GtkWidget        *level_frame;
  
  guint32           joystick_deviceid;
  gfloat            joystick_switch_level;
};


static void
preferences_apply_cb (GtkWidget *w, gint page, gpointer data)
{
  PreferencesData *prdata= (PreferencesData *) data;

  g_return_if_fail (prdata != NULL);

  switch (page)
    {
    case 0:
      /* FIXME: Add some checks and warnings here.  */
      if (prdata->selected_game > -1)
	load_game_by_number (prdata->selected_game);
      break;

    case 1:
      joystick_deviceid= prdata->joystick_deviceid;
      if (joystick_deviceid != GDK_CORE_POINTER)
	gdk_input_set_mode (joystick_deviceid, GDK_MODE_SCREEN);

      joystick_switch_level= prdata->joystick_switch_level;
      break;
    default:
      break;
    }
}


static void
preferences_changed_cb (GtkWidget *w, gpointer data)
{
  PreferencesData *prdata= (PreferencesData *) data;
  
  g_return_if_fail (prdata != NULL);

  gnome_property_box_changed (prdata->property_box);
}


static gint
preferences_delete_cb (GtkWidget *w, GdkEventAny *event, gpointer data)
{
  g_free (data);

  return FALSE;
}


static void 
game_selector_select_row (GtkCList * clist,
			  gint row, gint column,
			  GdkEvent * event, gpointer data)
{
  PreferencesData *prdata= (PreferencesData *) data;
  
  g_return_if_fail (prdata != NULL);

  preferences_changed_cb (GTK_WIDGET (clist), data);

  prdata->selected_game= row;
}


/* The joystick callbacks.  */

static void
preferences_set_joystick_device (GtkWidget *widget, gpointer data)
{
  guint32          deviceid= GPOINTER_TO_UINT(data);
  PreferencesData *prdata  = 
    (PreferencesData *) gtk_object_get_user_data (GTK_OBJECT (widget));

  prdata->joystick_deviceid= deviceid;
  
  if (deviceid == GDK_CORE_POINTER)
    {
      gtk_widget_set_sensitive (prdata->level_frame, FALSE);
    }
  else
    {
      gtk_widget_set_sensitive (prdata->level_frame, TRUE);
    }

  gnome_property_box_changed (prdata->property_box);
}


static void
preferences_set_joystick_switch_level (GtkAdjustment *adjust, gpointer data)
{
  PreferencesData *prdata  = 
    (PreferencesData *) gtk_object_get_user_data (GTK_OBJECT (adjust));

  prdata->joystick_switch_level= adjust->value;

  gnome_property_box_changed (prdata->property_box);
}


GtkWidget *
preferences_dialog_new (void)
{
  GtkWidget *propbox;
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *list;
  PreferencesData *prdata;

  prdata= g_malloc (sizeof (PreferencesData));

  propbox= gnome_property_box_new ();
  prdata->property_box= GNOME_PROPERTY_BOX (propbox);
  
  gtk_window_set_title (GTK_WINDOW(&GNOME_PROPERTY_BOX(propbox)->dialog.window),
			_("Gnome-Stones Preferences"));

  /* The first page of our preferences dialog. */
  box= gtk_vbox_new (TRUE, 4);

  /* The list of game names.  */
  list= gtk_clist_new (3);
  prdata->game_list= list;

  gtk_clist_set_column_title (GTK_CLIST (list), 0, _("Game title"));
  gtk_clist_set_column_width (GTK_CLIST (list), 0, 250);
  gtk_clist_set_column_title (GTK_CLIST (list), 1, _("Caves"));
  gtk_clist_set_column_width (GTK_CLIST (list), 1, 50);
  gtk_clist_set_column_justification (GTK_CLIST (list), 1, GTK_JUSTIFY_RIGHT);
  gtk_clist_set_column_title (GTK_CLIST (list), 2, _("Filename"));
  gtk_clist_column_titles_passive (GTK_CLIST (list));
  gtk_clist_column_titles_show (GTK_CLIST (list));
  gtk_widget_set_usize (list, -2, 200);
  
  gtk_clist_set_policy (GTK_CLIST (list), 
			GTK_POLICY_ALWAYS, 
			GTK_POLICY_AUTOMATIC);
  gtk_clist_set_selection_mode (GTK_CLIST (list), GTK_SELECTION_SINGLE);
  
  gtk_box_pack_start (GTK_BOX (box), list, FALSE, FALSE, 0);

  gtk_clist_freeze (GTK_CLIST (list));
  prdata->selected_game= -1;
  {
    GList *tmp= games;
    
    while (tmp)
      {
	char buffer[10];
	GameFile *file= (GameFile *)tmp->data;
	char     *entry[3];
	gint      row;
	
	entry[0]= file->gametitle;
	sprintf (buffer, "%d", file->caves);
	entry[1]= buffer;
	entry[2]= g_basename (file->filename);
	
	row= gtk_clist_append (GTK_CLIST (list), entry);

	if (game && strcmp (file->filename, game->filename) == 0)
	  {
	    gtk_clist_select_row (GTK_CLIST (list), row, 0);
	    prdata->selected_game= row;
	  }

	tmp= tmp->next;
      }
  }
  gtk_clist_thaw (GTK_CLIST (list));
  gtk_signal_connect (GTK_OBJECT (list), "select_row", 
		      GTK_SIGNAL_FUNC (game_selector_select_row),
		      prdata);

  gtk_widget_show (list);
  gtk_widget_show (box);

  label= gtk_label_new (_("Game"));
  gtk_notebook_append_page (GTK_NOTEBOOK 
			    (GNOME_PROPERTY_BOX (propbox)->notebook), 
			    box, label);

  /* The second page of our preferences dialog. */

  prdata->joystick_deviceid    = joystick_deviceid;
  prdata->joystick_switch_level= joystick_switch_level;

  box= gtk_vbox_new (TRUE, 4);
  
  {
    guint      i;
    GtkObject *adjust;
    GtkWidget *frame;
    GtkWidget *scale;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *menuitem;
    GtkWidget *optionmenu;
    GtkWidget *device_menu;
    GList     *tmp;
    GList     *device_info= gdk_input_list_devices ();

    frame= gtk_frame_new (_("Device"));
    gtk_box_pack_start (GTK_BOX (box), frame, TRUE, FALSE, 0);
    gtk_widget_show (frame);

    vbox= gtk_vbox_new (FALSE, 2);
    gtk_container_add (GTK_CONTAINER (frame), vbox);
    gtk_widget_show (vbox);

    hbox= gtk_hbox_new (FALSE, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
    gtk_widget_show (hbox);

    label= gtk_label_new (_("Joystick device:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
    gtk_widget_show (label);
    
    device_menu= gtk_menu_new ();

    /* We definatly have a "disable" entry.  */
    menuitem= gtk_menu_item_new_with_label (_("disabled"));
    gtk_object_set_user_data (GTK_OBJECT (menuitem), prdata);
    gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			(GtkSignalFunc) preferences_set_joystick_device,
			GUINT_TO_POINTER (GDK_CORE_POINTER));
    gtk_menu_append (GTK_MENU (device_menu), menuitem);
    gtk_widget_show (menuitem);
    
    for (tmp= device_info, i= 1; tmp; tmp= tmp->next, i++)
      {
	GdkDeviceInfo *info = (GdkDeviceInfo *) tmp->data;

	if (info->deviceid != GDK_CORE_POINTER)
	  {
	    menuitem= gtk_menu_item_new_with_label (info->name);

	    gtk_object_set_user_data (GTK_OBJECT (menuitem), prdata);
            gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
                                (GtkSignalFunc) preferences_set_joystick_device,
                                GUINT_TO_POINTER (info->deviceid));

	    gtk_menu_append (GTK_MENU (device_menu), menuitem);
	    gtk_widget_show (menuitem);
	  }

	if (info->deviceid == prdata->joystick_deviceid)
	  gtk_menu_set_active (GTK_MENU (device_menu), i);
      }
    
    optionmenu= gtk_option_menu_new ();
    gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu), device_menu);
    gtk_box_pack_start (GTK_BOX (hbox), optionmenu, FALSE, FALSE, 2);
    gtk_widget_show (optionmenu);
    
    gtk_widget_show (label);
    gtk_widget_show (hbox);
    gtk_widget_show (optionmenu);

    frame= gtk_frame_new (_("Binary joytick emulation"));
    gtk_box_pack_start (GTK_BOX (box), frame, TRUE, FALSE, 0);
    gtk_widget_show (frame);

    vbox= gtk_vbox_new (FALSE, 2);
    gtk_container_add (GTK_CONTAINER (frame), vbox);
    gtk_widget_show (vbox);

    hbox= gtk_hbox_new (FALSE, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
    gtk_widget_show (hbox);

    label= gtk_label_new (_("Switch level:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
    gtk_widget_show (label);

    adjust= gtk_adjustment_new (prdata->joystick_switch_level,
				0.0, 1.0, 0.02, 0.1, 0.0);
    gtk_object_set_user_data (adjust, prdata);
    gtk_signal_connect (adjust, "value_changed",
			(GtkSignalFunc) preferences_set_joystick_switch_level,
			NULL);
    
    scale= gtk_hscale_new (GTK_ADJUSTMENT (adjust));
    gtk_scale_set_digits (GTK_SCALE (scale), 2);
    gtk_box_pack_start (GTK_BOX (hbox), scale, FALSE, FALSE, 2);
    gtk_widget_show (scale);
    
    if (prdata->joystick_deviceid == GDK_CORE_POINTER)
      {
	gtk_widget_set_sensitive (GTK_WIDGET (frame), FALSE);
      }

    prdata->level_frame= frame;
  }
  
  gtk_widget_show (box);

  label= gtk_label_new (_("Joystick"));
  gtk_notebook_append_page (GTK_NOTEBOOK 
			    (GNOME_PROPERTY_BOX (propbox)->notebook), 
			    box, label);

  /* The third page of our preferences dialog. */
  box= gtk_vbox_new (TRUE, 4);

  label= gtk_label_new (_("Not yet implemented!"));
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  gtk_widget_show (label);
  gtk_widget_show (box);

  label= gtk_label_new (_("Sound"));
  gtk_notebook_append_page (GTK_NOTEBOOK 
			    (GNOME_PROPERTY_BOX (propbox)->notebook), 
			    box, label);

  gtk_signal_connect (GTK_OBJECT (propbox), "delete_event",
		      GTK_SIGNAL_FUNC (preferences_delete_cb), prdata);
  gtk_signal_connect (GTK_OBJECT (propbox), "apply",
		      GTK_SIGNAL_FUNC (preferences_apply_cb), prdata);
  return propbox;
}



/*****************************************************************************/
/* Menu callback functions */


static void
game_start_cb (GtkWidget *widget, gpointer data)
{
  GList       *tmp = NULL;
  GStonesCave *cave= NULL;
  
  if (!game) return;
  
  /* Initialize the player information.  */
  if (player) player_free (player);
  player= player_new (game);
  if (!player) return;

  tmp = g_list_nth (game->start_caves, start_cave);
  cave= cave_load (game, tmp->data);
  
  if (cave)
    curtain_start (cave);
}


static void
preferences_cb (GtkWidget *widget, gpointer data)
{
  gtk_widget_show (preferences_dialog_new ());
}


static void
quit_cb (GtkWidget *widget, gpointer data)
{
  exit (0);
}


static void
show_scores_cb (GtkWidget *widget, gpointer data)
{
  gnome_scores_display ("Gnome-Stones", APP_NAME, NULL, (int) data);
}


static void
about_cb (GtkWidget *widget, gpointer data)
{
  GtkWidget *about= NULL;
  
  const gchar *authors[]= {
    "Carsten Schaar <nhadcasc@fs-maphy.uni-hannover.de>",
    NULL
  };
  
  about= gnome_about_new (_("Gnome-Stones"), VERSION,
			  "(C) 1998 Carsten Schaar",
			  authors,
			  _("A game."),
			  NULL);
  gtk_widget_show (about);
}



/*****************************************************************************/
/* Menu definitions */


static GnomeUIInfo game_menu[]= {
  {
    GNOME_APP_UI_ITEM,
    N_("New"), N_("Start a new game"),
    game_start_cb, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW,
    'N', GDK_CONTROL_MASK, NULL
  },
  {
    GNOME_APP_UI_ITEM,
    N_("Scores"), N_("Show highscore table"),
    show_scores_cb, (gpointer) 0, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SCORES,
    0, (GdkModifierType) 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_ITEM,
    N_("Preferences..."), N_("Change Gnome-Stones preferences"),
    preferences_cb, (gpointer) 0, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PREF,
    0, (GdkModifierType) 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_ITEM,
    N_("Exit"), N_("Exit Gnome Stones"),
    quit_cb, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_QUIT,
    'Q', GDK_CONTROL_MASK, NULL
  },
  GNOMEUIINFO_END
};

static GnomeUIInfo help_menu[]= {
  {
    GNOME_APP_UI_ITEM,
    N_("About Gnome-Stones"), N_("Info about Gnome Stones"),
    about_cb, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT,
    0, (GdkModifierType) 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_HELP(APP_NAME),
  GNOMEUIINFO_END
};

static GnomeUIInfo main_menu[]= 
{
  GNOMEUIINFO_SUBTREE(N_("Game"), game_menu),
  GNOMEUIINFO_SUBTREE(N_("Help"), help_menu),
  GNOMEUIINFO_END
};



/*****************************************************************************/
/* Startup sequence.  */

typedef struct
{
  gboolean (*function)(void);
  char     *description;
} StartupSequence;


StartupSequence startup_sequence[]=
{
  {
    objects_register_all,
    N_("Registering game objects...")
  },
  {
    scan_private_game_directories,
    N_("Scanning private game directories...")
  },
  {
    scan_public_game_directories,
    N_("Scanning public game directories...")
  },
  {
    load_default_game,
    N_("Loading the default game...")
  }
};


static gint startup_function (gpointer data)
{
  static guint index= 0;
  static guint max  = 0;
  
  if (max == 0)
    max= sizeof (startup_sequence)/sizeof (startup_sequence[0]);

  gnome_appbar_set_status (GNOME_APPBAR (statusbar), 
			   _(startup_sequence[index].description));
  
  startup_sequence[index].function ();
  
  gnome_appbar_set_progress (GNOME_APPBAR (statusbar),
			     (gfloat) (index+1)/(gfloat) max);
  index++;

  if (index == max)
    {
      state= STATE_TITLE;
      gnome_appbar_refresh (GNOME_APPBAR (statusbar));
    }
  
  return (index != max);
}



/*****************************************************************************/
/* Main function.  */


int
main (int argc, char *argv[])
{
  GtkWidget *frame= NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *table= NULL;
  
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  gnome_score_init (APP_NAME);

  gnome_init (APP_NAME, NULL, argc, argv, 0, NULL);

  /* That's what a gnome application needs:  */
  app= gnome_app_new ("GnomeStones", _("Gnome-Stones"));
  gtk_window_set_policy (GTK_WINDOW (app), FALSE, FALSE, TRUE);

  /* ... a menu line, ... */
  gnome_app_create_menus   (GNOME_APP (app), main_menu);

  /* ... a toolbar, ... */
  /*  gnome_app_create_toolbar (GNOME_APP (app), toolbar); */

  /* ... a statusline ... */
  statusbar= gnome_appbar_new (TRUE, TRUE, GNOME_PREFERENCES_USER);
  gnome_app_set_statusbar (GNOME_APP (app), statusbar);

  gnome_appbar_set_default (GNOME_APPBAR (statusbar), _(default_message));

  /* and, last but not least the game display.  */
  vbox = gtk_vbox_new (FALSE, 0);
  
  game_widget= game_widget_create ();
  
  frame= gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_widget_show (frame);
  gtk_container_add (GTK_CONTAINER (frame), game_widget);

  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  table= status_widget_get ();
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);
  
  gnome_app_set_contents (GNOME_APP (app), vbox);
  
  gtk_widget_set_events (app, gtk_widget_get_events (app) | GAME_EVENTS);

  gtk_signal_connect (GTK_OBJECT (app), "key_press_event",
		      GTK_SIGNAL_FUNC (game_widget_key_press_callback), 0);
  gtk_signal_connect (GTK_OBJECT (app), "key_release_event",
		      GTK_SIGNAL_FUNC (game_widget_key_release_callback), 0);
  gtk_signal_connect (GTK_OBJECT (app), "delete_event",
		      GTK_SIGNAL_FUNC (gtk_widget_destroy), 0);
  gtk_signal_connect (GTK_OBJECT (app), "destroy",
		      GTK_SIGNAL_FUNC (gtk_main_quit), 0);
  
  gtk_widget_show (app);

  load_images ();

  gtk_idle_add (startup_function, NULL);

  gtk_main ();

  return 0;
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
