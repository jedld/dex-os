/*
  Name: DEX32 low-level keyboard device handler
  Copyright: 
  Author: Joseph Emmanuel DL Dayo
  Date: 28/11/03 15:22
  Description: This module provides low-level keyboard functions like getch()\
  
  1/1/2004 - Added enhancement support for the MLQsched module
  
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

void settogglebits(unsigned char b)
{
    write_kbd(0x60,0xED);
    write_kbd(0x60,b);
;};

/*****************************************************************************
*****************************************************************************/
static int inq(queue_t *q, unsigned int data)
{
	unsigned temp;

	temp = q->in_ptr + 1;
	if(temp >= q->size)
		temp = 0;
/* if in_ptr reaches out_ptr, the queue is full */
	if(temp == q->out_ptr)
		return -1;
	q->data[q->in_ptr] = data;
	q->in_ptr = temp;
	return 0;
}


static int deq(queue_t *q, unsigned int *data)
{
/* if out_ptr reaches in_ptr, the queue is empty */
    while (deq_busy);
    deq_busy=1;
	if(q->out_ptr == q->in_ptr)
     {
      deq_busy=0;
     return -1;
     };
	*data = q->data[q->out_ptr++];
	if(q->out_ptr >= q->size)
		q->out_ptr = 0;
		
      deq_busy=0;
	return 0;
}

static int empty(queue_t *q)
{
	return q->out_ptr == q->in_ptr;
}


//this function sends characters to the keyboard buffer
//from a string of characters
void sendtokeyb(const char *s,queue_t *q)
{
 int i;
 stopints();
 for (i=0;s[i];i++)
   {
    inq(q,s[i]);
   };
 startints();
};

int signal_foreground()
{
if (fg_getkeyboardowner()!=0)
  {
     sched_sysmes[0]=fg_getkeyboardowner();
     sched_sysmes[1]=SIG_TERM;
     sched_sysmes[2]=0;
     return 1;
  };
  return 0;
;};


int kill_foreground()
{
  if (fg_getkeyboardowner() != 0)
   {
     printf("\nForeground Application force terminated using Ctrl-X\n");
     if ( fg_vconsoles[fg_current]->pid != fg_getkeyboardowner())
     sigterm = fg_getkeyboardowner();
     return 1;
   };
   return 0;
};

void  kbd_irq(void)
{
	unsigned scan_code;
   char temp[255];
   unsigned int c;
   DWORD flags;
   dex32_stopints(&flags);
/* read I/O port 60h to reset interrupt at 8042 keyboard controller chip */
	scan_code = inportb(0x60);
    c = set_scancode_to_ascii(scan_code,kb_current_set);
   if (c!=-1)
    {

           if (!kb_dohotkey(c,kbd_status))
           {     
                   if (c == SOFT_RESET) ps_shutdown();
                        else 
                   if (c==KEY_F2+400) console_new();
                        else 
                   if (c==KEY_F1+400) dex32_set_tm_state(!dex32_get_tm_state());
                        else 
                   if (c==KEY_F4+400) kill_foreground();  //force terminate
                        else
                   if (c=='c'-'a')
                        signal_foreground(); //ask the app to terminate itself
                   else
                        {
                      	if (inq(&_q,c)==-1) beep();
                      	}
          	};
    };
    
   dex32_restoreints(flags);
};

static int read_kbd(void)
{
	unsigned long timeout;
	unsigned stat, data;

	for(timeout = 500000L; timeout != 0; timeout--)
	{
		stat = inportb(0x64);
/* loop until 8042 output buffer full */
		if(stat & 0x01)
		{
			data = inportb(0x60);
/* loop if parity error or receive timeout */
			if((stat & 0xC0) == 0)
				return data;
		}
	}

	return -1;
};

void keyboardflush()
 {
  char c,code;
  do
   {
    c=deq(&_q,&code);
   }
   while (c!=-1);
 };

void write_kbd(unsigned adr, unsigned data)
{
	unsigned long timeout;
	unsigned stat;

	for(timeout = 500000L; timeout != 0; timeout--)
	{
		stat = inportb(0x64);
/* loop until 8042 input buffer empty */
		if((stat & 0x02) == 0)
			break;
	}
	outportb(adr, data);
};

