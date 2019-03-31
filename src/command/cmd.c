/****************************************************************/
/*								*/
/*			       cmd.c				*/
/*								*/
/*		     command.com Top Level Driver 		*/
/*								*/
/*			  August 9, 1991			*/
/*								*/
/*		     Copyright (c) 1995, 1996			*/
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


/* $Logfile:   C:/dos-c/src/command/cmd.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/cmd.c_v  $ 
 * 
 *    Rev 1.3   29 Aug 1996 13:07:04   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.2   19 Feb 1996  3:18:54   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.1   01 Sep 1995 18:04:34   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:01:48   patv
 * Initial revision.
 */


#include <ctype.h>
#include "../../hdr/portab.h"
#define MAIN
#include "globals.h"
#include "proto.h"

static BYTE *RcsId = "$Header:   C:/dos-c/src/command/cmd.c_v   1.3   29 Aug 1996 13:07:04   patv  $";

#ifdef PROTO
struct table
{
	BYTE	*entry;
	BOOL	(*func)(COUNT, BYTE **);
};
#else
struct table
{
	BYTE	*entry;
	BOOL	(*func)();
};
#endif

#ifdef PROTO
struct table *lookup(struct table *, BYTE *);
VOID err_report(COUNT);
VOID put_prompt(BYTE *);
VOID Redirect(BYTE *, BYTE *, BYTE *, BOOL *);
VOID RestoreIO(COUNT, COUNT);
#else
struct table *lookup();
VOID err_report();
VOID put_prompt();
VOID Redirect();
VOID RestoreIO();
#endif

BOOL	ExecCmd();
BOOL	prompt();
BOOL	cmd_path();
BOOL	cmd_exit();
BOOL	type();
BOOL	cd();
BOOL	copy();
BOOL	del();
BOOL	ren();
BOOL	mkdir();
BOOL	rmdir();
BOOL	cmd_time();
BOOL	cmd_date();
BOOL	verify();
BOOL	ver();
BOOL	cmd_break();
BOOL 	batch();
BOOL	label_bat();
BOOL	pause_bat();
BOOL	call_bat();
BOOL	echo_bat();
BOOL	echo_dot_bat();
BOOL	for_bat();
BOOL	if_bat();
BOOL	rem_bat();
BOOL	shift_bat();
BOOL	goto_bat();
BOOL	set_bat();

/* External cmmands							*/

struct table  commands[] =
{
	{"break",	cmd_break},
	{"copy",	copy},
	{"cd",		cd},
	{"chdir",	cd},
	{"date",	cmd_date},
	{"del",		del},
	{"dir",		dir},
	{"erase",	del},
	{"exit",	cmd_exit},
	{"mkdir",	mkdir},
	{"md",		mkdir},
	{"path",	cmd_path},
	{"prompt",	prompt},
	{"ren",		ren},
	{"rmdir",	rmdir},
	{"rd",		rmdir},
	{"time",	cmd_time},
	{"type",	type},
	{"verify",	verify},
	{"ver",		ver},
	{"if",		if_bat},
	{"label",	label_bat},
	{"pause",	pause_bat},
	{"call",	call_bat},
	{"echo",	echo_bat},
	{"echo.",	echo_dot_bat},
	{"echo+",	echo_dot_bat},
	{"echo\"",	echo_dot_bat},
	{"echo/",	echo_dot_bat},
	{"echo[",	echo_dot_bat},
	{"echo]",	echo_dot_bat},
	{"echo:",	echo_dot_bat},
	{"for",		for_bat},
	{"rem",		rem_bat},
	{"shift",	shift_bat},
	{"goto",	goto_bat},
	{"set",		set_bat},
	{"",		ExecCmd}
};

static COUNT argc;
static BYTE *argv[NPARAMS];

static BOOL pflag, bootup = FALSE;

