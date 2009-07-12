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
GdkPixbuf = imports.gi.GdkPixbuf;
GConf = imports.gi.GConf;
GLib = imports.gi.GLib;
Pango = imports.gi.Pango;
GObject = imports.gi.GObject;
GnomeGamesSupport = imports.gi.GnomeGamesSupport;
ThemeLoader = imports.ThemeLoader;

GtkClutter.init(Seed.argv);
GnomeGamesSupport.runtime_init("same-gnome");
GnomeGamesSupport.stock_init();

GConf.init(Seed.argv);

light = imports.light;
board = imports.board;
score = imports.score;
about = imports.about;

handlers = {
	show_settings: function(selector, ud)
	{
		//Settings.show_settings();
	},
	show_about: function(selector, ud)
	{
		about.show_about_dialog();
	},
	reset_score: function(selector, ud)
	{
		board.new_game();
	},
	quit: function(selector, ud)
	{
		Gtk.main_quit();
	}
};

b = new Gtk.Builder();
b.add_from_file(imports.path.file_prefix + "/same-gnome.ui");
b.connect_signals(handlers);

var window = b.get_object("game_window");
var clutter_embed = b.get_object("clutter");

var stage = clutter_embed.get_stage();

var current_score = 0;
var timelines = [];

stage.signal.hide.connect(Gtk.main_quit);

stage.color = {alpha: 0};
stage.set_size((tiles_w * tile_size),(tiles_h * tile_size));
clutter_embed.set_size_request((tiles_w * tile_size),(tiles_h * tile_size));

// NOTE: show the window before the stage, and the stage before any children
window.show_all();
stage.show_all();

theme = ThemeLoader.load_themes().Tango;
ThemeLoader.load_theme(stage, theme);

var board = new board.Board();
stage.add_actor(board);
board.show();

board.new_game();

Gtk.main();

