/****************************************************************/
/*								*/
/*			    globals.h				*/
/*								*/
/*	       Global data structures and declarations		*/
/*								*/
/*		     Copyright (c) 1995, 1996			*/
/*		   	Pasquale J. Villani			*/
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

/* $Logfile:   C:/dos-c/src/command/globals.h_v  $ */

/*
 * $Log:   C:/dos-c/src/command/globals.h_v  $
   
      Rev 1.3   29 Aug 1996 13:07:02   patv
   Bug fixes for v0.91b
   
      Rev 1.2   19 Feb 1996  3:18:58   patv
   Added NLS, int2f and config.sys processing
   
      Rev 1.1   01 Sep 1995 18:04:46   patv
   First GPL release.
   
      Rev 1.0   02 Jul 1995 10:02:34   patv
   Initial revision.
 */

#ifdef MAIN
static BYTE *Globals_hRcsId = "$Header:   C:/dos-c/src/command/globals.h_v   1.3   29 Aug 1996 13:07:02   patv  $";
#endif

#include "../../hdr/device.h"
#include "../../hdr/mcb.h"
#include "../../hdr/pcb.h"
#include "../../hdr/date.h"
#include "../../hdr/time.h"
#include "../../hdr/fat.h"
#include "../../hdr/fcb.h"
#include "../../hdr/kbd.h"
#include "../../hdr/tail.h"
#include "../../hdr/process.h"
#include "../../hdr/dcb.h"
#include "../../hdr/sft.h"
#include "../../hdr/exe.h"
#include "../../hdr/fnode.h"
#include "../../hdr/dirmatch.h"
#include "../../hdr/error.h"
#include "../../hdr/file.h"

/*									*/
/* Convience switch for maintaining variables in a single location	*/
/*									*/
#ifdef MAIN
#define GLOBAL
#else
#define GLOBAL extern
#endif

/*									*/
/* Convience definitions of TRUE and FALSE				*/
/*									*/
#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

/*									*/
/* Constants and macros							*/
/*									*/
/* Defaults and limits - System wide					*/
#define	NFILES		16		/* number of files in table	*/
#define	NDEVS		4		/* number of supported devices	*/
#define NAMEMAX		64		/* Maximum path for CDS		*/

/* 0 = CON, standard input, can be redirected				*/
/* 1 = CON, standard output, can be redirected				*/
/* 2 = CON, standard error						*/
/* 3 = AUX, auxiliary							*/
/* 4 = PRN, list device							*/
/* 5 = 1st user file ...						*/
#define	STDIN		0
#define	STDOUT		1
#define	STDERR		2
#define	STDAUX		3
#define	STDPRN		4

#define MAX_CMDLINE	128
#define MAX_LABELS	20
#define NPARAMS		16
#define LABELSIZE	 9
#define ENV_DEFAULT	256

/* some defines for protability from ASCII				*/
#define ESC_CHAR	0x1b

typedef struct
{
	BYTE	lb_name[LABELSIZE];
	LONG	lb_posit;
} symbol;

GLOBAL symbol labels[MAX_LABELS];


GLOBAL COUNT
	default_drive,		/* The current selected drive		*/
	label_cnt,		/* Number of labels found		*/
	shift_offset,		/* To determine %1 if shift was called 	*/
	EnvSize;		/* Startup environment size		*/


GLOBAL	BYTE
	dflt_path_string[MAX_CMDLINE], 	/* generates "No Path"		*/
	*path,
	FAR *env,
	prompt_string[MAX_CMDLINE],
	switchchar,
	*tail,
	cmd_line[MAX_CMDLINE],
	args[NPARAMS][MAX_CMDLINE],
	posparam[NPARAMS][MAX_CMDLINE];

GLOBAL BOOL
	at_FLAG,		/* @ at beginning of command line	*/
	batch_FLAG,
	dosoptplus;		/* TRUE if more non-white in tail	*/

GLOBAL BYTE *dflt_pr_string
#ifdef MAIN
 =
# ifdef SHWR
	"[Shareware DOS-C] $p$g ";
# else
	"$p$g ";
# endif
#else
 ;
#endif

#define ANNOUNCE "\nDOS-C Command Shell\n%s"

GLOBAL BYTE *copyright
#ifdef MAIN
="(C) Copyright 1995, 1996\nPasquale J. Villani\nAll Rights Reserved\n";
#else
;
#endif

GLOBAL BYTE *version
#ifdef MAIN
="DOS-C version %d.%02d";
#else
;
#endif
#define VERSION_MAJOR		0
#define VERSION_MINOR		91

/* Error messages							*/

#define MAX_ERR_STR_LEN		50	/* Maximum length of %s 	*/
#define MAX_ERR_MESS_LEN	80	/* Maximum length fixed message	*/

GLOBAL BYTE
	*error_mess_str;

GLOBAL COUNT
	rtn_errlvl;	/* error level returned from external command	*/

