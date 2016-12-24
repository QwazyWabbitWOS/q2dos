/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * Coop specific code.
 *
 * =======================================================================
 */

#include "g_local.h"

#define COOP_VERSION			0.01a
#define COOP_VSTRING2(x) #x
#define COOP_VSTRING(x) COOP_VSTRING2(x)
#define COOP_STRING_VERSION  COOP_VSTRING(COOP_VERSION)

void CoopOpenJoinMenu(edict_t *ent);
void CoopOpenVoteMenu(edict_t *ent);
void CoopReturnToMain(edict_t *ent, pmenuhnd_t *p);
void CoopReturnToVoteMenu(edict_t *ent, pmenuhnd_t *p);
void CoopCredits(edict_t *ent, pmenuhnd_t *p);
void CoopChaseCam(edict_t *ent, pmenuhnd_t *p);
void CoopJoinGame(edict_t *ent, pmenuhnd_t *p);
void CoopMotd(edict_t *ent, pmenuhnd_t *p);

void CoopVoteMenu(edict_t *ent, pmenuhnd_t *p);
void CoopVoteGamemode(edict_t *ent, pmenuhnd_t *p);
void CoopVoteDifficulty(edict_t *ent, pmenuhnd_t *p);
void CoopVoteMap(edict_t *ent, pmenuhnd_t *p);
void CoopVoteRestartMap(edict_t *ent, pmenuhnd_t *p);
void CoopCheckGamemode(edict_t *ent, pmenuhnd_t *p);
void CoopCheckDifficulty(edict_t *ent, pmenuhnd_t *p);
void votemenu_loadmaplist (void);
void CoopVoteChangeMap(edict_t *ent, pmenuhnd_t *p);
void votemenu_cleanup_all (void);
void votemenu_cleanup_filebuffer (void);

/*-----------------------------------------------------------------------*/

pmenu_t creditsmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Programming", PMENU_ALIGN_CENTER, NULL},
	{"'[HCI]Mara'akate'", PMENU_ALIGN_CENTER, NULL},
	{"'Freewill'", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

static const int jmenu_level = 4;
static const int jmenu_gamemode = 5;
static const int jmenu_skill = 7;
static const int jmenu_players = 8;
static const int jmenu_spectators = 9;
static const int jmenu_motd = 14;

pmenu_t joinmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL}, /* 10 */
	{"Join The Game", PMENU_ALIGN_LEFT, CoopJoinGame},
	{"Become a Spectator", PMENU_ALIGN_LEFT, CoopChaseCam},
	{"Credits", PMENU_ALIGN_LEFT, CoopCredits},
	{"Message of The Day", PMENU_ALIGN_LEFT, CoopMotd},
	{"Voting Menu", PMENU_ALIGN_LEFT, CoopVoteMenu},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 16 */
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{"v" COOP_STRING_VERSION, PMENU_ALIGN_RIGHT, NULL},
};

pmenu_t nochasemenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"No one to chase", PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

pmenu_t votemenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Gamemode", PMENU_ALIGN_LEFT, CoopVoteGamemode},
	{"Difficulty", PMENU_ALIGN_LEFT, CoopVoteDifficulty},
	{"Change Map", PMENU_ALIGN_LEFT, CoopVoteMap},
	{"Restart Map", PMENU_ALIGN_LEFT, CoopVoteRestartMap},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

#define VGAMEMODEMENU_GAMEMODE 6
#define VGAMEMODEMENU_VANILLA 8
#define VGAMEMODEMENU_XATRIX 9
#define VGAMEMODEMENU_ROGUE 10

