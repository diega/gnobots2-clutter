#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "ggz.h"
#include "base16.h"

/* Function to encode strings to base16 (hex) with a nibble per hex character */

char *ggz_base16_encode(const char *text, int length)
{
	char *ret = NULL;
	int i;
	static const char hextable[20] = "0123456789abcdef";

	if(!text) return NULL;

	ret = ggz_malloc((length * 2 + 1)*sizeof(*ret));
	if(!ret) return NULL;

	for(i = 0; i < length; i++){
		ret[i*2] = hextable[(text[i]&0xf0)>>4];
		ret[i*2+1] = hextable[text[i]&0xf];
	}
	return ret;
}

