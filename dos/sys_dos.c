// sys_dos.c -- dos system driver, adapated from Quake 1

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dir.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <conio.h>
#include <crt0.h> // FS: Fake Mem Fix (QIP)
#include <dos.h> // FS: For detecting Windows NT

int _crt0_startup_flags = _CRT0_FLAG_UNIX_SBRK; // FS: Fake Mem Fix (QIP)

#include "dosisms.h"
#include "../qcommon/qcommon.h"
#include "../client/keys.h"
#include "errno.h"
#include "glob.h"

#define MINIMUM_WIN_MEMORY                      0x800000
#define MINIMUM_WIN_MEMORY_LEVELPAK     (MINIMUM_WIN_MEMORY + 0x100000)

#define KEYBUF_SIZE     256
static unsigned char    keybuf[KEYBUF_SIZE];
static int                              keybuf_head=0;
static int                              keybuf_tail=0;

extern char     start_of_memory __asm__("start");

byte        scantokey[128] = 
					{ 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6', 
	'7',    '8',    '9',    '0',    '-',    '=',    K_BACKSPACE, 9, // 0 
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', 
	'o',    'p',    '[',    ']',    13 ,    K_CTRL,'a',  's',      // 1 
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';', 
	'\'' ,    '`',    K_SHIFT,'\\',  'z',    'x',    'c',    'v',      // 2 
	'b',    'n',    'm',    ',',    '.',    '/',    K_SHIFT,'*', 
	K_ALT,' ',   0  ,    K_F1, K_F2, K_F3, K_F4, K_F5,   // 3 
	K_F6, K_F7, K_F8, K_F9, K_F10,0  ,    0  , K_HOME, 
	K_UPARROW,K_PGUP,'-',K_LEFTARROW,'5',K_RIGHTARROW,'+',K_END, //4 
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,              K_F11, 
	K_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
					}; 

byte        shiftscantokey[128] = 
					{ 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
	0  ,    27,     '!',    '@',    '#',    '$',    '%',    '^', 
	'&',    '*',    '(',    ')',    '_',    '+',    K_BACKSPACE, 9, // 0 
	'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I', 
	'O',    'P',    '{',    '}',    13 ,    K_CTRL,'A',  'S',      // 1 
	'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':', 
	'"' ,    '~',    K_SHIFT,'|',  'Z',    'X',    'C',    'V',      // 2 
	'B',    'N',    'M',    '<',    '>',    '?',    K_SHIFT,'*', 
	K_ALT,' ',   0  ,    K_F1, K_F2, K_F3, K_F4, K_F5,   // 3 
	K_F6, K_F7, K_F8, K_F9, K_F10,0  ,    0  , K_HOME, 
	K_UPARROW,K_PGUP,'_',K_LEFTARROW,'%',K_RIGHTARROW,'+',K_END, //4 
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,              K_F11, 
	K_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
					}; 

void TrapKey(void)
{
	keybuf[keybuf_head] = dos_inportb(0x60);
	dos_outportb(0x20, 0x20);


	keybuf_head = (keybuf_head + 1) & (KEYBUF_SIZE-1);
}

#define SC_UPARROW              0x48
#define SC_DOWNARROW    0x50
#define SC_LEFTARROW            0x4b
#define SC_RIGHTARROW   0x4d
#define SC_LEFTSHIFT   0x2a
#define SC_RIGHTSHIFT   0x36
#define SC_RIGHTARROW   0x4d

#define STDOUT 1
double	sys_msg_time;
double	sys_frame_time;
int	sys_checksum;

void MaskExceptions (void);
void Sys_PushFPCW_SetHigh (void);
void Sys_PopFPCW (void);
double Sys_FloatTime (void);

#define LEAVE_FOR_CACHE (512*1024)              //FIXME: tune
#define LOCKED_FOR_MALLOC (128*1024)    //FIXME: tune


int                     end_of_memory;
qboolean        lockmem, lockunlockmem, unlockmem;
qboolean		bSkipWinCheck, bSkipLFNCheck; // FS
static int      win95;
static int                      minmem;