pmenu_t votegamemodemenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Gamemode", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Current Gamemode: ", PMENU_ALIGN_CENTER, NULL}, /* 6 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Vanilla", PMENU_ALIGN_LEFT, CoopCheckGamemode}, /* 8 */
	{"Xatrix Mission Pack", PMENU_ALIGN_LEFT, CoopCheckGamemode},
	{"Rogue Mission Pack", PMENU_ALIGN_LEFT, CoopCheckGamemode}, /* 10 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Voting Menu", PMENU_ALIGN_LEFT, CoopReturnToVoteMenu}
};

#define VSKILLMENU_SKILL 6
#define VSKILLMENU_EASY 8
#define VSKILLMENU_MEDIUM 9
#define VSKILLMENU_HARD 10
#define VSKILLMENU_NIGHTMARE 11

pmenu_t voteskillmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Difficulty", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Current Skill: ", PMENU_ALIGN_CENTER, NULL}, /* 6 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Easy", PMENU_ALIGN_LEFT, CoopCheckDifficulty}, /* 8 */
	{"Medium", PMENU_ALIGN_LEFT, CoopCheckDifficulty},
	{"Hard", PMENU_ALIGN_LEFT, CoopCheckDifficulty},
	{"Nightmare", PMENU_ALIGN_LEFT, CoopCheckDifficulty}, /* 11 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Voting Menu", PMENU_ALIGN_LEFT, CoopReturnToVoteMenu}
};

#define MOTDMENU_START 4
#define MOTDMENU_END 16
#define MOTDMENU_MAXLINES MOTDMENU_END-MOTDMENU_START
pmenu_t motdmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL}, /* 16 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

char *coopMapFileBuffer = NULL;
int mapCount = 0;
pmenu_t *votemapmenu = NULL;

char *GetSkillString (void)
{
	switch(skill->intValue)
	{
	case 0:
		return "Easy";
	case 1:
		return "Medium";
	case 2:
		return "Hard";
	case 3:
		return "Nightmare";
	}

	return "Unknown";
}

void
CoopReturnToMain(edict_t *ent, pmenuhnd_t *p)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	CoopOpenJoinMenu(ent);
}

void
CoopReturnToVoteMenu(edict_t *ent, pmenuhnd_t *p)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	CoopOpenVoteMenu(ent);
}

static void
SetLevelName(pmenu_t *p)
{
	static char levelname[33];

	if (!p)
	{
		return;
	}

	levelname[0] = '*';

	if (g_edicts[0].message)
	{
		strncpy(levelname + 1, g_edicts[0].message, sizeof(levelname) - 2);
	}
	else
	{
		strncpy(levelname + 1, level.mapname, sizeof(levelname) - 2);
	}

	levelname[sizeof(levelname) - 1] = 0;
	p->text = levelname;
}

void
CoopUpdateJoinMenu(edict_t *ent)
{
	static char players[32];
	static char spectators[32];
	static char stats[32];
	static char gamemode[32];
	int numplayers, numspectators, i;

	if(!ent || !ent->client)
	{
		return;
	}

	SetLevelName(joinmenu + jmenu_level);

	numplayers = numspectators = 0;

	for (i = 0; i < maxclients->value; i++)
	{
		if (!g_edicts[i + 1].inuse)
		{
			continue;
		}

		if (game.clients[i].resp.spectator)
		{
			numspectators++;
		}
		else
		{
			numplayers++;
		}
	}

	Com_sprintf(gamemode, sizeof(gamemode), "*Gamemode: %s", sv_coop_gamemode->string);
	Com_sprintf(players, sizeof(players), "  (%d players)", numplayers);
	Com_sprintf(spectators, sizeof(spectators), "  (%d spectators)", numspectators);
	Com_sprintf(stats, sizeof(stats), "Difficulty: %s", GetSkillString());

	joinmenu[jmenu_gamemode].text = gamemode;
	joinmenu[jmenu_players].text = players;
	joinmenu[jmenu_spectators].text = spectators;
	joinmenu[jmenu_skill].text = stats;

	if(!motd || !motd->string[0])
	{
		joinmenu[jmenu_motd].text = NULL;
		joinmenu[jmenu_motd].SelectFunc = NULL;
	}

	return;
}

void
CoopOpenJoinMenu(edict_t *ent)
{
	if(!ent || !ent->client)
	{
		return;
	}

	ent->client->pers.didMotd = ent->client->resp.didMotd = true;
	CoopUpdateJoinMenu(ent);
	PMenu_Open(ent, joinmenu, 0, sizeof(joinmenu) / sizeof(pmenu_t), NULL, PMENU_NORMAL);
}

void
CoopOpenVoteMenu(edict_t *ent)
{
	if(!ent || !ent->client)
	{
		return;
	}

	ent->client->pers.didMotd = ent->client->resp.didMotd = true;
	PMenu_Open(ent, votemenu, 0, sizeof(votemenu) / sizeof(pmenu_t), NULL, PMENU_NORMAL);
}

void
CoopCredits(edict_t *ent, pmenuhnd_t *p)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu, -1,
			sizeof(creditsmenu) / sizeof(pmenu_t),
			NULL, PMENU_NORMAL);
}

void CoopChaseCam(edict_t *ent, pmenuhnd_t *p)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	gi.WriteByte (svc_stufftext);
	gi.WriteString ("spectator 1\n");
	gi.unicast(ent, true);
}

void CoopJoinGame(edict_t *ent, pmenuhnd_t *p)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	gi.WriteByte (svc_stufftext);
	gi.WriteString ("spectator 0\n");
	gi.unicast(ent, true);
}

void CoopUpdateMotdMenu (void)
{
	char separators[] = "|\n";
	char *motdString;
	char *listPtr = NULL;
	char *motdToken = NULL;
	int lineLength = 0;
	int currentLine = 0;
	int i = MOTDMENU_START;

	if(!motd || !motd->string || !motd->string[0]) /* FS: TODO: Read this from a file instead of a CVAR because CVAR seems to limit at 256 chars.  Max we could possibly squeeze is about 324 chars. */
	{
		return;
	}

	motdString = strdup(motd->string);

	motdToken = strtok_r(motdString, separators, &listPtr);
	if(!motdToken)
	{
		return;
	}

	for (i = MOTDMENU_START; i < MOTDMENU_MAXLINES; i++) /* FS: Clear it out first in case if we change it later */
	{
		motdmenu[i].text = NULL;
	}

	i = MOTDMENU_START;

	while(motdToken)
	{
		if(i >= MOTDMENU_MAXLINES)
		{
			break;
		}

		currentLine++;
		lineLength = strlen(motdToken);
		motdmenu[i].text = motdToken;
		if(lineLength > 27)
		{
			gi.cprintf(NULL, PRINT_CHAT, "Warning: MOTD string on line %i greater than 27 chars, current length is %i.  Truncating.\n", currentLine, lineLength);
			motdmenu[i].text[27] = 0;
		}
		motdToken = strtok_r(NULL, separators, &listPtr);
		i++;
	}
}

