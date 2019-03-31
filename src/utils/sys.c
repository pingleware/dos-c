
/****************************************************************/
/*								*/
/*			      sys.c				*/
/*			      DOS-C				*/
/*								*/
/*		       sys utility for DOS-C			*/
/*								*/
/*			Copyright (c) 1991			*/
/*			Pasquale J. Villani			*/
/*			All Rights Reserved			*/
/*								*/
/* This file is part of DOS-C.					*/
/*								*/
/* DOS-C is free software; you can redistribute it and/or	*/
/* modify it under the terms of the GNU General Public License	*/
/* as published by the Free Software Foundation; either version	*/
/* 2, or (at your option) any later version.			*/
/*								*/
/* DOS-C is distributed in the hope that it will be useful, but	*/
/* WITHOUT ANY WARRANTY; without even the implied warranty of	*/
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See	*/
/* the GNU General Public License for more details.		*/
/*								*/
/* You should have received a copy of the GNU General Public	*/
/* License along with DOS-C; see the file COPYING.  If not,	*/
/* write to the Free Software Foundation, 675 Mass Ave,		*/
/* Cambridge, MA 02139, USA.					*/
/****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>
#include <dos.h>
#include <ctype.h>
#include <mem.h>
#include "../../hdr/portab.h"
#include "../../hdr/device.h"

BYTE *pgm = "sys";

#ifdef PROTO
BOOL fl_reset(VOID);
COUNT fl_rd_status(WORD);
COUNT fl_read(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT fl_write(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
BOOL fl_verify(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
BOOL fl_format(WORD, BYTE FAR *);
VOID put_boot(COUNT);
BOOL check_space(COUNT, BYTE *);
COUNT ltop(WORD *, WORD *, WORD *, COUNT, COUNT, LONG, byteptr);
BOOL copy(COUNT, BYTE *);
BOOL DiskReset(VOID);
COUNT DiskRead(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT DiskWrite(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
#else
BOOL fl_reset();
COUNT fl_rd_status();
COUNT fl_read();
COUNT fl_write();
BOOL fl_verify();
BOOL fl_format();
VOID put_boot();
BOOL check_space();
COUNT ltop();
BOOL copy();
BOOL DiskReset();
COUNT DiskRead();
COUNT DiskWrite();
#endif

/*								*/
/* special word packing prototypes				*/
/*								*/
#ifdef NATIVE
# define getlong(vp, lp) (*(LONG *)(lp)=*(LONG *)(vp))
# define getword(vp, wp) (*(WORD *)(wp)=*(WORD *)(vp))
# define getbyte(vp, bp) (*(BYTE *)(bp)=*(BYTE *)(vp))
# define fgetlong(vp, lp) (*(LONG FAR *)(lp)=*(LONG FAR *)(vp))
# define fgetword(vp, wp) (*(WORD FAR *)(wp)=*(WORD FAR *)(vp))
# define fgetbyte(vp, bp) (*(BYTE FAR *)(bp)=*(BYTE FAR *)(vp))
# define fputlong(lp, vp) (*(LONG FAR *)(vp)=*(LONG FAR *)(lp))
# define fputword(wp, vp) (*(WORD FAR *)(vp)=*(WORD FAR *)(wp))
# define fputbyte(bp, vp) (*(BYTE FAR *)(vp)=*(BYTE FAR *)(bp))
#else
# ifdef PROTO
VOID getword(VOID *, WORD *);
VOID getbyte(VOID *, BYTE *);
VOID fgetlong(VOID FAR *, LONG FAR *);
VOID fgetword(VOID FAR *, WORD FAR *);
VOID fgetbyte(VOID FAR *, BYTE FAR *);
VOID fputlong(LONG FAR *, VOID FAR *);
VOID fputword(WORD FAR *, VOID FAR *);
VOID fputbyte(BYTE FAR *, VOID FAR *);
# else
VOID getword();
VOID getbyte();
VOID fgetlong();
VOID fgetword();
VOID fgetbyte();
VOID fputlong();
VOID fputword();
VOID fputbyte();
# endif
#endif

#define	SEC_SIZE	512
#define NDEV		4
#define COPY_SIZE	16384
#define	NRETRY		5

