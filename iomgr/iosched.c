//**************************************************************************
//DEX32 Disk drive scheduler
//April 3, 2003 by Joseph Emmanuel Dayo
//This currently implements a very simple first-come-first-served IO queue
//**************************************************************************

#include "../devmgr/dex32_devmgr.h"
#include "iosched.h"
#include "../stdlib/time.h"
#include "../perfmon/perf.h"

#define USE_BENCHMARK
//#define DEBUG_READ
//#define DEBUG_IOREADWRITE2
//#define DEBUG_IOREADWRITE
//The registers that the io scheduler uses
int IOmgr_pause=0;
int IOrequest_time = 0;
int flush_time=10;  /* Time in seconds before writes are flushed to the disk. Only
                       for devices that have caches*/
int flush_counter=0;
int forceflush=0;
IOrequest *IOjob;
IOrequest *cur;
int io_flushid = 0;
int flushing=0,flushok=1;

//performance monitor handles
int perf_monwrite=0,perf_monread=0;

sync_sharedvar IOrequest_busy;

//initializes all the data structures needed in this module
DWORD iomgr_init()
 {
   devmgr_iomgr iomgr;
   int i;
   IOjob=0;
   cur=0;
   memset(&IOrequest_busy,0,sizeof(sync_sharedvar));

   strcpy(iomgr.hdr.name,"default_iomgr");
   strcpy(iomgr.hdr.description,"DEX default I/O scheduler and manager");
   strcpy(IOrequest_busy.owner,iomgr.hdr.name);
   iomgr.hdr.type = DEVMGR_IOMGR;
   iomgr.hdr.size = sizeof(devmgr_iomgr);
   iomgr.init = iomgr_init;
   iomgr.complete = dex32_IOcomplete;
   iomgr.close = dex32_closeIO;
   iomgr.request = dex32_requestIO;
   devmgr_register((devmgr_generic*)&iomgr);
	 
	//Setup performance monitoring
	perf_monwrite = perf_new_entity("iomgr_writes");
	perf_monread = perf_new_entity("iomgr_reads");	 

 };

DWORD iomgr_flushmgr()
 {
 int ret;


 ret = devmgr_flushblocks();
 if (ret==-1)
    printf("Error writing to device %d. Data might be lost.\n",ret);

 };

