/*
  Name: sync.c
  Copyright: 
  Author: Joseph Emmanuel DL Dayo
  Date: 18/01/04 06:27
  Description: Provides kernel synchornization functions
*/
#ifndef SYNC_H
#define SYNC_H

//Defines the maximum number of user process that can allocate a synchornization
//handle
#define MAX_SYNC 65535

typedef struct _sync_sharedvar {
    char owner[50];    
    int busyflag;
    int pid;
    int ready;
    int wait;
} sync_sharedvar;



extern int testandset(DWORD *ptr);
void sync_entercrit(sync_sharedvar *var);
void sync_leavecrit(sync_sharedvar *var);
int sync_userallocate();

#endif
