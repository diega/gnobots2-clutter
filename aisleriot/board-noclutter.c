/*
 * Copyright © 1998, 2003 Jonathan Blandford <jrb@mit.edu>
 * Copyright © 2007, 2008 Christian Persch
 *
 * Some code copied from gtk+/gtk/gtkiconview (LGPL2+):
 * Copyright © 2002, 2004  Anders Carlsson <andersca@gnu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <libgames-support/games-card-images.h>
#include <libgames-support/games-files.h>
#include <libgames-support/games-marshal.h>
#include <libgames-support/games-pixbuf-utils.h>
#include <libgames-support/games-runtime.h>
#include <libgames-support/games-sound.h>

#include "conf.h"

#include "game.h"

#include "board.h"

#define AISLERIOT_BOARD_GET_PRIVATE(board)(G_TYPE_INSTANCE_GET_PRIVATE ((board), AISLERIOT_TYPE_BOARD, AisleriotBoardPrivate))

/* Enable keynav on non-hildon by default */
#if !defined(HAVE_HILDON) && !defined(DISABLE_KEYNAV)
#define ENABLE_KEYNAV
#endif /* !HAVE_HILDON */

/* The minimum size for the playing area. Almost completely arbitrary. */
#define BOARD_MIN_WIDTH 300
#define BOARD_MIN_HEIGHT 200

/* The limits for how much overlap there is between cards and
 * how much is allowed to slip off the bottom or right.
 */
#define MIN_DELTA (0.05)
#define MAX_DELTA (0.2)
#define MAX_OVERHANG (0.2)

/* The proportion of a slot dedicated to the card (horiz or vert). */
#ifndef DEFAULT_CARD_SLOT_RATIO
#ifdef HAVE_HILDON
#define DEFAULT_CARD_SLOT_RATIO (0.9)
#else
#define DEFAULT_CARD_SLOT_RATIO (0.8)
#endif
#endif /* !DEFAULT_CARD_SLOT_RATIO */

#define DOUBLE_TO_INT_CEIL(d) ((int) (d + 0.5))

#define STATIC_ASSERT(condition) STATIC_ASSERT_IMPL(condition, __LINE__)
#define STATIC_ASSERT_IMPL(condition, line) STATIC_ASSERT_IMPL2(condition, line)
#define STATIC_ASSERT_IMPL2(condition, line) typedef int _static_assert_line_##line[(condition) ? 1 : -1]

#ifdef HAVE_HILDON 
#define PIXBUF_DRAWING_LIKELIHOOD(cond) G_UNLIKELY (cond)
#else
#define PIXBUF_DRAWING_LIKELIHOOD(cond) (cond)
#endif /* HAVE_HILDON */

#if GLIB_CHECK_VERSION (2, 10, 0)
#define I_(string) g_intern_static_string (string)
#else
#define I_(string) string
#endif

typedef enum {
  CURSOR_DEFAULT,
  CURSOR_OPEN,
  CURSOR_CLOSED,
  CURSOR_DROPPABLE,
  LAST_CURSOR
} CursorType;

typedef enum {
  STATUS_NONE,
  STATUS_MAYBE_DRAG,
  STATUS_NOT_DRAG,
  STATUS_IS_DRAG,
  STATUS_SHOW,
  LAST_STATUS
} MoveStatus;

struct _AisleriotBoardPrivate
{
  AisleriotGame *game;

  GdkGC *draw_gc;
  GdkGC *bg_gc;
  GdkGC *slot_gc;
  GdkCursor *cursor[LAST_CURSOR];

  GamesCardTheme *theme;
  CardSize card_size;

  double width;
  double height;

  /* The size of a slot in pixels. */
  double xslotstep;
  double yslotstep;

  /* How much of the slot the card should take up */
  double card_slot_ratio;

  /* The offset of the cards within the slot. */
  int xoffset, yoffset;

  /* The offset within the window. */
  int xbaseoffset;

  /* Cards cache */
  GamesCardImages *images;

  /* Slot */
  gpointer slot_image; /* either a GdkPixbuf or GdkPixmap, depending on drawing mode */

  /* Button press */
  int last_click_x;
  int last_click_y;
  int double_click_time;
  guint32 last_click_time;

  /* Moving cards */
  Slot *moving_cards_origin_slot;
  int moving_cards_origin_card_id; /* The index of the card that was clicked on in hslot->cards; or -1 if the click wasn't on a card */
  GdkWindow *moving_cards_window;
  GByteArray *moving_cards;

  /* The 'reveal card' action's slot and card link */
  Slot *show_card_slot;
  int show_card_id;

  /* Click data */
  Slot *last_clicked_slot;
  int last_clicked_card_id;

  /* Focus handling */
  Slot *focus_slot;
  int focus_card_id; /* -1 for focused empty slot */
  int focus_line_width;
  int focus_padding;
  GdkRectangle focus_rect;

  /* Selection */
  Slot *selection_slot;
  int selection_start_card_id;
  GdkRectangle selection_rect;
  GdkColor selection_colour;

  /* Highlight */
  Slot *highlight_slot;

#ifdef HAVE_MAEMO
  /* Tap-and-Hold */
  Slot *tap_and_hold_slot;
  int tap_and_hold_card_id;
#endif

  /* Bit field */
  guint droppable_supported : 1;
  guint touchscreen_mode : 1;
  guint use_pixbuf_drawing : 1;
  guint show_focus : 1; /* whether the focus is drawn */
  guint interior_focus : 1;

  guint click_to_move : 1;

  guint geometry_set : 1;
  guint is_rtl : 1;

  guint last_click_left_click : 1;
  guint click_status : 4; /* enough bits for MoveStatus */

  guint show_selection : 1;
  guint show_highlight : 1;

  guint force_geometry_update : 1;
};

STATIC_ASSERT (LAST_STATUS < 16 /* 2^4 */);

enum
{
  PROP_0,
  PROP_GAME,
  PROP_THEME
};

#ifdef ENABLE_KEYNAV
enum
{
  ACTIVATE,
  MOVE_CURSOR,
  TOGGLE_SELECTION,
  SELECT_ALL,
  DESELECT_ALL,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];
#endif /* ENABLE_KEYNAV */

static void get_slot_and_card_from_point (AisleriotBoard *board,
                                          int x,
                                          int y,
                                          Slot **slot,
                                          int *_cardid);
static void slot_update_card_images      (AisleriotBoard *board,
                                          Slot *slot);
static void slot_update_card_images_full (AisleriotBoard *board,
                                          Slot *slot,
                                          int highlight_start_card_id);

#ifndef HAVE_HILDON 

/* These cursors borrowed from EOG */
/* FIXMEchpe use themeable cursors here! */
#define hand_closed_data_width 20
#define hand_closed_data_height 20
static const char hand_closed_data_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x80, 0x3f, 0x00,
  0x80, 0xff, 0x00, 0x80, 0xff, 0x00, 0xb0, 0xff, 0x00, 0xf0, 0xff, 0x00,
  0xe0, 0xff, 0x00, 0xe0, 0x7f, 0x00, 0xc0, 0x7f, 0x00, 0x80, 0x3f, 0x00,
  0x00, 0x3f, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define hand_closed_mask_width 20
#define hand_closed_mask_height 20
static const char hand_closed_mask_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x80, 0x3f, 0x00, 0xc0, 0xff, 0x00,
  0xc0, 0xff, 0x01, 0xf0, 0xff, 0x01, 0xf8, 0xff, 0x01, 0xf8, 0xff, 0x01,
  0xf0, 0xff, 0x01, 0xf0, 0xff, 0x00, 0xe0, 0xff, 0x00, 0xc0, 0x7f, 0x00,
  0x80, 0x7f, 0x00, 0x80, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define hand_open_data_width 20
#define hand_open_data_height 20
static const char hand_open_data_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
  0x60, 0x36, 0x00, 0x60, 0x36, 0x00, 0xc0, 0x36, 0x01, 0xc0, 0xb6, 0x01,
  0x80, 0xbf, 0x01, 0x98, 0xff, 0x01, 0xb8, 0xff, 0x00, 0xf0, 0xff, 0x00,
  0xe0, 0xff, 0x00, 0xe0, 0x7f, 0x00, 0xc0, 0x7f, 0x00, 0x80, 0x3f, 0x00,
  0x00, 0x3f, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define hand_open_mask_width 20
#define hand_open_mask_height 20
static const char hand_open_mask_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x3f, 0x00,
  0xf0, 0x7f, 0x00, 0xf0, 0x7f, 0x01, 0xe0, 0xff, 0x03, 0xe0, 0xff, 0x03,
  0xd8, 0xff, 0x03, 0xfc, 0xff, 0x03, 0xfc, 0xff, 0x01, 0xf8, 0xff, 0x01,
  0xf0, 0xff, 0x01, 0xf0, 0xff, 0x00, 0xe0, 0xff, 0x00, 0xc0, 0x7f, 0x00,
  0x80, 0x7f, 0x00, 0x80, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#endif /* !HAVE_HILDON */

/* Cursor */

#ifndef HAVE_HILDON 

static GdkCursor *
make_cursor (GtkWidget *widget,
             const char *data,
             const char *mask_data)
{
  const GdkColor fg = { 0, 65535, 65535, 65535 };
  const GdkColor bg = { 0, 0, 0, 0 };
  GdkPixmap *source;
  GdkPixmap *mask;
  GdkCursor *cursor;

  /* Yeah, hard-coded sizes are bad. */
  source = gdk_bitmap_create_from_data (widget->window, data, 20, 20);
  mask = gdk_bitmap_create_from_data (widget->window, mask_data, 20, 20);

  cursor = gdk_cursor_new_from_pixmap (source, mask, &fg, &bg, 10, 10);

  g_object_unref (source);
  g_object_unref (mask);

  return cursor;
}

#endif /* !HAVE_HILDON */

static void
set_cursor (AisleriotBoard *board,
            CursorType cursor)
{
#ifndef HAVE_HILDON 
  AisleriotBoardPrivate *priv = board->priv;

  gdk_window_set_cursor (GTK_WIDGET (board)->window,
                         priv->cursor[cursor]);
#endif /* !HAVE_HILDON */
}

/* If we are over a slot, set the cursor to the given cursor,
 * otherwise use the default cursor. */
static void
set_cursor_by_location (AisleriotBoard *board,
                        int x,
                        int y)
{
#ifndef HAVE_HILDON 
  AisleriotBoardPrivate *priv = board->priv;
  Slot *selection_slot = priv->selection_slot;
  int selection_start_card_id = priv->selection_start_card_id;
  Slot *slot;
  int card_id;
  gboolean drop_valid = FALSE;
  CursorType cursor = CURSOR_DEFAULT;

  get_slot_and_card_from_point (board, x, y, &slot, &card_id);

  if (priv->click_to_move &&
      slot != NULL &&
      selection_slot != NULL &&
      slot != selection_slot &&
      selection_start_card_id >= 0) {
    g_return_if_fail (selection_slot->cards->len > selection_start_card_id);

    drop_valid = aisleriot_game_drop_valid (priv->game,
                                            selection_slot->id,
                                            slot->id,
                                            selection_slot->cards->data + selection_start_card_id,
                                            selection_slot->cards->len - selection_start_card_id);
  }
  /* FIXMEchpe: special cursor when _drag_ is possible? */

  if (drop_valid) {
    cursor = CURSOR_DROPPABLE;
  } else if (slot != NULL &&
             card_id >= 0 &&
             !CARD_GET_FACE_DOWN (CARD (slot->cards->data[card_id]))) {
    if (priv->click_status == STATUS_NONE) {
      cursor = CURSOR_OPEN;
    } else {
      cursor = CURSOR_CLOSED;
    }
  }

  set_cursor (board, cursor);
#endif /* !HAVE_HILDON */
}

/* card drawing functions */

static void
set_background_from_baize (GtkWidget *widget,
                           GdkGC *gc)
{
  GError *error = NULL;
  GdkPixbuf *pixbuf;
  GdkPixmap *pixmap;
  char *path;
  int width, height;

  path = games_runtime_get_file (GAMES_RUNTIME_PIXMAP_DIRECTORY, "baize.png");

  pixbuf = gdk_pixbuf_new_from_file (path, &error);
  g_free (path);
  if (error) {
    g_warning ("Failed to load the baize pixbuf: %s\n", error->message);
    g_error_free (error);
    return;
  }

  g_assert (pixbuf != NULL);

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  pixmap = gdk_pixmap_new (widget->window, width, height, -1);
  if (!pixmap) {
    g_object_unref (pixbuf);
    return;
  }

  /* FIXMEchpe: if we ever use a baize with an alpha channel,
   * clear the pixmap first!
   */
  gdk_draw_pixbuf (pixmap, NULL, pixbuf,
                   0, 0, 0, 0, width, height,
                   GDK_RGB_DITHER_NORMAL, 0, 0);

  gdk_gc_set_tile (gc, pixmap);
  gdk_gc_set_fill (gc, GDK_TILED);

  g_object_unref (pixbuf);
  g_object_unref (pixmap);
}

/* Slot helpers */

static void
get_slot_and_card_from_point (AisleriotBoard *board,
                              int x,
                              int y,
                              Slot **slot,
                              int *_cardid)
{
  AisleriotBoardPrivate *priv = board->priv;
  GPtrArray *slots;
  gboolean got_slot = FALSE;
  int num_cards;
  int i, n_slots;
  int cardid;

  *slot = NULL;
  cardid = -1;

  slots = aisleriot_game_get_slots (priv->game);

  n_slots = slots->len;
  for (i = n_slots - 1; i >= 0; --i) {
    Slot *hslot = slots->pdata[i];

    /* if point is within our rectangle */
    if (hslot->rect.x <= x && x <= hslot->rect.x + hslot->rect.width &&
        hslot->rect.y <= y && y <= hslot->rect.y + hslot->rect.height) {
      num_cards = hslot->cards->len;

      if (got_slot == FALSE || num_cards > 0) {
        /* if we support exposing more than one card,
         * find the exact card  */

        gint depth = 1;

        if (hslot->pixeldx > 0)
          depth += (x - hslot->rect.x) / hslot->pixeldx;
        else if (hslot->pixeldx < 0)
          depth += (hslot->rect.x + hslot->rect.width - x) / -hslot->pixeldx;
        else if (hslot->pixeldy > 0)
          depth += (y - hslot->rect.y) / hslot->pixeldy;

        /* account for the last card getting much more display area
         * or no cards */

        if (depth > hslot->exposed)
          depth = hslot->exposed;
        *slot = hslot;

        /* card = #cards in slot + card chosen (indexed in # exposed cards) - # exposed cards */

        cardid = num_cards + depth - hslot->exposed;

        /* this is the topmost slot with a card */
        /* take it and run */
        if (num_cards > 0)
          break;

        got_slot = TRUE;
      }
    }
  }

  *_cardid = cardid > 0 ? cardid - 1 : -1;
}