// FS: Q2 needs it badly
// FS: See http://www.delorie.com/djgpp/doc/libc/libc_380.html for more information
/* ATTENTION FORKERS
   DO NOT REMOVE THE SLEEP OR WARNING!
   THIS IS SERIOUS, NO LFN AND SOME SKIN NAMES GET TRUNCATED
   WEIRD SHIT HAPPENS
   DON'T SEND ME BUG REPORTS FROM A SESSION WITH NO LFN DRIVER LOADED!
*/
void Sys_DetectLFN (void)
{
	unsigned int fd = _get_volume_info (NULL, 0, 0, NULL);

	if(bSkipLFNCheck)
	{
		return;
	}

	if(!(fd & _FILESYS_LFN_SUPPORTED))
	{
		printf("WARNING: Long file name support not detected!  Grab a copy of DOSLFN!\n");
		sleep(2);
		printf("Continuing to load Quake II. . .\n");
	}
}

qboolean Sys_DetectWinNT (void) // FS: Wisdom from Gisle Vanem
{
	if(_get_dos_version(1) == 0x0532)
	{
		return true;
	}

	return false;
}

void Sys_DetectWin95 (void)
{
	__dpmi_regs                             r;

	r.x.ax = 0x160a;                /* Get Windows Version */
	__dpmi_int(0x2f, &r);

	if((bSkipWinCheck) || (((r.x.ax) || (r.h.bh < 4)) && !(Sys_DetectWinNT())) )        /* Not windows or earlier than Win95 */
	{
		win95 = 0;
		lockmem = true;
		lockunlockmem = false;
		unlockmem = true;
	}
	else
	{
		Sys_Error("Microsoft Windows detected.  You must run Q2DOS in MS-DOS."); // FS: Warning.  Too many issues in Win9x and even XP.  QDOS is the same way.  So forget it.
	}
}

