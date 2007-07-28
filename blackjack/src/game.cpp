// -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil -*-
/*
 * Blackjack - game.cpp
 *
 * Copyright (C) 2003-2004 William Jon McCann <mccann@jhu.edu>
 *
 * This game is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "blackjack.h"
#include "events.h"
#include "draw.h"
#include "chips.h"
#include "splash.h"
#include "player.h"
#include "hand.h"
#include "game.h"

#include <iostream>
using namespace std;

BJGameRules      *rules = NULL;
LoadablePlayer   *strategy = NULL;
Hand             *dealer = NULL;
Probabilities    *dealerProbabilities = NULL;
Shoe             *shoe = NULL;
BJShoe           *distribution = NULL;
PlayerHand       *player = NULL;

GList            *playerHands = NULL;
gfloat           lastWager = 5.0;
Card             tempCard;
PlayerHand       *tempHand = NULL;

gint             numDecks;
gint             dealerSpeed;

gint             numHands;

gchar            *game_file = "";
gchar            *game_name;

gboolean         allSettled = FALSE;
gboolean         game_is_done = FALSE;
gboolean         first_hand = TRUE;

GList *rules_list = NULL;

BJGameRules::BJGameRules (bool lhitSoft17, bool ldoubleAnyTotal, 
                          bool ldouble9, bool ldoubleSoft, 
                          bool ldoubleAfterHit, bool ldoubleAfterSplit,
                          bool lresplit, bool lresplitAces, 
                          bool llateSurrender, int lnumDecks, int ldealerSpeed)
        : BJRules (lhitSoft17, ldoubleAnyTotal, 
                   ldouble9, ldoubleSoft, 
                   ldoubleAfterHit, ldoubleAfterSplit,
                   lresplit, lresplitAces, 
                   llateSurrender)
{
        numDecks = lnumDecks;
        dealerSpeed = ldealerSpeed;
}

int
BJGameRules::getDealerSpeed ()
{
        return dealerSpeed;
}

int
BJGameRules::getNumDecks ()
{
        return numDecks;
}

void
bj_game_show_hand_counts ()
{
        GList *temptr;
        for (temptr = playerHands; temptr; temptr = temptr->next)
                ((PlayerHand*)temptr->data)->showCount ();
        if (! bj_game_is_active ())
                dealer->showCount ();
}

gchar *
bj_game_file_to_name (const gchar* file)
{
        char *p, *buf = g_path_get_basename (file);

        if ((p = strrchr (buf, '.')))
                *p = '\0';
        for (p = buf; p = strchr (p, '_'), p && *p;)
                *p = ' ';
        for (p = buf; p = strchr (p, '-'), p && *p;)
                *p = ' ';
        buf[0] = toupper (buf[0]);
        p = g_strdup (_(buf));

        g_free (buf);
        return p;
}

/* copied from gnome-util.c */
static const char *
extension_pointer (const char * path)
{
        char * s, * t;
        
        g_return_val_if_fail (path != NULL, NULL);

        /* get the dot in the last element of the path */
        t = g_utf8_strrchr (path, -1, G_DIR_SEPARATOR);
        if (t != NULL)
                s = g_utf8_strrchr (t, -1, '.');
        else
                s = g_utf8_strrchr (path, -1, '.');
        
        if (s == NULL)
                return path + strlen (path); /* There is no extension. */
        else {
                ++s;      /* pass the . */
                return s;
        }
}


int
bj_is_ruleset (const gchar *file_name)
{
        return (!strcmp (extension_pointer (file_name), "rules"));
}

void
bj_game_find_rules (gchar *variation)
{
        GDir *dir;
        G_CONST_RETURN gchar* file_name;
        gint n_games = 0;

        dir = g_dir_open (BJ_RULES_DIR, 0, NULL);
        if (dir == NULL)
                return;
  
        while ((file_name = g_dir_read_name (dir)) != NULL) {
                if (! bj_is_ruleset (file_name))
                        continue;

                n_games++;
                rules_list = g_list_append (rules_list, g_strdup (file_name));
                if (! g_ascii_strcasecmp (variation, file_name)) 
                        bj_set_game_variation (file_name);
        }

        g_dir_close (dir);
}