void CoopMotd(edict_t *ent, pmenuhnd_t *p)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	CoopUpdateMotdMenu();
	PMenu_Open(ent, motdmenu, 0, sizeof(motdmenu) / sizeof(pmenu_t), NULL, PMENU_NORMAL);
}

void CoopVoteMenu(edict_t *ent, pmenuhnd_t *p)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	PMenu_Open(ent, votemenu, 0, sizeof(votemenu) / sizeof(pmenu_t), NULL, PMENU_NORMAL);
}

void CoopUpdateGamemodeMenu(edict_t *ent)
{
	static char gamemodestring[32];
	static char gamemode[32];

	if(!ent || !ent->client)
	{
		return;
	}

	switch(game.gametype)
	{
		case vanilla_coop:
			Com_sprintf(gamemode, sizeof(gamemode), "Vanilla");
			votegamemodemenu[VGAMEMODEMENU_VANILLA].text = NULL;
			votegamemodemenu[VGAMEMODEMENU_VANILLA].SelectFunc = NULL;
			break;
		case xatrix_coop:
			Com_sprintf(gamemode, sizeof(gamemode), "Xatrix");
			votegamemodemenu[VGAMEMODEMENU_XATRIX].text = NULL;
			votegamemodemenu[VGAMEMODEMENU_XATRIX].SelectFunc = NULL;
			break;
		case rogue_coop:
			Com_sprintf(gamemode, sizeof(gamemode), "Rogue");
			votegamemodemenu[VGAMEMODEMENU_ROGUE].text = NULL;
			votegamemodemenu[VGAMEMODEMENU_ROGUE].SelectFunc = NULL;
			break;
		default:
			break;
	}

	Com_sprintf(gamemodestring, sizeof(gamemodestring), "*Current Gamemode: %s", gamemode);

	votegamemodemenu[VGAMEMODEMENU_GAMEMODE].text = gamemodestring;
}

void CoopVoteGamemode(edict_t *ent, pmenuhnd_t *p)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	CoopUpdateGamemodeMenu(ent);
	PMenu_Open(ent, votegamemodemenu, 0, sizeof(votegamemodemenu) / sizeof(pmenu_t), NULL, PMENU_NORMAL);
}

