
/****************************************************************/
/*								*/
/*			      cds.h				*/
/*								*/
/*		    Current Directory structures		*/
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

/* $Logfile:   D:/dos-c/hdr/cds.h_v  $ */
#ifdef MAIN
static BYTE *Cds_hRcsId = "$Header:   D:/dos-c/hdr/cds.h_v   1.0   19 Feb 1996  3:15:52   patv  $";
#endif

/*
 * $Log:   D:/dos-c/hdr/cds.h_v  $
   
      Rev 1.0   19 Feb 1996  3:15:52   patv
   Added NLS, int2f and config.sys processing
 */

#define MAX_CDSPATH 67

struct cds
{
	BYTE	cdsCurrentPath[MAX_CDSPATH];
	UWORD	cdsFlags;
	struct dpb FAR *
		cdsDpb;

	union
	{
		WORD _cdsStrtClst;
		struct
		{
			BYTE FAR *
				_cdsRedirRec;
			UWORD	_cdsParam;
		} _cdsRedir;
	} _cdsUnion;

	WORD	cdsJoinOffset;

	/* For >3.3 compatibility
	BYTE	cdsNetFlag1;
	BYTE FAR *
		cdsIfs;
	UWORD	cdsNetFlags2;
	*/
};

#define cdsStrtClst _cdsUnion._csdStrtClst

/* Bits for cdsFlags						*/
#define CDSNETWDRV	0x8000
#define CDSPHYSDRV	0x4000
#define CDSJOINED	0x2000
#define CDSSUBST	0x1000
