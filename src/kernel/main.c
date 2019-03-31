
/****************************************************************/
/*                                                              */
/*                           main.c                             */
/*                            DOS-C                             */
/*                                                              */
/*                    Main Kernel Functions                     */
/*                                                              */
/*                   Copyright (c) 1995, 1996                   */
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


#define MAIN
#include "../../hdr/portab.h"
#include "globals.h"

/* $Logfile:   D:/dos-c/src/kernel/main.c_v  $ */
#ifdef VERSION_STRINGS
static BYTE *mainRcsId = "$Header:   D:/dos-c/src/kernel/main.c_v   1.3   29 May 1996 21:03:32   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/kernel/main.c_v  $
 * 
 *    Rev 1.3   29 May 1996 21:03:32   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.2   19 Feb 1996  3:21:36   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.1   01 Sep 1995 17:54:18   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995  8:33:18   patv
 * Initial revision.
 */

static COUNT BlockIndex = 0;


VOID init_kernel(VOID), signon(VOID), kernel(VOID);
static COUNT p_0(VOID);
VOID FsConfig(VOID);

VOID 
main (void)
{
	init_kernel();
#ifdef DEBUG
	printf("KERNEL: Boot drive = %d\n", BootDrive);
#endif
	signon();
	kernel();
}



VOID 
reinit_k (void)
{
	REG COUNT i;
	UWORD FAR *stack = (UWORD FAR *)&error_tos;

	/* Re-initialize drivers                        */
	syscon = &con_dev;

	/* Re-initialize stacks                         */
	api_sp = FP_OFF(stack);
	api_ss = FP_SEG(stack);

	/* Test ram for sane mcb's                      */
	if(DosMemCheck() != SUCCESS)
		panic("Memory corrupted");
}


static VOID 
init_kernel (void)
{
	REG struct dhdr FAR *dhp = (struct dhdr FAR *)&nul_dev;
	UWORD FAR *stack;
	COUNT i;

	/* initialize stack                                     */
	stack = (UWORD FAR *)&error_tos;

	/* Init oem hook - returns memory size in KB    */
	ram_top = init_oem();

	/* Initialize driver chain                                      */
	dhp = link_dhdr(dhp, (struct dhdr FAR *)&con_dev);
	dhp = link_dhdr(dhp, (struct dhdr FAR *)&clk_dev);
	dhp = link_dhdr(dhp, (struct dhdr FAR *)&blk_dev);
	syscon = (struct dhdr FAR *)&con_dev;
	clock = (struct dhdr FAR *)&clk_dev;

	for (i = 0x20; i <= 0x3f; i++)
	     setvec(i, empty_handler);

	/* set interrupt vectors                                        */
	setvec(0x20, int20_handler);
	setvec(0x21, int21_handler);
	setvec(0x22, int22_handler);
	setvec(0x23, int23_handler);
	setvec(0x24, int24_handler);
	setvec(0x25, int25_handler);
	setvec(0x26, int26_handler);
	setvec(0x27, int27_handler);
	setvec(0x28, int28_handler);
	setvec(0x29, int29_handler);    /* Requires Fast Con Driver     */
	setvec(0x2f, int2f_handler);

	/* Initialize stacks                                            */
	api_sp = FP_OFF(stack);
	api_ss = FP_SEG(stack);

	/* Initialize the screen handler for backspaces                 */
	scr_pos = 0;

	/* Do first initialization of system variable buffers so that   */
	/* we can read config.sys later.                                */
	PreConfig();

	/* Now config the temporary file system                         */
	FsConfig();
	
	/* Now process CONFIG.SYS                                       */
	DoConfig();

	/* and do final buffer allocation.                              */
	PostConfig();

	/* Now config the final file system                             */
	FsConfig();

	/* and process CONFIG.SYS one last time to load device drivers. */
	DoConfig();

	/* Now to initialize all special flags, etc.                    */
	mem_access_mode = FIRST_FIT;
	break_ena = TRUE;
	verify_ena = FALSE;
	InDOS = 0;
	version_flags = 0;
	pDirFileNode = 0;
	switchar = '/';
}