static int write_kbd_await_ack(unsigned val)
{
	int got;

	write_kbd(0x60, val);
	got = read_kbd();
	if(got != 0xFA)
	{
		
		return -1;
	}
	return 0;
};

/*Initializes the keyboard hardware, the desired scancode set is
  placed in ss (a value of zero means use the current scancode set
  the keyboard supports)*/
static int init_kbd(unsigned ss, unsigned typematic, unsigned xlat)
{
    	while(read_kbd() != -1)
    		/* nothing */;
    /* disable keyboard before programming it */
    	write_kbd_await_ack(0xF5);
    /* disable PS/2 mouse, set SYS bit, and Enable Keyboard Interrupt... */
    	write_kbd(0x64, 0x60);
    /* ...and either disable or enable AT-to-XT keystroke conversion */
    	write_kbd(0x60, xlat ? 0x65 : 0x25);

    /* program desired scancode set */
    	write_kbd_await_ack(0xF0);
    
    /*OK let's get the current scan code set*/
    if (ss == 0)
    {
        write_kbd_await_ack(0);
        ss = read_kbd();
        printf("kb_driver: Current set = %d.\n", ss);
    }
        else /*User specified the scancode to use*/
    {
    
    /*Make sure keyboard supports this set, if not we use the other one*/
     	if (write_kbd_await_ack(ss) == -1)
     	{
            if (ss == 2) 
                    ss = 3;
                else
                    ss = 1;       
                    
            write_kbd_await_ack(ss);
        };
     };   
     
     //Make the current keybaord scancode known to the whole driver
     kb_current_set = ss;
    /* we want all keys to return both a make code (when pressed)
    and a break code (when released -- scancode set 3 only) */
    	if(ss == 3)
    	{
    		write_kbd_await_ack(0xFA);
    	}
    /* set typematic delay and rate */
    	write_kbd_await_ack(0xF3);
    	write_kbd_await_ack(typematic);
    /* enable keyboard */
    	write_kbd_await_ack(0xF4);
    	return 0;
}

void kb_setleds(unsigned int keyboard_status)
{
        unsigned int temp;
    	write_kbd(0x60, 0xED);	/* "set LEDs" command */
    	temp = 0;
		if(keyboard_status & KBD_META_SCRL)
			temp |= 1;
		if(keyboard_status & KBD_META_NUM)
			temp |= 2;
		if(keyboard_status & KBD_META_CAPS)
			temp |= 4;
		write_kbd(0x60, temp);	/* bottom 3 bits set LEDs */
    
};

