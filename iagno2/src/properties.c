#include <gnome.h>

#include "properties.h"

Iagno2Properties *iagno2_properties_new ()
{
  Iagno2Properties *tmp;

  tmp = (Iagno2Properties *) g_malloc (sizeof (Iagno2Properties));

  tmp->draw_grid = gnome_config_get_bool
      ("/iagno2/Preferences/draw_grid=true");
  tmp->tileset = gnome_config_get_string
      ("/iagno2/Preferences/tileset=classic.png");

  tmp->player1 = gnome_config_get_string
      ("/iagno2/Preferences/player1=Human");

  tmp->player2 = gnome_config_get_string
      ("/iagno2/Preferences/player2=libiagno2-random.so");

  return (tmp);
}

void
iagno2_properties_destroy (Iagno2Properties *properties)
{
  g_free (properties->tileset);
  g_free (properties->player1);
  g_free (properties->player2);
  g_free (properties);
}

Iagno2Properties *
iagno2_properties_copy (Iagno2Properties *properties)
{
  Iagno2Properties *tmp;

  tmp = (Iagno2Properties *) g_malloc (sizeof (Iagno2Properties));

  tmp->draw_grid = properties->draw_grid;
  tmp->tileset = g_strdup (properties->tileset);
  tmp->player1 = g_strdup (properties->player1);
  tmp->player2 = g_strdup (properties->player2);

  return (tmp);
}

void
iagno2_properties_sync (Iagno2Properties *properties)
{
  gnome_config_set_bool ("/iagno2/Preferences/draw_grid",
                         properties->draw_grid);
  gnome_config_set_string ("/iagno2/Preferences/tileset",
                           properties->tileset);
  gnome_config_set_string ("/iagno2/Preferences/player1",
                           properties->player1);
  gnome_config_set_string ("/iagno2/Preferences/player2",
                           properties->player2);
  gnome_config_sync ();
}
