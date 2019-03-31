/****************************************************************/
/*								*/
/*			      echo.c				*/
/*								*/
/*		       command.com echo command 		*/
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


/* $Logfile:   C:/dos-c/src/command/echo.c_v  $*/

/*
 * $Log:   C:/dos-c/src/command/echo.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:06:52   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:36   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:01:52   patv
 * Initial revision.
 */


#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

static BYTE *RcsId = "$Header:   C:/dos-c/src/command/echo.c_v   1.2   29 Aug 1996 13:06:52   patv  $";

extern BOOL batch_FLAG;
extern BOOL echo_FLAG;

BOOL echo_bat(argc, argv)
COUNT argc;
BYTE *argv[];
{
	BYTE *lp, idx;

	if (argc == 1)
	{
		printf("ECHO is %s\n", echo_FLAG ? "ON" : "OFF");
		return TRUE;
	}
	else if (argc == 2)
	{
		BYTE *p;

		for(p = argv[1]; *p ; p++)
			*p = tolower(*p);

		if (strcmp(argv[1], "on") == 0)
		{
			echo_FLAG = TRUE;
			printf("ECHO is ON\n");
			return TRUE;
		}
		else if (strcmp(argv[1], "off") == 0)
		{
			echo_FLAG = FALSE;
			return TRUE;
		}
	}
	/* its user data						*/

	/* First trim leading white and trailing new line		*/
	lp = skipwh(tail);
	if(lp[(idx = strlen(lp))] == '\n')
		lp[idx] = '\0';

	/* Finally, print the user's data				*/
	printf("%s\n", lp);
	return TRUE;
}

BOOL echo_dot_bat()
{
	if (echo_FLAG)
		printf("\n");
	return TRUE;
}

