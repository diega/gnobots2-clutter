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
    return NULL;
  }

  g_module_symbol (tmp->module, "plugin_init",
      ((gpointer)&(tmp->plugin_init)));

  if (!g_module_symbol (tmp->module, "plugin_move",
                        ((gpointer)&tmp->plugin_move))) {
    g_module_close (tmp->module);
    return NULL;
  }

  if (!g_module_symbol (tmp->module, "plugin_name",
                        ((gpointer)&(tmp->plugin_name)))) {
    g_module_close (tmp->module);
    return NULL;
  }

  g_module_symbol (tmp->module, "plugin_preferences_cb",
                   ((gpointer)&(tmp->plugin_preferences_cb)));

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
