/*
 * properties.c - Properties dialog and config settings
 * written by Mark Rae <Mark.Rae@ed.ac.uk>
 */

#include <config.h>
#include <gnome.h>

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>

#include "gbdefs.h"
#include "gnobots.h"

/*
 * Variables
 */
static GtkWidget *propdlg            = NULL;
static int        make_safe_move;
static int        make_super_safe_move;
static int        make_safe_teleport;
static int        make_sound_on;
static char      *default_scenario   = NULL;
static char      *scenario           = NULL;
static int        make_default;

int safe_move_on           = TRUE;
int super_safe_move_on     = TRUE;
int safe_teleport_on       = TRUE;
int sound_on               = TRUE;

/*
 * Function Prototypes
 */
void load_properties();
void save_properties();
void properties_cb(GtkWidget*, gpointer);
void set_scenario(char*);

static void properties_destroy_cb(GtkWidget*, gpointer);
static void properties_change_cb(GtkWidget*, gpointer);


/*
 * Handle the 'Safe moves' button messages
 */
static void set_move_cb(
GtkWidget *widget,
void *data
){
    make_safe_move = GTK_TOGGLE_BUTTON(widget)->active;
}

/*
 * Handle the 'Super safe moves' button messages
 */
static void set_super_cb(
GtkWidget *widget,
void *data
){
    make_super_safe_move = GTK_TOGGLE_BUTTON(widget)->active;
}

/*
 * Handle the 'Safe Teleport' button messages
 */
static void set_teleport_cb(
GtkWidget *widget,
void *data
){
    make_safe_teleport = GTK_TOGGLE_BUTTON(widget)->active;
}

/*
 * Handle the 'Sound' button messages
 */
static void set_sound_cb(
GtkWidget *widget,
void *data
){
    make_sound_on = GTK_TOGGLE_BUTTON(widget)->active;
}

/*
 * Handle the 'Default scenario' button messages
 */
static void set_default_cb(
GtkWidget *widget,
void *data
){
    make_default = GTK_TOGGLE_BUTTON(widget)->active;
}

/*
 * Change the selection when the dropdown menu is changed
 */
static void set_selection(
GtkWidget *widget,
void *data
){
    if(scenario) g_free(scenario);
    scenario = g_strdup(data);
}

/*
 * Free the memory used by the widgets in the dropdown menu
 * when it closes
 */
void free_str(
GtkWidget *widget,
void *data
){
        free(data);
}

/*
 * Fill the dropdown menu with the names of available game pixmaps
 *
 * This is a slightly modified version of the code used in
 * 'same-gnome' by Miguel de Icaza, Federico Mena and Horacio Peña.
 */
static void fill_optmenu(
GtkWidget *menu
){
    struct dirent *e;
    char *dname = gnome_unconditional_pixmap_file("gnobots");
    DIR *dir;
    int itemno = 0;
        
    dir = opendir(dname);

    if(!dir) return;
        
    while((e = readdir(dir)) != NULL){
        GtkWidget *item;
        char *s = strdup(e->d_name);

        if(!strstr(e->d_name, ".png")) {
            free(s);
            continue;
        }

        /* check that we don't include the two pixmaps used for */
        /* the speech bubbles */
        if(!strcmp(e->d_name, YAHOO_PIXMAP_NAME)){
            free(s);
            continue;
        }
                        
        if(!strcmp(e->d_name, AIEEE_PIXMAP_NAME)){
            free(s);
            continue;
        }
                        
        item = gtk_menu_item_new_with_label(s);
        gtk_widget_show(item);
        gtk_menu_append(GTK_MENU(menu), item);
        gtk_signal_connect(GTK_OBJECT(item), "activate",
                            (GtkSignalFunc)set_selection, s);
        gtk_signal_connect(GTK_OBJECT(item), "destroy",
                            (GtkSignalFunc)free_str, s);
          
        if(scenario && !strcmp(scenario, s))
        {
            gtk_menu_set_active(GTK_MENU(menu), itemno);
        }
          
        itemno++;
    }
    closedir(dir);
}

