
/****************************************************************/
/*								*/
/*			   dosnames.c				*/
/*			     DOS-C				*/
/*								*/
/*    Generic parsing functions for file name specifications	*/
/*								*/
/*			Copyright (c) 1994			*/
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
/*								*/
/****************************************************************/

#include "../../hdr/portab.h"

/* $Logfile:   D:/dos-c/src/fs/dosnames.c_v  $ */
#ifndef IPL
static BYTE *dosnamesRcsId = "$Header:   D:/dos-c/src/fs/dosnames.c_v   1.4   29 May 1996 21:15:12   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/fs/dosnames.c_v  $
 * 
 *    Rev 1.4   29 May 1996 21:15:12   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.3   19 Feb 1996  3:20:08   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.2   01 Sep 1995 17:48:44   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:05:56   patv
 * Initial revision.
 * 
 */

#include "globals.h"

#define PathSep(c) ((c)=='/'||(c)=='\\')
#define DriveChar(c) (((c)>='A'&&(c)<='Z')||((c)>='a'&&(c)<='z'))

static BOOL bFileChar(UCOUNT uChar);
VOID XlateLcase (BYTE *szFname, COUNT nChars);
VOID DosTrimPath (BYTE FAR *lpszPathNamep);


static BOOL bFileChar(UCOUNT uChar)
{
	BYTE *pszValChar = ".\"/\\[]:|<>+=;,", *pszPtr;

	/* Null is not a valid character				*/
	if(NULL == uChar)
		return FALSE;

	/* Loop through invalid character set				*/
	for (pszPtr = pszValChar; *pszPtr != NULL; pszPtr++)
		if(uChar == *pszPtr)
			return FALSE;

	/* Not in excluded set, it's ok.				*/
	return TRUE;
}


/* Should be converted to a portable version after v1.0 is released.	*/
VOID 
XlateLcase (BYTE *szFname, COUNT nChars)
{
	while(nChars--)
	{
		if(*szFname >= 'a' && *szFname <= 'z')
			*szFname -= ('a' - 'A');
		++szFname;
	}
}


/* "Sometimes a cigar is just a cigar" - Sigmund Freud			*/
COUNT
DosNames(BYTE FAR *lpszFileName, struct dosnames FAR *lpDosname)
{
	COUNT nCharCount;
	BYTE FAR *lpszPath, FAR *lpszPathSep;
	BYTE FAR *lpszWork;


	/* Initialize the structure					*/
	lpDosname -> dn_drive = default_drive;
	*lpDosname -> dn_network =
	*lpDosname -> dn_path =
	*lpDosname -> dn_name = '\0';
	lpszPath = lpDosname -> dn_path;

	/* Start by cheking for a drive specifier ...			*/
	if(DriveChar(*lpszFileName) && ':' == lpszFileName[1])
	{
		/* found a drive, fetch it and bump pointer past drive	*/
		lpDosname -> dn_drive = *lpszFileName - 'A';
		if(lpDosname -> dn_drive > 26)
			lpDosname -> dn_drive -= ('a' - 'A');
		lpszFileName += 2;
	}
	/* or for a network name specifier				*/
	else if ('\\' == lpszFileName[1])
	{
		/* bump past network '\\' specifier			*/
		lpszFileName += 2;

		/* and copy network name				*/
		for(lpszWork = lpDosname -> dn_network, nCharCount = 0;
		 nCharCount < NAMEMAX
		 && bFileChar(*lpszFileName)
		 && !PathSep(*lpszFileName);
		 ++nCharCount)
		{
			*lpszWork++ = *lpszFileName++;
			*lpszWork = '\0';
		}
	}

	/* Loop through the name we got until a null terminator		*/
	for(nCharCount = 0, lpszPath = lpszWork = lpDosname -> dn_path;
	  nCharCount < NAMEMAX;
	  ++nCharCount)
	{
		if(!bFileChar(*lpszFileName)
		 && !PathSep(*lpszFileName)
		  && '.' != *lpszFileName)
			break;

		/* Mark the path seperator for later use.  We need to	*/
		/* split the file and the path later, so we always mark	*/
		/* last place we saw one.				*/
		/* NB: we mark the path because we're going to null	*/
		/* terminate there later				*/
		if(PathSep(*lpszFileName))
		{
			lpszWork = lpszPath;
			*lpszPath++ = '\\';
			++lpszFileName;
		}
		else
			*lpszPath++ = *lpszFileName++;
	}
	*lpszPath = '\0';

	/* Now that we got the path, let's seperate the file name from	*/
	/* the dir name.						*/
	if(lpDosname -> dn_path == lpszWork)
	{
		/* We never moved off the path, either it starts from	*/
		/* root and has only a file or it's just a specifier	*/
		/* for root.  It may also be a null			*/
		if(NULL == *lpDosname -> dn_path)
			return SUCCESS;

		if('\\' == *lpDosname -> dn_path)
		{
			if(NULL == lpDosname -> dn_path[1])
				/* Done, just exit			*/
				return SUCCESS;

			/* copy the file name and return		*/
			fstrncpy(lpDosname -> dn_name,
			 (lpDosname -> dn_path + 1),
			 FNAME_SIZE + FEXT_SIZE + 1);
			lpDosname -> dn_path[1] = NULL;
			return SUCCESS;
		}
		else
		{
			/* just a file specifier, make the path '.'	*/
			fstrncpy(lpDosname -> dn_name, lpDosname -> dn_path,
			  FNAME_SIZE + FEXT_SIZE + 1);
			fstrncpy(lpDosname -> dn_path, (BYTE FAR *)".", 1);
			return SUCCESS;
		}
	}
	else
	{
		/* Copy out the name and put a null in at the last	*/
		/* place we saw a seperator.				*/
		*lpszWork++ = NULL;
		fstrncpy(lpDosname -> dn_name, lpszWork,
		  FNAME_SIZE + FEXT_SIZE + 1);

		/* Clean up before leaving				*/
		DosTrimPath(lpDosname -> dn_path);
		return SUCCESS;
	}
}


