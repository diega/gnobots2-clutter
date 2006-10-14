#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "ggz.h"
#include "base64.h"

/* Base64 functions as per RFC 2045 */

/* The base64 code table */
static const char *b64table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

/* Lookup in the code table */
static int b64rev(int c)
{
	int i;

	for(i = 0; i < 64; i++)
		if(b64table[i] == c) return i;
	return 0;
}

/* Encodes to base64, allocating space as needed */
char *ggz_base64_encode(const char *text, int length)
{
	char *ret;
	unsigned char *tmp;
	int i, j;
	int matrix;

	if(!text) return NULL;

	ret = ggz_malloc(length * 2 + 1);

	/* Padding with NUL bytes */
	tmp = ggz_malloc(length + 4);
	tmp[length + 1] = 0;
	tmp[length + 2] = 0;
	tmp[length + 3] = 0;
	strncpy((char*)tmp, text, length);

	/* Bit conversion */
	j = 0;
	for(i = 0; i < length; i += 3)
	{
		matrix = ((tmp[i] << 16) + (tmp[i + 1] << 8) + (tmp[i + 2]));
		ret[j] = b64table[(matrix >> 18) & 0x3F];
		ret[j + 1] = b64table[(matrix >> 12) & 0x3F];
		ret[j + 2] = b64table[(matrix >> 6) & 0x3F];
		ret[j + 3] = b64table[(matrix) & 0x3F];
		j += 4;
	}
	/* Padding scheme: xxx->YYYY, x00->YY==, xx0->YYY= */
	if(length % 3)
	{
		for(i = j - 1; i != (j - 1 - (3 - length % 3)); i--)
			ret[i] = '=';
	}
	ret[j] = 0;

	ggz_free(tmp);

	return ret;
}

/* Decodes from base64, allocating space as needed */
char *ggz_base64_decode(const char *text, int length)
{
	char *ret;
	int i, matrix, j;

	if(!text) return NULL;
	ret = ggz_malloc(length + 1);

	/* Bit conversion */
	j = 0;
	for(i = 0; i < length; i += 4)
	{
		matrix = ((b64rev(text[i]) << 18)
			+ (b64rev(text[i + 1]) << 12)
			+ (b64rev(text[i + 2]) << 6)
			+ (b64rev(text[i + 3])));
		ret[j] = (matrix >> 16) & 0xFF;
		ret[j + 1] = (matrix >> 8) & 0xFF;
		ret[j + 2] = (matrix) & 0xFF;
		j += 3;
	}
	ret[j] = 0;

	return ret;
}

