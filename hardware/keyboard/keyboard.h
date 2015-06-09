/*
  Name: DEX32 low-level keyboard device handler
  Copyright: 
  Author: Joseph Emmanuel DL Dayo
  Date: 28/11/03 15:22
  Description: This module provides low-level keyboard functions like getch()
*/

//The size of the keyboard bufer
#define BUF_SIZE 256
#define KB_CURRENTSET 2

#define CAPS_LOCK 4
#define NUM_LOCK 2
#define SCROLL_LOCK 1


#define	RAW1_LEFT_CTRL		0x1D
#define	RAW1_LEFT_SHIFT		0x2A
#define	RAW1_CAPS_LOCK		0x3A
#define	RAW1_LEFT_ALT		0x38
#define	RAW1_RIGHT_ALT		0x38	/* same as left */
#define	RAW1_RIGHT_CTRL		0x1D	/* same as left */
#define	RAW1_RIGHT_SHIFT	0x36
#define	RAW1_SCROLL_LOCK	0x46
#define	RAW1_NUM_LOCK		0x45
#define	RAW1_DEL		0x53

#define	RAW2_LEFT_CTRL		0x14
#define	RAW2_LEFT_SHIFT		0x12
#define	RAW2_CAPS_LOCK		0x58
#define	RAW2_LEFT_ALT		0x11
#define	RAW2_RIGHT_ALT		0x11
#define	RAW2_RIGHT_CTRL		0x14
#define	RAW2_RIGHT_SHIFT	0x59
#define	RAW2_SCROLL_LOCK	0x7E
#define	RAW2_NUM_LOCK		0x77
#define	RAW2_DEL		    0x71

#define	RAW3_LEFT_CTRL		0x11
#define	RAW3_LEFT_SHIFT		0x12
#define	RAW3_CAPS_LOCK		0x14
#define	RAW3_LEFT_ALT		0x19
#define	RAW3_RIGHT_ALT		0x39
#define	RAW3_RIGHT_CTRL		0x58
#define	RAW3_RIGHT_SHIFT	0x59
#define	RAW3_SCROLL_LOCK	0x5F
#define	RAW3_NUM_LOCK		0x76
#define	RAW3_DEL		0x64

/*----------------------------------------------------------------------------
SCANCODE CONVERSION
----------------------------------------------------------------------------*/
#define CTRL_ALT    400
#define ALT_TAB     59

/* "ASCII" values for non-ASCII keys. All of these are user-defined.
function keys: */
#define	KEY_F1		0x80
#define	KEY_F2		(KEY_F1 + 1)
#define	KEY_F3		(KEY_F2 + 1)
#define	KEY_F4		(KEY_F3 + 1)
#define	KEY_F5		(KEY_F4 + 1)
#define	KEY_F6		(KEY_F5 + 1)
#define	KEY_F7		(KEY_F6 + 1)
#define	KEY_F8		(KEY_F7 + 1)
#define	KEY_F9		(KEY_F8 + 1)
#define	KEY_F10		(KEY_F9 + 1)
#define	KEY_F11		(KEY_F10 + 1)
#define	KEY_F12		(KEY_F11 + 1)
/* cursor keys */
#define	KEY_INS		0x90
#define	KEY_DEL		(KEY_INS + 1)
#define	KEY_HOME	(KEY_DEL + 1)
#define	KEY_END		(KEY_HOME + 1)
#define	KEY_PGUP	(KEY_END + 1)
#define	KEY_PGDN	(KEY_PGUP + 1)
#define	KEY_LFT		(KEY_PGDN + 1)
#define	KEY_UP		(KEY_LFT + 1)
#define	KEY_DN		(KEY_UP + 1)
#define	KEY_RT		(KEY_DN + 1)
/* print screen/sys rq and pause/break */
#define	KEY_PRNT	(KEY_RT + 1)
#define	KEY_PAUSE	(KEY_PRNT + 1)
/* these return a value but they could also act as additional meta keys */
#define	KEY_LWIN	(KEY_PAUSE + 1)
#define	KEY_RWIN	(KEY_LWIN + 1)
#define	KEY_MENU	(KEY_RWIN + 1)

#define SOFT_RESET  0xAFFF

