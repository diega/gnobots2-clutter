#ifndef __PHASER_CHESS_H__
#define __PHASER_CHESS_H__

#include <obgnome/obgnome.h>
@interface phaser_chess_app : Gnome_App
{
  Board_View *bv;
  Logical_Board *lb;
}
- initPhaserChessApp:(char *) app_id :(int) argc :(char **) argv;
@end

#define GRID_SZ 32

#endif /*__PHASER_CHESS_H__*/