#ifdef ENABLE_KEYNAV

static gboolean
test_slot_projection_intersects_x (Slot *slot,
                                   int x_start,
                                   int x_end)
{
  return slot->rect.x <= x_end &&
         slot->rect.x + slot->rect.width >= x_start;
}

static int
get_slot_index_from_slot (AisleriotBoard *board,
                          Slot *slot)
{
  AisleriotBoardPrivate *priv = board->priv;
  GPtrArray *slots;
  guint n_slots;
  int slot_index;

  g_assert (slot != NULL);

  slots = aisleriot_game_get_slots (priv->game);
  n_slots = slots->len;
  g_assert (n_slots > 0);

  for (slot_index = 0; slot_index < n_slots; ++slot_index) {
    if (g_ptr_array_index (slots, slot_index) == slot)
      break;
  }

  g_assert (slot_index < n_slots); /* the slot EXISTS after all */

  return slot_index;
}

#endif /* ENABLE_KEYNAV */

static void
get_rect_by_slot_and_card (AisleriotBoard *board,
                           Slot *slot,
                           int card_id,
                           int num_cards,
                           GdkRectangle *rect)
{
  AisleriotBoardPrivate *priv = board->priv;
  guint delta;
  int first_card_id, num;

  g_return_if_fail (slot != NULL && card_id >= -1);

  first_card_id = ((int) slot->cards->len) - ((int) slot->exposed);

  if (card_id >= first_card_id) {
    delta = card_id - first_card_id;
    num = num_cards - 1;

    rect->x = slot->rect.x + delta * slot->pixeldx;
    rect->y = slot->rect.y + delta * slot->pixeldy;
    rect->width = priv->card_size.width + num * slot->pixeldx;
    rect->height = priv->card_size.height + num * slot->pixeldy;
        
    if (priv->is_rtl &&
        slot->expanded_right) {
      rect->x += slot->rect.width - priv->card_size.width;
    }

  } else {
    /* card_id == -1 or no card available, return the slot rect.
     * Its size should be card_size.
     */
    *rect = slot->rect;
  }
}

/* Focus handling */

static void
widen_rect (GdkRectangle *rect,
            int delta)
{
  int x, y;

  x = rect->x - delta;
  y = rect->y - delta;

  rect->x = MAX (x, 0);
  rect->y = MAX (y, 0);
  rect->width = rect->width + 2 * delta;
  rect->height = rect->height + 2 * delta;
}

static void
get_focus_rect (AisleriotBoard *board,
                GdkRectangle *rect)
{
  AisleriotBoardPrivate *priv = board->priv;

  if (!priv->focus_slot)
    return;

  get_rect_by_slot_and_card (board,
                             priv->focus_slot,
                             priv->focus_card_id,
                             1, rect);
  widen_rect (rect, priv->focus_line_width + priv->focus_padding);
}

static void
set_focus (AisleriotBoard *board,
           Slot *slot,
           int card_id,
           gboolean show_focus)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);
  int top_card_id;

  /* Sanitise */
  top_card_id = slot ? ((int) slot->cards->len) - 1 : -1;
  card_id = MIN (card_id, top_card_id);

  if (priv->focus_slot == slot &&
      priv->focus_card_id == card_id &&
      priv->show_focus == show_focus)
    return;

  if (priv->focus_slot != NULL) {
    if (priv->show_focus &&
        GTK_WIDGET_HAS_FOCUS (widget)) {
      gdk_window_invalidate_rect (widget->window, &priv->focus_rect, FALSE);
    
      priv->show_focus = FALSE;
    }

    priv->focus_slot = NULL;
    priv->focus_card_id = -1;
  }

  priv->show_focus = show_focus;

  if (!slot)
    return;

  priv->focus_slot = slot;
  priv->focus_card_id = card_id;

  if (show_focus &&
      GTK_WIDGET_HAS_FOCUS (widget)) {
    get_focus_rect (board, &priv->focus_rect);
    gdk_window_invalidate_rect (widget->window, &priv->focus_rect, FALSE);
  }
}

/* Selection handling */

static void
get_selection_rect (AisleriotBoard *board,
                    GdkRectangle *rect)
{
  AisleriotBoardPrivate *priv = board->priv;
  int n_cards;

  if (!priv->selection_slot)
    return;

  n_cards = priv->selection_slot->cards->len - priv->selection_start_card_id;

  get_rect_by_slot_and_card (board,
                             priv->selection_slot,
                             priv->selection_start_card_id,
                             n_cards, rect);
}

static void
set_selection (AisleriotBoard *board,
               Slot *slot,
               int card_id,
               gboolean show_selection)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);

  if (priv->selection_slot == slot &&
      priv->selection_start_card_id == card_id &&
      priv->show_selection == show_selection)
    return;

  if (priv->selection_slot != NULL) {
    if (priv->show_selection) {
      gdk_window_invalidate_rect (widget->window, &priv->selection_rect, FALSE);

      /* Clear selection card images */
      slot_update_card_images_full (board, priv->selection_slot, G_MAXINT);
    }

    priv->selection_slot = NULL;
    priv->selection_start_card_id = -1;
  }

  priv->show_selection = show_selection;
  priv->selection_slot = slot;
  priv->selection_start_card_id = card_id;
  g_assert (slot != NULL || card_id == -1);

  if (!slot)
    return;

  g_assert (card_id < 0 || card_id < slot->cards->len);

  if (priv->show_selection) {
    get_selection_rect (board, &priv->selection_rect);
    gdk_window_invalidate_rect (widget->window, &priv->selection_rect, FALSE);
  
    slot_update_card_images_full (board, slot, card_id);
  }
}

/* Slot functions */

static void
slot_update_geometry (AisleriotBoard *board,
                      Slot *slot)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);
  GdkRectangle old_rect;
  GByteArray *cards;
  int delta, xofs, yofs, pixeldx;

  if (!priv->geometry_set)
    return;

  cards = slot->cards;
  old_rect = slot->rect;

  xofs = priv->xoffset;
  yofs = priv->yoffset;

  /* FIXMEchpe: what exactly is the purpose of the following lines? */
  if (slot->expanded_right)
    xofs = yofs;
  if (slot->expanded_down)
    yofs = xofs;

  if (priv->is_rtl) {
    slot->rect.x = priv->xslotstep * (priv->width - slot->x) - priv->card_size.width - xofs + priv->xbaseoffset;
  } else {
    slot->rect.x = priv->xslotstep * slot->x + xofs + priv->xbaseoffset;
  }

  slot->rect.y = priv->yslotstep * slot->y + yofs; /* FIXMEchpe + priv->ybaseoffset; */

  /* We need to make sure the cards fit within the board, even
   * when there are many of them. See bug #171417.
   */
  /* FIXMEchpe: check |slot->exposed| instead of cards->len? */
  pixeldx = 0;
  if (cards->len > 1) {
    double dx = 0, dy = 0;
    double n_cards = cards->len - 1; /* FIXMEchpe: slot->exposed - 1 ? */

    if (slot->expanded_down) {
      double y_from_bottom, max_dy = MAX_DELTA;

      if (slot->dy_set)
        max_dy = slot->expansion.dy;

      /* Calculate the compressed_dy that will let us fit within the board */
      y_from_bottom = ((double) (widget->allocation.height - slot->rect.y)) / ((double) priv->card_size.height);
      dy = (y_from_bottom - MAX_OVERHANG) / n_cards;
      dy = CLAMP (dy, MIN_DELTA, max_dy);
    } else if (slot->expanded_right) {
      if (priv->is_rtl) {
        double x_from_left, max_dx = MAX_DELTA;

        if (slot->dx_set)
          max_dx = slot->expansion.dx;

        x_from_left = ((double) slot->rect.x) / ((double) priv->card_size.width) + 1.0;
        dx = (x_from_left - MAX_OVERHANG) / n_cards;
        dx = CLAMP (dx, MIN_DELTA, max_dx);

        slot->pixeldx = DOUBLE_TO_INT_CEIL (- dx * priv->card_size.width);
        pixeldx = -slot->pixeldx;
      } else {
        double x_from_right, max_dx = MAX_DELTA;

        if (slot->dx_set)
          max_dx = slot->expansion.dx;

        x_from_right = ((double) (widget->allocation.width - slot->rect.x)) / ((double) priv->card_size.width);
        dx = (x_from_right - MAX_OVERHANG) / n_cards;
        dx = CLAMP (dx, MIN_DELTA, max_dx);

        pixeldx = slot->pixeldx = DOUBLE_TO_INT_CEIL (dx * priv->card_size.width);        
      }
    }

    slot->pixeldy = DOUBLE_TO_INT_CEIL (dy * priv->card_size.height);
  } else {
    slot->pixeldx = slot->pixeldy = 0;
  }

  slot->exposed = cards->len;
  if (0 < slot->expansion_depth &&
      slot->expansion_depth < slot->exposed) {
    slot->exposed = slot->expansion_depth;
  }

  if ((slot->pixeldx == 0 &&
       slot->pixeldy == 0 &&
       slot->exposed > 1) ||
      (cards->len > 0 &&
       slot->exposed < 1)) {
    slot->exposed = 1;
  }

  delta = slot->exposed > 0 ? slot->exposed - 1 : 0;

  slot->rect.width = priv->card_size.width + delta * pixeldx;
  slot->rect.height = priv->card_size.height + delta * slot->pixeldy;

  if (priv->is_rtl) {
    slot->rect.x -= slot->rect.width - priv->card_size.width;
  }

  if (GTK_WIDGET_REALIZED (widget)) {
    GdkRectangle damage = slot->rect;

    if (old_rect.width > 0 && old_rect.height > 0) {
      gdk_rectangle_union (&damage, &old_rect, &damage);
    }

    gdk_window_invalidate_rect (widget->window, &damage, FALSE);
  }

  slot->needs_update = FALSE;
}

static void
slot_update_card_images_full (AisleriotBoard *board,
                              Slot *slot,
                              int highlight_start_card_id)
{
  AisleriotBoardPrivate *priv = board->priv;
  GamesCardImages *images = priv->images;
  GPtrArray *card_images;
  guint n_cards, first_exposed_card_id, i;
  guint8 *cards;

  card_images = slot->card_images;
  g_ptr_array_set_size (card_images, 0);

  if (!priv->geometry_set)
    return;

  cards = slot->cards->data;
  n_cards = slot->cards->len;

  g_assert (n_cards >= slot->exposed);
  first_exposed_card_id = n_cards - slot->exposed;

  /* No need to get invisible cards from cache, which will just
   * slow us down!
   */
  for (i = 0; i < first_exposed_card_id; ++i) {
    g_ptr_array_add (card_images, NULL);
  }

  if (PIXBUF_DRAWING_LIKELIHOOD (priv->use_pixbuf_drawing)) {
    for (i = first_exposed_card_id; i < n_cards; ++i) {
      Card card = CARD (cards[i]);
      gboolean is_highlighted;

      is_highlighted = (i >= highlight_start_card_id);

      g_ptr_array_add (card_images,
                       games_card_images_get_card_pixbuf (images, card, is_highlighted));
    }
  } else {
    for (i = first_exposed_card_id; i < n_cards; ++i) {
      Card card = CARD (cards[i]);
      gboolean is_highlighted;

      is_highlighted = (i >= highlight_start_card_id);

      g_ptr_array_add (card_images,
                       games_card_images_get_card_pixmap (images, card, is_highlighted));
    }
  }
}

static void
slot_update_card_images (AisleriotBoard *board,
                         Slot *slot)
{
  AisleriotBoardPrivate *priv = board->priv;
  int highlight_start_card_id = G_MAXINT;

  if (G_UNLIKELY (slot == priv->highlight_slot &&
                  priv->show_highlight)) {
    highlight_start_card_id = slot->cards->len - 1;
  } else if (G_UNLIKELY (slot == priv->selection_slot &&
                         priv->selection_start_card_id >= 0 &&
                         priv->show_selection)) {
    highlight_start_card_id = priv->selection_start_card_id;
  }

  slot_update_card_images_full (board, slot, highlight_start_card_id);
}

/* helper functions */

static void
aisleriot_board_error_bell (AisleriotBoard *board)
{
#if GTK_CHECK_VERSION (2, 12, 0) || (defined (HAVE_HILDON) && !defined(HAVE_MAEMO_3))
  gtk_widget_error_bell (GTK_WIDGET (board));
#endif
}

/* Work out new sizes and spacings for the cards. */
static void
aisleriot_board_setup_geometry (AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);
  GPtrArray *slots;
  guint i, n_slots;
  CardSize card_size;
  gboolean size_changed;

  /* Nothing to do yet */
  if (aisleriot_game_get_state (priv->game) <= GAME_LOADED)
    return;

  g_return_if_fail (GTK_WIDGET_REALIZED (widget));
  g_return_if_fail (priv->width > 0 && priv->height > 0);

  priv->xslotstep = ((double) widget->allocation.width) / priv->width;
  priv->yslotstep = ((double) widget->allocation.height) / priv->height;

  size_changed = games_card_images_set_size (priv->images,
                                             priv->xslotstep,
                                             priv->yslotstep,
                                             priv->card_slot_ratio);

  games_card_images_get_size (priv->images, &card_size);
  priv->card_size = card_size;

  /* If the cards are too far apart, bunch them in the middle. */
  priv->xbaseoffset = 0;
  if (priv->xslotstep > (card_size.width * 3) / 2) {
    priv->xslotstep = (card_size.width * 3) / 2;
    /* FIXMEchpe: if there are expand-right slots, reserve the space for them instead? */
    priv->xbaseoffset = (widget->allocation.width - priv->xslotstep * priv->width) / 2;
  }
  if (priv->yslotstep > (card_size.height * 3) / 2) {
    priv->yslotstep = (card_size.height * 3) / 2;
    /* FIXMEchpe: if there are expand-down slots, reserve the space for them instead?
       priv->ybaseoffset = (widget->allocation.height - priv->yslotstep * priv->height) / 2;
    */
  }

  priv->xoffset = (priv->xslotstep - card_size.width) / 2;
  priv->yoffset = (priv->yslotstep - card_size.height) / 2;

  if (PIXBUF_DRAWING_LIKELIHOOD (priv->use_pixbuf_drawing)) {
    priv->slot_image = games_card_images_get_slot_pixbuf (priv->images, FALSE);
  } else {
    priv->slot_image = games_card_images_get_slot_pixmap (priv->images, FALSE);
  }

  gdk_gc_set_clip_mask (priv->slot_gc, games_card_images_get_slot_mask (priv->images));
  gdk_gc_set_clip_mask (priv->draw_gc, games_card_images_get_card_mask (priv->images));

  /* NOTE! Updating the slots checks that geometry is set, so
   * we set it to TRUE already.
   */
  priv->geometry_set = TRUE;

  /* Now recalculate the slot locations. */
  slots = aisleriot_game_get_slots (priv->game);

  n_slots = slots->len;
  for (i = 0; i < n_slots; ++i) {
    Slot *slot = slots->pdata[i];

    slot_update_geometry (board, slot);
    slot_update_card_images (board, slot);
  }

  /* Update the focus and selection rects */
  get_focus_rect (board, &priv->focus_rect);
  get_selection_rect (board, &priv->selection_rect);
}

