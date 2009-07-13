Gtk = imports.gi.Gtk;
Gio = imports.gi.Gio;
GtkBuilder = imports.gtkbuilder;
main = imports.main;
GConf = imports.gi.GConf;
ThemeLoader = imports.ThemeLoader;

GConf.init(Seed.argv);

// Defaults
var theme, colors, zealous, fly_score, size;
var default_theme = "Tango";
var default_size = 1;
var default_colors = 3;
var default_zealous = true;
var default_fly_score = true;

// Map theme names to themes
var themes = ThemeLoader.load_themes();
var sizes = [{name: "Small", columns: 6, rows: 5},
             {name: "Normal", columns: 15, rows: 10},
             {name: "Large", columns: 20, rows: 15}];

try
{
	gconf_client = GConf.Client.get_default();
	theme = themes[gconf_client.get_string("/apps/same-gnome-clutter/theme")];
	size = gconf_client.get_int("/apps/same-gnome-clutter/size");
	colors = gconf_client.get_int("/apps/same-gnome-clutter/colors");
	zealous = gconf_client.get_bool("/apps/same-gnome-clutter/zealous");
	fly_score = gconf_client.get_bool("/apps/same-gnome-clutter/fly_score");
	
	if(colors < 2 || colors > 4)
		colors = default_colors;
	
	if(theme == null)
		theme = themes[default_theme];
}
catch(e)
{
	print("Couldn't load settings from GConf.");
	theme = themes[default_theme];
	size = default_size;
	colors = default_colors;
	zealous = default_zealous;
	fly_score = default_fly_score;
}

// Settings Event Handler

SettingsWatcher = new GType({
	parent: Gtk.Button.type, // TODO: Can I make something inherit directly from GObject?!
	name: "SettingsWatcher",
	signals: [{name: "theme_changed"}, {name: "size_changed"}, {name: "colors_changed"}],
	init: function()
	{
		
	}
});

var Watcher = new SettingsWatcher();

// Settings UI

handlers = {
	select_theme: function(selector, ud)
	{
		new_theme = themes[selector.get_active_text()];

		if(new_theme == theme)
			return;
		
		theme = new_theme;
		ThemeLoader.load_theme(main.stage, theme);
		
		try
		{
			gconf_client.set_string("/apps/same-gnome-clutter/theme", selector.get_active_text());
		}
		catch(e)
		{
			print("Couldn't save settings to GConf.");
		}
	
		Watcher.signal.theme_changed.emit();
	},
	set_zealous_animation: function(widget, ud)
	{
		zealous = widget.active;
		
		try
		{
			gconf_client.set_bool("/apps/same-gnome-clutter/zealous", zealous);
		}
		catch(e)
		{
			print("Couldn't save settings to GConf.");
		}
	},
	set_fly_score: function(widget, ud)
	{
		fly_score = widget.active;
		
		try
		{
			gconf_client.set_bool("/apps/same-gnome-clutter/fly_score", fly_score);
		}
		catch(e)
		{
			print("Couldn't save settings to GConf.");
		}
	},
	update_size: function(widget, ud)
	{
		new_size = widget.get_active();
		
		if(new_size == size);
			return;
		
		size = new_size;
		
		try
		{
			gconf_client.set_int("/apps/same-gnome-clutter/size", size);
		}
		catch(e)
		{
			print("Couldn't save settings to GConf.");
		}
	
		Watcher.signal.size_changed.emit();
	},
	update_colors: function(widget, ud)
	{
		new_colors = widget.get_value();
		
		if(new_colors == colors)
			return;

		colors = new_colors;

		try
		{
			gconf_client.set_int("/apps/same-gnome-clutter/colors", colors);
		}
		catch(e)
		{
			print("Couldn't save settings to GConf.");
		}
	
		Watcher.signal.colors_changed.emit();
	},
	reset_defaults: function(widget, ud)
	{
		print("Not yet implemented.");
	}
};

// Settings UI Helper Functions

function show_settings()
{
	b = new Gtk.Builder();
	b.add_from_file(imports.Path.file_prefix + "/settings.ui");
	b.connect_signals(handlers);

	populate_theme_selector(b.get_object("theme-selector"));
	populate_size_selector(b.get_object("size-selector"));
	
	// Set current values
	b.get_object("size-selector").set_active(size);
	b.get_object("colors-spinner").value = colors;
	b.get_object("zealous-checkbox").active = zealous;
	b.get_object("fly-score-checkbox").active = fly_score;
	
	settings_dialog = b.get_object("dialog1");
	settings_dialog.set_transient_for(main.window);
	
	var result = settings_dialog.run();
	
	settings_dialog.destroy();
}

function populate_size_selector(selector)
{
	// Since we're using GtkBuilder, we can't make a Gtk.ComboBox.text. Instead,
	// we'll construct the cell renderer here, once, and use that.
	var cell = new Gtk.CellRendererText();
	selector.pack_start(cell, true);
	selector.add_attribute(cell, "text", 0);

	for(var i in sizes)
	{
		selector.append_text(sizes[i].name);
	}
}

function populate_theme_selector(selector)
{
	// Since we're using GtkBuilder, we can't make a Gtk.ComboBox.text. Instead,
	// we'll construct the cell renderer here, once, and use that.
	var cell = new Gtk.CellRendererText();
	selector.pack_start(cell, true);
	selector.add_attribute(cell, "text", 0);

	var i = 0;

	for(var th in themes)
	{
		selector.append_text(themes[th].name);
		
		if(themes[th].name == theme.name)
			selector.set_active(i);
		
		i++;
	}
}
