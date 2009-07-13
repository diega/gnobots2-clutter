#!/usr/bin/env seed

// Configuration

var tiles_w = 15;
var tiles_h = 10;
var tile_size = 50;
var offset = tile_size/2;

var max_colors = 3;
var fly_score = true;

imports.gi.versions.Clutter = "0.9";

Gtk = imports.gi.Gtk;
GtkClutter = imports.gi.GtkClutter;
GtkBuilder = imports.gtkbuilder;
Clutter = imports.gi.Clutter;
GConf = imports.gi.GConf;
GnomeGamesSupport = imports.gi.GnomeGamesSupport;

GtkClutter.init(Seed.argv);
GnomeGamesSupport.runtime_init("same-gnome");
GnomeGamesSupport.stock_init();

GConf.init(Seed.argv);

Light = imports.Light;
Board = imports.Board;
Score = imports.Score;
About = imports.About;
Settings = imports.Settings;
ThemeLoader = imports.ThemeLoader;

handlers = {
	show_settings: function(selector, ud)
	{
		Settings.show_settings();
	},
	show_about: function(selector, ud)
	{
		About.show_about_dialog();
	},
	show_scores: function(selector, ud)
	{
	},
	new_game: function(selector, ud)
	{
		board.new_game();
	},
	quit: function(selector, ud)
	{
		Gtk.main_quit();
	}
};

size_o = Settings.sizes[Settings.size];

b = new Gtk.Builder();
b.add_from_file(imports.Path.file_prefix + "/same-gnome.ui");
b.connect_signals(handlers);

var window = b.get_object("game_window");
var clutter_embed = b.get_object("clutter");

var stage = clutter_embed.get_stage();

var current_score = 0;

stage.signal.hide.connect(Gtk.main_quit);
stage.set_use_fog(false);

stage.color = {alpha: 0};
stage.set_size((size_o.columns * tile_size),
               (size_o.rows * tile_size));
clutter_embed.set_size_request((size_o.columns * tile_size),
                               (size_o.rows * tile_size));

// NOTE: show the window before the stage, and the stage before any children
window.show_all();
stage.show_all();

ThemeLoader.load_theme(stage, Settings.theme);

function size_changed()
{
	size_o = Settings.sizes[Settings.size];
	
	stage.set_size((size_o.columns * tile_size),
	               (size_o.rows * tile_size));
	clutter_embed.set_size_request((size_o.columns * tile_size),
	                               (size_o.rows * tile_size));

	var new_board = new Board.Board();
	new_board.new_game();
	stage.add_actor(new_board);
	stage.remove_actor(board);
	board.show();
	board = new_board;
}

Settings.Watcher.signal.size_changed.connect(size_changed);

var board = new Board.Board();
stage.add_actor(board);
board.show();

board.new_game();

Gtk.main();

