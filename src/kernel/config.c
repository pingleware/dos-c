
/****************************************************************/
/*								*/
/*			    config.c				*/
/*			      DOS-C				*/
/*								*/
/*		  config.sys Processing Functions		*/
/*								*/
/*			Copyright (c) 1996			*/
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

/* $Logfile:   D:/dos-c/src/kernel/config.c_v  $ */
#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Header:   D:/dos-c/src/kernel/config.c_v   1.1   29 May 1996 21:03:44   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/kernel/config.c_v  $
 * 
 *    Rev 1.1   29 May 1996 21:03:44   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.0   19 Feb 1996  3:22:16   patv
 * Added NLS, int2f and config.sys processing
 */

#if 0
#define dos_open(x,y)	_open((const char *)(x),(y))
#define dos_read(x,y,z)	_read((int)(x),(void *)(y),(unsigned)(z))
#define dos_close	_close
#endif

static BYTE FAR *lpOldLast;
static BYTE FAR *lpBase;
static BYTE szToken[LINESIZE];
static BYTE szLine[LINESIZE];
static COUNT nCfgLine;
static COUNT nPass;

VOID Buffers(BYTE *pLine);
VOID Files(BYTE *pLine);
VOID InitPgm(BYTE *pLine);
VOID CfgFailure(BYTE *pLine);
BYTE *GetNumArg(BYTE *pLine, COUNT *pnArg);
BYTE *GetStringArg(BYTE *pLine, BYTE *pszString);

struct table *LookUp(struct table *p, BYTE *token);

struct table
{
	BYTE	*entry;
	BYTE	pass;
	VOID	(*func)(BYTE *pLine);
};

static struct table  commands[] =
{
	{"buffers",	1,	Buffers},
	{"files",	1,	Files},
	{"command",	1,	InitPgm},
	/* rem is never executed by locking out pass			*/
	{"rem",		0,	CfgFailure},
	/* default action						*/
	{"", 		-1,	CfgFailure}
};

BYTE FAR *KernelAlloc(WORD nBytes);

/* Do first time initialization.  Store last so that we can reset it	*/
/* later.								*/
void
PreConfig(void)
{
	/* Set pass number						*/
	nPass = 0;

	/* Initialize the base memory pointers				*/
	lpOldLast = lpBase = (BYTE FAR *)&last;

	/* Begin by initializing our system buffers			*/
	buffers = (struct buffer FAR *)
		KernelAlloc(Config.cfgBuffers * sizeof(struct buffer));
#ifdef DEBUG
	printf("Preliminary buffer allocated at 0x%04x:0x%04x\n",
		FP_SEG(buffers), FP_OFF(buffers));
#endif

	/* Initialize the file table					*/
	f_nodes = (struct f_node FAR *)
		KernelAlloc(Config.cfgFiles * sizeof(struct f_node));
	/* sfthead = (sfttbl FAR *)&basesft; */
	/* FCBp = (sfttbl FAR *)&FcbSft; */
	FCBp = (sfttbl FAR *)
		KernelAlloc(sizeof(sftheader)
		 + Config.cfgFiles * sizeof(sft));
	sfthead = (sfttbl FAR *)
		KernelAlloc(sizeof(sftheader)
		 + Config.cfgFiles * sizeof(sft));
#ifdef DEBUG
	printf("Preliminary f_node allocated at 0x%04x:0x%04x\n",
		FP_SEG(f_nodes), FP_OFF(f_nodes));
	printf("Preliminary FCB table allocated at 0x%04x:0x%04x\n",
		FP_SEG(FCBp), FP_OFF(FCBp));
	printf("Preliminary sft table allocated at 0x%04x:0x%04x\n",
		FP_SEG(sfthead), FP_OFF(sfthead));
#endif

	/* Done.  Now initialize the MCB structure			*/
	/* This next line is 8086 and 80x86 real mode specific		*/
#ifdef DEBUG
	printf("Preliminary  allocation completed: top at 0x%04x:0x%04x\n",
		FP_SEG(lpBase), FP_OFF(lpBase));
#endif
	first_mcb = FP_SEG(lpBase) + ((FP_OFF(lpBase) + 0x0f) >> 4);

	/* We expect ram_top as Kbytes, so convert to paragraphs */
	mcb_init((mcb FAR *)(MK_FP(first_mcb, 0)),
	 (ram_top << 6) - first_mcb - 1);
	nPass = 1;
}