void CoopUpdateDifficultyMenu(edict_t *ent)
{
	static char skillstring[32];
	static char skillmodifier[32];

	if(!ent || !ent->client)
	{
		return;
	}

	switch(skill->intValue)
	{
		case 0:
			voteskillmenu[VSKILLMENU_EASY].text = NULL;
			voteskillmenu[VSKILLMENU_EASY].SelectFunc = NULL;
			break;
		case 1:
			voteskillmenu[VSKILLMENU_MEDIUM].text = NULL;
			voteskillmenu[VSKILLMENU_MEDIUM].SelectFunc = NULL;
			break;
		case 2:
			voteskillmenu[VSKILLMENU_HARD].text = NULL;
			voteskillmenu[VSKILLMENU_HARD].SelectFunc = NULL;
			break;
		case 3:
			voteskillmenu[VSKILLMENU_NIGHTMARE].text = NULL;
			voteskillmenu[VSKILLMENU_NIGHTMARE].SelectFunc = NULL;
			break;
		default:
			break;
	}

	Com_sprintf(skillmodifier, sizeof(skillmodifier), "%s", GetSkillString());
	Com_sprintf(skillstring, sizeof(skillstring), "*Current Skill: %s", skillmodifier);

	voteskillmenu[VSKILLMENU_SKILL].text = skillstring;
}

void CoopVoteDifficulty(edict_t *ent, pmenuhnd_t *p)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	CoopUpdateDifficultyMenu(ent);
	PMenu_Open(ent, voteskillmenu, 0, sizeof(voteskillmenu) / sizeof(pmenu_t), NULL, PMENU_NORMAL);
}

void CoopUpdateVoteMapMenu(edict_t *ent)
{
	int i = 0;
	char *mapToken = NULL;
	char *coopMapTokenBuffer = NULL;
	char *listPtr = NULL;
	char separators[] = "\n";

	if(!ent || !ent->client || !coopMapFileBuffer)
	{
		return;
	}

	coopMapTokenBuffer = strdup(coopMapFileBuffer);

	mapToken = strtok_r(coopMapTokenBuffer, separators, &listPtr);
	if(!mapToken)
	{
		if(coopMapTokenBuffer)
		{
			free(coopMapTokenBuffer);
		}

		return;
	}

	while(mapToken)
	{
		votemapmenu[i].text = strdup(mapToken);
		votemapmenu[i].align = PMENU_ALIGN_LEFT;
		votemapmenu[i].SelectFunc = CoopVoteChangeMap;
		mapToken = strtok_r(NULL, separators, &listPtr);
		i++;
	}

	if(coopMapTokenBuffer)
	{
		free(coopMapTokenBuffer);
	}
}

void CoopVoteMap(edict_t *ent, pmenuhnd_t *p)
{
	size_t size;

	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	votemenu_loadmaplist();
	size = sizeof(pmenu_t) * (mapCount + 2);
	votemapmenu = malloc(size);
	memset((pmenu_t *)votemapmenu, 0, size);
	CoopUpdateVoteMapMenu(ent);

	votemapmenu[mapCount].text = NULL;
	votemapmenu[mapCount].align = PMENU_ALIGN_LEFT;
	votemapmenu[mapCount].SelectFunc = NULL;

	votemapmenu[mapCount+1].text = "Return to Voting Menu";
	votemapmenu[mapCount+1].align = PMENU_ALIGN_LEFT;
	votemapmenu[mapCount+1].SelectFunc = CoopReturnToVoteMenu;

	PMenu_Open(ent, votemapmenu, 0, mapCount + 2, NULL, PMENU_SCROLLING);
	votemenu_cleanup_all();
}

void CoopVoteRestartMap(edict_t *ent, pmenuhnd_t *p)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	gi.WriteByte (svc_stufftext);
	gi.WriteString ("cmd vote restartmap\n");
	gi.unicast(ent, true);
}

void CoopVoteChangeMap(edict_t *ent, pmenuhnd_t *p)
{
	char mapVote[64];
	char votestring[64];

	if(!ent || !ent->client)
	{
		return;
	}

	Com_sprintf(mapVote, sizeof(mapVote), "%s", p->entries[p->cur].text);
	COM_StripExtension (mapVote, mapVote);
	Com_sprintf(votestring, sizeof(votestring), "cmd vote map %s\n", mapVote);

	PMenu_Close(ent);

	gi.WriteByte (svc_stufftext);
	gi.WriteString (votestring);
	gi.unicast(ent, true);
}

