/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _CACHE_H
#define _CACHE_H
#include "../devmgr/dex32_devmgr.h"
#include "../dextypes.h"


typedef struct _cache_entry {
	DWORD block;
    DWORD accessed;
    WORD  valid;
    WORD  dirty;  //Used by the driver to determine if a modified block
                 //has already been written to the disk
    char *buf;
} cache_entry;
	
	
typedef struct _device_cache_info {
	int devid;			  		  //The device id
	int cacheinterfaceid;		  //The device id assigned by the cache manager
	DWORD num_entries;			  //Number of entries
    DWORD size_per_entry; 		  //The size of each cell
	devmgr_block_desc block_desc; //For callback purposes
    cache_entry *cache;
	struct _device_cache_info *next;
} device_cache_info;

void cache_initialize(device_cache_info *devinfo);
void cache_invalidatecache(device_cache_info *devinfo);
device_cache_info *cache_info_fetch();
void cache_free(device_cache_info *devinfo,DWORD sectornumber);
int cache_shouldflush(device_cache_info *devinfo);
int cache_flush(device_cache_info *devinfo);
int cache_getcand(device_cache_info *devinfo);
int cache_store(device_cache_info *devinfo,char *buf,DWORD sectornumber,int dirty);
int cache_getcache(device_cache_info *devinfo,char *buf,DWORD sectornumber, DWORD numblocks);


#endif /* _CACHE_H */
