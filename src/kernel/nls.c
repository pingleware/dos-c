
/****************************************************************/
/*                                                              */
/*                            nls.c                             */
/*                            DOS-C                             */
/*                                                              */
/*    National Languge Support functions and data structures    */
/*                                                              */
/*                   Copyright (c) 1995, 1996                   */
/*                      Pasquale J. Villani                     */
/*                      All Rights Reserved                     */
/*                                                              */
/*                   Copyright (c) 1995, 1996                   */
/*                         Steffen Kaiser                       */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/****************************************************************/

/* $Logfile:   D:/dos-c/src/kernel/nls.c_v  $ */
#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Header:   D:/dos-c/src/kernel/nls.c_v   1.2   29 May 1996 21:03:46   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/kernel/nls.c_v  $
 * 
 *    Rev 1.2   29 May 1996 21:03:46   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.1   19 Feb 1996  4:34:46   patv
 * Corrected typo
 * 
 *    Rev 1.0   19 Feb 1996  3:21:46   patv
 * Added NLS, int2f and config.sys processing
 */

#include "../../hdr/portab.h"
#include "globals.h"

extern VOID INRPT FAR 
internalUpcase(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags);

#ifndef lint
static char const rcsid[] = 
	"$Id$";
#endif

/* one byte alignment */

#if defined(_MSC_VER) 
    #define asm __asm
    #pragma pack(1)
#elif defined(_QC) || defined(__WATCOM__)
    #pragma pack(1)
#elif defined(__ZTC__)
    #pragma ZTC align 1
#elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
    #pragma option -a-
#endif

struct ctryInfo {       /* Country Information DOS-38 */
	WORD dateFmt;                   /* 0: USA, 1: Europe, 2: Japan */
	char curr[5];                   /* ASCIZ of currency string */
	char thSep[2];                  /* ASCIZ of thousand's separator */
	char point[2];                  /* ASCIZ of decimal point */
	char dateSep[2];                /* ASCIZ of date separator */
	char timeSep[2];                /* ASCIZ of time separator */
	BYTE currFmt;                   /* format of currency:
						bit 0: currency string is placed 0: before, 1: behind number
						bit 1: currency string and number are separated by a space; 0: No, 1: Yes
					*/
	BYTE prescision;                /* */
	BYTE timeFmt;                   /* time format: 0: 12 hours; 1: 24 houres */
	VOID (FAR *upCaseFct)(VOID);    /* far call to a function mapping character in register AL */
	char dataSep[2];                /* ASCIZ of separator in data records */
};

struct _VectorTable
{
	VOID FAR *Table;
	BYTE FnCode;
}; 


struct _NlsInfo
{
	struct extCtryInfo
	{
		BYTE reserved[8];
		BYTE countryFname[64];
		WORD sysCodePage;
		WORD nFnEntries;
		struct _VectorTable VectorTable[6];
		
		/* Extended Country Information DOS-65-01 */
		WORD countryCode;               /* current COUNTRY= code */
		WORD codePage;                  /* current code page (CP) */

		struct ctryInfo nlsCtryInfo;
	} nlsExtCtryInfo;

/* characters of Yes/No prompt for DOS-65-23 */
	char yesCharacter;
	char noCharacter;

/* upcased characters for ECS-ASCII > 0x7f for DOS-65-02 */
	WORD upNCsize;                  /* number of entries in the following array */
	char upNormCh[128];

/* upcased characters for ECS-ASCII > 0x7f for file names for DOS-65-04 */
	WORD upFCsize;                  /* number of entries in the following array */
	char upFileCh[128];

/* collating sequence for ECS-ASCII 0..0xff for DOS-65-06 */
	WORD collSize;                  /* number of entries in the following array */
	char collSeq[256];

/* DBC support for DOS-65-07 */
	WORD dbcSize;                   /* number of entries in the following array */
	/* char dbcTable[1024]; no DBC support */
	WORD dbcEndMarker;              /* contains always 0 */

/* in file names permittable characters for DOS-65-05 */
	struct chFileNames {
		WORD fnSize;                    /* size of this structure */
		BYTE dummy1;
		char firstCh, lastCh;   /* first, last permittable character */
		BYTE dummy2;
		char firstExcl, lastExcl;       /* first, last excluded character */
		BYTE dummy3;
		BYTE numSep;            /* number of file name separators */
		char fnSeparators[14];
	} nlsFn;
} nlsInfo
#ifdef INIT_NLS_049
=               /* let's initialize it with values for Germany */
#include "049-437.nls"
#else
=               /* let's initialize it with default values (USA) */
#include "001-437.nls"
#endif
;

