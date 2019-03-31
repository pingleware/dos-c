/****************************************************************/
/*								*/
/*			     batch.c				*/
/*								*/
/*		      command.com Batch Support 		*/
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


/* $Logfile:   C:/dos-c/src/command/batch.c_v  $*/

/*
 * $Log:   C:/dos-c/src/command/batch.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:06:58   patv
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

static BYTE *RcsId = "$Header:   C:/dos-c/src/command/batch.c_v   1.2   29 Aug 1996 13:06:58   patv  $";

#define CTL_Z 26
#define NUM_PASSES 2	/* Currently a 2 pass interpreter, pass 1 for labels */
			/* pass 2 for execution				     */

BYTE *cursor;
WORD fileptr;
BOOL echo_FLAG;

BOOL batch(file)
BYTE *file;
{
	COUNT idx;
	COUNT file_parse();
	BOOL parse();

	/* check to see if currently processing a batch file 		*/
	/* if so clean up 						*/
	if(batch_FLAG)
	{
		COUNT fp;

		if((fp = DosOpen((BYTE FAR *)file, O_RDONLY)) < 0)
			return FALSE;
		else
			DosClose(fp);
		batch_FLAG = FALSE;
		for(idx = 0; idx < NPARAMS; idx++)
			posparam[idx][0] = '\0';
		for(label_cnt = 0; label_cnt < MAX_LABELS; label_cnt++)
		{
			labels[label_cnt].lb_name[0] = '\0';
			labels[label_cnt].lb_posit = '\0';
		}
		DosClose(fileptr);
	}

	/* open batch file for reading					*/
	if((fileptr = DosOpen((BYTE FAR *)file, O_RDONLY)) < 0)
	{
		batch_FLAG = FALSE;
		return FALSE;
	}

	/* Ok, now set mode to batch and initialize positional		*/
	/* parameter array.						*/
	echo_FLAG = TRUE;
	batch_FLAG = TRUE;
	shift_offset = 0;
	for(idx = 0; idx < NPARAMS; idx++)
		strcpy(posparam[idx], args[idx]);

	/* deal with command line */
	file_parse(fileptr);
	return TRUE;
}

COUNT file_parse(fileptr)
WORD fileptr;
{
	WORD rc_FLAG;
	COUNT nread;
	COUNT pass;
	BYTE line[MAX_CMDLINE];
	BYTE *p;

	for(pass = 1; pass <= NUM_PASSES; pass++)
	{
		default_drive = DosGetDrive();
		do
		{
			BOOL eof = FALSE, eol = FALSE;

			for(*line = 0, p = line, nread = 0;
				 !eol && !eof && (nread < MAX_CMDLINE); )
			{
				if(DosRead(fileptr, (BYTE FAR *)p, 1) != 1)
				{
					eof = TRUE;
					break;
				}
				switch(*p)
				{
				case CTL_Z:
				case '\r':
					*p = '\0';
					eol = TRUE;
					break;

				case '\n':
					*p = '\0';
					continue;

				default:
					++nread;
					++p;
					break;
				}
			}

			expand(cmd_line, line);

			default_drive = DosGetDrive();
			p = skipwh(cmd_line);

			/* dummy nread to get by a blank line		*/
			if(pass > 1 && *p == '\0')
			{
				nread = 1;
				if(echo_FLAG)
				{
					if(eof)
						printf("\n");
					put_prompt(prompt_string);
					printf("\n");
				}
				if(!eof)
					continue;
				else
                                	break;
			}

			if(echo_FLAG && pass > 1 && *p != '@' && *p != ':' && nread > 0)
			{
				printf("\n");
				put_prompt(prompt_string);
				printf("%s\n", cmd_line);
			}

			if((rc_FLAG = parse(cmd_line, strlen(cmd_line), pass)) != TRUE)
			{
				printf("ERROR parsing batch file\n");
				return (rc_FLAG);
			}
		} while (nread > 0);
		DosSeek(fileptr, 0, 0L);
	}

	DosClose(fileptr);
	batch_FLAG = FALSE;
	return TRUE;
}

BOOL parse(line, nread, pass)
BYTE line[];
COUNT nread;
COUNT pass;
{
	cursor = skipwh(line);

	if(pass == 1)
	{
		if(*cursor == ':')
			return label_bat(++cursor);
		else
			return TRUE;
	}

	switch(*cursor) {
		case '@':
			at_FLAG = TRUE;
			cursor = skipwh(++cursor);
			do_command(nread);
			at_FLAG = FALSE;
			break;
		case ':':
			/* labels processed in pass 1			*/
			break;
		default:
			do_command(nread);
			break;
	}
return TRUE;
}

