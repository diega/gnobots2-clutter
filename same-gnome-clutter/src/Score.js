Clutter = imports.gi.Clutter;
Pango = imports.gi.Pango;
main = imports.main;
Settings = imports.Settings;

Score = new GType({
	parent: Clutter.Group.type,
	name: "Score",
	init: function()
	{
		// Private
		var label;
		
		// Public
		this.hide_score = function (timeline, score)
		{
			if(!score)
				score = this;
			
			score.hide();
			main.stage.remove_actor(score);
		};
		
		this.animate_score = function (points)
		{
			if(points <= 0)
				return;
			
			label.set_font_name("Bitstrem Vera Sans Bold 40");
			label.set_text("+" + points);
			
			main.stage.add_actor(this);
			this.show();
			
			var a = this.animate(Clutter.AnimationMode.EASE_OUT_SINE,600,
			{
			    depth:  500,
			    opacity: 0
			});

			a.timeline.start();			
			a.timeline.signal.completed.connect(this.hide_score, this);
		};
		
		this.animate_final_score = function (points)
		{
			label.set_font_name("Bitstrem Vera Sans 50");
			label.set_markup("<b>Game Over!</b>\n" + points + " points");
			label.set_line_alignment(Pango.Alignment.CENTER);
			
			main.stage.add_actor(this);
			this.show();
			
			//this.opacity = 0;
			//this.y = -this.height;
			this.scale_x = this.scale_y = 0;
			
			var a = this.animate(Clutter.AnimationMode.EASE_OUT_ELASTIC,2000,
			{
			    scale_x: 1,
			    scale_y: 1,
				//y: [GObject.TYPE_INT, stage.width / 2],
			    opacity: 255,
			});
			
			a.timeline.start();
			//a.timeline.signal.completed.connect(this.hide_score, this);
		};
		
		// Implementation
		label = new Clutter.Text();
		label.set_color({red:255, green:255, blue:255, alpha:255});
		
		this.anchor_gravity = Clutter.Gravity.CENTER;
		this.add_actor(label);
		label.show();
		
		this.x = main.stage.width / 2;
		this.y = main.stage.height / 2;
	}
});
