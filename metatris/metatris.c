/* metatris.c - Guess.
 *
 * Copyright (C) 1999 Chris Lahey.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "config.h"
#include <gtk/gtk.h>
#include <gnome.h>
#include <stdlib.h>
#include <time.h>
#define _GNU_SOURCE
#include <stdio.h>

#include "metatris.h"

gint window_count;

#define GRID_ENTRY( window, x, y ) ( (window)->grid[ (x) + (y) * (window)->width ] )
#define PIECE_COUNT 7

/* #define AA_METATRIS 1 */
#define SHOW_SCORE 1

GdkImlibImage **metatris_images;
GdkImlibImage *metatris_bg;
gchar *current_look_theme_path;
gint color_count;

typedef struct
{
  GnomeCanvasItem *item;
  
  /* This is mostly used to determine which sprite to use.  */
  gint type;
  gint color;
  gint x;
  gint y;
} MetatrisEntry;

typedef struct
{
  MetatrisEntry **entries;
  gint entry_count;
} MetatrisPiece;

typedef struct
{
  GnomeApp *app;
  GnomeCanvas *canvas;
  gint width;
  gint height;
  MetatrisEntry **grid;
  MetatrisPiece *current_piece;
  gint timeout;
  gint score;
  gint line_count;
  gint level;

  GtkWidget *score_label;
} MetatrisWindow;

static void metatris_window_destroy( GtkWidget *widget, gpointer data );
static void file_new_window_callback( GtkWidget *widget, gpointer data );
static void file_new_callback( GtkWidget *widget, gpointer data );
static void file_exit_callback( GtkWidget *widget, gpointer data );
static void help_about_callback( GtkWidget *widget, gpointer data );
static void create_metatris_window( void );

static void
game_scores_callback( GtkWidget *widget, gpointer data )
{
  gnome_scores_display(_("Metatris"), "metatris", NULL, 0);  
}

static GnomeUIInfo filemenu[] =
{
  GNOMEUIINFO_MENU_NEW_WINDOW_ITEM( file_new_window_callback, NULL ),

  GNOMEUIINFO_SEPARATOR,
  
  GNOMEUIINFO_MENU_EXIT_ITEM(file_exit_callback, NULL),

  GNOMEUIINFO_END
};

static GnomeUIInfo gamemenu[] =
{
  GNOMEUIINFO_MENU_NEW_GAME_ITEM(file_new_callback, NULL),

  GNOMEUIINFO_MENU_SCORES_ITEM(game_scores_callback, NULL ),
  
  GNOMEUIINFO_END
};

static GnomeUIInfo helpmenu[] =
{
#if 0
  GNOMEUIINFO_HELP("Metatris"),
#endif
  GNOMEUIINFO_MENU_ABOUT_ITEM(help_about_callback, NULL),
  GNOMEUIINFO_END
};

GnomeUIInfo mainmenu[] = {
  GNOMEUIINFO_MENU_FILE_TREE(filemenu),
  GNOMEUIINFO_MENU_GAME_TREE(gamemenu),
  GNOMEUIINFO_MENU_HELP_TREE(helpmenu),
  GNOMEUIINFO_END
};

static void
metatris_window_destroy( GtkWidget *widget, gpointer data )
{
  MetatrisWindow *metatris = (MetatrisWindow *) data;
  g_free( metatris->grid );
  g_free( metatris );
  
  window_count --;
  if ( window_count <= 0 )
    gtk_exit( 0 );
}

static void
metatris_entry_destroy( MetatrisWindow *metatris, MetatrisEntry *entry )
{
  gtk_object_destroy( GTK_OBJECT( entry->item ) );
  g_free( entry );
}

static void
metatris_init_themes()
{
  gchar *current_look_theme_short;
  gint i;
  gint j;

  gnome_config_push_prefix( "/metatris/Themes/");
  current_look_theme_short = gnome_config_get_string_with_default( "look-theme=" DATADIR "/metatris/default-theme/", NULL );
  gnome_config_pop_prefix();
  gnome_config_sync();

  current_look_theme_path = g_strdup_printf( "=%smetatris.theme=/", current_look_theme_short );

  g_free( current_look_theme_short );
  
  gnome_config_push_prefix( current_look_theme_path );
  color_count = gnome_config_get_int_with_default( "Map/color_count=7", NULL );
  gnome_config_pop_prefix();
  gnome_config_sync();

  metatris_images = g_new( GdkImlibImage *, 256 * color_count );
  for ( j = 0; j < color_count; j++ )
    {
      for ( i = 0; i < 256; i++ )
	{
	  metatris_images[i + 256 * j] = NULL;
	}
    }
}

