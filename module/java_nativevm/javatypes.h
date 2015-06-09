/*
  Name: Java Types Definition
  Copyright: 
  Author: Joseph Emmanuel DL Dayo
  Date: 06/08/06 20:56
  Description: Contains the java types for the java classloader
*/

#ifndef _JAVATYPES_H_
#define _JAVATYPES_H_
typedef struct  __attribute__((packed)) _cp_info {
    	char tag;
    	char *info;
} cp_info;

typedef struct  __attribute__((packed)) _JavaClass_header {
    	DWORD magic_class;
    	WORD minor_version;
    	WORD major_version;
    	WORD constant_pool_count;
} JavaClass_header;

typedef struct  __attribute__((packed)) _constant_pool {
        cp_info *constant_pool;
} constant_pool;

typedef struct  __attribute__((packed)) _class_flags {
    	WORD access_flags;
    	WORD this_class;
    	WORD super_class;
    	WORD interfaces_count;
} class_flags;

typedef struct _classinfo {
      JavaClass_header header;  
} classinfo;

#endif
