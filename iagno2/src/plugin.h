#ifndef _IAGNO2_PLUGIN_H_
#define _IAGNO2_PLUGIN_H_

#include <gnome.h>
#include <gmodule.h>

#include "../reversi-iagno2/reversi.h"

typedef struct {
	GModule *module;
	void (*plugin_init)();
	gint (*plugin_move)(ReversiBoard *board, int player);
	const gchar *(*plugin_name)();
  const gchar *(*plugin_busy_message)();
	void (*plugin_preferences)(GtkWidget *widget);
  void (*plugin_about_window)(GtkWidget *parent);
} Iagno2Plugin;

Iagno2Plugin *iagno2_plugin_open (const gchar *plugin_file);

void iagno2_plugin_close (Iagno2Plugin *plugin);

#endif