static void
drag_begin (AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);
  Slot *hslot;
  int delta, height, width;
  int x, y;
  GdkPixmap *moving_pixmap;
  GdkBitmap *card_mask;
  GdkBitmap *moving_mask;
  GdkGC *gc1, *gc2;
  const GdkColor masked = { 0, 0, 0, 0 };
  const GdkColor unmasked = { 1, 0xffff, 0xffff, 0xffff };
  int num_moving_cards;
  gboolean use_pixbuf_drawing = priv->use_pixbuf_drawing;
  guint i;
  GByteArray *cards;
  GdkDrawable *drawable;
  GdkWindowAttr attributes;

  if (!priv->selection_slot ||
      priv->selection_start_card_id < 0) {
    priv->click_status = STATUS_NONE;
    return;
  }

  priv->click_status = STATUS_IS_DRAG;

  hslot = priv->moving_cards_origin_slot = priv->selection_slot;
  priv->moving_cards_origin_card_id = priv->selection_start_card_id;

  num_moving_cards = hslot->cards->len - priv->moving_cards_origin_card_id;

  cards = hslot->cards;

  /* Save game state */
  aisleriot_game_record_move (priv->game, hslot->id,
                              cards->data, cards->len);

  /* Unset the selection and focus. It'll be re-set if the drag is aborted */
  set_selection (board, NULL, -1, FALSE);
  set_focus (board, NULL, -1, FALSE);

  delta = hslot->exposed - num_moving_cards;

  /* (x,y) is the upper left edge of the topmost dragged card */
  x = hslot->rect.x + delta * hslot->pixeldx;
  if (priv->is_rtl &&
      hslot->expanded_right) {
    x += hslot->rect.width - priv->card_size.width;
  }

  priv->last_click_x -= x;
  priv->last_click_y -= y = hslot->rect.y + delta * hslot->pixeldy;;

  g_byte_array_set_size (priv->moving_cards, 0);
  g_byte_array_append (priv->moving_cards,
                       cards->data + priv->moving_cards_origin_card_id,
                       cards->len - priv->moving_cards_origin_card_id);

  /* Take the cards off of the stack */
  g_byte_array_set_size (cards, priv->moving_cards_origin_card_id);
  g_ptr_array_set_size (hslot->card_images, priv->moving_cards_origin_card_id);

  width = priv->card_size.width + (num_moving_cards - 1) * hslot->pixeldx;
  height = priv->card_size.height + (num_moving_cards - 1) * hslot->pixeldy;

  drawable = GDK_DRAWABLE (widget->window);

  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = 0;
  attributes.x = x;
  attributes.y = y;
  attributes.width = width;
  attributes.height = height;
  attributes.colormap = gdk_drawable_get_colormap (drawable);
  attributes.visual = gdk_drawable_get_visual (drawable);

  priv->moving_cards_window = gdk_window_new (widget->window, &attributes,
                                              GDK_WA_VISUAL | GDK_WA_COLORMAP | GDK_WA_X | GDK_WA_Y);

  moving_pixmap = gdk_pixmap_new (priv->moving_cards_window,
                                  width, height,
                                  gdk_drawable_get_visual (priv->moving_cards_window)->depth);
  moving_mask = gdk_pixmap_new (priv->moving_cards_window,
                                width, height,
                                1);

  gc1 = gdk_gc_new (moving_pixmap);
  gc2 = gdk_gc_new (moving_mask);

  gdk_draw_rectangle (moving_pixmap, gc1, TRUE,
                      0, 0, width, height);

  gdk_gc_set_foreground (gc2, &masked);
  gdk_draw_rectangle (moving_mask, gc2, TRUE,
                      0, 0, width, height);
  gdk_gc_set_foreground (gc2, &unmasked);

  card_mask = games_card_images_get_card_mask (priv->images);
  gdk_gc_set_clip_mask (gc1, card_mask);
  gdk_gc_set_clip_mask (gc2, card_mask);

  /* FIXMEchpe: RTL issue: this doesn't work right when we allow dragging of
   * more than one card from a expand-right slot. (But right now no game .scm
   * does allow that.)
   */
  x = y = 0;
  width = priv->card_size.width;
  height = priv->card_size.height;

  for (i = 0; i < priv->moving_cards->len; ++i) {
    Card hcard = CARD (priv->moving_cards->data[i]);

    gdk_gc_set_clip_origin (gc1, x, y);
    gdk_gc_set_clip_origin (gc2, x, y);

    if (PIXBUF_DRAWING_LIKELIHOOD (use_pixbuf_drawing)) {
      GdkPixbuf *pixbuf;

      pixbuf = games_card_images_get_card_pixbuf (priv->images, hcard, FALSE);
      if (!pixbuf)
        goto next;

      gdk_draw_pixbuf (moving_pixmap, gc1,
                       pixbuf,
                       0, 0, x, y, width, height,
                       GDK_RGB_DITHER_NONE, 0, 0);
      gdk_draw_rectangle (moving_mask, gc2, TRUE,
                          x, y, width, height);
    } else {
      GdkPixmap *pixmap;

      pixmap = games_card_images_get_card_pixmap (priv->images, hcard, FALSE);
      if (!pixmap)
        goto next;

      gdk_draw_drawable (moving_pixmap, gc1, pixmap,
                          0, 0, x, y, width, height);
      gdk_draw_rectangle (moving_mask, gc2, TRUE,
                          x, y, width, height);
    }

  next:

    x += hslot->pixeldx;
    y += hslot->pixeldy;
  }

  g_object_unref (gc1);
  g_object_unref (gc2);

  gdk_window_set_back_pixmap (priv->moving_cards_window,
			      moving_pixmap, 0);
  gdk_window_shape_combine_mask (priv->moving_cards_window,
				 moving_mask, 0, 0);

  g_object_unref (moving_pixmap);
  g_object_unref (moving_mask);

  gdk_window_show (priv->moving_cards_window);

  slot_update_geometry (board, hslot);
  slot_update_card_images (board, hslot);

  set_cursor (board, CURSOR_CLOSED);
}

static void
drag_end (AisleriotBoard *board,
          gboolean moved)
{
  AisleriotBoardPrivate *priv = board->priv;

  if (priv->moving_cards_window != NULL) {
    gdk_window_hide (priv->moving_cards_window);
    gdk_window_destroy (priv->moving_cards_window);
    priv->moving_cards_window = NULL;
  }

  /* FIXMEchpe: check that slot->cards->len == moving_cards_origin_card_id !!! FIXMEchpe what to do if not, abort the game? */
  /* Add the origin cards back to the origin slot */
  if (!moved &&
      priv->moving_cards_origin_slot != NULL &&
      priv->moving_cards->len > 0) {
    aisleriot_game_slot_add_cards (priv->game,
                                   priv->moving_cards_origin_slot,
                                   priv->moving_cards->data,
                                   priv->moving_cards->len);
  }

  priv->click_status = STATUS_NONE;
  priv->moving_cards_origin_slot = NULL;
  priv->moving_cards_origin_card_id = -1;
}

static gboolean
cards_are_droppable (AisleriotBoard *board,
                     Slot *slot)
{
  AisleriotBoardPrivate *priv = board->priv;

  return slot != NULL &&
         priv->moving_cards_origin_slot &&
         aisleriot_game_drop_valid (priv->game,
                                    priv->moving_cards_origin_slot->id,
                                    slot->id,
                                    priv->moving_cards->data,
                                    priv->moving_cards->len);
}

static Slot *
find_drop_target (AisleriotBoard *board,
                  gint x,
                  gint y)
{
  AisleriotBoardPrivate *priv = board->priv;
  Slot *new_hslot;
  Slot *retval = NULL;
  gint i, new_cardid;
  gint min_distance = G_MAXINT;

  /* Find a target directly under the center of the card. */
  get_slot_and_card_from_point (board,
                                x + priv->card_size.width / 2,
                                y + priv->card_size.height / 2,
                                &new_hslot, &new_cardid);

  if (cards_are_droppable (board, new_hslot))
    return new_hslot;

  /* If that didn't work, look for a target at all 4 corners of the card. */
  for (i = 0; i < 4; i++) {
    get_slot_and_card_from_point (board,
                                  x + priv->card_size.width * (i / 2),
                                  y + priv->card_size.height * (i % 2),
                                  &new_hslot, &new_cardid);

    if (!new_hslot)
      continue;

    /* This skips corners we know are not droppable. */
    if (!priv->droppable_supported || cards_are_droppable (board, new_hslot)) {
      gint dx, dy, distance_squared;

      dx = new_hslot->rect.x + (new_cardid - 1) * new_hslot->pixeldx - x;
      dy = new_hslot->rect.y + (new_cardid - 1) * new_hslot->pixeldy - y;

      distance_squared = dx * dx + dy * dy;

      if (distance_squared <= min_distance) {
	retval = new_hslot;
	min_distance = distance_squared;
      }
    }
  }

  return retval;
}

static void
drop_moving_cards (AisleriotBoard *board,
                   gint x,
                   gint y)
{
  AisleriotBoardPrivate *priv = board->priv;
  Slot *hslot;
  gboolean moved = FALSE;

  hslot = find_drop_target (board,
                            x - priv->last_click_x,
                            y - priv->last_click_y);

  if (hslot) {
    moved = aisleriot_game_drop_cards (priv->game,
                                       priv->moving_cards_origin_slot->id,
                                       hslot->id,
                                       priv->moving_cards->data,
                                       priv->moving_cards->len);
  }

  if (moved) {
    aisleriot_game_end_move (priv->game);
    games_sound_play ("click");
  } else {
    aisleriot_game_discard_move (priv->game);
    games_sound_play ("slide");
  }

  drag_end (board, moved);

  if (moved)
    aisleriot_game_test_end_of_game (priv->game);
}

static void
highlight_drop_target (AisleriotBoard *board,
                       Slot *slot)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);
  GdkRectangle rect;

  if (slot == priv->highlight_slot)
    return;

  /* Invalidate the old highlight rect */
  if (priv->highlight_slot != NULL &&
      priv->show_highlight) {
    get_rect_by_slot_and_card (board,
                               priv->highlight_slot,
                               priv->highlight_slot->cards->len - 1 /* it's ok if this is == -1 */,
                               1, &rect);
    gdk_window_invalidate_rect (widget->window, &rect, FALSE);

    /* FIXMEchpe only update the topmost card? */
    /* It's ok to call this directly here, since the old highlight_slot cannot
     * have been the same as the current selection_slot!
     */
    slot_update_card_images_full (board, priv->highlight_slot, G_MAXINT);
  }

  /* Need to set the highlight slot even when we the cards aren't droppable
   * since that can happen when the game doesn't support FEATURE_DROPPABLE.
   */
  priv->highlight_slot = slot;
  
  if (!cards_are_droppable (board, slot))
    return;

  if (!priv->show_highlight)
    return;

  /* Prepare the highlight pixbuf/pixmaps and invalidate the new highlight rect */
  get_rect_by_slot_and_card (board,
                             slot,
                             slot->cards->len - 1 /* it's ok if this is == -1 */,
                             1, &rect);
  gdk_window_invalidate_rect (widget->window, &rect, FALSE);

  /* FIXMEchpe only update the topmost card? */
  /* It's ok to call this directly, since the highlight slot is always
   * different from the selection slot!
   */
  slot_update_card_images_full (board, slot, ((int) slot->cards->len) - 1);
}

static void
reveal_card (AisleriotBoard *board,
             Slot *slot,
             int cardid)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);
  Card card;
  GdkRectangle rect;

  if (priv->show_card_slot == slot)
    return;

  if (priv->show_card_slot != NULL) {
    get_rect_by_slot_and_card (board,
                               priv->show_card_slot,
                               priv->show_card_id,
                               1, &rect);
    gdk_window_invalidate_rect (widget->window, &rect, FALSE);
    priv->show_card_slot = NULL;
    priv->show_card_id = -1;
    priv->click_status = STATUS_NONE;
  }

  if (!slot || cardid < 0 || cardid >= slot->cards->len - 1)
    return;

  card = CARD (slot->cards->data[cardid]);
  if (CARD_GET_FACE_DOWN (card))
    return;

  priv->show_card_slot = slot;
  priv->show_card_id = cardid;
  priv->click_status = STATUS_SHOW;

  get_rect_by_slot_and_card (board,
                            priv->show_card_slot,
                            priv->show_card_id,
                            1, &rect);
  gdk_window_invalidate_rect (widget->window, &rect, FALSE);
}

static void
clear_state (AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;

  highlight_drop_target (board, NULL);
  drag_end (board, FALSE /* FIXMEchpe ? */);

  reveal_card (board, NULL, -1);

  priv->click_status = STATUS_NONE;
  priv->last_clicked_slot = NULL;
  priv->last_clicked_card_id = -1;
}

static void
aisleriot_board_settings_update (GtkSettings *settings,
                                 GParamSpec *pspec,
                                 AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;
#if GTK_CHECK_VERSION (2, 10, 0)
  gboolean touchscreen_mode;
#endif /* GTK >= 2.10.0 */

   /* Set up the double-click detection. */
  g_object_get (settings,
                "gtk-double-click-time", &priv->double_click_time,
#if GTK_CHECK_VERSION (2, 10, 0)
                "gtk-touchscreen-mode", &touchscreen_mode,
#endif /* GTK >= 2.10.0 */
                NULL);

#if GTK_CHECK_VERSION (2, 10, 0)
  priv->touchscreen_mode = touchscreen_mode != FALSE;
#endif /* GTK >= 2.10.0 */
}

