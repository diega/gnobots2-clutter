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

	return (tmp);
}

void
iagno2_properties_destroy (Iagno2Properties *properties)
{
	g_free (properties->tileset);
	g_free (properties);
}

Iagno2Properties *
iagno2_properties_copy (Iagno2Properties *properties)
{
	Iagno2Properties *tmp;

	tmp = (Iagno2Properties *) g_malloc (sizeof (Iagno2Properties));

	tmp->draw_grid = properties->draw_grid;
	tmp->tileset = g_strdup (properties->tileset);

	return (tmp);
}

void
iagno2_properties_sync (Iagno2Properties *properties)
{
	gnome_config_set_bool ("/iagno2/Preferences/draw_grid",
			properties->draw_grid);
	gnome_config_set_string ("/iagno2/Preferences/tileset",
			properties->tileset);
	gnome_config_sync ();
}
