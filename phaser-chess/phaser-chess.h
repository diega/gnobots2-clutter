#ifndef __PHASER_CHESS_H__
#define __PHASER_CHESS_H__

@interface phaser_chess_app : Gtk_App
{
  Board_View *bv;
  Logical_Board *lb;
}
@end

#define GRID_SZ 32

#endif /*__PHASER_CHESS_H__*/
