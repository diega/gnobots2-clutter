#ifndef _IAGNO2_PLUGIN_H_
#define _IAGNO2_PLUGIN_H_

#include <gnome.h>
#include <gmodule.h>

#include "../reversi-iagno2/reversi.h"

typedef struct {
	GModule *module;

  /* Perform whatever initialization your plugin needs, ie init a random number
   * generator, set up a network connection... */
	void (*plugin_init)();

  /* Find a move on the given ReversiBoard for the given player.  Iagno2 has
   * already checked that you have a move, so you /must return a move (index
   * 0-63). */
	gint (*plugin_move)(ReversiBoard *board, int player);

  /* Return the string that is the name of your plugin.  This will show up in
   * the option menu in the preferences dialog. */
	const gchar *(*plugin_name)();

  /* The message you want your plugin to display in the status window while
   * deciding on a move to make. */
  const gchar *(*plugin_busy_message)();

  /* Pops up a configuration dialog (this should be modal) to configure your
   * plugin.  The plugin /must/ save the configuration options entered to disk
   * (using gnome_config, for example), as it is not guaranteed that this is the
   * same copy of the plugin that will be loaded in a given game.  This does not
   * have to be defined. */
	void (*plugin_preferences)(GtkWidget *widget);

  /* An about window (see the random plugin for an example), does not have to be
   * defined. */
  void (*plugin_about_window)(GtkWidget *parent);
} Iagno2Plugin;

Iagno2Plugin *iagno2_plugin_open (const gchar *plugin_file);

void iagno2_plugin_close (Iagno2Plugin *plugin);

#endif
