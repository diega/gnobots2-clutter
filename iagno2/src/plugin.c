#include <gnome.h>
#include <dirent.h>
#include <gmodule.h>

#include "plugin.h"

Iagno2Plugin *
iagno2_plugin_open (const gchar *plugin_file)
{
  Iagno2Plugin *tmp;
  const gchar *(*plugin_move)();

  tmp = (Iagno2Plugin *) g_malloc (sizeof (Iagno2Plugin));

  if (!(tmp->module = g_module_open (plugin_file, 0))) {
    printf ("Loading plugin %s failed.\n", plugin_file);
    return NULL;
  }

  g_module_symbol (tmp->module, "plugin_init",
      ((gpointer)&(tmp->plugin_init)));

  if (!g_module_symbol (tmp->module, "plugin_move",
                        ((gpointer)&tmp->plugin_move))) {
    g_module_close (tmp->module);
    printf ("Loading plugin %s failed.\n", plugin_file);
    return NULL;
  }

  if (!g_module_symbol (tmp->module, "plugin_name",
                        ((gpointer)&(tmp->plugin_name)))) {
    g_module_close (tmp->module);
    printf ("Loading plugin %s failed.\n", plugin_file);
    return NULL;
  }

  tmp->plugin_busy_message = NULL;
  g_module_symbol (tmp->module, "plugin_busy_message",
                   ((gpointer)&(tmp->plugin_busy_message)));

  tmp->plugin_preferences = NULL;
  g_module_symbol (tmp->module, "plugin_preferences",
                   ((gpointer)&(tmp->plugin_preferences)));

  if (!g_module_symbol (tmp->module, "plugin_about_window",
                        ((gpointer)&(tmp->plugin_about_window)))) {
    tmp->plugin_about_window = NULL;
  }

  return tmp;
}

void
iagno2_plugin_close (Iagno2Plugin *plugin)
{
  if (plugin != NULL) {
    g_module_close (plugin->module);
    g_free (plugin);
  }
}