enum error_mess {
	ACCESS_DENIED = 0,
	BAD_CMD_FILE_NAME,
	BAD_CMD_INTERPRETER,
	BOOT_ERROR,
	CANNOT_REMOVE,
	CANNOT_DELETE,
	CANNOT_FIND_SYS_FILES,
	CANNOT_LOAD_COMMAND,
	CANNOT_READ_FAT,
	INV_NUM_PARAMS,
	INV_PARAM,
	INV_PARAM_COMBO,
	CURRENT_DRIVE_NUM,
	INV_DRIVE,
	INV_COMMAND,
	INV_CMD,
	INV_DIR,
	INV_TIME,
	INV_DATE,
	INV_DISK_CHANGE,
	INV_FILE,
	INV_FUNCTION_PARAM,
	INV_PARTITION,
	INV_PATH,
	INV_SWITCH,
	INV_SYNTAX,
	NOT_DIR,
	NOT_EMPTY_DIR,
	INV_KEYWORD,
	DATA_ERR,
	EXEC_ERR,
	OS_ERR,
	READ_DIR_ERR,
	READ_SYS_ERR,
	WRITE_DEVICE_ERR,
	EXEC_FAIL,
	BAD_FAT,
	FILE_ON_ITSELF,
	FILE_CREATE_ERR,
	FILE_NOT_FOUND,
	GEN_FAIL,
	DOS_VER,
	INSUFF_DISK,
	INSUFF_MEM,
	FILE_PIPE_ERR,
	INTERNAL_ERR,
	MALLOC_ERR,
	ON_OFF,
	NO_FIXED_DISK,
	NO_DIR_ROOM,
	NON_SYS_DISK,
	OUT_ENV_SPACE,
	PARAM_FORMAT,
	PARAM_NOT_ALLOWED,
	PARAM_RANGE_ERR,
	PARAM_NOT_COMPAT,
	PARSE_ERR,
	PATH_NOT_FOUND,
	PRINTER_ERR,
	PROG_TOO_BIG,
	READ_FAULT,
	REQ_PARAM,
	PARAM_TWICE,
	SECTOR_NOT,
	SEEK_ERR,
	SHARE_ERR,
	SEARCH_DIR_ERR,
	SYNTAX_ERR,
	TOO_FILES_OPEN,
	TOO_MANY_PARAMS,
	TOO_MANY_REDIRECTS,
	PARENT_ABORT,
	UNREC_CMD_CONFIG,
	UNRECOV_W_R,
	WRITE_FAULT,
	WRITE,
	INV_LABEL,
	LABEL_NOT_FOUND,
	DUP_FILE
}; /* end error message enum */

GLOBAL BYTE *error_messages[]
#ifdef MAIN
	= {
	"Access denied",
	"Bad command or file name",
	"Bad or missing command interpreter",
	"Boot error",
	"Cannot remove \"%s\"",
	"Cannot delete \"%s\"",
	"Cannot find system files", 
	"Cannot load command,system halted",
	"Cannot read file allocation table",
	"Invalid number of parameters",
	"Invalid parameter(s)",
	"Invalid parameter combination",
	"Current drive is no longer valid",
	"Invalid drive specification",
	"Invalid command.com",
	"Invalid command name",
	"Invalid directory",
	"Invalid time parameter",
	"Invalid date parameter",
	"Invalid disk change",
	"Invalid file name",
	"Invalid function parameter",
	"Invalid partion table",
	"Invalid path",
	"Invalid switch - %s",
	"Invalid syntax",
	"Not directory",
	"Directory not empty",
	"Invalid keyword",
	"Data error",
	"Error in exec file",
	"Error loading operating system",
	"Error reading directory",
	"Error reading system file",
	"Error writing to device",
	"Exec failure",
	"File allocation table bad",
	"File cannot not be copied onto itself",
	"File creation error - \"%s\"",
	"File not found",
	"General failure",
	"Incorrect DOS version",
	"Insufficient disk space",
	"Insufficient memory",
	"Intermediate file error during pipe",
	"Internal error",
	"Memory allocation error",
	"Must specify ON or OFF",
	"No fixed disks present",
	"No room in directory",
	"Non-system disk or disk error",
	"Out of environment space",
	"Parameter format not correct",
	"Parameter value not allowed",
	"Parameter value not in allowed range",
	"Parameter not compatible",
	"Parse error",
	"Path not found",
	"Printer error",
	"Program too big to fit in memory",
	"Read fault error",
	"Required parameter missing",
	"Same parameter entered twice",
	"Sector not found",
	"Seek error",
	"Sharing violation",
	"Specified command search directory bad",
	"Syntax error",
	"Too many files open",
	"Too many parameters",
	"Too many redirections",
	"Top level process aborted, cannot continue",
	"Unrecognized command in config.sys",
	"Unrecoverable read or write error",
	"Write fault error",
	"Write protection error",
	"Invalid label",
	"Label not found",
	"Duplicate file name or file not found"
};
#else
	;
#endif
