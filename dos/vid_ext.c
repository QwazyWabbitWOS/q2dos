/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// extended video modes: VESA-specific DOS video stuff

#include <stdio.h>
#include <dpmi.h>

#include "../client/client.h"
#include "vid_dos.h"
#include "dosisms.h"

static int	totalvidmem;

vid_resolutions_t vid_resolutions[MAX_RESOLUTIONS];
int num_vid_resolutions = 0;

#define MODE_SUPPORTED_IN_HW		0x0001
#define COLOR_MODE			0x0008
#define GRAPHICS_MODE			0x0010
#define VGA_INCOMPATIBLE		0x0020
#define LINEAR_FRAME_BUFFER		0x0080
#define LINEAR_MODE			0x4000

// !!! if this is changed, MAX_HEIGHT and MAX_WIDTH must be changed in d_ifacea.h too !!!
// !!! if this is changed, MAX_HEIGHT and MAX_WIDTH must be changed in r_local.h too !!!
#define MAXVESAWIDTH		4000 /* FS: Was 1600 */
#define MAXVESAHEIGHT		4000 /* FS: Was 1200 */

int		VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes;
byte	*VGA_pagebase;

typedef struct
{
	int	modenum;
	int	mode_attributes;
	int	winasegment;
	int	winbsegment;
	int	bytes_per_scanline; // bytes per logical scanline (+16)
	int	win;		// window number (A=0, B=1)
	int	win_size;	// window size (+6)
	int	granularity;	// how finely i can set the window in vid mem (+4)
	int	width, height;	// displayed width and height (+18, +20)
	int	bits_per_pixel;	// er, better be 8, 15, 16, 24, or 32 (+25)
	int	bytes_per_pixel; // er, better be 1, 2, or 4
	int	memory_model;	// and better be 4 or 6, packed or direct color (+27)
	int	num_pages;	// number of complete frame buffer pages (+29)
	int	red_width;	// the # of bits in the red component (+31)
	int	red_pos;	// the bit position of the red component (+32)
	int	green_width;	// etc.. (+33)
	int	green_pos;	// (+34)
	int	blue_width;	// (+35)
	int	blue_pos;	// (+36)
	int	pptr;
	int	pagesize;
	int	numpages;
} modeinfo_t;

static modeinfo_t modeinfo;

// all bytes to avoid problems with compiler field packing
typedef struct vbeinfoblock_s {
	byte	VbeSignature[4];
	byte	VbeVersion[2];
	byte	OemStringPtr[4];
	byte	Capabilities[4];
	byte	VideoModePtr[4];
	byte	TotalMemory[2];
	byte	OemSoftwareRev[2];
	byte	OemVendorNamePtr[4];
	byte	OemProductNamePtr[4];
	byte	OemProductRevPtr[4];
	byte	Reserved[222];
	byte	OemData[256];
} vbeinfoblock_t;

static qboolean VID_ExtraGetModeInfo(int modenum);
/* FS: New stuff for planar/banked modes */
static void VID_AddPlanarModes(void);
static void VID_AddBankedModes(void);

/*
================
VID_ExtraFarToLinear
================
*/
static void *VID_ExtraFarToLinear (unsigned long addr)
{
	return real2ptr(((addr & 0xFFFF0000) >> 12) + (addr & 0xFFFF));
}

