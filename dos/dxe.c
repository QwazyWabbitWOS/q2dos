#ifndef GAME_HARD_LINKED

#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>
#include <sys/dxe.h>

#include "../qcommon/qcommon.h"

extern	void vectoangles2 (vec3_t value1, vec3_t angles);
extern	vec3_t monster_flash_offset [];

// FS: The following is gross, but I just figured this out.

DXE_EXPORT_TABLE (syms)
	DXE_EXPORT (FS_Gamedir)
	DXE_EXPORT (__dj_assert)
	DXE_EXPORT (__dj_ctype_tolower)
	DXE_EXPORT (__dj_ctype_toupper)
	DXE_EXPORT (__dj_huge_val)
	DXE_EXPORT (__dj_stderr)
	DXE_EXPORT (_doprnt)
	DXE_EXPORT (acos)
	DXE_EXPORT (asin)
	DXE_EXPORT (atan)
	DXE_EXPORT (atan2)
	DXE_EXPORT (atof)
	DXE_EXPORT (atoi)
	DXE_EXPORT (ceil)
	DXE_EXPORT (cos)
	DXE_EXPORT (crand)
	DXE_EXPORT (errno)
	DXE_EXPORT (exit)
	DXE_EXPORT (fclose)
	DXE_EXPORT (feof)
	DXE_EXPORT (fgetc)
	DXE_EXPORT (fgets)
	DXE_EXPORT (floor)
	DXE_EXPORT (fopen)
	DXE_EXPORT (fprintf)
	DXE_EXPORT (fputc)
	DXE_EXPORT (fputs)
	DXE_EXPORT (fread)
	DXE_EXPORT (free)
	DXE_EXPORT (fscanf)
	DXE_EXPORT (fseek)
	DXE_EXPORT (fwrite)
	DXE_EXPORT (getc)
	DXE_EXPORT (localtime)
	DXE_EXPORT (malloc)
	DXE_EXPORT (memcmp)
	DXE_EXPORT (memcpy)
	DXE_EXPORT (memset)
	DXE_EXPORT (monster_flash_offset)
	DXE_EXPORT (printf)
	DXE_EXPORT (putc)
	DXE_EXPORT (puts)
	DXE_EXPORT (qsort)
	DXE_EXPORT (rand)
	DXE_EXPORT (sin)
	DXE_EXPORT (sprintf)
	DXE_EXPORT (sqrt)
	DXE_EXPORT (srand)
	DXE_EXPORT (sscanf)
	DXE_EXPORT (stpcpy)
	DXE_EXPORT (strcasecmp)
	DXE_EXPORT (strcat)
	DXE_EXPORT (strchr)
	DXE_EXPORT (strcmp)
	DXE_EXPORT (strcpy)
	DXE_EXPORT (strdup)
	DXE_EXPORT (strftime)
	DXE_EXPORT (stricmp)
	DXE_EXPORT (strlen)
	DXE_EXPORT (strncmp)
	DXE_EXPORT (strncpy)
	DXE_EXPORT (strnicmp)
	DXE_EXPORT (strrchr)
	DXE_EXPORT (strstr)
	DXE_EXPORT (strtod)
	DXE_EXPORT (strtok)
	DXE_EXPORT (strtol)
	DXE_EXPORT (tan)
	DXE_EXPORT (time)
	DXE_EXPORT (tolower)
	DXE_EXPORT (vectoangles2)
	DXE_EXPORT (vsprintf)
DXE_EXPORT_END

static void (*game_library)(void);

void    Sys_UnloadGame (void)
{
	if (game_library)
	{
		dlclose (game_library);
	}

	game_library = NULL;
}

static int lastresort ()
{
	printf ("last resort function called!\n");
	return 0;
}

void *dxe_res (const char *symname)
{
	printf ("%s: undefined symbol in dynamic module.  Please report this as a bug!\n", symname);
	Com_Printf ("%s: undefined symbol in dynamic module.  Please report this as a bug!\n", symname);

	union
	{
		int (*from)(void);
		void *to;
	} func_ptr_cast;

	func_ptr_cast.from = lastresort;
	return func_ptr_cast.to;
}

void *Sys_GetGameAPI (void *parms)
{
	void	*(*GetGameAPI) (void *);
	char	name[MAX_OSPATH];
	char	curpath[MAX_OSPATH];
	char	*path;
	const char *gamename = "gamex86.dx3";

	getcwd(curpath, sizeof(curpath));

	Com_Printf("------- Loading %s -------\n", gamename);

	  // Set the error callback function
	_dlsymresolver = dxe_res;

	// Register the symbols exported into dynamic modules
	dlregsym (syms);

	// now run through the search paths
	path = NULL;

	while (1)
	{
		path = FS_NextPath (path);

		if (!path)
		{
			return NULL;		// couldn't find one anywhere
		}

		Com_sprintf (name, sizeof(name), "%s/%s/%s", curpath, path, gamename);
		game_library = dlopen (name, RTLD_LAZY);

		if (game_library)
		{
			Com_Printf ("LoadLibrary (%s)\n",name);
			break;
		}
	}

	*(void **) (&GetGameAPI) = dlsym (game_library, "_GetGameAPI");

	if (!GetGameAPI)
	{
		Sys_UnloadGame ();		
		return NULL;
	}

	return GetGameAPI (parms);
}

#endif // GAME_HARD_LINKED