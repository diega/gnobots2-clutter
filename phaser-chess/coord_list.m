#include "coord_list.h"

@implementation Coord_List : Object

- init_coord_list
{
    [super init];

    xs = NULL;
    ys = NULL;
    max = 0;
    num = 0;

    return self;
}


- (void) empty
{
    num = 0;
}


- (void) add : (int) x : (int) y
{
    if (num == max)
    {
	xs = (int *) realloc (xs, sizeof (int) * (max+1) * 2);
	ys = (int *) realloc (ys, sizeof (int) * (max+1) * 2);
	max++;
	max *= 2;
    }

    xs[ num ] = x;
    ys[ num ] = y;

    num++;
}

- (void) get_last : (int *) x : (int *) y
{
    (*x) = xs[ num ];
    (*y) = ys[ num ];
}

- (int) get_last_and_delete : (int *) x : (int *) y
{
    if (num == 0)
	return 0;

    num --;

    (*x) = xs[ num ];
    (*y) = ys[ num ];

    return 1;
}



@end
