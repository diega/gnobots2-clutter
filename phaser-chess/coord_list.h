#ifndef COORD_LIST_H
#define COORD_LIST_H 1

#include "objc_inc.h"

@interface Coord_List : Object
{
    int *xs;
    int *ys;
    int max;
    int num;
}
- init_coord_list;
- (void) empty;
- (void) add : (int) x : (int) y;
- (void) get_last : (int *) x : (int *) y;
- (int) get_last_and_delete : (int *) x : (int *) y;
@end

#endif