/*
 * Build the properties dialog box
 */
void properties_cb(
GtkWidget *widget,
gpointer  data
){
    GtkWidget *mainbox;
    
    GtkWidget *optionbox;
    GtkWidget *label;
    GtkWidget *optmenu;
    GtkWidget *menu;

    GtkWidget *togglebox;
    GtkWidget *safecb;
    GtkWidget *supersafecb;
    GtkWidget *telecb;
    GtkWidget *soundcb;    
    GtkWidget *defaultcb;
    
    GtkWidget *buttonbox;
    GtkWidget *button;
    
    if(propdlg) return;

    propdlg = gtk_window_new(GTK_WINDOW_DIALOG);
    gtk_widget_realize(propdlg);
    
    gtk_container_set_border_width(GTK_CONTAINER(propdlg), 10);
    GTK_WINDOW(propdlg)->position = GTK_WIN_POS_MOUSE;
    gtk_window_set_title(GTK_WINDOW(propdlg), _("Properties"));
    gtk_signal_connect(GTK_OBJECT(propdlg), "delete_event",
                       GTK_SIGNAL_FUNC(properties_destroy_cb), 0);

    mainbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(propdlg), mainbox);
                       
    optionbox = gtk_hbox_new(TRUE, 5);
    gtk_box_pack_start(GTK_BOX(mainbox), optionbox, TRUE, TRUE, 5);

    label = gtk_label_new (_("Select scenario:"));
    gtk_widget_show (label);
    gtk_box_pack_start_defaults (GTK_BOX(optionbox), label);

    optmenu = gtk_option_menu_new();
    menu = gtk_menu_new();
    fill_optmenu(menu);
    gtk_widget_show(optmenu);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu), menu);
    
    gtk_box_pack_start_defaults (GTK_BOX(optionbox), optmenu);
    
    gtk_widget_show(optionbox);

    
    togglebox = gtk_vbox_new(TRUE, 5);
    gtk_box_pack_start(GTK_BOX(mainbox), togglebox, TRUE, TRUE, 5);

    defaultcb = gtk_check_button_new_with_label(
                    _("Make it the default scenario"));
    GTK_TOGGLE_BUTTON(defaultcb)->active = make_default = 0;                    
    gtk_signal_connect (GTK_OBJECT(defaultcb), "clicked",
                        (GtkSignalFunc)set_default_cb, NULL);
    gtk_box_pack_start(GTK_BOX(togglebox), defaultcb, TRUE, TRUE, 5);
    gtk_widget_show (defaultcb);
    
    safecb = gtk_check_button_new_with_label(
                _("Safe mode"));
    GTK_TOGGLE_BUTTON(safecb)->active = make_safe_move = safe_move_on;
    gtk_signal_connect (GTK_OBJECT(safecb), "clicked",
                (GtkSignalFunc)set_move_cb, NULL);
    gtk_box_pack_start(GTK_BOX(togglebox), safecb, TRUE, TRUE, 5);
    gtk_widget_show (safecb);
    
    supersafecb = gtk_check_button_new_with_label(
                _("Super safe mode"));
    GTK_TOGGLE_BUTTON(supersafecb)->active = make_safe_move = safe_move_on;
    gtk_signal_connect (GTK_OBJECT(supersafecb), "clicked",
                (GtkSignalFunc)set_move_cb, NULL);
    gtk_box_pack_start(GTK_BOX(togglebox), supersafecb, TRUE, TRUE, 5);
    gtk_widget_show (supersafecb);
    
    telecb = gtk_check_button_new_with_label(
                _("Safe Teleports"));
    GTK_TOGGLE_BUTTON(telecb)->active = make_safe_teleport = safe_teleport_on;
    gtk_signal_connect (GTK_OBJECT(telecb), "clicked",
                (GtkSignalFunc)set_teleport_cb, NULL);
    gtk_box_pack_start(GTK_BOX(togglebox), telecb, TRUE, TRUE, 5);
    gtk_widget_show (telecb);
    
    soundcb = gtk_check_button_new_with_label(
                _("Sound"));
    GTK_TOGGLE_BUTTON(soundcb)->active = make_sound_on = sound_on;
    gtk_signal_connect (GTK_OBJECT(soundcb), "clicked",
                (GtkSignalFunc)set_sound_cb, NULL);
    gtk_box_pack_start(GTK_BOX(togglebox), soundcb, TRUE, TRUE, 5);
    gtk_widget_show (soundcb);
    
    gtk_widget_show(togglebox);

    
    buttonbox = gtk_hbox_new(TRUE, 5);
    gtk_box_pack_start(GTK_BOX(mainbox), buttonbox, TRUE, TRUE, 5);
                       
    button = gnome_stock_button(GNOME_STOCK_BUTTON_OK);
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
                       (GtkSignalFunc)properties_change_cb,
                       (gpointer)1);
    gtk_box_pack_start(GTK_BOX(buttonbox), button, TRUE, TRUE, 5);
    gtk_widget_show(button);

    button = gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL);
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
                       (GtkSignalFunc)properties_destroy_cb,
                       (gpointer)1);
    gtk_box_pack_start(GTK_BOX(buttonbox), button, TRUE, TRUE, 5);
    gtk_widget_show(button);

    gtk_widget_show(buttonbox);

    gtk_widget_show(mainbox);
    
    gtk_widget_show(propdlg);
}

