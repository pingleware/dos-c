/****************************************************************/
/*								*/
/*			      dir.c				*/
/*								*/
/*		        DOS "dir" Command 			*/
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


/* $Logfile:   C:/dos-c/src/command/dir.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/dir.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:07:00   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:32   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:01:40   patv
 * Initial revision.
 */


#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

static BYTE *RcsId = "$Header:   C:/dos-c/src/command/dir.c_v   1.2   29 Aug 1996 13:07:00   patv  $";


#ifdef PROTO
COUNT chk_line(COUNT);
#else
COUNT chk_line();
#endif


#define D_ALL	D_NORMAL | D_RDONLY | D_HIDDEN | D_SYSTEM | D_DIR | D_ARCHIVE


BOOL dir(argc, argv)
COUNT argc;
BYTE *argv[];
{
	WORD count = 0, disp_line, columns = 0;
	ULONG total = 0l;
	BYTE vid[67], *ext;
	BYTE cudir[67];
	BYTE sdrive[2];
	dmatch dmp;
	COUNT at_mask, at_pat;
	COUNT free_clst, sec_size, clst_size, n_clst;
	BOOL wflag, pflag, lflag, bflag;
	COUNT driveno = -1, cudrvno;
	BYTE pattern[MAX_CMDLINE] = "", formatted[MAX_CMDLINE];
	BYTE path[MAX_CMDLINE] = "", sopt[MAX_CMDLINE] = "";

	/* parse for options						*/
	wflag = pflag = lflag = bflag = count = FALSE;
	dosopt("$d$p*[a:wplb]+", (BYTE FAR *)tail,
		&driveno, path, pattern, sopt, &wflag, &pflag, &lflag, &bflag);

	/* Set defaults for file name and path if not supplied.		*/
	if(strcmp(path, "") == 0)
		strcpy(path, ".");
	if(strcmp(pattern,"") == 0)
		strcpy(pattern,"*.*");

	/* Intialize the dta to put the dir info into			*/
	DosSetDta((BYTE FAR *)&dmp);

	/* Get the current drive, so we can switch back.		*/
	cudrvno = DosGetDrive();

	/* Switch to the requested directory to list			*/
	DosSetDrive(driveno < 0 ? cudrvno : driveno);

	/* Get the current directory. Note that the DOS system call	*/
	/* does not return drive or leading '\', so we need to add	*/
	/* them.							*/
	cudir[0] = '\\';
	DosPwd(DosGetDrive() + 1, (BYTE FAR *)&cudir[1]);

	/* Change to the path and then test the file name. If it is a	*/
	/* directory, switch to it, because the user wants a listing of	*/
	/* its content.							*/
	if(DosCd((BYTE FAR *)path) != SUCCESS)
	{
		error_message(INV_DIR);
		DosSetDrive(cudrvno);
		return FALSE;
	}
	if(!iswild(pattern))
	{
		if(DosFindFirst(D_DIR, (BYTE FAR *)pattern) != SUCCESS)
		{
			error_message(FILE_NOT_FOUND);
			return TRUE;
		}
		else
		{
			if(dmp.dm_attr_fnd & D_DIR)
			{
				if(DosCd((BYTE FAR *)pattern) != SUCCESS)
				{
					error_message(INV_DIR);
					DosSetDrive(cudrvno);
					return FALSE;
				}
				else
					strcpy(pattern, "*.*");
			}
		}
	}


	/* Get the new directory.					*/
	path[0] = '\\';
	DosPwd(DosGetDrive() + 1, (BYTE FAR *)&path[1]);

	/* Intialize the dta to put the dir info into			*/
	DosSetDta((BYTE FAR *)&dmp);

	/* Get the volume label				*/
	if(DosFindFirst(D_VOLID, (BYTE FAR *)"*.*") != SUCCESS)
		sprintf(vid, "has no label");
	else
	{
		for(ext = dmp.dm_name; *ext != NULL; ext++)
			if(*ext == '.')
			{
				*ext++ = NULL;
				break;
			}
		sprintf(vid, "is %s%s", dmp.dm_name, ext);
	}

	/* Display the header, if not turned off			*/
	sdrive[0] = 'A' + DosGetDrive();
	sdrive[1] = NULL;
	if(bflag)
		disp_line = 0;
	else
		disp_line = 4;
	if(!bflag)
	{
		printf("\n Volume in drive %s %s\n", sdrive, vid);
		printf(" Directory of %s:%s\n\n", sdrive, path);
	}

	/* Build the attribute mask and pattern				*/
	at_mask = *sopt == '\0' ? D_RDONLY | D_HIDDEN | D_SYSTEM : 0;
	at_pat = 0;
	for(ext = sopt; *ext != '\0'; ++ext)
	{
		if(*ext == '-')
		{
			++ext;
			switch(tolower(*ext))
			{
			case 'a':
				at_mask |= D_ARCHIVE;
				at_pat &= ~D_ARCHIVE;
				break;

			case 'd':
				at_mask |= D_DIR;
				at_pat &= ~D_DIR;
				break;

			case 'r':
				at_mask |= D_RDONLY;
				at_pat &= ~D_RDONLY;
				break;

			case 'h':
				at_mask |= D_HIDDEN;
				at_pat &= ~D_HIDDEN;
				break;

			case 's':
				at_mask |= D_SYSTEM;
				at_pat &= ~D_SYSTEM;
				break;
			}
		}
		else
		{
			switch(tolower(*ext))
			{
			case 'a':
				at_mask |= D_ARCHIVE;
				at_pat |= D_ARCHIVE;
				break;

			case 'd':
				at_mask |= D_DIR;
				at_pat |= D_DIR;
				break;

			case 'r':
				at_mask |= D_RDONLY;
				at_pat |= D_RDONLY;
				break;

			case 'h':
				at_mask |= D_HIDDEN;
				at_pat |= D_HIDDEN;
				break;

			case 's':
				at_mask |= D_SYSTEM;
				at_pat |= D_SYSTEM;
				break;
			}
		}
	}

	if(DosFindFirst(D_ALL, (BYTE FAR *)pattern) != SUCCESS)
	{
		error_message(FILE_NOT_FOUND);
		return TRUE;
	}

	if(wflag)
	{
		do
		{
			char *p;

			if((*sopt != 0) && !((dmp.dm_attr_fnd & at_mask) == at_pat))
				continue;
			if(dmp.dm_attr_fnd & D_DIR)
			{
				if(bflag && *dmp.dm_name == '.')
					continue;
				sprintf(formatted, "[%s]", dmp.dm_name);
				if(lflag)
					strlwr(formatted);
				printf("%-14s ", formatted);
				++columns;
				if(columns == 5)
				{
					if(pflag)
						disp_line = chk_line(disp_line);
					printf("\n");
					columns = 0;
				}
			}
			else
			{
				ext = "   ";
				sprintf(formatted, "%s", dmp.dm_name);
				if(lflag)
					strlwr(formatted);
				printf("%-15s",formatted);
				++columns;
				if(columns == 5)
				{
					if(pflag)
						disp_line = chk_line(disp_line);
					printf("\n");
					columns = 0;
				}
			}
		++count;
		total += dmp.dm_size;
		}
		while(DosFindNext() == SUCCESS);
	}
	else
	{
		do
		{
			WORD hour = TM_HOUR(dmp.dm_time);

			if((*sopt != 0) && !((dmp.dm_attr_fnd & at_mask) == at_pat))
				continue;
			if(dmp.dm_name[0] == '.')
				ext = "";
			else
				for(ext = dmp.dm_name; *ext != NULL; ext++)
				{
					if(*ext == '.')
					{
						*ext++ = NULL;
						break;
					}
				}
			if(lflag)
			{
				strlwr(dmp.dm_name);
				strlwr(ext);
			}
			if(dmp.dm_attr_fnd & D_DIR)
			{
				if(bflag && *dmp.dm_name == '.')
					continue;
				if(pflag)
					disp_line = chk_line(disp_line);
				printf(bflag ? "\n%s.%s" : "\n   %8s %3s  <DIR>      %-2d-%-02d-%-02d  %-2d:%-02d%s",
					dmp.dm_name, ext,
					DT_MONTH(dmp.dm_date),
					DT_DAY(dmp.dm_date),
					(DT_YEAR(dmp.dm_date) + 1980) % 100,
					hour > 12 ? hour - 12 : (hour == 0) ? 12 : hour,
					TM_MIN(dmp.dm_time),
					hour >= 12 ? "p" : "a");
			}
			else
			{
				if(pflag)
					disp_line = chk_line(disp_line);
				printf(bflag ? "\n%s.%s" : "\n   %8s %3s %-10ld  %-2d-%-02d-%-02d  %-2d:%-02d%s",
					dmp.dm_name, ext, dmp.dm_size,
					DT_MONTH(dmp.dm_date),
					DT_DAY(dmp.dm_date),
					(DT_YEAR(dmp.dm_date) + 1980) % 100,
					hour > 12 ? hour - 12 : (hour == 0) ? 12 : hour,
					TM_MIN(dmp.dm_time),
					hour >= 12 ? "p" : "a");
			}
			++count;
			total += dmp.dm_size;
		}
		while(DosFindNext() == SUCCESS);
	}

	/* /b does not print any statistics				*/
	if(bflag)
	{
		printf("\n\n");
	}
	else
	/* Now print the available free bytes (It's really clusters	*/
	/* translated to bytes.						*/
	{
		DosFree(0, (COUNT FAR *)&clst_size,(COUNT FAR *) &free_clst, (COUNT FAR *)&sec_size, (COUNT FAR *)&n_clst);
		printf("\n  %-10d file(s)   %-10ld bytes\n", count, total);
		printf(  "                       %-10ld bytes free\n\n",
			(ULONG)free_clst
			* (ULONG)sec_size
			* (ULONG)clst_size);
	}

	/* Change back to the current drive and directory, so that we	*/
	/* look like we just did a listing without all the gymnastics.	*/
	if((DosCd((BYTE FAR *)cudir)) != SUCCESS)
	{
		DosSetDrive(cudrvno);
		error_message(INV_DIR);
		return FALSE;
	}
	else
	{
		DosSetDrive(cudrvno);
		return TRUE;
	}
}

static COUNT chk_line(disp_line)
COUNT disp_line;
{
	BYTE line[MAX_CMDLINE];

	if(disp_line == 23)
	{
		printf("\nStrike a key when ready . . .");
		DosRead(STDIN, line, MAX_CMDLINE);
		return 0;
	}
	return ++disp_line;
}