static struct media_info
{
	ULONG	mi_size;		/* physical sector size		*/
	UWORD	mi_heads;		/* number of heads (sides)	*/
	UWORD	mi_cyls;		/* number of cyl/drive		*/
	UWORD	mi_sectors;		/* number of sectors/cyl	*/
	ULONG	mi_offset;		/* relative partition offset	*/
};

static struct media_info miarray[NDEV] =
{
	{720l, 2, 40, 9, 0l},
	{720l, 2, 40, 9, 0l},
	{720l, 2, 40, 9, 0l},
	{720l, 2, 40, 9, 0l}
};

COUNT drive;
UBYTE newboot[SEC_SIZE], oldboot[SEC_SIZE];

#define SBSIZE		51
#define SBOFFSET	11

VOID main(argc, argv)
COUNT argc;
BYTE *argv[];
{
	if(argc != 2)
	{
		fprintf(stderr, "Useage: %s drive\n drive = A,B,etc.\n", pgm);
		exit(1);
	}
	drive = *argv[1] - (islower(*argv[1]) ? 'a' : 'A');
	DiskReset();
	put_boot(drive);
	if(!check_space(drive, newboot))
	{
		fprintf(stderr, "%s: cannot transfer system files\n", pgm);
		exit(1);
	}
	if(!copy(drive, "ipl.sys"))
	{
		fprintf(stderr, "%s: cannot copy \"IPL.SYS\"\n", pgm);
		exit(1);
	}
	if(!copy(drive, "kernel.exe"))
	{
		fprintf(stderr, "%s: cannot copy \"KERNEL.EXE\"\n", pgm);
		exit(1);
	}
	if(!copy(drive, "boot.bin"))
	{
		fprintf(stderr, "%s: cannot copy \"BOOT.BIN\"\n", pgm);
		exit(1);
	}
	if(!copy(drive, "command.com"))
	{
		fprintf(stderr, "%s: cannot copy \"COMMAND.COM\"\n", pgm);
		exit(1);
	}
	exit(0);
}


VOID put_boot(drive)
COUNT drive;
{
	COUNT i;
	COUNT ifd;

	if((ifd = open("boot.bin", O_RDONLY | O_BINARY)) < 0)
	{
		fprintf(stderr, "%s: can't open\"boot.bin\"\n", pgm);
		exit(1);
	}

	if(read(ifd, newboot, SEC_SIZE) < SEC_SIZE)
	{
		fprintf(stderr, "%s: error read \"boot.bin\"", pgm);
		exit(1);
	}

	close(ifd);

	if((i = DiskRead(drive, 0, 0, 1, 1, (BYTE far *)oldboot)) != 0)
	{
		fprintf(stderr, "%s: floppy read error (code = 0x%02x)\n", pgm, i & 0xff);
		exit(1);
	}

	memcpy(&newboot[SBOFFSET], &oldboot[SBOFFSET], SBSIZE);

	if((i = DiskWrite(drive, 0, 0, 1, 1, (BYTE far *)newboot)) != 0)
	{
		fprintf(stderr, "%s: floppy write error (code = 0x%02x)\n", pgm, i & 0xff);
		exit(1);
	}
}


BOOL check_space(drive, newboot)
COUNT drive;
BYTE *newboot;
{
	BYTE *bpbp;
	BYTE nfat;
	UWORD nfsect;
	ULONG hidden, count;
	ULONG block;
	UBYTE nreserved;
	UCOUNT i;
	WORD track, head, sector;
	UBYTE buffer[SEC_SIZE];
	ULONG bpb_huge;
	UWORD bpb_nsize;

	/* get local information				*/
	getbyte((VOID *)&newboot[BT_BPB + BPB_NFAT], &nfat);
	getword((VOID *)&newboot[BT_BPB + BPB_NFSECT], &nfsect);
	getlong((VOID *)&newboot[BT_BPB + BPB_HIDDEN], &hidden);
	getbyte((VOID *)&newboot[BT_BPB + BPB_NRESERVED], &nreserved);

	getlong((VOID *)&newboot[BT_BPB + BPB_HUGE], &bpb_huge);
	getword((VOID *)&newboot[BT_BPB + BPB_NSIZE], &bpb_nsize);

	count = miarray[drive].mi_size = bpb_nsize == 0 ?
	 bpb_huge : bpb_nsize;

	/* Fix media information for disk			*/
	getword((&(((BYTE *)&newboot[BT_BPB])[BPB_NHEADS])), &miarray[drive].mi_heads);
	head = miarray[drive].mi_heads;
	getword((&(((BYTE *)&newboot[BT_BPB])[BPB_NSECS])), &miarray[drive].mi_sectors);
	if(miarray[drive].mi_size == 0)
		getlong(&((((BYTE *)&newboot[BT_BPB])[BPB_HUGE])), &miarray[drive].mi_size);
	sector = miarray[drive].mi_sectors;
	if(head == 0 || sector == 0)
	{
		fprintf(stderr, "Drive initialization failure\n");
		exit(1);
	}
	miarray[drive].mi_cyls = count / (head * sector);

	/* Now compute directory start				*/
	block = hidden + nfat * nfsect + nreserved;
	ltop(&track, &sector, &head, drive, 1, block, (byteptr)buffer);
	if((i = DiskRead(drive, head, track, sector, 1, (BYTE far *)buffer)) != 0)
	{
		fprintf(stderr, "%s: directory error (code = 0x%02x)\n", pgm, i & 0xff);
		exit(1);
	}
	return ((*buffer == '\0') || (*buffer == 0xe5));
}

