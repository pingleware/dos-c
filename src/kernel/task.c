/****************************************************************/
/*                                                              */
/*                           task.c                             */
/*                                                              */
/*                 Task Manager for DOS Processes               */
/*                                                              */
/*                      Copyright (c) 1995                      */
/*                      Pasquale J. Villani                     */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/****************************************************************/

#include "../../hdr/portab.h"
#include "globals.h"

/* $Logfile:   C:/dos-c/src/kernel/task.c_v  $ */
#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Header:   C:/dos-c/src/kernel/task.c_v   1.5   29 Aug 1996 13:07:22   patv  $";
#endif

/*
 * $Log:   C:/dos-c/src/kernel/task.c_v  $
 * 
 *    Rev 1.5   29 Aug 1996 13:07:22   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.4   29 May 1996 21:03:36   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.3   19 Feb 1996  3:21:48   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.2   01 Sep 1995 17:54:22   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:51:58   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:34:06   patv
 * Initial revision.
 */

#define LOADNGO 0
#define LOAD    1
#define OVERLAY 3

static exe_header header;

#define CHUNK 16384
#define MAXENV 32768

#ifndef PROTO
COUNT ChildEnv(exec_blk FAR *, UWORD *, char far *);
#else
COUNT ChildEnv();
#endif


#ifndef IPL
COUNT 
ChildEnv (exec_blk FAR *exp, UWORD *pChildEnvSeg, char far *pathname)
{
	BYTE FAR *pSrc;
	BYTE FAR *pDest;
	UWORD nEnvSize;
	COUNT RetCode;
	UWORD MaxEnvSize;
	psp FAR *ppsp = MK_FP( cu_psp, 0);

	/* create a new environment for the process             */
	/* copy parent's environment if exec.env_seg == 0       */

	pSrc = exp -> exec.env_seg ? 
			MK_FP(exp -> exec.env_seg, 0) :
			MK_FP(ppsp -> ps_environ, 0);
		   
	if (!pSrc)    /* no environment to copy */
	{
		*pChildEnvSeg = 0;
		return SUCCESS;
	}

	for(nEnvSize = 0; *pSrc != '\0'; )
	{
		while(*pSrc != '\0')
		{
			++pSrc;
			++nEnvSize;
		}
		/* account for terminating null         */
		++nEnvSize;
		++pSrc;
	}

	/* Test env size and abort if greater than max          */
	if(nEnvSize >= MAXENV)
		return DE_INVLDENV;

	/* allocate enough space for env + path                 */
	if((RetCode = DosMemAlloc(long2para(nEnvSize + 65), 
	  FIRST_FIT, (seg FAR *)pChildEnvSeg,
	  (UWORD FAR *)MaxEnvSize)) < 0)
		return RetCode;
	else
		pDest = MK_FP(*pChildEnvSeg + 1, 0);

	/* fill the new env and inform the process of its       */
	/* location throught the psp                            */
	pSrc = exp -> exec.env_seg ? 
			MK_FP(exp -> exec.env_seg, 0) :
			MK_FP(ppsp -> ps_environ, 0);
	
	/* copy the environment */
	for( ; *pSrc != '\0'; )
	{
		while (*pSrc) 
		{
			*pDest++ = *pSrc++;  
		} 
		pSrc++;
		*pDest++ = 0;
	}
	*pDest++ = 0;

	/* initialize 'extra strings' count */
	*pDest++ = 1;
	*pDest++ = 0;
	
	/* copy complete pathname */
	truename(pathname, pDest);
	
	return SUCCESS;
}

#endif


