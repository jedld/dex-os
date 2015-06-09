/* 
   Page Addressing Extensions version.
   This function maps the linear address to a physical address:
   The function automatically allocates a page for a page table
   the bottom 12 bits of the linear address is discarded since
   it is not used by the mapper*/
void maplineartophysical_pae(unsigned int *pdptableptr, /*page-directory-pointer table*/
                             unsigned int linearaddr, /*the paged aligned linear address requested*/
                             unsigned int physical,   /*the paged aligned physical address to map to*/
							 unsigned int attribute /*used to specify the page attributes to be applied*/
					    )
   {
	  unsigned int pdptableindex,pagedirindex,pagetableindex;
	  unsigned int *pdptable,*pagedir,*pagetable;
	  /*get the index of to the page-directory-pointer table*/

	  pdptableindex = linearaddr >> 30;
	  pagedirindex = (linearaddr & 0x3FFFFFFF) >> 21
	  pagetableindex= (linearaddr  & 0x1FFFFF) >> 12;
	  
	  /*get the location of the page table and mask the first 12 bits*/
	  DWORD pdptableentry_l=pdptableptr[pdptableindex*2];
	  DWORD pdptableentry_h=pdptableptr[pdptableindex*2+1];
	  
	  /*We can only map until 4GB without Paging and PAE so that the high bit 
      of the pdptablentry is useless, for now*/
	  pagedir = (unsigned int*)(pdptableentry_l & 0xFFFFF000);
	  if (pagedir == 0) { //There seems to be no entry yet
	     DWORD ptr = (DWORD) mempop();
	     pagedir = (unsigned int*)ptr; 
	     memset(pagedir,0,4096);
	     pdptableptr[pdptableindex*2] = ptr | 1;
      }   
	  
	  DWORD pagedirentry_l = pagedir[pagedirindex*2];
  	  DWORD pagedirentry_h = pagedir[pagedirindex*2+1];
	  pagetable = (unsigned int*)(pagedirentry_l & 0xFFFFF000);
	  	  
	  if (pagetable==0)  /*there is no entry?*/
		     {
  			    /*map a new memory location*/
  			    DWORD ptr = (DWORD) mempop();
				pagedir[pagedirindex*2] = ptr | 1 | PG_USER | PG_WR;
				pagetable=(unsigned int*)ptr;
				/*clear the locations of the page table to zero*/
				memset(pagetable,0,4096);
             };
	  physical=(physical & 0xFFFFF000) | attribute;
      pagetable[pagetableindex*2]=physical;
      /*done!*/
   };

void mem_init_pae()
{
    DWORD i;
    pagedir1=mempop(); //obtain the first pagedirectory

    clearpagetable(pagedir1);
    
    //map the first 1MB one to one
    for (i=0;i<0xB8000;i+=0x1000)
        maplineartophysical_pae((DWORD*)pagedir1,i,i        /*,stackbase*/,1 );
    for (i=0xb8000;i<0x100000;i+=0x1000)
        maplineartophysical_pae((DWORD*)pagedir1,i,i        /*,stackbase*/,1 | PG_USER | PG_WR );
    for (i=0x100000;i<0x300000;i+=0x1000)
        maplineartophysical_pae((DWORD*)pagedir1,i,i        /*,stackbase*/,1);
    
    maplineartophysical_pae((DWORD*)pagedir1,(DWORD)(stackbase - 0x1000) ,0,0);
    
    maplineartophysical_pae((DWORD*)pagedir1,(DWORD)SYS_PAGEDIR_VIR,(DWORD)pagedir1    /*,stackbase*/,1);
    maplineartophysical_pae((DWORD*)pagedir1,(DWORD)SYS_PAGEDIR2_VIR,
    (DWORD)pagedir1[SYS_PAGEDIR_VIR >> 22]&0xFFFFF000,1);
    maplineartophysical_pae((DWORD*)pagedir1,(DWORD)SYS_PAGEDIR3_VIR,
    (DWORD)pagedir1[SYS_PAGEDIR_VIR >> 22]&0xFFFFF000,1);
    maplineartophysical_pae((DWORD*)pagedir1,(DWORD)SYS_KERPDIR_VIR,(DWORD)pagedir1    /*,stackbase*/,1);

    setpagedir(pagedir1);
    enablepaging();
};