BOOL 
IsDevice (BYTE *pszFileName)
{
	struct dosnames Dosname;
	COUNT nLen = min(strlen(pszFileName),8);
	REG struct dhdr FAR *dhp = (struct dhdr FAR *)&nul_dev;

	/* break up the name first				*/
	DosNames((BYTE FAR *)pszFileName, (struct dosnames FAR *)&Dosname);

	/* Test 1 - does it start with a \dev or /dev		*/
	if((fstrcmp(Dosname.dn_name, (BYTE FAR *)"/dev") == 0)
	 || (fstrcmp(Dosname.dn_name, (BYTE FAR *)"\\dev") == 0))
		return TRUE;

	/* Test 2 - is it on the device chain?			*/
	for(; -1l != (LONG)dhp; dhp = dhp -> dh_next)
	{
		BYTE cBuffer[8];
		COUNT nIdx;

		/* Skip if block device				*/
		if(!(dhp -> dh_attr & ATTR_CHAR))
			continue;

		/* copy to cBuffer and space fill		*/
		for(nIdx = 0;
		 Dosname.dn_name[nIdx] != '.' && nIdx < nLen; ++nIdx)
		 	cBuffer[nIdx] = Dosname.dn_name[nIdx];
		for(; nIdx > 8; ++nIdx)
		 	cBuffer[nIdx] = ' ';

		/* now compare					*/
		if(fstrncmp((BYTE FAR *)cBuffer, dhp -> dh_name, 8) == 0)
			return TRUE;
	}

	return FALSE;
}


VOID
DosTrimPath (BYTE FAR *lpszPathNamep)
{
	BYTE FAR *lpszLast, FAR *lpszNext;
	COUNT nChars, flDotDot;

	for(lpszLast = lpszNext = lpszPathNamep, nChars = 0;
	  *lpszNext != '\0' && nChars < NAMEMAX; )
	{
		/* Initialize flag for loop.				*/
		flDotDot = FALSE;

		/* If we are at a path seperator, check for extra path	*/
		/* seperator, '.' and '..' to reduce.			*/
		if(*lpszNext == '\\')
		{
			/* If it's '\', just move everything down one.	*/
			if(*(lpszNext + 1) == '\\')
				fstrncpy(lpszNext, lpszNext + 1, NAMEMAX);
			/* also ckech for '.' and '..' and move down	*/
			/* as appropriate.				*/
			else if(*(lpszNext + 1) == '.')
			{
				if(*(lpszNext + 2) == '.'
				 && *(lpszNext + 3) == '\\')
				{
					fstrncpy(lpszLast, lpszNext + 3, NAMEMAX);
					/* bump back to the last	*/
					/* seperator.			*/
					lpszNext = lpszLast;
					/* set lpszLast to the last one	*/
					if(lpszLast <= lpszPathNamep)
						continue;
					do
					{
						--lpszLast;
					}
					while(lpszLast != lpszPathNamep
					 && *lpszLast != '\\');
					flDotDot = TRUE;
				}
				/* Note: we skip strange stuff that	*/
				/* starts with '.'			*/
				else if(*(lpszNext + 2) == '\\')
				{
					fstrncpy(lpszNext, lpszNext + 2, NAMEMAX);
				}
				/* If we're at theend of a string, just	*/
				/* exit.				*/
				else if(*(lpszNext + 2) == NULL)
					return;
			}
			else
			{
				/* No '.' or '\' so mark it and bump	*/
				/* past					*/
				lpszLast = lpszNext++;
				continue;
			}

			/* Done.  Now set last to next to mark this	*/
			/* instance of path seperator.			*/
			if(!flDotDot)
				lpszLast = lpszNext;
		}
		else
			/* For all other cases, bump lpszNext for the	*/
			/* next check					*/
			++lpszNext;
	}
}