//this defines the threaded function which performs the actual
//reads and writes to the block device
DWORD iomgr_diskmgr()
{
   IOrequest *ptr;
   char *vidmem = (char*) 0xB8000;
   DWORD lastjob=0;
   devmgr_block_desc *myblock;
   flush_counter=time();
   do
    {
      ptr=IOmgr_obtainjob(0,0,lastjob);
      
      while (IOmgr_pause) {
		            #ifdef DEBUG_IOREADWRITE
                    char *scrptr = (char*)0xB8000;
	  				if (*scrptr=='*') *scrptr='+';
						 else
				    if (*scrptr=='+') *scrptr='=';
                         else						
					if (*scrptr=='=') *scrptr='?';
						else
					*scrptr='*';	
                    #endif
	  };
      
      if (ptr==0){
		//Commit writes to the disk if a specified time interval has
		//been met.
		        if ( (flush_counter - time() > flush_time) ||
                   (time() - flush_counter > flush_time ) || forceflush)
                    {
                     forceflush=0;

                     if (shouldflush()) iomgr_flushmgr();
                     flush_counter=time();
                    };
         
         continue;};


      if (ptr!=0)
      {
		  #ifdef DEBUG_IOREADWRITE
		  printf("dex32_diskmgr: Job obtained. Processing...\n");
		  #endif
         sync_entercrit(&IOrequest_busy);   
         /*Turn of task switching to improve performance*/
         //disable_taskswitching();
         
         do {
            //read or write data to the disk
            //   sigwait=current_process->processid;
            if (ptr->type==IO_READ)
              {
				  
				 #ifdef USE_BENCHMARK
				 int pstart = time();
				 #endif
				  
                 #ifdef DEBUG_READ
                 printf("dex32_diskmgr: Reading block %d..\n",ptr->lowblock);
                 #endif		
                 myblock = (devmgr_block_desc*)devmgr_devlist[ptr->deviceid];
                 
                 devmgr_setcontext(ptr->deviceid);
                 
                 if (myblock->hdr.type!=DEVMGR_BLOCK)
                 {
                 printf("IO ERROR: Device %d is not a block device!\n",ptr->deviceid);
                 }
                       else
                 { 
                 #ifdef DEBUG_READ         
                 printf("reading %d blocks starting at %d to location 0x%x.\n",
                           ptr->num_of_blocks, ptr->lowblock,ptr->buf);
                 #endif

                 if (myblock->read_block(ptr->lowblock,ptr->buf,ptr->num_of_blocks))
                 {
                  lastjob=ptr->lowblock;
                  ptr->status=IO_COMPLETE;
                 } 
                       else
                 {
                   #ifdef DEBUG_READ
                   printf("io_mgr(): read error?\n");
                   #endif
                   ptr->status=IO_ERROR;
                 };

                #ifdef DEBUG_READ          
          	    printf("dex32_diskmgr: read block Done..\n");
          	    #endif
          	    
	          };	    
              	 #ifdef USE_BENCHMARK
				 int pstop = time();
			     perf_record(perf_monwrite,pstop-pstart);
				 #endif             
	        }
      	        else
            if (ptr->type==IO_WRITE)
            {
            	    #ifdef DEBUG_IOREADWRITE
            	    printf("dex32_diskmgr: Writing block %d to device %d\n",ptr->lowblock,ptr->deviceid);
            	    #endif
	             	#ifdef USE_BENCHMARK
				 	int pstart = time();
				 	#endif    
	                //obtain the device descriptor       
	                myblock = (devmgr_block_desc*)devmgr_devlist[ptr->deviceid];
	                
                    devmgr_setcontext(ptr->deviceid);
                    
                    if (myblock->hdr.type!=DEVMGR_BLOCK)
                    {
                        printf("IO ERROR: Device %d is not a block device!\n",ptr->deviceid);
                    }
                            else        
                    if (myblock->write_block==0)
                    {
                         printf("IO ERROR: Device %d does not support writes!\n",ptr->deviceid);
                         ptr->status=IO_ERROR;
                    }
                            else         
                    if (myblock->write_block(ptr->lowblock,ptr->buf,ptr->num_of_blocks))
                    {
                         lastjob=ptr->lowblock;
                         ptr->status=IO_COMPLETE;
                    } 
                            else
                          ptr->status=IO_ERROR;
                    #ifdef DEBUG_IOREADWRITE
                    printf("dex32_diskmgr: write block Done..\n");
                    #endif
					#ifdef USE_BENCHMARK
				 	int pstop = time();
					perf_record(perf_monread,pstop-pstart);
				 	#endif    
                }
                ptr= IOmgr_obtainjob(0,0,lastjob);

           }
          while (ptr!=0);    
          sync_leavecrit(&IOrequest_busy);
          //enable_taskswitching(); /*Turn on task switiching*/
      };
      
                    #ifdef DEBUG_IOREADWRITE
                    char *scrptr = (char*)0xB8000;
	  				if (*scrptr=='|') *scrptr='/';
						 else
				    if (*scrptr=='/') *scrptr='-';
                         else						
					if (*scrptr=='-') *scrptr='\\';
						else
					*scrptr='|';	
                    #endif
    } while (1);

;};

//dequeue a request
IOrequest *IOmgr_obtainjob(int deviceid,
    DWORD lblockhigh,DWORD lblocklow /*for optimization*/)
 {
  IOrequest *ptr,*tmp,*handlecand;
  DWORD mindist=0xFFFFFFFF;
                    #ifdef DEBUG_IOREADWRITE
                    printf("dex32_diskmgr: obtaining job\n");
                    #endif
  //Wait until the IO manager is ready
  sync_entercrit(&IOrequest_busy);
  
  if (IOjob==0) {
          sync_leavecrit(&IOrequest_busy);
          return 0;
            };
            
  ptr=IOjob;
  handlecand = IOjob;
  
  while (ptr!=0)
   {
      DWORD dist;
      
      if ( lblocklow > ptr->lowblock) 
           dist=lblocklow-ptr->lowblock;
      else
           dist=ptr->lowblock-lblocklow;
           
      if (dist<mindist)
         {
          handlecand=ptr;
          mindist=dist;
         };
         
     ptr=ptr->next;
   ;};

   ptr=handlecand;

  //remove the JOB from the queue and return
  if (ptr == IOjob)
      IOjob=ptr->next;

  if (ptr->prev!=0)
      ptr->prev->next=ptr->next;

  if (ptr->next!=0)
      ptr->next->prev=ptr->prev;

  sync_leavecrit(&IOrequest_busy);
  return ptr;
;};