/* Note: this unsets the selection! hslot may be equal to priv->selection_slot. */
static gboolean
aisleriot_board_move_selected_cards_to_slot (AisleriotBoard *board,
                                             Slot *hslot)
{
  AisleriotBoardPrivate *priv = board->priv;
  Slot *selection_slot = priv->selection_slot;
  int selection_start_card_id = priv->selection_start_card_id;
  gboolean moved;
  guint8 *cards;
  guint n_cards;

  if (!selection_slot ||
      priv->selection_start_card_id < 0)
    return FALSE;

  /* NOTE: We cannot use aisleriot_game_drop_valid here since the
   * game may not support the "droppable" feature.
   */

  set_selection (board, NULL, -1, FALSE);

  priv->click_status = STATUS_NONE;

  aisleriot_game_record_move (priv->game,
                              selection_slot->id,
                              selection_slot->cards->data,
                              selection_slot->cards->len);

  /* Store the cards, since the move could alter slot->cards! */
  g_assert (selection_slot->cards->len >= selection_start_card_id);
  n_cards = selection_slot->cards->len - selection_start_card_id;

  cards = g_alloca (n_cards);
  memcpy (cards,
          selection_slot->cards->data + selection_start_card_id,
          n_cards);

  /* Now take the cards off of the origin slot. We'll update the slot geometry later */
  g_byte_array_set_size (selection_slot->cards, selection_start_card_id);
  selection_slot->needs_update = TRUE;

  moved = aisleriot_game_drop_cards (priv->game,
                                      selection_slot->id,
                                      hslot->id,
                                      cards,
                                      n_cards);
  if (moved) {
    aisleriot_game_end_move (priv->game);

    games_sound_play ("click");

    if (selection_slot->needs_update)
      g_signal_emit_by_name (priv->game, "slot-changed", selection_slot); /* FIXMEchpe! */

    aisleriot_game_test_end_of_game (priv->game);
  } else {
    /* Not moved; discard the move add the cards back to the origin slot */
    aisleriot_game_discard_move (priv->game);
    aisleriot_game_slot_add_cards (priv->game, selection_slot, cards, n_cards);
  }

  return moved;
}

/* Keynav */

#ifdef ENABLE_KEYNAV

/** Keynav specification:
 *
 * Focus state consists of the slot that has the focus, and the card
 * in that slot of the card that has the focus; if the slot has no cards
 * the focus is on the empty slot itself.
 *
 * Without modifiers, for focus movement:
 * Left (Right): For right-extended slots, moves the focus to the card
 *   under (over) the currently focused card on the same slot.
 *   If the focused card is already the bottommost (topmost)
 *   card of the slot, or the slot is not right-extended, moves the focus
 *   to the topmost (bottommost) card on the slot left (right) to the currently
 *   focused slot (wraps around from first to last slot and vv.)
 * Up (Down): For down-extended slots, moves the focus to the card
 *   under (over) the currently focused card on the same slot.
 *   If the focused card is already the bottommost (topmost) card
 *   of the slot, or the slot is not down-extended, moves the focus to
 *   the topmost (bottommost) card of the slot over (under) the currently
 *   focused slot (wraps around vertically)
 * Home (End): For down- or right-extended slots, moves the focus to the
 *   start (end) of the stack of face-up cards on the currently focused slot.
 *   If the focus is already on that card, moves the focus over a stack of
 *   face-down cards into the next stack of face-up cards (e.g. Athena), or
 *   the bottommost (topmost) card on the slot if there are no such cards.
 * PageUp, PageDown: Acts like <control>Up, <control>Down
 * Space: selects the cards from the currently focused card to the topmost
 *   card of the slot, if this is allowed by the game. If the focused card is
 *   the first selected card already, unsets the selection.
 * Return: Performs the button press action on the focused card.
 *   If no action was performed by that, moves the selected card(s) to the top
 *   of the focused slot, if this is allowed by the game.
 *
 * With <control>, for focus movement:
 * Left (Right): Moves the focus to the bottommost (topmost) card on the
 *   slot; if that card is already focused, moves the focus to the
 *   topmost/bottommost card on the slot left/right to the currently
 *   focused slot (wraps around from first to last slot and vv.)
 * Up (Down): Moves the focus to the topmost (bottommost) card of the slot
 *   over (under) the currently focused slot (wraps around vertically)
 * Home (End): moves the focus to the bottommost (topmost) card on the
 *   first (last) slot
 * Return: Performs the double-click action on the focused card
 *
 * With <shift>: extends the selection; focus movement itself occurs
 *   like for the same key without modifiers.
 * Left (Right): for right-extended slots, extends (shrinks) the selection
 *   by one card, if this is allowed by the game
 * Up (Down): for down-extended slots, extends (shrinks) the selection
 *   by one card, if this is allowed by the game
 * Home: extends the selection maximally in the focused slot, as allowed
 *   by the game
 * End: shrinks the selection into nonexistence
 * 
 * With <control><shift>: extends selection like with <shift> alone,
 *   and moves focus like with <control> alone
 *
 * Other keyboard shortcuts:
 * <control>A: extends the selection maximally in the focused slot, as allowed
 *   by the game
 * <shift><control>A: unsets the selection
 *
 * Notes:
 * If no slot is currently focused:
 * Right, Up, Down, PgUp, PgDown, Home: moves the focus to the bottommost card on the first
 *   slot
 * Left, End: moves the focus to the topmost card on the last slot
 */

static void
aisleriot_board_add_move_binding (GtkBindingSet  *binding_set,
                                  guint           keyval,
                                  guint           modmask,
                                  GtkMovementStep step,
                                  gint            count)
{
  gtk_binding_entry_add_signal (binding_set, keyval,
                                modmask,
                                "move-cursor", 2,
                                G_TYPE_ENUM, step,
                                G_TYPE_INT, count);

  if (modmask & GDK_CONTROL_MASK)
   return;

  gtk_binding_entry_add_signal (binding_set, keyval,
                                modmask | GDK_CONTROL_MASK,
                                "move-cursor", 2,
                                G_TYPE_ENUM, step,
                                G_TYPE_INT, count);

}

static void
aisleriot_board_add_move_and_select_binding (GtkBindingSet  *binding_set,
                                             guint           keyval,
                                             guint           modmask,
                                             GtkMovementStep step,
                                             gint            count)
{
  aisleriot_board_add_move_binding (binding_set, keyval, modmask, step, count);
  aisleriot_board_add_move_binding (binding_set, keyval, modmask | GDK_SHIFT_MASK, step, count);
}

static void
aisleriot_board_add_activate_binding (GtkBindingSet  *binding_set,
                                      guint           keyval,
                                      guint           modmask)
{
  gtk_binding_entry_add_signal (binding_set, keyval, modmask,
                                "activate", 0);

  if (modmask & GDK_CONTROL_MASK)
    return;

  gtk_binding_entry_add_signal (binding_set, keyval, modmask | GDK_CONTROL_MASK,
                                "activate", 0);
}

static gboolean
aisleriot_board_move_cursor_in_slot (AisleriotBoard *board,
                                     int count)
{
  AisleriotBoardPrivate *priv = board->priv;
  Slot *focus_slot;
  int new_focus_card_id, first_card_id;

  focus_slot = priv->focus_slot;
  first_card_id = ((int) focus_slot->cards->len) - ((int) focus_slot->exposed);
  new_focus_card_id = priv->focus_card_id + count;
  if (new_focus_card_id < first_card_id || new_focus_card_id >= (int) focus_slot->cards->len)
    return FALSE;

  set_focus (board, focus_slot, new_focus_card_id, TRUE);
  return TRUE;
}

static gboolean
aisleriot_board_move_cursor_start_end_in_slot (AisleriotBoard *board,
                                               int count)
{
  AisleriotBoardPrivate *priv = board->priv;
  Slot *focus_slot = priv->focus_slot;
  int first_card_id, top_card_id, new_focus_card_id;
  guint8 *cards;

  if (focus_slot->cards->len == 0)
    return FALSE;

  g_assert (priv->focus_card_id >= 0);

  /* Moves the cursor to the first/last card above/below a face-down
   * card, or the start/end of the slot if there are no face-down cards
   * between the currently focused card and the slot start/end.
   * (Jumping over face-down cards and landing on a non-face-down card
   * happens e.g. in Athena.)
   */
  cards = focus_slot->cards->data;
  top_card_id = ((int) focus_slot->cards->len) - 1;
  first_card_id = ((int) focus_slot->cards->len) - ((int) focus_slot->exposed);
  new_focus_card_id = priv->focus_card_id;

  /* Set new_focus_card_id to the index of the last face-down card
    * in the run of face-down cards.
    */
  do {
    new_focus_card_id += count;
  } while (new_focus_card_id >= first_card_id &&
           new_focus_card_id <= top_card_id &&
           CARD_GET_FACE_DOWN (((Card) cards[new_focus_card_id])));

  /* We went one too far */
  new_focus_card_id -= count;

  /* Now get to the start/end of the run of face-up cards */
  do {
    new_focus_card_id += count;
  } while (new_focus_card_id >= first_card_id &&
           new_focus_card_id <= top_card_id &&
           !CARD_GET_FACE_DOWN (((Card) cards[new_focus_card_id])));

  if (new_focus_card_id < first_card_id ||
      new_focus_card_id > top_card_id ||
      CARD_GET_FACE_DOWN (((Card) cards[new_focus_card_id]))) {
    /* We went one too far */
    new_focus_card_id -= count;
  }

  new_focus_card_id = CLAMP (new_focus_card_id, first_card_id, top_card_id);
  set_focus (board, focus_slot, new_focus_card_id, TRUE);

  return TRUE;
}

static gboolean
aisleriot_board_extend_selection_in_slot (AisleriotBoard *board,
                                          int count)
{
  AisleriotBoardPrivate *priv = board->priv;
  Slot *focus_slot, *selection_slot;
  int new_selection_start_card_id, first_card_id;

  focus_slot = priv->focus_slot;
  selection_slot = priv->selection_slot;
  first_card_id = ((int) focus_slot->cards->len) - ((int) focus_slot->exposed);

  if (selection_slot == focus_slot) {
    new_selection_start_card_id = priv->selection_start_card_id + count;

    /* Can only extend the selection if the focus is adjacent to the selection */
    if (priv->focus_card_id - 1 > new_selection_start_card_id ||
        priv->focus_card_id + 1 < new_selection_start_card_id)
      return FALSE;
  } else {
    /* No selection yet */
    new_selection_start_card_id = ((int) focus_slot->cards->len) + count;

    /* Must have the topmost card focused */
    if (new_selection_start_card_id != priv->focus_card_id)
      return FALSE;
  }

  if (new_selection_start_card_id < first_card_id)
    return FALSE;

  /* If it's the top card, unselect all */
  if (new_selection_start_card_id >= focus_slot->cards->len) {
    set_selection (board, NULL, -1, FALSE);
    return TRUE;
  }
    
  if (!aisleriot_game_drag_valid (priv->game,
                                  focus_slot->id,
                                  focus_slot->cards->data + new_selection_start_card_id,
                                  focus_slot->cards->len - new_selection_start_card_id))
    return FALSE;

  set_selection (board, focus_slot, new_selection_start_card_id, TRUE);

  /* Try to move the cursor too, but don't beep if that fails */
  aisleriot_board_move_cursor_in_slot (board, count);
  return TRUE;
}

static gboolean
aisleriot_board_extend_selection_in_slot_maximal (AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;
  Slot *focus_slot = priv->focus_slot;
  int new_selection_start_card_id, n_selected;

  n_selected = 0;
  new_selection_start_card_id = ((int) focus_slot->cards->len) - 1;
  while (new_selection_start_card_id >= 0) {
    if (!aisleriot_game_drag_valid (priv->game,
                                    focus_slot->id,
                                    focus_slot->cards->data + new_selection_start_card_id,
                                    focus_slot->cards->len - new_selection_start_card_id))
      break;

    ++n_selected;
    --new_selection_start_card_id;
  }

  if (n_selected == 0)
    return FALSE;

  set_selection (board, focus_slot, new_selection_start_card_id + 1, TRUE);
  return TRUE;
}

static gboolean
aisleriot_board_move_cursor_left_right_by_slot (AisleriotBoard *board,
                                                int count,
                                                gboolean wrap)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);
  GPtrArray *slots;
  guint n_slots;
  Slot *focus_slot, *new_focus_slot;
  int focus_slot_index, new_focus_slot_index;
  int new_focus_slot_topmost_card_id, new_focus_card_id;
  gboolean is_rtl;
#if GTK_CHECK_VERSION (2, 12, 0)
  GtkDirectionType direction;
#endif

  slots = aisleriot_game_get_slots (priv->game);
  if (!slots || slots->len == 0)
    return FALSE;

  n_slots = slots->len;

  focus_slot = priv->focus_slot;
  g_assert (focus_slot != NULL);

  focus_slot_index = get_slot_index_from_slot (board, focus_slot);

  /* Move visually */
  is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;
  if (is_rtl) {
    new_focus_slot_index = focus_slot_index - count;
  } else {
    new_focus_slot_index = focus_slot_index + count;
  }

  /* Wrap-around? */
  if (new_focus_slot_index < 0 ||
      new_focus_slot_index >= n_slots) {
    if (!wrap)
      return FALSE;

#if GTK_CHECK_VERSION (2, 12, 0)
    if (count > 0) {
      direction = GTK_DIR_RIGHT;
    } else {
      direction = GTK_DIR_LEFT;
    }

    if (!gtk_widget_keynav_failed (widget, direction)) {
       return gtk_widget_child_focus (gtk_widget_get_toplevel (widget), direction);
    }
#endif /* GTK 2.12. 0 */

    if (new_focus_slot_index < 0) {
      new_focus_slot_index = ((int) n_slots) - 1;
    } else {
      new_focus_slot_index = 0;
    }
  }

  g_assert (new_focus_slot_index >= 0 && new_focus_slot_index < n_slots);

  new_focus_slot = slots->pdata[new_focus_slot_index];
  new_focus_slot_topmost_card_id = ((int) new_focus_slot->cards->len) - 1;

  if (new_focus_slot->expanded_right) {
    if ((is_rtl && count < 0) ||
        (!is_rtl && count > 0)) {
      if (new_focus_slot->cards->len > 0) {
        new_focus_card_id = ((int) new_focus_slot->cards->len) - ((int) new_focus_slot->exposed);
      } else {
        new_focus_card_id = -1;
      }
    } else {
      new_focus_card_id = new_focus_slot_topmost_card_id;
    }
  } else {
    /* Just take the topmost card */
    new_focus_card_id = new_focus_slot_topmost_card_id;
  }

  set_focus (board, new_focus_slot, new_focus_card_id, TRUE);
  return TRUE;
}

