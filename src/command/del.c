/****************************************************************/
/*								*/
/*			      del.c				*/
/*								*/
/*		      command.com "del" Command 		*/
/*								*/
/*			 November 6, 1991			*/
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

/* $Logfile:   C:/dos-c/src/command/del.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/del.c_v  $
 * 
 *    Rev 1.2   29 Aug 1996 13:06:58   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:30   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:01:26   patv
 * Initial revision.
 */


#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

static BYTE *RcsId = "$Header:   C:/dos-c/src/command/del.c_v   1.2   29 Aug 1996 13:06:58   patv  $";


/* Local convienence define						*/
#define FileDelete(file)	(DosDelete((BYTE FAR *)file) == SUCCESS)


BOOL del()
{
	dmatch dmp;
	BYTE pattern[MAX_CMDLINE] = "";
	BYTE path[MAX_CMDLINE] = "";
	BYTE target_path[MAX_CMDLINE];
	BOOL pflag = FALSE;
	COUNT cudrvno, driveno, args, deleted = 0;

	/* Get the current directory, so that we can return later	*/
	FindHome();

	/* Get the command line options					*/
	args = dosopt("$d$p*[p]+", (BYTE FAR *)tail,
	 &driveno, path, pattern, &pflag);
	if(args < 1 || strcmp(pattern,"") == 0)
	{
		error_message(REQ_PARAM);
		GoHome();
		return FALSE;
	}

	/* Check for the "*.*" pattern and issue a warning if found	*/
	if(strcmp(pattern,"*.*") == 0)
	{
		BYTE line[MAX_CMDLINE] = "", *resp;

		FOREVER
		{
			printf("All files in directory will be deleted!\nAre you sure (Y/N)?");
			DosRead(STDIN, (BYTE FAR *)line, MAX_CMDLINE);

			resp = skipwh(line);
			if(*resp == 'n' || *resp == 'N')
			{
				GoHome();
				return TRUE;
			}
			else if(*resp == 'y' || *resp == 'Y')
				break;
			else
				continue;
		}
	}

	/* Set the dta to a local area.					*/
	DosSetDta((BYTE FAR *)&dmp);

	/* And switch to the new directory.				*/
	/* Get the current drive, so we can switch back.       		*/
	cudrvno = DosGetDrive();

	/* Switch to the requested directory.				*/
	DosSetDrive(driveno < 0 ? cudrvno : driveno);
	*target_path = '\\';
	DosPwd(DosGetDrive() + 1, (BYTE FAR *)&target_path[1]);

	/* Change to the path and then test the file name. If it is a	*/
	/* directory, concatenate the source base name to it to build a	*/
	/* full path.							*/
	if(*path != '\0' && DosCd((BYTE FAR *)path) != SUCCESS)
	{
		error_mess_str = path;
		error_message(PATH_NOT_FOUND);
		GoHome();
		return FALSE;
	}

	/* Look for the first (and maybe only) occurrance of the user	*/
        /* specified pattern.						*/
	if(DosFindFirst(D_DIR, (BYTE FAR *)pattern) != SUCCESS)
	{
		error_mess_str = pattern;
		error_message(FILE_NOT_FOUND);
		GoHome();
		return TRUE;
	}

	/* Now follow standard DOS convention and loop through the	*/
	/* directory, expanding wild cards				*/
	do
	{
		if(!(dmp.dm_attr_fnd & D_DIR))
		{
			/* Delete each file, and prompt if /p option	*/
			/* was issued.					*/
			if(pflag)
			{
				BYTE line[MAX_CMDLINE] = "", *resp;

				FOREVER
				{
					printf("Delete %s (Y/N)?", dmp.dm_name);
					DosRead(STDIN, (BYTE FAR *)line, MAX_CMDLINE);

					resp = skipwh(line);
					if(*resp == 'n' || *resp == 'N')
						break;
					else if(*resp == 'y' || *resp == 'Y')
					{
						if(!FileDelete(dmp.dm_name))
							error_message(ACCESS_DENIED);
						break;
					}
					else
						continue;
				}
			}
			else
			/* Just delete each file found.			*/
			{
				if(FileDelete(dmp.dm_name))
					++deleted;
			}
		}
	}
	while(DosFindNext() == SUCCESS);

	/* Here's an oddball.  If we found a file, but could not delete	*/
	/* it, we need to warn the user.  So here it is.		*/
	if(!pflag && deleted < 1)
		error_message(ACCESS_DENIED);

	DosCd((BYTE FAR *)target_path);
	return GoHome();
}