/*									*/
/* Do logical block number to physical head/track/sector mapping	*/
/*									*/
static COUNT ltop(trackp, sectorp, headp, unit, count, strt_sect, strt_addr)
WORD *trackp, *sectorp, *headp;
REG COUNT unit;
LONG strt_sect;
COUNT count;
byteptr strt_addr;
{
#ifdef I86
	ULONG ltemp;
#endif
	REG ls, ps;

#ifdef I86
	/* Adjust for segmented architecture				*/
	ltemp = (((ULONG)mk_segment(strt_addr) << 4) + mk_offset(strt_addr)) & 0xffff;
	/* Test for 64K boundary crossing and return count large	*/
	/* enough not to exceed the threshold.				*/
	count = (((ltemp + SEC_SIZE * count) & 0xffff0000l) != 0l)
		? (0xffffl - ltemp) / SEC_SIZE
		: count;
#endif

	*trackp = strt_sect / (miarray[unit].mi_heads * miarray[unit].mi_sectors);
	*sectorp = strt_sect % miarray[unit].mi_sectors + 1;
	*headp = (strt_sect % (miarray[unit].mi_sectors * miarray[unit].mi_heads))
		/ miarray[unit].mi_sectors;
	if(((ls = *headp * miarray[unit].mi_sectors + *sectorp - 1) + count) >
		(ps = miarray[unit].mi_heads * miarray[unit].mi_sectors))
		count = ps - ls;
	return count;
}


BOOL copy(drive, file)
COUNT drive;
BYTE *file;
{
	BYTE dest[64];
	COUNT ifd, ofd, ret;
	BYTE buffer[COPY_SIZE];
	struct ftime ftime;

	sprintf(dest, "%c:\\%s", 'A'+drive, file);
	if((ifd = open((BYTE FAR *)file, O_RDONLY | O_BINARY)) < 0)
	{
		fprintf(stderr, "%s: \"%s\" not found\n", pgm, file);
		return FALSE;
	}
	_fmode = O_BINARY;
	if((ofd = creat((BYTE FAR *)dest, S_IREAD | S_IWRITE)) < 0)
	{
		fprintf(stderr, "%s: can't create\"%s\"\n", pgm, dest);
		return FALSE;
	}
	while((ret = read(ifd, (VOID *)buffer, COPY_SIZE)) == COPY_SIZE)
		write(ofd, (VOID *)buffer, ret);
	if(ret >= 0)
		write(ofd, (VOID *)buffer, ret);
	getftime(ifd, &ftime);
	setftime(ofd, &ftime);
	close(ifd);
	close(ofd);
	return TRUE;
}


BOOL DiskReset(VOID)
{
	fl_reset();
}

COUNT DiskRead(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR *buffer)
{
	int nRetriesLeft;

	for(nRetriesLeft = NRETRY; nRetriesLeft > 0; --nRetriesLeft)
	{
		if(fl_read(drive, head, track, sector, count, buffer) == count)
			return count;
	}
	return 0;
}

COUNT DiskWrite(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR *buffer)
{
	int nRetriesLeft;

	for(nRetriesLeft = NRETRY; nRetriesLeft > 0; --nRetriesLeft)
	{
		if(fl_write(drive, head, track, sector, count, buffer) == count)
			return count;
	}
	return 0;
}