static gboolean
aisleriot_board_move_cursor_up_down_by_slot (AisleriotBoard *board,
                                             int count)
{
  AisleriotBoardPrivate *priv = board->priv;
  GPtrArray *slots;
  guint n_slots;
  Slot *focus_slot, *new_focus_slot;
  int focus_slot_index, new_focus_slot_index;
  int new_focus_slot_topmost_card_id, new_focus_card_id;
  int x_start, x_end;

  slots = aisleriot_game_get_slots (priv->game);
  if (!slots || slots->len == 0)
    return FALSE;

  n_slots = slots->len;

  focus_slot = priv->focus_slot;
  g_assert (focus_slot != NULL);

  x_start = focus_slot->rect.x;
  x_end = x_start + focus_slot->rect.width;

  focus_slot_index = get_slot_index_from_slot (board, focus_slot);

  new_focus_slot_index = focus_slot_index;
  do {
    new_focus_slot_index += count;
  } while (new_focus_slot_index >= 0 &&
           new_focus_slot_index < n_slots &&
           !test_slot_projection_intersects_x (slots->pdata[new_focus_slot_index], x_start, x_end));

  if (new_focus_slot_index < 0 || new_focus_slot_index == n_slots) {
#if GTK_CHECK_VERSION (2, 12, 0)
    GtkWidget *widget = GTK_WIDGET (board);
    GtkDirectionType direction;

    if (count > 0) {
      direction = GTK_DIR_DOWN;
    } else {
      direction = GTK_DIR_UP;
    }

    if (!gtk_widget_keynav_failed (widget, direction)) {
       return gtk_widget_child_focus (gtk_widget_get_toplevel (widget), direction);
    }
#endif /* GTK 2.12. 0 */

    /* Wrap around */
    if (count > 0) {
      new_focus_slot_index = -1;
    } else {
      new_focus_slot_index = n_slots;
    }

    do {
      new_focus_slot_index += count;
    } while (new_focus_slot_index != focus_slot_index &&
             !test_slot_projection_intersects_x (slots->pdata[new_focus_slot_index], x_start, x_end));
  }

  g_assert (new_focus_slot_index >= 0 && new_focus_slot_index < n_slots);

  new_focus_slot = slots->pdata[new_focus_slot_index];
  new_focus_slot_topmost_card_id = ((int) new_focus_slot->cards->len) - 1;

  if (new_focus_slot->expanded_down) {
    if (count > 0) {
      if (new_focus_slot->cards->len > 0) {
        new_focus_card_id = ((int) new_focus_slot->cards->len) - ((int) new_focus_slot->exposed);
      } else {
        new_focus_card_id = -1;
      }
    } else {
      new_focus_card_id = new_focus_slot_topmost_card_id;
    }
  } else {
    /* Just take the topmost card */
    new_focus_card_id = new_focus_slot_topmost_card_id;
  }

  set_focus (board, new_focus_slot, new_focus_card_id, TRUE);
  return TRUE;
}

static gboolean
aisleriot_board_move_cursor_start_end_by_slot (AisleriotBoard *board,
                                               int count)
{
  AisleriotBoardPrivate *priv = board->priv;
  GPtrArray *slots;
  Slot *new_focus_slot;
  int new_focus_card_id;

  slots = aisleriot_game_get_slots (priv->game);
  if (!slots || slots->len == 0)
    return FALSE;

  if (count > 0) {
    new_focus_slot = (Slot *) slots->pdata[slots->len - 1];
    new_focus_card_id = ((int) new_focus_slot->cards->len) - 1;
  } else {
    new_focus_slot = (Slot *) slots->pdata[0];
    if (new_focus_slot->cards->len > 0) {
      new_focus_card_id = ((int) new_focus_slot->cards->len) - ((int) new_focus_slot->exposed);
    } else {
      new_focus_card_id = -1;
    }
  }

  g_assert (new_focus_slot != NULL);
  g_assert (new_focus_card_id >= -1);

  set_focus (board, new_focus_slot, new_focus_card_id, TRUE);
  return TRUE;
}

static gboolean
aisleriot_board_move_cursor_left_right (AisleriotBoard *board,
                                        int count,
                                        gboolean is_control)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);
  gboolean is_rtl;

  is_rtl = (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL);

  /* First try in-slot focus movement */
  if (!is_control &&
      priv->focus_slot->expanded_right &&
      aisleriot_board_move_cursor_in_slot (board, is_rtl ? -count : count))
    return TRUE;

  /* Cannot move in-slot; move focused slot */
  return aisleriot_board_move_cursor_left_right_by_slot (board, count, TRUE);
}

static gboolean
aisleriot_board_move_cursor_up_down (AisleriotBoard *board,
                                     int count,
                                     gboolean is_control)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);
  gboolean is_rtl;

  g_assert (priv->focus_slot != NULL);

  is_rtl = (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL);

  /* First try in-slot focus movement */
  if (!is_control &&
      priv->focus_slot->expanded_down &&
      aisleriot_board_move_cursor_in_slot (board, count))
    return TRUE;

  /* Cannot move in-slot; move focused slot */
  return aisleriot_board_move_cursor_up_down_by_slot (board, count);
}

static gboolean
aisleriot_board_extend_selection_left_right (AisleriotBoard *board,
                                             int count)
{
  AisleriotBoardPrivate *priv = board->priv;

  if (!priv->focus_slot->expanded_right)
    return FALSE;

  return aisleriot_board_extend_selection_in_slot (board, count);
}

static gboolean
aisleriot_board_extend_selection_up_down (AisleriotBoard *board,
                                          int count)
{
  AisleriotBoardPrivate *priv = board->priv;

  if (!priv->focus_slot->expanded_down)
    return FALSE;

  return aisleriot_board_extend_selection_in_slot (board, count);
}

static gboolean
aisleriot_board_extend_selection_start_end (AisleriotBoard *board,
                                            int count)
{
  AisleriotBoardPrivate *priv = board->priv;
  Slot *focus_slot = priv->focus_slot;
  int new_focus_card_id;

  if (count > 0) {
    /* Can only shrink the selection if the focus is on the selected slot,
     * and the focused card is on or below the start of the selection.
     */
    if (priv->selection_slot == focus_slot &&
        priv->selection_start_card_id >= priv->focus_card_id) {
      set_selection (board, NULL, -1, FALSE);
      new_focus_card_id = ((int) focus_slot->cards->len);
    } else {
      aisleriot_board_error_bell (board);
      return FALSE;
    }

  } else {
    if (!aisleriot_board_extend_selection_in_slot_maximal (board)) {
      set_selection (board, NULL, -1, FALSE);
      aisleriot_board_error_bell (board);
      return FALSE;
    }

    new_focus_card_id = priv->selection_start_card_id;
  }

  set_focus (board, focus_slot, new_focus_card_id, TRUE);
  return TRUE;
}

#endif /* ENABLE_KEYNAV */

/* Game state handling */

static void
game_type_changed_cb (AisleriotGame *game,
                      AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;
  guint features;

  features = aisleriot_game_get_features (game);

  priv->droppable_supported = ((features & FEATURE_DROPPABLE) != 0);
  priv->show_highlight = priv->droppable_supported;
}

static void
game_cleared_cb (AisleriotGame *game,
                 AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;

  priv->geometry_set = FALSE;

  /* So we don't re-add the cards to the now-dead slot */
  priv->highlight_slot = NULL;
  priv->last_clicked_slot = NULL;
  priv->moving_cards_origin_slot = NULL;
  priv->selection_slot = NULL;
  priv->show_card_slot = NULL;

  clear_state (board);
}

static void
game_new_cb (AisleriotGame *game,
             AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;

  clear_state (board);

  set_focus (board, NULL, -1, FALSE);
  set_selection (board, NULL, -1, FALSE);

  aisleriot_game_get_geometry (game, &priv->width, &priv->height);

  aisleriot_board_setup_geometry (board);

#if 0
  g_print ("{ %.3f , %.3f /* %s */ },\n",
           priv->width, priv->height,
           aisleriot_game_get_game_file (priv->game));
#endif

  gtk_widget_queue_draw (GTK_WIDGET (board));
}

static void
slot_changed_cb (AisleriotGame *game,
                 Slot *slot,
                 AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;

  slot_update_geometry (board, slot);
  slot_update_card_images (board, slot);

  if (slot == priv->moving_cards_origin_slot) {
    /* PANIC! */
    /* FIXMEchpe */
  }
  if (slot == priv->selection_slot) {
    set_selection (board, NULL, -1, FALSE);

    /* If this slot changes while we're in a click cycle, abort the action.
     * That prevents a problem where the cards that were selected and
     * about to be dragged have vanished because of autoplay; c.f. bug #449767.
     * Note: we don't use clear_state() here since we only want to disable
     * the highlight and revealed card if that particular slot changed, see
     * the code above. And we don't clear last_clicked_slot/card_id either, so
     * a double-click will still work.
     */
    priv->click_status = STATUS_NONE;
  }
  if (slot == priv->focus_slot) {
    /* Try to keep the focus intact. If the focused card isn't there
     * anymore, this will set the focus to the topmost card of there
     * same slot, or the slot itself if there are no cards on it.
     * If the slot was empty but now isn't, we set the focus to the
     * topmost card.
     */
    if (priv->focus_card_id < 0) {
      set_focus (board, slot, ((int) slot->cards->len) - 1, priv->show_focus);
    } else {
      set_focus (board, slot, priv->focus_card_id, priv->show_focus);
    }
  }
  if (slot == priv->highlight_slot) {
    highlight_drop_target (board, NULL);
  }
}

/* Class implementation */

G_DEFINE_TYPE (AisleriotBoard, aisleriot_board, GTK_TYPE_DRAWING_AREA);

/* AisleriotBoardClass methods */

#ifdef ENABLE_KEYNAV

static void
aisleriot_board_activate (AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);
  Slot *focus_slot = priv->focus_slot;
  Slot *selection_slot = priv->selection_slot;
  int selection_start_card_id = priv->selection_start_card_id;
  guint state = 0;

  if (!GTK_WIDGET_HAS_FOCUS (widget))
    return;

  if (!focus_slot) {
    aisleriot_board_error_bell (board);
    return;
  }

  /* Focus not shown? Show it, and do nothing else */
  if (!priv->show_focus) {
    set_focus (board, focus_slot, priv->focus_card_id, TRUE);
    return;
  }

  if (!gtk_get_current_event_state (&state))
    state = 0;

  /* Control-Activate is double-click */
  if (state & GDK_CONTROL_MASK) {
    aisleriot_game_record_move (priv->game, -1, NULL, 0);
    if (aisleriot_game_button_double_clicked_lambda (priv->game, focus_slot->id)) {
      aisleriot_game_end_move (priv->game);
    } else {
      aisleriot_game_discard_move (priv->game);
      aisleriot_board_error_bell (board);
    }

    aisleriot_game_test_end_of_game (priv->game);

    return;
  }

  /* Try single click action */
  aisleriot_game_record_move (priv->game, -1, NULL, 0);

  if (aisleriot_game_button_clicked_lambda (priv->game, focus_slot->id)) {
    aisleriot_game_end_move (priv->game);
    games_sound_play ("click");
    aisleriot_game_test_end_of_game (priv->game);

    return;
  }

  aisleriot_game_discard_move (priv->game);

  /* If we have a selection, and the topmost card of a slot is focused,
   * try to move the selected cards to the focused slot.
   *
   * NOTE: We cannot use aisleriot_game_drop_valid here since the
   * game may not support the "droppable" feature.
   */
  if (selection_slot != NULL &&
      selection_start_card_id >= 0 &&
      priv->focus_card_id == ((int) focus_slot->cards->len) - 1) {
    if (aisleriot_board_move_selected_cards_to_slot (board, focus_slot)) {
      /* Select the new topmost card */
      set_focus (board, focus_slot, ((int) focus_slot->cards->len - 1), TRUE);

      return;
    }

    /* Trying to move the cards has unset the selection; re-select them */
    set_selection (board, selection_slot, selection_start_card_id, TRUE);
  }

  aisleriot_board_error_bell (board);
}

static gboolean
aisleriot_board_move_cursor (AisleriotBoard *board,
                             GtkMovementStep step,
                             int count)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);
  guint state;
  gboolean is_control, is_shift, moved = FALSE;

  if (!GTK_WIDGET_HAS_FOCUS (widget))
    return FALSE;

  g_return_val_if_fail (step == GTK_MOVEMENT_LOGICAL_POSITIONS ||
                        step == GTK_MOVEMENT_VISUAL_POSITIONS ||
                        step == GTK_MOVEMENT_DISPLAY_LINES ||
                        step == GTK_MOVEMENT_PAGES ||
                        step == GTK_MOVEMENT_BUFFER_ENDS, FALSE);

  /* No focus? Set focus to the first/last slot */
  /* This will always return TRUE, no need for keynav-failed handling */
  if (!priv->focus_slot) {
    switch (step) {
      case GTK_MOVEMENT_DISPLAY_LINES:
      case GTK_MOVEMENT_PAGES:
        /* Focus the first slot */
        return aisleriot_board_move_cursor_start_end_by_slot (board, -1);
      case GTK_MOVEMENT_LOGICAL_POSITIONS:
      case GTK_MOVEMENT_VISUAL_POSITIONS:
        /* Move as if we'd been on the last/first slot */
        if (gtk_widget_get_direction (widget) != GTK_TEXT_DIR_RTL) {
          return aisleriot_board_move_cursor_start_end_by_slot (board, -count);
        }
        /* fall-through */
      default:
        return aisleriot_board_move_cursor_start_end_by_slot (board, count);
    }
  }

  g_assert (priv->focus_slot != NULL);

  if (!gtk_get_current_event_state (&state))
    state = 0;

  is_shift = (state & GDK_SHIFT_MASK) != 0;
  is_control = (state & GDK_CONTROL_MASK) != 0;

  switch (step) {
    case GTK_MOVEMENT_LOGICAL_POSITIONS:
    case GTK_MOVEMENT_VISUAL_POSITIONS:
      if (is_shift) {
        moved = aisleriot_board_extend_selection_left_right (board, count);
      } else {
        moved = aisleriot_board_move_cursor_left_right (board, count, is_control);
      }
      break;
    case GTK_MOVEMENT_DISPLAY_LINES:
      if (is_shift) {
        moved = aisleriot_board_extend_selection_up_down (board, count);
      } else {
        moved = aisleriot_board_move_cursor_up_down (board, count, is_control);
      }
      break;
    case GTK_MOVEMENT_PAGES:
      if (!is_shift) {
        moved = aisleriot_board_move_cursor_up_down (board, count, TRUE);
      }
      break;
    case GTK_MOVEMENT_BUFFER_ENDS:
      if (is_shift) {
        moved = aisleriot_board_extend_selection_start_end (board, count);
      } else if (is_control) {
        moved = aisleriot_board_move_cursor_start_end_by_slot (board, count);
      } else {
        moved = aisleriot_board_move_cursor_start_end_in_slot (board, count);
      }
      break;
    default:
      g_assert_not_reached ();
  }

  /* Show focus */
  if (!moved &&
      !priv->show_focus) {
    set_focus (board, priv->focus_slot, priv->focus_card_id, TRUE);
  }

  return moved;
}