/*
================
VID_InitExtra
================
*/
void VID_InitExtra (void)
{
	int		nummodes;
	short		*pmodenums;
	vbeinfoblock_t	*pinfoblock;
	unsigned long	addr;

	pinfoblock = (vbeinfoblock_t *) dos_getmemory(sizeof(vbeinfoblock_t));
	if (!pinfoblock) {
		Com_DPrintf (DEVELOPER_MSG_GFX, "VID_InitExtra: Unable to allocate low memory.\n");
		return;
	}

	pinfoblock->VbeSignature[0] = 'V';
	pinfoblock->VbeSignature[1] = 'B';
	pinfoblock->VbeSignature[2] = 'E';
	pinfoblock->VbeSignature[3] = '2';

	memset(vid_resolutions,0x0,sizeof(vid_resolutions));

	VID_AddPlanarModes(); /* FS: Add 320x240 Mode-X first */

	/* We always have mode 13 VGA */
	vid_resolutions[num_vid_resolutions].mode=num_vid_resolutions;
	vid_resolutions[num_vid_resolutions].vesa_mode=-1;
	vid_resolutions[num_vid_resolutions].height=200;
	vid_resolutions[num_vid_resolutions].width=320;
	vid_resolutions[num_vid_resolutions].isLFB=false;
	vid_resolutions[num_vid_resolutions].isBanked=false;

	Com_sprintf(vid_resolutions[num_vid_resolutions].menuname, sizeof(vid_resolutions[num_vid_resolutions].menuname), "[VGA 320x200]");
	num_vid_resolutions++;

	if(COM_CheckParm("-vgaonly"))
	{
		VID_AddBankedModes(); /* FS: Added */
		return;	//test for VGA only
	}

// see if VESA support is available
	regs.x.ax = 0x4f00;
	regs.x.es = ptr2real(pinfoblock) >> 4;
	regs.x.di = ptr2real(pinfoblock) & 0xf;
	dos_int86(0x10);

	if (regs.x.ax != 0x4f)
	{
		dos_freememory(pinfoblock);
		return;		// no VESA support
	}

	if (pinfoblock->VbeVersion[1] < 0x02)
	{
		dos_freememory(pinfoblock);
		return;		// not VESA 2.0 or greater
	}

	addr = ( (pinfoblock->OemStringPtr[0]      ) |
		 (pinfoblock->OemStringPtr[1] <<  8) |
		 (pinfoblock->OemStringPtr[2] << 16) |
		 (pinfoblock->OemStringPtr[3] << 24));
	Com_Printf ("VESA 2.0 compliant adapter:\n%s\n",
			(char *) VID_ExtraFarToLinear(addr));

	totalvidmem  = ( (pinfoblock->TotalMemory[0]     ) |
			 (pinfoblock->TotalMemory[1] << 8) ) << 16;
//	Com_Printf ("%dk video memory\n", totalvidmem >> 10);

	addr = ( (pinfoblock->VideoModePtr[0]      ) |
		 (pinfoblock->VideoModePtr[1] <<  8) |
		 (pinfoblock->VideoModePtr[2] << 16) |
		 (pinfoblock->VideoModePtr[3] << 24));
	pmodenums = (short *) VID_ExtraFarToLinear(addr);

// find 8 bit modes until we either run out of space or run out of modes
	nummodes = 0;

	while ((*pmodenums != -1) && (nummodes < MAX_RESOLUTIONS))
	{
		if (VID_ExtraGetModeInfo (*pmodenums))
		{
			nummodes++;
		}
		pmodenums++;
	}

	dos_freememory(pinfoblock);
	VID_AddBankedModes(); /* FS: Add banked modes at the end of the video list. */
}

static void VID_AddPlanarModes(void)
{
	vid_resolutions[num_vid_resolutions].mode=num_vid_resolutions;
	vid_resolutions[num_vid_resolutions].vesa_mode=-1;
	vid_resolutions[num_vid_resolutions].height=240;
	vid_resolutions[num_vid_resolutions].planarAddress = (void *)real2ptr(0xA0000);
	vid_resolutions[num_vid_resolutions].width=320;
	vid_resolutions[num_vid_resolutions].isLFB=false;
	vid_resolutions[num_vid_resolutions].isBanked=false;
	vid_resolutions[num_vid_resolutions].isPlanar=true;

	Com_sprintf(vid_resolutions[num_vid_resolutions].menuname, sizeof(vid_resolutions[num_vid_resolutions].menuname), "[VGA-X 320x240]");
	num_vid_resolutions++;
}