static void
metatris_init()
{
  srand( time( NULL ) );
#if AA_METATRIS
  metatris_bg = gnome_canvas_load_alpha( MYDATADIR "/metatris-bg.png" );
#else
  metatris_bg = gdk_imlib_load_image( MYDATADIR "/metatris-bg.png" );
#endif
  metatris_init_themes();
}

static GdkImlibImage *
get_image_invalid( gint which, gint color )
{
  if ( metatris_images[ which + 256 *  color ] == NULL )
    {
      gchar *path, *short_path;
      gchar name[17];
      gboolean def;
      gnome_config_push_prefix( "=" MYDATADIR "/metatris.theme=/");

      if ( snprintf( name, 17, "Map/piece-%02x-%02x", which, color ) == -1 )
	{
	  g_error( "Error Will Robinson" );
	}
      
      short_path = gnome_config_get_string_with_default( name, &def );

      if ( def )
	{
	  g_free( short_path );

	  if ( snprintf( name, 17, "Map/piece-%02x-%02x", which, 0 ) == -1 )
	    g_error( "Error Will Robinson" );
	  short_path = gnome_config_get_string_with_default( name, &def );

	}
      
      if ( def )
	{
	  g_free( short_path );

	  if ( snprintf( name, 17, "Map/piece-%02x-%02x", 0, color ) == -1 )
	    g_error( "Error Will Robinson" );
	  short_path = gnome_config_get_string_with_default( name, &def );

	}
      
      if ( def )
	{
	  g_free( short_path );

	  if ( snprintf( name, 17, "Map/piece-%02x-%02x", 0, 0 ) == -1 )
	    g_error( "Error Will Robinson" );
	  short_path = gnome_config_get_string_with_default( name, &def );

	}
      
      path = g_strdup_printf( MYDATADIR "/%s", short_path );
#if AA_METATRIS
      metatris_images[which + 256 * color] = gnome_canvas_load_alpha( path );
#else
      metatris_images[which + 256 * color] = gdk_imlib_load_image( path );
#endif
      g_free( path );

      gnome_config_pop_prefix();
      gnome_config_sync();
      if ( metatris_images[which + 256 * color] == NULL )
	g_error( "Can't find images.\n" );
    }
  return metatris_images[which+ 256 * color];
}

#define get_image( which, color ) ( metatris_images[(which) + 256 * (color)] != NULL ? metatris_images[(which) + 256 * (color)] : get_image_invalid( (which), (color) ) )

static void file_new_window_callback( GtkWidget *widget, gpointer data )
{
  create_metatris_window();
}

static MetatrisPiece *
create_metatris_piece( MetatrisWindow *metatris )
{
  MetatrisPiece *piece = g_new( MetatrisPiece, 1 );
  int i;
  piece->entries = g_new( MetatrisEntry *, 4 );
  piece->entry_count = 4;
  for ( i = 0; i < piece->entry_count; i++ )
    {
      piece->entries[i] = g_new( MetatrisEntry, 1 );
      piece->entries[i]->item = NULL;
      piece->entries[i]->type = 0;
      piece->entries[i]->color = 0;
      piece->entries[i]->x = 0;
      piece->entries[i]->y = 0;
    }
  return piece;
}

static void
destroy_metatris_piece( MetatrisWindow *metatris, MetatrisPiece *piece )
{
  g_free( piece->entries );
  g_free( piece );
}

static void
show_metatris_piece( MetatrisWindow *metatris, MetatrisPiece *piece )
{
  GnomeCanvasGroup *root = gnome_canvas_root (GNOME_CANVAS (metatris->canvas));
  int i;

  for( i = 0; i < piece->entry_count; i++ )
    {
      piece->entries[i]->item
	= gnome_canvas_item_new (root,
				 gnome_canvas_image_get_type (),
				 "image", get_image( piece->entries[i]->type, piece->entries[i]->color ),
				 "x", (double) piece->entries[i]->x,
				 "y", (double) piece->entries[i]->y,
				 "width", (double) 1,
				 "height", (double) 1,
				 "anchor", GTK_ANCHOR_NW,
				 NULL);
    }
}

