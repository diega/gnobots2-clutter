/*
 * properties.h - Exported variables and Functions from properties.c
 */

#ifndef PROPERTIES_H
#define PROPERTIES_H

extern int safe_move_on;
extern int super_safe_move_on;
extern int safe_teleport_on;
extern int sound_on;

void load_properties();
void save_properties();
void properties_cb(GtkWidget*, gpointer);
void set_scenario(char*);

#endif // PROPERTIES_H

