/*
 * File: misc.h
 * Author: Brent Hendricks
 * Project: GGZ Common components library
 * Date: 2001-10-12
 *
 * Internal header file for ggz components lib
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

struct _GGZFile {
	int fdes;	/* File descriptor */
	char *buf;	/* Data buffer */
	char *p;	/* Current position in buffer */
	char *e;	/* Points one char past end of valid data in buffer */
	int bufsize;	/* Current buffer size */
};
