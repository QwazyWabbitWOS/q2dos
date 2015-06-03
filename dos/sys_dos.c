// sys_null.h -- null system driver to aid porting efforts

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/nearptr.h>
#include <dpmi.h>
#include <conio.h>
#include <bios.h>
#include <crt0.h> // FS: Fake Mem Fix (QIP)

#include "dosisms.h"
#include "../qcommon/qcommon.h"
#include "../client/keys.h"
#include "errno.h"
#include "glob.h"

int _crt0_startup_flags = _CRT0_FLAG_UNIX_SBRK; // FS: Fake Mem Fix (QIP)
#define KEYBUF_SIZE     256
static unsigned char    keybuf[KEYBUF_SIZE];
static int                              keybuf_head=0;
static int                              keybuf_tail=0;

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
//      static int ctrl=0;
	keybuf[keybuf_head] = dos_inportb(0x60);
	dos_outportb(0x20, 0x20);
	/*
	if (scantokey[keybuf[keybuf_head]&0x7f] == K_CTRL)
		ctrl=keybuf[keybuf_head]&0x80;
	if (ctrl && scantokey[keybuf[keybuf_head]&0x7f] == 'c')
		Sys_Error("ctrl-c hit\n");
	*/
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
int	curtime;
unsigned	sys_msg_time;
unsigned	sys_frame_time;
int	sys_checksum;

void MaskExceptions (void);
void Sys_PushFPCW_SetHigh (void);
void Sys_PopFPCW (void);
double Sys_FloatTime (void);

#define LEAVE_FOR_CACHE (512*1024)              //FIXME: tune
#define LOCKED_FOR_MALLOC (128*1024)    //FIXME: tune
#define MINIMUM_WIN_MEMORY                      0x800000
#define MINIMUM_WIN_MEMORY_LEVELPAK     (MINIMUM_WIN_MEMORY + 0x100000)

int                     end_of_memory;
qboolean        lockmem, lockunlockmem, unlockmem;
static int      win95;
extern char     start_of_memory __asm__("start");
static int                      minmem;

void Sys_DetectWin95 (void)
{
	__dpmi_regs                             r;

	r.x.ax = 0x160a;                /* Get Windows Version */
	__dpmi_int(0x2f, &r);

	if(r.x.ax || r.h.bh < 4)        /* Not windows or earlier than Win95 */
	{
		win95 = 0;
		lockmem = true;
		lockunlockmem = false;
		unlockmem = true;
	}
	else
	{
		printf("Microsoft Windows detected.  Please run Q2DOS in pure MS-DOS for best stability.\n"); // FS: Warning
		win95 = 1;
		lockunlockmem = COM_CheckParm ("-winlockunlock");

		if (lockunlockmem)
			lockmem = true;
		else
			lockmem = COM_CheckParm ("-winlock");

		unlockmem = lockmem && !lockunlockmem;
	}
}