/* "meta bits"
0x0100 is reserved for non-ASCII keys, so start with 0x200 */
#define	KBD_META_ALT	0x0200	/* Alt is pressed */
#define	KBD_META_CTRL	0x0400	/* Ctrl is pressed */
#define	KBD_META_SHIFT	0x0800	/* Shift is pressed */
#define	KBD_META_ANY	(KBD_META_ALT | KBD_META_CTRL | KBD_META_SHIFT)
#define	KBD_META_CAPS	0x1000	/* CapsLock is on */
#define	KBD_META_NUM	0x2000	/* NumLock is on */
#define	KBD_META_SCRL	0x4000	/* ScrollLock is on */
/*****************************************************************************
*****************************************************************************/


int start_ps=0;
unsigned char statusbits=0,ostatus=0;
unsigned kbd_status;

int deq_busy=0; int busy=0;

typedef struct
{
	unsigned int *data;
	unsigned size, in_ptr, out_ptr;
} queue_t;

static unsigned int _kbd_buf[BUF_SIZE];


static queue_t _q =
{
	_kbd_buf, BUF_SIZE, 0, 0
};

//This structure is used for defining hotkeys
typedef struct _kb_hotkey_info {
    int id;
    unsigned int key, //The ascii equivalent of the keystroke
             status;   /*possible status bits (SHIFT pressed, CAPS LOCk etc.) 
                         of the keyboard, set to 0xFF for any status*/
    void (*handler)(); /*The function that will be called if this hotkey is pressed*/
    
    //This is a doubly linked list so .....
    struct _kb_hotkey_info *prev;
    struct _kb_hotkey_info *next;
    
} kb_hotkey_info;

kb_hotkey_info *hotkey_list = 0;
int kb_totalhotkeys = 0;

int kb_current_set = KB_CURRENTSET;

sync_sharedvar kb_busywait;

/****************** The Key Map table for all the various keyboard 
 scancode sets*/
	static const unsigned char map[] =
	{
/* 00 */0,	0x1B,	'1',	'2',	'3',	'4',	'5',	'6',
/* 08 */'7',	'8',	'9',	'0',	'-',	'=',	'\b',	'\t',
/* 10 */'q',	'w',	'e',	'r',	't',	'y',	'u',	'i',
/* 1Dh is left Ctrl */
/* 18 */'o',	'p',	'[',	']',	'\n',	0,	'a',	's',
/* 20 */'d',	'f',	'g',	'h',	'j',	'k',	'l',	';',
/* 2Ah is left Shift */
/* 28 */'\'',	'`',	0,	'\\',	'z',	'x',	'c',	'v',
/* 36h is right Shift */
/* 30 */'b',	'n',	'm',	',',	'.',	'/',	0,	0,
/* 38h is left Alt, 3Ah is Caps Lock */
/* 38 */0,	' ',	0,	KEY_F1,	KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,
/* 45h is Num Lock, 46h is Scroll Lock */
/* 40 */KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10,0,	0,	KEY_HOME,
/* 48 */KEY_UP,	KEY_PGUP,'-',	KEY_LFT,'5',	KEY_RT,	'+',	KEY_END,
/* 50 */KEY_DN,	KEY_PGDN,KEY_INS,KEY_DEL,0,	0,	0,	KEY_F11,
/* 58 */KEY_F12
	};

static const unsigned char shift_map[] =
	{
/* 00 */0,	0x1B,	'!',	'@',	'#',	'$',	'%',	'^',
/* 08 */'&',	'*',	'(',	')',	'_',	'+',	'\b',	'\t',
/* 10 */'Q',	'W',	'E',	'R',	'T',	'Y',	'U',	'I',
/* 1Dh is left Ctrl */
/* 18 */'O',	'P',	'{',	'}',	'\n',	0,	'A',	'S',
/* 20 */'D',	'F',	'G',	'H',	'J',	'K',	'L',	':',
/* 2Ah is left Shift */
/* 28 */'"',	'~',	0,	'|',	'Z',	'X',	'C',	'V',
/* 36h is right Shift */
/* 30 */'B',	'N',	'M',	'<',	'>',	'?',	0,	0,
/* 38h is left Alt, 3Ah is Caps Lock */
/* 38 */0,	' ',	0,	KEY_F1,	KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,
/* 45h is Num Lock, 46h is Scroll Lock */
/* 40 */KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10,0,	0,	KEY_HOME,
/* 48 */KEY_UP,	KEY_PGUP,'-',	KEY_LFT,'5',	KEY_RT,	'+',	KEY_END,
/* 50 */KEY_DN,	KEY_PGDN,KEY_INS,KEY_DEL,0,	0,	0,	KEY_F11,
/* 58 */KEY_F12
	};
	
