/*
  Name: sync.c
  Copyright: 
  Author: Joseph Emmanuel DL Dayo
  Date: 18/01/04 06:27
  Description: Provides kernel synchornization functions
  -used hardware bit test and set instruction instead
*/

#include "sync.h"

int sync_handles_totalitems = 0;
sync_sharedvar *sync_handles_list=0;

void sync_justwait(sync_sharedvar *var)
{
   while (var->busyflag && var->pid!=getprocessid());
};

void sync_entercrit(sync_sharedvar *var)
{
    while (testandset(&var->busyflag)) {
          if (var->pid==getprocessid()) break;
    }
    var->pid = getprocessid();
    var->wait++;
};


/*void sync_entercrit(sync_sharedvar *var)
{
    while (var->busy && var->pid!=getprocessid());
    var->busy = 1;
    var->pid = getprocessid();
    var->wait++;
};*/


void sync_leavecrit(sync_sharedvar *var)
{
 
    var->wait--;
    if (var->wait<0) {
         printf("sync: warning wrong number of enter-leave pairs detected!\n");
         printf("sync: sync value %d\n",var->wait);
         printf("sync: owner %s\n",var->owner);
     }
    if (var->wait==0) var->busyflag = 0;
};

//Allocates a synchronization handle for user mode programs
int sync_userallocate()
{
    if (sync_handles_list == 0) {
       sync_handles_list = (sync_sharedvar*)malloc(MAX_SYNC * sizeof(sync_sharedvar));
       sync_handles_totalitems = 0;
    }
    if (sync_handles_totalitems >= MAX_SYNC) return -1;
    sync_handles_totalitems++;
    
    return sync_handles_totalitems;
}
