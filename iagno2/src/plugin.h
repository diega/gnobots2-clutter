#ifndef _IAGNO2_PLUGIN_H_
#define _IAGNO2_PLUGIN_H_

#include <glib.h>
#include <gmodule.h>

typedef struct {
	GModule *module;
	void (*plugin_init)();
	gint (*plugin_move)(gchar *board);
	const gchar *(*plugin_name)();
  const gchar *(*plugin_busy_message)();
	gint (*plugin_preferences_cb)(GtkWidget *widget, gpointer data);
} Iagno2Plugin;

Iagno2Plugin *iagno2_plugin_open (const gchar *plugin_file);

void iagno2_plugin_close (Iagno2Plugin *plugin);

#endif