static const unsigned char map2[] = {

/*0x0*/ '1', KEY_F9, 0, KEY_F5, KEY_F3, KEY_F1, KEY_F2, KEY_F12, 0, KEY_F10,
/*0xA*/ KEY_F8, KEY_F6, KEY_F4, '\t', '`', 0, 0, 0 /*ALT*/, 0 /*R Shift*/, 0,
/*0x14*/ 0 /*RCtrl*/, 'q', '1', 0, 0, 0, 'z', 's', 'a', 'w',
/*0x1E*/ '2', 0, 0, 'c', 'x', 'd', 'e', '4', '3', 0 ,
/*0x28*/ 0, ' ', 'v', 'f', 't', 'r', '5', 0, 0, 'n',
/*0x32*/ 'b', 'h', 'g', 'y', '6', 0, 0, 0, 'm', 'j',
/*0x3C*/ 'u', '7', '8', 0, 0, ',', 'k', 'i', 'o', '0',
/*0x46*/ '9', 0, 0, '.', '/', 'l' , ';', 'p', '-', 0,
/*0x50*/ 0, 0, '\'', 0, '[', '=', 0, 0, 0 /*CapsLock*/,0 /*RShift*/,
/*0x5A*/ '\n', ']', 0, '\\', 0, 0, 0, 0, 0, 0,
/*0x64*/ 0, 0, '\b', 0, 0, KEY_END, 0, KEY_LFT , KEY_HOME /*KP-7 / Home*/,0,
/*0x6E*/ 0, 0, '0' /*KP-0 / Ins*/, KEY_DEL /*KP-. / Del*/, KEY_DN /*KP-2 / Down*/
         ,'5' /*KP-5*/, KEY_RT /*KP-6 / Right*/, KEY_UP /*KP-8 / Up*/, '\e' /*ESC*/,0 /*NumLock*/,
/*0x78*/ KEY_F11, '+' /*KP-+*/, KEY_PGDN /*KP-3 / PgDn*/, '-'/*KP--*/, '*'/*KP-**/,KEY_PGUP, 0, 0
};	