static void VID_AddBankedModes(void)
{
	vid_resolutions[num_vid_resolutions].mode=num_vid_resolutions;
	vid_resolutions[num_vid_resolutions].vesa_mode=0x0101;
	vid_resolutions[num_vid_resolutions].height=480;
	vid_resolutions[num_vid_resolutions].width=640;
	vid_resolutions[num_vid_resolutions].isLFB=false;
	vid_resolutions[num_vid_resolutions].isBanked=true;

	Com_sprintf(vid_resolutions[num_vid_resolutions].menuname, sizeof(vid_resolutions[num_vid_resolutions].menuname), "[VGA-B 640x480]");
	num_vid_resolutions++;

	vid_resolutions[num_vid_resolutions].mode=num_vid_resolutions;
	vid_resolutions[num_vid_resolutions].vesa_mode=0x0103;
	vid_resolutions[num_vid_resolutions].height=600;
	vid_resolutions[num_vid_resolutions].width = 800;
	vid_resolutions[num_vid_resolutions].isLFB=false;
	vid_resolutions[num_vid_resolutions].isBanked=true;

	Com_sprintf(vid_resolutions[num_vid_resolutions].menuname, sizeof(vid_resolutions[num_vid_resolutions].menuname), "[VGA-B 800x600]");
	num_vid_resolutions++;

	vid_resolutions[num_vid_resolutions].mode=num_vid_resolutions;
	vid_resolutions[num_vid_resolutions].vesa_mode=0x0105;
	vid_resolutions[num_vid_resolutions].height=768;
	vid_resolutions[num_vid_resolutions].width = 1024;
	vid_resolutions[num_vid_resolutions].isLFB=false;
	vid_resolutions[num_vid_resolutions].isBanked=true;

	Com_sprintf(vid_resolutions[num_vid_resolutions].menuname, sizeof(vid_resolutions[num_vid_resolutions].menuname), "[VGA-B 1024x768]");
	num_vid_resolutions++;
}