/*
 * Change properties 
 */
void properties_change_cb(
GtkWidget *widget,
gpointer  data
){
    if(!propdlg) return;

    safe_move_on = make_safe_move;
    super_safe_move_on = make_super_safe_move;
    safe_teleport_on = make_safe_teleport;
    sound_on = make_sound_on;    

    if(make_default){
        if(default_scenario) g_free(default_scenario);
        default_scenario = g_strdup(scenario);        
    }

    load_tile_pixmap(scenario);
    clear_game_area();
    
    properties_destroy_cb(widget, data);
}


/*
 * Destroy the properties dialog box
 */
void properties_destroy_cb(
GtkWidget *widget,
gpointer  data
){
    if(!propdlg) return;

    gtk_widget_destroy(propdlg);

    propdlg = NULL;    
}

void set_scenario(
char *scn
){
    if(scenario) g_free(scenario);
    
    /* use default if none is specified */
    if(!scn){
        scenario = g_strdup(default_scenario);        
    } else {
        scenario = g_strdup(scn);
    }

    load_tile_pixmap(scenario);
    clear_game_area();
}

void load_properties(
){
    if(default_scenario) g_free(default_scenario);
    default_scenario = gnome_config_get_string_with_default(
            "/gnobots/Properties/Scenario=robots.png", NULL);
                            
    if(scenario) g_free(scenario);
    scenario = g_strdup(default_scenario);        

    safe_move_on = gnome_config_get_int_with_default(
                "/gnobots/Properties/SafeMove=1", NULL);
    super_safe_move_on = gnome_config_get_int_with_default(
                "/gnobots/Properties/SuperSafeMove=1", NULL);
    safe_teleport_on = gnome_config_get_int_with_default(
                "/gnobots/Properties/SafeTeleport=1", NULL);
    sound_on = gnome_config_get_int_with_default(
                "/gnobots/Properties/Sound=1", NULL);
}

void save_properties(
){
    gnome_config_set_string("/gnobots/Properties/Scenario", default_scenario);
    gnome_config_set_int("/gnobots/Properties/SafeMove", safe_move_on);
    gnome_config_set_int("/gnobots/Properties/SuperSafeMove", super_safe_move_on);
    gnome_config_set_int("/gnobots/Properties/SafeTeleport", safe_teleport_on);
    gnome_config_set_int("/gnobots/Properties/Sound", sound_on);
    
    gnome_config_sync();
}