/* The following code is 8086 dependant                         */
VOID 
new_psp (psp FAR *p, int psize)
{
	REG COUNT i;
	BYTE FAR *lpPspBuffer;
	psp FAR *q = MK_FP(cu_psp, 0);

	/* Clear out new psp first                              */
	for(lpPspBuffer = p, i = 0; i < sizeof(psp) ; ++i)
		*lpPspBuffer = 0;
	
	/* initialize all entries and exits                     */
	/* CP/M-like exit point                                 */
	p -> ps_exit = 0x20cd;     /* bitte sch”n! */

	/* CP/M-like entry point - jump to special entry        */
	p -> ps_farcall= 0xea;
#ifndef IPL
	p -> ps_reentry = int21_entry;
#endif
	/* unix style call - 0xcd 0x21 0xcb (int 21, retf)      */
	p -> ps_unix[0] = 0xcd;
	p -> ps_unix[1] = 0x21; 
	p -> ps_unix[2] = 0xcb; 

	/* Now for parent-child relationships                   */
	/* parent psp segment                                   */
	p -> ps_parent = cu_psp;
	/* previous psp pointer                                 */
	p -> ps_prevpsp = q;

#ifndef IPL
	/* Environment and memory useage parameters             */
	/* memory size in paragraphs                            */
	p -> ps_size = psize;
	/* environment paragraph                                */
	p -> ps_environ = 0;
	/* process dta                                          */
	p -> ps_dta = (BYTE FAR *)(&p -> ps_cmd_count);
	
	/* terminate address                                    */
	p -> ps_isv22 = getvec(0x22);
	/* break address                                        */
	p -> ps_isv23 = getvec(0x23);
	/* critical error address                               */
	p -> ps_isv24 = getvec(0x24);
#endif

	/* File System parameters                               */
	/* user stack pointer - int 21                          */
	p -> ps_stack = (BYTE FAR *)-1;
	/* file table - 0xff is unused                          */
	for(i = 0; i < 20; i++)
		p -> ps_files[i] = 0xff;
	
	/* maximum open files                                   */
	p -> ps_maxfiles = 20;
	/* open file table pointer                              */
	p -> ps_filetab = p -> ps_files;

#ifndef IPL
	/* clone the file table                                 */
	if(InDOS > 0)
	{
		REG COUNT i;

		for(i = 0; i < 20; i++)
		{
			REG COUNT ret;

			if(q -> ps_filetab[i] != 0xff
			 && ((ret = CloneHandle(q -> ps_filetab[i])) >= 0))
				p -> ps_filetab[i] = ret;
			else
				p -> ps_filetab[i] = 0xff;
		}
	}
	else
	{
		/* initialize stdin, stdout, etc                        */
		p -> ps_files[STDIN] = 0;       /* stdin                */
		p -> ps_files[STDOUT] = 1;      /* stdout               */
		p -> ps_files[STDERR] = 2;      /* stderr               */
		p -> ps_files[STDAUX] = 3;      /* stdaux               */
		p -> ps_files[STDPRN] = 4;      /* stdprn               */
       }
#else
	/* initialize stdin, stdout, etc                        */
	p -> ps_files[STDIN] = 0;       /* stdin                */
	p -> ps_files[STDOUT] = 1;      /* stdout               */
	p -> ps_files[STDERR] = 2;      /* stderr               */
	p -> ps_files[STDAUX] = 3;      /* stdaux               */
	p -> ps_files[STDPRN] = 4;      /* stdprn               */
#endif

	/* first command line argument                          */
	p -> ps_fcb1.fcb_drive = 1;
	/* second command line argument                         */
	p -> ps_fcb2.fcb_drive = 2;

	/* local command line                                   */
	p -> ps_cmd_count = 0;  /* command tail                 */
	p -> ps_cmd[0] = 0;     /* command tail                 */
}


