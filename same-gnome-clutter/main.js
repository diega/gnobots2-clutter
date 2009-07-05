#!/usr/bin/env seed

// Configuration

var file_prefix = "/usr/local/share/gnome-games/same-gnome-clutter";

var tiles_w = 15;
var tiles_h = 10;
var tile_size = 50;
var offset = tile_size/2;

var max_colors = 3;
var fly_score = true;

imports.gi.versions.Clutter = "0.9";

Gtk = imports.gi.Gtk;
GtkClutter = imports.gi.GtkClutter;
Clutter = imports.gi.Clutter;
GdkPixbuf = imports.gi.GdkPixbuf;
GConf = imports.gi.GConf;
GLib = imports.gi.GLib;
Pango = imports.gi.Pango;
GObject = imports.gi.GObject;
GnomeGamesSupport = imports.gi.GnomeGamesSupport;

GtkClutter.init(Seed.argv);
GnomeGamesSupport.runtime_init("same-gnome");
GnomeGamesSupport.stock_init();

GConf.init(Seed.argv);

light = imports.light;
board = imports.board;
score = imports.score;

b = new Gtk.Builder();
b.add_from_file(file_prefix + "/same-seed.ui");
//b.connect_signals(handlers);

var window = b.get_object("main_window");
var clutter_embed = b.get_object("clutter");
window.signal.hide.connect(Gtk.main_quit);
//b.get_object("game_vbox").pack_start(clutter_embed, true, true);

var stage = clutter_embed.get_stage();

var current_score = 0;
var timelines = [];

stage.signal.hide.connect(Gtk.main_quit);

stage.color = {alpha: 0};
stage.set_size((tiles_w * tile_size),(tiles_h * tile_size));
clutter_embed.set_size_request((tiles_w * tile_size),(tiles_h * tile_size));

// TODO: determine size of window before we show it
// NOTE: show the window before the stage, and the stage before any children
window.show_all();
stage.show_all();

var board = new board.Board();
stage.add_actor(board);
stage.show_all();

board.new_game();

Gtk.main();