VOID main()
{
	COUNT nread;
	BOOL bool_FLAG = FALSE;
	BOOL cflag;
	BYTE FAR *cmd_tail;
	BYTE *p_ptr;
	extern UWORD _psp;
	psp FAR *p;
	COUNT driveno = -1;
	BYTE pattern[MAX_CMDLINE] = "";
	BYTE path[MAX_CMDLINE] = "", esize[MAX_CMDLINE] = "";

	/* Initialize the interpreter					*/
	p = MK_FP(_psp, 0);
	switchchar = '/';
	batch_FLAG = FALSE;
	argv[0] = args[0];
	argv[1] = (BYTE *)0;
	args[0][0] = '\0';
	*tail = '\0';
	env = (BYTE FAR *)MK_FP(p -> ps_environ, 0);
	cmd_tail = MK_FP(_psp, 0x81);
	fstrncpy((BYTE FAR *)tail, cmd_tail, 0x7f);

	pflag = cflag = FALSE;
	dosopt("$d$p*[e:pc]+", (BYTE FAR *)tail,
		&driveno, path, pattern, esize, &pflag, &cflag);

	/* Get the passed-in Environment size and make certain we	*/
	/* allocate enough space					*/
	EnvSize = EnvSizeUp();
	if(EnvSize < ENV_DEFAULT)
		EnvSize = ENV_DEFAULT;
	if(*esize != '\0')
	{
		COUNT size = atoi(esize);

		bool_FLAG = EnvAlloc(size);
		EnvSize = size;
	}
	else
		bool_FLAG = EnvAlloc(EnvSize);

	if(!bool_FLAG)
		error_message(OUT_ENV_SPACE);

	/* Check what PROMPT is set in env to over ride default 	*/
	p_ptr = EnvLookup("PROMPT");
	if(p_ptr != (BYTE *)0)
		scopy(p_ptr, prompt_string);
	else
		scopy(dflt_pr_string, prompt_string);

	/* Check what PATH is set in env to over ride default 		*/
	p_ptr = EnvLookup("PATH");
	if(p_ptr != (BYTE *)0)
		scopy(p_ptr, path);
	else
		scopy(dflt_path_string, path);
	if(!cflag)
	{
		if(pflag)
		{
			/* Special MS-DOS compatability initialization,	*/
			/* all command shells terminate onto		*/
			/* themselves, but we always terminate at the	*/
			/* root shell. If anyone complains, we'll	*/
			/* change it.					*/
#ifndef DEBUG
			p -> ps_parent = _psp;
#endif

			/* Try to exec autoexec.bat			*/
			bootup = TRUE;
			*tail = '\0';
			if(!batch(".\\autoexec.bat", tail))
			{
				*tail = '\0';
				cmd_date(1, argv);
				cmd_time(1, argv);
				bootup = FALSE;
			}
		}
		else
		{
			/* Announce our version				*/
			printf(ANNOUNCE, copyright);
#ifdef SHWR
			printf("**** Shareware version ****\nPlease register your copy.\n");
#else
			printf("\n\n");
#endif
		}

		FOREVER
		{
			default_drive = DosGetDrive();
			put_prompt(prompt_string);
			if((nread = DosRead(STDIN, (BYTE FAR *)cmd_line, MAX_CMDLINE)) < 0)
				continue;
			do_command(nread);
		}
	}
	else
	{
		BYTE FAR *p;

		default_drive = DosGetDrive();
		for(p = cmd_tail; *p != '\r'; p++)
		{
			if(*p == '/' && (*(p + 1) == 'c' || *(p + 1) == 'C'))
				break;
		}
		p += 2;
		fstrncpy((BYTE FAR *)cmd_line, p, 0x7f);
		for(nread = 0; *p != '\r'; nread++, p++)
			;
		++nread;
		do_command(nread);
	}
}


VOID Redirect(cmd_line, Input, Output, AppendMode)
BYTE *cmd_line, *Input, *Output;
BOOL *AppendMode;
{
	BYTE LocalBuffer[MAX_CMDLINE], *lp, *dp = cmd_line;

	/* First - create an image, since we'll be copying back into	*/
	/* the original buffer.						*/
	strcpy(LocalBuffer, cmd_line);

	/* Next, start looking for redirect symbols.			*/
	lp = skipwh(LocalBuffer);
	while(*lp != '\0')
	{
		switch(*lp)
		{
		case '<':
			lp = scan(++lp, Input);
			break;

		case '>':
			if(*(lp + 1) == '>')
			{
				++lp;
				*AppendMode = TRUE;
			}
			lp = scan(++lp, Output);
			break;

		default:
			*dp++ = *lp++;
			break;
		}
	}
	*dp = '\0';
}


