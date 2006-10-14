/********************************************************************** 
 Copyright (C) 1996-2006 the Freeciv Dev Team and the GGZ Dev Team
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   $Id$
***********************************************************************/
#ifndef __GGZ_DIO_H__
#define __GGZ_DIO_H__

#include <stdbool.h>

typedef struct GGZDataIO GGZDataIO;

/* Initializers. */
GGZDataIO *ggz_dio_new(int socket);
void ggz_dio_free(GGZDataIO * dio);

void ggz_dio_set_read_freeze(GGZDataIO * dio, bool frozen);
void ggz_dio_set_auto_flush(GGZDataIO * dio, bool autoflush);
void ggz_dio_set_writeable_callback(GGZDataIO * dio,
				    void writeable_callback(struct
							    GGZDataIO *,
							    bool));
void ggz_dio_set_read_callback(GGZDataIO * dio,
			       void read_callback(GGZDataIO *, void *),
			       void *read_callback_userdata);


/* Network r/w functions. */
int ggz_dio_read_data(GGZDataIO * dio);
int ggz_dio_write_data(GGZDataIO * dio);

int ggz_dio_flush(GGZDataIO * dio);

bool ggz_dio_is_write_pending(const GGZDataIO * dio);
int ggz_dio_get_socket(const GGZDataIO * dio);

void ggz_dio_packet_start(GGZDataIO * dio);
void ggz_dio_packet_end(GGZDataIO * dio);

/* gets */
void ggz_dio_get_char(GGZDataIO * dio, char *dest);
void ggz_dio_get_int(GGZDataIO * dio, int *dest);

void ggz_dio_get_uint8(GGZDataIO * dio, int *dest);
void ggz_dio_get_uint16(GGZDataIO * dio, int *dest);
void ggz_dio_get_uint32(GGZDataIO * dio, unsigned int *dest);

void ggz_dio_get_sint8(GGZDataIO * dio, int *dest);
void ggz_dio_get_sint16(GGZDataIO * dio, int *dest);
void ggz_dio_get_sint32(GGZDataIO * dio, signed int *dest);

void ggz_dio_get_bool8(GGZDataIO * dio, bool * dest);
void ggz_dio_get_bool32(GGZDataIO * dio, bool * dest);
void ggz_dio_get_memory(GGZDataIO * dio, void *dest, size_t dest_size);
void ggz_dio_get_string(GGZDataIO * dio, char *dest, size_t max_dest_size);
void ggz_dio_get_string_alloc(GGZDataIO * dio, char **dest);

/* puts */
void ggz_dio_put_char(GGZDataIO * dio, char dest);
void ggz_dio_put_int(GGZDataIO * dio, int dest);

void ggz_dio_put_uint8(GGZDataIO * dio, int value);
void ggz_dio_put_uint16(GGZDataIO * dio, int value);
void ggz_dio_put_uint32(GGZDataIO * dio, int value);

void ggz_dio_put_sint8(GGZDataIO * dio, int value);
void ggz_dio_put_sint16(GGZDataIO * dio, int value);
void ggz_dio_put_sint32(GGZDataIO * dio, int value);

void ggz_dio_put_bool8(GGZDataIO * dio, bool value);
void ggz_dio_put_bool32(GGZDataIO * dio, bool value);

void ggz_dio_put_memory(GGZDataIO * dio, const void *value, size_t size);
void ggz_dio_put_string(GGZDataIO * dio, const char *value);

#endif /* GGZ_DIO_H */