int dex32_IOcomplete(DWORD handle)
  {
  int retval;
  IOrequest *ptr;
                    #ifdef DEBUG_IOREADWRITE
                    //printf("IOcomplete enter.\n");
                    #endif
  //wait until the I/O manager is ready
  sync_entercrit(&IOrequest_busy);
  
                     #ifdef DEBUG_IOREADWRITE
                    //printf("IOcomplete check handle.\n");
                    #endif
  
  ptr=(IOrequest*)handle;

  if (ptr->status==IO_COMPLETE) retval=1;
     else
  if (ptr->status==IO_ERROR) retval=-1;
     else
  if (ptr->status==IO_PENDING) retval=0;
    else //ptr->status was given an unknown value, this is impossible
          //unless a process overwrites the IOrequest data structure
     printf("iomgr() data structure protection error\n");
  sync_leavecrit(&IOrequest_busy);
  
  return retval;

  ;};

void dex32_closeIO(DWORD handle)
  {
  IOrequest *ptr;

  //wait until the I/O manager is ready
  sync_entercrit(&IOrequest_busy);
  
  ptr=(IOrequest*)handle;
  free(ptr);
  
  sync_leavecrit(&IOrequest_busy);
  ;};

DWORD dex32_requestIO(int deviceid,int type,DWORD block,DWORD numblocks, void *buf)
{
     IOrequest *ptr;
     devmgr_block_desc *myblock = (devmgr_block_desc*) devmgr_getdevice(deviceid);
     DWORD flags;
     //wait until the I/O manager is ready
     //printf("blcok %d request to addr %x",block,buf);
                 #ifdef DEBUG_READ
                 printf("dex32_diskmgr: read request to %d posted.\n",block);
                 #endif		
     sync_entercrit(&IOrequest_busy);
     //storeflags(&flags);
     //stopints();
     #ifdef DEBUG_IOREADWRITE2
     printf("R(");
     #endif
	 devmgr_setcontext(deviceid);
     if (myblock->getcache!=0)
     if (type==IO_READ&&myblock->getcache(buf,block,numblocks))
      {
        ptr=(IOrequest*)malloc(sizeof(IOrequest));
        ptr->rID=(DWORD)ptr;
        ptr->type = type;
        ptr->lowblock = block;
        ptr->num_of_blocks = numblocks;
        ptr->status = IO_COMPLETE;
        ptr->buf=buf;
       

        #ifdef DEBUG_IOREADWRITE2
        printf(")r\n");
        #endif
        //restoreflags(flags);
        sync_leavecrit(&IOrequest_busy);
        return (DWORD)ptr->rID;
      };
      
      if (myblock->putcache!=0)
      if (type==IO_WRITE&& myblock->putcache(buf,block,numblocks))
      {
        ptr=(IOrequest*)malloc(sizeof(IOrequest));
        ptr->rID=(DWORD)ptr;
        ptr->type = type;
        ptr->lowblock = block;
        ptr->status = IO_COMPLETE;
        ptr->buf=buf;
        ptr->num_of_blocks = numblocks;

        
        #ifdef DEBUG_IOREADWRITE2
        printf(")r\n");
        #endif
        //restoreflags(flags);
        sync_leavecrit(&IOrequest_busy);
        return (DWORD)ptr->rID;
      };
      
     //queue the request
     ptr=(IOrequest*)malloc(sizeof(IOrequest));
     if (IOjob==0) 
     {
      IOjob=ptr;
      ptr->next=0;
      ptr->prev=0;
     }
      else
     {
      ptr->next=IOjob;
      ptr->prev=0;
      IOjob->prev=ptr;
      IOjob=ptr;
     };
     
      ptr->deviceid = deviceid;    
      ptr->rID=(DWORD)ptr;
      ptr->type = type;
      ptr->lowblock = block;
      ptr->status = IO_PENDING;
      ptr->buf=buf;
      ptr->time=IOrequest_time++;
      ptr->num_of_blocks = numblocks;
      #ifdef DEBUG_IOREADWRITE2
      printf(")r\n");
      #endif
      //restoreflags(flags);      
      sync_leavecrit(&IOrequest_busy);

      return (DWORD)ptr->rID;
;};
