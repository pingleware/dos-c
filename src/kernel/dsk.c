
/****************************************************************/
/*								*/
/*			      dsk.c				*/
/*								*/
/*			Copyright (c) 1995			*/
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

#include "../../hdr/portab.h"
#include "globals.h"

/* $Logfile:   D:/dos-c/src/kernel/dsk.c_v  $ */
#ifdef VERSION_STRINGS
static BYTE *dskRcsId = "$Header:   D:/dos-c/src/kernel/dsk.c_v   1.4   29 May 1996 21:03:32   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/kernel/dsk.c_v  $
 * 
 *    Rev 1.4   29 May 1996 21:03:32   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.3   19 Feb 1996  3:21:36   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.2   01 Sep 1995 17:54:18   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:52:00   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:32:42   patv
 * Initial revision.
 */

#ifdef PROTO
BOOL fl_reset(VOID);
COUNT fl_rd_status(WORD);
COUNT fl_read(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT fl_write(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
BOOL fl_verify(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
BOOL fl_format(WORD, BYTE FAR *);
#else
BOOL fl_reset();
COUNT fl_rd_status();
COUNT fl_read();
COUNT fl_write();
BOOL fl_verify();
BOOL fl_format();
#endif

#define NDEV		4		/* only one for demo		*/
#define	SEC_SIZE	512		/* size of sector in bytes	*/
#define N_RETRY		5		/* number of retries permitted	*/
#define	NENTRY		25		/* total size of dispatch table	*/

#define dsk_unit(x)	((x)>=2?(((x)-2)|0x80):(x))
#define hd(x)		((x)>=2)

union
{
	BYTE	bytes[2 * SEC_SIZE];
	boot	boot_sector;
} buffer;

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

static bpb bpbarray[NDEV] =
{
	{
		SEC_SIZE,		/* Physical sector size		*/
		2,			/* Sectors/cluster		*/
		1,			/* reserved sectors		*/
		2,			/* # FAT's			*/
		112,			/* # dir entries		*/
		720,			/* # sectors on disk		*/
		0xfd,			/* media descriptor byte	*/
		2			/* # FAT sectors		*/
	},
	{
		SEC_SIZE,		/* Physical sector size		*/
		2,			/* Sectors/cluster		*/
		1,			/* reserved sectors		*/
		2,			/* # FAT's			*/
		112,			/* # dir entries		*/
		720,			/* # sectors on disk		*/
		0xfd,			/* media descriptor byte	*/
		2			/* # FAT sectors		*/
	},
	{
		SEC_SIZE,		/* Physical sector size		*/
		2,			/* Sectors/cluster		*/
		1,			/* reserved sectors		*/
		2,			/* # FAT's			*/
		112,			/* # dir entries		*/
		720,			/* # sectors on disk		*/
		0xfd,			/* media descriptor byte	*/
		2			/* # FAT sectors		*/
	},
	{
		SEC_SIZE,		/* Physical sector size		*/
		2,			/* Sectors/cluster		*/
		1,			/* reserved sectors		*/
		2,			/* # FAT's			*/
		112,			/* # dir entries		*/
		720,			/* # sectors on disk		*/
		0xfd,			/* media descriptor byte	*/
		2			/* # FAT sectors		*/
	}
};


static bpb *bpbptrs[NDEV] =
{
	&bpbarray[0],
	&bpbarray[1],
	&bpbarray[2],
	&bpbarray[3]
};

#define N_PART 4

static COUNT part_map[N_PART];

static WORD head, track, sector, ret;
static WORD count;
static COUNT nDevices;

#define PARTOFF	0x1be

static struct
{
	BYTE	peBootable;
	BYTE	peBeginHead;
	BYTE	peBeginSector;
	UWORD	peBeginCylinder;
	BYTE	peFileSystem;
	BYTE	peEndHead;
	BYTE	peEndSector;
	UWORD	peEndCylinder;
	LONG	peStartSector;
	LONG	peSectors;
} partition[4];


#ifdef PROTO
WORD init(rqptr), mediachk(rqptr), bldbpb(rqptr), blockio(rqptr), blk_error(rqptr);
COUNT ltop(WORD *, WORD *, WORD *, COUNT, COUNT, LONG, byteptr);
WORD dskerr(COUNT);
COUNT get_part(COUNT drive, COUNT idx);
#else
WORD init(), mediachk(), bldbpb(), blockio(), blk_error();
WORD dskerr();
COUNT get_part();
#endif

/*									*/
/* the function dispatch table						*/
/*									*/

#ifdef PROTO
static WORD (*dispatch[NENTRY]) (rqptr) =
#else
static WORD (*dispatch[NENTRY]) () =
#endif
{
	init,				/* Initialize			*/
	mediachk,			/* Media Check			*/
	bldbpb,				/* Build BPB			*/
	blk_error,			/* Ioctl In			*/
	blockio,			/* Input (Read)			*/
	blk_error,			/* Non-destructive Read		*/
	blk_error,			/* Input Status			*/
	blk_error,			/* Input Flush			*/
	blockio,			/* Output (Write)		*/
	blockio,			/* Output with verify		*/
	blk_error,			/* Output Status		*/
	blk_error,			/* Output Flush			*/
	blk_error,			/* Ioctl Out			*/
	blk_error,			/* Device Open			*/
	blk_error,			/* Device Close			*/
	blk_error,			/* Removable Media		*/
	blk_error,			/* Output till busy		*/
	blk_error,			/* undefined			*/
	blk_error,			/* undefined			*/
	blk_error,			/* Generic Ioctl		*/
	blk_error,			/* undefined			*/
	blk_error,			/* undefined			*/
	blk_error,			/* undefined			*/
	blk_error,			/* Get Logical Device		*/
	blk_error			/* Set Logical Device		*/
};

#define SIZEOF_PARTENT	16

#define FAT12		0x01
#define	FAT16SMALL	0x04
#define	EXTENDED	0x05
#define	FAT16LARGE	0x06

COUNT 
get_part (COUNT drive, COUNT idx)
{
	REG retry = N_RETRY;
	REG BYTE *p = (BYTE *)&buffer.bytes[PARTOFF + (idx * SIZEOF_PARTENT)];
	BYTE packed_byte, pb1;

	do
	{
		ret = fl_read((WORD)dsk_unit(drive), (WORD)0, (WORD)0, (WORD)1, (WORD)1, (byteptr)&buffer);
	} while (ret != 0 && --retry > 0);
	if(ret != 0)
		return FALSE;
	getbyte((VOID *)p, &partition[idx].peBootable);
	++p;
	getbyte((VOID *)p, &partition[idx].peBeginHead);
	++p;
	getbyte((VOID *)p, &packed_byte);
	partition[idx].peBeginSector = packed_byte & 0x3f;
	++p;
	getbyte((VOID *)p, &pb1);
	++p;
	partition[idx].peBeginCylinder = pb1 + ((0xc0 & packed_byte) << 2);
	getbyte((VOID *)p, &partition[idx].peFileSystem);
	++p;
	getbyte((VOID *)p, &partition[idx].peEndHead);
	++p;
	getbyte((VOID *)p, &packed_byte);
	partition[idx].peEndSector = packed_byte & 0x3f;
	++p;
	getbyte((VOID *)p, &pb1);
	++p;
	partition[idx].peEndCylinder = pb1 + ((0xc0 & packed_byte) << 2);
	getlong((VOID *)p, &partition[idx].peStartSector);
	p += sizeof(LONG);
	getlong((VOID *)p, &partition[idx].peSectors);
	return TRUE;
}


COUNT 
blk_driver (rqptr rp)
{
	if(rp -> r_unit >= NDEV)
		return failure(E_UNIT);
	if(rp -> r_command > NENTRY)
	{
		return failure(E_FAILURE);	/* general failure */
	}
	else
		return ((*dispatch[rp -> r_command])(rp));
}


static WORD 
init (rqptr rp)
{
	REG COUNT idx, off = 0;
	COUNT Drive;

	/* Reset the drives						*/
	fl_reset();

	nDevices = 2;

	for(Drive = 2; Drive < NDEV; Drive++)
	{
		COUNT RetCode;

		/* Retrieve all the partition information		*/
		for(RetCode = TRUE, idx = 0; RetCode && (idx < N_PART); idx++)
			RetCode = get_part(Drive, idx);
		if(!RetCode)
			break;
		else
			++nDevices;

		/* Search for the first DOS partition and start		*/
		/* building the map for the hard drive			*/
		for(idx = 0; idx < N_PART; idx++)
		{
			if(partition[idx].peFileSystem == FAT12
			|| partition[idx].peFileSystem == FAT16SMALL
			|| partition[idx].peFileSystem == FAT16LARGE)
			{
				miarray[Drive+ off].mi_offset
				 = partition[idx].peStartSector;
				part_map[off++] = idx;
				break;
			}
		}

		/* Then search for the remaing DOS extended partitions	*/
		/* and finish building the map for the hard drive	*/
		for(idx = 0; idx < N_PART; idx++)
		{
			if(partition[idx].peFileSystem == EXTENDED)
			{
				miarray[Drive + off].mi_offset
				 = partition[idx].peStartSector;
				part_map[off++] = idx;
				break;
			}
		}
	}

	rp -> r_nunits = nDevices;
	rp -> r_bpbptr = bpbptrs;
	rp -> r_endaddr = device_end();
	return S_DONE;
}


static WORD 
mediachk (rqptr rp)
{
	if(hd(rp -> r_unit))
		rp -> r_mcretcode = M_NOT_CHANGED;
	else
		rp -> r_mcretcode = tdelay((LONG)37) ? M_DONT_KNOW : M_NOT_CHANGED;
	return S_DONE;
}


static WORD 
bldbpb (rqptr rp)
{
	REG retry = N_RETRY;
	ULONG count;
	byteptr trans;
	WORD local_word;

	if(hd(rp -> r_unit))
	{
		head = partition[part_map[rp -> r_unit - 2]].peBeginHead;
		sector = partition[part_map[rp -> r_unit - 2]].peBeginSector;
		track = partition[part_map[rp -> r_unit - 2]].peBeginCylinder;
	}
	else
	{
		head = 0;
		sector = 1;
		track = 0;
	}

	do
	{
		ret = fl_read((WORD)dsk_unit(rp -> r_unit), (WORD)head, (WORD)track, (WORD)sector, (WORD)1, (byteptr)&buffer);
	} while (ret != 0 && --retry > 0);
	if(ret != 0)
		return(dskerr(ret));

	getword(&((((BYTE *)&buffer.bytes[BT_BPB]))[BPB_NBYTE]), &bpbarray[rp -> r_unit].bpb_nbyte);
	getbyte(&((((BYTE *)&buffer.bytes[BT_BPB]))[BPB_NSECTOR]), &bpbarray[rp -> r_unit].bpb_nsector);
	getword(&((((BYTE *)&buffer.bytes[BT_BPB]))[BPB_NRESERVED]), &bpbarray[rp -> r_unit].bpb_nreserved);
	getbyte(&((((BYTE *)&buffer.bytes[BT_BPB]))[BPB_NFAT]), &bpbarray[rp -> r_unit].bpb_nfat);
	getword(&((((BYTE *)&buffer.bytes[BT_BPB]))[BPB_NDIRENT]), &bpbarray[rp -> r_unit].bpb_ndirent);
	getword(&((((BYTE *)&buffer.bytes[BT_BPB]))[BPB_NSIZE]), &bpbarray[rp -> r_unit].bpb_nsize);
	getword(&((((BYTE *)&buffer.bytes[BT_BPB]))[BPB_NSIZE]), &bpbarray[rp -> r_unit].bpb_nsize);
	getbyte(&((((BYTE *)&buffer.bytes[BT_BPB]))[BPB_MDESC]), &bpbarray[rp -> r_unit].bpb_mdesc);
	getword(&((((BYTE *)&buffer.bytes[BT_BPB]))[BPB_NFSECT]), &bpbarray[rp -> r_unit].bpb_nfsect);
	getword(&((((BYTE *)&buffer.bytes[BT_BPB]))[BPB_NSECS]), &bpbarray[rp -> r_unit].bpb_nsecs);
	getword(&((((BYTE *)&buffer.bytes[BT_BPB]))[BPB_NHEADS]), &bpbarray[rp -> r_unit].bpb_nheads);
	getlong(&((((BYTE *)&buffer.bytes[BT_BPB])[BPB_HIDDEN])), &bpbarray[rp -> r_unit].bpb_hidden);
	getlong(&((((BYTE *)&buffer.bytes[BT_BPB])[BPB_HUGE])), &bpbarray[rp -> r_unit].bpb_huge);
#ifdef DSK_DEBUG
	printf("BPB_NBYTE     = %04x\n", bpbarray[rp -> r_unit].bpb_nbyte);
	printf("BPB_NSECTOR   = %02x\n", bpbarray[rp -> r_unit].bpb_nsector);
	printf("BPB_NRESERVED = %04x\n", bpbarray[rp -> r_unit].bpb_nreserved);
	printf("BPB_NFAT      = %02x\n", bpbarray[rp -> r_unit].bpb_nfat);
	printf("BPB_NDIRENT   = %04x\n", bpbarray[rp -> r_unit].bpb_ndirent);
	printf("BPB_NSIZE     = %04x\n", bpbarray[rp -> r_unit].bpb_nsize);
	printf("BPB_MDESC     = %02x\n", bpbarray[rp -> r_unit].bpb_mdesc);
	printf("BPB_NFSECT    = %04x\n", bpbarray[rp -> r_unit].bpb_nfsect);
#endif
	rp -> r_bpptr = &bpbarray[rp -> r_unit];
	count = miarray[rp -> r_unit].mi_size =
	 bpbarray[rp -> r_unit].bpb_nsize == 0 ?
	  bpbarray[rp -> r_unit].bpb_huge :
	  bpbarray[rp -> r_unit].bpb_nsize;
	getword((&(((BYTE *)&buffer.bytes[BT_BPB])[BPB_NHEADS])), &miarray[rp -> r_unit].mi_heads);
	head = miarray[rp -> r_unit].mi_heads;
	getword((&(((BYTE *)&buffer.bytes[BT_BPB])[BPB_NSECS])), &miarray[rp -> r_unit].mi_sectors);
	if(miarray[rp -> r_unit].mi_size == 0)
		getlong(&((((BYTE *)&buffer.bytes[BT_BPB])[BPB_HUGE])), &miarray[rp -> r_unit].mi_size);
	sector = miarray[rp -> r_unit].mi_sectors;
	if(head == 0 || sector == 0)
	{
		tmark();
		return failure(E_FAILURE);
	}
	miarray[rp -> r_unit].mi_cyls = count / (head * sector);
	tmark();
#ifdef DSK_DEBUG
	printf("BPB_NSECS     = %04x\n", sector);
	printf("BPB_NHEADS    = %04x\n", head);
	printf("BPB_HIDDEN    = %08lx\n", bpbarray[rp -> r_unit].bpb_hidden);
	printf("BPB_HUGE      = %08lx\n", bpbarray[rp -> r_unit].bpb_huge);
#endif
	return S_DONE;
}

static WORD 
blockio (rqptr rp)
{
	REG retry = N_RETRY, remaining;
	UWORD cmd, total;
	ULONG start;
	byteptr trans;

	cmd = rp -> r_command;
	total = 0;
	trans = rp -> r_trans;
	tmark();
	for
	(
		remaining = rp -> r_count,
		 start = (rp -> r_start != HUGECOUNT ? rp -> r_start : rp -> r_huge)
		  + miarray[rp -> r_unit].mi_offset;
		remaining > 0;
		remaining -= count, trans += count * SEC_SIZE, start += count
	)
	{
		if(ltop(&track, &sector, &head,  rp -> r_unit, 1, start, trans) != 1)
		{
/*			printf("split sector at 0x%lx", trans);*/
		}
		count = ltop(&track, &sector, &head,  rp -> r_unit, remaining, start, trans);
		total += count;
		do
		{
			switch(cmd)
			{
			case C_INPUT:
				ret = fl_read((WORD)dsk_unit(rp -> r_unit), (WORD)head, (WORD)track, (WORD)sector, (WORD)count, trans);
				break;

			case C_OUTPUT:
			case C_OUTVFY:
				ret = fl_write((WORD)dsk_unit(rp -> r_unit), (WORD)head, (WORD)track, (WORD)sector, (WORD)count, trans);
				break;

			default:
				return failure(E_FAILURE);
			}
		} while (ret != 0 && --retry > 0);
		if(ret != 0)
		{
			rp -> r_count = 0;
			return dskerr(ret);
		}
		if(cmd == C_OUTVFY)
		{
			ret = fl_verify((WORD)dsk_unit(rp -> r_unit), (WORD)head, (WORD)track, (WORD)sector, (WORD)count, rp -> r_trans);
			if(ret != 0)
			{
				rp -> r_count = 0;
				return dskerr(ret);
			}
		}
	}
	rp -> r_count = total;
	return S_DONE;
}

static WORD 
blk_error (rqptr rp)
{
	rp -> r_count = 0;
	return failure(E_FAILURE);	/* general failure */
}



static WORD 
dskerr (COUNT code)
{
/*	printf("diskette error:\nhead = %d\ntrack = %d\nsector = %d\ncount = %d\n",
		head, track, sector, count);*/
	switch(code & 0x03)
	{
	case 1:		/* invalid command - general failure */
		if(code & 0x08)
			return(E_FAILURE);
		else
			return failure(E_CMD);

	case 2:		/* address mark not found - general  failure */
		return failure(E_FAILURE);

	case 3:		/* write protect */
		return failure(E_WRPRT);

	default:
		if(code & 0x80)		/* time-out */
			return failure(E_NOTRDY);
		else if(code & 0x40)	/* seek error */
			return failure(E_SEEK);
		else if(code & 0x10)	/* CRC error */
			return failure(E_CRC);
		else if(code & 0x04)
			return failure(E_NOTFND);
		else
			return failure(E_FAILURE);
	}
}

/*									*/
/* Do logical block number to physical head/track/sector mapping	*/
/*									*/
static COUNT 
ltop (WORD *trackp, WORD *sectorp, WORD *headp, REG COUNT unit, COUNT count, LONG strt_sect, byteptr strt_addr)
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


