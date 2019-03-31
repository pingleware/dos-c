/****************************************************************/
/*								*/
/*			      helpexe.c				*/
/*								*/
/*		      DOS External "help" Command 		*/
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


/* $Logfile:   C:/dos-c/src/command/helpexe.c_v  $ */


/*
 * $Log:   C:/dos-c/src/command/helpexe.c_v  $
 * 
 *    Rev 1.2   29 Aug 1996 13:06:54   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:38   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:02:02   patv
 * Initial revision.
 */

static char *RcsId = "$Header:   C:/dos-c/src/command/helpexe.c_v   1.2   29 Aug 1996 13:06:54   patv  $";

#include <stdio.h>
#include <dos.h>
#include <ctype.h>
#include <string.h>
#include "../../hdr/portab.h"
#include "../../hdr/error.h"
#include "globals.h"

#define HELP_FILE "./helpfile"
#define HELP_FILE_DELIMITER "===\n"

VOID main(argc, argv)
COUNT argc;
BYTE **argv;
{
	FILE *helpptr;
	BYTE line[MAX_CMDLINE];
	BYTE cmd_marker[20]; /* delimiter + command */
	BYTE *help_str = "dummy_dos";
	REG COUNT i = 0;
	COUNT c;
	BOOL got_one;

	if(argc == 1)
	{
		if((helpptr = fopen(HELP_FILE,"r")) == NULL)
		{
			printf("\nSorry, help data file is missing\n");
			return;
		}

		while((c = fgetc(helpptr)) != '\f')
			;
		while(fgets(line,MAX_CMDLINE,helpptr) != '\0')
			printf("%s",line);

		fclose(helpptr);
		return;
	}

	if(argc > 3)
	{
		printf("\nUsage: help command_name or command_name \\? \n");
		return;
	}

	help_str = argv[1];

	if((helpptr = fopen(HELP_FILE,"r")) == NULL)
	{
		printf("help -- provide brief command explanations\nUsage:  help [command] or command \\?\n\nSorry, help data file is missing\n");
		return;
	}

	while(help_str[i] != '\0')
	{
		help_str[i]=toupper(help_str[i]);
		i++;
	}

	sprintf(cmd_marker,"%s%s",help_str,HELP_FILE_DELIMITER);
	got_one = FALSE;
	while(fgets(line, MAX_CMDLINE, helpptr), *line != '\f')
	{
		if(got_one)
		{
			fclose(helpptr);
			break;
		}

		if(strcmp(line,cmd_marker) == 0)
		{
			got_one = TRUE;
			while((fgets(line,MAX_CMDLINE,helpptr) != NULL) &&
				 (strcmp(line,".\n") != 0))
					printf("%s",line);
		}
	}

	if(!got_one)
		printf("\nhelp -- no help information for command: %s\n",help_str);
	return;
}