static gboolean
check_piece_existance_possibility( MetatrisWindow *metatris, MetatrisPiece *piece )
{
  if ( piece )
    {
      int i;
      for ( i = 0; i < piece->entry_count; i++ )
	{
	  MetatrisEntry *entry = piece->entries[i];
	  if ( entry->y >= metatris->height ||
	       GRID_ENTRY( metatris, entry->x, entry->y ) != NULL )
	    {
	      return FALSE;
	    }
	}
      return TRUE;
    }
  else
    return FALSE;
}

static void
update_score( MetatrisWindow *metatris )
{
#if SHOW_SCORE
  gchar new_text[512];
  snprintf( new_text, 512, "%d", metatris->score );
  gtk_label_set_text( GTK_LABEL( metatris->score_label ), new_text );
#endif
}


static void
end_game( MetatrisWindow *metatris )
{
  gint pos;
  if ( metatris->timeout )
    gtk_timeout_remove( metatris->timeout );
  metatris->timeout = 0;
  pos = gnome_score_log( metatris->score, NULL, TRUE );
  gnome_scores_display(_("Metatris"), "metatris", NULL, pos);
}

static gint drop_piece_callback( gpointer data );


static void
add_piece( MetatrisWindow *metatris )
{
  MetatrisPiece *piece = create_metatris_piece( metatris );
  gint which;
  gint i;

  which = rand() % PIECE_COUNT;

  switch ( which )
    {
    case 0:
      piece->entries[0]->x = 4;
      piece->entries[0]->y = 1;
      piece->entries[0]->type = 0x8a;
      
      piece->entries[1]->x = 3;
      piece->entries[1]->y = 1;
      piece->entries[1]->type = 0x0c;
      
      piece->entries[2]->x = 5;
      piece->entries[2]->y = 1;
      piece->entries[2]->type = 0x81;
      
      piece->entries[3]->x = 4;
      piece->entries[3]->y = 0;
      piece->entries[3]->type = 0x70;
      break; 
   case 1:
      piece->entries[0]->x = 4;
      piece->entries[0]->y = 0;
      piece->entries[0]->type = 0x88;
      
      piece->entries[1]->x = 3;
      piece->entries[1]->y = 0;
      piece->entries[1]->type = 0x08;
      
      piece->entries[2]->x = 5;
      piece->entries[2]->y = 0;
      piece->entries[2]->type = 0x88;
      
      piece->entries[3]->x = 6;
      piece->entries[3]->y = 0;
      piece->entries[3]->type = 0x80;
      break;
    case 2:
      piece->entries[0]->x = 4;
      piece->entries[0]->y = 1;
      piece->entries[0]->type = 0x89;
      
      piece->entries[1]->x = 3;
      piece->entries[1]->y = 1;
      piece->entries[1]->type = 0x0a;
      
      piece->entries[2]->x = 5;
      piece->entries[2]->y = 1;
      piece->entries[2]->type = 0x80;
      
      piece->entries[3]->x = 3;
      piece->entries[3]->y = 0;
      piece->entries[3]->type = 0x30;
      break;
    case 3:
      piece->entries[0]->x = 4;
      piece->entries[0]->y = 1;
      piece->entries[0]->type = 0x8c;
      
      piece->entries[1]->x = 3;
      piece->entries[1]->y = 1;
      piece->entries[1]->type = 0x08;
      
      piece->entries[2]->x = 5;
      piece->entries[2]->y = 1;
      piece->entries[2]->type = 0x82;
      
      piece->entries[3]->x = 5;
      piece->entries[3]->y = 0;
      piece->entries[3]->type = 0x60;
      break;
    case 4:
      piece->entries[0]->x = 4;
      piece->entries[0]->y = 0;
      piece->entries[0]->type = 0xb0;
      
      piece->entries[1]->x = 3;
      piece->entries[1]->y = 0;
      piece->entries[1]->type = 0x18;
      
      piece->entries[2]->x = 4;
      piece->entries[2]->y = 1;
      piece->entries[2]->type = 0x0b;
      
      piece->entries[3]->x = 5;
      piece->entries[3]->y = 1;
      piece->entries[3]->type = 0x81;
      break;
    case 5:
      piece->entries[0]->x = 4;
      piece->entries[0]->y = 0;
      piece->entries[0]->type = 0x68;
      
      piece->entries[1]->x = 5;
      piece->entries[1]->y = 0;
      piece->entries[1]->type = 0xc0;
      
      piece->entries[2]->x = 3;
      piece->entries[2]->y = 1;
      piece->entries[2]->type = 0x0c;
      
      piece->entries[3]->x = 4;
      piece->entries[3]->y = 1;
      piece->entries[3]->type = 0x86;
      break;
    case 6:
      piece->entries[0]->x = 4;
      piece->entries[0]->y = 0;
      piece->entries[0]->type = 0x38;
      
      piece->entries[1]->x = 5;
      piece->entries[1]->y = 0;
      piece->entries[1]->type = 0xe0;
      
      piece->entries[2]->x = 4;
      piece->entries[2]->y = 1;
      piece->entries[2]->type = 0x0e;
      
      piece->entries[3]->x = 5;
      piece->entries[3]->y = 1;
      piece->entries[3]->type = 0x83;
      break;
    }

  for( i = 0; i < piece->entry_count; i++ )
    {
      piece->entries[i]->color = which % color_count;
    }
  
  if ( ! check_piece_existance_possibility( metatris, piece ) )
    {
      end_game( metatris );
    }
  else
    {
      metatris->current_piece = piece;
      show_metatris_piece( metatris, piece );
      if ( metatris->timeout )
	gtk_timeout_remove( metatris->timeout );
      metatris->timeout = gtk_timeout_add( 1000 / metatris->level, drop_piece_callback, metatris );
    }
}