static void
aisleriot_board_select_all (AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;
  Slot *focus_slot = priv->focus_slot;

  if (!focus_slot ||
      focus_slot->cards->len == 0 ||
      !aisleriot_board_extend_selection_in_slot_maximal (board)) {
    set_selection (board, NULL, -1, FALSE);
    aisleriot_board_error_bell (board);
  }
}

static void
aisleriot_board_deselect_all (AisleriotBoard *board)
{
  set_selection (board, NULL, -1, FALSE);
}

static void
aisleriot_board_toggle_selection (AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;
  Slot *focus_slot;
  int focus_card_id;

  focus_slot = priv->focus_slot;
  if (!focus_slot)
    return;

  focus_card_id = priv->focus_card_id;

  /* Focus not shown? Show it, and proceed */
  if (!priv->show_focus) {
    set_focus (board, focus_slot, focus_card_id, TRUE);
  }

  if (focus_card_id < 0) {
    aisleriot_board_error_bell (board);
    return;
  }

  /* If the selection isn't currently showing, don't truncate it.
   * Otherwise we get unexpected results when clicking on some cards
   * (which selects them but doesn't show the selection) and then press
   * Space or Shift-Up/Down etc.
   */
  if (priv->selection_slot == focus_slot &&
      priv->selection_start_card_id == focus_card_id &&
      priv->show_selection) {
    set_selection (board, NULL, -1, FALSE);
    return;
  }

  if (!aisleriot_game_drag_valid (priv->game,
                                  focus_slot->id,
                                  focus_slot->cards->data + focus_card_id,
                                  focus_slot->cards->len - focus_card_id)) {
    aisleriot_board_error_bell (board);
    return;
  }

  set_selection (board, focus_slot, focus_card_id, TRUE);
}

#endif /* ENABLE_KEYNAV */

/* GtkWidgetClass methods */
static void
aisleriot_board_realize (GtkWidget *widget)
{
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
  AisleriotBoardPrivate *priv = board->priv;
  GdkDisplay *display;

  GTK_WIDGET_CLASS (aisleriot_board_parent_class)->realize (widget);

  display = gtk_widget_get_display (widget);

  games_card_images_set_drawable (priv->images, widget->window);

  priv->draw_gc = gdk_gc_new (widget->window);

  priv->bg_gc = gdk_gc_new (widget->window);
  set_background_from_baize (widget, priv->bg_gc);
  
  priv->slot_gc = gdk_gc_new (widget->window);

#ifndef HAVE_HILDON 
  /* Create cursors */
  priv->cursor[CURSOR_DEFAULT] = gdk_cursor_new_for_display (display, GDK_LEFT_PTR);
  priv->cursor[CURSOR_OPEN] = make_cursor (widget, hand_open_data_bits, hand_open_mask_bits);
  priv->cursor[CURSOR_CLOSED] = make_cursor (widget, hand_closed_data_bits, hand_closed_mask_bits);
  priv->cursor[CURSOR_DROPPABLE] = gdk_cursor_new_for_display (display, GDK_DOUBLE_ARROW); /* FIXMEchpe: better cursor */
#endif /* !HAVE_HILDON */

  aisleriot_board_setup_geometry (board);
}

static void
aisleriot_board_unrealize (GtkWidget *widget)
{
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
  AisleriotBoardPrivate *priv = board->priv;
#ifndef HAVE_HILDON 
  guint i;
#endif

  priv->geometry_set = FALSE;

  g_object_unref (priv->draw_gc);
  priv->draw_gc = NULL;
  g_object_unref (priv->bg_gc);
  priv->bg_gc = NULL;
  g_object_unref (priv->slot_gc);
  priv->slot_gc = NULL;

#ifndef HAVE_HILDON 
  for (i = 0; i < LAST_CURSOR; ++i) {
    gdk_cursor_unref (priv->cursor[i]);
    priv->cursor[i] = NULL;
  }
#endif /* !HAVE_HILDON*/

  games_card_images_set_drawable (priv->images, NULL);

  clear_state (board);

  priv->slot_image = NULL;

  GTK_WIDGET_CLASS (aisleriot_board_parent_class)->unrealize (widget);
}

static void
aisleriot_board_screen_changed (GtkWidget *widget,
                                GdkScreen *previous_screen)
{
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
  GdkScreen *screen;
  GtkSettings *settings;

  if (GTK_WIDGET_CLASS (aisleriot_board_parent_class)->screen_changed) {
    GTK_WIDGET_CLASS (aisleriot_board_parent_class)->screen_changed (widget, previous_screen);
  }

  screen = gtk_widget_get_screen (widget);
  if (screen == previous_screen)
    return;

  if (previous_screen) {
    g_signal_handlers_disconnect_by_func (gtk_settings_get_for_screen (previous_screen),
                                          G_CALLBACK (aisleriot_board_settings_update),
                                          board);
  }

  if (screen == NULL)
    return;

  settings = gtk_settings_get_for_screen (screen);

  aisleriot_board_settings_update (settings, NULL, board);
  g_signal_connect (settings, "notify::gtk-double-click-time",
                    G_CALLBACK (aisleriot_board_settings_update), board);
#if GTK_CHECK_VERSION (2, 10, 0)
  g_signal_connect (settings, "notify::gtk-touchscreen-mode",
                    G_CALLBACK (aisleriot_board_settings_update), board);
#endif /* GTK >= 2.10.0 */
}

static void
aisleriot_board_style_set (GtkWidget *widget,
                           GtkStyle *previous_style)
{
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
  AisleriotBoardPrivate *priv = board->priv;
  GdkColor *colour = NULL;
  gboolean interior_focus;
  double card_slot_ratio;

  gtk_widget_style_get (widget,
                        "focus-line-width", &priv->focus_line_width,
                        "focus-padding", &priv->focus_padding,
                        "interior-focus", &interior_focus,
                        "selection-color", &colour,
                        "card-slot-ratio", &card_slot_ratio,
                        NULL);

#if 0
  g_print ("style-set: focus width %d padding %d interior-focus %s card-slot-ratio %.2f\n",
           priv->focus_line_width,
           priv->focus_padding,
           interior_focus ? "t" : "f",
           card_slot_ratio);
#endif

  priv->interior_focus = interior_focus != FALSE;

  priv->card_slot_ratio = card_slot_ratio;
  /* FIXMEchpe: if the ratio changed, we should re-layout the board below */

  if (colour != NULL) {
    priv->selection_colour = *colour;
    gdk_color_free (colour);
  } else {
    const GdkColor default_colour = { 0, 0 /* red */, 0 /* green */, 0xaa00 /* blue */};

    priv->selection_colour = default_colour;
  }

  games_card_images_set_selection_color (priv->images,
                                         &priv->selection_colour);

  GTK_WIDGET_CLASS (aisleriot_board_parent_class)->style_set (widget, previous_style);
}

static void
aisleriot_board_direction_changed (GtkWidget *widget,
                                   GtkTextDirection previous_direction)
{
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
  AisleriotBoardPrivate *priv = board->priv;
  GtkTextDirection direction;

  direction = gtk_widget_get_direction (widget);

  priv->is_rtl = (direction == GTK_TEXT_DIR_RTL);

  if (direction != previous_direction) {
    priv->force_geometry_update = TRUE;
  }

  /* This will queue a resize */
  GTK_WIDGET_CLASS (aisleriot_board_parent_class)->direction_changed (widget, previous_direction);
}

static void
aisleriot_board_size_allocate (GtkWidget *widget,
                               GtkAllocation *allocation)
{
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
  AisleriotBoardPrivate *priv = board->priv;
  gboolean is_same;

  is_same = (memcmp (&widget->allocation, allocation, sizeof (GtkAllocation)) == 0);
  
  GTK_WIDGET_CLASS (aisleriot_board_parent_class)->size_allocate (widget, allocation);

  if (is_same && !priv->force_geometry_update)
    return;

  priv->force_geometry_update = FALSE;

  if (GTK_WIDGET_REALIZED (widget)) {
    aisleriot_board_setup_geometry (board);
  }
}

static void
aisleriot_board_size_request (GtkWidget *widget,
                              GtkRequisition *requisition)
{
  requisition->width = BOARD_MIN_WIDTH;
  requisition->height = BOARD_MIN_HEIGHT;
}

#ifdef ENABLE_KEYNAV

static gboolean
aisleriot_board_focus (GtkWidget *widget,
                       GtkDirectionType direction)
{
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
  AisleriotBoardPrivate *priv = board->priv;

  if (!priv->focus_slot) {
    int count;

    switch (direction) {
      case GTK_DIR_TAB_FORWARD:
        count = 1;
        break;
      case GTK_DIR_TAB_BACKWARD:
        count = -1;
        break;
      default:
	/* FIXME: can this happen? */
	return GTK_WIDGET_CLASS (aisleriot_board_parent_class)->focus (widget, direction);
    }

    return aisleriot_board_move_cursor_start_end_by_slot (board, -count);
  }

#if 0
  if (aisleriot_board_move_cursor_left_right_by_slot (board, count, FALSE))
    return TRUE;
#endif

  return GTK_WIDGET_CLASS (aisleriot_board_parent_class)->focus (widget, direction);
}

#endif /* ENABLE_KEYNAV */

/* The gtkwidget.c focus in/out handlers queue a shallow draw;
 * that's ok for us but maybe we want to optimise this a bit to
 * only do it if we have a focus to draw/erase?
 */
static gboolean
aisleriot_board_focus_in (GtkWidget *widget,
                          GdkEventFocus *event)
{
#ifdef ENABLE_KEYNAV
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
  AisleriotBoardPrivate *priv = board->priv;

  /* Paint focus */
  if (priv->show_focus &&
      priv->focus_slot != NULL) {
    gdk_window_invalidate_rect (widget->window, &priv->focus_rect, FALSE);
  }
#endif /* ENABLE_KEYNAV */

  return FALSE;
}

static gboolean
aisleriot_board_focus_out (GtkWidget *widget,
                           GdkEventFocus *event)
{
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
#ifdef ENABLE_KEYNAV
  AisleriotBoardPrivate *priv = board->priv;
#endif /* ENABLE_KEYNAV */

  clear_state (board);

#ifdef ENABLE_KEYNAV
  /* Hide focus */
  if (priv->show_focus &&
      priv->focus_slot != NULL) {
    gdk_window_invalidate_rect (widget->window, &priv->focus_rect, FALSE);
  }
#endif /* ENABLE_KEYNAV */

  return FALSE;
}

static gboolean
aisleriot_board_button_press (GtkWidget *widget,
                              GdkEventButton *event)
{
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
  AisleriotBoardPrivate *priv = board->priv;
  Slot *hslot;
  int cardid;
  guint button;
  gboolean drag_valid;
  guint state;
  gboolean is_double_click, show_focus;

  /* NOTE: It's ok to just return instead of chaining up, since the
   * parent class has no class closure for this event.
   */

  /* ignore the gdk synthetic double/triple click events */
  if (event->type != GDK_BUTTON_PRESS)
    return FALSE;

  /* Don't do anything if a modifier is pressed */
  state = event->state & gtk_accelerator_get_default_mod_mask ();
  if (state != 0)
    return FALSE;

  button = event->button;

  /* We're only interested in left, middle and right-clicks */
  if (button < 1 || button > 3)
    return FALSE;

  /* If we already have a click, ignore this new one */
  if (priv->click_status != STATUS_NONE) {
    return FALSE;
  }

  /* If the game hasn't started yet, start it now */
  aisleriot_game_start (priv->game);

  get_slot_and_card_from_point (board, event->x, event->y, &hslot, &cardid);

  is_double_click = button == 2 ||
                    (priv->last_click_left_click &&
                     (event->time - priv->last_click_time <= priv->double_click_time) &&
                     priv->last_clicked_slot == hslot &&
                     priv->last_clicked_card_id == cardid);

  priv->last_click_x = event->x;
  priv->last_click_y = event->y;
  priv->last_clicked_slot = hslot;
  priv->last_clicked_card_id = cardid;
  priv->last_click_time = event->time;
  priv->last_click_left_click = button == 1;

  if (!hslot) {
    set_focus (board, NULL, -1, FALSE);
    set_selection (board, NULL, -1, FALSE);

    priv->click_status = STATUS_NONE;

    return FALSE;
  }

  set_cursor (board, CURSOR_CLOSED);

  /* First check if it's a right-click: if so, we reveal the card and do nothing else */
  if (button == 3) {
    /* Don't change the selection here! */
    reveal_card (board, hslot, cardid);

    return TRUE;
  }

  /* Clear revealed card */
  reveal_card (board, NULL, -1);

  /* We can't let Gdk do the double-click detection; since the entire playing
   * area is one big widget it can't distinguish between single-clicks on two
   * different cards and a double-click on one card.
   */
  if (is_double_click) {
    Slot *clicked_slot = hslot;

    priv->click_status = STATUS_NONE;

    /* Reset this since otherwise 3 clicks will be interpreted as 2 double-clicks */
    priv->last_click_left_click = FALSE;

    aisleriot_game_record_move (priv->game, -1, NULL, 0);
    if (aisleriot_game_button_double_clicked_lambda (priv->game, clicked_slot->id)) {
      aisleriot_game_end_move (priv->game);
    } else {
      aisleriot_game_discard_move (priv->game);
    }

    aisleriot_game_test_end_of_game (priv->game);

    set_cursor (board, CURSOR_OPEN);

    return TRUE;
  }

  /* button == 1 from now on */

  if (priv->selection_slot == NULL)
    goto set_selection;

  /* In click-to-move mode, we need to test whether moving the selected cards
   * to this slot does a move. Note that it is necessary to do this both if
   * the clicked slot is the selection_slot (and the clicked card the topmost
   * card below the selection), and if it's not the selection_slot, since some
   * games depend on this behaviour (e.g. Treize). See bug #565560.
   *
   * Note that aisleriot_board_move_selected_cards_to_slot unsets the selection,
   * so we need to fall through to set_selection if no move was done.
    */
  if (priv->click_to_move &&
      priv->selection_start_card_id >= 0 &&
      (hslot != priv->selection_slot || cardid + 1 == priv->selection_start_card_id)) {

    /* Try to move the selected cards to the clicked slot */
    if (aisleriot_board_move_selected_cards_to_slot (board, hslot))
      return TRUE;

    /* Move failed if this wasn't the selection_slot slot */
    if (hslot != priv->selection_slot) {
      aisleriot_board_error_bell (board);
    }
  }

  if (hslot != priv->selection_slot ||
      cardid != priv->selection_start_card_id)
    goto set_selection;
    
  /* Single click on the selected slot & card, we take that to mean to deselect,
   * but only in click-to-move mode.
   */
  if (priv->click_to_move) {
    set_selection (board, NULL, -1, FALSE);

    /* Reveal the card on left click */
    reveal_card (board, hslot, cardid);

    return TRUE;
  }

set_selection:

  if (cardid >= 0) {
    drag_valid = aisleriot_game_drag_valid (priv->game,
                                            hslot->id,
                                            hslot->cards->data + cardid,
                                            hslot->cards->len - cardid);
  } else {
    drag_valid = FALSE;
  }

  if (drag_valid) {
    set_selection (board, hslot, cardid, priv->click_to_move);
    priv->click_status = priv->click_to_move ? STATUS_NOT_DRAG : STATUS_MAYBE_DRAG;
  } else {
    set_selection (board, NULL, -1, FALSE);
    priv->click_status = STATUS_NOT_DRAG;
  }

  /* If we're already showing focus or just clicked on the
   * card with the (hidden) focus, show the focus on the
   * clicked card.
   */
  show_focus = priv->show_focus ||
               (hslot == priv->focus_slot &&
                cardid == priv->focus_card_id);
  set_focus (board, hslot, cardid, show_focus);

  /* Reveal the card on left click */
  if (priv->click_to_move) {
    reveal_card (board, hslot, cardid);
  }

  return FALSE;
}

