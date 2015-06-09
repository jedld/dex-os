/*
  Name: perf.h
  Copyright: 
  Author: Joseph Emmanuel DL Dayo
  Date: 07/02/04 07:36
  Description: The module that handles the ISO9660 filesystem on most CD-ROMS
  
    DEX educational extensible operating system 1.0 Beta
    Copyright (C) 2004  Joseph Emmanuel DL Dayo

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
*/
#ifndef PERF_H
#define PERF_H
#include "../dextypes.h"

#define PERF_MAX_ENTRIES 250
typedef struct _perf_statsentry {
	unsigned char entry_name[30]; //The name of the entity to be monitored
	DWORD low_water_mark;	      //The low water mark
	DWORD avg_water_mark;	      //The average water mark	
	DWORD high_water_mark;	      //The high water mark	
	DWORD cur_water_mark;	      //The high water mark		
	DWORD hits;		      //The number of hits to this entity
} perf_statsentry;



int perf_new_entity(char *entityname);
void perf_record(int handle,DWORD elapsed);
void perf_resetcounters();
void perf_printstats();

#endif