static void
check_for_lines( MetatrisWindow *metatris )
{
  gint i, j;
  gboolean full;
  int fullcount = 0;
  gint points;
  for ( i = 0; i < metatris->height; i++ )
    {
      full = TRUE;
      for ( j = 0; j < metatris->width; j++ )
	{
	  if ( GRID_ENTRY( metatris, j, i ) == NULL )
	    {
	      full = FALSE;
	      break;
	    }
	}
      if ( full )
	{
	  int k;
	  fullcount ++;
	  metatris->line_count ++;
	  metatris->level = MAX( metatris->level, metatris->line_count / 10 + 1 );
	  for ( j = 0; j < metatris->width; j++ )
	    {
	      metatris_entry_destroy( metatris, GRID_ENTRY( metatris, j, i ) );
	      GRID_ENTRY( metatris, j, i ) = NULL;
	      if ( i > 0 && GRID_ENTRY( metatris, j, i - 1 ) )
		{
		  MetatrisEntry *entry = GRID_ENTRY( metatris, j, i - 1 );
		  gint oldtype = entry->type;
		  entry->type -= 0x20 & entry->type;
		  entry->type -= 0x10 & entry->type;
		  entry->type -= 0x40 & entry->type;
		  if ( entry->type != oldtype )
		    {
		      gnome_canvas_item_set( entry->item,
					     "image", get_image( entry->type, entry->color ),
					     NULL);
		    }
		}
	      if ( i < metatris->height - 1 && GRID_ENTRY( metatris, j, i + 1 ) )
		{
		  MetatrisEntry *entry = GRID_ENTRY( metatris, j, i + 1 );
		  gint oldtype = entry->type;
		  entry->type -= 0x02 & entry->type;
		  entry->type -= 0x01 & entry->type;
		  entry->type -= 0x04 & entry->type;
		  if ( entry->type != oldtype )
		    {
		      gnome_canvas_item_set( entry->item,
					     "image", get_image( entry->type, entry->color ),
					     NULL);
		    }
		}
	    }
	  for( k = i; k > 0; k-- )
	    {
	      for ( j= 0; j < metatris->width; j++ )
		{
		  GRID_ENTRY( metatris, j, k ) = GRID_ENTRY( metatris, j, k - 1 );
		  if ( GRID_ENTRY( metatris, j, k ) && GRID_ENTRY( metatris, j, k )->item != NULL )
		    {
		      gnome_canvas_item_move( GRID_ENTRY( metatris, j, k )->item, 0, 1 );
		      GRID_ENTRY( metatris, j, k )->y ++;
		    }
		}
	    }
	  for ( j = 0; j < metatris->width; j++ )
	    {
	      GRID_ENTRY( metatris, j, 0 ) = NULL;
	    }
	}
    }
  if ( fullcount > 0 )
    {
      points = metatris->level * 100;
      fullcount --;
      for ( ; fullcount > 0; fullcount -- )
	points *= 3;
      metatris->score += points;
      update_score( metatris );
    }
}