#define normalCh nlsInfo.upNormCh
#define fileCh nlsInfo.upFileCh
#define yesChar nlsInfo.yesCharacter
#define noChar nlsInfo.noCharacter

#define PathSep(c) ((c)=='/'||(c)=='\\')
#define DriveChar(c) (((c)>='A'&&(c)<='Z')||((c)>='a'&&(c)<='z'))

/* standard alignment */

#if defined (_MSC_VER) || defined(_QC) || defined(__WATCOMC__)
 #pragma pack()
#elif defined (__ZTC__)
 #pragma ZTC align
#elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
 #pragma option -a.
#endif

COUNT NlsFuncInst(VOID);

COUNT NlsFuncInst(VOID)
{
	BYTE cNlsRet;

	asm {
		xor     bx,bx
		mov     ax,1400h
		int     2Fh
		mov     cNlsRet,al
	}

	/* Return the al register as sign extended:                     */
	/*               0 == not installed, ok to install              */
	/*               1 == not installed, not ok to install          */
	/*              -1 == installed                                 */
	return cNlsRet;
}


BOOL
GetGlblCodePage(UWORD FAR *ActvCodePage, UWORD FAR *SysCodePage)
{
	*ActvCodePage = nlsInfo.nlsExtCtryInfo.codePage;
	*SysCodePage = nlsInfo.nlsExtCtryInfo.sysCodePage;
	return TRUE;
}


BOOL
SetGlblCodePage(UWORD FAR *ActvCodePage, UWORD FAR *SysCodePage)
{
	nlsInfo.nlsExtCtryInfo.codePage = *ActvCodePage;
	nlsInfo.nlsExtCtryInfo.sysCodePage = *SysCodePage;
	return TRUE;
}


UWORD SetCtryInfo(UBYTE FAR *lpShrtCode, UWORD FAR *lpLongCode,
	BYTE FAR *lpTable, UBYTE *nRetCode)
{
	UWORD CntryCode;
	UBYTE nNlsEntry;
	UWORD uSegTable, uOffTable;
	UBYTE nLclRet;

	/* Get the Country Code according to the DOS silly rules.       */
	if(0xff != *lpShrtCode)
		CntryCode = *lpShrtCode;
	else
		CntryCode = *lpLongCode;

	/* If it's the same country code as what's installed, just      */
	/* return because there's nothing to do.                        */
	if(CntryCode == nlsInfo.nlsExtCtryInfo.countryCode)
	{
		*nRetCode = 0;
		return CntryCode;
	}

	/* Check if nlsfunc is installed                                */
	if(NlsFuncInst() >= 0)
	{
		*nRetCode = 0xff;
		return 0xffff;
	}

	/* Get the country information from nlsfunc                     */
	uSegTable = FP_SEG(lpTable);
	uOffTable = FP_OFF(lpTable);
	asm {
		push    ds
		mov     bx,CntryCode
		mov     ax,1404h
		mov     ax,uSegTable
		mov     dx,uOffTable
		mov     ds,ax
		int     2Fh
		pop     ds
		mov     CntryCode,bx
		mov     nLclRet,al
	}
	*nRetCode = nLclRet;
	return CntryCode;
} 