VOID do_command(nread)
COUNT nread;
{
	REG struct table *p;
	REG BYTE *lp;
	COUNT index = 0;
	BYTE Input[MAX_CMDLINE], Output[MAX_CMDLINE];
	BOOL AppendMode;
	COUNT OldStdin = -1, OldStdout = -1, ErrorCode;
	BOOL IORedirected = FALSE;

	if(nread <= 0)
		return;
	cmd_line[nread] = '\0';

	/* Pre-scan the command line and look for any re-directs	*/
	*Input = *Output = '\0';
	AppendMode = FALSE;
	Redirect(cmd_line, Input, Output, &AppendMode);
	IORedirected = (*Input != '\0' || *Output != '\0');
	if(*Input != '\0')
	{
		COUNT Handle;

		if(!DosDupHandle(STDIN, (COUNT FAR *)&OldStdin, (COUNT FAR *)&ErrorCode))
		{
			RestoreIO(OldStdin, -1);
			error_message(INTERNAL_ERR);
			return;
		}

		Handle = DosOpen((BYTE FAR *)Input, O_RDWR);
		if((Handle < 0) || (!DosForceDupHandle(Handle, STDIN, (COUNT FAR *)&ErrorCode)))
		{
			RestoreIO(OldStdin, -1);
			error_message(INTERNAL_ERR);
			return;
		}
		DosClose(Handle);
	}

	if(*Output != '\0')
	{
		COUNT Handle;

		if(!DosDupHandle(STDOUT, (COUNT FAR *)&OldStdout, (COUNT FAR *)&ErrorCode))
		{
			RestoreIO(-1, OldStdout);
			error_message(INTERNAL_ERR);
			return;
		}

		if(AppendMode)
		{
			if((Handle = DosOpen((BYTE FAR *)Output, O_RDWR)) < 0)
			{
				RestoreIO(-1, OldStdout);
				error_message(INTERNAL_ERR);
				return;
			}
			DosSeek(Handle, 2, 0l);
		}
		else
			Handle = DosCreat((BYTE FAR *)Output, D_NORMAL | D_ARCHIVE);

		if((Handle < 0) || (!DosForceDupHandle(Handle, STDOUT, (COUNT FAR *)&ErrorCode)))
		{
			RestoreIO(-1, OldStdout);
			error_message(INTERNAL_ERR);
			return;
		}
		DosClose(Handle);
	}

	for(argc = 0; argc < 16; argc++)
	{
		argv[argc] = (BYTE *)0;
		args[argc][0] = '\0';
	}
	lp = scanspl(cmd_line, args[0], '/');

	if(args[0][0] == '@')
	{
		at_FLAG = TRUE;
		index++;
	}
	else
		at_FLAG = FALSE;

	/* If preceeded by a @, swallow it, it was taken care of	*/
	/* elsewhere.  Also, change case so that our command verb is	*/
	/* case sensitive.						*/
	while(args[0][index] != '\0')
	{

		if(at_FLAG)
			args[0][index-1] = tolower(args[0][index]);
		else
			args[0][index] = tolower(args[0][index]);
		index++;
	}
	if(at_FLAG)
		args[0][index-1] = '\0';

	argv[0] = args[0];
	/* this kludge is for an MS-DOS wart emulation */
	tail = skipwh(lp);

	for(argc = 1; argc < NPARAMS; argc++)
	{
		lp = scan(lp, args[argc]);
		if(*args[argc] == '\0')
			break;
		else
			argv[argc] = args[argc];
	}

	if(*argv[0] != '\0')
	{
		/* Look for just a drive change command, and execute	*/
		/* it if found.						*/
		if(argv[0][1] == ':' && argv[0][2] == NULL)
		{
			BYTE c = argv[0][0];

			if(c >= 'a' && c <= 'z')
				c = c - 'a' + 'A';
			if(c >= 'A' && c <= 'Z')
				DosSetDrive(c - 'A');
		}

		/* It may be a help command request.			*/
		else if((argv[1][0] == switchchar) &&
			 (argv[1][1] == '?'))
		{
			strcpy(tail, " ");
			strcat(tail, argv[0]);
			strcat(tail, "\r\n");
			argc = 2;
			argv[1] = argv[0];
			argv[0] = "help";
			argv[2] = 0;
			ExecCmd(argc, argv);
			if(IORedirected)
				RestoreIO(OldStdin, OldStdout);
		}
		/* do a normal command execution			*/
		else
		{
			p = lookup(commands, argv[0]);
			(*(p -> func))(argc, argv);
			if(IORedirected)
				RestoreIO(OldStdin, OldStdout);
		}
	}
}