static void
land_piece( MetatrisWindow *metatris )
{
  if ( metatris->current_piece )
    {
      int i;
      for ( i = 0; i < metatris->current_piece->entry_count; i++ )
	{
	  MetatrisEntry *entry = metatris->current_piece->entries[ i ];
	  GRID_ENTRY( metatris, entry->x, entry->y ) = entry;
	}
      destroy_metatris_piece( metatris, metatris->current_piece );
      metatris->current_piece = NULL;
    }
  check_for_lines( metatris );
  if ( metatris->timeout )
    gtk_timeout_remove( metatris->timeout );
  metatris->timeout = 0;
}

static gint
move_piece( MetatrisWindow *metatris, gint x, gint y )
{
  if ( metatris->current_piece )
    {
      int i;
      for ( i = 0; i < metatris->current_piece->entry_count; i++ )
	{
	  MetatrisEntry *entry = metatris->current_piece->entries[i];
	  if ( entry->x + x >= metatris->width ||
	       entry->x + x < 0 ||
	       entry->y + y >= metatris->height ||
	       entry->y + y < 0 ||
	       GRID_ENTRY( metatris, entry->x + x, entry->y + y ) != NULL )
	    {
	      return FALSE;
	    }
	}
      for ( i = 0; i < metatris->current_piece->entry_count; i++ )
	{
	  MetatrisEntry *entry = metatris->current_piece->entries[i];
	  entry->x += x;
	  entry->y += y;
	  gnome_canvas_item_move( entry->item, x, y );
	}
    }
  return TRUE;
}

static gint
drop_piece ( MetatrisWindow * metatris )
{
  if ( ! move_piece( metatris, 0, 1 ) )
    {
      land_piece( metatris );
      add_piece( metatris );
      return FALSE;
    }
  else
    return TRUE;
}

static gint
drop_piece_callback( gpointer data )
{
  MetatrisWindow *metatris = (MetatrisWindow *) data;
  if ( drop_piece( metatris ) )
    return TRUE;
  return FALSE;
}

static void
clear_board( MetatrisWindow *metatris )
{
  gint i, j;
  for ( i = 0; i < metatris->width; i++ )
    {
      for ( j = 0; j < metatris->height; j++ )
	{
	  if ( GRID_ENTRY( metatris, i, j ) )
	    metatris_entry_destroy( metatris, GRID_ENTRY( metatris, i, j ) );
	  GRID_ENTRY( metatris, i, j ) = NULL;
	}
    }
}

static gint
new_x( gint x, gint y, gint centx, gint centy, gint quarters )
{
  x -= centx;
  y -= centy;
  switch( quarters % 4 )
    {
    case 0:
      /* Nothing. */
      break;
    case 1:
      x = -y;
      break;
    case 2:
      x = -x;
      break;
    case 3:
      x = y;
      break;
    }
  return x + centx;
}

static gint
new_y( gint x, gint y, gint centx, gint centy, gint quarters )
{
  x -= centx;
  y -= centy;
  switch( quarters % 4 )
    {
    case 0:
      /* Nothing. */
      break;
    case 1:
      y = x;
      break;
    case 2:
      y = -y;
      break;
    case 3:
      y = -x;
      break;
    }
  return y + centy;
}

static gint
new_type( gint oldtype, gint quarters )
{
  gint i;
  
  if ( oldtype == 0xff )
    return 0xff;
  
  quarters %= 4;
  for ( i = 0; i < quarters; i++ )
    {
      oldtype <<= 2;
      oldtype %= 255;
    }
  return oldtype;
}