#ifndef IPL
static COUNT 
DosComLoader (BYTE FAR *namep, exec_blk FAR *exp, COUNT mode)
{
	COUNT rc, env_size, nread;
	UWORD mem;
	UWORD env, asize;
	BYTE FAR *sp, FAR *dp;
	psp FAR *p;
	mcb FAR *mp, FAR *envp = 0;
	iregs FAR *irp;
	LONG com_size;

	if(mode != OVERLAY)
	{
		if((rc = ChildEnv(exp, &env, namep)) != SUCCESS)
			return rc;

		/* Allocate our memory and pass back any errors         */
		if((rc = DosMemAlloc(0, LARGEST, (seg FAR *)&mem, (UWORD FAR *)&asize)) < 0)
		{
				if(env != 0)
			 DosMemFree(env);
				return rc;
		}
		mp = MK_FP(mem, 0);
		++mem;
		envp = MK_FP(env, 0);
	}
	else mem = exp -> load.load_seg;

	/* Now load the executable                              */
	/* If file not found - error                            */
	/* NOTE - this is fatal because we lost it in transit   */
	/* from DosExec!                                        */
	if((rc = dos_open(namep, 0)) < 0)
		fatal("(DosComLoader) com file lost in transit");
	/* do it in 32K chunks                                  */
	sp = MK_FP(mem, mode == OVERLAY? 0 : sizeof(psp));
	for(com_size = 65536l, nread = 0; com_size > 0; )
	{
		nread = dos_read(rc, sp, CHUNK);
		sp += nread;
		com_size -= nread;
		if(nread < CHUNK)
			break;
	}
	dos_close(rc);

	if(mode == OVERLAY)
		 return SUCCESS;

	/* point to the PSP so we can build it                  */
	
	p = MK_FP(mem, 0);
	new_psp(p, mem + asize);

	p -> ps_environ = env == 0 ? 0 : env + 1;

	/* complete the psp by adding the command line          */
	p -> ps_cmd_count = exp -> exec.cmd_line -> ctCount;
	fbcopy(exp -> exec.cmd_line -> ctBuffer, p -> ps_cmd, 127);
#if 0
	/* stick a new line on the end for safe measure         */
	p -> ps_cmd[p -> ps_cmd_count] = '\r';
#endif
	/* identify the mcb as this functions'                  */
	/* use the file name less extension - left adjusted and */
	/* space filled                                         */
	mp -> m_psp = mem;
	if (envp) envp -> m_psp = mem;   

	for(asize = 0; asize < 8; asize++)
	{
		if(namep[asize] != '.')
			mp -> m_name[asize] = namep[asize];
		else
			break;
	}
	for(; asize < 8; asize++)
			mp -> m_name[asize] = ' ';

	/* build the user area on the stack                     */
	irp = MK_FP(mem, (0xfffe - sizeof(iregs)));

	/* start allocating REGs                                */
	irp -> ES = irp -> DS = mem;
	irp -> CS = mem;
	irp -> IP = 0x100;
	irp -> AX = 0xffff; /* for now, until fcb code is in    */
	irp -> BX =
	irp -> CX =
	irp -> DX =
	irp -> SI =
	irp -> DI =
	irp -> BP = 0;
	irp -> FLAGS = 0x200;

	/* Transfer control to the executable                   */
	p -> ps_parent = cu_psp;
	p -> ps_prevpsp = (BYTE FAR *)MK_FP(cu_psp, 0);
	p -> ps_stack = (BYTE FAR *)user_r;
	user_r -> FLAGS &= ~FLG_CARRY;
	
	switch(mode)
	{
	case LOADNGO:
	{        
		cu_psp = mem;
		dta = p -> ps_dta;
		if (InDOS) --InDOS;
		exec_user(irp);
		/* We should never be here                      */
		fatal("KERNEL RETURNED!!!");
		break;
	}
	case LOAD:
		exp -> exec.stack = (BYTE FAR *)irp;
		exp -> exec.start_addr = MK_FP(irp -> CS, irp -> IP);
		return SUCCESS;
	}
}


VOID 
return_user (void)
{
	psp FAR *p, FAR *q;
	REG COUNT i;

	/* restore parent                                       */
	p = MK_FP(cu_psp, 0);

	for(i = 0; i < p -> ps_maxfiles; i++)
	{    
			DosClose(i);
	}

	/* When process returns - restore the isv               */
	setvec(0x22, p -> ps_isv22);
	setvec(0x23, p -> ps_isv23);
	setvec(0x24, p -> ps_isv24);

	/* And free all process memory if not a TSR return      */
	if(!tsr)
	{
		FreeProcessMem(cu_psp);
	}
	cu_psp = p -> ps_parent;
	q = MK_FP(cu_psp, 0);
	dta = q -> ps_dta;

	if (InDOS) --InDOS;
	exec_user(p -> ps_stack);
}
#endif