static gboolean
aisleriot_board_button_release (GtkWidget *widget,
                                GdkEventButton *event)
{
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
  AisleriotBoardPrivate *priv = board->priv;
  /* guint state; */

  /* NOTE: It's ok to just return instead of chaining up, since the
   * parent class has no class closure for this event.
   */

  /* We just abort any action on button release, even if the button-up
   * is not the one that started the action. This greatly simplifies the code,
   * and is also the right thing to do, anyway.
   */

  /* state = event->state & gtk_accelerator_get_default_mod_mask (); */

  switch (priv->click_status) {
    case STATUS_SHOW:
      reveal_card (board, NULL, -1);
      break;

    case STATUS_IS_DRAG:
      highlight_drop_target (board, NULL);
      drop_moving_cards (board, event->x, event->y);
      break;

    case STATUS_MAYBE_DRAG:
    case STATUS_NOT_DRAG: {
      Slot *slot;
      int card_id;

      /* Don't do the action if the mouse moved away from the clicked slot; see bug #329183 */
      get_slot_and_card_from_point (board, event->x, event->y, &slot, &card_id);
      if (!slot || slot != priv->last_clicked_slot)
        break;

      aisleriot_game_record_move (priv->game, -1, NULL, 0);
      if (aisleriot_game_button_clicked_lambda (priv->game, slot->id)) {
        aisleriot_game_end_move (priv->game);
	games_sound_play_for_event ("click", (GdkEvent *) event);
      } else {
        aisleriot_game_discard_move (priv->game);
      }

      aisleriot_game_test_end_of_game (priv->game);

      break;
    }

    case STATUS_NONE:
      break;
  }

  priv->click_status = STATUS_NONE;

  set_cursor_by_location (board, event->x, event->y);

  return TRUE;
}

static gboolean
aisleriot_board_motion_notify (GtkWidget *widget,
                               GdkEventMotion * event)
{
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
  AisleriotBoardPrivate *priv = board->priv;

  /* NOTE: It's ok to just return instead of chaining up, since the
   * parent class has no class closure for this event.
   */

  if (priv->click_status == STATUS_IS_DRAG) {
    Slot *slot;
    int x, y;

    x = event->x - priv->last_click_x;
    y = event->y - priv->last_click_y;

    slot = find_drop_target (board, x, y);
    highlight_drop_target (board, slot);

    gdk_window_move (priv->moving_cards_window, x, y);
    /* FIXMEchpe: why? */
    gdk_window_clear (priv->moving_cards_window);

    set_cursor (board, CURSOR_CLOSED);
  } else if (priv->click_status == STATUS_MAYBE_DRAG &&
             gtk_drag_check_threshold (widget,
                                       priv->last_click_x,
                                       priv->last_click_y,
                                       event->x,
                                       event->y)) {
    drag_begin (board);
  } else {
    set_cursor_by_location (board, event->x, event->y);
  }

  return FALSE;
}

static gboolean
aisleriot_board_key_press (GtkWidget *widget,
                           GdkEventKey* event)
{
  return GTK_WIDGET_CLASS (aisleriot_board_parent_class)->key_press_event (widget, event);
}

#ifdef HAVE_MAEMO

static gboolean
aisleriot_board_tap_and_hold_query_cb (GtkWidget *widget,
                                       GdkEvent *_event,
                                       AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;
  /* BadDocs! should mention that this is a GdkEventButton! */
  GdkEventButton *event = (GdkEventButton *) _event;
  Slot *slot;
  int card_id;

  /* NOTE: Returning TRUE from this function means that tap-and-hold
   * is disallowed here; FALSE means to allow tap-and-hold.
   */

  /* In click-to-move mode, we always reveal with just the left click */
  if (priv->click_to_move)
    return TRUE;

  get_slot_and_card_from_point (board, event->x, event->y, &slot, &card_id);
  if (!slot ||
      card_id < 0 ||
      card_id == slot->cards->len - 1 ||
      CARD_GET_FACE_DOWN (CARD (slot->cards->data[card_id])))
    return TRUE;

  priv->tap_and_hold_slot = slot;
  priv->tap_and_hold_card_id = card_id;

  return FALSE; /* chain up to the default handler */
}

static void
aisleriot_board_tap_and_hold_cb (GtkWidget *widget,
                                 AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;

  if (priv->tap_and_hold_slot == NULL ||
      priv->tap_and_hold_card_id < 0)
    return;

  reveal_card (board, priv->tap_and_hold_slot, priv->tap_and_hold_card_id);
}

#endif /* HAVE_MAEMO */

static gboolean
aisleriot_board_expose_event (GtkWidget *widget,
                              GdkEventExpose *event)
{
  AisleriotBoard *board = AISLERIOT_BOARD (widget);
  AisleriotBoardPrivate *priv = board->priv;
  GdkRegion *region = event->region;
  GdkRectangle *rects;
  int i, n_rects;
  GPtrArray *slots;
  guint n_slots;
  Slot **exposed_slots;
  Slot *highlight_slot;
  guint n_exposed_slots;
  gboolean use_pixbuf_drawing = priv->use_pixbuf_drawing;

  /* NOTE: It's ok to just return instead of chaining up, since the
   * parent class has no class closure for this event.
   */

  if (event->window != widget->window)
    return FALSE;

  if (gdk_region_empty (region))
    return FALSE;

  /* First paint the background */

  gdk_region_get_rectangles (region, &rects, &n_rects);
  if (n_rects == 0)
    return FALSE;

#if 0
  {
    g_print ("Exposing area %d:%d@(%d,%d) ", event->area.width, event->area.height,
             event->area.x, event->area.y);
    for (i = 0; i < n_rects; ++i) {
      g_print ("[Rect %d:%d@(%d,%d)] ", rects[i].width, rects[i].height, rects[i].x, rects[i].y);
    }
    g_print ("\n");
  }
#endif

  for (i = 0; i < n_rects; ++i) {
    gdk_draw_rectangle (widget->window, priv->bg_gc,
                        TRUE,
                        rects[i].x, rects[i].y,
                        rects[i].width, rects[i].height);
  }
  g_free (rects);

  /* Only draw the the cards when the geometry is set, and we're in a resize */
  if (!priv->geometry_set)
    return TRUE;

  /* Now draw the slots and cards */
  slots = aisleriot_game_get_slots (priv->game);

  n_slots = slots->len;

  /* First check which slots are exposed */
  /* It's fine to allocate on the stack, since there'll never be very many slots */
  exposed_slots = g_newa (Slot*, n_slots);
  n_exposed_slots = 0;

  for (i = 0; i < n_slots; ++i) {
    Slot *slot = slots->pdata[i];

    /* Check whether this slot needs to be drawn */
    if (gdk_region_rect_in (region, &slot->rect) == GDK_OVERLAP_RECTANGLE_OUT)
      continue;

    exposed_slots[n_exposed_slots++] = slot;
  }

  highlight_slot = priv->highlight_slot;

  /* First draw the slots. Otherwise they'll overlap on cards
   * (e.g. in Elevator, after a card was removed).
   */
  for (i = 0; i < n_exposed_slots; ++i) {
    Slot *hslot = exposed_slots[i];
    int x, y;

    /* FIXMEchpe: if ((hslot->length - hslot->exposed) >= 0) ?? */
    if (hslot->cards->len > 0)
      continue;

    /* If the slot it empty, draw the slot image */
    x = hslot->rect.x;
    y = hslot->rect.y;

    gdk_gc_set_clip_origin (priv->slot_gc, x, y);

    if (PIXBUF_DRAWING_LIKELIHOOD (use_pixbuf_drawing)) {
      GdkPixbuf *pixbuf;

      if (G_LIKELY (hslot != highlight_slot)) {
        pixbuf = priv->slot_image;
      } else {
        pixbuf = games_card_images_get_slot_pixbuf (priv->images,
                                                    priv->show_highlight);
      }

      if (!pixbuf)
        continue;

      gdk_draw_pixbuf (widget->window, priv->slot_gc, pixbuf,
                       0, 0, x, y,
                       priv->card_size.width, priv->card_size.height,
                       GDK_RGB_DITHER_NONE, 0, 0);
    } else {
      GdkPixmap *pixmap;

      if (G_LIKELY (hslot != highlight_slot)) {
        pixmap = priv->slot_image;
      } else {
        pixmap = games_card_images_get_slot_pixmap (priv->images,
                                                    priv->show_highlight);
      }

      if (!pixmap)
        continue;

      gdk_draw_drawable (widget->window, priv->slot_gc, pixmap,
                         0, 0, x, y,
                         priv->card_size.width, priv->card_size.height);
    }
  }

  /* Now draw the cards */
  for (i = 0; i < n_exposed_slots; ++i) {
    Slot *hslot = exposed_slots[i];
    GByteArray *cards = hslot->cards;
    gpointer *card_images = hslot->card_images->pdata;
    GdkRectangle card_rect;
    guint j, n_cards;

    n_cards = cards->len;
    if (n_cards == 0)
      continue;

    card_rect.x = hslot->rect.x;
    card_rect.y = hslot->rect.y;
    card_rect.width = priv->card_size.width;
    card_rect.height = priv->card_size.height;

    if (priv->is_rtl &&
        hslot->expanded_right) {
      card_rect.x += hslot->rect.width - priv->card_size.width;
    }

    for (j = n_cards - hslot->exposed; j < n_cards; ++j) {
      /* Check whether this card needs to be drawn */
      /* FIXMEchpe: we can be even smarter here, by checking
       * with the rect of the part of the card that's not going 
       * to be obscured by later drawn cards anyway.
       */
      if (gdk_region_rect_in (region, &card_rect) == GDK_OVERLAP_RECTANGLE_OUT)
        goto next;

      if (PIXBUF_DRAWING_LIKELIHOOD (use_pixbuf_drawing)) {
        GdkPixbuf *pixbuf;

        pixbuf = card_images[j];
        if (!pixbuf)
          goto next;

        gdk_gc_set_clip_origin (priv->draw_gc, card_rect.x, card_rect.y);
        gdk_draw_pixbuf (widget->window, priv->draw_gc, pixbuf,
                         0, 0, card_rect.x, card_rect.y, -1, -1,
                         GDK_RGB_DITHER_NONE, 0, 0);
      } else {
        GdkPixmap *pixmap;

        pixmap = card_images[j];
        if (!pixmap)
          goto next;

        gdk_gc_set_clip_origin (priv->draw_gc, card_rect.x, card_rect.y);
        gdk_draw_drawable (widget->window, priv->draw_gc, pixmap,
                           0, 0, card_rect.x, card_rect.y, -1, -1);
      }

    next:

      card_rect.x += hslot->pixeldx;
      card_rect.y += hslot->pixeldy;
    }
  }

  /* Draw the revealed card */
  if (priv->show_card_slot != NULL &&
      gdk_region_rect_in (region, &priv->show_card_slot->rect) != GDK_OVERLAP_RECTANGLE_OUT) {
    GdkRectangle card_rect;

    get_rect_by_slot_and_card (board,
                               priv->show_card_slot,
                               priv->show_card_id,
                               1, &card_rect);

    if (PIXBUF_DRAWING_LIKELIHOOD (use_pixbuf_drawing)) {
      GdkPixbuf *pixbuf;

      pixbuf = priv->show_card_slot->card_images->pdata[priv->show_card_id];
      if (!pixbuf)
        goto draw_focus;

      gdk_gc_set_clip_origin (priv->draw_gc, card_rect.x, card_rect.y);
      gdk_draw_pixbuf (widget->window, priv->draw_gc, pixbuf,
                       0, 0, card_rect.x, card_rect.y, -1, -1,
                       GDK_RGB_DITHER_NONE, 0, 0);
    } else {
      GdkPixmap *pixmap;

      pixmap = priv->show_card_slot->card_images->pdata[priv->show_card_id];
      if (!pixmap)
        goto draw_focus;

      gdk_gc_set_clip_origin (priv->draw_gc, card_rect.x, card_rect.y);
      gdk_draw_drawable (widget->window, priv->draw_gc, pixmap,
                          0, 0, card_rect.x, card_rect.y, -1, -1);
    }
  }

draw_focus:

#ifdef ENABLE_KEYNAV
  if (G_UNLIKELY (priv->show_focus &&
                  priv->focus_slot != NULL &&
                  GTK_WIDGET_HAS_FOCUS (widget))) {
    GdkRectangle focus_rect;

    /* Check whether this needs to be drawn */
    if (gdk_region_rect_in (region, &priv->focus_rect) == GDK_OVERLAP_RECTANGLE_OUT)
      goto expose_done;

    if (priv->interior_focus) {
      focus_rect = priv->focus_rect;
    } else {
      get_rect_by_slot_and_card (board,
                                 priv->focus_slot,
                                 priv->focus_card_id,
                                 1,
                                 &focus_rect);
    }

    gtk_paint_focus (widget->style,
                     widget->window,
                     GTK_WIDGET_STATE (widget),
                     &priv->focus_rect,
                     widget,
                     NULL /* FIXME ? */,
                     focus_rect.x,
                     focus_rect.y,
                     focus_rect.width,
                     focus_rect.height);
  }

expose_done:
#endif /* ENABLE_KEYNAV */

  /* Parent class has no expose handler, no need to chain up */
  return TRUE;
}