static void
rotate_piece( MetatrisWindow *metatris )
{
  MetatrisPiece *piece = metatris->current_piece;
  if ( piece )
    {
      int i;
      MetatrisEntry *rot = piece->entries[0];
      for ( i = 0; i < piece->entry_count; i++ )
	{
	  MetatrisEntry *entry = piece->entries[i];
	  gint newx = new_x( entry->x, entry->y, rot->x, rot->y, 1 );
	  gint newy = new_y( entry->x, entry->y, rot->x, rot->y, 1 );
	  if ( newx >= metatris->width ||
	       newx < 0 ||
	       newy >= metatris->height ||
	       newy < 0 ||
	       GRID_ENTRY( metatris, newx, newy ) != NULL )
	    {
	      return;
	    }
	}
      for ( i = 1; i < piece->entry_count; i++ )
	{
	  MetatrisEntry *entry = metatris->current_piece->entries[i];
	  gint newx = new_x( entry->x, entry->y, rot->x, rot->y, 1 );
	  gint newy = new_y( entry->x, entry->y, rot->x, rot->y, 1 );
	  gnome_canvas_item_move( entry->item, newx - entry->x, newy - entry->y );
	  entry->x = newx;
	  entry->y = newy;
	}
      for ( i = 0; i < piece->entry_count; i++ )
	{
	  MetatrisEntry *entry = metatris->current_piece->entries[i];
	  gint newtype = new_type( entry->type, 1 );
	  if ( entry->type != newtype )
	    {
	      entry->type = newtype;
	      gnome_canvas_item_set( entry->item,
				 "image", get_image( entry->type, entry->color ),
				 NULL);

	    }
	}
    }
}

static gint
metatris_key_press( GtkWidget *widget, GdkEventKey *event, gpointer data )
{
  gint key = event->keyval;
  gboolean return_val = FALSE;
  MetatrisWindow *metatris = (MetatrisWindow *) data;
  
  switch ( key )
    {
    case GDK_Left:
    case GDK_KP_Left:
      move_piece( metatris, -1, 0 );
      return_val = TRUE;
      break;
    case GDK_Right:
    case GDK_KP_Right:
      move_piece( metatris, 1, 0 );
      return_val = TRUE;
      break;
    case GDK_Up:
    case GDK_KP_Up:
      rotate_piece( metatris );
      return_val = TRUE;
      break;
    case GDK_Down:
    case GDK_KP_Down:
      if ( metatris->timeout )
	{
	  gtk_timeout_remove( metatris->timeout );
	  metatris->timeout = 0;
	}
      while( drop_piece( metatris ) )
	{
	  metatris->score += metatris->level;
	}
      update_score( metatris );
      return_val = TRUE;
      break;
    }
  return return_val;  
}

static void start_game( MetatrisWindow *metatris )
{
  gint i;
  clear_board( metatris );
  metatris->score = 0;
  metatris->line_count = 0;
  metatris->level = 1;
  if ( metatris->current_piece )
    {
      for ( i = 0; i < metatris->current_piece->entry_count; i++ )
	{
	  metatris_entry_destroy( metatris, metatris->current_piece->entries[i] );
	  metatris->current_piece->entries[i] = NULL;
	}
      destroy_metatris_piece( metatris, metatris->current_piece );
      metatris->current_piece = NULL;
    }
  add_piece( metatris );
}

static void file_new_callback( GtkWidget *widget, gpointer data )
{
  start_game( (MetatrisWindow *) data);
}

static void file_exit_callback( GtkWidget *widget, gpointer data )
{
  gtk_exit( 0 );
}

static void
metatris_size_allocate( GtkWidget *widget, GtkAllocation *allocation, gpointer data )
{
  GnomeCanvas *canvas = GNOME_CANVAS( widget );
  MetatrisWindow *metatris = (MetatrisWindow *) data;
  gnome_canvas_set_pixels_per_unit( canvas, ((double) allocation->height) / ((double) metatris->height) );
}

static gint
reset_usize( gpointer data )
{
  gtk_widget_set_usize( GTK_WIDGET( data ), 0, 0 );
  return FALSE;
}