VOID FsConfig(VOID)
{
	REG COUNT i;

	/* Initialize the file tables                                   */
	for(i = 0; i < Config.cfgFiles; i++)
		f_nodes[i].f_count = 0;

	/* The system file tables need special handling and are "hand	*/
	/* built. Included is the stdin, stdout, stdaux and atdprn.     */
	sfthead -> sftt_next = (sfttbl FAR *)-1;
	sfthead -> sftt_count = SFTMAX;
	for(i = 0; i < sfthead -> sftt_count ; i++)
	{
		sfthead -> sftt_table[i].sft_count = 0;
		sfthead -> sftt_table[i].sft_status = -1;
	}
	/* 0 is /dev/con (stdin)                                        */
	sfthead -> sftt_table[0].sft_count = 1;
	sfthead -> sftt_table[0].sft_mode = SFT_MREAD;
	sfthead -> sftt_table[0].sft_attrib = 0;
	sfthead -> sftt_table[0].sft_flags =
	  (con_dev.dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FEOF | SFT_FSTDIN | SFT_FSTDOUT;
	sfthead -> sftt_table[0].sft_psp = DOS_PSP;
	fbcopy(
		(VOID FAR *)"CON        ",
		(VOID FAR *)sfthead -> sftt_table[0].sft_name, 11);
	sfthead -> sftt_table[0].sft_dev = (struct dhdr FAR *)&con_dev;

	/* 1 is /dev/con (stdout)                                       */
	sfthead -> sftt_table[1].sft_count = 1;
	sfthead -> sftt_table[1].sft_mode = SFT_MWRITE;
	sfthead -> sftt_table[1].sft_attrib = 0;
	sfthead -> sftt_table[1].sft_flags = 
	  (con_dev.dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FEOF | SFT_FSTDIN  | SFT_FSTDOUT;
	sfthead -> sftt_table[1].sft_psp = DOS_PSP;
	fbcopy(
		(VOID FAR *)"CON        ",
		(VOID FAR *)sfthead -> sftt_table[1].sft_name, 11);
	sfthead -> sftt_table[1].sft_dev = (struct dhdr FAR *)&con_dev;

	/* 2 is /dev/con (stderr)                                       */
	sfthead -> sftt_table[2].sft_count = 1;
	sfthead -> sftt_table[2].sft_mode = SFT_MWRITE;
	sfthead -> sftt_table[2].sft_attrib = 0;
	sfthead -> sftt_table[2].sft_flags = 
	  (con_dev.dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FEOF | SFT_FSTDIN  | SFT_FSTDOUT;
	sfthead -> sftt_table[2].sft_psp = DOS_PSP;
	fbcopy(
		(VOID FAR *)"CON        ",
		(VOID FAR *)sfthead -> sftt_table[2].sft_name, 11);
	sfthead -> sftt_table[2].sft_dev = (struct dhdr FAR *)&con_dev;

	/* 3 is /dev/aux (/dev/null for now)                            */
	sfthead -> sftt_table[3].sft_count = 1;
	sfthead -> sftt_table[3].sft_mode = SFT_MWRITE;
	sfthead -> sftt_table[3].sft_attrib = 0;
	sfthead -> sftt_table[3].sft_flags = 
	  (nul_dev.dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FNUL;
	sfthead -> sftt_table[3].sft_psp = DOS_PSP;
	fbcopy(
		(VOID FAR *)"NUL        ",
		(VOID FAR *)sfthead -> sftt_table[3].sft_name, 11);
	sfthead -> sftt_table[3].sft_dev = (struct dhdr FAR *)&nul_dev;

	/* 4 is /dev/prn (/dev/null for now)                            */
	sfthead -> sftt_table[4].sft_count = 1;
	sfthead -> sftt_table[4].sft_mode = SFT_MWRITE;
	sfthead -> sftt_table[4].sft_attrib = 0;
	sfthead -> sftt_table[4].sft_flags =
	  (nul_dev.dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FNUL;
	sfthead -> sftt_table[4].sft_psp = DOS_PSP;
	fbcopy(
		(VOID FAR *)"NUL        ",
		(VOID FAR *)sfthead -> sftt_table[4].sft_name, 11);
	sfthead -> sftt_table[4].sft_dev = (struct dhdr FAR *)&nul_dev;

	/* Log-in the default drive.                                    */
	/* Get the boot drive from the ipl and use it for default.      */
	default_drive = 0; /* C if not A                */

	/* Initialzie the current directory structures                  */
	for(i = 0; i < NDEVS; i++)
		scopy("\\", blk_devices[i].dpb_path);

	/* Initialze the disk buffer management functions               */
	init_buffers();
}


static VOID signon()
{
	printf("\nDOS-C compatibility %d.%02d\n%s\n",
		os_major, os_minor, copyright);
	printf(os_release, REVISION_MAJOR, REVISION_MINOR, BUILD);
}


static VOID kernel()
{
	seg asize;
	BYTE FAR *ep, *sp;
	COUNT ret_code;
#ifndef KDB
	static BYTE *path = "PATH=";
#endif

#ifdef KDB
	kdb();
#else
	/* create the master environment area                           */
	cu_psp = 8;    /* to get the right signature */
	if(DosMemAlloc(0x20, FIRST_FIT, (seg FAR *)&master_env, (seg FAR *)&asize) < 0)
		fatal("cannot allocate master environment space");

	/* populate it with the minimum environment                     */
	++master_env;       
	ep = MK_FP(master_env, 0);   
	
	for(sp = path; *sp != 0; )
		*ep++ = *sp++;

	*ep++ = '\0';
	*ep++ = '\0';
	*((int *)ep)++ = 0;    
#endif
	ret_code = p_0();
	exit(ret_code);
}

/* process 0                                                            */
static COUNT
p_0(VOID)
{
	exec_blk exb;
	CommandTail Cmd;
	BYTE FAR *szfInitialPrgm = (BYTE FAR *)Config.cfgInit;

	/* Execute command.com /P from the drive we just booted from    */
	exb.exec.env_seg = master_env;
	strcpy(Cmd.ctBuffer, Config.cfgInitTail);
	Cmd.ctCount = 6;
	exb.exec.cmd_line = (CommandTail FAR *)&Cmd;
	exb.exec.fcb_1 = exb.exec.fcb_2 = (fcb FAR *)0;
#ifdef DEBUG
	printf("Process 0 starting: %s\n\n", (BYTE *)szfInitialPrgm);
#endif 
	if(DosExec(0, (exec_blk FAR *)&exb, szfInitialPrgm) != SUCCESS)
	{
		printf("\nBad or missing Command Interpreter.\n");
		return -1;
	}
	else
	{
		printf("\nSystem shutdown complete\nReboot now.\n");
		return 0;
	}
}


VOID 
init_device (struct dhdr FAR *dhp)
{
	request rq;

	rq.r_unit = 0;
	rq.r_status = 0;
	rq.r_command = C_INIT;
	rq.r_length = sizeof(request);
	execrh((request FAR *)&rq, dhp);

	/* check for a block device and update  device control block    */
	if(!(dhp -> dh_attr & ATTR_CHAR) && (rq.r_nunits != 0))
	{
		REG COUNT Index;

		for(Index = 0; Index < rq.r_nunits; Index++, BlockIndex++)
		{
			if (lastdrive)
			    blk_devices[lastdrive - 1].dpb_next = &blk_devices[lastdrive];

			blk_devices[lastdrive].dpb_unit = lastdrive;
			blk_devices[lastdrive].dpb_subunit = Index;
			blk_devices[lastdrive].dpb_device = dhp;
			blk_devices[lastdrive].dpb_flags = M_CHANGED;
			++lastdrive;
			++nblkdev;
		}
		blk_devices[lastdrive].dpb_next = (void FAR *)0xFFFFFFFF;
	}
	DPBp = &blk_devices[0];
}


struct dhdr FAR *link_dhdr(struct dhdr FAR *lp, struct dhdr FAR *dhp)
{
	lp -> dh_next = dhp;
	init_device(dhp);
	return dhp;
}


