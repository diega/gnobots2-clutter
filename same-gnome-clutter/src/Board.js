Clutter = imports.gi.Clutter;
GLib = imports.gi.GLib;
Light = imports.Light;
Score = imports.Score;
main = imports.main;
Settings = imports.Settings;

Board = new GType({
	parent: Clutter.Group.type,
	name: "Board",
	init: function(self)
	{
		// Private
		var lights = [], all_lights = [];
		var last_light;
		var animating = false;
		var anim_timeline;
		var final_score;
		
		// TODO: when a click is rejected, queue it up, like in the C version
		
		function done_animating()
		{
			animating = false;
			
			return false;
		}
		
		function _connected_lights(li)
		{
			if(!li || li.visited || li.get_closed())
				return [ ];
			
			var x = li.get_light_x();
			var y = li.get_light_y();
			
			li.visited = true;
			
			var con = [li];
			
			while(GLib.main_context_pending())
				GLib.main_context_iteration();
			
			var a = [], b = [], c = [], d = [];
			
			if(lights[x][y+1] && (li.get_state() == lights[x][y+1].get_state()))
				a = _connected_lights(lights[x][y+1]);
			
			if(lights[x][y-1] && (li.get_state() == lights[x][y-1].get_state()))
				b = _connected_lights(lights[x][y-1]);
			
			if(lights[x+1] && lights[x+1][y] && 
			   (li.get_state() == lights[x+1][y].get_state()))
				c = _connected_lights(lights[x+1][y]);
			
			if(lights[x-1] && lights[x-1][y] &&
			   (li.get_state() == lights[x-1][y].get_state()))
				d = _connected_lights(lights[x-1][y]);
			
			return con.concat(a,b,c,d);
		}
		
		function connected_lights(li)
		{
			for(var i in all_lights)
				all_lights[i].visited = false;
			
			if(!li.get_light_x) // We're picking something other than a light!
				return [ li ];
			
			return _connected_lights(li);
		}
		
		function calculate_score(n_lights)
		{
			if (n_lights < 3)
				return 0;

			return (n_lights - 2) * (n_lights - 2);
		}
		
		function light_lights_from(li)
		{
			var i;
			
			var cl = connected_lights(li);
			
			if(cl.length < 2)
				return false;
			
			for(i in cl)
				cl[i].opacity = 255;
			
			oldcl = cl;
		}
		
		function update_score(tiles)
		{
			var points_awarded = calculate_score(tiles);
			
			if(main.fly_score)
			{
				var score_text = new Score.Score();
				score_text.animate_score(points_awarded);
			}
			
			main.current_score += points_awarded;
			
			print(main.current_score);
			
			if(self.has_completed())
			{
				if(self.has_won())
					main.current_score += 1000;
				
				final_score = new Score.Score();
				final_score.animate_final_score(main.current_score);
				
				print("Done with: " + main.current_score + " points!");
			}
		}
		
		function light_entered(actor, event)
		{
			if(actor === last_light)
				return false;
			
			last_light = actor;
			
			light_lights_from(actor);
			
			return false;
		}
		
		function light_left(actor, event)
		{
			var connected = connected_lights(actor);
			
			for(var i in connected)
				if(!connected[i].get_closed())
					connected[i].opacity = 180;
			
			return false;
		}
		
		function colors_changed()
		{
			self.new_game();
		}
		
		// Public
		this.has_completed = function ()
		{
			for(var i in all_lights)
			{
				li = all_lights[i];
				
				if(!li.get_closed() && (connected_lights(li).length > 1))
					return false;
			}
			
			return true;
		};
		
		this.has_won = function ()
		{
			for(var i in all_lights)
			{
				li = all_lights[i];
				
				if(!li.get_closed())
					return false;
			}
			
			return true;
		};
		
		this.get_lights = function ()
		{
			return lights;
		};
		
		this.remove_region = function (actor, event)
		{
			if(animating)
				return false;
						
			var cl = connected_lights(actor);
			
			if(cl.length < 2)
				return false;
			
			var close_timeline = new Clutter.Timeline({duration: 500});
			
			for(var i in cl)
				cl[i].close_tile(close_timeline);
			
			close_timeline.start();
			
			var real_x = 0, timeline = 0;
			
			animating = true;
			
			anim_timeline = new Clutter.Timeline({duration: 500});
			
			for(var x in lights)
			{
				var y, li;
				var good_lights = [];
				var bad_lights = [];
				
				for(y in lights[x])
				{
					li = lights[x][y];
					
					if(!li.get_closed())
						good_lights.push(li);
					else
						bad_lights.push(li);
				}
				
				lights[real_x] = good_lights.concat(bad_lights);
				
				var empty_col = true;
				
				for(y in lights[real_x])
				{
					li = lights[real_x][y];
					
					li.set_light_x(real_x);
					li.set_light_y(parseInt(y,10));
					
					var new_x = real_x * main.tile_size + main.offset;
					var new_y = (main.size_o.rows - y - 1) * main.tile_size + main.offset;
					
					if(!li.get_closed() && ((new_x != li.x) ||
										    (new_y != li.y)))
					{
						li.animate_to(new_x, new_y, anim_timeline);
					}
					
					if(!li.get_closed())
						empty_col = false;
					
					GLib.main_context_iteration();
				}
				
				GLib.main_context_iteration();
				
				if(!empty_col)
					real_x++;
			}
		
			anim_timeline.signal.completed.connect(done_animating);
			anim_timeline.start();
			
			for(; real_x < main.size_o.columns; real_x++)
				lights[real_x] = null;
			
			update_score(cl.length);
			
			cl = last_light = null;
			
			return false;
		};
		
		this.new_game = function ()
		{
			var children = self.get_children();
			
			for(var i in children)
				self.remove_actor(children[i]);
			
			if(final_score)
				final_score.hide_score();
			
			main.current_score = 0;
			
			all_lights = [];
			
			for(var x = 0; x < main.size_o.columns; x++)
			{
				lights[x] = [];
				for(var y = 0; y < main.size_o.rows; y++)
				{
					var li = new Light.Light();
				
					li.set_light_x(x);
					li.set_light_y(y);
				
					li.set_position(x * main.tile_size + main.offset,
									(main.size_o.rows - y - 1) * main.tile_size + main.offset);
					self.add_actor(li);
					li.signal.button_release_event.connect(self.remove_region);
					li.signal.enter_event.connect(light_entered);
					li.signal.leave_event.connect(light_left);
				
					lights[x][y] = li;
					all_lights.push(lights[x][y]);
				}
			}
		};
		
		// Implementation
		this.reactive = true;
		
		Settings.Watcher.signal.colors_changed.connect(colors_changed);
	}
});
	