static void
create_metatris_window( void )
{
  GtkWidget *widget;
  GtkWindow *window;
  GnomeApp *app;
  MetatrisWindow *metatris = g_new( MetatrisWindow, 1 );
  GtkWidget *main_vbox;
  GtkWidget *score_hbox;
  GtkWidget *canvas;
  gint i;

  window_count ++;
  
  widget = gnome_app_new( "Metatris", "Metatris" );
  window = GTK_WINDOW( widget );
  app = GNOME_APP( widget );

  metatris->app = app;
  metatris->width = 10;
  metatris->height = 20;
  metatris->current_piece = NULL;
  metatris->timeout = 0;
  metatris->score = 0;
  metatris->line_count = 0;
  metatris->level = 1;

  metatris->grid = g_new( MetatrisEntry *, metatris->height * metatris->width );
  for ( i = 0; i < metatris->height * metatris->width; i++ )
    {
      metatris->grid[ i ] = NULL;
    }

  gnome_app_create_menus_with_data( app, mainmenu, (gpointer) metatris );

  main_vbox = gtk_vbox_new( FALSE, 0 );
#if AA_METATRIS
  gtk_widget_push_visual( gdk_rgb_get_visual() );
  gtk_widget_push_colormap( gdk_rgb_get_cmap() );
  canvas = gnome_canvas_new_aa();
  metatris->canvas = GNOME_CANVAS( canvas );
  gtk_widget_pop_visual();
  gtk_widget_pop_colormap();
#else
  gtk_widget_push_visual( gdk_imlib_get_visual() );
  gtk_widget_push_colormap( gdk_imlib_get_colormap() );
  canvas = gnome_canvas_new();
  metatris->canvas = GNOME_CANVAS( canvas );
  gtk_widget_pop_visual();
  gtk_widget_pop_colormap();
#endif
  
  gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (metatris->canvas) ),
			 gnome_canvas_image_get_type (),
			 "image", metatris_bg,
			 "x", (double) 0,
			 "y", (double) 0,
			 "width", (double) metatris->width,
			 "height", (double) metatris->height,
			 "anchor", GTK_ANCHOR_NW,
			 NULL);
  
  gtk_widget_set_usize( canvas, 320, 640 );
  gnome_canvas_set_scroll_region( GNOME_CANVAS( canvas ), 0, 0, metatris->width, metatris->height );
  gnome_canvas_set_pixels_per_unit( GNOME_CANVAS( canvas ), 32 );

  gtk_signal_connect( GTK_OBJECT( canvas ),
		      "size_allocate",
		      GTK_SIGNAL_FUNC( metatris_size_allocate ),
		      (gpointer) metatris );

  gtk_signal_connect( GTK_OBJECT( app ),
		      "destroy",
		      GTK_SIGNAL_FUNC( metatris_window_destroy ),
		      (gpointer) metatris );

  gtk_signal_connect( GTK_OBJECT( app ),
		      "key_press_event",
		      GTK_SIGNAL_FUNC( metatris_key_press ),
		      (gpointer) metatris );

  
  gtk_box_pack_start( GTK_BOX( main_vbox ), canvas, TRUE, TRUE, 0 );

#if SHOW_SCORE
  score_hbox = gtk_hbox_new( FALSE, 0 );
  gtk_box_pack_start( GTK_BOX( main_vbox ), score_hbox, FALSE, FALSE, 0 );
  gtk_container_set_resize_mode ( GTK_CONTAINER( score_hbox ), GTK_RESIZE_QUEUE);
  metatris->score_label = gtk_label_new( "0" );
  gtk_box_pack_start( GTK_BOX( score_hbox ), metatris->score_label, TRUE, TRUE, 0 );
#endif

  /* Add the main vbox to the window */
  gnome_app_set_contents( app, main_vbox );

  gtk_widget_show_all( widget );
  gtk_timeout_add( 0, reset_usize, canvas );
  
}

int main( int argc, char *argv[])
{

  gnome_score_init ("metatris");

  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  gnome_init( "Metatris", VERSION, argc, argv );

  metatris_init();
 
  window_count = 0;
  
  create_metatris_window();
  gtk_main();

  return 0;
}

static void help_about_callback( GtkWidget *widget, gpointer data )
{
  const gchar *authors[] =
  {
    "Christopher James Lahey <clahey@umich.edu>",
    NULL
  };
     
  GtkWidget *about =
    gnome_about_new ( _("Metatris"), "0.1.0",
		      _("Copyright (C) 1999, Christopher James Lahey"),
		      authors,
		      _( "Metatris, the Gnome Pattern Matching System.  Enjoy." ),
		      MYDATADIR "metatris.png");
  gtk_widget_show (about);                                            
}
