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
 
/***************************************************************************
 *            cache.c
 *
 *  Sun Apr  8 19:45:36 2007
 *  Copyright  2007  Joseph Emmanuel DL Day
 *  An implementation of a generic block device cache
 *  joseph.dayo@gmail.com
 ****************************************************************************/
 
#include "cache.h"
 
device_cache_info *cache_head_ptr=0; 
 
void cache_initialize(device_cache_info *devinfo) {
  int i;
  devinfo->cache = (cache_entry*)malloc(devinfo->num_entries*sizeof(cache_entry));
  //initialize the cache
  for (i=0;i<devinfo->num_entries;i++)
   {
     devinfo->cache[i].valid=0;
     devinfo->cache[i].accessed=0;
     devinfo->cache[i].dirty=0;
	 devinfo->cache[i].buf=(char*)malloc(devinfo->size_per_entry);
   };
};

void cache_invalidatecache(device_cache_info *devinfo)
 {
  int i;
  for (i=0;i<devinfo->num_entries;i++)
   {
     devinfo->cache[i].valid=0;
   };
};

void cache_free(device_cache_info *devinfo,DWORD sectornumber)
 {
  int index;

   if (devinfo->cache[sectornumber].valid)
          {
            devinfo->cache[sectornumber].valid=0;
          };
 };

int cache_shouldflush(device_cache_info *devinfo)
 {
  int res=0,index;
    for (index=0;index<devinfo->num_entries;index++)
    {
       if ( devinfo->cache[index].valid && devinfo->cache[index].dirty)
            {
             res=1;break;
            };
    }
    return res;
 };
 
 int cache_flush(device_cache_info *devinfo)
  {
    int res=0,index,block;
    start_priority();

    for (index=0;index<devinfo->num_entries;index++)
    {
       if (devinfo->cache[index].valid&&devinfo->cache[index].dirty)
            {
   	          int retry=0,i=0;
	      #ifdef DEBUG_FLUSHMGR
  		  printf("dex32_flushmgr: commiting write..\n");
	      #endif
			  devinfo->block_desc.write_block(index,devinfo->cache[index].buf,1); 
              if (res==0) res= -1; 
              devinfo->cache[index].dirty=0;

	      #ifdef DEBUG_FLUSHMGR
		  printf("dex32_flushmgr: commit write done..\n");
	      #endif

            };
    };
    stop_priority();
    return res;
  };
 
 int cache_getcand(device_cache_info *devinfo)
 {
   DWORD min=0xFFFFFFFF;
   int index,c=0;
   //obtain the oldest cache slot
   for (index=0;index<devinfo->num_entries;index++)
     {
       if (devinfo->cache[index].accessed<min&&!devinfo->cache[index].dirty)
         {
           min=devinfo->cache[index].accessed;
           c=index;
         };

     };
   return c;
 };
 
 int cache_store(device_cache_info *devinfo,char *buf,DWORD sectornumber,int dirty)
 {
   int index;
   int freeslot = -1;
   int computedslot = sectornumber%devinfo->num_entries;
   
    if (memcmp(devinfo->cache[computedslot].buf,buf,devinfo->size_per_entry)!=0)
        {
              memcpy(devinfo->cache[computedslot].buf,buf,devinfo->size_per_entry);
			  devinfo->cache[computedslot].block = sectornumber;
              devinfo->cache[computedslot].valid=1;
              devinfo->cache[computedslot].accessed=0;
              devinfo->cache[computedslot].dirty=dirty;
        };
    return 1;
 };

int cache_storecache(device_cache_info *devinfo,char *buf,DWORD sectornumber, DWORD numblocks)
 {
  int index=0;
  int i, res = 0,ofs = 0;
  for (i=0;i<numblocks;i++)
  {  
    cache_store(devinfo,buf+ofs,sectornumber+i,0);
    ofs+=devinfo->block_desc.get_block_size();
  };
  return 1;
 };  
 
 
