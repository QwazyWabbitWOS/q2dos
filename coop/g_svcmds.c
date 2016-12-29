
#include "g_local.h"


void	Svcmd_Test_f (void)
{
	gi.cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

writeip
Dumps "addip <ip>" commands to listip.cfg so it can be execed at a later date.  The filter lists are not saved and restored by default, because I beleive it would cause too much confusion.

filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

typedef struct
{
	unsigned	mask;
	unsigned	compare;
} ipfilter_t;

#define	MAX_IPFILTERS	1024

ipfilter_t	ipfilters[MAX_IPFILTERS];
int			numipfilters;

/*
=================
StringToFilter
=================
*/
qboolean
StringToFilter(char *s, ipfilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];

	if (!s || !f)
	{
		return false;
	}

	for (i = 0; i < 4; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}

	for (i = 0; i < 4; i++)
	{
		if ((*s < '0') || (*s > '9'))
		{
			gi.cprintf(NULL, PRINT_HIGH, "Bad filter address: %s\n", s);
			return false;
		}

		j = 0;

		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}

		num[j] = 0;
		b[i] = atoi(num);

		if (b[i] != 0)
		{
			m[i] = 255;
		}

		if (!*s)
		{
			break;
		}

		s++;
	}

	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;

	return true;
}

qboolean
SV_FilterPacket(char *from)
{
	int i;
	unsigned in;
	byte m[4];
	char *p;

	if (!from)
	{
		return false;
	}

	i = 0;
	p = from;

	while (*p && i < 4)
	{
		m[i] = 0;

		while (*p >= '0' && *p <= '9')
		{
			m[i] = m[i] * 10 + (*p - '0');
			p++;
		}

		if (!*p || (*p == ':'))
		{
			break;
		}

		i++, p++;
	}

	in = *(unsigned *)m;

	for (i = 0; i < numipfilters; i++)
	{
		if ((in & ipfilters[i].mask) == ipfilters[i].compare)
		{
			return (int)filterban->value;
		}
	}

	return (int)!filterban->value;
}

void
SVCmd_AddIP_f(void)
{
	int i;

	if (gi.argc() < 3)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  addip <ip-mask>\n");
		return;
	}

	for (i = 0; i < numipfilters; i++)
	{
		if (ipfilters[i].compare == 0xffffffff)
		{
			break; /* free spot */
		}
	}

	if (i == numipfilters)
	{
		if (numipfilters == MAX_IPFILTERS)
		{
			gi.cprintf(NULL, PRINT_HIGH, "IP filter list is full\n");
			return;
		}

		numipfilters++;
	}

	if (!StringToFilter(gi.argv(2), &ipfilters[i]))
	{
		ipfilters[i].compare = 0xffffffff;
	}
}

void
SVCmd_RemoveIP_f(void)
{
	ipfilter_t f;
	int i, j;

	if (gi.argc() < 3)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv removeip <ip-mask>\n");
		return;
	}

	if (!StringToFilter(gi.argv(2), &f))
	{
		return;
	}

	for (i = 0; i < numipfilters; i++)
	{
		if ((ipfilters[i].mask == f.mask) &&
			(ipfilters[i].compare == f.compare))
		{
			for (j = i + 1; j < numipfilters; j++)
			{
				ipfilters[j - 1] = ipfilters[j];
			}

			numipfilters--;
			gi.cprintf(NULL, PRINT_HIGH, "Removed.\n");
			return;
		}
	}

	gi.cprintf(NULL, PRINT_HIGH, "Didn't find %s.\n", gi.argv(2));
}

void
SVCmd_ListIP_f(void)
{
	int i;
	byte b[4];

	gi.cprintf(NULL, PRINT_HIGH, "Filter list:\n");

	for (i = 0; i < numipfilters; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		gi.cprintf(NULL, PRINT_HIGH, "%3i.%3i.%3i.%3i\n", b[0], b[1], b[2], b[3]);
	}
}

