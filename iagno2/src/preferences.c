#include <gnome.h>
#include <dirent.h>

#include "defines.h"
#include "properties.h"
#include "iagno2.h"
#include "plugin.h"

static GtkWidget *preferences_dialog = NULL;
static Iagno2Properties *tmp_properties;

extern Iagno2Properties *properties;
extern GtkWidget *app;

extern gchar whose_turn;
extern gboolean game_in_progress;

static void
destroy_cb (GtkWidget *widget, gpointer data)
{
  iagno2_properties_destroy (tmp_properties);
}

static void
apply_cb (GtkWidget *widget, gint pagenum, gpointer data)
{
  gboolean new_tileset = 0;
  gboolean toggle_grid = 0;

  if (pagenum == -1) {
    Iagno2Properties *old_properties;

    old_properties = iagno2_properties_copy (properties);
    
    iagno2_properties_destroy (properties);
    properties = iagno2_properties_copy (tmp_properties);
    iagno2_properties_sync (properties);

    if (strcmp (old_properties->tileset, properties->tileset)) {
      new_tileset = 1;
    }

    if (old_properties->draw_grid != properties->draw_grid) {
      toggle_grid = 1;
    }

    if (new_tileset) {
      iagno2_tileset_load ();
      iagno2_set_bg_color ();
      iagno2_force_board_redraw ();
    }

    if (toggle_grid || new_tileset) {
      iagno2_show_grid_lines ();
    }

    if (strcmp (old_properties->player1, properties->player1)) {
      iagno2_initialize_players (1);
      if ((whose_turn == BLACK_TILE) && game_in_progress) {
        iagno2_setup_current_player (FALSE);
      }
    }

    if (strcmp (old_properties->player2, properties->player2)) {
      iagno2_initialize_players (2);
      if ((whose_turn == WHITE_TILE) && game_in_progress) {
        iagno2_setup_current_player (FALSE);
      }
    }

    iagno2_properties_destroy (old_properties);
  }
}

static void
draw_grid_cb (GtkWidget *widget, gpointer data)
{
  if (!preferences_dialog)
    return;

  if (GTK_TOGGLE_BUTTON (widget)->active) {
    tmp_properties->draw_grid = 1;
  } else {
    tmp_properties->draw_grid = 0;
  }

  gnome_property_box_changed (GNOME_PROPERTY_BOX (preferences_dialog));
}

static void
tileset_cb (GtkWidget *widget, gpointer data)
{
  if (!preferences_dialog)
    return;

  if (strcmp ((gchar *)data, tmp_properties->tileset)) {
    g_free (tmp_properties->tileset);
    tmp_properties->tileset = g_strdup (data);
    gnome_property_box_changed (GNOME_PROPERTY_BOX (preferences_dialog));
  }
}

static void
player1_cb (GtkWidget *widget, gpointer data)
{
  if (!preferences_dialog)
    return;

  if (strcmp ((gchar *)data, tmp_properties->player1)) {
    g_free (tmp_properties->player1);
    tmp_properties->player1 = g_strdup (data);
    gnome_property_box_changed (GNOME_PROPERTY_BOX (preferences_dialog));
  }
}

static void
player2_cb (GtkWidget *widget, gpointer data)
{
  if (!preferences_dialog)
    return;

  if (strcmp ((gchar *)data, tmp_properties->player2)) {
    g_free (tmp_properties->player2);
    tmp_properties->player2 = g_strdup (data);
    gnome_property_box_changed (GNOME_PROPERTY_BOX (preferences_dialog));
  }
}

static void
configure_cb (GtkWidget *widget, gpointer data)
{
  gchar *name;
  gchar *filename;
  Iagno2Plugin *plugin;
  
  if ((gint) data == 1) {
    if (!strcmp ("Human", tmp_properties->player1)) {
      gnome_ok_dialog_parented (_("What does configuring yourself mean?"),
                                GTK_WINDOW (preferences_dialog));
      return;
    }
    name = g_strconcat ("iagno2/", tmp_properties->player1, NULL);
  } else {
    if (!strcmp ("Human", tmp_properties->player2)) {
      gnome_ok_dialog_parented (_("What does configuring yourself mean?"),
                                GTK_WINDOW (preferences_dialog));
      return;
    }
    name = g_strconcat ("iagno2/", tmp_properties->player2, NULL);
  }

  filename = gnome_unconditional_libdir_file (name);

  if (!(plugin = iagno2_plugin_open (filename))) {
    g_free (filename);
    return;
  }

  if (plugin->plugin_preferences) {
    plugin->plugin_preferences (preferences_dialog);
  } else {
    gnome_ok_dialog_parented (_("This plugin has no configuration options."),
                              GTK_WINDOW (preferences_dialog));
  }

  iagno2_plugin_close (plugin);

  g_free (filename);
  g_free (name);
}

