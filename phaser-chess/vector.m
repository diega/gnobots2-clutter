#include "vector.h"

@implementation Vector : Object

- init
{
  elements = NULL;
  num_elements = 0;
  max_elements = 0;

  return self;
}

- (int) size
{
  return num_elements;
}

- (unsigned char *) add : (unsigned char *) element
{
  [self grow : num_elements + 1];
  elements[ num_elements ++] = element;
  return element;
}

- (int) index_of : (unsigned char *) element
{
  int i;

  for (i=0; i<num_elements; i++)
    if (elements[ i ] == element)
      return i;

  return -1;
}

- (unsigned char *) set : (int) index : (unsigned char *) element
{
  unsigned char *was;
  [self grow : index + 1];
  was = elements[ index ];
  elements[ index ] = element;
  if (index >= num_elements)
    num_elements = index + 1;
  return was;
}

- (unsigned char *) unset : (int) index
{
  unsigned char *was = NULL;
  if (index <= num_elements) was = elements[ index ];
  elements[ index ] = NULL;
  return was;
}

- (unsigned char *) unset_element : (unsigned char *) element
{
  int i;

  for (i=0; i<num_elements; i++)
    if (elements[ i ] == element)
      elements[ i ] = NULL;

  return element;
}

- (unsigned char *) insert : (int) index : (unsigned char *) element
{
  int i;

  [self grow : num_elements + 1];

  for (i=num_elements; i>index; i--)
    elements[ i ] = elements[ i-1 ];

  elements[ index ] = element;

  return element;
}

- (unsigned char *) delete : (int) index
{
  int i;
  unsigned char *was = NULL;

  if (index >= num_elements)
    return was;

  was = elements[ index ];

  for (i=index; i<num_elements-1; i++)
    elements[ i ] = elements[ i+1 ];

  elements[ num_elements-- ] = NULL;

  return was;
}

- (void) delete_all
{
  int i;

  for (i=0; i<num_elements; i++)
    elements[ i ] = NULL;

  num_elements = 0;
}

- (unsigned char *) get : (int) index
{
  if (index >= num_elements)
    return NULL;
  return elements[ index ];
}

- (void) grow : (int) number_of_elements
{
  int i;
  unsigned char **new_elements;

  while (max_elements <= number_of_elements)
    {
      if (elements == NULL)
	new_elements = (unsigned char **) malloc (sizeof (char *) *
						  max_elements * 2 + 1);
      else
	new_elements = (unsigned char **) realloc (elements,
						   sizeof (char *) *
						   max_elements * 2 + 1);


      if (new_elements == NULL)
	{
	  perror ("malloc or realloc");
	  return;
	}

      elements = new_elements;

      for (i=max_elements; i < max_elements*2+1; i++)
	elements[ i ] = NULL;

      max_elements *= 2;
      max_elements ++;
    }
}

- (void) free_vector
{
  free (elements);
  [super free];
}

@end

