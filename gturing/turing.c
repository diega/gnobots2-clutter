/* turing.c - a Turing machine simulator.
 * Copyright (C) 1998 The Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "turing.h"

#define BLANK           '_'
#define END_TAG         '\n'
#define COMMENT_TAG     '#'
#define TOKEN_SEPARATOR " "

#define LEFT  'l'
#define RIGHT 'r'

/* An utility for fread_states: exhausts any spaces found in fd. */
static void read_spaces(FILE *fd)
{
	int buff;
	
	while (((buff = fgetc(fd)) != EOF) && isspace(buff))
		;
	
	ungetc(buff, fd);
}

static tape *new_cell(void)
{
	tape *temp;
	
	temp = malloc(sizeof(tape));
	temp->value = BLANK;
	temp->next = NULL;
	temp->prev = NULL;
	
	return temp;
}

static state *new_state(void)
{
	state *temp;
	
	temp = malloc(sizeof(state));
	
	temp->no = temp->read = temp->write = temp->move = temp->new = 0;
	temp->next = NULL;
	
	return temp;
}

turing *new_turing(void)
{
	turing *machine;
	
	machine = malloc(sizeof(turing));
	machine->state = 0;
	machine->pos = 0;
	machine->actualtape = machine->tapehead = NULL;
	machine->actualstate = machine->statehead = NULL;

	return machine;
}
	
static void turing_free_tape(turing *machine)
{
	tape *temp, *next;
	
	for (temp = machine->tapehead; temp; temp = next)
		{
			next = temp->next;
			free (temp);
		}
	
	machine->tapehead = NULL;
}

/* Gets the tape from tape_string. */
void turing_set_tape(turing *machine, char *ptr)
{
	tape *temp, *last;
	char buff;
	
	turing_free_tape(machine);
	
	last = machine->tapehead = NULL;
	
	while ((buff = *(ptr++)) != 0)
		{
			temp = new_cell();
			temp->value = (buff == ' ')? BLANK : buff;
			
			if (last)
				{
					last->next = temp;
					temp->prev = last;
					last = temp;
				}
			else
				last = machine->tapehead = temp;
		}
	 machine->actualtape = machine->tapehead;
}

char *turing_get_tape(turing *machine)
{
	char *buff, *tmpbuff;
	tape *temp;
	int i;
	
	i = 1;
	for (temp = machine->tapehead; temp; temp = temp->next)
		i++;
	
	tmpbuff = buff = malloc(sizeof(char) * i);
	
	for (temp = machine->tapehead; temp; temp = temp->next, tmpbuff ++)
		*tmpbuff = (temp->value == BLANK)? ' ' : temp->value;
	
	*tmpbuff = 0;
	
	return buff;
}

/* Reads states from file. Supports #comments, empty lines and misc info
 * after the state declaration in the same line. fscanf sucks: discovered the
 * blessings of strtok (thanks to Federico Mena). */
int turing_fread_states(turing *machine, char *filename)
{
	FILE *fd;
	state *temp;
	int count;
	char *token;
	char  line[1000];
	
	machine->statehead = temp = NULL;
	
	if ((fd = fopen(filename, "r")) == NULL)
		return -1;
		
	do
		{
			read_spaces(fd);
			
			fgets(line, 1000, fd);
			if ((*line == COMMENT_TAG) || (*line == END_TAG))
				continue;
			
			token = strtok(line, TOKEN_SEPARATOR);

			count = 0;

			if (temp == NULL)
				temp = new_state();
			
			while (token) {
				switch (count) {
					case 0:
						temp->no = atoi(token);
						break;

					case 1:
						temp->read = *token;
						break;

					case 2:
						temp->write = *token;
						break;

					case 3:
						temp->move = *token;
						break;

					case 4:
						temp->new = atoi(token);
						break;

					default:
						fprintf(stderr, "turing_fread_states(): wrong states file format.\n");
						exit(1);
						break;
				} /* switch */

				token = strtok(NULL, TOKEN_SEPARATOR);

				if (count == 4)
					break;
				
				count++;
			} /* while */
			 
			if (count == 4)
				{
					temp->next = machine->statehead;
					machine->statehead = temp;
					temp = NULL;
				}
		}
	while (temp == NULL);
	
	if (temp != NULL)
		free (temp);
	
	fclose(fd);
	 
	if (machine->statehead)
		{
			for (temp = machine->statehead; temp->next; temp = temp->next)
				;
			machine->state = temp->no;
			machine->actualstate = temp;
		}
	
	return 0;
}

/* Search the next state to execute depending on what we are reading and the
 * state field of the turing *machine. */
static state *turing_search_state(turing *machine)
{
	state *temp;
	
	for (temp = machine->actualstate; temp; temp = temp->next)
		if (temp->no == machine->state)
			return temp;
	
	return NULL;
}

/* Moves the tape. If we try to get out of the range of the list, we create a
 * new cell with a blank in it (giving the impression that it is infinite. */
static void turing_move_tape(turing *machine)
{
	if (tolower(machine->actualstate->move == LEFT))
		(machine->pos > 0)? machine->pos-- : 0;
	else
		machine->pos++;
			
	if (tolower(machine->actualstate->move == LEFT))
		{
			if (machine->actualtape->prev == NULL)
				{
					machine->pos = 0;
					machine->actualtape->prev = new_cell();
					machine->actualtape->prev->next = machine->actualtape;
					machine->tapehead = machine->actualtape->prev;
				}
			machine->actualtape = machine->actualtape->prev;
		}
	else
		{
			if (machine->actualtape->next == NULL)
				{
					machine->actualtape->next = new_cell();
					machine->actualtape->next->prev = machine->actualtape;
				}
			machine->actualtape = machine->actualtape->next;
		}
}

/* Runs the next state to execute. */
int turing_run_state(turing *machine)
{
	machine->actualstate = machine->statehead;
	
	while (machine->actualstate && 
				 ((machine->actualstate->no != machine->state) || 
					(machine->actualstate->read != machine->actualtape->value)))
		{
			machine->actualstate = machine->actualstate->next;
			machine->actualstate = turing_search_state(machine);
		}
	
	if (machine->actualstate)
		{
			machine->actualtape->value = machine->actualstate->write;
			turing_move_tape(machine);
			machine->state = machine->actualstate->new;
			return 1;
		}
	
	return 0;
}