static const unsigned char shift_map2[] = {

/*0x0*/ '1', KEY_F9, 0, KEY_F5, KEY_F3, KEY_F1, KEY_F2, KEY_F12, 0, KEY_F10,
/*0xA*/ KEY_F8, KEY_F6, KEY_F4,'\t', '~', 0, 0, 0 /*RAlt*/, 0 /*LShift*/, 0,
/*0x14*/0 /*'e0 RCtrl*/,'Q', '!', 0, 0, 0, 'Z', 'S', 'A', 'W',
/*0x1E*/'@', 0, 0, 'C', 'X', 'D', 'E', '$', '#', 0,
/*0x28*/ 0, ' ', 'V', 'F', 'T', 'R', '%', 0, 0, 'N',
/*0x32*/'B', 'H', 'G', 'Y', '^', 0, 0, 0, 'M', 'J',
/*0x3C*/'U', '&', '*', 0, 0, '<', 'K', 'I', 'O', ')',
/*0x46*/ '(', 0, 0, '>', '?', 'L', ':', 'P', '_', 0,
/*0x50*/ 0, 0, '"', 0, '{', '+', 0, 0, 0 /*CapsLock*/, 0 /*RShift*/,
/*0x5A*/ '\n', '}', 0, '|', 0, 0, 0, 0, 0, 0,
/*0x64*/ 0, 0, '\b', 0, 0, KEY_END, 0, KEY_LFT , KEY_HOME /*KP-7 / Home*/,0,
/*0x6E*/ 0, 0, '0' /*KP-0 / Ins*/, KEY_DEL /*KP-. / Del*/, KEY_DN /*KP-2 / Down*/
         ,'5' /*KP-5*/, KEY_RT /*KP-6 / Right*/, KEY_UP /*KP-8 / Up*/, '\e' /*ESC*/,0 /*NumLock*/,
/*0x78*/ KEY_F11, '+' /*KP-+*/, KEY_PGDN /*KP-3 / PgDn*/, '-'/*KP--*/, '*'/*KP-**/,KEY_PGUP, 0, 0
};


	
//	static const unsigned char map2[] =
//	{
///* 00 */0, KEY_F9, '1', KEY_F5,	KEY_F3,	KEY_F1,	KEY_F2,	KEY_F12,
///* 08 */'7', KEY_F10, '9', 0,	0,	0,	0,	'q',
///* 10 */'1',	0,	0,	0, 0, 'c', 'x', 'd',
///* 1Dh is left Ctrl */
///* 18 */'e',	'4',	'3',	0,	0,	' ',	0,	'n',
///* 20 */'b',	'h',	'g',	'y',	'6',	0,	0,	0,
///* 2Ah is left Shift */
///* 28 */0, ',', 'k', 'l', 'o', '0', '9', 0,
///* 36h is right Shift */
///* 30 */0,	'.',	0,	0,	'\'',	0,	'[', '=',
///* 38h is left Alt, 3Ah is Caps Lock */
///* 38 */0,	0,	'\b',	0,	0,	'1','0','.',
///* 45h is Num Lock, 46h is Scroll Lock */
///* 40 */KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10,0,	0,	KEY_HOME,
///* 48 */KEY_UP,	KEY_PGUP,'-',	KEY_LFT,'5',	KEY_RT,	'+',	KEY_END,
///* 50 */KEY_DN,	KEY_PGDN,KEY_INS,KEY_DEL,0,	0,	0,	KEY_F11,
///* 58 */KEY_F12
//	};
//	
//	static const unsigned char shift_map2[] =
//	{
///* 00 */0,	0x1B,	'!',	'@',	'#',	'$',	'%',	'^',
///* 08 */'&',	'*',	'(',	')',	'_',	'+',	'\b',	'\t',
///* 10 */'Q',	'W',	'E',	'R',	'T',	'Y',	'U',	'I',
///* 1Dh is left Ctrl */
///* 18 */'O',	'P',	'{',	'}',	'\n',	0,	'A',	'S',
///* 20 */'D',	'F',	'G',	'H',	'J',	'K',	'L',	':',
///* 2Ah is left Shift */
///* 28 */'"',	'~',	0,	'|',	'Z',	'X',	'C',	'V',
///* 36h is right Shift */
///* 30 */'B',	'N',	'M',	'<',	'>',	'?',	0,	0,
///* 38h is left Alt, 3Ah is Caps Lock */
///* 38 */0,	' ',	0,	KEY_F1,	KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,
///* 45h is Num Lock, 46h is Scroll Lock */
///* 40 */KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10,0,	0,	KEY_HOME,
///* 48 */KEY_UP,	KEY_PGUP,'-',	KEY_LFT,'5',	KEY_RT,	'+',	KEY_END,
///* 50 */KEY_DN,	KEY_PGDN,KEY_INS,KEY_DEL,0,	0,	0,	KEY_F11,
///* 58 */KEY_F12
//	};

	static const unsigned char map3[] =
	{
/* 00 */0,	0,	0,	0,	0,	0,	0,	KEY_F1,
/* 08 */0x1B,	0,	0,	0,	0,	0x09,	'`',	KEY_F2,
/* 10 */0,	0,	0,	0,	0,	'Q',	'1',	KEY_F3,
/* 18 */0,	0,	'z',	's',	'a',	'w',	'2',	KEY_F4,
/* 20 */0,	'c',	'x',	'd',	'e',	'4',	'3',	KEY_F5,
/* 28 */0,	' ',	'v',	'f',	't',	'r',	'5',	KEY_F6,
/* 30 */0,	'n',	'b',	'h',	'g',	'y',	'6',	KEY_F7,
/* 38 */0,	0,	'm',	'j',	'u',	'7',	'8',	KEY_F8,
/* 40 */0,	',',	'k',	'i',	'o',	'0',	'9',	KEY_F9,
/* 48 */0,	'.',	'/',	'l',	';',	'p',	'-',	KEY_F10,
/* 50 */0,	0,	'\'',	0,	'[',	'=',	KEY_F11,KEY_PRNT,
/* 58 */0,	0,	'\n',	']',	'\\',	0,	KEY_F12,0,
/* 60 */KEY_DN,	KEY_LFT,KEY_PAUSE,KEY_UP,KEY_DEL,KEY_END,0x08,	KEY_INS,
/* 68 */0,	KEY_END,KEY_RT,	KEY_LFT,KEY_HOME,KEY_PGDN,KEY_HOME,KEY_PGUP,
/* 70 */KEY_INS,KEY_DEL,KEY_DN,	'5',	KEY_RT,	KEY_UP,	0,	'/',
/* 78 */0,	0x0D,	KEY_PGDN,0,	'+',	KEY_PGUP,'*',	0,
/* 80 */0,	0,	0,	0,	'-',	0,	0,	0,
/* 88 */0,	0,	0,	KEY_LWIN,KEY_RWIN,KEY_MENU,0,	0
	};

	static const unsigned char shift_map3[] =
	{
/* 00 */0,	0,	0,	0,	0,	0,	0,	KEY_F1,
/* 08 */0x1B,	0,	0,	0,	0,	0x09,	'~',	KEY_F2,
/* 11 is left Ctrl; 12 is left Shift; 14 is CapsLock */
/* 10 */0,	0,	0,	0,	0,	'q',	'!',	KEY_F3,
/* 19 is left Alt */
/* 18 */0,	0,	'z',	's',	'a',	'w',	'@',	KEY_F4,
/* 20 */0,	'c',	'x',	'd',	'e',	'$',	'#',	KEY_F5,
/* 28 */0,	' ',	'v',	'f',	't',	'r',	'%',	KEY_F6,
/* 30 */0,	'n',	'b',	'h',	'g',	'y',	'^',	KEY_F7,
/* 39 is right Alt */
/* 38 */0,	0,	'm',	'j',	'u',	'&',	'*',	KEY_F8,
/* 40 */0,	'<',	'k',	'i',	'o',	')',	'(',	KEY_F9,
/* 48 */0,	'>',	'?',	'l',	':',	'p',	'_',	KEY_F10,
/* 50 */0,	0,	'"',	0,	'{',	'+',	KEY_F11,KEY_PRNT,
/* 58 is right Ctrl; 59 is right Shift; 5F is Scroll Lock */
/* 58 */0,	0,	'\n',	'}',	'|',	0,	KEY_F12,0,
/* 60 */KEY_DN,	KEY_LFT,KEY_PAUSE,KEY_UP,KEY_DEL,KEY_END,0x08,	KEY_INS,
/* 68 */0,	'1',	KEY_RT,	'4',	'7',	KEY_PGDN,KEY_HOME,KEY_PGUP,
/* 76 is Num Lock */
/* 70 */'0',	'.',	'2',	'5',	'6',	'8',	0,	'/',
/* 78 */0,	0x0D,	'3',	0,	'+',	'9',	'*',	0,
/* 80 */0,	0,	0,	0,	'-',	0,	0,	0,
/* 88 */0,	0,	0,	KEY_LWIN,KEY_RWIN,KEY_MENU,0,	0
	};
	

/***********************Prototype definition Here*****************************/
void write_kbd(unsigned adr, unsigned data);
static int set_scancode_to_ascii(unsigned code,unsigned int code_set);
void settogglebits(unsigned char b);
static int inq(queue_t *q, unsigned int data);
static int deq(queue_t *q, unsigned int *data);
static int empty(queue_t *q);
void sendtokeyb(const char *s,queue_t *q);
int signal_foreground();
void setkeyfocus(DWORD id);
void kb_removehotkey(int id);
int kb_dohotkey(WORD key, WORD status);
int kb_addhotkey(WORD key,WORD status,void (*handler)());
int kb_keypressed();
int kb_dequeue(int *val);
void kb_setleds(unsigned int keyboard_status);
void keyboard_wait();
int kill_foreground();
void  kbd_irq(void);
static int read_kbd(void);
void keyboardflush();
void write_kbd(unsigned adr, unsigned data);
static int write_kbd_await_ack(unsigned val);
static int init_kbd(unsigned ss, unsigned typematic, unsigned xlat);
char pause();
int kb_ready();
char getch();
unsigned int getchw();
void installkeyboard();