BOOL prompt(argc, argv)
WORD argc;
BYTE *argv[];
{
	BYTE *p;
	BYTE *cmd = "PROMPT";

	if(argc == 1)
	{
		p = EnvLookup(cmd);
		if(p != (BYTE *)0)
			scopy(p, prompt_string);
		else
		{
			scopy(dflt_pr_string, prompt_string);
			EnvClearVar(cmd);
		}
	}
	else
	{
		/* Trim trailing newline				*/
		for(p = tail; (*p != '\r') && (*p != '\n'); p++)
			;
		*p = '\0';

		/* should be scopy(argv[1], &pr_string[1]); but to	*/
		/* emulate an MS-DOS wart, is				*/
		scopy(tail, prompt_string);

		/* Now set the environment variable for all children to	*/
		/* see.							*/
		EnvSetVar(cmd, prompt_string);
	}
	return TRUE;
}


struct table *lookup(p, token)
struct table *p;
BYTE *token;
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


VOID RestoreIO(DupStdin, DupStdout)
{
	COUNT ErrorCode;

	if(DupStdin >= 0)
	{
		if(!DosForceDupHandle(DupStdin, STDIN, (COUNT FAR *)&ErrorCode))
			error_message(INTERNAL_ERR);
		DosClose(DupStdin);
	}

	if(DupStdout >= 0)
	{
		if(!DosForceDupHandle(DupStdout, STDOUT, (COUNT FAR *)&ErrorCode))
			error_message(INTERNAL_ERR);
		DosClose(DupStdout);
	}
}