GList *
bj_game_get_rules_list ()
{
        return rules_list;
}

char *
bj_game_get_rules_name ()
{
        return game_name;
}

char *
bj_game_get_rules_file ()
{
        return game_file;
}

gboolean
bj_game_is_active ()
{
        return (! (game_is_done || first_hand));
}

gboolean
bj_game_is_first_hand ()
{
        return (first_hand);
}

void
bj_game_set_active (gboolean value)
{
        if (value) {
                game_is_done = FALSE;
                first_hand = FALSE;
        }
        else
                game_is_done = TRUE;

}

gdouble
bj_get_deal_delay ()
{
        return (bj_get_quick_deal ()) ? 1 : dealerSpeed;
}

static xmlXPathObjectPtr
getnodeset (xmlDocPtr doc,
            xmlChar  *xpath)
{	
	xmlXPathContextPtr context;
	xmlXPathObjectPtr  result;

	context = xmlXPathNewContext (doc);
	result = xmlXPathEvalExpression (xpath, context);
	if (xmlXPathNodeSetIsEmpty (result->nodesetval)) {
                g_warning ("Node set is empty for %s", xpath);
		return NULL;
        }

	xmlXPathFreeContext (context);

	return result;
}

static xmlChar *
get_first_xpath_value (xmlDocPtr doc,
                       xmlChar  *xpath)
{ 
        xmlChar          *keyword = NULL;
	xmlXPathObjectPtr result;
	xmlNodeSetPtr     nodeset;

	result = getnodeset (doc, xpath);
        if (result) {
		nodeset = result->nodesetval;
                if (nodeset->nodeNr > 0)
                        keyword = xmlGetProp (nodeset->nodeTab[0], (const xmlChar *) "value");
	}

        return keyword;
}

static gboolean
get_xml_rule_boolean (xmlDocPtr   doc,
                      const char *name,
                      gboolean    default_value)
{
        xmlChar *str;
        gboolean value;
        xmlChar *xpath;

        xpath = (xmlChar *)g_strdup_printf ("/BlackjackRuleDefinition/%s", name);

        str = get_first_xpath_value (doc, xpath);
        if (str)
                value = (strcmp ((char *)str, "TRUE") == 0);
        else
                value = default_value;

        xmlFree (str);

        return value;
}

static int
get_xml_rule_int (xmlDocPtr   doc,
                  const char *name,
                  gint        default_value)
{
        xmlChar *str;
        gint     value;
        xmlChar *xpath;

        xpath = (xmlChar *)g_strdup_printf ("/BlackjackRuleDefinition/%s", name);

        str = get_first_xpath_value (doc, xpath);
        if (str)
                value = atoi ((char *)str);
        else
                value = default_value;

        xmlFree (str);

        return value;
}