/* Do second pass initialization.					*/
/* Also, run config.sys to load drivers.				*/
void
PostConfig(void)
{
	/* Set pass number						*/
	nPass = 2;

	/* Initialize the base memory pointers from last time.		*/
	lpBase = lpOldLast;

	/* Begin by initializing our system buffers			*/
	buffers = (struct buffer FAR *)
		KernelAlloc(Config.cfgBuffers * sizeof(struct buffer));
#ifdef DEBUG
	printf("Buffer allocated at 0x%04x:0x%04x\n",
		FP_SEG(buffers), FP_OFF(buffers));
#endif

	/* Initialize the file table					*/
	f_nodes = (struct f_node FAR *)
		KernelAlloc(Config.cfgFiles * sizeof(struct f_node));
	/* sfthead = (sfttbl FAR *)&basesft; */
	/* FCBp = (sfttbl FAR *)&FcbSft; */
	FCBp = (sfttbl FAR *)
		KernelAlloc(sizeof(sftheader)
		 + Config.cfgFiles * sizeof(sft));
	sfthead = (sfttbl FAR *)
		KernelAlloc(sizeof(sftheader)
		 + Config.cfgFiles * sizeof(sft));
#ifdef DEBUG
	printf("f_node allocated at 0x%04x:0x%04x\n",
		FP_SEG(f_nodes), FP_OFF(f_nodes));
	printf("FCB table allocated at 0x%04x:0x%04x\n",
		FP_SEG(FCBp), FP_OFF(FCBp));
	printf("sft table allocated at 0x%04x:0x%04x\n",
		FP_SEG(sfthead), FP_OFF(sfthead));
#endif

	/* Done.  Now initialize the MCB structure			*/
	/* This next line is 8086 and 80x86 real mode specific		*/
#ifdef DEBUG
	printf("Allocation completed: top at 0x%04x:0x%04x\n",
		FP_SEG(lpBase), FP_OFF(lpBase));
#endif
	first_mcb = FP_SEG(lpBase) + ((FP_OFF(lpBase) + 0x0f) >> 4);

	/* We expect ram_top as Kbytes, so convert to paragraphs */
	mcb_init((mcb FAR *)(MK_FP(first_mcb, 0)),
	 (ram_top << 6) - first_mcb - 1);
}


VOID DoConfig(VOID)
{
	COUNT nFileDesc;
	COUNT nRetCode;

	/* Check to see if we have a config.sys file.  If not, just	*/
	/* exit since we don't force the user to have one.		*/
	if((nFileDesc = dos_open((BYTE FAR *)"config.sys", 0)) < 0)
	{
#ifdef DEBUG
		printf("CONFIG.SYS not found\n");
#endif
		return;
	}

	/* Have one -- initialize.					*/
	nCfgLine = 0;
	nPass = 1;

	/* Read each line into the buffer and then parse the line,	*/
	/* do the table lookup and execute the handler for that		*/
	/* function.							*/
	FOREVER
	{
		BYTE cRead;
		COUNT nCharRead = 0;
		BOOL bEof, bEol;
		BYTE *pLine, *pTmp;
		struct table *pEntry;

		/* Initialize to a null line				*/
		szLine[0] = '\0';
		bEol = FALSE;

		/* Read a line from config				*/
		while(!bEol)
		{
			/* Read a charachter				*/
			nRetCode =
			 dos_read(nFileDesc, (BYTE FAR *)&cRead, 1);

			/* and check for EOF				*/
			if(bEof = (nRetCode != 1))
			{
				bEol = TRUE;
				break;
			}

			switch(cRead)
			{
			case '\r':
				++nCfgLine;
				bEol = TRUE;
				break;

			case '\0':
			case '\n':
				break;

			default:
				szLine[nCharRead++] = cRead;
				szLine[nCharRead] = '\0';
				if(nCharRead >= (LINESIZE - 1))
					bEol = TRUE;
				break;
			}
		}

		/* Skip leading white space and get verb.		*/
		pLine = scan(szLine, szToken);

		/* Translate the verb to lower case ...			*/
		for(pTmp = szToken; *pTmp != '\0'; pTmp++)
			*pTmp = tolower(*pTmp);

		/* If the line was blank, skip it.  Otherwise, look up	*/
		/* the verb and execute the appropriate function.	*/
		if(*szToken != '\0')
		{
			pEntry = LookUp(commands, szToken);
			if(pEntry -> pass < 0 || pEntry -> pass == nPass)
				(*(pEntry -> func))(pLine);
		}

		/* Check bEof.  If it's true, exit.  This allows	*/
		/* for a line ending with an EOF.			*/
		if(bEof)
			break;
	}
	dos_close(nFileDesc);
}