int cache_getcache(device_cache_info *devinfo,char *buf,DWORD sectornumber, DWORD numblocks)
 {
  int index=0;
  int i, res = 0,ofs = 0;
  
	 
  for (i=0;i<numblocks;i++)
  {  
	int computedslot = (sectornumber+i)%devinfo->num_entries;
    res = 0;
    if (devinfo->cache[computedslot].valid)
          {
			if (devinfo->cache[computedslot].block == sectornumber+i) {  
            	memcpy(buf + ofs,devinfo->cache[computedslot].buf,devinfo->size_per_entry);
            	devinfo->cache[computedslot].accessed = time_count;
            	res = 1; 
			}
          };
    ofs+=devinfo->block_desc.get_block_size();
    if (res == 0) return 0;
  };
  return 1;
 }; 

 //stub interfaces for cached devices
 int cache_interface_invalidate_cache() {
	  device_cache_info *cacheinfo = cache_info_fetch();
	  cache_invalidatecache(cacheinfo);
 }
 
 
 int cache_interface_read_block(int block,char *blockbuff, DWORD numblocks) {
	  device_cache_info *cacheinfo =cache_info_fetch();
	  devmgr_block_desc *blockdev = &cacheinfo->block_desc;
	  int ret = bridges_link(blockdev,&blockdev->read_block,block,blockbuff,numblocks,0,0,0);
	  cache_storecache(cacheinfo,blockbuff,block,numblocks);
	  return ret;
 }	 
 
 int cache_interface_write_block(int block,char *blockbuff,DWORD numblocks) {
	  device_cache_info *cacheinfo =cache_info_fetch();
	  devmgr_block_desc *blockdev = &cacheinfo->block_desc;
	  int ret = bridges_link(blockdev,&blockdev->write_block,block,blockbuff,numblocks,0,0,0);
	  cache_storecache(cacheinfo,blockbuff,block,numblocks);
 }
 
 int cache_interface_flush_device() {
	  device_cache_info *cacheinfo =cache_info_fetch();
	  cache_flush(cacheinfo);
 }
 
 int cache_interface_getcache(char *buf,DWORD sectornumber,DWORD numblocks) {
	 device_cache_info *cacheinfo =cache_info_fetch();
	 return cache_getcache(cacheinfo,buf,sectornumber,numblocks);
 }
 
 /*
 Generate a device interface that provides caching for the target block device.
 */
 int cache_generate_interface(devmgr_block_desc *blockdev,int cachesize) {
	 devmgr_block_desc blockdesc;
	 
	 //generate a device cache based on the block device information
	 device_cache_info *cacheinfo = (device_cache_info*)malloc(sizeof(device_cache_info));
	 cacheinfo->size_per_entry = bridges_link(blockdev,&blockdev->get_block_size,0,0,0,0,0,0);
	 cacheinfo->num_entries = cachesize / cacheinfo->size_per_entry;
	 cacheinfo->devid = blockdev->hdr.id;
	 //Return with error if the cache size is going to be too small.
	 if (cacheinfo->num_entries < 5) return - 1;
	 cache_initialize(cacheinfo);
	 
	 //Setup the generated cached device and register it in the device manager
	 blockdesc.hdr.size = sizeof(devmgr_block_desc);
	 blockdesc.hdr.type = DEVMGR_BLOCK;
	 char name[500];
	 strcpy(name,blockdev->hdr.name);
	 strcat(name,"[Cached]");
	 strcpy(blockdesc.hdr.name,name);
	 strcpy(blockdesc.hdr.description,blockdev->hdr.description);
	 blockdesc.read_block = cache_interface_read_block;
	 blockdesc.write_block = cache_interface_write_block;
	 blockdesc.invalidate_cache = cache_interface_invalidate_cache;
	 blockdesc.init_device = blockdev->init_device;
	 blockdesc.flush_device = cache_interface_flush_device;
	 blockdesc.getcache = cache_interface_getcache;
	 blockdesc.total_blocks = blockdev->total_blocks;
	 blockdesc.get_block_size = blockdev->get_block_size;
	 
	 memcpy(blockdev,&cacheinfo->block_desc,sizeof(devmgr_block_desc));
	 cacheinfo->cacheinterfaceid = devmgr_register((devmgr_generic*)&blockdesc);
	 //Add this device to the list of cache managed by the cache manager
	 cache_addnode(cacheinfo); 
 }
 
 int cache_addnode(device_cache_info *cacheinfo)  {
	 if (cache_head_ptr==0) {
		 cache_head_ptr=cacheinfo;
		 cacheinfo->next=0;
	 } else {
		 device_cache_info *ptr = cache_head_ptr;
		 while (ptr->next!=0) ptr=ptr->next;
		 ptr->next = cacheinfo;
		 cacheinfo->next=0;
	 }
 }
 
 device_cache_info *cache_info_fetch() {
	device_cache_info *ptr = cache_head_ptr;
	 int deviceid = devmgr_getcontext();
	 while (ptr!=0) {
		if (ptr->cacheinterfaceid) 
			return ptr;
		else
			ptr=ptr->next;
	 }		
 }
 
 int cache_sendmessage(int type,const char *message) {
	     if (type == DEVMGR_MESSAGESTR)
       		{
			  int l = strlen(message);
              char *temp = malloc(l+1), *p;
              
              //make a temporary copy sinced we cannot modify message
              strcpy(temp,message);
			             //get the second parameter
              p = strtok(temp," ");
              p = strtok(0," ");
              if (p!=0)
                   {
					     //Generate cache around device id
                         if  (strcmp(p,"-gencache")==0) {
							 char *devname = strtok(0," ");
							 if (devname!=0) {
							    char *cachesize = strtok(0," ");
								devmgr_generic *gendev = (devmgr_generic*)devmgr_getdevicebyname(devname);
								if (gendev->type==DEVMGR_BLOCK) {
									devmgr_block_desc *myblock = (devmgr_block_desc*)gendev;
									cache_generate_interface(myblock,100000);
								}
							 }
							 
						 }
				   }
			}
 }
 
 //Starts the cache provider, this will register itself as a generic device
 void cache_start_provider() {
	 devmgr_generic cache_info;
	 cache_info.size = sizeof(devmgr_generic);
	 cache_info.type = DEVMGR_DEV;
	 strcpy(cache_info.name,"syscache");
	 strcpy(cache_info.description,"cache for block devices");
	 cache_info.sendmessage = cache_sendmessage;
	 
	 //register devices to the device manager
	 devmgr_register((devmgr_generic*) &cache_info);
 }