void
SVCmd_WriteIP_f(void)
{
	FILE *f;
	char name[MAX_OSPATH];
	byte b[4];
	int i;
	cvar_t *game;

	game = gi.cvar("game", "", 0);

	if (!*game->string)
	{
		sprintf(name, "%s/listip.cfg", GAMEVERSION);
	}
	else
	{
		sprintf(name, "%s/listip.cfg", game->string);
	}

	gi.cprintf(NULL, PRINT_HIGH, "Writing %s.\n", name);

	f = fopen(name, "wb");

	if (!f)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Couldn't open %s\n", name);
		return;
	}

	fprintf(f, "set filterban %d\n", (int)filterban->value);

	for (i = 0; i < numipfilters; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		fprintf(f, "sv addip %i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
	}

	fclose(f);
}

void SVCmd_SayPerson_f (void)
{
	char msg[2000]; /* FS: Q2Admin magic limit */
	char *p = NULL;
	edict_t	*client;
	int playernum;
	size_t messageLen;

	memset(msg, 0, sizeof(msg));

	if (gi.argc() < 5)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv !say_person CL <playernum> <message>\n");
		return;
	}

	playernum = atoi(gi.argv(3));

	if(playernum > game.maxclients)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Didn't find %d.\n", playernum);
		return;
	}

	client = &g_edicts[playernum + 1];

	if (!client || !client->inuse || !client->client || !client->client->pers.connected)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Didn't find %d.\n", playernum);
		return;
	}

	Com_sprintf(msg, sizeof(msg), "!say_person CL %s ", gi.argv(3));
	messageLen = strlen(msg);
	p = gi.args() + messageLen;
	if(!p)
	{
		return;
	}

	Com_sprintf (msg, sizeof(msg), "(private message) ");
	Com_strcat (msg, sizeof(msg), p);

	/* don't let text be too long for malicious reasons */
	if (strlen(msg) > 150)
	{
		msg[150] = 0;
	}

	Com_strcat (msg, sizeof(msg), "\n");

	gi.cprintf(client, PRINT_CHAT, "%s", msg);
}

void SVCmd_StuffCmd_f (void)
{
	char cmd[2000]; /* FS: Q2Admin magic limit */
	char *p = NULL;
	edict_t	*client = NULL;
	int playernum;
	size_t messageLen;

	memset(cmd, 0, sizeof(cmd));

	if (gi.argc() < 5)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv !stuff CL <playernum> <cmd>\n");
		return;
	}

	playernum = atoi(gi.argv(3));

	if(playernum > game.maxclients)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Didn't find %d.\n", playernum);
		return;
	}

	client = &g_edicts[playernum + 1];

	if (!client || !client->inuse || !client->client || !client->client->pers.connected)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Didn't find %d.\n", playernum);
		return;
	}

	Com_sprintf(cmd, sizeof(cmd), "!stuff CL %s ", gi.argv(3));
	messageLen = strlen(cmd);
	p = gi.args() + messageLen;
	if(!p)
	{
		return;
	}

	Com_sprintf (cmd, sizeof(cmd), "%s\n", p);

	gi.WriteByte(svc_stufftext);
	gi.WriteString(cmd);
	gi.unicast(client, true);
}

/*
 * ServerCommand will be called when an "sv" command is issued.
 * The game can issue gi.argc() / gi.argv() commands to get the
 * rest of the parameters
 */
void
ServerCommand(void)
{
	char *cmd;

	cmd = gi.argv(1);

	if (Q_stricmp(cmd, "test") == 0)
	{
		Svcmd_Test_f();
	}
	else if (Q_stricmp(cmd, "addip") == 0)
	{
		SVCmd_AddIP_f();
	}
	else if (Q_stricmp(cmd, "removeip") == 0)
	{
		SVCmd_RemoveIP_f();
	}
	else if (Q_stricmp(cmd, "listip") == 0)
	{
		SVCmd_ListIP_f();
	}
	else if (Q_stricmp(cmd, "writeip") == 0)
	{
		SVCmd_WriteIP_f();
	}
	else if (Q_stricmp(cmd, "!say_person") == 0) /* FS: Tastyspleen q2admin additions */
	{
		SVCmd_SayPerson_f();
	}
	else if (Q_stricmp(cmd, "!stuff") == 0) /* FS: Tastyspleen q2admin additions */
	{
		SVCmd_StuffCmd_f();
	}
	else
	{
		gi.cprintf(NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
	}
}
