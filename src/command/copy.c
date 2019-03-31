/****************************************************************/
/*								*/
/*			      copy.c				*/
/*								*/
/*		      command.com copy command 			*/
/*								*/
/*			  August 9, 1991			*/
/*								*/
/*		        Copyright (c) 1995			*/
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


/* $Logfile:   C:/dos-c/src/command/copy.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/copy.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:07:00   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:32   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:01:38   patv
 * Initial revision.
 */


#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

static BYTE *RcsId = "$Header:   C:/dos-c/src/command/copy.c_v   1.2   29 Aug 1996 13:07:00   patv  $";

#define CHUNK	16384

BOOL copy()
{
	COUNT ifd, ofd, ret, CopyCount = 0;
	seg MemSeg;
	BYTE FAR *MemBuffer;
	date fdate;
	time ftime;
	BYTE FromPath[MAX_CMDLINE], ToPath[MAX_CMDLINE];
	COUNT dest_driveno = -1, src_driveno = -1;
	BYTE src_path[MAX_CMDLINE], src[MAX_CMDLINE];
	BYTE dest_path[MAX_CMDLINE], dest[MAX_CMDLINE];
	BOOL bflag, aflag, AppendMode = FALSE, FirstTime = TRUE, IsWild, Error;
	dmatch dmp;

	/* Intialize the dta to put the dir info into			*/
	DosSetDta((BYTE FAR *)&dmp);

	/* And initialize our current location				*/
        FindHome();

	/* Parse the command line, and get our destination		*/
	/* First initialize parameters ...				*/
	aflag = bflag = FALSE;
	*dest = *src = *src_path = *dest_path = *FromPath = *ToPath = '\0';

	/* Then get the options.					*/
	dosopt("$d$p*$d$p*[ab]+", (BYTE FAR *)tail,
	 &src_driveno, src_path, src,
	 &dest_driveno, dest_path, dest,
	 &aflag, &bflag);

	/* Need at least source or source and destination		*/
	if(*src == '\0')
	{
		error_message(INV_NUM_PARAMS);
		GoHome();
		return FALSE;
	}

	/* Now start looking at what was passed.			*/
	/* First, substitute current drive if the source or destination	*/
	/* drive was not specified.					*/
	if(src_driveno < 0)
		src_driveno = DosGetDrive();
	if(dest_driveno < 0)
		dest_driveno = DosGetDrive();

	/* If the source path was not specified, get the current path	*/
	/* and substitute that.						*/
	if(*src_path == '\0')
	{
		*src_path = '\\';
		DosPwd(src_driveno + 1, (BYTE FAR *)&src_path[1]);
	}


	/* If no destination has been specified, build one ...		*/
	/* If the destination path was not specified, get the current	*/
	/* path and substitute that.					*/
	if(*dest_path == '\0')
	{
		/* No specification, build it as source in this		*/
		/* directory.						*/
		*dest_path = '\\';
		DosPwd(dest_driveno + 1, (BYTE FAR *)&dest_path[1]);

	}

	/* Now check for special case where dest is really a directory	*/
	if(*dest != '\0')
	{
		COUNT cudrvno;
		BYTE cudir[67];

		/* Get where we are					*/
		cudrvno = DosGetDrive();
		*cudir = '\\';
		DosPwd(cudrvno + 1, (BYTE FAR *)&cudir[1]);

		/* Change to the path and then test the file name. If	*/
		/* it is a directory, concatenate the source base name	*/
		/* to it to build a full path.				*/
		DosSetDrive(dest_driveno);
		if(DosCd((BYTE FAR *)dest_path) == SUCCESS)
		{
			/* This is a gimmick.  If the cd does not work,	*/
			/* the path is unaffected, so we let DOS do our	*/
			/* building of the path.			*/
			if(DosCd((BYTE FAR *)dest) != SUCCESS)
				AppendMode = TRUE;
			DosPwd(DosGetDrive() + 1, (BYTE FAR *)&dest_path[1]);
		}

		/* Change back to the current drive and directory.	*/
		DosSetDrive(cudrvno);
		if((DosCd((BYTE FAR *)cudir)) != SUCCESS)
		{
			error_message(INV_DIR);
			GoHome();
			return FALSE;
		}
	}

	/* Since we have to comply with wild card specifications, the	*/
	/* only way to expand the source is to switch there, do your	*/
	/* thing and then switch back.					*/

	/* Switch to the requested directory to copy from		*/
	DosSetDrive(src_driveno);

	/* Change to the path and then test the file name. If it is a	*/
	/* directory, concatenate the source base name to it to build a	*/
	/* full path.							*/
	if(DosCd((BYTE FAR *)src_path) != SUCCESS)
	{
		error_message(FILE_NOT_FOUND);
		GoHome();
		return FALSE;
	}

	/* Finally ! Expand the wild cards!				*/
	IsWild = iswild(src);
	if(DosFindFirst(D_NORMAL | D_RDONLY , (BYTE FAR *)src) != SUCCESS)
	{
		error_message(FILE_NOT_FOUND);
		GoHome();
		return TRUE;
	}

	do
	{
		BYTE *s;

		/* Build the from file specification and the to file	*/
		/* specification from above parts.			*/
		/* First the source					*/
		*FromPath = 'A' +  src_driveno;
		strcpy(&FromPath[1], ":");
		strcat(FromPath, src_path);
		if(FromPath[3] != '\0')
			strcat(FromPath, "\\");
		strcat(FromPath, dmp.dm_name);
		for(s = FromPath; *s != '\0'; s++)
			*s = toupper(*s);

		/* Then the destination.				*/
		if(!AppendMode || (AppendMode && FirstTime))
		{
			*ToPath = 'A' +  dest_driveno;
			strcpy(&ToPath[1], ":");
			strcat(ToPath, dest_path);
			if(ToPath[3] != '\0')
				strcat(ToPath, "\\");
			if(AppendMode)
				strcat(ToPath, dest);
			else
				strcat(ToPath, dmp.dm_name);
			for(s = ToPath; *s != '\0'; s++)
				*s = toupper(*s);
		}

		/* Check for a match between source and destination.	*/
		/* Quit if they match					*/
		if(strcmp(FromPath, ToPath) == 0)
		{
			if(AppendMode && !IsWild)
			{
				error_message(FILE_ON_ITSELF);
				if(AppendMode && !FirstTime)
					DosClose(ofd);
				GoHome();
			}
			else
                        	continue;
			return FALSE;
		}

		if(IsWild)
			printf("  %s\n", dmp.dm_name);
		if((ifd = DosOpen((BYTE FAR *)FromPath, O_RDONLY)) < 0)
		{
			strcpy(error_mess_str, FromPath);
			error_message(FILE_NOT_FOUND);
			if(AppendMode && !FirstTime)
				DosClose(ofd);
			GoHome();
			return FALSE;
		}
		if(!AppendMode || (AppendMode && FirstTime))
		{
			if((ofd = DosCreat((BYTE FAR *)ToPath, D_NORMAL)) < 0)
			{
				strcpy(error_mess_str, ToPath);
				error_message(FILE_CREATE_ERR);
				GoHome();
				DosClose(ifd);
				if(AppendMode && !FirstTime)
					DosClose(ofd);
				return FALSE;
			}
			FirstTime = FALSE;
		}

		/* Get a buffer from DOS				*/
		MemSeg = DosAllocMem(CHUNK / PARASIZE, (BOOL FAR *)&Error);
		if(Error)
		{
			error_message(INSUFF_MEM);
			GoHome();
			DosClose(ifd);
			if(AppendMode && !FirstTime)
				DosClose(ofd);
			return FALSE;
		}
		MemBuffer = MK_FP(MemSeg, 0);

                /* Dos the file copy itself				*/
		while((ret = DosRead(ifd, (BYTE FAR *)MemBuffer, CHUNK)) > 0)
		{
			if(DosWrite(ofd, (BYTE FAR *)MemBuffer, ret) != ret)
			{
				error_message(INSUFF_DISK);
				DosClose(ifd);
				DosClose(ofd);
				DosDelete((BYTE FAR *)ToPath);
				return FALSE;
			}
			if(ret != CHUNK)
				break;
		}

		/* Free the buffer					*/
		DosFreeMem(MemSeg, (BOOL FAR *)&Error);

		/* And update the file time if applicable		*/
		if(!AppendMode || (AppendMode && !IsWild))
		{
			DosGetftime(ifd, &fdate, &ftime);
			DosSetftime(ofd, &fdate, &ftime);
		}
		DosClose(ifd);
		++CopyCount;
		if(!AppendMode)
			DosClose(ofd);
	}
	while(DosFindNext() == SUCCESS);

	/* Change back to the current drive and directory.		*/
	if(AppendMode)
		DosClose(ofd);
	if(AppendMode)
		printf("\n%d File(s) copied\n", CopyCount);
	return GoHome();
}