void *dos_getmaxlockedmem(int *size)
{
	__dpmi_free_mem_info    meminfo;
	__dpmi_meminfo                  info;
	int                                             working_size;
	void                                    *working_memory;
	int                                             last_locked;
        //int                                             extra,  i, j, allocsize;
	int                                     i, j, extra, allocsize; // FS: 2GB Fix
	static char                             *msg = "Locking data...";
	// int                                             m, n;
	byte                                    *x;
	unsigned long   ul; // FS: 2GB Fix

// first lock all the current executing image so the locked count will
// be accurate.  It doesn't hurt to lock the memory multiple times
	last_locked = __djgpp_selector_limit + 1;
	info.size = last_locked - 4096;
	info.address = __djgpp_base_address + 4096;

	if (lockmem)
	{
		if(__dpmi_lock_linear_region(&info))
		{
			Sys_Error ("Lock of current memory at 0x%lx for %ldKb failed!\n",
						info.address, info.size/1024);
		}
	}

	__dpmi_get_free_memory_information(&meminfo);

	if (!win95)             /* Not windows or earlier than Win95 */
	{
		//working_size = meminfo.maximum_locked_page_allocation_in_pages * 4096;
		ul = meminfo.maximum_locked_page_allocation_in_pages * 4096; // FS: 2GB fix
	}
	else
	{
//                working_size = meminfo.largest_available_free_block_in_bytes -
//                                LEAVE_FOR_CACHE;
		ul = meminfo.largest_available_free_block_in_bytes -
		LEAVE_FOR_CACHE; // FS: 2GB fix
	}

        if (ul > 0x7fffffff)
                ul = 0x7fffffff; /* limit to 2GB */
        working_size = (int) ul;
	working_size &= ~0xffff;                /* Round down to 64K */
	working_size += 0x10000;

	do
	{
		working_size -= 0x10000;                /* Decrease 64K and try again */
		working_memory = sbrk(working_size);
	} while (working_memory == (void *)-1);

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
				allocsize = minmem + LOCKED_FOR_MALLOC;
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
				goto Locked;
	
			write (STDOUT, ".", 1);
		}

	// finally, try with the absolute minimum amount
		for (i=0 ; i<10 ; i++)
		{
			info.size = minmem + LOCKED_FOR_MALLOC;

			if (!__dpmi_lock_linear_region(&info))
				goto Locked;
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

void Sys_GetMemory(void)
{
// FS: TODO FIXME FIXME doesn't have quakeparms.memsize or anything that I see
#if 0
	int             j, tsize;

	j = COM_CheckParm("-mem");
	if (j)
	{
		quakeparms.memsize = (int) (Q_atof(com_argv[j+1]) * 1024 * 1024);
		quakeparms.membase = malloc (quakeparms.memsize);
	}
	else
	{
		int j;
		//quakeparms.membase = dos_getmaxlockedmem (&quakeparms.memsize);

		if (extended_mod) // FS: We're not foolin' around with big boy mods...
			j=64;
		else
			j=32; // FS: from QW
		quakeparms.memsize = (int) j * 1024 * 1024; 
		quakeparms.membase = malloc (quakeparms.memsize);
	}

	fprintf(stderr, "malloc'd: %d\n", quakeparms.memsize);

	if (COM_CheckParm ("-noclear")) // FS: Wanted the option
	{
		return;
	}
	else
	{
		printf("Clearing allocated memory...\n");
		memset(quakeparms.membase,0x0,quakeparms.memsize); // JASON: Clear memory on startup
		printf("Done!  Continuing to load Quake.\n");
	}

	if (COM_CheckParm ("-heapsize"))
	{
		tsize = Q_atoi (com_argv[COM_CheckParm("-heapsize") + 1]) * 1024;

		if (tsize < quakeparms.memsize)
			quakeparms.memsize = tsize;
	}
#endif
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
}

void Sys_Error (char *error, ...)
{
	va_list		argptr;

   {
      __dpmi_regs r;

      r.x.ax = 3;
      __dpmi_int(0x10, &r);
   }	//return to text mode

	printf ("Sys_Error: ");	
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");
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
	dos_restoreintr(9); // FS: Give back the keyboard
	if (unlockmem)
	{
		dos_unlockmem (&start_of_memory,
					   end_of_memory - (int)&start_of_memory);
//		dos_unlockmem (quakeparms.membase, quakeparms.memsize);
	}
   {
      __dpmi_regs r;

      r.x.ax = 3;
      __dpmi_int(0x10, &r);
   }//return to text mode
	exit (0);
}

void	Sys_UnloadGame (void)
{
}

#ifdef GAME_HARD_LINKED
void *GetGameAPI (void *import);

void	*Sys_GetGameAPI (void *parms)
{
	return GetGameAPI (parms);
}
// needs to be statically linked for null
// otherwise it sits here to satisfy the linker AFIK
#else
void	*Sys_GetGameAPI (void *parms)
{
	return NULL;
}
#endif	


// FS: Doesn't work
char *Sys_ConsoleInput (void)
{
#if 0
	static char     text[256];
	static int      len = 0;
	char            ch;

	if (!dedicated || !dedicated->value)
		return NULL;

	if (! kbhit())
		return NULL;

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
#endif
	return NULL;
}

void	Sys_ConsoleOutput (char *string)
{
//printf("Sys_ConsoleOutput: %s",string);
//	if (dedicated || dedicated->value)
//		printf("%s",string);
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

char *Sys_GetClipboardData( void )
{
	return NULL;
}

int		hunkcount;


byte	*membase;
int		hunkmaxsize;
int		cursize;

void	*Hunk_Begin (int maxsize)
{
	// reserve a huge chunk of memory, but don't commit any yet
	cursize = 0;
	hunkmaxsize = maxsize;
	membase = malloc (maxsize);
	if (!membase)
		Sys_Error ("VirtualAlloc reserve failed %d bytes",maxsize);
	memset (membase, 0, maxsize);
	return (void *)membase;

}

void	*Hunk_Alloc (int size)
{
	void	*buf;

	// round to cacheline
	size = (size+31)&~31;

	cursize += size;
	if (cursize > hunkmaxsize)
		Sys_Error ("Hunk_Alloc overflow");

	return (void *)(membase+cursize-size);
}

void	Hunk_Free (void *buf)
{
	free (buf);
	hunkcount--;
}

int		Hunk_End (void)
{
	hunkcount++;
//Com_Printf ("hunkcount: %i\n", hunkcount);
	return cursize;
}

int		Sys_Milliseconds (void)
{
	struct timeval tp;
	struct timezone tzp;
	static int		secbase;

	gettimeofday(&tp, &tzp);
	
	if (!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec/1000;
	}

	curtime = (tp.tv_sec - secbase)*1000 + tp.tv_usec/1000;
	
	return curtime;
}

void	Sys_Mkdir (char *path)
{
	// FS: This works, just not on DOSBOX for some reason.
	mkdir (path, 0777);
//	printf("Sys_Mkdir [%s]: UNIMPLEMENTED!\n",path);
}
static	DIR		*fdir;
static	char	findbase[MAX_OSPATH];
static	char	findpath[MAX_OSPATH];
static	char	findpattern[MAX_OSPATH];

static qboolean CompareAttributes(char *path, char *name,
	unsigned musthave, unsigned canthave )
{
	struct stat st;
	char fn[MAX_OSPATH];

// . and .. never match
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return false;

	return true;

	if (stat(fn, &st) == -1)
		return false; // shouldn't happen

	if ( ( st.st_mode & S_IFDIR ) && ( canthave & SFF_SUBDIR ) )
		return false;

	if ( ( musthave & SFF_SUBDIR ) && !( st.st_mode & S_IFDIR ) )
		return false;

	return true;
}

char *Sys_FindFirst (char *path, unsigned musthave, unsigned canhave)
{
	struct dirent *d;
	char *p;

	if (fdir)
		Sys_Error ("Sys_BeginFind without close");

//	COM_FilePath (path, findbase);
	strcpy(findbase, path);

	if ((p = strrchr(findbase, '/')) != NULL) {
		*p = 0;
		strcpy(findpattern, p + 1);
	} else
		strcpy(findpattern, "*");

	if (strcmp(findpattern, "*.*") == 0)
		strcpy(findpattern, "*");
	
	if ((fdir = opendir(findbase)) == NULL)
		return NULL;
	while ((d = readdir(fdir)) != NULL) {
		if (!*findpattern || glob_match(findpattern, d->d_name)) {
//			if (*findpattern)
//				printf("%s matched %s\n", findpattern, d->d_name);
			if (CompareAttributes(findbase, d->d_name, musthave, canhave)) {
				sprintf (findpath, "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}
	return NULL;
}

char *Sys_FindNext (unsigned musthave, unsigned canhave)
{
	struct dirent *d;

	if (fdir == NULL)
		return NULL;
	while ((d = readdir(fdir)) != NULL) {
		if (!*findpattern || glob_match(findpattern, d->d_name)) {
//			if (*findpattern)
//				printf("%s matched %s\n", findpattern, d->d_name);
			if (CompareAttributes(findbase, d->d_name, musthave, canhave)) {
				sprintf (findpath, "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}
	return NULL;
}

void Sys_FindClose (void)
{
	if (fdir != NULL)
		closedir(fdir);
	fdir = NULL;
}

void	Sys_Init (void)
{
	MaskExceptions ();

	Sys_SetFPCW ();

	_go32_interrupt_stack_size = 4 * 1024;;
	_go32_rmcb_stack_size = 4 * 1024;
}

double Sys_FloatTime (void)
{
	return (double) uclock() / (double) UCLOCKS_PER_SEC; // FS: Win9X/Fast PC Fix (QIP)
}

//=============================================================================

int main (int argc, char **argv)
{
	int time, oldtime, newtime;

	Sys_DetectWin95 ();
	Sys_PageInProgram ();
	Sys_GetMemory ();

	Sys_Init();
	Qcommon_Init (argc, argv);
	oldtime = Sys_Milliseconds ();
	dos_registerintr(9, TrapKey);

    /* main window message loop */
	while (1)
	{
		do
		{
			newtime = Sys_Milliseconds ();
			time = newtime - oldtime;
		} while (time < 1);
		Qcommon_Frame (time);
		sys_frame_time = newtime; // FS: Need to update this for input to work properly
		oldtime = newtime;
	}
#if 0
//Con_Printf ("Top of stack: 0x%x\n", &time);
	oldtime = Sys_FloatTime ();
	while (1)
	{
		newtime = Sys_FloatTime ();
		time = newtime - oldtime;
		Qcommon_Frame (time);
		sys_frame_time = newtime; // FS: Need to update this for input to work properly
		oldtime = newtime;
	}
#endif
//	return oldtime; // FS: Compiler warning
}

void Sys_MakeCodeWriteable(void)
{
} //MS-DOS is always writeable