static int set_scancode_to_ascii(unsigned code,unsigned int code_set)
{

	static unsigned saw_break_code, saw_extended_code;
/**/
	unsigned temp;

/* check for break code (i.e. a key is released) */
	//set 1
    if(code >= 0x80 && code_set == 1)
	{
		saw_break_code = 1;
		code &= 0x7F;
	}
	
	if (code == 0xF0 && code_set == 2)
	{
	    saw_break_code = 1;
	    return -1;
	}
	
	/*OK, this is an extended code.. let's decode it.*/
	if (code == 0xe0 && code_set == 2)
	{
	    return -1;
	}
	//set 3
	if(code == 0xF0 && code_set == 3)
	{
		saw_break_code = 1;
		return -1;
	}
	
/* the only break codes we're interested in are Shift, Ctrl, Alt */
	if(saw_break_code && code_set == 1)
	{
		if(code == RAW1_LEFT_ALT || code == RAW1_RIGHT_ALT)
			kbd_status &= ~KBD_META_ALT;
		else if (code == RAW1_LEFT_CTRL || code == RAW1_RIGHT_CTRL)
			kbd_status &= ~KBD_META_CTRL;
		else if (code == RAW1_LEFT_SHIFT || code == RAW1_RIGHT_SHIFT)
			kbd_status &= ~KBD_META_SHIFT;
		saw_break_code = 0;
		return -1;
	}
	
	if( saw_break_code && code_set == 2)
	{
		
    		if ( (code == RAW2_LEFT_ALT) ||  (code == RAW2_RIGHT_ALT) )
    			kbd_status &= ~KBD_META_ALT;
	    		else 
            if ( (code == RAW2_LEFT_CTRL) || (code == RAW2_RIGHT_CTRL ) )
    			kbd_status &= ~KBD_META_CTRL;
 			
    		else if (code == RAW2_LEFT_SHIFT || code == RAW2_RIGHT_SHIFT )
    			kbd_status &= ~KBD_META_SHIFT;
    			
    		saw_extended_code = 0;
		    saw_break_code = 0;

		return -1;
	}

	if(saw_break_code && code_set == 3 )
	{
		if(code == RAW3_LEFT_ALT || code == RAW3_RIGHT_ALT)
			kbd_status &= ~KBD_META_ALT;
		else if(code == RAW3_LEFT_CTRL || code == RAW3_RIGHT_CTRL)
			kbd_status &= ~KBD_META_CTRL;
		else if(code == RAW3_LEFT_SHIFT || code == RAW3_RIGHT_SHIFT)
			kbd_status &= ~KBD_META_SHIFT;
		saw_break_code = 0;
		return -1;
	}

/* it's a make code: check the "meta" keys, as above */
    if (code_set == 1)
    {
        	if(code == RAW1_LEFT_ALT || code == RAW1_RIGHT_ALT)
        	{
        		kbd_status |= KBD_META_ALT;
        		return -1;
        	}
        	
        	if(code == RAW1_LEFT_CTRL || code == RAW1_RIGHT_CTRL)
        	{
        		kbd_status |= KBD_META_CTRL;
        		return -1;
        	}
        	
        	if(code == RAW1_LEFT_SHIFT || code == RAW1_RIGHT_SHIFT)
        	{
        		kbd_status |= KBD_META_SHIFT;
        		return -1;
        	}
	};
	
	if (code_set == 2)
    {
        	if(code == RAW2_LEFT_ALT || (code == RAW2_RIGHT_ALT) )
        	{
        		kbd_status |= KBD_META_ALT;
        		return -1;
        	}
        	
        	if(code == RAW2_LEFT_CTRL || code == RAW2_RIGHT_CTRL )
        	{
        		kbd_status |= KBD_META_CTRL;
        		return -1;
        	}
        	
        	if(code == RAW2_LEFT_SHIFT || (code == RAW2_RIGHT_SHIFT) )
        	{
        		kbd_status |= KBD_META_SHIFT;
        		return -1;
        	}
  	};
	
	if (saw_extended_code) 
	{
    	saw_extended_code=0;
        return -1;
	};
	
	if (code_set == 3)
	{
        	if(code == RAW3_LEFT_ALT || code == RAW3_RIGHT_ALT)
        	{
        		kbd_status |= KBD_META_ALT;
        		return -1;
        	}
        	if(code == RAW3_LEFT_CTRL || code == RAW3_RIGHT_CTRL)
        	{
        		kbd_status |= KBD_META_CTRL;
        		return -1;
        	}
        	if(code == RAW3_LEFT_SHIFT || code == RAW3_RIGHT_SHIFT)
        	{
        		kbd_status |= KBD_META_SHIFT;
        		return -1;
        	}
	};
/* Scroll Lock, Num Lock, and Caps Lock set the LEDs. These keys
have on-off (toggle or XOR) action, instead of momentary action */
    if (code_set == 1)
    {
        	if(code == RAW1_SCROLL_LOCK)
        	{
        		kbd_status ^= KBD_META_SCRL;
        		kb_setleds(kbd_status);
        		return -1;
        	}
        	if(code == RAW1_NUM_LOCK)
        	{
        		kbd_status ^= KBD_META_NUM;
        		kb_setleds(kbd_status);
        		return -1;
        	}
        	
        	if(code == RAW1_CAPS_LOCK)
        	{
        		kbd_status ^= KBD_META_CAPS;
        		kb_setleds(kbd_status);
        		return -1;
        	};
	};
	
	 if (code_set == 2  )
    {
        	if(code == RAW2_SCROLL_LOCK)
        	{
        		kbd_status ^= KBD_META_SCRL;
        		kb_setleds(kbd_status);
        		return -1;
        	}
        	if(code == RAW2_NUM_LOCK)
        	{
        		kbd_status ^= KBD_META_NUM;
        		kb_setleds(kbd_status);
        		return -1;
        	}
        	
        	if(code == RAW2_CAPS_LOCK)
        	{
        		kbd_status ^= KBD_META_CAPS;
        		kb_setleds(kbd_status);
        		return -1;
        	};
	};
	
	if (code_set == 3)
	{
        	if(code == RAW3_SCROLL_LOCK)
        	{
        		kbd_status ^= KBD_META_SCRL;
        		kb_setleds(kbd_status);
        		return -1;
        	}
        	if(code == RAW3_NUM_LOCK)
        	{
        		kbd_status ^= KBD_META_NUM;
        		kb_setleds(kbd_status);
        		return -1;
        	}
        	if(code == RAW3_CAPS_LOCK)
        	{
        		kbd_status ^= KBD_META_CAPS;
        		kb_setleds(kbd_status);
        		return -1;
        	}
	};    
	
    if ( (kbd_status & KBD_META_ALT) && (kbd_status & KBD_META_CTRL) && code_set == 1 )
        {
      		if(code >= sizeof(shift_map) / sizeof(shift_map[0]))
			return -1;
   		    temp = shift_map[code];
   		    if (temp == KEY_DEL) return SOFT_RESET;
   		    return temp+400;
        };	
        
     if ( (kbd_status & KBD_META_ALT) && (kbd_status & KBD_META_CTRL) && code_set == 2 )
        {
      		if(code >= sizeof(shift_map2) / sizeof(shift_map2[0]))
			return -1;
   		    temp = shift_map2[code];
   		    if (temp == KEY_DEL) return SOFT_RESET;
   		    
   		    return temp+400;
        };	    
        
    if ( (kbd_status & KBD_META_ALT) && (kbd_status & KBD_META_CTRL) && code_set == 3 )
        {
      		if(code >= sizeof(shift_map3) / sizeof(shift_map3[0]))
			return -1;
   		    temp = shift_map3[code];
   		    if (temp == KEY_DEL) return SOFT_RESET;
   		    return temp+400;
        };	
		
	
	/*if(kbd_status & KBD_META_ALT)
		return code+300;*/

/* convert A-Z[\]^_ to control chars */
	if(kbd_status & KBD_META_CTRL)
	{
	    if (code_set == 1)
	    {
        	if(code >= sizeof(map) / sizeof(map[0]))
    			return -1;
    		temp = map[code];
		}
		else
       if (code_set == 2)
	    {
        	if(code >= sizeof(map2) / sizeof(map2[0]))
    			return -1;
    		temp = map2[code];
		}
		else
		if (code_set == 3)
		{
        	if(code >= sizeof(map3) / sizeof(map3[0]))
   			return -1;
    		temp = map3[code];
		};
		
		if(temp >= 'a' && temp <= 'z')
			return temp - 'a';
		if(temp >= '[' && temp <= '_')
			return temp - '[' + 0x1B;
		return -1;
	}
/* convert raw scancode to ASCII */
	if(kbd_status & KBD_META_SHIFT)
	{
/* ignore invalid scan codes */
	    if (code_set == 1)
	    {
    		if(code >= sizeof(shift_map) / sizeof(shift_map[0]))
    			return -1;
    		temp = shift_map[code];
		}
		else
		if (code_set == 2)
	    {
    		if(code >= sizeof(shift_map2) / sizeof(shift_map2[0]))
    			return -1;
    		temp = shift_map2[code];
		}
		else
		if (code_set == 3)
		{
        	if(code >= sizeof(shift_map3) / sizeof(shift_map3[0]))
   			return -1;
    		temp = shift_map3[code];
		};

/* defective keyboard? non-US keyboard? more than 104 keys? */
		if(temp == 0)
			return -1;
/* caps lock? */
		if(kbd_status & KBD_META_CAPS)
		{
			if(temp >= 'A' && temp <= 'Z')
            {
                switch (code_set)
                {
                case 1 : temp = map[code]; break;
                case 2 : temp = map2[code]; break;
                case 3 : temp = map3[code]; break;
                };
            };
		}
	}
	else
	{
	    if (code_set == 1)
	    {
    		if(code >= sizeof(map) / sizeof(map[0]))
    			return -1;
    		temp = map[code];
		}
		else
		if (code_set == 2)
		{
			if(code >= sizeof(map2) / sizeof(map2[0]))
    			return -1;
    		temp = map2[code];
		}
		else
		{
    		if(code >= sizeof(map3) / sizeof(map3[0]))
    			return -1;
    		temp = map3[code];
		};
		
		if(temp == 0)
			return -1;
			
		if(kbd_status & KBD_META_CAPS)
		{
			if (temp >= 'a' && temp <= 'z')
            { 
                switch (code_set)
                {
                case 1 : temp = shift_map[code]; break;
                case 2 : temp = shift_map2[code]; break;
                case 3 : temp = shift_map3[code]; break;
                };
            };
		}
	}
	
	return temp;
};

