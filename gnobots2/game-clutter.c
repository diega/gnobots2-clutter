#include <clutter/clutter.h>
#include <gtk/gtk.h> //for gnobots.h
#include <libgames-support/games-scores.h> //for gnobots.h

#include "game-clutter.h"
#include "gnobots.h"
#include "graphics.h"
#include "gbdefs.h"
#include "cursors.h"

ClutterActor *get_player();
void delete_clutter_player();
void move_explosion (gint, gint, gint, gint);
void delete_clutter_explosion (gint, gint);
gboolean explosion_exists(gint, gint);
ClutterActor *get_explosion(gint, gint);
gboolean robot1_exists (gint, gint);
ClutterActor *get_robot1 (gint, gint);
gboolean robot2_exists (gint, gint);
ClutterActor *get_robot2 (gint, gint);
void move_clutter_robot (gint, gint, gint, gint);
void delete_clutter_robot (gint, gint);
void resize_clutter_cb (GtkWidget *, GtkAllocation *, gpointer);
gboolean clutter_move_cb (GtkWidget *, GdkEventMotion *, gpointer);

static void delete_clutter_actor (ClutterActor *actor);

static ClutterActor *player;
static ClutterActor *explosions[GAME_WIDTH][GAME_HEIGHT];
static ClutterActor *robots2[GAME_WIDTH][GAME_HEIGHT];
static ClutterActor *robots1[GAME_WIDTH][GAME_HEIGHT];

ClutterActor*
get_player(){
  if (player == NULL){
    player = clutter_texture_new_from_file ("img/player.png", NULL);
    clutter_container_add_actor (CLUTTER_CONTAINER (stage), player);
    clutter_actor_set_anchor_point_from_gravity (player, CLUTTER_GRAVITY_CENTER);
    clutter_actor_set_scale (player, 0.4, 0.4);
  }
  return player;
}

void
delete_clutter_player ()
{
  delete_clutter_actor (get_player());
  player = NULL;
}

static void
delete_clutter_actor (ClutterActor* actor){
  clutter_actor_hide (actor);
  clutter_actor_destroy (actor);
}

void
move_explosion (gint origx, gint origy, gint x, gint y)
{
  move_clutter_object(x, y, explosions[origx][origy]);
  explosions[x][y] = explosions[origx][origy];
  explosions[origx][origy] = NULL;
}

void
delete_clutter_explosion (gint x, gint y)
{
  delete_clutter_actor (get_explosion(x, y));
  explosions[x][y] = NULL;
}

gboolean
explosion_exists(gint x, gint y)
{
  return NULL != explosions[x][y];
}

ClutterActor*
get_explosion(gint x, gint y)
{
  ClutterActor *explosion = explosions[x][y];
  if (NULL == explosion){
    explosion = clutter_texture_new_from_file ("img/explosion.png", NULL);
    explosions[x][y] = explosion;
    clutter_container_add_actor (CLUTTER_CONTAINER (stage), explosion);
    clutter_actor_set_anchor_point_from_gravity (explosion, CLUTTER_GRAVITY_CENTER);
    clutter_actor_set_scale (explosion, 0.4, 0.4);
  }
  return explosion;
}

gboolean
robot1_exists(gint x, gint y)
{
  return NULL != robots1[x][y];
}

ClutterActor*
get_robot1(gint x, gint y)
{
  ClutterActor *robot = robots1[x][y];
  if (NULL == robot){
    robot = clutter_texture_new_from_file ("img/robot1.png", NULL);
    robots1[x][y] = robot;
    clutter_container_add_actor (CLUTTER_CONTAINER (stage), robot);
    clutter_actor_set_anchor_point_from_gravity (robot, CLUTTER_GRAVITY_CENTER);
    clutter_actor_set_scale (robot, 0.4, 0.4);
  }
  return robot;
}

gboolean
robot2_exists(gint x, gint y){
  return NULL != robots2[x][y];
}

ClutterActor*
get_robot2(gint x, gint y){
  ClutterActor *robot = robots2[x][y];
  if (NULL == robot){
    robot = clutter_texture_new_from_file ("img/robot2.png", NULL);
    robots2[x][y] = robot;
    clutter_container_add_actor (CLUTTER_CONTAINER (stage), robot);
    clutter_actor_set_anchor_point_from_gravity (robot, CLUTTER_GRAVITY_CENTER);
    clutter_actor_set_scale (robot, 0.4, 0.4);
  }
  return robot;
}

void
move_clutter_robot (gint origx, gint origy, gint x, gint y)
{
  if (robot1_exists(origx, origy))
  {
    move_clutter_object( x, y, robots1[origx][origy]);
    robots1[x][y] = robots1[origx][origy];
    robots1[origx][origy] = NULL;
  } else if (robot2_exists(origx, origy))
  {
    move_clutter_object( x, y, robots2[origx][origy]);
    robots2[x][y] = robots2[origx][origy];
    robots2[origx][origy] = NULL;
  }
}

void
delete_clutter_robot (gint x, gint y)
{
  if (robot1_exists(x, y))
  {
    delete_clutter_actor(robots1[x][y]);
    robots1[x][y] = NULL;
  } else if (robot2_exists(x, y))
  {
    clutter_actor_hide (robots2[x][y]);
    clutter_actor_destroy (robots2[x][y]);
    robots2[x][y] = NULL;
  }
}

/*
void
resize_clutter_cb (GtkWidget *widget, GtkAllocation *allocation, gpointer data)
{
  g_printf("me llaman\n");
  gint i, j;

  scale_clutter_object(player_xpos, player_ypos, allocation->width, allocation->height, get_player());
  for (i = 0; i < GAME_WIDTH; ++i) {
    for (j = 0; j < GAME_HEIGHT; ++j) {
      if (robot1_exists(i, j))
        scale_clutter_object(i, j, allocation->width, allocation->height, get_robot1(i, j));
      else if (robot2_exists(i, j))
        scale_clutter_object(i, j, allocation->width, allocation->height, get_robot2(i, j));
      if (explosion_exists(i, j))
        scale_clutter_object(i, j, allocation->width, allocation->height, get_explosion(i, j));
    }
  }
}

gboolean
clutter_move_cb (GtkWidget * widget, GdkEventMotion * e, gpointer data)
{
  int dx, dy;

  get_dir (e->x, e->y, &dx, &dy);

  set_clutter_cursor_by_direction (e->x, e->y, dx, dy);

  return TRUE;
}
*/