void CoopCheckGamemode(edict_t *ent, pmenuhnd_t *p)
{
	char votestring[64];

	if(!ent || !ent->client)
	{
		return;
	}

	switch(p->cur)
	{
		case VGAMEMODEMENU_VANILLA:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote gamemode vanilla\n");
			break;
		case VGAMEMODEMENU_XATRIX:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote gamemode xatrix\n");
			break;
		case VGAMEMODEMENU_ROGUE:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote gamemode rogue\n");
			break;
		default:
			break;
	}
	PMenu_Close(ent);

	if(votestring[0])
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString(votestring);
		gi.unicast(ent, true);
	}
}

void CoopCheckDifficulty(edict_t *ent, pmenuhnd_t *p)
{
	char votestring[64];

	if(!ent || !ent->client)
	{
		return;
	}

	switch(p->cur)
	{
		case VSKILLMENU_EASY:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote skill 0\n");
			break;
		case VSKILLMENU_MEDIUM:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote skill 1\n");
			break;
		case VSKILLMENU_HARD:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote skill 2\n");
			break;
		case VSKILLMENU_NIGHTMARE:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote skill 3\n");
			break;
		default:
			break;
	}
	PMenu_Close(ent);

	if(votestring[0])
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString(votestring);
		gi.unicast(ent, true);
	}
}

void votemenu_cleanup_all (void)
{
	int i;

	for(i = 0; i < mapCount; i++)
	{
		if(votemapmenu[i].text)
		{
			free(votemapmenu[i].text);
		}
	}

	free(votemapmenu);
	votemenu_cleanup_filebuffer();
}

void votemenu_cleanup_filebuffer (void)
{
	if(coopMapFileBuffer)
	{
		free(coopMapFileBuffer);
		coopMapFileBuffer = NULL;
	}
}

void votemenu_loadmaplist (void)
{
	char fileName[MAX_OSPATH];
	char *mapToken = NULL;
	char *coopMapTokenBuffer = NULL;
	char *listPtr = NULL;
	char separators[] = "\n";
	long fileSize;
	FILE *f = NULL;
	size_t toEOF = 0;

	if(sv_coop_maplist->string[0] == 0)
	{
		gi.cprintf(NULL, PRINT_CHAT, "votemenu_loadmaplist: sv_coop_maplist CVAR empty!\n");
		return;
	}

	Sys_Mkdir(va("%s/maps", gamedir->string));
	Com_sprintf(fileName, sizeof(fileName), "%s/%s", gamedir->string, sv_coop_maplist->string);

	f = fopen(fileName, "r");
	if(!f)
	{
		gi.cprintf(NULL, PRINT_CHAT, "votemenu_loadmaplist: couldn't find '%s'!\n", sv_coop_maplist->string);
		return;
	}

	/* obtain file size */
	fseek (f, 0, SEEK_END);
	fileSize = ftell (f);
	fseek (f, 0, SEEK_SET);

	votemenu_cleanup_filebuffer();

	coopMapFileBuffer = (char *)malloc(sizeof(char)*(fileSize+2)); // FS: In case we have to add a newline terminator
	if(!coopMapFileBuffer)
	{
		gi.cprintf(NULL, PRINT_CHAT, "votemenu_loadmaplist: can't allocate memory for coopMapFileBuffer!\n");
		fclose(f);
		return;
	}
	toEOF = fread(coopMapFileBuffer, sizeof(char), fileSize, f);
	fclose(f);
	if(toEOF <= 0)
	{
		gi.cprintf(NULL, PRINT_CHAT, "votemenu_loadmaplist: cannot read file '%s' into memory!\n", sv_coop_maplist->string);
		votemenu_cleanup_filebuffer();
		return;
	}

	/* FS: Add newline terminator for some paranoia */
	coopMapFileBuffer[toEOF] = '\n';
	coopMapFileBuffer[toEOF+1] = '\0';

	coopMapTokenBuffer = strdup(coopMapFileBuffer);

	mapToken = strtok_r(coopMapTokenBuffer, separators, &listPtr);
	if(!mapToken)
	{
		if(coopMapTokenBuffer)
		{
			free(coopMapTokenBuffer);
		}
		return;
	}

	mapCount = 0;
	while(mapToken)
	{
		mapCount++;
		mapToken = strtok_r(NULL, separators, &listPtr);
	}

	if(coopMapTokenBuffer)
	{
		free(coopMapTokenBuffer);
	}

	return;
}