char pause()
 {
   unsigned int code,c;
   do
   {
    sleep(20);        
    c=deq(&_q,&code);
   }
   while (c==-1);
  
   return ((char)code);
 };

int kb_keypressed()
{
   
   
   if ( getprocessid() != fg_getkeyboardowner())    
   return 0;
   return !empty(&_q);
};

int kb_ready()
{
   return !empty(&_q);
};

char getch()
 {
   unsigned int code,c;
   do
   {
    keyboard_wait();
    c=deq(&_q,&code);
    	
   }
   while (c==-1);
   
   return ((char)code);
 };

//waits until control of the keyboard is released by another process
void keyboard_wait()
{
    
   devmgr_scheduler_extension *cursched = 
        (devmgr_scheduler_extension*)extension_table[CURRENT_SCHEDULER].iface; 
  sleep(50);    
    
  if (fg_current)  
  while ( getprocessid() != fg_getkeyboardowner())
    {
        
   /***** Send to the scheduler, if it supports it, 
    that this process is to be sent to the io-queue, this is
    quite hackish though***/
        
    if (getprocessid() != fg_getkeyboardowner())
        devmgr_sendmessage(cursched->hdr.id,2,current_process->processid);
    else
    	devmgr_sendmessage(cursched->hdr.id,3,current_process->processid);
     	   
     //we may probably allow child threads to share the keyoard with its parent
     if ( (current_process->status & PS_ATTB_THREAD) 
           && current_process->accesslevel!=ACCESS_SYS
           && current_process->owner == fg_getkeyboardowner())
     break;
     taskswitch();
     
    };
 };

