typedef struct {
	gboolean draw_grid;
	gchar *tileset;
} Iagno2Properties;

Iagno2Properties *iagno2_properties_new ();

void iagno2_properties_destroy (Iagno2Properties *properties);

Iagno2Properties *iagno2_properties_copy (Iagno2Properties *properties);

void iagno2_properties_sync (Iagno2Properties *properties);
