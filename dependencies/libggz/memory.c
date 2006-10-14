/*
 * File: memory.c
 * Author: Rich Gade
 * Project: GGZ Core Client Lib
 * Date: 02/15/01
 * $Id$
 *
 * This is the code for handling memory allocation for ggzcore
 *
 * Copyright (C) 2001-2002 Brent Hendricks.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#ifndef NO_THREADING
#include <pthread.h>
#endif

#include "ggz.h"

#include "misc.h" /* Internal data/functions */
#include "support.h"


/* This is used basically as a LIFO stack, on the basis that most malloc/free
 * pairs are somewhat close together, so keep recent mallocs near the top */
typedef struct _memptr {
	struct _memptr	*next;
	const char	*tag;
	int		line;
	void		*ptr;
	unsigned int	size;
} _memptr;

static struct _memptr	*alloc = NULL;

enum lock_status {
  NEED_LOCK, HAVE_LOCK
};

/* Some platforms don't have pthread or good threading.  In this case we
 * assume the program won't be threaded.  This must be specifically
 * enabled in configure. */
#ifndef NO_THREADING
static pthread_mutex_t	mut = PTHREAD_MUTEX_INITIALIZER;
# define LOCK() pthread_mutex_lock(&mut)
# define UNLOCK() pthread_mutex_unlock(&mut)
#else
# define LOCK() (void)0
# define UNLOCK() (void) 0
#endif

static void * _ggz_allocate(const unsigned int size,
                            const char *tag, int line,
			    enum lock_status lock)
{
	struct _memptr *newmem;

	/* Try to allocate our memory */
	newmem = malloc(sizeof(_memptr) + size); /* should be only "malloc" call. */
	if(newmem == NULL)
		ggz_error_sys_exit("Memory allocation failure: %s/%d", tag, 
				    line);

	/* We've got the memory, so set it up */
	newmem->tag = tag;
	newmem->line = line;
	newmem->ptr = newmem + 1;	/* Add one sizeof(_memptr) */
	newmem->size = size;

	/* Put this at the head of the list */
	if (lock == NEED_LOCK) {
		LOCK();
	}
	newmem->next = alloc;
	alloc = newmem;
	if (lock == NEED_LOCK) {
		UNLOCK();
	}

	ggz_debug(GGZ_MEM_DEBUG, "%d bytes allocated at %p from %s/%d",
		  size, newmem->ptr, tag, line);

	return newmem->ptr;
}


void * _ggz_malloc(const size_t size, const char *tag, int line)
{
	void *new;

	/* Sanity checks */
	if(!tag)
		tag = "<unknown>";
	if(!size) {
		ggz_error_msg("ggz_malloc: 0 byte malloc requested from %s/%d",
			      tag, line);
		return NULL;
	}

	/* Get a chunk of memory */
	new = _ggz_allocate(size, tag, line, NEED_LOCK);

	/* Clear the memory to zilcho */
	memset(new, 0, size);

	return new;
}


void * _ggz_realloc(const void *ptr, const size_t size,
                    const char *tag, int line)
{
	struct _memptr *targetmem;
	void *new;

	/* Sanity checks */
	if(!tag)
		tag = "<unknown>";
	if(!size) {
		_ggz_free(ptr, tag, line);
		return NULL;
	}

	/* If ptr is NULL, treat this like a call to malloc */
	if (ptr == NULL)
		return _ggz_malloc(size, tag, line);

	/* Search through allocated memory for this chunk */
	LOCK();
	targetmem = alloc;
	while(targetmem != NULL && ptr != targetmem->ptr) {
		targetmem = targetmem->next;
	}

	/* This memory was never allocated via ggz */
	if(targetmem == NULL) {
		UNLOCK();
		ggz_error_msg("Memory reallocation <%p> failure: %s/%d",
			       ptr, tag, line);
		return NULL;
	}

	/* Try to allocate our memory */
	new = _ggz_allocate(size, tag, line, HAVE_LOCK);

	/* Copy the old to the new */
	if(size > targetmem->size) {
		memcpy(new, targetmem->ptr, targetmem->size);
		/* And zero out the rest of the block */
		memset((char*)new + targetmem->size, 0,
		       size-targetmem->size);
	} else
		memcpy(new, targetmem->ptr, size);
	UNLOCK();

	ggz_debug(GGZ_MEM_DEBUG,
		  "Reallocated %d bytes at %p to %d bytes from %s/%d",
		  targetmem->size, targetmem->ptr, size, tag, line);

	/* And free the old chunk */
	_ggz_free(targetmem->ptr, tag, line);

	return new;
}


int _ggz_free(const void *ptr, const char *tag, int line)
{
	struct _memptr *prev, *targetmem;
	unsigned int oldsize;

	/* Sanity checks */
	if(!tag)
		tag = "<unknown>";

	/* Search through allocated memory for this chunk */
	prev = NULL;
	LOCK();
	targetmem = alloc;
	while(targetmem != NULL && ptr != targetmem->ptr) {
		prev = targetmem;
		targetmem = targetmem->next;
	}

	/* This memory was never allocated via ggz */
	if(targetmem == NULL) {
		UNLOCK();
		ggz_error_msg("Memory deallocation <%p> failure: %s/%d",
			       ptr, tag, line);
		return -1;
	}

	/* Juggle pointers so we can remove this item */
	if(prev == NULL)
		alloc = targetmem->next;
	else
		prev->next = targetmem->next;
	oldsize = targetmem->size;
	UNLOCK();

	ggz_debug(GGZ_MEM_DEBUG, "%d bytes deallocated at %p from %s/%d",
		  oldsize, ptr, tag, line);

	free(targetmem); /* should be the only "free" call */

	return 0;
}


int ggz_memory_check(void)
{
	int flag = 0;
	struct _memptr *memptr;

	ggz_log(GGZ_MEM_DEBUG, "*** Memory Leak Check ***");

	LOCK();
	if(alloc != NULL) {
		memptr = alloc;
		while(memptr != NULL) {
			ggz_log(GGZ_MEM_DEBUG,
				"%d bytes left allocated at %p by %s/%d",
				memptr->size, memptr->ptr,
				memptr->tag, memptr->line);
			memptr = memptr->next;
		}
		
		flag = -1;
	} else {
		ggz_log(GGZ_MEM_DEBUG, "All clean!");
	}
	UNLOCK();

	ggz_log(GGZ_MEM_DEBUG, "*** End Memory Leak Check ***");

	return flag;
}


char * _ggz_strdup(const char *src, const char *tag, int line)
{
	unsigned len;
	char *new;

	/* If you ask for a copy of NULL, you get NULL back */
	if (src == NULL)
		return NULL;

	/* Sanity check */
	if(!tag)
		tag = "<unknown>";


	len = strlen(src);

	ggz_debug(GGZ_MEM_DEBUG,
		  "Allocating memory for length %d string from %s/%d",
		  len+1, tag, line);

	new = _ggz_allocate(len+1, tag, line, NEED_LOCK);

	memcpy(new, src, len+1);

	return new;
}