unsigned int getchw()
 {
  unsigned int code,c;
  do
   {
    keyboard_wait();   
    c=deq(&_q,&code);
   }
   while (c==-1);
   if (current_process->accesslevel==ACCESS_USER)
   return code;
 };

void installkeyboard()
 {
   int c;
   settogglebits(0);
  
 };

void kb_pause()
{
int c,code;
//empty keyboard buffer
keyboardflush();
//wait until a key is pressed
do
 {
          sleep(20);   
          c=deq(&_q,&code);
 }
while (c==-1);


};

int kb_getchar()
{
int c,code;
c=deq(&_q,&code);
if (c==-1) 
{
  sleep(20);         
  return -1;
};
return code;
};

int kb_dequeue(int *val)
{
int retval;
if (getprocessid()!=fg_getkeyboardowner())
          retval= -1;
                else
          retval= deq(&_q,val);
return retval;
};

int kb_dohotkey(WORD key, WORD status)
{
    int retval = 0;
    kb_hotkey_info *ptr = hotkey_list;
    
    sync_entercrit(&kb_busywait);
    
    while (ptr!=0)
        {
            if  (ptr->key == key && ( (ptr->status & status) || (ptr->status == 0xFF) ) )
                            {
                                ptr->handler();
                                retval = 1;
                                break;
                            };
            ptr= ptr->next;    
        };    
        
    sync_leavecrit(&kb_busywait);
    return retval;
};