/* GObjectClass methods */

static void
aisleriot_board_init (AisleriotBoard *board)
{
  GtkWidget *widget = GTK_WIDGET (board);
  AisleriotBoardPrivate *priv;

  priv = board->priv = AISLERIOT_BOARD_GET_PRIVATE (board);

  gtk_widget_set_name (widget, "aisleriot-board");

  GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_FOCUS);

  priv->is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;

  priv->force_geometry_update = FALSE;

  priv->click_to_move = FALSE;
  priv->show_selection = FALSE;

  priv->show_card_id = -1;

  priv->moving_cards = g_byte_array_sized_new (SLOT_CARDS_N_PREALLOC);

  gtk_widget_set_events (widget,
			 gtk_widget_get_events (widget) |
                         GDK_EXPOSURE_MASK |
                         GDK_BUTTON_PRESS_MASK |
                         GDK_POINTER_MOTION_MASK |
                         GDK_BUTTON_RELEASE_MASK);
  /* FIMXEchpe: no need for motion events on maemo, unless we explicitly activate drag mode */

#ifdef HAVE_MAEMO
  gtk_widget_tap_and_hold_setup (widget, NULL, NULL, GTK_TAP_AND_HOLD_PASS_PRESS);
  g_signal_connect (widget, "tap-and-hold-query",
                    G_CALLBACK (aisleriot_board_tap_and_hold_query_cb), board);
  g_signal_connect (widget, "tap-and-hold",
                    G_CALLBACK (aisleriot_board_tap_and_hold_cb), board);
#endif /* HAVE_MAEMO */
}

static GObject *
aisleriot_board_constructor (GType type,
			     guint n_construct_properties,
			     GObjectConstructParam *construct_params)
{
  GObject *object;
  AisleriotBoard *board;
  AisleriotBoardPrivate *priv;

  object = G_OBJECT_CLASS (aisleriot_board_parent_class)->constructor
            (type, n_construct_properties, construct_params);

  board = AISLERIOT_BOARD (object);
  priv = board->priv;

  g_assert (priv->game != NULL);

  priv->images = games_card_images_new ();

  return object;
}

static void
aisleriot_board_finalize (GObject *object)
{
  AisleriotBoard *board = AISLERIOT_BOARD (object);
  AisleriotBoardPrivate *priv = board->priv;

  g_signal_handlers_disconnect_matched (priv->game,
                                        G_SIGNAL_MATCH_DATA,
                                        0, 0, NULL, NULL, board);
  g_object_unref (priv->game);

  g_byte_array_free (priv->moving_cards, TRUE);

  g_object_unref (priv->images);

  if (priv->theme)
    g_object_unref (priv->theme);

#if 0
  screen = gtk_widget_get_settings (widget);
  g_signal_handlers_disconnect_matched (settings, G_SIGNAL_MATCH_DATA,
                                        0, 0, NULL, NULL,
                                        widget);
#endif

  G_OBJECT_CLASS (aisleriot_board_parent_class)->finalize (object);
}

static void
aisleriot_board_get_property (GObject *object,
                              guint prop_id,
                              GValue *value,
                              GParamSpec *pspec)
{
  AisleriotBoard *board = AISLERIOT_BOARD (object);

  switch (prop_id) {
    case PROP_THEME:
      g_value_set_object (value, aisleriot_board_get_card_theme (board));
      break;
  }
}

static void
aisleriot_board_set_property (GObject *object,
                              guint prop_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
  AisleriotBoard *board = AISLERIOT_BOARD (object);
  AisleriotBoardPrivate *priv = board->priv;

  switch (prop_id) {
    case PROP_GAME:
      priv->game = AISLERIOT_GAME (g_value_dup_object (value));

      g_signal_connect (priv->game, "game-type",
                        G_CALLBACK (game_type_changed_cb), board);
      g_signal_connect (priv->game, "game-cleared",
                        G_CALLBACK (game_cleared_cb), board);
      g_signal_connect (priv->game, "game-new",
                        G_CALLBACK (game_new_cb), board);
      g_signal_connect (priv->game, "slot-changed",
                        G_CALLBACK (slot_changed_cb), board);

      break;
    case PROP_THEME:
      aisleriot_board_set_card_theme (board, g_value_get_object (value));
      break;
  }
}

static void
aisleriot_board_class_init (AisleriotBoardClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
#ifdef ENABLE_KEYNAV
  GtkBindingSet *binding_set;
#endif

  g_type_class_add_private (gobject_class, sizeof (AisleriotBoardPrivate));

  gobject_class->constructor = aisleriot_board_constructor;
  gobject_class->finalize = aisleriot_board_finalize;
  gobject_class->set_property = aisleriot_board_set_property;
  gobject_class->get_property = aisleriot_board_get_property;

  widget_class->realize = aisleriot_board_realize;
  widget_class->unrealize = aisleriot_board_unrealize;
  widget_class->screen_changed = aisleriot_board_screen_changed;
  widget_class->style_set = aisleriot_board_style_set;
  widget_class->direction_changed = aisleriot_board_direction_changed;
  widget_class->size_allocate = aisleriot_board_size_allocate;
  widget_class->size_request = aisleriot_board_size_request;
#ifdef ENABLE_KEYNAV
  widget_class->focus = aisleriot_board_focus;
#endif /* ENABLE_KEYNAV */
  widget_class->focus_in_event = aisleriot_board_focus_in;
  widget_class->focus_out_event = aisleriot_board_focus_out;
  widget_class->button_press_event = aisleriot_board_button_press;
  widget_class->button_release_event = aisleriot_board_button_release;
  widget_class->motion_notify_event = aisleriot_board_motion_notify;
  widget_class->key_press_event = aisleriot_board_key_press;
  widget_class->expose_event = aisleriot_board_expose_event;

#ifdef ENABLE_KEYNAV
  klass->activate = aisleriot_board_activate;
  klass->move_cursor = aisleriot_board_move_cursor;
  klass->select_all = aisleriot_board_select_all;
  klass->deselect_all = aisleriot_board_deselect_all;
  klass->toggle_selection = aisleriot_board_toggle_selection;

  /* Keybinding signals */
  widget_class->activate_signal = signals[ACTIVATE] =
    g_signal_new (I_("activate"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (AisleriotBoardClass, activate),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);

  signals[MOVE_CURSOR] =
    g_signal_new (I_("move-cursor"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (AisleriotBoardClass, move_cursor),
                  NULL, NULL,
                  games_marshal_BOOLEAN__ENUM_INT,
                  G_TYPE_BOOLEAN,
                  2,
                  GTK_TYPE_MOVEMENT_STEP,
                  G_TYPE_INT);

  signals[TOGGLE_SELECTION] =
    g_signal_new (I_("toggle-selection"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (AisleriotBoardClass, toggle_selection),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);

  signals[SELECT_ALL] =
    g_signal_new (I_("select-all"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (AisleriotBoardClass, select_all),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);

  signals[DESELECT_ALL] =
    g_signal_new (I_("deselect-all"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (AisleriotBoardClass, deselect_all),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
#endif /* ENABLE_KEYNAV */

  /* Properties */
  g_object_class_install_property
    (gobject_class,
     PROP_GAME,
     g_param_spec_object ("game", NULL, NULL,
                          AISLERIOT_TYPE_GAME,
                          G_PARAM_WRITABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property
    (gobject_class,
     PROP_THEME,
     g_param_spec_object ("theme", NULL, NULL,
                          GAMES_TYPE_CARD_THEME,
                          G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  gtk_widget_class_install_style_property
    (widget_class,
     g_param_spec_boxed ("selection-color", NULL, NULL,
                         GDK_TYPE_COLOR,
                         G_PARAM_READABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  /**
   * AisleriotBoard:card-slot-ratio:
   *
   * The ratio of card to slot size. Note that this is the ratio of
   * card width/slot width and card height/slot height, not of
   * card area/slot area.
  */
  gtk_widget_class_install_style_property
    (widget_class,
     g_param_spec_double ("card-slot-ratio", NULL, NULL,
                          0.1, 1.0, DEFAULT_CARD_SLOT_RATIO,
                          G_PARAM_READABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

#ifdef ENABLE_KEYNAV
  /* Keybindings */
  binding_set = gtk_binding_set_by_class (klass);

  /* Cursor movement */
  aisleriot_board_add_move_and_select_binding (binding_set, GDK_Left, 0,
                                               GTK_MOVEMENT_VISUAL_POSITIONS, -1);
  aisleriot_board_add_move_and_select_binding (binding_set, GDK_KP_Left, 0,
                                               GTK_MOVEMENT_VISUAL_POSITIONS, -1);

  aisleriot_board_add_move_and_select_binding (binding_set, GDK_Right, 0,
                                               GTK_MOVEMENT_VISUAL_POSITIONS, 1);
  aisleriot_board_add_move_and_select_binding (binding_set, GDK_KP_Right, 0,
                                               GTK_MOVEMENT_VISUAL_POSITIONS, 1);
  
  aisleriot_board_add_move_and_select_binding (binding_set, GDK_Up, 0,
                                               GTK_MOVEMENT_DISPLAY_LINES, -1);
  aisleriot_board_add_move_and_select_binding (binding_set, GDK_KP_Up, 0,
                                               GTK_MOVEMENT_DISPLAY_LINES, -1);

  aisleriot_board_add_move_and_select_binding (binding_set, GDK_Down, 0,
                                               GTK_MOVEMENT_DISPLAY_LINES, 1);
  aisleriot_board_add_move_and_select_binding (binding_set, GDK_KP_Down, 0,
                                               GTK_MOVEMENT_DISPLAY_LINES, 1);

  aisleriot_board_add_move_and_select_binding (binding_set, GDK_Home, 0,
                                               GTK_MOVEMENT_BUFFER_ENDS, -1);
  aisleriot_board_add_move_and_select_binding (binding_set, GDK_KP_Home, 0,
                                               GTK_MOVEMENT_BUFFER_ENDS, -1);

  aisleriot_board_add_move_and_select_binding (binding_set, GDK_End, 0,
                                               GTK_MOVEMENT_BUFFER_ENDS, 1);
  aisleriot_board_add_move_and_select_binding (binding_set, GDK_KP_End, 0,
                                               GTK_MOVEMENT_BUFFER_ENDS, 1);

  aisleriot_board_add_move_binding (binding_set, GDK_Page_Up, 0,
                                    GTK_MOVEMENT_PAGES, -1);
  aisleriot_board_add_move_binding (binding_set, GDK_KP_Page_Up, 0,
                                    GTK_MOVEMENT_PAGES, -1);

  aisleriot_board_add_move_binding (binding_set, GDK_Page_Down, 0,
                                    GTK_MOVEMENT_PAGES, 1);
  aisleriot_board_add_move_binding (binding_set, GDK_KP_Page_Down, 0,
                                    GTK_MOVEMENT_PAGES, 1);

  /* Selection */
  gtk_binding_entry_add_signal (binding_set, GDK_space, 0,
                                "toggle-selection", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KP_Space, 0,
                                "toggle-selection", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_a, GDK_CONTROL_MASK,
                                "select-all", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_a, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                                "deselect-all", 0);

  /* Activate */
  aisleriot_board_add_activate_binding (binding_set, GDK_Return, 0);
  aisleriot_board_add_activate_binding (binding_set, GDK_ISO_Enter, 0);
  aisleriot_board_add_activate_binding (binding_set, GDK_KP_Enter, 0);
#endif /* ENABLE_KEYNAV */
}

/* public API */

GtkWidget *
aisleriot_board_new (AisleriotGame *game)
{
  return g_object_new (AISLERIOT_TYPE_BOARD,
                       "game", game,
                       NULL);
}

void
aisleriot_board_set_card_theme (AisleriotBoard *board,
                                GamesCardTheme *theme)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);

  g_return_if_fail (AISLERIOT_IS_BOARD (board));
  g_return_if_fail (GAMES_IS_CARD_THEME (theme));

  if (theme == priv->theme)
    return;

  if (priv->theme) {
    g_object_unref (priv->theme);
  }

  priv->geometry_set = FALSE;
  priv->slot_image = NULL;

  priv->theme = g_object_ref (theme);
  
  games_card_images_set_theme (priv->images, priv->theme);

  if (GTK_WIDGET_REALIZED (widget)) {
    /* Update card size and slot locations for new card theme (might have changed aspect!)*/
    aisleriot_board_setup_geometry (board);

    gtk_widget_queue_draw (widget);
  }

  g_object_notify (G_OBJECT (board), "theme");
}

GamesCardTheme *
aisleriot_board_get_card_theme (AisleriotBoard *board)
{
  AisleriotBoardPrivate *priv = board->priv;

  return priv->theme;
}

void
aisleriot_board_set_click_to_move (AisleriotBoard *board,
                                   gboolean click_to_move)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);

  click_to_move = click_to_move != FALSE;
  if (priv->click_to_move == click_to_move)
    return;

  /* Clear the selection. Do this before setting the new value,
   * since otherwise selection won't get cleared correctly.
   */
  set_selection (board, NULL, -1, FALSE);

  priv->click_to_move = click_to_move;

  /* FIXMEchpe why? */
  if (GTK_WIDGET_REALIZED (widget)) {
    gtk_widget_queue_draw (widget);
  }
}

void
aisleriot_board_abort_move (AisleriotBoard *board)
{
  clear_state (board);
}

void
aisleriot_board_set_pixbuf_drawing (AisleriotBoard *board,
                                    gboolean use_pixbuf_drawing)
{
  AisleriotBoardPrivate *priv = board->priv;
  GtkWidget *widget = GTK_WIDGET (board);

  use_pixbuf_drawing = use_pixbuf_drawing != FALSE;

  if (use_pixbuf_drawing == priv->use_pixbuf_drawing)
    return;

  priv->use_pixbuf_drawing = use_pixbuf_drawing;

  games_card_images_set_cache_mode (priv->images,
                                    use_pixbuf_drawing ? CACHE_PIXBUFS : CACHE_PIXMAPS);

  if (GTK_WIDGET_REALIZED (widget) &&
      priv->geometry_set) {
    /* Need to update the geometry, so we update the cached card images! */
    aisleriot_board_setup_geometry (board);

    gtk_widget_queue_draw (widget);
  }
}