UWORD GetCtryInfo(UBYTE FAR *lpShrtCode, UWORD FAR *lpLongCode,
	BYTE FAR *lpTable)
{
	UWORD CntryCode;

	if(0xff != *lpShrtCode)
		CntryCode = *lpShrtCode;
	else
		CntryCode = *lpLongCode;

	fbcopy((BYTE FAR *)&nlsInfo.nlsExtCtryInfo.nlsCtryInfo,
	  lpTable, sizeof(struct ctryInfo));
	return nlsInfo.nlsExtCtryInfo.countryCode;
} 


BOOL
ExtCtryInfo(UBYTE nOpCode, UWORD CodePageID, UWORD InfoSize, VOID FAR *Information)
{
	VOID FAR *lpSource;
	COUNT nIdx;

	if(0xffff != CodePageID)
	{
		UBYTE nNlsEntry;

		if(NlsFuncInst() >= 0)
			return FALSE;

		asm {
			mov     bp,word ptr nOpCode
			mov     bx,CodePageID
			mov     si,word ptr Information+2
			mov     ds,si
			mov     si,word ptr Information
			mov     ax,1402h
			int     2Fh
			cmp     al,0
			mov     nNlsEntry,al
		}
		if(0 != nNlsEntry)
			return FALSE;

		return  TRUE;
	}

	CodePageID = nlsInfo.nlsExtCtryInfo.codePage;

	for(nIdx = 0; nIdx < nlsInfo.nlsExtCtryInfo.nFnEntries; nIdx++)
	{
		if(nlsInfo.nlsExtCtryInfo.VectorTable[nIdx].FnCode == nOpCode)
		{
			BYTE FAR *bp = Information;
			lpSource = nlsInfo.nlsExtCtryInfo.VectorTable[nIdx].Table;

			if (nOpCode == 1)
			{
			    bp++;     // first byte unused    
			    *bp = (BYTE)(sizeof(struct ctryInfo) + 4);
			    bp += 2;

			    fbcopy(lpSource, bp, InfoSize > 3? InfoSize - 3 : 0);
			} else {
			    *bp++ = nOpCode;
			    *((VOID FAR **)bp) = lpSource;
			}
			return TRUE;
		}
	}

	return FALSE;
}


#pragma argsused
VOID INRPT FAR 
internalUpcase(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags)
{            if(ax & 0x80)
		ax = (ax & 0xff00) | (nlsInfo.upNormCh[ax & 0x7f] & 0xff);
}

char upMChar(UPMAP map, char ch)
/* upcase character ch according to the map */
{       return (ch >= 'a' && ch <= 'z')? ch + 'A' - 'a':
			((unsigned)ch > 0x7f? map[ch & 0x7f]: ch);
}

VOID upMMem(UPMAP map, char FAR *str, unsigned len)
{       REG unsigned c;

	if(len) do {
		if((c = *str) >= 'a' && c <= 'z')
			*str += 'A' - 'a';
		else if(c > 0x7f)
			*str = map[c & 0x7f];
		++str;
	} while(--len);
}

BYTE yesNo(char ch)     /* returns: 0: ch == "No", 1: ch == "Yes", 2: ch crap */
{
	ch = upMChar(normalCh, ch);
	if(ch == noChar)
		return 0;
	if(ch == yesChar)
		return 1;
	return 2;
}


char upChar(char ch) /* upcase a single character */
{
	return upMChar(normalCh, ch);
}


VOID upString(char FAR *str)    /* upcase a string */
{
	upMMem(normalCh, str, fstrlen(str));
}


VOID upMem(char FAR *str, unsigned len) /* upcase a memory area */
{
	upMMem(normalCh, str, len);
}

char upFChar(char ch) /* upcase a single character for file names */
{
	return upMChar(fileCh, ch);
}


VOID upFString(char FAR *str)   /* upcase a string for file names */
{
	upMMem(fileCh, str, fstrlen(str));
}


VOID upFMem(char FAR *str, unsigned len)        /* upcase a memory area for file names */
{
	upMMem(fileCh, str, len);
}