BJGameRules *
bj_game_read_rules (char *filename)
{
        BJGameRules *ruleset;
        gboolean use_default = FALSE;
        gboolean hitSoft17,
                doubleAnyTotal,
                double9,
                doubleSoft,
                doubleAfterHit,
                doubleAfterSplit,
                resplit,
                resplitAces,
                lateSurrender;
        gint lnumDecks,
                ldealerSpeed;
        char   *contents;
        gsize   length;
        GError *error = NULL;

        lnumDecks = 6;
        hitSoft17 = FALSE;
        doubleAnyTotal = TRUE;
        double9 = TRUE;
        doubleSoft = TRUE;
        doubleAfterHit = FALSE;
        doubleAfterSplit = TRUE;
        resplit = TRUE;
        resplitAces = TRUE;
        lateSurrender = TRUE;
        ldealerSpeed = 500;

        if (!g_file_get_contents (filename, &contents, &length, &error))
                use_default = TRUE;

        if (!use_default) {
             	xmlDocPtr  doc;
                xmlNodePtr node;

                contents = (char *)g_realloc (contents, length + 1);
                contents [length] = '\0';

                doc = xmlParseMemory (contents, length);
                if (doc == NULL)
                        doc = xmlRecoverMemory (contents, length);

                g_free (contents);

                /* If the document has no root, or no name */
                if (!doc || !doc->children || !doc->children->name) {
                        if (doc != NULL)
                                xmlFreeDoc (doc);
                        use_default = TRUE;
                } else {
                        node = xmlDocGetRootElement (doc);

                        lnumDecks = get_xml_rule_int (doc, "NumberOfDecks", lnumDecks);
                        ldealerSpeed = get_xml_rule_int (doc, "DealerSpeed", ldealerSpeed);

                        hitSoft17 = get_xml_rule_boolean (doc, "DealerHitsSoft17", hitSoft17);
                        doubleAnyTotal = get_xml_rule_boolean (doc, "DoubleDownAnyTotal", doubleAnyTotal);
                        double9 = get_xml_rule_boolean (doc, "DoubleDown9", double9);
                        doubleSoft = get_xml_rule_boolean (doc, "DoubleDownSoft", doubleSoft);
                        doubleAfterHit = get_xml_rule_boolean (doc, "DoubleDownAfterHit", doubleAfterHit);
                        doubleAfterSplit = get_xml_rule_boolean (doc, "DoubleDownAfterSplit", doubleAfterSplit);
                        resplit = get_xml_rule_boolean (doc, "ResplitAllowed", resplit);
                        resplitAces = get_xml_rule_boolean (doc, "ResplitAcesAllowed", resplitAces);
                        lateSurrender = get_xml_rule_boolean (doc, "SurrenderAllowed", lateSurrender);

                        xmlFreeDoc (doc);
                }
        }

        // Compute basic strategy.
        ruleset = new BJGameRules (hitSoft17, doubleAnyTotal, 
                                   double9, doubleSoft, 
                                   doubleAfterHit, doubleAfterSplit,
                                   resplit, resplitAces, 
                                   lateSurrender, lnumDecks, ldealerSpeed);
  
        return ruleset;
}


BJGameRules *
bj_game_find_and_read_rules (gchar *filename)
{
        gchar *installed_filename;
        BJGameRules *ruleset = NULL;

        installed_filename = g_build_filename (BJ_RULES_DIR, filename, NULL);

        if (g_file_test (installed_filename, G_FILE_TEST_EXISTS))
                ruleset = bj_game_read_rules (installed_filename);

        g_free (installed_filename);

        return ruleset;
}

static gchar *
bj_game_get_config_dir (void)
{
#define GNOME_DOT_GNOME            ".gnome2/"
        gchar *conf_dir = NULL;

        conf_dir = g_build_filename (g_get_home_dir (),
                                     GNOME_DOT_GNOME,
                                     "blackjack.d",
                                     NULL);
        return conf_dir;
}

static void
bj_game_ensure_config_dir_exists (const char *dir)
{
        if (g_file_test (dir, G_FILE_TEST_IS_DIR) == FALSE) {
                if (g_file_test (dir, G_FILE_TEST_EXISTS) == TRUE) {
                        // FIXME: use a dialog
                        cerr << dir << " exists, please move it out of the way." << endl;
                }
                if (g_mkdir (dir, 488) != 0)
                        cerr << "Failed to create directory " << dir << endl;
        }
}