static void
about_cb (GtkWidget *widget, gpointer data)
{
  gchar *name;
  gchar *filename;
  Iagno2Plugin *plugin;
  
  if ((gint) data == 1) {
    if (!strcmp ("Human", tmp_properties->player1)) {
      gnome_ok_dialog_parented (_("Why don't you tell me about yourself?"),
                                GTK_WINDOW (preferences_dialog));
      return;
    }
    name = g_strconcat ("iagno2/", tmp_properties->player1, NULL);
  } else {
    if (!strcmp ("Human", tmp_properties->player2)) {
      gnome_ok_dialog_parented (_("Why don't you tell me about yourself?"),
                                GTK_WINDOW (preferences_dialog));
      return;
    }
    name = g_strconcat ("iagno2/", tmp_properties->player2, NULL);
  }

  filename = gnome_unconditional_libdir_file (name);

  if (!(plugin = iagno2_plugin_open (filename))) {
    g_free (filename);
    return;
  }

  if (plugin->plugin_about_window) {
    plugin->plugin_about_window (preferences_dialog);
  } else {
    gnome_ok_dialog_parented (_("This plugin has no about information."),
                              GTK_WINDOW (preferences_dialog));
  }

  iagno2_plugin_close (plugin);

  g_free (filename);
  g_free (name);
}

static void
free_str (GtkWidget *widget, gpointer data)
{
  g_free (data);
}

static void
fill_tileset_menu (GtkWidget *menu)
{
  struct dirent *entry;
  gchar *directory_name;
  DIR *directory;
  gint itemno = 0;

  directory_name = gnome_unconditional_pixmap_file ("iagno2");

  directory = opendir (directory_name);

  if (!directory)
    return;

  while ((entry = readdir (directory)) != NULL) {
    GtkWidget *item;
    gchar *tileset = g_strdup (entry->d_name);
    if (!strstr (tileset, ".png")) {
      g_free (tileset);
      continue;
    }

    item = gtk_menu_item_new_with_label (tileset);
    gtk_menu_append (GTK_MENU (menu), item);
    gtk_signal_connect (GTK_OBJECT (item), "activate",
                        GTK_SIGNAL_FUNC (tileset_cb), tileset);
    gtk_signal_connect (GTK_OBJECT (item), "destroy",
                        GTK_SIGNAL_FUNC (free_str), tileset);

    if (!strcmp(properties->tileset, tileset)) {
      gtk_menu_set_active (GTK_MENU (menu), itemno);
    }

    itemno++;
  }

  closedir (directory);
}

static void
fill_plugin_menu (GtkWidget *menu, int player)
{
  struct dirent *entry;
  gchar *directory_name;
  DIR *directory;
  gint itemno = 1;
  GtkWidget *item;

  directory_name = gnome_unconditional_libdir_file ("iagno2/");
  
  directory = opendir (directory_name);

  if (!directory) {
    return;
  }

  item = gtk_menu_item_new_with_label (_("Human"));
  gtk_menu_append (GTK_MENU (menu), item);
  gtk_widget_show (item);
  if (player == 1) {
    gtk_signal_connect (GTK_OBJECT (item), "activate",
                        GTK_SIGNAL_FUNC (player1_cb), "Human");
    if (!strcmp (properties->player1, "Human")) {
      gtk_menu_set_active (GTK_MENU (menu), 0);
    }
  } else {
    gtk_signal_connect (GTK_OBJECT (item), "activate",
                        GTK_SIGNAL_FUNC (player2_cb), "Human");
    if (!strcmp (properties->player2, "Human")) {
      gtk_menu_set_active (GTK_MENU (menu), 0);
    }
  }

  while ((entry = readdir (directory)) != NULL) {
    gchar *plugin_file_name = g_strdup (entry->d_name);
    Iagno2Plugin *plugin;
    gchar *full_plugin_file_name;
    gchar *plugin_name;

    if (!strstr (plugin_file_name, ".so.0.0.0")) {
      g_free (plugin_file_name);
      continue;
    }

    full_plugin_file_name = g_strconcat (directory_name, plugin_file_name,
                                         NULL);

    plugin = iagno2_plugin_open (full_plugin_file_name);

    plugin_name = g_strdup (plugin->plugin_name ());

    iagno2_plugin_close (plugin);

    g_free (full_plugin_file_name);

    item = gtk_menu_item_new_with_label (plugin_name);
    gtk_menu_append (GTK_MENU (menu), item);
    gtk_widget_show (item);
    if (player == 1) {
      gtk_signal_connect (GTK_OBJECT (item), "activate",
                          GTK_SIGNAL_FUNC (player1_cb), plugin_file_name);
      if (!strcmp(properties->player1, plugin_file_name)) {
        gtk_menu_set_active (GTK_MENU (menu), itemno);
      }
    } else {
      gtk_signal_connect (GTK_OBJECT (item), "activate",
                          GTK_SIGNAL_FUNC (player2_cb), plugin_file_name);
      if (!strcmp(properties->player2, plugin_file_name)) {
        gtk_menu_set_active (GTK_MENU (menu), itemno);
      }
    }
    gtk_signal_connect (GTK_OBJECT (item), "destroy",
                        GTK_SIGNAL_FUNC (free_str), plugin_file_name);


    g_free (plugin_name);

    itemno++;
  }
}