struct table *LookUp(struct table *p, BYTE *token)
{
	while(*(p -> entry) != '\0')
	{
		if(strcmp(p -> entry, token) == 0)
			break;
		else
			++p;
	}
	return p;
}

BYTE *GetNumArg(BYTE *pLine, COUNT *pnArg)
{
	/* Skip leading white space					*/
	pLine = skipwh(pLine);

	/* We must find a '=' because form is VERB = NUMBER		*/
	if('=' != *pLine)
	{
		CfgFailure(pLine);
		return (BYTE *)0;
	}

	/* Found '=', now look for NUMBER				*/
	pLine = skipwh(++pLine);
	if(!isnum(pLine))
	{
		CfgFailure(pLine);
		return (BYTE *)0;
	}
	return GetNumber(pLine, pnArg);
}


BYTE *GetStringArg(BYTE *pLine, BYTE *pszString)
{
	/* Skip leading white space					*/
	pLine = skipwh(pLine);

	/* We must find a '=' because form is VERB = STRING [STRINGS]	*/
	if('=' != *pLine)
	{
		CfgFailure(pLine);
		return (BYTE *)0;
	}

	/* Found '=', now look for STRING				*/
	pLine = skipwh(++pLine);

	/* just return whatever string is there, including null		*/
	return scan(pLine, pszString);
}


static VOID Buffers(BYTE *pLine)
{
	COUNT nBuffers;

	/* Get the argument						*/
	if(GetNumArg(pLine, &nBuffers) == (BYTE *)0)
		return;

	/* Got the value, assign either default or new value		*/
	Config.cfgBuffers = max(Config.cfgBuffers, nBuffers);
}


static VOID Files(BYTE *pLine)
{
	COUNT nFiles;

	/* Get the argument						*/
	if(GetNumArg(pLine, &nFiles) == (BYTE *)0)
		return;

	/* Got the value, assign either default or new value		*/
	Config.cfgFiles = max(Config.cfgFiles, nFiles);
}


static VOID InitPgm(BYTE *pLine)
{
	/* Get the string argument that represents the new init pgm	*/
	GetStringArg(pLine, Config.cfgInit);

	/* Now take whatever tail is left and add it on as a single	*/
	/* string.							*/
	strcpy(Config.cfgInitTail, pLine);

	/* and add a DOS new line just to be safe			*/
	strcat(Config.cfgInitTail, "\r\n");
}


static VOID CfgFailure(BYTE *pLine)
{
	BYTE *pTmp = szLine;

	printf("CONFIG.SYS error in line %d\n", nCfgLine);
	printf(">>>%s\n   ", pTmp);
	while(pTmp++ != pLine)
		printf(" ");
	printf("^\n");
}

static BYTE FAR *
KernelAlloc(WORD nBytes)
{
	BYTE FAR *lpAllocated = lpBase;

	lpBase += nBytes;
	return lpAllocated;
}