__dpmi_meminfo	info; // FS: Sigh, moved this here because everyone wants me to free this shit at exit.  Again, I'm pretty sure CWSDPMI is already taking care of this...
void *dos_getmaxlockedmem(int *size)
{
	__dpmi_free_mem_info    meminfo;
	int	working_size;
	void	*working_memory;
	int	last_locked;
//	int	extra,  i, j, allocsize;
	int	i, j, extra, allocsize; // FS: 2GB Fix
	static char	*msg = "Locking data...";
	// int	m, n;
	byte	*x;
	unsigned long	ul; // FS: 2GB Fix

// first lock all the current executing image so the locked count will
// be accurate.  It doesn't hurt to lock the memory multiple times
	last_locked = __djgpp_selector_limit + 1;
	info.size = last_locked - 4096;
	info.address = __djgpp_base_address + 4096;

	if (lockmem)
	{
		if(__dpmi_lock_linear_region(&info))
		{
			Sys_Error ("Lock of current memory at 0x%lx for %ldKb failed!",
						info.address, info.size/1024);
		}
	}

	__dpmi_get_free_memory_information(&meminfo);

	if (!win95)             /* Not windows or earlier than Win95 */
	{
		ul = meminfo.maximum_locked_page_allocation_in_pages * 4096; // FS: 2GB fix
	}
	else
	{
		ul = meminfo.largest_available_free_block_in_bytes -
		LEAVE_FOR_CACHE; // FS: 2GB fix
	}

	if (ul > 0x7fffffff)
	{
		ul = 0x7fffffff; /* limit to 2GB */
	}

	working_size = (int) ul;
	working_size &= ~0xffff;                /* Round down to 64K */
	working_size += 0x10000;

	do
	{
		working_size -= 0x10000;                /* Decrease 64K and try again */
		working_memory = sbrk(working_size);
	}
	while (working_memory == (void *)-1);

	extra = 0xfffc - ((unsigned)sbrk(0) & 0xffff);

	if (extra > 0)
	{
		sbrk(extra);
		working_size += extra;
	}

// now grab the memory
	info.address = last_locked + __djgpp_base_address;

	if (!win95)
	{
	    info.size = __djgpp_selector_limit + 1 - last_locked;

		while (info.size > 0 && __dpmi_lock_linear_region(&info))
		{
			info.size -= 0x1000;
			working_size -= 0x1000;
			sbrk(-0x1000);
		}
	}
	else
	{                       /* Win95 section */
		j = COM_CheckParm("-winmem");

		minmem = MINIMUM_WIN_MEMORY;

		if (j)
		{
			allocsize = ((int)(Q_atoi(com_argv[j+1]))) * 0x100000 +
					LOCKED_FOR_MALLOC;

			if (allocsize < (minmem + LOCKED_FOR_MALLOC))
			{
				allocsize = minmem + LOCKED_FOR_MALLOC;
			}
		}
		else
		{
			allocsize = minmem + LOCKED_FOR_MALLOC;
		}

		if (!lockmem)
		{
		// we won't lock, just sbrk the memory
			info.size = allocsize;
			goto UpdateSbrk;
		}

		// lock the memory down
		write (STDOUT, msg, strlen (msg));

		for (j=allocsize ; j>(minmem + LOCKED_FOR_MALLOC) ;
			 j -= 0x100000)
		{
			info.size = j;
	
			if (!__dpmi_lock_linear_region(&info))
			{
				goto Locked;
			}
	
			write (STDOUT, ".", 1);
		}

	// finally, try with the absolute minimum amount
		for (i=0 ; i<10 ; i++)
		{
			info.size = minmem + LOCKED_FOR_MALLOC;

			if (!__dpmi_lock_linear_region(&info))
			{
				goto Locked;
			}
		}

	Sys_Error ("Can't lock memory; %ld Mb lockable RAM required. "
				"Try shrinking smartdrv.", info.size / 0x100000);

Locked:

UpdateSbrk:

		info.address += info.size;
		info.address -= __djgpp_base_address + 4; // ending point, malloc align
		working_size = info.address - (int)working_memory;
		sbrk(info.address-(int)sbrk(0));                // negative adjustment
	}


	if (lockunlockmem)
	{
		__dpmi_unlock_linear_region (&info);
		printf ("Locked and unlocked %d Mb data\n", working_size / 0x100000);
	}
	else if (lockmem)
	{
		printf ("Locked %d Mb data\n", working_size / 0x100000);
	}
	else
	{
		printf ("Allocated %d Mb data\n", working_size / 0x100000);
	}

// touch all the memory to make sure it's there. The 16-page skip is to
// keep Win 95 from thinking we're trying to page ourselves in (we are
// doing that, of course, but there's no reason we shouldn't)
	x = (byte *)working_memory;

	for (j=0 ; j<4 ; j++) // FS: 2GB Fix
	{
		for (i=0 ; i<(working_size - 16 * 0x1000) ; i += 4)
		{
			sys_checksum += *(int *)&x[i];
			sys_checksum += *(int *)&x[i + 16 * 0x1000];
		}
	}

// give some of what we locked back for malloc before returning.  Done
// by cheating and passing a negative value to sbrk
	working_size -= LOCKED_FOR_MALLOC;
	sbrk( -(LOCKED_FOR_MALLOC));
	*size = working_size;

	return working_memory;
}

/*
int virtualmemsize;
byte *virtualmembase;
*/

int Sys_Get_Physical_Memory(void) // FS: From DJGPP tutorial
{
	_go32_dpmi_meminfo info;
	_go32_dpmi_get_free_memory_information(&info);

	if (info.available_physical_pages != -1)
	{
		return info.available_physical_pages * 4096;
	}
	return info.available_memory;
}