static COUNT 
DosExeLoader (BYTE FAR *namep, exec_blk FAR *exp, COUNT mode)
{
	COUNT rc, env_size, i, nBytesRead;
	UWORD mem, env, asize, start_seg;
	ULONG image_size;
	ULONG image_offset;
	BYTE FAR *sp, FAR * dp;
	psp FAR *p;
	mcb FAR *mp, FAR *envp = 0;
	iregs FAR *irp;
	UWORD reloc[2];
	seg FAR *spot;
	LONG exe_size;

#ifdef IPL
	BYTE szNameBuffer[64];
#endif

#ifndef IPL
	/* Clone the environement and create a memory arena     */
	if(mode != OVERLAY)
	{
		if((rc = ChildEnv(exp, &env, namep)) != SUCCESS)
			return rc;
	}
	else
		mem = exp -> load.load_seg;

	envp = MK_FP(env, 0);
#endif

	/* compute image offset from the header                 */
#ifdef IPL
	fscopy(namep, (BYTE FAR *)szNameBuffer);
	printf("\nEXE loader loading: %s", szNameBuffer);
#endif
	image_offset = (ULONG)header.exHeaderSize * 16l;

	/* compute image size by removing the offset from the   */
	/* number pages scaled to bytes plus the remainder and  */
	/* the psp                                              */
	/*  First scale the size                                */
	image_size = (ULONG)(header.exPages) * 512l;
#if 0
	/* remove the offset                                    */
	image_size -= image_offset;
	/* add in the remainder bytes                           */
	if(header.exExtraBytes != 0)
		image_size -= (ULONG)(512 - header.exExtraBytes);
#endif
	/* and finally add in the psp size                      */
	if(mode != OVERLAY)
		image_size += (ULONG)long2para((LONG)sizeof(psp));

#ifndef IPL
	if(mode != OVERLAY)
	{
		/* Now find out how many paragraphs are available       */
		if((rc = DosMemLargest((seg FAR *)&asize)) != SUCCESS)
			return rc;
		else
		{
			exe_size = (LONG)long2para(image_size) + header.exMinAlloc + long2para((LONG)sizeof(psp));
			if(exe_size > asize)
				return DE_NOMEM;
			else if(((LONG)long2para(image_size) + header.exMaxAlloc + long2para((LONG)sizeof(psp))) < asize)
				exe_size = (LONG)long2para(image_size) + header.exMaxAlloc + long2para((LONG)sizeof(psp));
			else
				exe_size = asize;
		}

		/* Allocate our memory and pass back any errors         */
		/* We can still get an error on first fit if the above  */
		/* returned size was a bet fit case                     */
		if((rc = DosMemAlloc((seg)exe_size, FIRST_FIT, (seg FAR *)&mem, (UWORD FAR *)&asize)) < 0)
		{
			if(rc == DE_NOMEM)
			{
				if((rc = DosMemAlloc(0, LARGEST, (seg FAR *)&mem, (UWORD FAR *)&asize)) < 0)
				{
					if(env != 0)
						DosMemFree(env);
					return rc;
				}
				/* This should never happen, but ...    */
				if(asize < exe_size)
				{
					if(env != 0)
						DosMemFree(env);
					DosMemFree(mem);
					return rc;
				}
			}
			else
			{
				if(env != 0)
					DosMemFree(env);
				return rc;
			}
		}
		else
		/* with no error, we got exactly what we asked for      */
			asize = exe_size;
	}
#else
	mem = KERNSEG;
#endif

#ifndef IPL
	if(mode != OVERLAY)
	{
#endif
		/* memory found large enough - continue processing      */
		mp = MK_FP(mem, 0);
		++mem;
#ifndef IPL
	}
	else
		mem = exp -> load.load_seg;
#endif

#ifdef IPL
	printf(".");
#endif
	/* create the start seg for later computations          */
	if(mode == OVERLAY)
		start_seg = mem;
	else
		start_seg = mem + long2para((LONG)sizeof(psp));

	/* Now load the executable                              */
	/* If file not found - error                            */
	/* NOTE - this is fatal because we lost it in transit   */
	/* from DosExec!                                        */
	if((rc = dos_open(namep, 0)) < 0)
		fatal("(DosExeLoader) exe file lost in transit");

#ifdef IPL
	printf(".");
#endif
	/* offset to start of image                             */
	if (dos_lseek(rc, image_offset, 0) != image_offset)
	{
#ifndef IPL
		if(mode != OVERLAY)
		{
			DosMemFree(--mem);
			if(env != 0)
				DosMemFree(env);
		}
#endif
		return DE_INVLDDATA;
	}

#ifdef IPL
	printf(".");
#endif

	/* read in the image in 32K chunks                      */
	if(mode != OVERLAY)
		exe_size = image_size - long2para((LONG)sizeof(psp));
	else
		exe_size = image_size;
	sp = MK_FP(start_seg, 0x0);
	while(exe_size > 0)
	{
		nBytesRead = dos_read((COUNT)rc, (VOID FAR *)sp, (COUNT)(exe_size < CHUNK ? exe_size : CHUNK));
		sp = add_far((VOID FAR *)sp, (ULONG)CHUNK);
		exe_size -= nBytesRead;
		if(nBytesRead == 0 || exe_size <= 0)
			break;
#ifdef IPL
		printf(".");
#endif
	}

#if 0
	/* Error if we did not read the entire image            */
	if(exe_size != 0)
		fatal("Broken exe loader (exe_size != 0)");
#endif

	/* relocate the image for new segment                   */
	dos_lseek(rc, (LONG)header.exRelocTable, 0);
	for (i=0; i < header.exRelocItems; i++)
	{
		if (dos_read(rc, (VOID FAR *)&reloc[0], sizeof(reloc)) != sizeof(reloc))
		{
			return DE_INVLDDATA;
		}
		if(mode == OVERLAY)
		{
			spot = MK_FP(reloc[1] + mem, reloc[0]);
			*spot = *spot + exp -> load.reloc;
		}
		else
		{
		/*      spot = MK_FP(reloc[1] + mem + 0x10, reloc[0]);*/
			spot = MK_FP(reloc[1] + start_seg, reloc[0]);
			*spot = *spot + start_seg;
		}
	}

#ifdef IPL
	printf(".");
#endif
	/* and finally close the file                           */
	dos_close(rc);

	/* exit here for overlay                                */
	if(mode == OVERLAY)
		return SUCCESS;

	/* point to the PSP so we can build it                  */
	p = MK_FP(mem, 0);   
	new_psp(p, mem + asize);

#ifndef IPL
	p -> ps_environ = env == 0 ? 0 : env + 1;
#else
	p -> ps_environ = 0;
#endif

	/* complete the psp by adding the command line          */
	p -> ps_cmd_count = exp -> exec.cmd_line -> ctCount;
	fbcopy(exp -> exec.cmd_line -> ctBuffer, p -> ps_cmd, 127);
#if 0        
	/* stick a new line on the end for safe measure         */
	p -> ps_cmd[p -> ps_cmd_count] = '\r';
#endif
	/* identify the mcb as this functions'                  */
	/* use the file name less extension - left adjusted and */
	/* space filled                                         */
	mp -> m_psp = mem;
	if (envp) envp -> m_psp = mem;

	for(i = 0; i < 8; i++)
	{
		if(namep[i] != '.')
			mp -> m_name[i] = namep[i];
		else
			break;
	}
	for(; i < 8; i++)
			mp -> m_name[i] = ' ';

	/* build the user area on the stack                     */
	irp = MK_FP(header.exInitSS + start_seg, ((header.exInitSP - sizeof(iregs)) & 0xffff));

#ifdef IPL
	printf(".\n");
#endif
	/* start allocating REGs                                */
	/* Note: must match es & ds memory segment              */
	irp -> ES = irp -> DS = mem;
	irp -> CS = header.exInitCS + start_seg;
	irp -> IP = header.exInitIP;
	irp -> AX = 0xffff; /* for now, until fcb code is in    */
#ifdef IPL
	irp -> BX = BootDrive;
	irp -> CX = NumFloppies;
#else
	irp -> BX =
	irp -> CX =
#endif
	irp -> DX =
	irp -> SI =
	irp -> DI =
	irp -> BP = 0;
	irp -> FLAGS = 0x200; 
					
	/* Transfer control to the executable                   */
	p -> ps_parent = cu_psp;
	p -> ps_prevpsp = (BYTE FAR *)MK_FP(cu_psp, 0);
	p -> ps_stack = (BYTE FAR *)user_r;
	user_r -> FLAGS &= ~FLG_CARRY;

#ifdef IPL
	printf("Starting kernel ...\n");
#endif
	switch(mode)
	{
	case LOADNGO:
		cu_psp = mem;
		dta = p -> ps_dta;
		if (InDOS) --InDOS;
		exec_user(irp);
		/* We should never be here                      */
		fatal("KERNEL RETURNED!!!");
		break;

#ifndef IPL
	case LOAD:
		exp -> exec.stack = (BYTE FAR *)irp;
		exp -> exec.start_addr = MK_FP(irp -> CS, irp -> IP);
		return SUCCESS;
#endif
	}
}


COUNT 
DosExec (COUNT mode, exec_blk FAR *ep, BYTE FAR *lp)
{
	COUNT rc, nNumRead;

	/* If file not found - free ram and return error        */
#ifdef IPL
	printf(".");
#endif

	if((rc = dos_open(lp, 0)) < 0)
		return DE_FILENOTFND;
	if((nNumRead = dos_read(rc, (VOID FAR *)&header, sizeof(exe_header)))
	 != sizeof(exe_header))
	{
#ifdef IPL
		printf("Internal IPL error - Read failure (read %d != %d)",
		 nNumRead, sizeof(exe_header));
#endif
		return DE_INVLDDATA;
	}

	dos_close(rc);
#ifdef IPL
	printf(".");
#endif

	if(header.exSignature != MAGIC)
#ifndef IPL
		return DosComLoader(lp, ep, mode);
#else
	{
		char szFileName[32];

		fmemcpy((BYTE FAR *)szFileName, lp);
		printf("\nFile: %s (MAGIC = %04x)", szFileName, header.exSignature);
		fatal("IPL can't load .com files!");
	}
#endif
	else
		return DosExeLoader(lp, ep, mode);
}


