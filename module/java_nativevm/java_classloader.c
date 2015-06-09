/*
  Name: quick java class loader
  Copyright: 
  Author: Joseph Emmanuel DL Dayo
  Date: 06/08/06 08:30
  Description: This module handles java class file loading
*/
#include "javatypes.h"

int java_loadusermodule(
        char *module_name, //the name of the module
        char *class_image,  //the location of the pe image
        DWORD base, //the desired base address to load the image so
                    //that dex can perform the necessary relocations
        int mode,   //Determines what kind of executable to load
        char *p,    //the parameters
        char *workdir,
        PCB386 *parent //the parent PCB for EXEs, the parent pagedirectory for DLLs
        ) //location of image to load
{
        classinfo *classinfo_ptr=(classinfo*)class_image;
        if (classinfo_ptr->header.magic_class = 0xCAFEBABE) {
           int major_version = classinfo_ptr->header.major_version;
           int minor_version = classinfo_ptr->header.minor_version;
           printf("java class has been detected. execute not yet supported.\n");
           printf("major version: %d\n", major_version);
           printf("minor version: %d\n", minor_version);
           //a linked list used to keep track of the virtual addresses used
           //by this process so that it could be freed when the process
           //terminates
           return 0;
        }
        return 0;
};