/*
================
VID_ExtraGetModeInfo
================
*/
static qboolean VID_ExtraGetModeInfo(int modenum)
{
	char	*infobuf;
	int		numimagepages;
	__dpmi_meminfo	phys_mem_info;

	infobuf = dos_getmemory(256);

	regs.x.ax = 0x4f01;
	regs.x.cx = modenum;
	regs.x.es = ptr2real(infobuf) >> 4;
	regs.x.di = ptr2real(infobuf) & 0xf;
	dos_int86(0x10);

	if (regs.x.ax != 0x4f)
	{
		dos_freememory(infobuf);
		return false;
	}
	else
	{
		modeinfo.modenum = modenum;
		modeinfo.bits_per_pixel = *(char*)(infobuf+25);
		modeinfo.bytes_per_pixel = (modeinfo.bits_per_pixel+1)/8;
		modeinfo.width = *(short*)(infobuf+18);
		modeinfo.height = *(short*)(infobuf+20);

	// we do only 8-bpp in software
		if ((modeinfo.bits_per_pixel != 8) ||
			(modeinfo.bytes_per_pixel != 1) ||
			(modeinfo.width > MAXVESAWIDTH) ||
			(modeinfo.height > MAXVESAHEIGHT))
		{
			dos_freememory(infobuf);
			return false;
		}

		modeinfo.mode_attributes = *(short*)infobuf;

	// we only want color graphics modes that are supported by the hardware
		if ((modeinfo.mode_attributes &
			 (MODE_SUPPORTED_IN_HW | COLOR_MODE | GRAPHICS_MODE)) !=
			(MODE_SUPPORTED_IN_HW | COLOR_MODE | GRAPHICS_MODE))
		{
			dos_freememory(infobuf);
			return false;
		}

	// we only work with linear frame buffers, except for 320x200, which can
	// effectively be linear when banked at 0xA000
		if (!(modeinfo.mode_attributes & LINEAR_FRAME_BUFFER))
		{
			if ((modeinfo.width != 320) || (modeinfo.height != 200))
			{
				dos_freememory(infobuf);
				return false;
			}
		}

		modeinfo.bytes_per_scanline = *(short*)(infobuf+16);

		modeinfo.pagesize = modeinfo.bytes_per_scanline * modeinfo.height;

		if (modeinfo.pagesize > totalvidmem)
		{
			dos_freememory(infobuf);
			return false;
		}

	// force to one page if the adapter reports it doesn't support more pages
	// than that, no matter how much memory it has--it may not have hardware
	// support for page flipping
		numimagepages = *(unsigned char *)(infobuf+29);

		if (numimagepages <= 0)
		{
		// wrong, but there seems to be an ATI VESA driver that reports 0
			modeinfo.numpages = 1;
		}
		else if (numimagepages < 3)
		{
			modeinfo.numpages = numimagepages;
		}
		else
		{
			modeinfo.numpages = 3;
		}

		if (*(char*)(infobuf+2) & 5)
		{
			modeinfo.winasegment = *(unsigned short*)(infobuf+8);
			modeinfo.win = 0;
		}
		else if (*(char*)(infobuf+3) & 5)
		{
			modeinfo.winbsegment = *(unsigned short*)(infobuf+8);
			modeinfo.win = 1;
		}
		modeinfo.granularity = *(short*)(infobuf+4) * 1024;
		modeinfo.win_size = *(short*)(infobuf+6) * 1024;
		modeinfo.bits_per_pixel = *(char*)(infobuf+25);
		modeinfo.bytes_per_pixel = (modeinfo.bits_per_pixel+1)/8;
		modeinfo.memory_model = *(unsigned char*)(infobuf+27);
		modeinfo.num_pages = *(char*)(infobuf+29) + 1;

		modeinfo.red_width = *(char*)(infobuf+31);
		modeinfo.red_pos = *(char*)(infobuf+32);
		modeinfo.green_width = *(char*)(infobuf+33);
		modeinfo.green_pos = *(char*)(infobuf+34);
		modeinfo.blue_width = *(char*)(infobuf+35);
		modeinfo.blue_pos = *(char*)(infobuf+36);

		modeinfo.pptr = *(long *)(infobuf+40);

		//8bit linear only
		if((modeinfo.memory_model==0x4)&&(modeinfo.bits_per_pixel==8))
		{
			Com_Printf("VESA mode 0x%0x %dx%d supported\n",modeinfo.modenum,modeinfo.width,modeinfo.height);

			if (num_vid_resolutions < MAX_RESOLUTIONS)
			{
				vid_resolutions[num_vid_resolutions].mode=num_vid_resolutions;
				vid_resolutions[num_vid_resolutions].vesa_mode=modeinfo.modenum;
				vid_resolutions[num_vid_resolutions].height=modeinfo.height;
				vid_resolutions[num_vid_resolutions].width=modeinfo.width;

				phys_mem_info.address = (int)modeinfo.pptr;
				phys_mem_info.size = 0x400000;

				if (__dpmi_physical_address_mapping(&phys_mem_info))
					return false;

				vid_resolutions[num_vid_resolutions].address = real2ptr (phys_mem_info.address);

				if (!(COM_CheckParm("-bankedvga"))) /* FS: FIXME, just add all modes for banked */
				{
					vid_resolutions[num_vid_resolutions].isLFB=true;
				}
				else
				{
					vid_resolutions[num_vid_resolutions].isLFB=false;
					vid_resolutions[num_vid_resolutions].isBanked=true;
				}
				Com_sprintf(vid_resolutions[num_vid_resolutions].menuname, sizeof(vid_resolutions[num_vid_resolutions].menuname), "[VESA %dx%d]",modeinfo.width,modeinfo.height);
				num_vid_resolutions++;
			}
		}
#ifdef VESA_DEBUG_OUTPUT
		printf("VID: (VESA) info for mode 0x%x\n", modeinfo.modenum);
		printf("  mode attrib = 0x%0x\n", modeinfo.mode_attributes);
		printf("  win a attrib = 0x%0x\n", *(unsigned char*)(infobuf+2));
		printf("  win b attrib = 0x%0x\n", *(unsigned char*)(infobuf+3));
		printf("  win a seg 0x%0x\n", (int) modeinfo.winasegment);
		printf("  win b seg 0x%0x\n", (int) modeinfo.winbsegment);
		printf("  bytes per scanline = %d\n", modeinfo.bytes_per_scanline);
		printf("  width = %d, height = %d\n", modeinfo.width, modeinfo.height);
		printf("  win = %c\n", 'A' + modeinfo.win);
		printf("  win granularity = %d\n", modeinfo.granularity);
		printf("  win size = %d\n", modeinfo.win_size);
		printf("  bits per pixel = %d\n", modeinfo.bits_per_pixel);
		printf("  bytes per pixel = %d\n", modeinfo.bytes_per_pixel);
		printf("  memory model = 0x%x\n", modeinfo.memory_model);
		printf("  num pages = %d\n", modeinfo.num_pages);
		printf("  red width = %d\n", modeinfo.red_width);
		printf("  red pos = %d\n", modeinfo.red_pos);
		printf("  green width = %d\n", modeinfo.green_width);
		printf("  green pos = %d\n", modeinfo.green_pos);
		printf("  blue width = %d\n", modeinfo.blue_width);
		printf("  blue pos = %d\n", modeinfo.blue_pos);
		printf("  phys mem = %x\n", modeinfo.pptr);
#endif
	}

	dos_freememory(infobuf);
	return true;
}