BOOL ExecCmd(argc, argv)
COUNT argc;
BYTE *argv[];
{
	exec_blk exb;
	COUNT err;
	BYTE tmppath[64];
	COUNT idx;
	BOOL ext = FALSE;
	BYTE *extp;
	COUNT len;
	BYTE *lp;
	CommandTail CmdTail;
	fcb fcb1, fcb2;
	static BYTE *extns[2] =
	{
		".com",
		".exe"
	};
	static BYTE *batfile = ".bat";
	BYTE PathString[MAX_CMDLINE];
	BYTE Path[MAX_CMDLINE], *pPath;

	/* Build the path string and create the full string that	*/
	/* includes ".\" so that the current directory is searched	*/
	/* first.  Note that Path is initialized outside the loop.	*/
	strcpy(Path, ".\\");
	strcpy(PathString, EnvLookup("PATH"));
	pPath = PathString;

	do
	{
		/* Build a path to the command.				*/
		if(*pPath == ';')
			++pPath;
		strcpy(tmppath, Path);
		if(*tmppath != '\0' && !((tmppath[strlen(tmppath) - 1] != '\\') == 0))
			strcat(tmppath, "\\");
		strcat(tmppath, argv[0]);

		/* batch processing					*/
		/* search for an extension in the specification		*/
		for(idx = len = strlen(argv[0]) ; idx > 0 && idx > (len - FEXT_SIZE - 2); --idx)
		{
			if(argv[0][idx] == '.')
			{
				ext = TRUE;
				extp = &argv[0][idx];
				break;
			}
		}

		/* If no extension was found, the entire path was	*/
		/* specified and we do not append an extension.		*/
		if(!ext)
		{
			strcat(tmppath, batfile);
			extp = batfile;
		}

		/* if it ends with a '.bat' (either user supplied or	*/
		/* previously added), try to run as a batch.		*/
		if((strcmp(extp, batfile) == 0) && batch(tmppath, tail))
		{
			if(pflag && bootup)
				bootup = FALSE;
			return TRUE;
		}

		/* tail comes in as a string with trailing newline. 	*/
		/* Convert it to a return only and put it into CmdTail	*/
		/* format						*/
		CmdTail.ctCount = (argc > 1) ? strlen(tail) : 1;
		strcpy(CmdTail.ctBuffer, " ");
		strcpy(&CmdTail.ctBuffer[1], (argc > 1) ? tail : "");
		CmdTail.ctBuffer[CmdTail.ctCount] = '\0';
		if(CmdTail.ctCount < LINESIZE)
			CmdTail.ctBuffer[CmdTail.ctCount] = '\0';
		rtn_errlvl = 0;
		exb.exec.env_seg = FP_SEG(env);
		exb.exec.cmd_line = (CommandTail FAR *)&CmdTail;

#if 0
		if(argc > 1)
		{
			DosParseFilename((BYTE FAR *)argv[1], (fcb FAR *)&fcb1, 0);
			exb.exec.fcb_1 = (fcb FAR *)&fcb1;
		}
		else
			exb.exec.fcb_1 = (fcb FAR *)0;
		if(argc > 2)
		{
			exb.exec.fcb_2 = (fcb FAR *)&fcb2;
			DosParseFilename((BYTE FAR *)argv[2], (fcb FAR *)&fcb2, 0);
		}
		else
			exb.exec.fcb_2 = (fcb FAR *)0;
#else
		exb.exec.fcb_1 = (fcb FAR *)0;
		exb.exec.fcb_2 = (fcb FAR *)0;
#endif

		for(idx = 0; idx < 2; idx++)
		{
			strcpy(tmppath, Path);
			if(*tmppath != '\0' && !((tmppath[strlen(tmppath) - 1] != '\\') == 0))
				strcat(tmppath, "\\");
			strcat(tmppath, argv[0]);
			if(!ext)
			{
				strcat(tmppath, extns[idx]);
				extp = extns[idx];
			}
			if(!(strcmp(extp, extns[idx]) == 0))
				continue;
			if((rtn_errlvl = err = DosExec((BYTE FAR *)tmppath, (exec_blk FAR *)&exb)) != SUCCESS)
			{
				switch(err)
				{
				case DE_FILENOTFND:
					continue;

				case DE_INVLDFUNC:
					rtn_errlvl = INV_FUNCTION_PARAM;
					goto errmsg;

				case DE_PATHNOTFND:
					rtn_errlvl = PATH_NOT_FOUND;
					goto errmsg;

				case DE_TOOMANY:
					rtn_errlvl = TOO_FILES_OPEN;
					goto errmsg;

				case DE_ACCESS:
					rtn_errlvl = ACCESS_DENIED;
					goto errmsg;

				case DE_NOMEM:
					rtn_errlvl = INSUFF_MEM;
					goto errmsg;

				default:
					rtn_errlvl = EXEC_ERR;
				errmsg:
					error_message(rtn_errlvl);
					return FALSE;
				}
			}
			else
			{
				rtn_errlvl = DosRtnValue() & 0xff;
				return TRUE;
			}
		}
		if(err < 0 || idx == 2)
		{
			if(!(err == DE_FILENOTFND || idx == 2))
			{
				error_message(EXEC_FAIL);
				return FALSE;
			}
			continue;
		}
	}
	while(*Path = '\0', pPath = scanspl(pPath, Path, ';'), *Path != '\0');
	error_message(BAD_CMD_FILE_NAME);
	return FALSE;
}


BOOL cmd_exit(argc, argv)
COUNT argc;
BYTE FAR *argv[];
{
#ifndef DEBUG
	/* Don't exit from a permanent shell				*/
	if(pflag)
		return TRUE;
#endif

	/* If no values passed, return errorvalue = 0			*/
	if(argc == 1)
		DosExit(0);

	/* otherwise return what the user asked for			*/
	else
	{
		COUNT ret_val;
		static BYTE nums[] = "0123456789";
		BYTE FAR *p;

		for(ret_val = 0, p = argv[1]; isdigit(*p); p++)
		{
			COUNT j;

			for(j = 0; j < 10; j++)
				if(nums[j] == *p)
					break;
			ret_val += j;
		}
		DosExit(ret_val);
	}
        return TRUE;
}


VOID sto(c)
COUNT c;
{
	BYTE ch[1];

	*ch = c;
	DosWrite(STDOUT, (BYTE FAR *)ch, 1);
}

