#ifndef __VECTOR_H__
#define __VECTOR_H__

#include "objc_inc.h"
#include "config.h"

@interface Vector : Object
{
  unsigned char **elements;
  int num_elements;
  int max_elements;
}

- init;
- (int) size;
- (int) index_of : (unsigned char *) element;
- (unsigned char *) add : (unsigned char *) element;
- (unsigned char *) set : (int) index : (unsigned char *) element;
- (unsigned char *) unset : (int) index;
- (unsigned char *) unset_element : (unsigned char *) element;
- (unsigned char *) insert : (int) index : (unsigned char *) element;
- (unsigned char *) delete : (int) index;
- (void) delete_all;
- (unsigned char *) get : (int) index;
- (void) grow : (int) number_of_elements;
- (void) free_vector;

@end

#endif /* __VECTOR_H__ */