static void
bj_game_eval_installed_file (gchar *file)
{
        char *installed_filename;

        if (g_file_test (file, G_FILE_TEST_EXISTS))
                return;
  
        installed_filename = g_build_filename (BJ_RULES_DIR, file, NULL);

        if (g_file_test (installed_filename, G_FILE_TEST_EXISTS)) {
                rules = bj_game_read_rules (installed_filename);
                g_free (installed_filename);
                
                // set globals
                numDecks = rules->getNumDecks ();
                dealerSpeed = rules->getDealerSpeed ();

                BJStrategy maxValueStrategy;
                Progress progress;

                gchar *cache_filename = g_strdup_printf ("%s%s", file, ".dat");

                char *config_dir = bj_game_get_config_dir ();
                bj_game_ensure_config_dir_exists (config_dir);
                installed_filename = g_build_filename (config_dir, cache_filename, NULL);
                g_free (config_dir);
                        
                gboolean use_cache = FALSE;
                if (g_file_test (installed_filename, G_FILE_TEST_EXISTS))
                        use_cache = TRUE;

                if (! use_cache)
                        splash_new ();
                strategy = new LoadablePlayer (numDecks, rules, 
                                               maxValueStrategy, 
                                               progress, 
                                               (use_cache) ? installed_filename : NULL);

                if (! use_cache) {
                        strategy->saveXML (installed_filename);
                        splash_destroy ();
                }

                g_free (cache_filename);
        } 
        else {
                gchar *message = g_strdup_printf ("%s\n %s", _("Blackjack can't load the requested file"),
                                                  installed_filename);
                gchar *message2 = _("Please check your Blackjack installation");
                GtkWidget *w = gtk_message_dialog_new_with_markup (GTK_WINDOW (toplevel_window),
                                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                                   GTK_MESSAGE_ERROR,
                                                                   GTK_BUTTONS_CLOSE,
                                                                   "<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s",
                                                                   message, message2);
                gtk_container_set_border_width (GTK_CONTAINER (w), 6);

                gtk_dialog_run (GTK_DIALOG (w));
                gtk_widget_destroy (w);
                g_free (message);
                exit (1);
        }
        g_free (installed_filename);
}

void 
bj_game_cancel ()
{
        bj_hand_cancel ();
        bj_game_set_active (FALSE);
        bj_press_data_free ();
        bj_chip_stack_press_data_free ();
}

void
bj_game_new (gchar* file, guint *seedp )
{
        gint min_w, min_h;

        bj_game_cancel ();

        first_hand = TRUE;

        bj_show_balance (bj_get_balance ());

        if (file && strcmp (file, game_file)) {
                game_file = file;
                
                bj_game_eval_installed_file (file);
                game_name = bj_game_file_to_name (file);
                
                if (option_dialog) {
                        gtk_widget_destroy (option_dialog);
                        option_dialog = NULL;
                }

                bj_set_game_variation (file);
        }

        if (seedp)
                seed = *seedp;
        else
                seed = g_random_int ();

        g_random_set_seed (seed);

        min_w = 600;
        min_h = 400;
        gtk_widget_set_size_request (playing_area, min_w, min_h);
        if (surface)
                bj_draw_refresh_screen ();

        // Prepare to play blackjack.

        if (dealer)
                delete dealer;
        dealer = new Hand;
        dealer->hslot = NULL;
        if (dealerProbabilities)
                delete dealerProbabilities;
        dealerProbabilities = new Probabilities (rules->getHitSoft17 ());
        if (shoe)
                delete shoe;
        shoe = new Shoe (numDecks);
        if (distribution)
                delete distribution;
        distribution = new BJShoe (numDecks);

        bj_clear_table ();

        bj_make_window_title (game_name);
        bj_update_control_menu ();
}

void
bj_clear_table ()
{
        GList* temptr;

        // delete all slots except 0,1
        delete_surface ();
        numHands = 1;

        for (temptr = playerHands; temptr; temptr = temptr->next)
                g_free (temptr->data);
        g_list_free (playerHands);
        playerHands = NULL;
        player = (PlayerHand*)g_malloc (sizeof (PlayerHand));
        playerHands = g_list_append (playerHands, player);
  
        // Create slots
        bj_slot_add (0);
        bj_slot_add (1);
        bj_draw_set_geometry (1, 2);

        // Clear the table.
        dealer->hslot = (hslot_type) g_list_nth_data (slot_list, 0);
        player->hslot = (hslot_type) g_list_nth_data (slot_list, 1);

        bj_chip_stack_new_with_value (bj_get_wager (),
                                      player->hslot->x - 0.2,
                                      player->hslot->y + 0.2);

        // Create source chip stacks 
        bj_chip_stack_create_sources ();

        bj_draw_rescale_cards ();

        player->nextHand = NULL;
        player->reset ();
        dealer->reset ();
        dealerProbabilities->reset ();

        first_hand = TRUE;
        gtk_widget_queue_draw (playing_area);
}