void Sys_Memory_Stats_f (void)
{
	Com_Printf("%d Mb available for Q2DOS.\n", (Sys_Get_Physical_Memory() / 0x100000) ); // FS: Added
	Com_Printf("%lu Virtual Mb available for Q2DOS.\n", (_go32_dpmi_remaining_virtual_memory() / 0x100000) ); // FS: Added
}
/*
================
Sys_PageInProgram

walks the text, data, and bss to make sure it's all paged in so that the
actual physical memory detected by Sys_GetMemory is correct.
================
*/
void Sys_PageInProgram(void)
{
	int             i, j;

	end_of_memory = (int)sbrk(0);

	if (lockmem)
	{
		if (dos_lockmem ((void *)&start_of_memory,
						 end_of_memory - (int)&start_of_memory))
			Sys_Error ("Couldn't lock text and data");
	}

	if (lockunlockmem)
	{
		dos_unlockmem((void *)&start_of_memory,
						 end_of_memory - (int)&start_of_memory);
		printf ("Locked and unlocked %d Mb image\n",
				(end_of_memory - (int)&start_of_memory) / 0x100000);
	}
	else if (lockmem)
	{
		printf ("Locked %d Mb image\n",
				(end_of_memory - (int)&start_of_memory) / 0x100000);
	}
	else
	{
		printf ("Loaded %d Mb image\n",
				(end_of_memory - (int)&start_of_memory) / 0x100000);
	}

// touch the entire image, doing the 16-page skip so Win95 doesn't think we're
// trying to page ourselves in
	for (j=0 ; j<4 ; j++)
	{
		for(i=(int)&start_of_memory ; i<(end_of_memory - 16 * 0x1000) ; i += 4)
		{
			sys_checksum += *(int *)i;
			sys_checksum += *(int *)(i + 16 * 0x1000);
		}
	}
	
	printf("%d Mb available for Q2DOS.\n", (Sys_Get_Physical_Memory() / 0x100000) ); // FS: Added
	printf("%lu Virtual Mb available for Q2DOS.\n", (_go32_dpmi_remaining_virtual_memory() / 0x100000) ); // FS: Added
}

void Sys_SetTextMode (void) // FS: This was used twice, let's make it a little cleaner if we can.
{
	__dpmi_regs r;

	// return to text mode
	r.x.ax = 3;
	__dpmi_int(0x10, &r);
}

void Sys_Error (char *error, ...)
{
	va_list		argptr;

	if (!dedicated || !dedicated->value)
	{
		dos_restoreintr(9); // FS: Give back the keyboard
	}

	Sys_SetTextMode();

	printf ("Sys_Error: ");	
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");
	
	__dpmi_free_physical_address_mapping(&info);	
	__djgpp_nearptr_disable(); // FS: Everyone else is a master DOS DPMI programmer.  Pretty sure CWSDPMI is already taking care of this...

#if 0
{	//we crash here so we can get a backtrace.  Yes it is ugly, and no this should never be in production!
	int j,k;
fflush(stdout);
	j=0;
	k=5/j;	//divide by zero!
}
#endif
	exit (1);
}

void Sys_Quit (void)
{
	if(!dedicated || !dedicated->value)
	{
		dos_restoreintr(9); // FS: Give back the keyboard
	}

	if (unlockmem)
	{
		dos_unlockmem (&start_of_memory,
					   end_of_memory - (int)&start_of_memory);
	}

	Sys_SetTextMode();
	
	__dpmi_free_physical_address_mapping(&info);	
	__djgpp_nearptr_disable(); // FS: Everyone else is a master DOS DPMI programmer.  Pretty sure CWSDPMI is already taking care of this...

	exit (0);
}

void *GetGameAPI (void *import);
#ifdef GAME_HARD_LINKED
void	Sys_UnloadGame (void)
{
}

void	*Sys_GetGameAPI (void *parms)
{
	return GetGameAPI (parms);
}
// needs to be statically linked for null
// otherwise it sits here to satisfy the linker AFIK
#else

#endif	


char *Sys_ConsoleInput (void)
{
	static char     text[256];
	static int      len = 0;
	char            ch;

	if (!dedicated || !dedicated->value)
	{
		return NULL;
	}

	if (! kbhit())
	{
		return NULL;
	}

	ch = getche();

	switch (ch)
	{
		case '\r':
			putch('\n');
			if (len)
			{
				text[len] = 0;
				len = 0;
				return text;
			}
			break;

		case '\b':
			putch(' ');
			if (len)
			{
				len--;
				putch('\b');
			}
			break;

		default:
			text[len] = ch;
			len = (len + 1) & 0xff;
			break;
	}

	return NULL;
}