//search for a hotkey definition with ID and then removes it
void kb_removehotkey(int id)
{
    kb_hotkey_info *ptr = hotkey_list;

    sync_entercrit(&kb_busywait);
    
    while (ptr!=0)
        {
              if (ptr->id == id)
                  {
                        if (hotkey_list == ptr)
                           {
                               hotkey_list = ptr->next;
                           };              
                           
                        if (ptr->next) ptr->next->prev = ptr->prev;
                        if (ptr->prev) ptr->prev->next = ptr->next;   
                        free(ptr); 
                        break;
                  };  
              ptr = ptr->next;
        };
        
    sync_leavecrit(&kb_busywait);
};

int kb_addhotkey(WORD key,WORD status,void (*handler)())
{
    kb_hotkey_info *hotkey = (kb_hotkey_info*) malloc(sizeof(kb_hotkey_info));
    hotkey->key = key;
    hotkey->status = status;
    hotkey->handler = handler;
    hotkey->id = kb_totalhotkeys++;
    hotkey->prev = 0;
    
    sync_entercrit(&kb_busywait);

    //case when this is the first hotkey defined
    if (hotkey_list == 0)
        {
              hotkey->next = 0;
              hotkey_list = hotkey;
        }
    else
    //otherwise ..
        {
              //Add to the head  
              hotkey_list->prev = hotkey;
              hotkey->next = hotkey_list;
              hotkey_list = hotkey;  
        };
        
    sync_leavecrit(&kb_busywait);
    
    return hotkey->id;
};

//Handles parameters passed to the keyboard driver
int kb_sendmessage(int type,const char *message)
{
    if (type == DEVMGR_MESSAGESTR)
       {
              int l = strlen(message), i;
              char *p[100];
              char *temp = malloc(l+1);
              char *s;
              int c = 0;
              //make a temporary copy sinced we cannot modify message
              
              strcpy(temp,message);
              
              s=strtok(temp," ");
              
              do 
                 {
                      p[c]=s;
                      c++;
                      s=strtok(0," ");
                 }
              while (s!=0);
              
              for (i=0;i<c;i++)
                 {
                     if (strcmp(p[i],"-typerate")==0)                 
                         {
                               if (i+1 < c)
                               {
                               int rate = atoi(p[i+1]);
                               init_kbd(1,rate,0);                               
                               printf("keyb: Typematic rate changed\n");
                               i++;
                               };
                         }
                         
                     if (strcmp(p[i],"-codeset")==0)
                        printf("keyb: Using set (%d)\n", kb_current_set);
                        
                     if (strcmp(p[i],"-useset")==0)
                         {
                               if (i+1 < c)
                               {
                                   int set = atoi(p[i+1]);
                                   
                                   if (set>=1 && set <=3)
                                   {
                                           init_kbd(set,3,0);
                                           printf("keyb: keyboard now using set [%d].\n",set);
                                           i++;
                                   };
                               };
                         }
                 };
              free(temp);
              return 1;
       };
       

       return 1;
};


void init_keyboard()
{
    int devid;
    devmgr_char_desc mykeyboard;
    //initialize the keyboard interface according to the DEX 1.00 driver spec
    memset(&mykeyboard,0,sizeof(mykeyboard));
    mykeyboard.hdr.size = sizeof(mykeyboard);
    
    //set the type of device
    mykeyboard.hdr.type = DEVMGR_CHAR;
    strcpy(mykeyboard.hdr.name,"keyb");
    strcpy(mykeyboard.hdr.description,"Default generic keyboard driver 1.00");
    
    //fill up required fields
    mykeyboard.init_device = installkeyboard;
    mykeyboard.ready_put = 0; //you can only get from this device
    mykeyboard.ready_get = kb_ready;
    mykeyboard.get_char =kb_getchar;
    mykeyboard.put_char = 0;
    mykeyboard.hdr.sendmessage = kb_sendmessage;
    mykeyboard.set_callback_handler= 0;
    mykeyboard.get_callback_handler= 0;

    //register this device
    devid = devmgr_register( (devmgr_char_desc*)&mykeyboard);
    memset(&kb_busywait,0,sizeof(kb_busywait));
    
    //assign the keyboard wrapper to IRQ 1
    irq_addhandler(devid,1,kbd_irq);
    
    init_kbd(KB_CURRENTSET,3,0);

    hotkey_list = 0;
};

