#ifndef GGZ_BASE64_H
#define GGZ_BASE64_H

/* Encodes to base64, allocating space as needed */
char *ggz_base64_encode(const char *text, int length);

/* Decodes from base64, allocating space as needed */
char *ggz_base64_decode(const char *text, int length);

#endif