void
iagno2_preferences_cb (GtkWidget *widget, gpointer data)
{
  GtkWidget *label;
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *option_menu;
  GtkWidget *menu;
  GtkWidget *vbox;
  GtkWidget *hbox2;
  GtkWidget *button;
  
  if (preferences_dialog) {
    return;
  }

  tmp_properties = iagno2_properties_copy (properties);

  preferences_dialog = gnome_property_box_new ();
  gnome_dialog_set_parent (GNOME_DIALOG (preferences_dialog), GTK_WINDOW (app));
  gtk_window_set_title (GTK_WINDOW (preferences_dialog),
                        _("Iagno2 Preferences"));
  gtk_signal_connect (GTK_OBJECT (preferences_dialog), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroyed),
                      &preferences_dialog);
  gtk_signal_connect (GTK_OBJECT (preferences_dialog), "destroy",
                      GTK_SIGNAL_FUNC (destroy_cb), NULL);

  label = gtk_label_new (_("Players"));

  hbox = gtk_hbox_new (TRUE, GNOME_PAD_SMALL);
  gtk_container_border_width (GTK_CONTAINER (hbox), GNOME_PAD_SMALL);

  frame = gtk_frame_new (_("Dark Player"));

  option_menu = gtk_option_menu_new ();
  menu = gtk_menu_new ();
  fill_plugin_menu (menu, 1);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);

  vbox = gtk_vbox_new (TRUE, GNOME_PAD_SMALL);
  gtk_container_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

  gtk_box_pack_start (GTK_BOX (vbox), option_menu, TRUE, TRUE, 0);

  hbox2 = gtk_hbox_new (TRUE, GNOME_PAD_SMALL);
  gtk_container_border_width (GTK_CONTAINER (hbox2), 0);

  button = gtk_button_new_with_label (_("Configure..."));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (configure_cb), (gpointer) 1);
  gtk_box_pack_start (GTK_BOX (hbox2), button, TRUE, TRUE, 0);
  button = gtk_button_new_with_label (_("About..."));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (about_cb), (gpointer) 1);
  gtk_box_pack_start (GTK_BOX (hbox2), button, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), hbox2, TRUE, TRUE, 0);

  gtk_container_add (GTK_CONTAINER (frame), vbox);

  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

  frame = gtk_frame_new (_("Light Player"));

  option_menu = gtk_option_menu_new ();
  menu = gtk_menu_new ();
  fill_plugin_menu (menu, 2);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);

  vbox = gtk_vbox_new (TRUE, GNOME_PAD_SMALL);
  gtk_container_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

  gtk_box_pack_start (GTK_BOX (vbox), option_menu, TRUE, TRUE, 0);

  hbox2 = gtk_hbox_new (TRUE, GNOME_PAD_SMALL);
  gtk_container_border_width (GTK_CONTAINER (hbox2), 0);

  button = gtk_button_new_with_label (_("Configure..."));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (configure_cb), (gpointer) 2);
  gtk_box_pack_start (GTK_BOX (hbox2), button, TRUE, TRUE, 0);
  button = gtk_button_new_with_label (_("About..."));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (about_cb), (gpointer) 2);
  gtk_box_pack_start (GTK_BOX (hbox2), button, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), hbox2, TRUE, TRUE, 0);

  gtk_container_add (GTK_CONTAINER (frame), vbox);

  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

  gnome_property_box_append_page (GNOME_PROPERTY_BOX (preferences_dialog),
                                  hbox, label);

  gtk_signal_connect (GTK_OBJECT (preferences_dialog), "apply",
                      GTK_SIGNAL_FUNC (apply_cb), NULL);

  gtk_widget_show_all (preferences_dialog);


  /*
  GtkWidget *label;
  GtkWidget *table;
  GtkWidget *button;
  GtkWidget *hbox;
  GtkWidget *label2;
  GtkWidget *option_menu;
  GtkWidget *menu;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *box;

  if (preferences_dialog)
    return;

  tmp_properties = iagno2_properties_copy (properties);

  preferences_dialog = gnome_property_box_new ();
  gnome_dialog_set_parent (GNOME_DIALOG (preferences_dialog),
                           GTK_WINDOW (app));
  gtk_window_set_title (GTK_WINDOW (preferences_dialog),
                        _("Iagno2 Preferences"));
  gtk_signal_connect (GTK_OBJECT (preferences_dialog), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroyed),
                      &preferences_dialog);
  gtk_signal_connect (GTK_OBJECT (preferences_dialog), "destroy",
                      GTK_SIGNAL_FUNC (destroy_cb), NULL);

  // First page of properties

  label = gtk_label_new (_("Graphics"));

  table = gtk_table_new (1, 2, TRUE);
  gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD);
  gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);
  gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD);
  
  button = gtk_check_button_new_with_label (_("Draw grid lines"));
  if (properties->draw_grid)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  gtk_signal_connect (GTK_OBJECT (button), "toggled",
                      GTK_SIGNAL_FUNC (draw_grid_cb), NULL);

  gtk_table_attach (GTK_TABLE (table), button,
                    0, 1,
                    0, 1,
                    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
                    0, 0);

  hbox = gtk_hbox_new (FALSE, GNOME_PAD);

  label2 = gtk_label_new (_("Tileset: "));

  gtk_box_pack_start (GTK_BOX (hbox), label2, FALSE, FALSE, 0);

  option_menu = gtk_option_menu_new ();
  menu = gtk_menu_new ();
  fill_tileset_menu (menu);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);

  gtk_box_pack_start (GTK_BOX (hbox), option_menu, TRUE, TRUE, 0);

  gtk_table_attach (GTK_TABLE (table), hbox,
                    1, 2,
                    0, 1,
                    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
                    0, 0);

  gnome_property_box_append_page (GNOME_PROPERTY_BOX (preferences_dialog),
                                  table, label);

  // Second page of properties

  label = gtk_label_new (_("Players"));

  table = gtk_table_new (1, 2, TRUE);
  gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD);
  gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);
  gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD);

  frame = gtk_frame_new (_("Player 1"));
  gtk_container_border_width (GTK_CONTAINER (frame), 0);

  vbox = gtk_vbox_new (TRUE, GNOME_PAD);
  gtk_container_border_width (GTK_CONTAINER (vbox), GNOME_PAD);

  option_menu = gtk_option_menu_new ();
  menu = gtk_menu_new ();
  fill_tileset_menu (menu);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
  
  gtk_box_pack_start (GTK_BOX (vbox), option_menu, TRUE, TRUE, 0);

  gtk_container_add (GTK_CONTAINER (frame), vbox);

  gtk_table_attach (GTK_TABLE (table), frame,
                    0, 1,
                    0, 1,
                    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
                    0, 0);

  frame = gtk_frame_new (_("Player 2"));
  gtk_container_border_width (GTK_CONTAINER (frame), 0);

  gtk_table_attach (GTK_TABLE (table), frame,
                    1, 2,
                    0, 1,
                    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
                    0, 0);

  gnome_property_box_append_page (GNOME_PROPERTY_BOX (preferences_dialog),
                                  table, label);

  // Set it all up

  gtk_signal_connect (GTK_OBJECT (preferences_dialog), "apply",
                      GTK_SIGNAL_FUNC (apply_cb), NULL);

  gtk_widget_show_all (preferences_dialog);
  */
}
