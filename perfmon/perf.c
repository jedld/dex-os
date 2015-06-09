/*
  Name: perf.c
  Copyright: 
  Author: Joseph Emmanuel DL Dayo
  Date: 07/02/04 07:36
  Description: The module that handles performance monitoring
  
    DEX educational extensible operating system 1.0 Beta
    Copyright (C) 2007  Joseph Emmanuel DL Dayo

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
#include "../dextypes.h"
#include "perf.h"

perf_statsentry perf_entitytable[PERF_MAX_ENTRIES];   //The statistical entry table	
int perf_total_entries=0;

/*
Create a new performance entity, a performance entity is a
module or function that has its performance metrics stored measured.
*/
int perf_new_entity(char *entityname) {
	int i;
	//scan entity table for duplicates, if a duplicate is found, just return its index
	for (i=0; i < perf_total_entries && i < PERF_MAX_ENTRIES; i++) {
		if (strcmp(perf_entitytable[i].entry_name,entityname)==0) {
		return i;
		}			
	}
	int cur_index = perf_total_entries;
	
	strcpy(perf_entitytable[cur_index].entry_name,entityname);
	perf_entitytable[cur_index].low_water_mark = 0;
	perf_entitytable[cur_index].high_water_mark = 0;
	perf_entitytable[cur_index].avg_water_mark = 0;
	perf_entitytable[cur_index].cur_water_mark = 0;
	perf_total_entries++;
	return cur_index;
}

/*
update a performance entity benchmark
*/
void perf_record(int handle,DWORD elapsed) {
	DWORD weight = perf_entitytable[handle].hits;
	if (elapsed < perf_entitytable[handle].low_water_mark) 
		perf_entitytable[handle].low_water_mark = elapsed;
		
	if (elapsed > perf_entitytable[handle].high_water_mark)
		perf_entitytable[handle].high_water_mark = elapsed;
	
	perf_entitytable[handle].cur_water_mark = elapsed;
	perf_entitytable[handle].avg_water_mark = ((perf_entitytable[handle].avg_water_mark * weight) + elapsed)/(weight+1);
	perf_entitytable[handle].hits++;
}

/*
	Resets all performance counters to zero
*/
void perf_resetcounters() {
	int i;
	for (i=0; i < perf_total_entries && i < PERF_MAX_ENTRIES; i++) {
			perf_entitytable[i].low_water_mark = 0;
			perf_entitytable[i].high_water_mark = 0;
			perf_entitytable[i].avg_water_mark = 0;
			perf_entitytable[i].hits = 0;
			perf_entitytable[i].cur_water_mark = 0;
		}

}


void perf_printstats() {
	int i;
	printf("entity			low 	avg 	high 	now\n");
	for (i=0; i < perf_total_entries && i < PERF_MAX_ENTRIES; i++) {
			printf("%16s %8u %8u %8u %8u\n",
				perf_entitytable[i].entry_name,
				perf_entitytable[i].low_water_mark,
				perf_entitytable[i].avg_water_mark,
				perf_entitytable[i].high_water_mark,
				perf_entitytable[i].cur_water_mark);
		}			
}