void	Sys_ConsoleOutput (char *string)
{
	if (!dedicated || !dedicated->value)
	{
		return;
	}

	printf("%s",string);
}

#define SC_RSHIFT       0x36 
#define SC_LSHIFT       0x2a 
void Sys_SendKeyEvents (void)
{
	int k, next;
	int outkey;

// get key events

	while (keybuf_head != keybuf_tail)
	{
		sys_msg_time = Sys_Milliseconds();//Sys_FloatTime();
		k = keybuf[keybuf_tail++];
		keybuf_tail &= (KEYBUF_SIZE-1);

		if (k==0xe0)
			continue;               // special / pause keys
		next = keybuf[(keybuf_tail-2)&(KEYBUF_SIZE-1)];
		if (next == 0xe1)
			continue;                               // pause key bullshit
		if (k==0xc5 && next == 0x9d) 
		{ 
			Key_Event (K_PAUSE, true, sys_msg_time);
			continue; 
		} 

		// extended keyboard shift key bullshit 
		if ( (k&0x7f)==SC_LSHIFT || (k&0x7f)==SC_RSHIFT ) 
		{ 
			if ( keybuf[(keybuf_tail-2)&(KEYBUF_SIZE-1)]==0xe0 ) 
				continue; 
			k &= 0x80; 
			k |= SC_RSHIFT; 
		} 

		if (k==0xc5 && keybuf[(keybuf_tail-2)&(KEYBUF_SIZE-1)] == 0x9d)
			continue; // more pause bullshit

		outkey = scantokey[k & 0x7f];

		if (k & 0x80)
		{
			Key_Event (outkey, false, sys_msg_time);
		}
		else
		{
			Key_Event (outkey, true, sys_msg_time);
		}
	}
}

void Sys_AppActivate (void)
{
}

void Sys_CopyProtect (void)
{
}

char *Sys_GetClipboardData(void)
{
	return NULL;
}

void	Sys_Init (void)
{
	MaskExceptions ();

	Sys_SetFPCW ();

	_go32_interrupt_stack_size = 4 * 1024;
	_go32_rmcb_stack_size = 4 * 1024;
}

double Sys_FloatTime (void)
{
	return (double) uclock() / (double) UCLOCKS_PER_SEC; // FS: Win9X/Fast PC Fix (QIP)
}

void Sys_ParseEarlyArgs(int argc, char **argv) // FS: Parse some very specific args before Qcommon_Init
{
	int i = 0;

	for (i = 1; i < argc; i++)
	{
		if(strncasecmp((char*)argv[i] + 1,"skipwincheck", 12) == 0)
		{
			bSkipWinCheck = true;
		}

		if(strncasecmp((char*)argv[i] + 1,"skiplfncheck", 12) == 0)
		{
			bSkipLFNCheck = true;
		}

	}
}

//=============================================================================

int main (int argc, char **argv)
{
	double time, oldtime, newtime;

	Sys_ParseEarlyArgs(argc, argv);
	Sys_DetectLFN();
	Sys_DetectWin95 ();
	Sys_PageInProgram ();

	Sys_Init();

	Qcommon_Init (argc, argv);

	oldtime = Sys_Milliseconds ();

	if (!dedicated || !dedicated->value)
	{
		dos_registerintr(9, TrapKey);
	}

    /* main window message loop */
	while (1)
	{
		do
		{
			newtime = Sys_Milliseconds ();
			time = newtime - oldtime;
		}
		while (time < 1);

		Qcommon_Frame (time);
		sys_frame_time = newtime; // FS: Need to update this for input to work properly
		oldtime = newtime;
	}
}

void Sys_MakeCodeWriteable(void)
{
// MS-DOS is always writeable
}
