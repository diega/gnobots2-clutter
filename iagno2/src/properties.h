#ifndef _IAGNO2_PROPERTIES_H_
#define _IAGNO2_PROPERTIES_H_

typedef struct {
	gboolean draw_grid;
	gchar *tileset;
	gchar *player1;
	gchar *player2;
} Iagno2Properties;

Iagno2Properties *iagno2_properties_new ();

void iagno2_properties_destroy (Iagno2Properties *properties);

Iagno2Properties *iagno2_properties_copy (Iagno2Properties *properties);

void iagno2_properties_sync (Iagno2Properties *properties);

#endif
